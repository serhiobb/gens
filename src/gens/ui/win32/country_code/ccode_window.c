/***************************************************************************
 * Gens: (Win32) Country Code Window.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#include "ccode_window.h"
#include "gens/gens_window.h"

#include "emulator/gens.hpp"
#include "emulator/g_main.hpp"

// C includes.
#include <stdio.h>
#include <string.h>

// Win32 includes.
#include "libgsft/w32u/w32u_windows.h"
#include "libgsft/w32u/w32u_windowsx.h"
#include "libgsft/w32u/w32u_commctrl.h"
#include "ui/win32/resource.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"
#include "libgsft/gsft_win32_gdi.h"

// Window.
HWND ccode_window = NULL;
static BOOL	ccode_window_child_windows_created = FALSE;

// Window class.
static WNDCLASS	ccode_wndclass;

// Window size. (NOTE: THESE ARE IN DIALOG UNITS, and must be converted to pixels using DLU_X() / DLU_Y().)
#define CCODE_WINDOW_WIDTH  170
#define CCODE_WINDOW_DEF_HEIGHT 80
static int ccode_window_height;	// in pixels, not DLU

#define CCODE_WINDOW_BTN_SIZE      15

#define CCODE_WINDOW_FRACOUNTRY_WIDTH  (CCODE_WINDOW_WIDTH-10)
#define CCODE_WINDOW_FRACOUNTRY_HEIGHT (CCODE_WINDOW_DEF_HEIGHT-10-15-5)

#define CCODE_WINDOW_LSTCOUNTRYCODES_WIDTH  (CCODE_WINDOW_FRACOUNTRY_WIDTH-5-10-CCODE_WINDOW_BTN_SIZE)
#define CCODE_WINDOW_LSTCOUNTRYCODES_HEIGHT (CCODE_WINDOW_FRACOUNTRY_HEIGHT-10-5)

// Window procedure.
static LRESULT CALLBACK ccode_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	lstCountryCodes;
static HWND	btnOK, btnCancel, btnApply;

// Widget creation functions.
static void WINAPI ccode_window_create_child_windows(HWND hWnd);
static void WINAPI ccode_window_create_lstCountryCodes(HWND container);
static void WINAPI ccode_window_create_up_down_buttons(HWND container);

// Configuration load/save functions.
static void WINAPI ccode_window_init(void);
static void WINAPI ccode_window_save(void);

// Callbacks.
static void WINAPI ccode_window_callback_btnUp_clicked(void);
static void WINAPI ccode_window_callback_btnDown_clicked(void);

/** BEGIN: Common Controls v6.0 **/

// w32api's headers don't include BUTTON_IMAGELIST or related macros and constants.
#ifndef BCM_SETIMAGELIST
typedef struct
{
	HIMAGELIST himl;
	RECT margin;
	UINT uAlign;
} BUTTON_IMAGELIST, *PBUTTON_IMAGELIST;
#define BUTTON_IMAGELIST_ALIGN_LEFT 0
#define BUTTON_IMAGELIST_ALIGN_CENTER 4
#define BUTTON_IMAGELIST_ALIGN_RIGHT 1
#define BCM_FIRST 0x1600
#define BCM_SETIMAGELIST (BCM_FIRST + 0x0002)
#define Button_SetImageList(hWnd, pbuttonImageList) \
	SendMessage((hWnd), BCM_SETIMAGELIST, 0, (LPARAM)(pbuttonImageList))
#endif

// Image lists.
static HIMAGELIST	imglArrowUp = NULL;
static HIMAGELIST	imglArrowDown = NULL;

/** END: Common Controls v6.0 **/


/**
 * ccode_window_show(): Show the Country Code window.
 */
