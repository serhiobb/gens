/***************************************************************************
 * Gens: Debug Messages.                                                   *
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

#ifndef GENS_DEBUG_MSG_H
#define GENS_DEBUG_MSG_H

#include <stdio.h>

#define DEBUG_CHANNEL_gens	1
#define DEBUG_CHANNEL_input	1

#define DEBUG_CHANNEL_mdp	1
#define DEBUG_CHANNEL_ym2612	0
#define DEBUG_CHANNEL_psg	0
#define DEBUG_CHANNEL_pcm	0

/**
 * DEBUG_MSG(): Output a debug message.
 * @param channel Debug channel. (string)
 * @param level Debug level. (integer)
 * @param msg Message.
 * @param ... Parameters.
 */
#define DEBUG_MSG(channel, level, msg, ...)	\
{						\
	if (DEBUG_CHANNEL_ ##channel >= level)	\
		fprintf(stderr, "%s:%d:%s(): " msg "\n", #channel, level, __func__, ##__VA_ARGS__);	\
}

#endif /* GENS_DEBUG_MSG_H */
