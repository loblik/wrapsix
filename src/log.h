/*
 *  WrapSix
 *  Copyright (C) 2008-2012  xHire <xhire@wrapsix.org>
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

#ifndef LOG_H
#define LOG_H


enum log_level { LOG_ERROR = 1, LOG_WARN = 2, LOG_INFO = 3, LOG_DEBUG = 4 };

void log_set_level(enum log_level level);

void log_debug(const char *msg, ...);
void log_info(const char *msg, ...);
void log_warn(const char *msg, ...);
void log_error(const char *msg, ...);

#endif /* LOG_H */
