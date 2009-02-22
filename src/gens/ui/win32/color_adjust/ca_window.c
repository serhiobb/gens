/***************************************************************************
 * Gens: (GTK+) Color Adjustment Window.                                   *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ca_window.h"
#include "gens/gens_window.h"

// C includes.
#include <stdio.h>

// Win32 includes.
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

// Gens Win32 resources.
#include "ui/win32/resource.h"

// Unused Parameter macro.
#include "macros/unused.h"

// Gens includes.
#include "emulator/g_palette.h"
#include "emulator/g_main.hpp"
#include "emulator/g_update.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/file/rom.hpp"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */


// Window.
HWND ca_window = NULL;

// Window class.
static WNDCLASS ca_wndclass;

// Window size.
#define CA_WINDOW_WIDTH  306
#define CA_WINDOW_HEIGHT 140

#define CA_TRACKBAR_WIDTH  201
#define CA_TRACKBAR_HEIGHT 24

// Window procedure.
static LRESULT CALLBACK ca_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	trkContrast;
static HWND	lblContrastVal;
static HWND	trkBrightness;
static HWND	lblBrightnessVal;

static HWND	chkGrayscale;
static HWND	chkInverted;

// Widget creation functions.
static void	ca_window_create_child_windows(HWND hWnd);

// Color adjustment load/save functions.
static void	ca_window_init(void);
static void	ca_window_save(void);


/**
 * ca_window_show(): Show the Color Adjustment window.
 */
void ca_window_show(void)
{
	if (ca_window)
	{
		// Color Adjustment window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(ca_window, SW_SHOW);
		return;
	}
	
	if (ca_wndclass.lpfnWndProc != ca_window_wndproc)
	{
		// Create the window class.
		ca_wndclass.style = 0;
		ca_wndclass.lpfnWndProc = ca_window_wndproc;
		ca_wndclass.cbClsExtra = 0;
		ca_wndclass.cbWndExtra = 0;
		ca_wndclass.hInstance = ghInstance;
		ca_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC));
		ca_wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		ca_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		ca_wndclass.lpszMenuName = NULL;
		ca_wndclass.lpszClassName = "ca_window";
		
		RegisterClass(&ca_wndclass);
	}
	
	// Create the window.
	ca_window = CreateWindow("ca_window", "Color Adjustment",
				 WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 CA_WINDOW_WIDTH, CA_WINDOW_HEIGHT,
				 gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(ca_window, CA_WINDOW_WIDTH, CA_WINDOW_HEIGHT);
	
	// Center the window on the parent window.
	// TODO: Change Win32_centerOnGensWindow to accept two parameters.
	Win32_centerOnGensWindow(ca_window);
	
	UpdateWindow(ca_window);
	ShowWindow(ca_window, SW_SHOW);
}


/**
 * ca_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void ca_window_create_child_windows(HWND hWnd)
{
	// Style for the trackbars.
	static const unsigned int trkStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_HORZ | TBS_BOTTOM;
	
	// "Contrast" label.
	HWND lblContrast = CreateWindow(WC_STATIC, "Co&ntrast",
					WS_CHILD | WS_VISIBLE | SS_LEFT,
					8, 16, 56, 16,
					hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblContrast, fntMain, TRUE);
	
	// "Contrast" trackbar.
	trkContrast = CreateWindow(TRACKBAR_CLASS, NULL, trkStyle,
				   8+56, 14,
				   CA_TRACKBAR_WIDTH, CA_TRACKBAR_HEIGHT,
				   hWnd, (HMENU)IDC_TRK_CA_CONTRAST, ghInstance, NULL);
	SendMessage(trkContrast, TBM_SETPAGESIZE, 0, 10);
	SendMessage(trkContrast, TBM_SETTICFREQ, 25, 0);
	SendMessage(trkContrast, TBM_SETRANGE, TRUE, MAKELONG(-100, 100));
	SendMessage(trkContrast, TBM_SETPOS, TRUE, 0);
	
	// "Contrast" value label.
	lblContrastVal = CreateWindow(WC_STATIC, "0",
				      WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
				      8+56+CA_TRACKBAR_WIDTH+8, 16, 32, 16,
				      hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblContrastVal, fntMain, TRUE);
	
	// "Brightness" label.
	HWND lblBrightness = CreateWindow(WC_STATIC, "&Brightness",
					WS_CHILD | WS_VISIBLE | SS_LEFT,
					8, 16+32, 56, 16,
					hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblBrightness, fntMain, TRUE);
	
	// "Brightness" trackbar.
	trkBrightness = CreateWindow(TRACKBAR_CLASS, NULL, trkStyle,
				     8+56, 14+32,
				     CA_TRACKBAR_WIDTH, CA_TRACKBAR_HEIGHT,
				     hWnd, (HMENU)IDC_TRK_CA_CONTRAST, ghInstance, NULL);
	SendMessage(trkBrightness, TBM_SETPAGESIZE, 0, 10);
	SendMessage(trkBrightness, TBM_SETTICFREQ, 25, 0);
	SendMessage(trkBrightness, TBM_SETRANGE, TRUE, MAKELONG(-100, 100));
	SendMessage(trkBrightness, TBM_SETPOS, TRUE, 0);
	
	// "Brightness" value label.
	lblBrightnessVal = CreateWindow(WC_STATIC, "0",
					WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
					8+56+CA_TRACKBAR_WIDTH+8, 16+32, 32, 16,
					hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblBrightnessVal, fntMain, TRUE);
	
	// Center the checkboxes.
	static const unsigned int chkLeft = (CA_WINDOW_WIDTH-80-8-80+16)/2;
	
	// "Grayscale" checkbox.
	chkGrayscale = CreateWindow(WC_BUTTON, "&Grayscale",
				    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
				    chkLeft, 16+32+32, 80, 16,
				    hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkGrayscale, fntMain, TRUE);
	
	// "Inverted" checkbox.
	chkInverted = CreateWindow(WC_BUTTON, "&Inverted",
				   WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
				   chkLeft+8+80, 16+32+32, 80, 16,
				   hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkInverted, fntMain, TRUE);
	
	// Create the dialog buttons.
	
	// TODO: Center the buttons, or right-align them?
	// They look better center-aligned in this window...
	static const unsigned int btnLeft = (CA_WINDOW_WIDTH-75-8-75-8-75)/2;
	// OK button.
	HWND btnOK = CreateWindow(WC_BUTTON, "&OK",
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				  btnLeft, CA_WINDOW_HEIGHT-8-24,
				  75, 23,
				  hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	// Apply button.
	HWND btnApply = CreateWindow(WC_BUTTON, "&Apply",
				     WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				     btnLeft+8+75, CA_WINDOW_HEIGHT-8-24,
				     75, 23,
				     hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, TRUE);
	
	// Cancel button.
	HWND btnCancel = CreateWindow(WC_BUTTON, "&Cancel",
				      WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				      btnLeft+8+75+8+75, CA_WINDOW_HEIGHT-8-24,
				      75, 23,
				      hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
		
	// Initialize the color adjustment spinbuttons.
	ca_window_init();

	// Set focus to the "Contrast" trackbar.
	SetFocus(trkContrast);
}


/**
 * ca_window_close(): Close the Color Adjustment window.
 */
