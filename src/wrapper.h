/*
 *  WrapSix
 *  Copyright (C) 2008-2013  xHire <xhire@wrapsix.org>
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

#ifndef WRAPPER_H
#define WRAPPER_H

#include "ipv4.h"
#include "ipv6.h"
#include <arpa/inet.h>


/* +++ INTERNAL CONFIGURATION +++ */
#define MAX_MTU		1500	/* maximum MTU on IPv6 side */
#define PACKET_BUFFER	1514	/* buffer for any packet */
/* --- INTERNAL CONFIGURATION --- */

#define IP4_TXT(array, ip) \
    char (array)[100]; \
    inet_ntop(AF_INET, ip, array, sizeof(array)); \

#define IP6_TXT(array, ip) \
    char (array)[100]; \
    inet_ntop(AF_INET6, ip, array, sizeof(array)); \

/* MAC address structure */
struct s_mac_addr {
       unsigned char           addr[6];
} __attribute__ ((__packed__));

extern int tun_fd;

extern unsigned short		mtu;
extern struct ifreq		interface;
extern struct s_mac_addr	mac;
extern struct s_ipv6_addr	ndp_multicast_addr;
extern struct s_ipv6_addr	wrapsix_ipv6_prefix;
extern struct s_ipv4_addr	wrapsix_ipv4_addr;
extern struct s_ipv6_addr	host_ipv6_addr;
extern struct s_ipv4_addr	host_ipv4_addr;

void ipv6_to_ipv4(struct s_ipv6_addr *ipv6_addr, struct s_ipv4_addr *ipv4_addr);
void ipv4_to_ipv6(struct s_ipv4_addr *ipv4_addr, struct s_ipv6_addr *ipv6_addr);

#endif /* WRAPPER_H */
