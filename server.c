/*
 *   The contents of this file are subject to the Mozilla Public License
 *   Version 1.1 (the "License"); you may not use this file except in
 *   compliance with the License. You may obtain a copy of the License at
 *   http://www.mozilla.org/MPL/

 *   Software distributed under the License is distributed on an "AS IS"
 *   basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *   License for the specific language governing rights and limitations
 *   under the License.

 *   The Original Code is MyIM.

 *   The Initial Developer of the Original Code is Hoang Xuan Phu.
 *   Portions created by the Initial Developer are Copyright (C) 2011
 *   the Initial Developer. All Rights Reserved.

 *   Contributor(s): Hoang Xuan Phu.

 *   Alternatively, the contents of this file may be used under the terms
 *   of the GNU Affero General Public License version 3 or later
 *   (the "AGPL License"), in which case the
 *   provisions of the AGPL License are applicable instead of those
 *   above.  If you wish to allow use of your version of this file only
 *   under the terms of the AGPL License and not to allow others to use
 *   your version of this file under the MPL, indicate your decision by
 *   deleting  the provisions above and replace  them with the notice and
 *   other provisions required by the AGPL License.  If you do not delete
 *   the provisions above, a recipient may use your version of this file
 *   under either the MPL or the AGPL License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(void) {

    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status;
    struct addrinfo *res;

    if ((status = getaddrinfo(NULL, "3490", &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    int listener;
    int yes = 1;
    struct addrinfo *servinfo;

    for (servinfo = res; servinfo != NULL; servinfo = servinfo->ai_next) {

        listener = socket(servinfo->ai_family,
                servinfo->ai_socktype,
                servinfo->ai_protocol);

        if (listener != -1) { 

            // lose the pesky "address already in use" error message
            setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

            if (bind(listener, servinfo->ai_addr, servinfo->ai_addrlen) != -1) {
                break;
            } else {
                close(listener);
            }
        }
    }

    if (servinfo == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(res);

    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
    int fdmax;        // maximum file descriptor number
    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        int i;
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    struct sockaddr_storage remoteaddr; // client address
                    socklen_t addrlen;
                    addrlen = sizeof remoteaddr;
                    int newfd;        // newly accept()ed socket descriptor
                    newfd = accept(listener,
                            (struct sockaddr *)&remoteaddr,
                            &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        char remoteIP[INET6_ADDRSTRLEN];
                        printf("selectserver: new connection from %s on "
                                "socket %d\n",
                                inet_ntop(remoteaddr.ss_family,
                                    get_in_addr((struct sockaddr*)&remoteaddr),
                                    remoteIP, INET6_ADDRSTRLEN),
                                newfd);
                    }
                } else {
                    // handle data from a client
                    int nbytes;
                    char buf[256];    // buffer for client data
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        int j;
                        for(j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!

    return 0;
}