void ccode_window_show(void)
{
	if (ccode_window)
	{
		// Country Code window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(ccode_window, SW_SHOW);
		return;
	}
	
	ccode_window_child_windows_created = FALSE;
	
	if (ccode_wndclass.lpfnWndProc != ccode_window_wndproc)
	{
		// Create the window class.
		ccode_wndclass.style = 0;
		ccode_wndclass.lpfnWndProc = ccode_window_wndproc;
		ccode_wndclass.cbClsExtra = 0;
		ccode_wndclass.cbWndExtra = 0;
		ccode_wndclass.hInstance = ghInstance;
		ccode_wndclass.hIcon = LoadIconA(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		ccode_wndclass.hCursor = NULL;
		ccode_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		ccode_wndclass.lpszMenuName = NULL;
		ccode_wndclass.lpszClassName = "ccode_window";
		
		pRegisterClassU(&ccode_wndclass);
	}
	
	// Create the window.
	ccode_window = pCreateWindowU("ccode_window", "Country Code Order",
					WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					CW_USEDEFAULT, CW_USEDEFAULT,
					DLU_X(CCODE_WINDOW_WIDTH), DLU_Y(CCODE_WINDOW_DEF_HEIGHT),
					gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	// NOTE: This is done in ccode_window_create_child_windows() to compensate for listbox variations.
	//Win32_setActualWindowSize(ccode_window, DLU_X(CCODE_WINDOW_WIDTH), DLU_Y(CCODE_WINDOW_DEF_HEIGHT);
	
	// Center the window on the parent window.
	// NOTE: This is done in ccode_window_create_child_windows() to compensate for listbox variations.
	// TODO: Change Win32_centerOnGensWindow to accept two parameters.
	//Win32_centerOnGensWindow(ccode_window);
	
	UpdateWindow(ccode_window);
	ShowWindow(ccode_window, SW_SHOW);
}


/**
 * ccode_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void WINAPI ccode_window_create_child_windows(HWND hWnd)
{
	// Add a frame for country code selection.
	HWND fraCountry = pCreateWindowU(WC_BUTTON, "Country Code Order",
					WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					DLU_X(5), DLU_Y(5),
					DLU_X(CCODE_WINDOW_FRACOUNTRY_WIDTH),
					DLU_Y(CCODE_WINDOW_FRACOUNTRY_HEIGHT),
					hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(fraCountry, w32_fntMessage, TRUE);
	
	// Create the Country Code treeview.
	ccode_window_create_lstCountryCodes(hWnd);
	
	// Adjust the window height based on the listbox's actual height.
	gsft_win32_set_actual_window_size(hWnd, DLU_X(CCODE_WINDOW_WIDTH), ccode_window_height);
	gsft_win32_center_on_window(hWnd, gens_window);
	
	// Adjust the frame's height.
	SetWindowPos(fraCountry, 0, 0, 0,
			DLU_X(CCODE_WINDOW_FRACOUNTRY_WIDTH),
			DLU_Y(CCODE_WINDOW_FRACOUNTRY_HEIGHT - CCODE_WINDOW_DEF_HEIGHT) + ccode_window_height,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	
	// Create the Up/Down buttons.
	ccode_window_create_up_down_buttons(hWnd);
	
	// Create the dialog buttons.
	
	// TODO: Center the buttons, or right-align them?
	// They look better center-aligned in this window...
	int btnLeft = DLU_X((CCODE_WINDOW_WIDTH-50-5-50-5-50)/2);
	const int btnInc = DLU_X(5+50);
	
	// OK button.
	btnOK = pCreateWindowU(WC_BUTTON, "&OK",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
					btnLeft, ccode_window_height-DLU_Y(5+14),
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFontU(btnOK, w32_fntMessage, TRUE);
	
	// Cancel button.
	btnLeft += btnInc;
	btnCancel = pCreateWindowU(WC_BUTTON, "&Cancel",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					btnLeft, ccode_window_height-DLU_Y(5+14),
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFontU(btnCancel, w32_fntMessage, TRUE);
	
	// Apply button.
	btnLeft += btnInc;
	btnApply = pCreateWindowU(WC_BUTTON, "&Apply",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					btnLeft, ccode_window_height-DLU_Y(5+14),
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFontU(btnApply, w32_fntMessage, TRUE);
	
	// Disable the "Apply" button initially.
	Button_Enable(btnApply, FALSE);
	
	// Initialize the internal data variables.
	ccode_window_init();
	
	// Set focus to the listbox.
	SetFocus(lstCountryCodes);
}


/**
 * ccode_window_create_lstCountryCodes(): Create the Country Codes listbox.
 * @param container Container for the listbox.
 */
static void WINAPI ccode_window_create_lstCountryCodes(HWND container)
{
	// Create the listbox.
	lstCountryCodes = pCreateWindowExU(WS_EX_CLIENTEDGE, WC_LISTBOX, NULL,
						WS_CHILD | WS_VISIBLE | WS_TABSTOP,
						DLU_X(5+5), DLU_Y(5+10),
						DLU_X(CCODE_WINDOW_LSTCOUNTRYCODES_WIDTH),
						DLU_Y(CCODE_WINDOW_LSTCOUNTRYCODES_HEIGHT),
						container, NULL, ghInstance, NULL);
	SetWindowFontU(lstCountryCodes, w32_fntMessage, TRUE);
	
	// Check what the listbox's actual height is.
	RECT r;
	GetWindowRect(lstCountryCodes, &r);
	
	// Determine the window height based on the listbox's adjusted size.
	ccode_window_height = DLU_Y(CCODE_WINDOW_DEF_HEIGHT) + (r.bottom - r.top)
				- DLU_Y(CCODE_WINDOW_LSTCOUNTRYCODES_HEIGHT);
}


/**
 * ccode_window_create_up_down_buttons(): Create the Up/Down buttons.
 * @param container Container for the buttons.
 */
static void WINAPI ccode_window_create_up_down_buttons(HWND container)
{
	// Create the Up/Down buttons.
	// TODO: BS_ICON apparently doesn't work on NT4 and earlier.
	// See http://support.microsoft.com/kb/142226
	
	// Load the icons.
	HICON icoUp   = (HICON)LoadImageA(ghInstance, MAKEINTRESOURCE(IDI_ARROW_UP),
						IMAGE_ICON, 16, 16, LR_SHARED);
	HICON icoDown = (HICON)LoadImageA(ghInstance, MAKEINTRESOURCE(IDI_ARROW_DOWN),
						IMAGE_ICON, 16, 16, LR_SHARED);
	
	// Create the buttons.
	
	// "Up" button.
	HWND btnUp = pCreateWindowU(WC_BUTTON, NULL,
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_ICON | BS_CENTER,
					DLU_X(5+5+CCODE_WINDOW_LSTCOUNTRYCODES_WIDTH+5),
					DLU_Y(5+10),
					DLU_X(CCODE_WINDOW_BTN_SIZE), DLU_Y(CCODE_WINDOW_BTN_SIZE),
					container, (HMENU)IDC_COUNTRY_CODE_UP, ghInstance, NULL);
	SetWindowFontU(btnUp, w32_fntMessage, TRUE);
	
	// "Down" button.
	HWND btnDown = pCreateWindowU(WC_BUTTON, NULL,
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_ICON | BS_CENTER,
					DLU_X(5+5+CCODE_WINDOW_LSTCOUNTRYCODES_WIDTH+5),
					DLU_Y(5+10+CCODE_WINDOW_LSTCOUNTRYCODES_HEIGHT) +
						(ccode_window_height-DLU_Y(CCODE_WINDOW_DEF_HEIGHT))-DLU_Y(CCODE_WINDOW_BTN_SIZE),
					DLU_X(CCODE_WINDOW_BTN_SIZE), DLU_Y(CCODE_WINDOW_BTN_SIZE),
					container, (HMENU)IDC_COUNTRY_CODE_DOWN, ghInstance, NULL);
	SetWindowFontU(btnDown, w32_fntMessage, TRUE);
	
	// Set the button icons.
	if (comctl32_dll_version >= 0x05520000)
	{
		// Common Controls v6.0. (DLL version 5.82)
		// Use BUTTON_IMAGELIST and Button_SetImageList().
		// This ensures that visual styles are applied correctly.
		
		// "Up" button image list.
		imglArrowUp = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR4, 1, 1);
		ImageList_SetBkColor(imglArrowUp, CLR_NONE);
		ImageList_AddIcon(imglArrowUp, icoUp);
		
		// "Down" button image list.
		imglArrowDown = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR4, 1, 1);
		ImageList_SetBkColor(imglArrowDown, CLR_NONE);
		ImageList_AddIcon(imglArrowDown, icoDown);
		
		// BUTTON_IMAGELIST struct.
		BUTTON_IMAGELIST bimglBtn;
		bimglBtn.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
		bimglBtn.margin.top = 0;
		bimglBtn.margin.bottom = 0;
		bimglBtn.margin.left = 1;
		bimglBtn.margin.right = 0;
		
		// "Up" button.
		bimglBtn.himl = imglArrowUp;
		Button_SetImageList(btnUp, &bimglBtn);
		
		// "Down" button.
		bimglBtn.himl = imglArrowDown;
		Button_SetImageList(btnDown, &bimglBtn);
	}
	else
	{
		// Older version of Common Controls. Use BM_SETIMAGE.
		Button_SetTextU(btnUp, "Up");
		Button_SetTextU(btnDown, "Down");
		pSendMessageU(btnUp, BM_SETIMAGE, IMAGE_ICON, (LPARAM)icoUp);
		pSendMessageU(btnDown, BM_SETIMAGE, IMAGE_ICON, (LPARAM)icoDown);
	}
}


