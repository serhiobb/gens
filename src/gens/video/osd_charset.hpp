/***************************************************************************
 * Gens: On-Screen Display Character Set.                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#ifndef GENS_OSD_CHARSET_HPP
#define GENS_OSD_CHARSET_HPP

#include <stdint.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************
 * Format: 16 bytes per character, 8 bits per line. *
 * 1 == opaque dot; 0 == transparent dot            *
 * MSB == left-most dot; LSB == right-most dot      *
 ****************************************************/

/** Typedef for 8x16 character set page. */
typedef uint8_t osd_charset_page_t[256][16];

int osd_charset_prerender(const char *str, uint8_t prerender_buf[16][1024]);

#ifdef __cplusplus
}
#endif

#endif /* GENS_OSD_CHARSET_HPP */
