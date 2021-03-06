/***************************************************************************
 * Gens: (Win32) Directory Configuration Window.                           *
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

#include "dir_window.hpp"
#include "ui/common/dir_window_common.h"
#include "gens/gens_window.h"

// Plugin Manager.
#include "plugins/pluginmgr.hpp"

// MDP error codes.
#include "mdp/mdp_error.h"

// C includes.
#include <string.h>

// C++ includes.
#include <string>
#include <vector>
#include <list>
using std::string;
using std::vector;
using std::list;

// Win32 includes.
#include "libgsft/w32u/w32u_windows.h"
#include "libgsft/w32u/w32u_windowsx.h"
#include "libgsft/w32u/w32u_commctrl.h"
#include "ui/win32/resource.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"
#include "libgsft/gsft_win32_gdi.h"
#include "libgsft/gsft_file.h"
#include "libgsft/gsft_szprintf.h"

// Gens includes.
#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"
#include "util/file/file.hpp"


// Window.
HWND dir_window = NULL;

// Window class.
static WNDCLASS dir_wndclass;

// Window size. (NOTE: THESE ARE IN DIALOG UNITS, and must be converted to pixels using DLU_X() / DLU_Y().)
#define DIR_WINDOW_WIDTH  225
#define DIR_WINDOW_HEIGHT_DEFAULT ((DIR_WINDOW_ENTRIES_COUNT*15)+10+10+10+15)
#define DIR_FRAME_HEIGHT(entries) (((entries)*15)+10+5)

// Widgets.
static HWND	btnOK, btnCancel, btnApply;
#define IDC_DIR_DIRECTORY 0x1200

// Directory widgets.
typedef struct _dir_widget_t
{
	HWND	txt;
	string	title;
	bool	is_plugin;
	int	id;
} dir_widget_t;
static vector<dir_widget_t> vectDirs;

// Command value bases.
#define IDC_DIR_BTNCHANGE	0x8000

// Window procedure.
static LRESULT CALLBACK dir_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widget creation functions.
static void WINAPI dir_window_create_child_windows(HWND hWnd);
static HWND WINAPI dir_window_create_dir_widgets(const char *title, HWND parent, int y, int id);

// Directory configuration load/save functions.
static void WINAPI dir_window_init(void);
static void WINAPI dir_window_save(void);

// Callbacks.
static void WINAPI dir_window_callback_btnChange_clicked(int dir);


/**
 * dir_window_show(): Show the Directory Configuration window.
 */
