/**
 * GENS: (GTK+) Color Adjust Window - Miscellaneous Functions.
 */


#include <string.h>

#include "color_adjust_window.h"
#include "color_adjust_window_callbacks.h"
#include "color_adjust_window_misc.h"
#include "gens/gens_window.h"

#include <gtk/gtk.h>
#include "gtk-misc.h"

#include "g_palette.h"
#include "vdp_io.h"
#include "g_main.h"
#include "g_sdldraw.h"


/**
 * Open_Color_Adjust(): Opens the Color Adjust window.
 */
void Open_Color_Adjust(void)
{
	GtkWidget *ca;
	GtkWidget *hscale_slider_contrast, *hscale_slider_brightness;
	GtkWidget *check_options_grayscale, *check_options_inverted;
	
	ca = create_color_adjust_window();
	if (!ca)
	{
		// Either an error occurred while creating the Color Adjust window,
		// or the Color Adjust window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(ca), GTK_WINDOW(gens_window));
	
	// Load settings.
	hscale_slider_contrast = lookup_widget(ca, "hscale_slider_contrast");
	gtk_range_set_value(GTK_RANGE(hscale_slider_contrast), Contrast_Level - 100);
	hscale_slider_brightness = lookup_widget(ca, "hscale_slider_brightness");
	gtk_range_set_value(GTK_RANGE(hscale_slider_brightness), Brightness_Level - 100);
	check_options_grayscale = lookup_widget(ca, "check_options_grayscale");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_options_grayscale), Greyscale);
	check_options_inverted = lookup_widget(ca, "check_options_inverted");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_options_inverted), Invert_Color);
	
	// Show the Color Adjust window.
	gtk_widget_show_all(ca);
}


/**
 * CA_Save(): Save the settings.
 */
void CA_Save(void)
{
	GtkWidget *hscale_slider_contrast, *hscale_slider_brightness;
	GtkWidget *check_options_grayscale, *check_options_inverted;
	
	// Save settings.
	hscale_slider_contrast = lookup_widget(color_adjust_window, "hscale_slider_contrast");
	Contrast_Level = gtk_range_get_value(GTK_RANGE(hscale_slider_contrast)) + 100;
	hscale_slider_brightness = lookup_widget(color_adjust_window, "hscale_slider_brightness");
	Brightness_Level = gtk_range_get_value(GTK_RANGE(hscale_slider_brightness)) + 100;
	check_options_grayscale = lookup_widget(color_adjust_window, "check_options_grayscale");
	Greyscale = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_options_grayscale));
	check_options_inverted = lookup_widget(color_adjust_window, "check_options_inverted");
	Invert_Color = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_options_inverted));
	
	// Recalculate palettes.
	Recalculate_Palettes();
	if (Genesis_Started || _32X_Started || SegaCD_Started)
	{
		// Emulation is running. Update the CRAM.
		CRam_Flag = 1;
		if (!Paused)
			Update_Emulation_One();
	}
}
