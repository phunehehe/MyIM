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


//#include <sys/types.h>
//#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


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

        listener = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

        if (listener != -1) { 

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

    return 0;
}

