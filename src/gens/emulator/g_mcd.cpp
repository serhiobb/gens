/**
 * Gens: Sega CD (Mega CD) initialization and main loop code.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// C includes.
#include <stdint.h>
#include <string.h>

// C++ includes.
#include <string>
using std::string;

#include "gens.hpp"
#include "g_md.hpp"
#include "g_mcd.hpp"
#include "g_main.hpp"
#include "g_update.hpp"
#include "options.hpp"

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"
#include "gens_core/mem/mem_s68k.h"

#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/z80/cpu_z80.h"
#include "mdZ80/mdZ80.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/io/io.h"
#include "segacd/cd_sys.hpp"
#include "segacd/lc89510.h"
#include "gens_core/gfx/gfx_cd.h"
#include "gens_core/sound/pcm.h"
#include "segacd/cd_sys.hpp"
#include "segacd/cd_file.h"

// Save file handlers.
#include "util/file/save.hpp"
#include "util/file/mcd_bram.h"

// VDP rendering functions.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/TAB336.h"

#include "util/sound/wave.h"
#include "util/sound/gym.hpp"

#include "libgsft/gsft_byteswap.h"
#include "macros/force_inline.h"

#include "ui/gens_ui.hpp"

// Audio Handler.
#include "audio/audio.h"

// MDP Event Manager.
#include "plugins/eventmgr.hpp"


// SegaCD disc header.
// Used for hard reset to know the game name.
static unsigned char SegaCD_Header[512];


/**
 * Detect_Country_SegaCD(): Detect the country code of a SegaCD game.
 * @return BIOS filename for the required country.
 */
const char* Detect_Country_SegaCD(void)
{
	if (SegaCD_Header[0x10B] == 0x64)
	{
		Game_Mode = 1;
		CPU_Mode = 1;
		return BIOS_Filenames.MegaCD_EU;
	}
	else if (SegaCD_Header[0x10B] == 0xA1)
	{
		Game_Mode = 0;
		CPU_Mode = 0;
		return BIOS_Filenames.MegaCD_JP;
	}
	else
	{
		Game_Mode = 1;
		CPU_Mode = 0;
		return BIOS_Filenames.SegaCD_US;
	}
}


/**
 * Init_SegaCD(): Initialize the Sega CD with the specified ISO image.
 * @param iso_name Filename of the ISO, or NULL for physical CD-ROM.
 * @return 1 if successful; 0 if an error occurred.
 */
int Init_SegaCD(const char* iso_name)
{
	const char* BIOS_To_Use;
	
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	GensUI::setWindowTitle_Init("SegaCD", false);
	
	// Clear the SCD struct.
	memset(&SCD, 0x00, sizeof(SCD));
	
	if (Reset_CD((char*)SegaCD_Header, iso_name))
	{
		// Error occured while setting up Sega CD emulation.
		// TODO: Show a message box.
		GensUI::setWindowTitle_Idle();
		return 0;
	}
	
	// Check what BIOS should be used.
	// TODO: Get rid of magic numbers.
	switch (Country)
	{
		default:
		case -1: // Autodetection.
			BIOS_To_Use = Detect_Country_SegaCD();
			break;
		
		case 0: // Japan (NTSC)
			Game_Mode = 0;
			CPU_Mode = 0;
			BIOS_To_Use = BIOS_Filenames.MegaCD_JP;
			break;
		
		case 1: // US (NTSC)
			Game_Mode = 1;
			CPU_Mode = 0;
			BIOS_To_Use = BIOS_Filenames.SegaCD_US;
			break;
		
		case 2: // Europe (PAL)
			Game_Mode = 1;
			CPU_Mode = 1;
			BIOS_To_Use = BIOS_Filenames.MegaCD_EU;
			break;
		
		case 3: // Japan (PAL)
			Game_Mode = 0;
			CPU_Mode = 1;
			BIOS_To_Use = BIOS_Filenames.MegaCD_JP;
			break;
	}
	
	// Attempt to load the Sega CD BIOS.
	if (ROM::loadSegaCD_BIOS(BIOS_To_Use) == NULL)
	{
		GensUI::msgBox(
			"Your Sega CD BIOS files aren't configured correctly.\n"
			"Go to menu 'Options -> BIOS/Misc Files' to set them up.",
			"BIOS Configuration Error"
			);
		GensUI::setWindowTitle_Idle();
		return 0;
	}
	
	// Update the CD-ROM name.
	ROM::updateCDROMName(SegaCD_Header, Game_Mode);
	
	// Update the window title with the game name.
	Options::setGameName(1);
	
	Flag_Clr_Scr = 1;
	Settings.Paused = Frame_Number = 0;
	SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
	BRAM_Ex_State &= 0x100;
	Controller_1_COM = Controller_2_COM = 0;
#ifdef GENS_DEBUGGING
	STOP_DEBUGGING();
#endif
	
	// Set clock rates depending on the CPU mode (NTSC / PAL).
	Set_Clock_Freq(1);
	
	// Initialize VDP_Lines.Display.
	VDP_Set_Visible_Lines();
	
	// TODO: Why are these two bytes set to 0xFF?
	Rom_Data.u8[0x72] = 0xFF;
	Rom_Data.u8[0x73] = 0xFF;
	be16_to_cpu_array(Rom_Data.u8, Rom_Size);
	ROM_ByteSwap_State |= ROM_BYTESWAPPED_MD_ROM;
	
	// Reset all CPUs and other components.
	M68K_Reset(2);
	S68K_Reset();
	Z80_Reset();
	VDP_Reset();
	LC89510_Reset();
	Init_RS_GFX();
	
	PCM_Init(audio_get_sound_rate());
	
	// Initialize the controller state.
	Init_Controllers();
	
	// Initialize sound.
	if (audio_get_enabled())
	{
		audio_end();
		
		if (audio_init(AUDIO_BACKEND_DEFAULT))
			audio_set_enabled(false);
		else
		{
			if (audio_play_sound)
				audio_play_sound();
		}
	}
	
	// Initialize BRAM.
	BRAM_Load();
	
	Reset_Update_Timers();
	
	if (SegaCD_Accurate)
	{
		Update_Frame = Do_SegaCD_Frame_Cycle_Accurate;
		Update_Frame_Fast = Do_SegaCD_Frame_No_VDP_Cycle_Accurate;
	}
	else
	{
		Update_Frame = Do_SegaCD_Frame;
		Update_Frame_Fast = Do_SegaCD_Frame_No_VDP;
	}
	
	return 1;
}


