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
#include <unistd.h>
#include <string.h>
#include <netinet/ether.h>

#include "net.h"
#include "packet.h"

enum {
	CMD_SCAN,
	CMD_GET,
	CMD_SET
};

struct ctx {
	char	*ifname;
	uint8_t my_mac[ETH_ALEN];
	uint8_t	dst_mac[ETH_ALEN];
	int	sock;
	int	cmd;
	char	*arg_name;
	char	*arg_value;
};

void usage(char *prog)
{
	printf(
		"%s [options] <command> [arguments]\n"
		"\n"
		" Available commands:\n"
		"    scan               Returns a list of devices on current network\n"
		"    set <name> <val>   Set a property\n"
		"    get <name>         Get a property\n"
		"\n"
		" Available options:\n"
		"    -i <ifname>        Set network interaface to bind to\n"
		"    -d <addr>          Set destination MAC address\n"
		"\n",
		prog);
}

void do_cmd(struct ctx *ctx)
{
	struct packet *packet;

	ctx->sock = init_sock(ctx->ifname);
	if (ctx->sock < 0) {
		printf("could not initialize socket\n");
		return;
	}

	if (get_if_mac(ctx->ifname, ctx->my_mac)) {
		printf("Could not retrieve interface mac\n");
		return;
	}

	if (ctx->cmd == CMD_SCAN) {
		packet = packet_new(ctx->my_mac, ctx->dst_mac, OP_READ_REQ);
		packet_add_record(packet, RT_MODEL, 0, NULL);
		packet_add_record(packet, RT_NAME, 0, NULL);
		packet_add_record(packet, RT_MAC, 0, NULL);
		packet_add_record(packet, RT_IP, 0, NULL);
		packet_add_record(packet, RT_DHCP, 0, NULL);
		packet_add_record(packet, RT_FIRMWARE, 0, NULL);
		packet_add_record(packet, RT_NPORTS, 0, NULL);
		packet_add_tail(packet);
#ifdef DEBUG
		packet_dump(packet, stdout, 1);
#endif
		net_send_packet(ctx->sock, packet);
		packet_free(packet);
		packet = net_receive_response(ctx->sock);
		if (packet) {
#ifdef DEBUG
			packet_dump(packet, stdout, 0);
#endif
			printf("---------- Response ----------\n");
			packet_print_records(packet);
		}
	} else {
		printf("net implemented yet\n");
	}

}

int main(int argc, char **argv)
{
	struct ctx ctx;
	int opt;
	char *addr;

	memset(&ctx, 0, sizeof(struct ctx));

	while ((opt = getopt(argc, argv, "d:i:h")) != -1) {
		switch (opt) {
		case 'd':
			addr = (char *)ether_aton(optarg);
			if (!addr) {
				printf("Invalid destination mac %s\n", optarg);
				return 1;
			}
			memcpy(ctx.dst_mac, addr, ETH_ALEN);
			break;
		case 'i':
			ctx.ifname = strdup(optarg);
			break;
		case 'h':
			usage(argv[0]);
			return 0;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (!argv[optind]) {
		printf("Command is required\n");
		return 1;
	}

	if (!ctx.ifname) {
		printf("Interface name is required\n");
		return 1;
	}

	if (!strcmp(argv[optind], "scan")) {
		ctx.cmd = CMD_SCAN;
	} else if (!strcmp(argv[optind], "set")) {
		if (optind + 2 >= argc) {
			printf("Not enough arguments for set command\n");
			return 1;
		}
		ctx.cmd = CMD_SET;
		ctx.arg_name = strdup(argv[optind+1]);
		ctx.arg_value = strdup(argv[optind+2]);
	} else if (!strcmp(argv[optind], "get")) {
		if (optind + 1 >= argc) {
			printf("Not enough arguments for get command\n");
			return 1;
		}
		ctx.cmd = CMD_GET;
		ctx.arg_name = strdup(argv[optind+1]);
	} else {
		printf("Unrecognized command '%s'\n", argv[optind]);
		usage(argv[0]);
	}

	do_cmd(&ctx);

	return 0;
}
