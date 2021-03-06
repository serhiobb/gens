/***************************************************************************
 * libgsft: Common functions.                                              *
 * gsft_iconv.c: iconv() wrapper.                                          *
 *                                                                         *
 * Copyright (c) 2009-2010 by David Korth                                  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef GSFT_ICONV_H
#define GSFT_ICONV_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

char *gsft_iconv(const char *src, size_t src_bytes_len,
		 const char *src_charset, const char *dest_charset);

#ifdef __cplusplus
}
#endif

#endif /* GENS_CHARSET_HPP */