/**
 * ccode_window_close(): Close the About window.
 */
void ccode_window_close(void)
{
	if (!ccode_window)
		return;
	
	// Destroy the window.
	DestroyWindow(ccode_window);
	ccode_window = NULL;
	
	// Destroy the image lists.
	if (imglArrowUp)
	{
		ImageList_Destroy(imglArrowUp);
		imglArrowUp = NULL;
	}
	if (imglArrowDown)
	{
		ImageList_Destroy(imglArrowDown);
		imglArrowDown = NULL;
	}
}


/**
 * ccode_window_init(): Initialize the file text boxes.
 */
static void WINAPI ccode_window_init(void)
{
	// Set up the country order listbox.
	// Elements in Country_Order[3] can have one of three values:
	// - 0 [USA]
	// - 1 [JAP]
	// - 2 [EUR]
	
	// Make sure the country code order is valid.
	Check_Country_Order();
	
	// Clear the listbox.
	ListBox_ResetContentU(lstCountryCodes);
	
	// Country codes.
	// TODO: Move this to a common file.
	static const char ccodes[3][8] = {"USA", "Japan", "Europe"};
	
	// Add the country codes to the listbox in the appropriate order.
	int i;
	for (i = 0; i < 3; i++)
	{
		ListBox_InsertStringU(lstCountryCodes, i, ccodes[Country_Order[i]]);
		ListBox_SetItemDataU(lstCountryCodes, i, Country_Order[i]);
	}
	
	// Disable the "Apply" button initially.
	Button_Enable(btnApply, FALSE);
}