/**
 * Reload_SegaCD(): Reloads the Sega CD.
 * @param iso_name Filename of the ISO, or NULL for physical CD-ROM.
 * @return 1 if successful.
 */
int Reload_SegaCD(const char* iso_name)
{
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	// Save the current BRAM.
	BRAM_Save();
	
	// Set the window title to "Reinitializing..."
	GensUI::setWindowTitle_Init(((CPU_Mode == 0 && Game_Mode == 1) ? "SegaCD" : "MegaCD"), true);
	
	// Update the CD-ROM name.
	Reset_CD((char*)SegaCD_Header, iso_name);
	ROM::updateCDROMName(SegaCD_Header, Game_Mode);
	
	// Update the window title with the game name.
	Options::setGameName(1);
	
	// Load the new BRAM.
	BRAM_Load();
	
	return 1;
}


/**
 * Reset_SegaCD(): Resets the Sega CD.
 */
void Reset_SegaCD(void)
{
	char *BIOS_To_Use;
	
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	if (Game_Mode == 0)
		BIOS_To_Use = BIOS_Filenames.MegaCD_JP;
	else if (CPU_Mode)
		BIOS_To_Use = BIOS_Filenames.MegaCD_EU;
	else
		BIOS_To_Use = BIOS_Filenames.SegaCD_US;
	
	// Load the BIOS file.
	string z_filename;
	ROM_ByteSwap_State &= ~ROM_BYTESWAPPED_MD_ROM;
	if (ROM::loadROM(BIOS_To_Use, z_filename, Game) <= 0)
	{
		GensUI::msgBox(
			"Your Sega CD BIOS files aren't configured correctly.\n"
			"Go to menu 'Options -> BIOS/Misc Files' to set them up.",
			"BIOS Configuration Error"
			);
		return;
	}
	
	Controller_1_COM = Controller_2_COM = 0;
	SRAM_ON = 0;
	SRAM_Write = 0;
	Settings.Paused = 0;
	BRAM_Ex_State &= 0x100;
	
	// Update the CD-ROM name.
	ROM::updateCDROMName(SegaCD_Header, Game_Mode);
	
	// Update the window title with the game name.
	Options::setGameName(1);
	
	// TODO: Why are these two bytes set to 0xFF?
	Rom_Data.u8[0x72] = 0xFF;
	Rom_Data.u8[0x73] = 0xFF;
	be16_to_cpu_array(Rom_Data.u8, Rom_Size);
	ROM_ByteSwap_State |= ROM_BYTESWAPPED_MD_ROM;
	
	// Reset all CPUs and other components.
	M68K_Reset(2);
	S68K_Reset();
	Z80_Reset();
	LC89510_Reset();
	VDP_Reset();
	Init_RS_GFX();
	PCM_Reset();
	YM2612_Reset();
	
	// Initialize the controller state.
	Init_Controllers();
	
	if (CPU_Mode)
		VDP_Status |= 1;
	else
		VDP_Status &= ~1;
}


