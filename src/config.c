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

#define _POSIX_C_SOURCE 201112L

#include <arpa/inet.h>	/* inet_pton */
#include <ifaddrs.h>	/* struct ifaddrs, getifaddrs, freeifaddrs */
#include <netdb.h>	/* getnameinfo, NI_NUMERICHOST */
#include <stdio.h>	/* FILE, fopen, getc, feof, fclose, perror */
#include <stdlib.h>	/* exit */
#include <string.h>	/* strcmp, strncpy */

#include "config.h"
#include "ipv4.h"
#include "ipv6.h"
#include "log.h"
#include "wrapper.h"

#define SYNTAX_ERROR(file, line, pos, fatal) { \
			log_error("Syntax error in configuration file %s on " \
				  "line %d, position %d", file, line, pos); \
			fclose(f); \
			if (fatal) { \
				exit(1); \
            } else { \
				return 1; \
			} \
		}

#define	C_DEFAULT_MTU		1280
#define	C_DEFAULT_PREFIX	"64:ff9b::"

void cfg_guess_interface(char *cinterface);

/**
 * Configuration file parser.
 *
 * @param	config_file	Configuration file (with path) to parse
 * @param	cmtu		Pointer to variable where to save MTU
 * @param	oto		One-time configuration options (used only
 * 				locally to init other things)
 * @param	init		0 or 1, whether or not to initialize
 * 				configuration with defaults
 *
 * @return      0 for success
 * @return      1 for failure in case of some syntax error when reloading
 * 		configuration
 * @return	exit with code 1 in case of some syntax error when initializing
 * 		configuration
 */
int cfg_parse(const char *config_file, unsigned short *cmtu,
	      struct s_cfg_opts *oto, unsigned char init)
{
	FILE *f;
	unsigned int ln = 0;
	char c;

	unsigned short opt_len, wht_len, val_len;
	char tmp_opt[32], tmp_val[256];

	/* set defaults */
	*cmtu = C_DEFAULT_MTU;
	oto->interface[0] = '\0';
	cfg_guess_interface(oto->interface);
	strncpy(oto->prefix, C_DEFAULT_PREFIX, sizeof(oto->prefix));
	oto->ipv4_address[0] = '\0';
    oto->level = LOG_WARN;

	f = fopen(config_file, "r");

	if (f == NULL) {
		log_warn("Configuration file %s doesn't exist, using defaults",
			 config_file);

		if (init) {
			log_error("IPv4 address unconfigured! Exiting");
			exit(1);
		}

		return 0;
	}

	while (c = getc(f), !feof(f)) {
		ln++;

		/* comments */
		if (c == '#') {
			/* skip this line */
			while (c = getc(f), c != '\n' && !feof(f));

			continue;
		}

		/* empty lines */
		if (c == '\n') {
			continue;
		}

		/* option is only of small letters */
		if (c < 'a' || c > 'z') {
			SYNTAX_ERROR(config_file, ln, 0, init);
		}

		/* read option */
		for (opt_len = 0; c != ' ' && c != '\t'; opt_len++,
		    c = getc(f)) {
			if (feof(f) || !((c >= 'a' && c <= 'z') || c == '_' ||
			    (c >= '0' && c <= '9'))) {
				SYNTAX_ERROR(config_file, ln, opt_len, init);
			}

			if (opt_len < 32 - 1) {
				tmp_opt[opt_len] = c;
			} else {
				SYNTAX_ERROR(config_file, ln, opt_len, init);
			}
		}
		tmp_opt[opt_len] = '\0';

		/* skip white space */
		for (wht_len = 0; c == ' ' || c == '\t'; wht_len++,
		    c = getc(f)) {
			if (feof(f)) {
				SYNTAX_ERROR(config_file, ln, opt_len + wht_len,
					     init);
			}
		}

		/* read value */
		for (val_len = 0; c != ' ' && c != '\t' && c != '\n'; val_len++,
		    c = getc(f)) {
			if (feof(f)) {
				SYNTAX_ERROR(config_file, ln, opt_len, init);
			}

			if (val_len < 128 - 1) {
				tmp_val[val_len] = c;
			} else {
				SYNTAX_ERROR(config_file, ln, opt_len, init);
			}
		}
		tmp_val[val_len] = '\0';

		/* skip rest of this line */
		if (c != '\n') {
			while (c = getc(f), c != '\n' && !feof(f));
		}

		/* recognize the option */
		if (!strcmp(tmp_opt, "mtu")) {
			*cmtu = atoi(tmp_val);
			if (*cmtu > MAX_MTU) {
				log_warn("MTU setting is over maximum (%d), "
					 "falling to default value (%d)",
					 MAX_MTU, C_DEFAULT_MTU);
				*cmtu = C_DEFAULT_MTU;
			}
		} else if (!strcmp(tmp_opt, "interface")) {
            if (val_len >= IFNAMSIZ)
            {
			    log_error("interface name too long");
				SYNTAX_ERROR(config_file, ln, opt_len + wht_len, init);
            }
			strncpy(oto->interface, tmp_val,
				sizeof(oto->interface));
		} else if (!strcmp(tmp_opt, "prefix")) {
            if (val_len >= INET6_ADDRSTRLEN)
            {
			    log_error("prefix option too long");
				SYNTAX_ERROR(config_file, ln, opt_len + wht_len, init);
            }
			strncpy(oto->prefix, tmp_val, sizeof(oto->prefix));
		} else if (!strcmp(tmp_opt, "ipv4_address")) {
			if (val_len < INET_ADDRSTRLEN) {
				strncpy(oto->ipv4_address, tmp_val,
					sizeof(oto->ipv4_address));
			} else {
			    log_error("ipv4_address option too long");
				SYNTAX_ERROR(config_file, ln, opt_len + wht_len,
					     init);
			}
		} else if (!strcmp(tmp_opt, "ipv6_address")) {
			if (val_len < INET6_ADDRSTRLEN) {
				strncpy(oto->ipv6_address, tmp_val,
					sizeof(oto->ipv6_address));
			} else {
			    log_error("ipv6_address option too long");
				SYNTAX_ERROR(config_file, ln, opt_len + wht_len,
					     init);
			}
        } else if (!strcmp(tmp_opt, "log_level")) {
            oto->level = 0;
            for (int i = 0; i < 4; i++) {
                char *levels[] = { "error", "warn", "info", "debug" };
                if (strcmp(tmp_val, levels[i]) == 0) {
                    oto->level = i + 1;
                }
            }
            if (oto->level == 0) {
			    log_error("unknown log level value");
			    SYNTAX_ERROR(config_file, ln, 0, init);
            }
		} else {
			log_error("Unknown configuration option");
			SYNTAX_ERROR(config_file, ln, 0, init);
		}
	}

	fclose(f);

	if (init && oto->ipv4_address[0] == '\0') {
		log_error("IPv4 address unconfigured! Exiting");
		exit(1);
	}

	return 0;
}

/**
 * Gets name of first non-loopback network interface.
 *
 * @param	cinterface	Where to save the interface name
 */
void cfg_guess_interface(char *cinterface)
{
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		return;
	}

	/* Walk through linked list, maintaining head pointer so we can free
	 * list later */

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL) {
			continue;
		}

		/* skip loopback */
		if (strcmp(ifa->ifa_name, "lo")) {
			strncpy(cinterface, ifa->ifa_name,
				sizeof(((struct s_cfg_opts *) NULL)->interface)
			);
			break;
		}
	}

	freeifaddrs(ifaddr);
}
