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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#include "net.h"

int init_sock(const char *ifname)
{
	struct sockaddr_in addr;
	char name[IFNAMSIZ];
	int val, sock;
#if 0
	struct ifreq ifr;
#endif

	/* Create the socket */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

#if 0
	/* Obtain IP address of the interface */
	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
		perror("could not get interface IP");
		return -1;
	};
#endif

	/* Bind to desired source address and port */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
#if 0
	addr.sin_addr.s_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
#endif
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(63321);

	if (bind(sock, (struct sockaddr *)&addr,
		 sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		return -1;
	}

	/* Allow transmission of broadcast packets */
	val = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &val, sizeof(int)) < 0) {
		perror("setsockopt broadcast");
		return -1;
	}

	memset(name, 0, IFNAMSIZ);
	strncpy(name, ifname, IFNAMSIZ);
	if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, name, IFNAMSIZ) < 0) {
		perror("setsockopt bind to device");
		return -1;
	}

	return sock;
}

int get_if_mac(const char *ifname, uint8_t *dst)
{
	int sock;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (!sock) {
		perror("socket");
		return -1;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	if (ioctl(sock, SIOCGIFHWADDR, &ifr)) {
		perror("ioctl");
		return -1;
	}

	memcpy(dst, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	return 0;
}

int net_send_packet(int sock, struct packet *packet)
{
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_BROADCAST;
	addr.sin_port = htons(63322);

	return sendto(sock, packet_data(packet), packet_len(packet), 0,
		      (struct sockaddr *)&addr, sizeof(addr));
}

struct packet *net_receive_response(int socket)
{
	char buffer[2048];
	int ret;
	struct timeval tv;

	tv.tv_sec = 3;
	tv.tv_usec = 0;

	setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	ret = recvfrom(socket, buffer, sizeof(buffer), 0, NULL, 0);

	if (ret < 0) {
		printf("no response\n");
		return NULL;
	} else {
		return packet_new_buffer(buffer, ret);
	}
}
