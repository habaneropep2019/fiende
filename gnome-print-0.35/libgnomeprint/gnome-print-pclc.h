/* -*- Mode: C; tab-width: 2; indent-tabs-mode: t; c-basic-offset: 2 -*- */
#ifndef __GNOME_PRINT_PCLC_H__
#define __GNOME_PRINT_PCLC_H__

#include <libgnomeprint/gnome-print.h>

BEGIN_GNOME_DECLS

typedef enum {
	PCL_COLOR_MODE_BLACK,
	PCL_COLOR_MODE_CMY,
	PCL_COLOR_MODE_CMYK,
	PCL_COLOR_MODE_RGB,
} PCLColorModes;

typedef enum {
	PCL_COMPRESSION_METHOD_NO_COMPRESSION, /* 0 */
	PCL_COMPRESSION_METHOD_RLC,            /* 1 */
	PCL_COMPRESSION_METHOD_TIFF,           /* 2 */
	PCL_COMPRESSION_METHOD_DROW,           /* 3 Delta row */
	PLC_COMPRESSION_METHOD_RESERVED_4,     /* 4 */
	PLC_COMPRESSION_METHOD_RESERVED_5,     /* 5 */
	PLC_COMPRESSION_METHOD_RESERVED_6,     /* 6 */
	PLC_COMPRESSION_METHOD_RESERVED_7,     /* 7 */
	PLC_COMPRESSION_METHOD_RESERVED_8,     /* 8 */
	PLC_COMPRESSION_METHOD_RESERVED_9,     /* 9 */
} PCLCompressionModes;

typedef struct _PCLJobData {
	gint uel;

	float margin_top;
	float margin_bottom;
	float margin_left;
	float margin_right;

	gint paper_run_dpi;
	gint paper_width_dpi;
	gint x_dpi;
	gint y_dpi;

	gint color_mode;
	gint simplex_duplex;
	gint paper_size_capable;
	gint paper_size;
	gint media_type;
	gint orientation;

	gint number_of_planes;
	
	gint new_end_graphics_method;

	gint  calibrate;
	float calibration_brightness;
	float calibration_contrast;
	float calibration_red;
	float calibration_green;
	float calibration_blue;
	float calibration_gamma;
	float calibration_density;
	float calibration_saturation;
} PCLJobData;

extern PCLJobData * jobdata;

void pclc_dump_structure_values_to_console (void);
void pclc_new_job_data (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

END_GNOME_DECLS

#endif /* __GNOME_PRINT_PCLC_H__ */



