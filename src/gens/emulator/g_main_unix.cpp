/**
 * Gens: Main loop. (Linux specific code)
 */


#include "g_main.hpp"
#include "g_main_unix.hpp"

#include <sys/stat.h>
#include <cstdio>
#include <stdlib.h>
#include <cstring>

// C++ includes
#include <list>
using std::list;

#include "g_palette.h"
#include "gens_ui.hpp"
#include "g_md.hpp"
#include "g_32x.hpp"

#include "parse.hpp"
#include "options.hpp"

#include "gens_core/vdp/vdp_io.h"
#include "util/file/config_file.hpp"
#include "util/sound/gym.hpp"

// Video, Audio, Input.
#include "video/vdraw_sdl.h"
#include "video/vdraw_cpp.hpp"
#include "audio/audio_sdl.hpp"
#include "input/input_sdl.h"

#include "gens/gens_window_sync.hpp"

#include "port/timer.h"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */


/**
 * Get_Save_Path(): Create the default save path.
 * @param *buf Buffer to store the default save path in.
 */
void Get_Save_Path(char *buf, size_t n)
{
	strncpy(buf, getenv("HOME"), n);
	
	// OS-specific save path.
	#ifdef GENS_OS_MACOSX
		strcat(buf, "/Library/Application Support/Gens/");
	#else // Other Unix or Linux system.
		strcat(buf, "/.gens/");
	#endif
}


/**
 * Create_Save_Directory(): Create the default save directory.
 * @param *dir Directory name.
 */
void Create_Save_Directory(const char *dir)
{
	mkdir(dir, 0755);
}


/**
 * main(): Main loop.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return Error code.
 */
int main(int argc, char *argv[])
{
	if (geteuid() == 0)
	{
		// Don't run Gens/GS as root!
		const char gensRootErr[] = "Error: Gens/GS should not be run as root.\nPlease log in as a regular user.\n";
		
		fprintf(stderr, gensRootErr);
		
		#ifdef GENS_UI_GTK
			// Check if X is running.
			char *display = getenv("DISPLAY");
			if (display)
			{
				gtk_init(0, NULL);
				GensUI::msgBox(gensRootErr, GENS_APPNAME " - Permissions Error", GensUI::MSGBOX_ICON_ERROR);
			}
		#endif /* GENS_UI_GTK */
		
		return 1;
	}
	
	// Initialize the timer.
	// TODO: Make this unnecessary.
	init_timer();
	
	// Initialize vdraw_sdl.
	vdraw_init();
	vdraw_backend_init_subsystem(VDRAW_BACKEND_SDL);
	
	// Initialize input_sdl.
	input_init(INPUT_BACKEND_SDL);
	
	// Initialize the audio object.
	audio = new Audio_SDL();
	
	// Initialize the Settings struct.
	if (Init_Settings())
	{
		// Error initializing settings.
		delete audio;
		input_end();
		vdraw_end();
		return 1;	// TODO: Replace with a better error code.
	}
	
	// Parse command line arguments.
	parseArgs(argc, argv);
	
	// Recalculate the palettes, in case a command line argument changed a video setting.
	Recalculate_Palettes();
	
	Init_Genesis_Bios();
	
	// Initialize Gens.
	if (!Init())
		return 0;
	
	// Initialize the UI.
	GensUI::init(argc, argv);
	
	// not yet finished (? - wryun)
	//initializeConsoleRomsView();
	
#ifdef GENS_OPENGL
	// Check if OpenGL needs to be enabled.
	// This also initializes SDL or SDL+GL.
	Options::setOpenGL(Video.OpenGL);
#else
	// Initialize SDL.
	vdraw_backend_init(VDRAW_BACKEND_SDL);
#endif
	
	if (strcmp(PathNames.Start_Rom, "") != 0)
	{
		if (ROM::openROM(PathNames.Start_Rom) == -1)
		{
			fprintf(stderr, "%s(): Failed to load %s\n", __func__, PathNames.Start_Rom);
		}
	}
	
	// Update the UI.
	GensUI::update();
	
	// Reset the renderer.
	vdraw_reset_renderer(TRUE);
	
	// Synchronize the Gens window.
	Sync_Gens_Window();
	
	// Run the Gens Main Loop.
	GensMainLoop();
	
	Get_Save_Path(Str_Tmp, GENS_PATH_MAX);
	strcat(Str_Tmp, "gens.cfg");
	Config::save(Str_Tmp);
	
	End_All();
	return 0;
}