/**
 * draw_SegaCD_LED(): Draw a SegaCD LED.
 * @param screen Screen buffer.
 * @param color LED color.
 * @param startCol Left-most column of the LED.
 */
template<typename pixel>
static FORCE_INLINE void T_Draw_SegaCD_LED(pixel* screen, pixel color, unsigned short start_column)
{
	// Get the line offsets.
	const unsigned int bottom_line = (VDP_Lines.Visible.Total + VDP_Lines.Visible.Border_Size);
	pixel *line1 = &screen[(TAB336[bottom_line - 4] + 8) + 4 + start_column];
	pixel *line2 = &screen[(TAB336[bottom_line - 2] + 8) + 4 + start_column];
	
	*line1		= color;
	*(line1 + 1)	= color;
	*(line1 + 2)	= color;
	*(line1 + 3)	= color;
	
	*line2		= color;
	*(line2 + 1)	= color;
	*(line2 + 2)	= color;
	*(line2 + 3)	= color;
}


/**
 * SegaCD_Display_LED(): Display the LEDs on the Sega CD interface.
 */
static void SegaCD_Display_LED(void)
{
	// TODO: Optimize this function.
	// TODO: Draw the LEDs on the screen buffer, not the MD screen.
	
	// Ready LED (green)
	static const uint16_t ledReady_15 = 0x03E0;
	static const uint16_t ledReady_16 = 0x07C0;
	static const uint32_t ledReady_32 = 0x00F800;
	
	// Access LED (red)
	static const uint16_t ledAccess_15 = 0x7C00;
	static const uint16_t ledAccess_16 = 0xF800;
	static const uint32_t ledAccess_32 = 0xF80000;
	
	// Ready LED
	if (LED_Status & 2)
	{
		if (bppMD == 15)
			T_Draw_SegaCD_LED(MD_Screen.u16, ledReady_15, 0);
		else if (bppMD == 16)
			T_Draw_SegaCD_LED(MD_Screen.u16, ledReady_16, 0);
		else //if (bppMD == 32)
			T_Draw_SegaCD_LED(MD_Screen.u32, ledReady_32, 0);
	}
	
	// Access LED
	if (LED_Status & 1)
	{
		if (bppMD == 15)
			T_Draw_SegaCD_LED(MD_Screen.u16, ledAccess_15, 8);
		else if (bppMD == 16)
			T_Draw_SegaCD_LED(MD_Screen.u16, ledAccess_16, 8);
		else //if (bppMD == 32)
			T_Draw_SegaCD_LED(MD_Screen.u32, ledAccess_32, 8);
	}
}


/**
 * T_gens_do_MCD_line(): Do an MCD line.
 * @param LineType Line type.
 * @param VDP If true, VDP is updated.
 * @param perfect_sync If true, use perfect synchronization.
 */