void ca_window_close(void)
{
	if (!ca_window)
		return;
	
	// Destroy the window.
	DestroyWindow(ca_window);
	ca_window = NULL;
}


/**
 * ca_window_init(): Initialize the Color Adjustment window widgets.
 */
static void ca_window_init(void)
{
	char buf[16];
	
	// Contrast.
	SendMessage(trkContrast, TBM_SETPOS, TRUE, (Contrast_Level - 100));
	sprintf(buf, "%d", (Contrast_Level - 100));
	SetWindowText(lblContrastVal, buf);
	
	// Brightness.
	SendMessage(trkBrightness, TBM_SETPOS, TRUE, (Brightness_Level - 100));
	sprintf(buf, "%d", (Brightness_Level - 100));
	SetWindowText(lblBrightnessVal, buf);
	
	// Checkboxes.
	Button_SetCheck(chkGrayscale, (Greyscale ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkInverted, (Invert_Color ? BST_CHECKED : BST_UNCHECKED));
}


/**
 * ca_window_save(): Save the color adjustment settings.
 */
static void ca_window_save(void)
{
	Contrast_Level = (SendMessage(trkContrast, TBM_GETPOS, 0, 0) + 100);
	Brightness_Level = (SendMessage(trkBrightness, TBM_GETPOS, 0, 0) + 100);
	
	Greyscale = (Button_GetCheck(chkGrayscale) == BST_CHECKED);
	Invert_Color = (Button_GetCheck(chkInverted) == BST_CHECKED);
	
	// Recalculate palettes.
	Recalculate_Palettes();
	if (Game)
	{
		// Emulation is running. Update the CRAM.
		CRam_Flag = 1;
		
		if (!Paused)
		{
			// TODO: Just update CRAM. Don't update the frame.
			Update_Emulation_One();
			#ifdef GENS_DEBUGGER
				if (Debug)
					Update_Debug_Screen();
			#endif
		}
	}
}


/**
 * ca_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK ca_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			ca_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					ca_window_save();
					DestroyWindow(hWnd);
					break;
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case IDAPPLY:
					ca_window_save();
					break;
				default:
					// Unknown command identifier.
					break;
			}
			break;
		
		case WM_HSCROLL:
		{
			// Trackbar scroll.
			char buf[16];
			int scrlPos;
			
			switch (LOWORD(wParam))
			{
				case TB_THUMBPOSITION:
				case TB_THUMBTRACK:
					// Scroll position is in the high word of wParam.
					scrlPos = (signed short)HIWORD(wParam);
					break;
				
				default:
					// Send TBM_GETPOS to the trackbar to get the position.
					scrlPos = SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
					break;
			}
			
			// Convert the scroll position to a string.
			sprintf(buf, "%d", scrlPos);
			
			// Set the value label.
			if ((HWND)lParam == trkContrast)
				SetWindowText(lblContrastVal, buf);
			else if ((HWND)lParam == trkBrightness)
				SetWindowText(lblBrightnessVal, buf);
			
			break;
		}
		
		case WM_DESTROY:
			if (hWnd != ca_window)
				break;
			
			ca_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}