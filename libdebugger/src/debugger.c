/*
 * Copyright 2024, UNSW Sydney
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 */

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <utils/util.h>
#include <debugger/debugger.h>
#include <sos/gen_config.h>
#undef PACKED
#include <pico_socket.h>
#include <pico_ipv4.h>


#define AOS_BASEPORT (26800)
#define MAX_PAYLOAD_SIZE 1024


struct debugger {
	struct pico_ip4 inaddr_any;
	struct pico_socket *pico_socket;
	struct pico_socket *accepted_pico_socket;
	// void (*handler)(struct debugger *debugger, char c);
	uint32_t peer;
	uint16_t port;
};

static char buf[MAX_PAYLOAD_SIZE];
static struct debugger debugger = {};

/* */
static void debugger_recv_handler(uint16_t ev, struct pico_socket *s) {
	char *read_ptr = buf;
	if (ev == PICO_SOCK_EV_CONN) {
		ZF_LOGE("debugger: Got connection request");
		if (debugger.accepted_pico_socket == NULL) {
			struct pico_ip4 addr;
			uint16_t port;
			debugger.accepted_pico_socket = pico_socket_accept(debugger.pico_socket,
															   &addr, &port);
		}
	}
	else if (ev == PICO_SOCK_EV_RD /* && debugger.handler */) {
		ZF_LOGE("debugger: Got a packet");
		int read = 0;
		do {
			read = pico_socket_recvfrom(debugger.accepted_pico_socket, buf,
										MAX_PAYLOAD_SIZE,
										&debugger.peer, &debugger.port);
			if (read_ptr >= buf + MAX_PAYLOAD_SIZE) {
				ZF_LOGE("Exceeded buffer size");
			}
			printf("Read: %d\n", read);
			read_ptr += read;
		} while (read > 0);
		// *read_ptr = 0;
		// printf("%s\n", buf);
	} else if (ev == PICO_SOCK_EV_ERR) {
		ZF_LOGE("Pico recv error");
	} else if (ev == PICO_SOCK_EV_CLOSE) {
		pico_socket_close(debugger.accepted_pico_socket);
	} else {
		ZF_LOGE("Got another packet %d", ev);
	}
}


struct debugger *debugger_init(void) {
	if (debugger.pico_socket != NULL) {
		ZF_LOGE("Debugger already initialised");
		return NULL;
	}

	debugger.pico_socket = pico_socket_open(PICO_PROTO_IPV4, PICO_PROTO_TCP, &debugger_recv_handler);
	if (!debugger.pico_socket) {
		ZF_LOGE("Debugger connection failed");
		return NULL;
	}

	struct pico_ip4 gateway;
	pico_string_to_ipv4(CONFIG_SOS_GATEWAY, &gateway.addr);

	struct pico_ip4 *src = pico_ipv4_source_find(&gateway);
	unsigned char *octets = (unsigned char *) &src->addr;
	printf("libdebugger using udp port %d\n", AOS_BASEPORT + octets[3]);

	/* Configure peer/port ofr sendto */
	pico_string_to_ipv4(CONFIG_SOS_GATEWAY, &debugger.peer);
	uint16_t port_be = short_be(AOS_BASEPORT + octets[3]);
	debugger.port = port_be;

	int err = pico_socket_bind(debugger.pico_socket, &debugger.inaddr_any, &port_be);
	if (err) {
		return NULL;
	}

	err = pico_socket_listen(debugger.pico_socket, 10);

	//err = pico_socket_connect(network_console.pico_socket. &network_console.peer)

	return &debugger;
}


