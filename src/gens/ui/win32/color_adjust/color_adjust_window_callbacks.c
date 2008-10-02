/***************************************************************************
 * Gens: (GTK+) Color Adjustment Window - Callback Functions.              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#include "color_adjust_window.h"
#include "color_adjust_window_callbacks.h"
#include "color_adjust_window_misc.h"

#include "emulator/gens.hpp"
#include "emulator/g_main.hpp"

// Gens Win32 resources
#include "ui/win32/resource.h"


LRESULT CALLBACK Color_Adjust_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			Color_Adjust_Window_CreateChildWindows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(color_adjust_window);
			return 0;
		
		case WM_CTLCOLORSTATIC:
			if (hWnd != color_adjust_window)
				break;
			
#if 0
			// Set the title and version labels to transparent.
			if ((HWND)lParam == lblGensTitle ||
				(HWND)lParam == lblGensDesc)
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (LRESULT)GetStockObject(NULL_BRUSH);
			}
			return TRUE;
#endif
			break;
		
		case WM_COMMAND:
#if 0
			if (LOWORD(wParam) == 0x8472)
				DestroyWindow(about_window);
#endif
			break;
		
		case WM_DESTROY:
			if (hWnd != color_adjust_window)
				break;
			
			color_adjust_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


#if 0
/**
 * Window is closed.
 */
gboolean on_color_adjust_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(widget);
	GENS_UNUSED_PARAMETER(event);
	GENS_UNUSED_PARAMETER(user_data);
	
	gtk_widget_destroy(color_adjust_window);
	color_adjust_window = NULL;
	return FALSE;
}


/**
 * Cancel
 */
void on_button_ca_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	gtk_widget_destroy(color_adjust_window);
	color_adjust_window = NULL;
}


/**
 * Apply
 */
void on_button_ca_Apply_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	CA_Save();
}


/**
 * Save
 */
void on_button_ca_Save_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	CA_Save();
	gtk_widget_destroy(color_adjust_window);
	color_adjust_window = NULL;
}
#endif
