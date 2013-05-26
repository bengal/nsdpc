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

#ifndef _PACKET_H_
#define _PACKET_H_

#include <stdio.h>
#include <stdint.h>

#define ETH_ALEN	6

#define SIGN		"NSDP"
#define SIGN_LEN	4

enum {
	OP_READ_REQ	= 1,
	OP_READ_RESP	= 2,
	OP_WRITE_REQ	= 3,
	OP_WRITE_RESP	= 4
};

/* Record types */
enum {
	RT_MODEL	= 0x0001,
	RT_NAME		= 0x0003,
	RT_MAC		= 0x0004,
	RT_IP		= 0x0006,
	RT_NEW_PASSWORD	= 0x0009,
	RT_PASSWORD	= 0x000a,
	RT_DHCP		= 0x000b,
	RT_FIRMWARE	= 0x000d,
	RT_REBOOT	= 0x0013,
	RT_SPEED	= 0x0c00,
	RT_PORT_STAT	= 0x1000,
	RT_VLAN_SUPP	= 0x2000,
	RT_NPORTS	= 0x6000,
	RT_TAIL		= 0xffff
};

struct record_h {
	uint16_t	type;
	uint16_t	len;
};

#define REC_H_LEN	sizeof(struct record_h)

struct msg_h {
	uint8_t		ver;
	uint8_t		op;
	uint8_t		rsvd1[6];
	uint8_t		src[ETH_ALEN];
	uint8_t		dst[ETH_ALEN];
	uint8_t		rsvd2[2];
	uint16_t	seq;
	uint8_t		sig[4];
	uint8_t		rsvd3[4];
};

#define MSG_H_LEN	sizeof(struct msg_h)

struct packet;
struct packet_list;

struct packet *packet_new(uint8_t *src, uint8_t *dst, int op);
int packet_len(struct packet *packet);
char * packet_data(struct packet *packet);
void packet_add_record(struct packet *packet, int type, int len, char *data);
void packet_add_tail(struct packet *packet);
void packet_free(struct packet *packet);
void packet_dump(struct packet *packet, FILE *out, int direction);
struct packet *packet_list_next(struct packet_list *list);
struct packet *packet_new_buffer(char *data, char len);
void packet_print_records(struct packet *packet);

#endif /* _PACKET_H_ */
