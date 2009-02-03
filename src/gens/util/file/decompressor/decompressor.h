/***************************************************************************
 * Gens: File Decompression Function Definitions.                          *
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

#ifndef GENS_DECOMPRESSOR_H
#define GENS_DECOMPRESSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/**
 * file_list_t: Linked list of files from a decompressor.
 */
typedef struct _file_list_t
{
	int filesize;		// Filesize.
	char *filename;		// Created using strdup().
	
	struct _file_list_t *next;	// Next file.
} file_list_t;

/**
 * file_list_t_free(): Free a list of files.
 * @param file_list Pointer to the first file in the list.
 */
void file_list_t_free(file_list_t *file_list);

/**
 * decompressor_detect_format(): Detect if this file can be handled by this decompressor.
 * @param zF Open file handle.
 * @return Non-zero if this file can be handled; 0 if it can't be.
 */
typedef int (*decompressor_detect_format)(FILE *zF);

/**
 * decompressor_get_file_info(): Get information from all files in the archive.
 * @param zF Open file handle.
 * @param filename Filename of the archive.
 * @return Pointer to the first file in the list, or NULL on error.
 */
typedef file_list_t* (*decompressor_get_file_info)(FILE *zF, const char *filename);

/**
 * decompressor_get_file(): Get a file from the archive.
 * @param zF Open file handle.
 * @param filename Filename of the archive.
 * @param file_list Pointer to decompressor_file_list_t element to get from the archive.
 * @param buf Buffer to read the file into.
 * @param size Size of buf (in bytes).
 * @return Number of bytes read, or -1 on error.
 */
typedef int (*decompressor_get_file)(FILE *zF, const char *filename,
				     file_list_t *file_list,
				     unsigned char *buf, const int size);

/**
 * decompressor_t: Struct containing function pointers to various decompresssors.
 */
typedef struct _decompressor_t
{
	decompressor_detect_format	detect_format;
	decompressor_get_file_info	get_file_info;
	decompressor_get_file		get_file;
} decompressor_t;

#ifdef __cplusplus
}
#endif

#endif /* GENS_DECOMPRESSOR_H */
