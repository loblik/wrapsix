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

#ifndef UDP_H
#define UDP_H

/* UDP header structure */
struct s_udp {
	uint16_t    port_src;	/* 16 b; source port */
	uint16_t    port_dest;	/* 16 b; destination port */
	uint16_t    len;		/* 16 b; header + data length */
	uint16_t    checksum;	/* 16 b; optional checksum */
} __attribute__ ((__packed__));

int udp_ipv4(struct s_ipv4 *ip4, char *payload,
	     unsigned short payload_size);
int udp_ipv6(struct s_ipv6 *ip6, char *payload,
	     unsigned short payload_size);

#endif /* UDP_H */