void dir_window_show(void)
{
	if (dir_window)
	{
		// Directory Configuration window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(dir_window, SW_SHOW);
		return;
	}
	
	if (dir_wndclass.lpfnWndProc != dir_window_wndproc)
	{
		// Create the window class.
		dir_wndclass.style = 0;
		dir_wndclass.lpfnWndProc = dir_window_wndproc;
		dir_wndclass.cbClsExtra = 0;
		dir_wndclass.cbWndExtra = 0;
		dir_wndclass.hInstance = ghInstance;
		dir_wndclass.hIcon = LoadIconA(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		dir_wndclass.hCursor = LoadCursorA(NULL, IDC_ARROW);
		dir_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		dir_wndclass.lpszMenuName = NULL;
		dir_wndclass.lpszClassName = "dir_window";
		
		pRegisterClassU(&dir_wndclass);
	}
	
	// Create the window.
	dir_window = pCreateWindowU("dir_window", "Configure Directories",
					WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					CW_USEDEFAULT, CW_USEDEFAULT,
					DLU_X(DIR_WINDOW_WIDTH), DLU_Y(DIR_WINDOW_HEIGHT_DEFAULT),
					gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	// NOTE: This is done in dir_window_create_child_windows to compensate for plugin directories.
	//Win32_setActualWindowSize(dir_window, DLU_X(DIR_WINDOW_WIDTH), DLU_Y(DIR_WINDOW_HEIGHT);
	
	// Center the window on the parent window.
	// NOTE: This is done in dir_window_create_child_windows to compensate for plugin directories.
	// TODO: Change Win32_centerOnGensWindow to accept two parameters.
	//Win32_centerOnGensWindow(dir_window);
	
	UpdateWindow(dir_window);
	ShowWindow(dir_window, SW_SHOW);
}


/**
 * cc_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void WINAPI dir_window_create_child_windows(HWND hWnd)
{
	// Create the internal directory entry frame.
	HWND fraInternalDirs = pCreateWindowU(WC_BUTTON, "Gens/GS Directories",
						WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
						DLU_X(5), DLU_Y(5),
						DLU_X(DIR_WINDOW_WIDTH-10),
						DLU_Y(DIR_FRAME_HEIGHT(DIR_WINDOW_ENTRIES_COUNT)),
						hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(fraInternalDirs, w32_fntMessage, true);
	
	// Initialize the directory widget vector.
	vectDirs.clear();
	vectDirs.reserve(DIR_WINDOW_ENTRIES_COUNT + PluginMgr::lstDirectories.size());
	
	// Create all internal directory entry widgets.
	int curTop = 5+13;
	dir_widget_t dir_widget;
	dir_widget.is_plugin = false;
	for (unsigned int dir = 0; dir < DIR_WINDOW_ENTRIES_COUNT; dir++, curTop += 15)
	{
		dir_widget.id = dir;
		dir_widget.title = string(dir_window_entries[dir].title);
		
		dir_widget.txt = dir_window_create_dir_widgets(
					dir_window_entries[dir].title,
					hWnd, DLU_Y(curTop), vectDirs.size());
		vectDirs.push_back(dir_widget);
	}
	
	// If any plugin directories exist, create the plugin directory entry frame.
	if (!PluginMgr::lstDirectories.empty())
	{
		// Create the plugin directory entry frame.
		curTop = 5+DIR_FRAME_HEIGHT(DIR_WINDOW_ENTRIES_COUNT)+5;
		HWND fraPluginDirs = pCreateWindowU(WC_BUTTON, "Plugin Directories",
							WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
							DLU_X(5), DLU_Y(curTop),
							DLU_X(DIR_WINDOW_WIDTH-10),
							DLU_Y(DIR_FRAME_HEIGHT(PluginMgr::lstDirectories.size())),
							hWnd, NULL, ghInstance, NULL);
		SetWindowFontU(fraPluginDirs, w32_fntMessage, true);
		
		// Create all plugin directory entry widgets.
		int dir = 0x10;
		curTop += 10+3;
		dir_widget.is_plugin = true;
		
		for (list<mdpDir_t>::iterator iter = PluginMgr::lstDirectories.begin();
		     iter != PluginMgr::lstDirectories.end(); iter++, dir++, curTop += 15)
		{
			dir_widget.id = (*iter).id;
			dir_widget.title = (*iter).name;
			
			dir_widget.txt = dir_window_create_dir_widgets(
						(*iter).name.c_str(),
						hWnd, DLU_Y(curTop), vectDirs.size());			
			vectDirs.push_back(dir_widget);
		}
	}
	
	// Calculate the window height. (in pixels)
	int dir_window_height = DLU_Y(5+DIR_FRAME_HEIGHT(DIR_WINDOW_ENTRIES_COUNT)+5+15+5);
	if (!PluginMgr::lstDirectories.empty())
	{
		dir_window_height += DLU_Y(DIR_FRAME_HEIGHT(PluginMgr::lstDirectories.size())+5);
	}
	
	// Set the actual window size.
	gsft_win32_set_actual_window_size(hWnd, DLU_X(DIR_WINDOW_WIDTH), dir_window_height);
	
	// Center the window on the parent window.
	gsft_win32_center_on_window(hWnd, gens_window);
	
	// Create the dialog buttons.
	int btnLeft = DLU_X(DIR_WINDOW_WIDTH-5-50-5-50-5-50);
	const int btnInc = DLU_X(5+50);
	
	// OK button.
	btnOK = pCreateWindowU(WC_BUTTON, "&OK",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				btnLeft, dir_window_height-DLU_Y(5+14),
				DLU_X(50), DLU_Y(14),
				hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFontU(btnOK, w32_fntMessage, true);
	
	// Cancel button.
	btnLeft += btnInc;
	btnCancel = pCreateWindowU(WC_BUTTON, "&Cancel",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					btnLeft, dir_window_height-DLU_Y(5+14),
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFontU(btnCancel, w32_fntMessage, true);
	
	// Apply button.
	btnLeft += btnInc;
	btnApply = pCreateWindowU(WC_BUTTON, "&Apply",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					btnLeft, dir_window_height-DLU_Y(5+14),
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFontU(btnApply, w32_fntMessage, true);
	
	// Disable the "Apply" button initially.
	Button_Enable(btnApply, false);
	
	// Initialize the directory entries.
	dir_window_init();
}


/**
 * dir_window_create_dir_widgets(): Create directory widgets.
 * @param title Directory title.
 * @param container Container.
 * @param y Vertical coordinate. (in pixels)
 * @param id Directory ID.
 * @return hWnd of the directory textbox.
 */
static HWND WINAPI dir_window_create_dir_widgets(const char *title, HWND container, int y, int id)
{
	// Create the label for the directory.
	HWND lblTitle = pCreateWindowU(WC_STATIC, title,
					WS_CHILD | WS_VISIBLE | SS_LEFT,
					DLU_X(5+5), y,
					DLU_X(45), DLU_Y(10),
					container, NULL, ghInstance, NULL);
	SetWindowFontU(lblTitle, w32_fntMessage, true);
	
	// Create the textbox for the directory.
	HWND txtDirectory = pCreateWindowExU(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
						WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,
						DLU_X(5+5+45+5), y,
						DLU_X(DIR_WINDOW_WIDTH-(5+45+10+45+5+10)), DLU_Y(12),
						container, (HMENU)(IDC_DIR_DIRECTORY), ghInstance, NULL);
	SetWindowFontU(txtDirectory, w32_fntMessage, true);
	
	// Create the "Change" button for the directory.
	// TODO: Use an icon?
	HWND btnChange = pCreateWindowU(WC_BUTTON, "Change...",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					DLU_X(DIR_WINDOW_WIDTH-5-45-5), y,
					DLU_X(45), DLU_Y(12),
					container, (HMENU)(IDC_DIR_BTNCHANGE + id), ghInstance, NULL);
	SetWindowFontU(btnChange, w32_fntMessage, true);
	
	return txtDirectory;
}


/**
 * dir_window_close(): Close the OpenGL Resolution window.
 */
void dir_window_close(void)
{
	if (!dir_window)
		return;
	
	// Destroy the window.
	DestroyWindow(dir_window);
	dir_window = NULL;
}


/**
 * dir_window_init(): Initialize the Directory Configuration entries.
 */
static void WINAPI dir_window_init(void)
{
	char dir_buf[GENS_PATH_MAX];
	char dir_buf_rel[GENS_PATH_MAX];
	
	// Internal directories.
	for (size_t dir = 0; dir < vectDirs.size(); dir++)
	{
		if (!vectDirs[dir].is_plugin)
		{
			// Internal directory.
			Edit_SetTextU(vectDirs[dir].txt,
					dir_window_entries[vectDirs[dir].id].entry);
		}
		else
		{
			// Plugin directory.
			mapDirItems::iterator dirIter = PluginMgr::tblDirectories.find(vectDirs[dir].id);
			if (dirIter == PluginMgr::tblDirectories.end())
				continue;
			
			list<mdpDir_t>::iterator lstDirIter = (*dirIter).second;
			const mdpDir_t& mdpDir = *lstDirIter;
			
			// Get the directory.
			if (mdpDir.get(vectDirs[dir].id, dir_buf, sizeof(dir_buf)) != MDP_ERR_OK)
			{
				// Error retrieving the directory.
				continue;
			}
			
			// Directory retrieved.
			// If possible, convert it to a relative pathname.
			gsft_file_abs_to_rel(dir_buf, PathNames.Gens_Save_Path,
						dir_buf_rel, sizeof(dir_buf_rel));
			Edit_SetTextU(vectDirs[dir].txt, dir_buf_rel);
		}
	}
	
	// Disable the "Apply" button initially.
	Button_Enable(btnApply, false);
}

/**
 * dir_window_save(): Save the Directory Configuration entries.
 */
static void WINAPI dir_window_save(void)
{
	size_t len;
	char dir_buf[GENS_PATH_MAX];
	char dir_buf_abs[GENS_PATH_MAX];
	
	for (size_t dir = 0; dir < vectDirs.size(); dir++)
	{
		if (!vectDirs[dir].is_plugin)
		{
			// Internal directory.
			char *entry = dir_window_entries[vectDirs[dir].id].entry;
			
			// Get the entry text.
			Edit_GetTextU(vectDirs[dir].txt, entry, GENS_PATH_MAX);
			
			// Make sure the end of the directory has a slash.
			// TODO: Do this in functions that use pathnames.
			len = strlen(entry);
			if (len > 0 && entry[len-1] != GSFT_DIR_SEP_CHR)
			{
				// String needs to be less than 1 minus the max path length
				// in order to be able to append the directory separator.
				if (len < (GENS_PATH_MAX-1))
				{
					entry[len] = GSFT_DIR_SEP_CHR;
					entry[len+1] = 0x00;
				}
			}
		}
		else
		{
			// Plugin directory.
			
			// Get the entry text.
			Edit_GetTextU(vectDirs[dir].txt, dir_buf, sizeof(dir_buf));
			
			// Make sure the entry is null-terminated.
			dir_buf[sizeof(dir_buf)-1] = 0x00;
			
			// Make sure the end of the directory has a slash.
			// TODO: Do this in functions that use pathnames.
			len = strlen(dir_buf);
			if (len > 0 && dir_buf[len-1] != GSFT_DIR_SEP_CHR)
			{
				// String needs to be less than 1 minus the max path length
				// in order to be able to append the directory separator.
				if (len < (GENS_PATH_MAX-1))
				{
					dir_buf[len] = GSFT_DIR_SEP_CHR;
					dir_buf[len+1] = 0x00;
				}
			}
			
			// If necessary, convert the pathname to an absolute pathname.
			gsft_file_rel_to_abs(dir_buf, PathNames.Gens_Save_Path,
						dir_buf_abs, sizeof(dir_buf_abs));
			
			mapDirItems::iterator dirIter = PluginMgr::tblDirectories.find(vectDirs[dir].id);
			if (dirIter == PluginMgr::tblDirectories.end())
				continue;
			
			list<mdpDir_t>::iterator lstDirIter = (*dirIter).second;
			const mdpDir_t& mdpDir = *lstDirIter;
			
			// Set the directory.
			mdpDir.set(vectDirs[dir].id, dir_buf_abs);
		}
	}
	
	// Disable the "Apply" button.
	Button_Enable(btnApply, false);
}


/**
 * dir_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK dir_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			dir_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					dir_window_save();
					DestroyWindow(hWnd);
					break;
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case IDAPPLY:
					dir_window_save();
					break;
				default:
					switch (LOWORD(wParam) & 0xFF00)
					{
						case IDC_DIR_BTNCHANGE:
							// Change a directory.
							dir_window_callback_btnChange_clicked(LOWORD(wParam) & 0x7FFF);
							break;
						case IDC_DIR_DIRECTORY:
							if (HIWORD(wParam) == EN_CHANGE)
							{
								// Directory textbox was changed.
								// Enable the "Apply" button.
								Button_Enable(btnApply, true);
							}
							break;
						default:
							break;
					}
					break;
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != dir_window)
				break;
			
			dir_window = NULL;
			break;
		
		default:
			break;
	}
	
	return pDefWindowProcU(hWnd, message, wParam, lParam);
}


/**
 * dir_window_callback_btnChange_clicked(): A "Change" button was clicked.
 * @param dir Directory number.
 */
static void WINAPI dir_window_callback_btnChange_clicked(int dir)
{
	char dir_buf[GENS_PATH_MAX];
	char dir_buf_abs[GENS_PATH_MAX];
	string new_dir;
	
	if (dir >= vectDirs.size())
		return;
	
	dir_widget_t *dir_widget = &vectDirs[dir];
	
	// Get the currently entered directory.
	Edit_GetTextU(dir_widget->txt, dir_buf, sizeof(dir_buf));
	dir_buf[sizeof(dir_buf)-1] = 0x00;
	
	// Convert the current directory to an absolute pathname, if necessary.
	gsft_file_rel_to_abs(dir_buf, PathNames.Gens_Save_Path,
				dir_buf_abs, sizeof(dir_buf_abs));
	
	// Set the title of the window.
	char tmp[128];
	szprintf(tmp, sizeof(tmp), "Select %s Directory", dir_widget->title.c_str());
	
	// Request a new directory.
	new_dir = GensUI::selectDir(tmp, dir_buf_abs, dir_window);
	
	// If "Cancel" was selected, don't do anything.
	if (new_dir.empty())
		return;
	
	// Make sure the end of the directory has a slash.
	if (new_dir.at(new_dir.length() - 1) != GSFT_DIR_SEP_CHR)
		new_dir += GSFT_DIR_SEP_CHR;
	
	// Convert the new directory to a relative pathname, if possible.
	gsft_file_abs_to_rel(new_dir.c_str(), PathNames.Gens_Save_Path,
				dir_buf, sizeof(dir_buf));
	
	// Set the new directory.
	Edit_SetTextU(dir_widget->txt, dir_buf);
	
	// Enable the "Apply" button.
	Button_Enable(btnApply, true);
}
