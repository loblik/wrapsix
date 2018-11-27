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

#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include "config.h"

int transmission_read(char *buffer, unsigned int length);
int transmission_init(struct s_cfg_opts	cfg);
int transmission_quit(void);
int transmit_ipv6(const char *data, unsigned int length);
int transmit_ipv4(const char *data, unsigned int length);

#endif /* TRANSMITTER_H */
