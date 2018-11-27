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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/ipv6.h>
#include <linux/ip.h>
#include <linux/if_tun.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "ipv4.h"
#include "log.h"
#include "transmitter.h"
#include "wrapper.h"


static int tun_fd = -1;

static int transmit_tun(const char *data, unsigned int length);

/**
 * Initialize sockets and all needed properties. Should be called only once on
 * program startup.
 *
 * @return	0 for success
 * @return	1 for failure
 */
int transmission_init(struct s_cfg_opts	cfg)
{
    struct ifreq ifr;

    if((tun_fd = open("/dev/net/tun", O_RDWR)) < 0 ) {
      perror("Opening /dev/net/tun");
      return 0;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    if (cfg.interface) {
      strncpy(ifr.ifr_name, cfg.interface, IFNAMSIZ);
    }

    if(ioctl(tun_fd, TUNSETIFF, (void *)&ifr) < 0 ) {
      perror("ioctl(TUNSETIFF)");
      close(tun_fd);
      return 0;
    }

	return 1;
}

/**
 * Read packet from TUN device.
 *
 * @return	0 for success
 * @return	1 for failure
 */
int transmission_read(char *buffer, unsigned int length)
{
    int bytes = read(tun_fd, buffer, length);
    if (bytes < 0) {
        perror("read");
        return -1;
    }
    return bytes;
}

/**
 * Close TUN device. Should be called only once on program shutdown.
 *
 * @return	0 for success
 * @return	1 for failure
 */
int transmission_quit(void)
{
    close(tun_fd);
}

/**
 * Send IPv6 packet with IPv6 header supplied.
 *
 * @param	data	Raw packet data, including IPv6 header
 * @param	length	Length of the whole packet in bytes
 *
 * @return	0 for success
 * @return	1 for failure
 */
int transmit_ipv6(const char *data, unsigned int length)
{
    struct ipv6hdr *ip = (struct ipv6hdr *)(data);
    struct s_ipv6 *ip2 = (struct s_ipv6*)(data);

    IP6_TXT(from, &ip->saddr);
    IP6_TXT(to, &ip->daddr);
    log_debug("IPv6 send: %s > %s", from, to);

	return transmit_tun(data, length);
}

/**
 * Send IPv4 packet with IPv4 header supplied.
 *
 * @param	data	Raw packet data including IPv4 header
 * @param	length	Length of the whole packet in bytes
 *
 * @return	0 for success
 * @return	1 for failure
 */
int transmit_ipv4(const char *data, unsigned int length)
{
    struct s_ipv4 *hdr = (struct s_ipv4*)data;

    IP4_TXT(src, &hdr->ip_src);
    IP4_TXT(dst, &hdr->ip_dest);
	log_debug("IPv4 send: %s > %s", src, dst);

	return transmit_tun(data, length);
}

/**
 * Send IP packet payload to tun device
 *
 * @param	data	Raw IP packet
 * @param	length	Total length in bytes
 *
 * @return	0 for success
 * @return	1 for failure
 */
int transmit_tun(const char *data, unsigned int length)
{
    int ret = write(tun_fd, data, length);
    if (ret < 0)
    {
	    log_error("could not send TUN packet");
        return 1;
    }
    return 0;
}