template<LineType_t LineType, bool VDP, bool perfect_sync>
static FORCE_INLINE void T_gens_do_MCD_line(void)
{
	int *buf[2], i, j;
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Lines.Display.Current][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Lines.Display.Current][0];
	if (PCM_Enable)
		PCM_Update(buf, Sound_Extrapol[VDP_Lines.Display.Current][1]);
	YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Lines.Display.Current][1]);
	YM_Len += Sound_Extrapol[VDP_Lines.Display.Current][1];
	PSG_Len += Sound_Extrapol[VDP_Lines.Display.Current][1];
	Update_CDC_TRansfert();
	
	if (perfect_sync)
	{
		i = Cycles_M68K + 24;
		j = Cycles_S68K + 39;
	}
	
	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (S68K_State == 1)
		Cycles_S68K += CPL_S68K;
	if (VDP_Reg.DMAT_Length)
		main68k_addCycles(VDP_Update_DMA());
	
	// TODO: Consolidate this code.
	switch (LineType)
	{
		case LINETYPE_ACTIVEDISPLAY:
			// In visible area.
			VDP_Status |= 0x0004;	// HBlank = 1
			
			if (!perfect_sync)
			{
				// Perfect Sync is disabled.
				main68k_exec(Cycles_M68K - 404);
			}
			else
			{
				// Perfect sync is enabled.
				// Use instruction by instruction execution.
				while (i < (Cycles_M68K - 404))
				{
					main68k_exec(i);
					i += 24;
					
					if (j < (Cycles_S68K - 658))
					{
						sub68k_exec(j);
						j += 39;
					}
				}
				
				main68k_exec(Cycles_M68K - 404);
				sub68k_exec(Cycles_S68K - 658);
			}
			
			VDP_Status &= ~0x0004;	// HBlank = 0
			
			if (--VDP_Reg.HInt_Counter < 0)
			{
				VDP_Reg.HInt_Counter = VDP_Reg.m5.H_Int;
				VDP_Int |= 0x4;
				VDP_Update_IRQ_Line();
			}
			
			if (VDP)
			{
				// VDP needs to be updated.
				VDP_Render_Line();
			}
			
			if (perfect_sync)
			{
				// Perfect sync is enabled.
				// Use instruction by instruction execution.
				while (i < Cycles_M68K)
				{
					main68k_exec(i);
					i += 24;
					
					if (j < Cycles_S68K)
					{
						sub68k_exec(j);
						j += 39;
					}
				}
			}
			
			break;
		
		case LINETYPE_VBLANKLINE:
			// VBlank line!
			if (--VDP_Reg.HInt_Counter < 0)
			{
				VDP_Int |= 0x4;
				VDP_Update_IRQ_Line();
			}
			
			VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
			
			if (perfect_sync)
			{
				// Perfect sync is enabled.
				// Use instruction by instruction execution.
				while (i < (Cycles_M68K - 360))
				{
					main68k_exec(i);
					i += 24;
					
					if (j < (Cycles_S68K - 586))
					{
						sub68k_exec(j);
						j += 39;
					}
				}
			}
			
			main68k_exec(Cycles_M68K - 360);
			sub68k_exec(Cycles_S68K - 586);
			Z80_EXEC(168);
			
			VDP_Status &= ~0x0004;		// HBlank = 0
			VDP_Status |=  0x0080;		// V Int happened
			VDP_Int |= 0x8;
			VDP_Update_IRQ_Line();
			mdZ80_interrupt(&M_Z80, 0xFF);
			
			if (VDP)
			{
				// VDP needs to be updated.
				VDP_Render_Line();
			}
			
			if (perfect_sync)
			{
				// Perfect sync is enabled.
				// Use instruction by instruction execution.
				while (i < Cycles_M68K)
				{
					main68k_exec(i);
					i += 24;
					
					if (j < Cycles_S68K)
					{
						sub68k_exec(j);
						j += 39;
					}
				}
			}
			
			break;
		
		case LINETYPE_BORDER:
		default:
			// Not visible area.
			// TODO: We're processing HBlank here, but we don't for MD...
			VDP_Status |= 0x0004;	// HBlank = 1
			
			if (VDP)
			{
				// VDP needs to be updated.
				VDP_Render_Line();
			}
			
			if (!perfect_sync)
			{
				// Perfect sync is disabled.
				main68k_exec(Cycles_M68K - 404);
				VDP_Status &= ~0x0004;	// HBlank = 0
			}
			else
			{
				// Perfect sync is enabled.
				// Use instruction by instruction execution.
				while (i < (Cycles_M68K - 404))
				{
					main68k_exec(i);
					i += 24;
					
					if (j < (Cycles_S68K - 658))
					{
						sub68k_exec(j);
						j += 39;
					}
				}
				
				main68k_exec(Cycles_M68K - 404);
				sub68k_exec(Cycles_S68K - 658);
				
				VDP_Status &= ~0x0004;	// HBlank = 0
				
				while (i < Cycles_M68K)
				{
					main68k_exec(i);
					i += 24;
					
					if (j < Cycles_S68K)
					{
						sub68k_exec(j);
						j += 39;
					}
				}
			}
			
			break;
	}
	
	main68k_exec(Cycles_M68K);
	sub68k_exec(Cycles_S68K);
	Z80_EXEC(0);
	
	Update_SegaCD_Timer();
}


/**
 * T_gens_do_MCD_frame(): Do an MCD frame.
 * @param VDP If true, VDP is updated.
 * @param perfect_sync If true, use perfect synchronization.
 */
