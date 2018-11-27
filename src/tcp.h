/*
 *  WrapSix
 *  Copyright (C) 2008-2017  xHire <xhire@wrapsix.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TCP_H
#define TCP_H

#define TCP_FLAG_FIN	0x01
#define TCP_FLAG_SYN	0x02
#define TCP_FLAG_RST	0x04

#define TCP_STATE_INIT	1
#define TCP_STATE_EST	2
#define TCP_STATE_FIN4	3
#define TCP_STATE_FIN6	4
#define TCP_STATE_FIN64	5
#define TCP_STATE_TRANS	6

/* TCP header structure */
struct s_tcp {
	uint16_t    port_src;	/* 16 b; source port */
	uint16_t    port_dest;	/* 16 b; destination port */
	uint32_t    seq;		/* 32 b; sequence number */
	uint32_t    ack;		/* 32 b; acknowledgement number */
	uint8_t     offset;		/*  4 b; data offset
					    6 b; reserved (zeros) */
	uint8_t     flags;		/*  6 b; flags */
	uint16_t    window;		/* 16 b; size of the receive window */
	uint16_t    checksum;	/* 16 b */
} __attribute__ ((__packed__));

int tcp_ipv4(struct s_ipv4 *ip4, char *payload,
	     unsigned short payload_size);
int tcp_ipv6(struct s_ipv6 *ip6, char *payload,
	     unsigned short payload_size);

#endif /* TCP_H */
