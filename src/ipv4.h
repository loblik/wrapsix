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

#ifndef IPV4_H
#define IPV4_H

#include <stdint.h>

/* IPv4 flags */
#define IPV4_FLAG_DONT_FRAGMENT		0x4000
#define IPV4_FLAG_MORE_FRAGMENTS	0x2000

/* IPv4 address structure */
struct s_ipv4_addr {
	uint8_t addr[4];
} __attribute__ ((__packed__));

/* IPv4 header structure */
struct s_ipv4 {
	uint8_t     		ver_hdrlen;	/*  4 b; version,
						    4 b; header length in 4 B */
	uint8_t         	tos;		/*  8 b; type of service */
    uint16_t            len;		/* 16 b; total packet length */
    uint16_t            id;		/* 16 b; id of the packet
                                    (for fragmentation) */
    uint16_t            flags_offset;	/*  3 b; flags,
						   13 b; fragment offset in B */
	uint8_t             ttl;		/*  8 b; time to live */
	uint8_t             proto;		/*  8 b; protocol in payload */
	uint16_t            checksum;	/* 16 b */
	struct s_ipv4_addr	ip_src;		/* 32 b; source address */
	struct s_ipv4_addr	ip_dest;	/* 32 b; destination address */
} __attribute__ ((__packed__));

/* IPv4 pseudoheader structure for checksum */
struct s_ipv4_pseudo {
	struct s_ipv4_addr	ip_src;		/* 32 b; source address */
	struct s_ipv4_addr	ip_dest;	/* 32 b; destination address */
	uint8_t             zeros;		/*  8 b */
	uint8_t         	proto;		/*  8 b; protocol in payload */
	uint16_t    		len;		/* 16 b; payload length */
} __attribute__ ((__packed__));

/* IPv4 pseudoheader structure for checksum update */
struct s_ipv4_pseudo_delta {
	struct s_ipv4_addr	ip_src;		/* 32 b; source address */
	struct s_ipv4_addr	ip_dest;	/* 32 b; destination address */
	uint16_t            port;		/* 16 b; transport layer
							 address */
} __attribute__ ((__packed__));

int ipv4(char *packet, unsigned short length);

#endif /* IPV4_H */