template<bool VDP, bool perfect_sync>
static FORCE_INLINE int T_gens_do_MCD_frame(void)
{
	// Initialize VDP_Lines.Display.
	VDP_Set_Visible_Lines();

	// Check if VBlank is allowed.
	VDP_Check_NTSC_V30_VBlank();
	
	// S68K is always 12.5 MHz regardless of region.
	// TODO: PAL is 312*50 == 15,600 Hz;
	// NTSC is 262*60 = 15,720 Hz.
	CPL_S68K = 795;
	
	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;
	
	Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer();
	sub68k_tripOdometer();
	mdZ80_clear_odo(&M_Z80);
	
	// Raise the MDP_EVENT_PRE_FRAME event.
	EventMgr::RaiseEvent(MDP_EVENT_PRE_FRAME, NULL);
	
	// Set the VRam flag to force a VRam update.
	VDP_Flags.VRam = 1;
	
	// Interlaced frame status.
	// Both Interlaced Modes 1 and 2 set this bit on odd frames.
	// This bit is cleared on even frames and if not running in interlaced mode.
	if (VDP_Reg.m5.Set4 & 0x2)
		VDP_Status ^= 0x0010;
	else
		VDP_Status &= ~0x0010;
	
	/** Main execution loops. **/
	
	/** Loop 0: Top border. **/
	for (VDP_Lines.Display.Current = 0;
	     VDP_Lines.Visible.Current < 0;
	     VDP_Lines.Display.Current++, VDP_Lines.Visible.Current++)
	{
		T_gens_do_MCD_line<LINETYPE_BORDER, VDP, perfect_sync>();
	}
	
	/** Visible line 0. **/
	VDP_Reg.HInt_Counter = VDP_Reg.m5.H_Int;	// Initialize HInt_Counter.
	VDP_Status &= ~0x0008;				// Clear VBlank status.
	
	/** Loop 1: Active display. **/
	for (;
	     VDP_Lines.Visible.Current < VDP_Lines.Visible.Total;
	     VDP_Lines.Display.Current++, VDP_Lines.Visible.Current++)
	{
		T_gens_do_MCD_line<LINETYPE_ACTIVEDISPLAY, VDP, perfect_sync>();
	}
	
	/** Loop 2: VBlank line. **/
	T_gens_do_MCD_line<LINETYPE_VBLANKLINE, VDP, perfect_sync>();
	
	/** Loop 3: Bottom border. **/
	for (VDP_Lines.Display.Current++, VDP_Lines.Visible.Current++;
	     VDP_Lines.Display.Current < VDP_Lines.Display.Total;
	     VDP_Lines.Display.Current++, VDP_Lines.Visible.Current++)
	{
		T_gens_do_MCD_line<LINETYPE_BORDER, VDP, perfect_sync>();
	}
	
	// Update the PSG and YM2612 output.
	PSG_Special_Update();
	YM2612_Special_Update();
	
	// Update CD audio.
	int *buf[2];
	buf[0] = Seg_L;
	buf[1] = Seg_R;
	Update_CD_Audio(buf, audio_seg_length);
	
	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (WAV_Dumping)
		wav_dump_update();
	if (GYM_Dumping)
		gym_dump_update(0, 0, 0);
	
	if (VDP && Show_LED)
		SegaCD_Display_LED();
	
	// Raise the MDP_EVENT_POST_FRAME event.
	mdp_event_post_frame_t post_frame;
	post_frame.width = vdp_getHPix();
	post_frame.height = VDP_Lines.Visible.Total;
	post_frame.pitch = 336;
	post_frame.bpp = bppMD;
	
	int screen_offset = (TAB336[VDP_Lines.Visible.Border_Size] + 8);
	if (post_frame.width < 320)
		screen_offset += ((320 - post_frame.width) / 2);
	
	if (bppMD == 32)
		post_frame.md_screen = &MD_Screen.u32[screen_offset];
	else
		post_frame.md_screen = &MD_Screen.u16[screen_offset];
	
	EventMgr::RaiseEvent(MDP_EVENT_POST_FRAME, &post_frame);
	
	return 1;
}


int Do_SegaCD_Frame_No_VDP(void)
{
	return T_gens_do_MCD_frame<false, false>();
}
int Do_SegaCD_Frame_No_VDP_Cycle_Accurate(void)
{
	return T_gens_do_MCD_frame<false, true>();
}
int Do_SegaCD_Frame(void)
{
	return T_gens_do_MCD_frame<true, false>();
}
int Do_SegaCD_Frame_Cycle_Accurate(void)
{
	return T_gens_do_MCD_frame<true, true>();
}
