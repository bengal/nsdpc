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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/ether.h>

#include "packet.h"

struct packet {
	char	*data;
	int	len;
	int	mlen;
	char	*ptr;
};

struct packet *packet_new(uint8_t *src, uint8_t *dst, int op)
{
	struct packet *packet;
	struct msg_h *header;
	int size;

	srand(time(NULL));

	packet = calloc(1, sizeof(struct packet));
	if (!packet)
		return NULL;

	size = MSG_H_LEN + 32;

	packet->data = calloc(1, size);
	if (!packet->data) {
		free(packet);
		return NULL;
	}
	packet->len = MSG_H_LEN;
	packet->mlen = size;

	header = (struct msg_h *)packet->data;
	header->ver = 1;
	header->op = op;
	header->seq = htons(rand());
	memcpy(header->src, src, ETH_ALEN);
	memcpy(header->dst, dst, ETH_ALEN);
	memcpy(header->sig, SIGN, SIGN_LEN);

	return packet;
}

struct packet *packet_new_buffer(char *data, char len)
{
	struct packet *packet;

	packet = calloc(1, sizeof(struct packet));
	if (!packet)
		return NULL;
	packet->data = malloc(len);
	if (!packet->data) {
		free(packet);
		return NULL;
	}
	memcpy(packet->data, data, len);
	packet->len = len;
	packet->mlen = len;
	return packet;
}

char *packet_data(struct packet *packet)
{
	return packet->data;
}

int packet_len(struct packet *packet)
{
	return packet->len;
}

void packet_add_record(struct packet *packet, int type, int len, char *data)
{
	struct record_h *header;
	int rec_size = len + REC_H_LEN;
	int new_size;

	if (packet->len + rec_size >= packet->mlen) {
		new_size = packet->len + rec_size;
		packet->data = realloc(packet->data, new_size);
		packet->mlen = new_size;
	}

	header = (struct record_h *)(packet->data + packet->len);
	header->type = htons(type);
	header->len = htons(len);
	memcpy(packet->data + REC_H_LEN, data, len);
	packet->len += rec_size;
}

void packet_add_tail(struct packet *packet)
{
	packet_add_record(packet, RT_TAIL, 0, NULL);
}

void packet_free(struct packet *packet)
{
	if (packet) {
		if (packet->data)
			free(packet->data);
		free(packet);
	}
}

void packet_dump(struct packet *packet, FILE *out, int direction)
{
	int i;

	printf("%s PACKET: ", direction ? "OUT" : "IN ");

	for (i = 0; i < packet->len; i++) {
		fprintf(out, "%02X ", packet->data[i]);
	}
	fprintf(out, "\n");
}

void packet_parse_records(struct packet *packet,
			  void (*fn)(int type, int len, char *data))
{
	char *ptr = packet->data;
	struct record_h *record;
	int type, len;

	ptr += MSG_H_LEN;

	while (ptr + REC_H_LEN < packet->data + packet->len) {
		record = (struct record_h *)ptr;
		type = ntohs(record->type);
		len = ntohs(record->len);
		if (ptr + REC_H_LEN + len >= packet->data + packet->len)
			break;
		fn(type, len, ptr + REC_H_LEN);
		ptr += REC_H_LEN + len;
	}
}

void rec_printer_str(int len, char *data)
{
	char *buffer;

	buffer = alloca(len + 1);
	memcpy(buffer, data, len);
	buffer[len] = 0;
	printf("%s", buffer);
}

void rec_printer_mac(int len, char *data)
{
	if (len == ETH_ALEN)
		printf("%s", ether_ntoa((struct ether_addr *)data));
	else
		printf("malformed address");
}

void rec_printer_ip(int len, char *data)
{
	struct in_addr *addr = (struct in_addr *)data;
	printf("%s", inet_ntoa(*addr));
}

void rec_printer_int(int len, char *data)
{
	if (len == 1)
		printf("%d", *data);
}

void rec_printer_boolean(int len, char *data)
{
	if (len != 1 || (*data != 0 && *data != 1))
		printf("unknown");
	else if (*data == 1)
		printf("true");
	else
		printf("false");
}

struct {
	int type;
	char *desc;
	void (*printer)(int len, char *data);
} record_printers[] = {
	{ RT_MODEL,	"Model",	rec_printer_str },
	{ RT_NAME,	"Name",		rec_printer_str },
	{ RT_MAC,	"MAC",		rec_printer_mac },
	{ RT_PASSWORD,	"Password",	rec_printer_str },
	{ RT_IP,	"IP",		rec_printer_ip },
	{ RT_DHCP,	"DHCP enabled",	rec_printer_boolean },
	{ RT_FIRMWARE,	"Firmware",	rec_printer_str},
	{ RT_NPORTS,	"Num of ports",	rec_printer_int }
};

#define NUM_REC_PRINTERS ((sizeof record_printers)/(sizeof record_printers[0]))

void print_record(int type, int len, char *data)
{
	int i;

	for (i = 0; i < NUM_REC_PRINTERS; i++) {
		if (type == record_printers[i].type) {
			printf("%-16s: ", record_printers[i].desc);
			record_printers[i].printer(len, data);
			printf("\n");
			break;
		}
	}
}

void packet_print_records(struct packet *packet)
{
	packet_parse_records(packet, print_record);
}
