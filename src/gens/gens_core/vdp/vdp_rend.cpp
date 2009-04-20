/***************************************************************************
 * Gens: VDP rendering functions.                                          *
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

#include "vdp_rend.h"
#include "vdp_io.h"

// bppMD
#include "emulator/g_main.hpp"

// C includes.
#include <stdint.h>


/**
 * T_VDP_Update_Palette(): VDP palette update function.
 * @param hs If true, updates highlight/shadow.
 * @param mask Color mask.
 * @param MD_palette MD color palette.
 * @param palette Full color palette.
 */
template<bool hs, typename pixel>
static inline void T_VDP_Update_Palette(pixel mask, pixel *MD_palette, const pixel *palette)
{
	if (VDP_Layers & VDP_LAYER_PALETTE_LOCK)
		return;
	
	// Disable the CRAM flag, since the palette is being updated.
	CRam_Flag = 0;
	
	// 16-bit CRAM pointer.
	const uint16_t *cram_16 = (uint16_t*)CRam;
	
	// Color mask. Depends on VDP register 0, bit 2 (Palette Select).
	// If set, allows full MD palette.
	// If clear, only allows the LSB of each color component.
	const uint16_t color_mask = (VDP_Reg.Set1 & 0x04) ? 0x0FFF : 0x0222;
	
	// Update all 64 colors.
	for (int i = 62; i >= 0; i -= 2)
	{
		pixel color1 = cram_16[i];
		pixel color2 = cram_16[i + 1];
		
		// Mask the color using the color_mask.
		color1 &= color_mask;
		color2 &= color_mask;
		
		// Get the palette color.
		color1 = palette[color1];
		color2 = palette[color2];
		
		// Set the new color.
		MD_palette[i]     = color1;
		MD_palette[i + 1] = color2;
		
		if (hs)
		{
			// Update the highlight and shadow colors.
			
			// Normal color.
			MD_palette[i + 192]	= color1;
			MD_palette[i + 1 + 192]	= color2;
			
			// Shadow color.
			color1 = (color1 >> 1) & mask;
			color2 = (color2 >> 1) & mask;
			MD_palette[i + 64]	= color1;
			MD_palette[i + 1 + 64]	= color2;
			
			// Highlight color.
			color1 += mask;
			color2 += mask;
			MD_palette[i + 128]	= color1;
			MD_palette[i + 1 + 128]	= color2;
		}
	}
	
	// Update the background color.
	pixel color_bg = MD_palette[VDP_Reg.BG_Color & 0x3F];
	MD_palette[0] = color_bg;
	
	if (hs)
	{
		// Update the background color for highlight and shadow.
		
		// Normal color.
		MD_palette[192] = color_bg;
		
		// Shadow color.
		color_bg = (color_bg >> 1) & mask;
		MD_palette[64] = color_bg;
		
		// Highlight color.
		color_bg += mask;
		MD_palette[128] = color_bg;
	}
}


/**
 * VDP_Update_Palette(): Update the palette.
 */
void VDP_Update_Palette(void)
{
	const uint16_t mask16 = (bppMD == 15 ? 0x3DEF : 0x7BEF);
	T_VDP_Update_Palette<false>(mask16, MD_Palette, Palette);
	T_VDP_Update_Palette<false>((uint32_t)0x7F7F7F, MD_Palette32, Palette32);
}

/**
 * VDP_Update_Palette_HS(): Update the palette, including highlight and shadow.
 */
void VDP_Update_Palette_HS(void)
{
	const uint16_t mask16 = (bppMD == 15 ? 0x3DEF : 0x7BEF);
	T_VDP_Update_Palette<true>(mask16, MD_Palette, Palette);
	T_VDP_Update_Palette<true>((uint32_t)0x7F7F7F, MD_Palette32, Palette32);
}
