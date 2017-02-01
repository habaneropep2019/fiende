#include <config.h>

#include <libgnomeprint/gnome-print-pclc.h>

#define _PCL_DPI_TEMP 300

/* This is the structure that will be provided TO the driver
   by gnome-print, and will be filled in this order :

1.  gnome-print will call PCL_create_jobdata()
    and the default values will be loaded.
2.  gnome-print will change the default values with values
    specific to that printer.
3.  gnome-print will the modify the values that the user
    has configured for that printer.
4.  The application and the print dialog will load the
    job specific parameters */

void
pclc_new_job_data (void)
{
/* TODO : Divide this data in what is specific to this driver
   and what is common across all drivers. Chema */
	jobdata = g_new0 (PCLJobData, 1);

	jobdata->uel = TRUE;
/* TRUE = the printer understands the Universal Exit Languaje command
   most LJ printers do except for the III IIID & IIIP. */

	jobdata->margin_top = 1/6;
	jobdata->margin_bottom = 1/6;
	jobdata->margin_left = 1/6;
	jobdata->margin_right = 1/6;
/* Printable Area Margins in inches. ([1] Page 2.7.) This seems way too optimistic
   for Lasers. For Deskjet margins are greater. Need to test in the field. */

	jobdata->paper_run_dpi   = _PCL_DPI_TEMP;
	jobdata->paper_width_dpi = _PCL_DPI_TEMP;
	jobdata->x_dpi=jobdata->paper_width_dpi;
	jobdata->y_dpi=jobdata->paper_run_dpi;
/* Do not use xdpi and ydpi, run and width are clear because it's independent of
   orientation, we should load xdip and ydpi with corresponding values */

	jobdata->simplex_duplex = -1;
/* -1   = printer does not supports it,
    0   = printer supports it but no duplex has been chosen
    1,2 = duplex printing ([1]Page 4-5) long and short edge
          long edge is NOT supported by all duplex capable printers*/

	jobdata->paper_size_capable=FALSE;
/* Althou most printers do, the default is that this printer does
   not understand the paper_size Escape code. Most profiles should
   have this enabled, since most printers can select paper size*/

	jobdata->paper_size = 2;
/* Letter is default, maybe we should fall bacj to gnome_print_default_paper.
   For printers that support custom paper sizes we can choose any size */

        jobdata->media_type = -1;
/* -1 = Printer does not understand the mediatype command.
   other = mediatype.
   LaserJets do not support media type [3]Page34 */

	jobdata->orientation = 0;
/* 0 = Portrait
   1 = Landscape
   2 = Reverse Portrait
   3 = Reverse Landscape */

#ifdef ENABLE_COLORS
        jobdata->color_mode = PCL_COLOR_MODE_RGB;
	jobdata->number_of_planes = 3;
#else	
        jobdata->color_mode = PCL_COLOR_MODE_BLACK;
	jobdata->number_of_planes = 1;
#endif	
/* See enum above */
/* Number of planes or colors. 1 for black
   and 3 for RGB */
	
	jobdata->calibrate = TRUE;
/* The default for calibration if off, but we
   still need to load the calibration parameters.
   The calibration parameters will range from 1 to 200
   with 100 beeing the default, we might want to change
   this latter*/
	jobdata->calibration_brightness=100;
	jobdata->calibration_contrast=100;
	jobdata->calibration_red=100;
	jobdata->calibration_green=100;
	jobdata->calibration_blue=100;
	jobdata->calibration_gamma=100;
	jobdata->calibration_density=100;
	jobdata->calibration_saturation=100;

/* The end raster graphics command  was changed for some stupid
   and unkown reason, I am not aware of any printers that NEED
   to use the new method. Set this to True if needed. Chema */
	jobdata->new_end_graphics_method=FALSE;
}                                                  

void
pclc_dump_structure_values_to_console (void)
{
	/* We print all the parameters as a debuging tool ....*/
	g_print("Here are the parameters for this print job :\n");

	if (jobdata->uel)
		g_print("This printer DOES understand UEL\n");
	else
		g_print("This printer DOES NOT understands UEL\n");

	switch (jobdata->simplex_duplex){
	case -1:
		g_print("This printer does not support Duplexing\n");
		break;
	case 0:
		g_print("This printer supports duplexing but it is not needed\n");
		break;
	case 1:
	case 2:
		g_print("This job will be printed with duplexing\n");
		break;
	}

	if ((jobdata->orientation%2)==0)
		g_print("Portrait or reverse portrait orientation\n");
	else
		g_print("Landscape or reverse Landscape  orientation\n");

	if (jobdata->paper_size_capable)
		g_print("You can't select paper size for this printer\n");
	else
		g_print("The paper size selcted (in HPL numbers) is : %i\n", jobdata->paper_size);
	
	if (jobdata->media_type == -1)
		g_print("You can't select media type for this printer\n");
	else
		g_print("The media type selected is number : %i\n", jobdata->media_type);

	switch (jobdata->color_mode){
	case PCL_COLOR_MODE_BLACK:
		g_print("Color mode : black\n");
		break;
	case PCL_COLOR_MODE_CMY:
		g_print("Color mode : CMY\n");
		break;
	case PCL_COLOR_MODE_CMYK:
		g_print("Color mode : CMYK\n");
		break;
	default:
		g_print("Color mode undefined\n");
		break;
	}

	g_print("Reslution is : %d in paper direction and %d in paper width\n",
		jobdata->paper_run_dpi, jobdata->paper_width_dpi);

	if (jobdata->calibrate == TRUE)
	{
		g_print("The calibration parameters have been set as follows :\n");
		g_print("Brightness : %f\n", jobdata->calibration_brightness);
		g_print("Contrast   : %f\n", jobdata->calibration_contrast);
		g_print("Red        : %f\n", jobdata->calibration_red);
		g_print("Green      : %f\n", jobdata->calibration_green);
		g_print("Blue       : %f\n", jobdata->calibration_blue);
		g_print("Gamma      : %f\n", jobdata->calibration_gamma);
		g_print("Density    : %f\n", jobdata->calibration_density);
		g_print("Saturation : %f\n", jobdata->calibration_saturation);
	}
	else
		g_print("No calibration parameters can be set for this printer\n");
	
	g_print("\n\n");
}
