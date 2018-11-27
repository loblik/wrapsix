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

#define _DEFAULT_SOURCE

#include <arpa/inet.h>		/* inet_pton */
#include <linux/ethtool.h>	/* struct ethtool_value */
#include <linux/sockios.h>	/* SIOCETHTOOL */
#include <linux/ip.h>
#include <net/if.h>		/* struct ifreq */
#include <netinet/in.h>		/* htons */
#include <netpacket/packet.h>	/* struct packet_mreq, struct sockaddr_ll */
#include <stdio.h>		/* perror */
#include <stdlib.h>		/* srand */
#include <string.h>		/* strncpy */
#include <sys/ioctl.h>		/* ioctl, SIOCGIFINDEX */
#include <sys/types.h>		/* caddr_t */
#include <time.h>		/* time, time_t */
#include <unistd.h>		/* close */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
#include "autoconfig.h"
#endif /* HAVE_CONFIG_H */
#include "config.h"
#include "icmp.h"
#include "ipv4.h"
#include "ipv6.h"
#include "log.h"
#include "nat.h"
#include "transmitter.h"
#include "wrapper.h"

unsigned short mtu;

struct ifreq		interface;
struct s_mac_addr	mac;
struct s_ipv6_addr	ndp_multicast_addr;
struct s_ipv6_addr	wrapsix_ipv6_prefix;
struct s_ipv4_addr	wrapsix_ipv4_addr;
struct s_ipv6_addr	wrapsix_ipv6_addr;
int tun_fd;

static int process(char *packet, unsigned short length);

int main(int argc, char **argv)
{
	struct s_cfg_opts	cfg;

	struct packet_mreq	pmr;
	struct ethtool_value	ethtool;

	int	length;
	char	buffer[PACKET_BUFFER];

	int	i;
	time_t	prevtime, curtime;

	log_info(PACKAGE_STRING " is starting");

	/* load configuration */
	if (argc == 1) {
		cfg_parse(SYSCONFDIR "/wrapsix.conf", &mtu, &cfg, 1);
	} else {
		cfg_parse(argv[1], &mtu, &cfg, 1);
	}

	log_info("Using: interface %s", cfg.interface);
	log_info("       NAT64 prefix %s", cfg.prefix);
	log_info("       MTU %d", mtu);

	inet_pton(AF_INET, cfg.ipv4_address, &wrapsix_ipv4_addr);
	inet_pton(AF_INET6, cfg.ipv6_address, &wrapsix_ipv6_addr);

	/* using block because of the temporary variable */
	{
		char ip_text[40];


		inet_ntop(AF_INET, &wrapsix_ipv4_addr, ip_text, sizeof(ip_text));
		log_info("       IPv4 address %s", ip_text);
		inet_ntop(AF_INET6, &wrapsix_ipv6_addr, ip_text, sizeof(ip_text));
		log_info("       IPv6 address %s", ip_text);
	}

	/* some preparations */
	/* compute binary IPv6 address of NDP multicast */
	inet_pton(AF_INET6, "ff02::1:ff00:0", &ndp_multicast_addr);

	/* compute binary IPv6 address of WrapSix prefix */
	inet_pton(AF_INET6, cfg.prefix, &wrapsix_ipv6_prefix);

	/* compute binary IPv4 address of WrapSix */
	inet_pton(AF_INET, cfg.ipv4_address, &wrapsix_ipv4_addr);

	/* initiate sending socket */
	if (!transmission_init(cfg)) {
		log_error("Unable to initiate sending socket");
		return 1;
	}

	/* initiate NAT tables */
	nat_init();

	/* initiate random numbers generator */
	srand((unsigned int) time(NULL));

	/* initialize time */
	prevtime = time(NULL);

	/* read packets from tun device */
	for (i = 1;; i++) {
		length = transmission_read(buffer, PACKET_BUFFER);
		if (length < 0) {
			log_error("Unable to retrieve data from socket");
			return 1;
		}

		if (length > PACKET_BUFFER) {
			log_error("Received packet is too big (%d B). Please "
				  "tune NIC offloading features and report "
				  "this issue to " PACKAGE_BUGREPORT, length);
			continue;
		}

		process(buffer, length);

		if (i % 250000) {
			curtime = time(NULL);
			/* 2 seconds is minimum normal timeout */
			if ((curtime - prevtime) >= 2) {
				nat_cleaning();
				prevtime = curtime;
			}
			i = 0;
		}
	}

	/* clean-up */
	transmission_quit();

	/* empty NAT tables */
	nat_quit();

	/* close TUN */
	close(tun_fd);

	return 0;
}

/**
 * Decide what to do with a packet and pass it for further processing.
 *
 * @param	packet	Packet data
 * @param	length	Packet data length
 *
 * @return	0 for success
 * @return	1 for failure
 */
static int process(char *packet, unsigned short length)
{
	/* sanity check: out of every combination this is the smallest one */
	if (length < sizeof(struct s_ipv4) +
	    sizeof(struct s_icmp)) {
		log_error("length to short");
		return 1;
	}

    struct iphdr *ip = (struct iphdr *)(packet);

	switch (ip->version) {
		case 4:
			return ipv4(packet, length);
		case 6:
			return ipv6(packet, length);
		default:
			log_debug("unknown IP version");
			return 1;
	}

	#undef payload_length
	#undef payload
}

/**
 * Translator of IPv6 address with embedded IPv4 address to that IPv4 address.
 *
 * @param	ipv6_addr	IPv6 address (as data source)
 * @param	ipv4_addr	Where to put final IPv4 address
 */
void ipv6_to_ipv4(struct s_ipv6_addr *ipv6_addr, struct s_ipv4_addr *ipv4_addr)
{
	memcpy(ipv4_addr, ipv6_addr->addr + 12, 4);
}

/**
 * Translator of IPv4 address to IPv6 address with WrapSix' prefix.
 *
 * @param	ipv4_addr	IPv4 address (as data source)
 * @param	ipv6_addr	Where to put final IPv6 address
 */
void ipv4_to_ipv6(struct s_ipv4_addr *ipv4_addr, struct s_ipv6_addr *ipv6_addr)
{
	memcpy(ipv6_addr, &wrapsix_ipv6_prefix, 12);
	memcpy(ipv6_addr->addr + 12, ipv4_addr, 4);
}