/**
 * ccode_window_save(): Save the Country Code order.
 */
static void WINAPI ccode_window_save(void)
{
	// Save settings.
	int i;
	for (i = 0; i < 3; i++)
	{
		Country_Order[i] = ListBox_GetItemDataU(lstCountryCodes, i);
	}
	
	// Validate the country code order.
	Check_Country_Order();
	
	// Disable the "Apply" button.
	Button_Enable(btnApply, FALSE);
}


/**
 * ccode_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK ccode_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			if (!ccode_window_child_windows_created)
				ccode_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					ccode_window_save();
					DestroyWindow(hWnd);
					break;
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case IDAPPLY:
					ccode_window_save();
					break;
				case IDC_COUNTRY_CODE_UP:
					ccode_window_callback_btnUp_clicked();
					break;
				case IDC_COUNTRY_CODE_DOWN:
					ccode_window_callback_btnDown_clicked();
					break;
				default:
					// Unknown command identifier.
					break;
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != ccode_window)
				break;
			
			ccode_window = NULL;
			
			// Destroy the image lists.
			if (imglArrowUp)
			{
				ImageList_Destroy(imglArrowUp);
				imglArrowUp = NULL;
			}
			if (imglArrowDown)
			{
				ImageList_Destroy(imglArrowDown);
				imglArrowDown = NULL;
			}
			
			break;
	}
	
	return pDefWindowProcU(hWnd, message, wParam, lParam);
}


/**
 * ccode_window_callback_btnUp_clicked(): "Up" button was clicked.
 */
