/*
 * nsdpc : management tool for Netgear switches supporting NSPD protocol
 *
 * Copyright (C) 2013 <b.galvani@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _NET_H_
#define _NET_H_

#include <stdint.h>

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#include "packet.h"

int init_sock(const char *ifname);
int get_if_mac(const char *ifname, uint8_t *dst);
int net_send_packet(int sock, struct packet *packet);
struct packet *net_receive_response(int socket);

#endif /* _NET_H_ */