static void WINAPI ccode_window_callback_btnUp_clicked(void)
{
	int curIndex = ListBox_GetCurSelU(lstCountryCodes);
	if (curIndex == -1 || curIndex == 0)
		return;
	
	// Swap the current item and the one above it.
	char aboveItem[32]; int aboveItemData;
	char curItem[32];   int curItemData;
	
	// Get the current text and data for each item.
	pListBox_GetTextU(lstCountryCodes, curIndex - 1, aboveItem);
	pListBox_GetTextU(lstCountryCodes, curIndex, curItem);
	aboveItemData = ListBox_GetItemDataU(lstCountryCodes, curIndex - 1);
	curItemData = ListBox_GetItemDataU(lstCountryCodes, curIndex);
	
	// Swap the strings and data.
	ListBox_DeleteStringU(lstCountryCodes, curIndex - 1);
	ListBox_InsertStringU(lstCountryCodes, curIndex - 1, curItem);
	ListBox_SetItemDataU(lstCountryCodes, curIndex - 1, curItemData);
	
	ListBox_DeleteStringU(lstCountryCodes, curIndex);
	ListBox_InsertStringU(lstCountryCodes, curIndex, aboveItem);
	ListBox_SetItemDataU(lstCountryCodes, curIndex, aboveItemData);
	
	// Set the current selection.
	ListBox_SetCurSelU(lstCountryCodes, curIndex - 1);
	
	// Enable the "Apply" button.
	Button_Enable(btnApply, TRUE);
}


/**
 * ccode_window_callback_btnDown_clicked(): "Down" button was clicked.
 */
static void WINAPI ccode_window_callback_btnDown_clicked(void)
{
	int curIndex = ListBox_GetCurSelU(lstCountryCodes);
	if (curIndex == -1 || curIndex >= (ListBox_GetCountU(lstCountryCodes) - 1))
		return;
	
	// Swap the current item and the one below it.
	char curItem[32];   int curItemData;
	char belowItem[32]; int belowItemData;
	
	// Get the current text and data for each item.
	pListBox_GetTextU(lstCountryCodes, curIndex, curItem);
	pListBox_GetTextU(lstCountryCodes, curIndex + 1, belowItem);
	curItemData = ListBox_GetItemDataU(lstCountryCodes, curIndex);
	belowItemData = ListBox_GetItemDataU(lstCountryCodes, curIndex + 1);
	
	// Swap the strings and data.
	ListBox_DeleteStringU(lstCountryCodes, curIndex);
	ListBox_InsertStringU(lstCountryCodes, curIndex, belowItem);
	ListBox_SetItemDataU(lstCountryCodes, curIndex, belowItemData);
	
	ListBox_DeleteStringU(lstCountryCodes, curIndex + 1);
	ListBox_InsertStringU(lstCountryCodes, curIndex + 1, curItem);
	ListBox_SetItemDataU(lstCountryCodes, curIndex + 1, curItemData);
	
	// Set the current selection.
	ListBox_SetCurSelU(lstCountryCodes, curIndex + 1);
	
	// Enable the "Apply" button.
	Button_Enable(btnApply, TRUE);
}
