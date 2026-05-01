#ifndef _HELLO_CUSTOM_WIDGETS_PATH_H_
#define _HELLO_CUSTOM_WIDGETS_PATH_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    EGUI_VIEW_PATH_COMMAND_MOVE_TO = 0,
    EGUI_VIEW_PATH_COMMAND_LINE_TO,
    EGUI_VIEW_PATH_COMMAND_QUAD_TO,
    EGUI_VIEW_PATH_COMMAND_CUBIC_TO,
    EGUI_VIEW_PATH_COMMAND_CLOSE,
} egui_view_path_command_type_t;

typedef struct egui_view_path_command egui_view_path_command_t;
struct egui_view_path_command
{
    uint8_t type;
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
    int16_t x3;
    int16_t y3;
};

typedef struct egui_view_path_data egui_view_path_data_t;
struct egui_view_path_data
{
    uint16_t viewport_width;
    uint16_t viewport_height;
    uint8_t command_count;
    const egui_view_path_command_t *commands;
};

typedef struct egui_view_path egui_view_path_t;
struct egui_view_path
{
    egui_view_t base;
    const egui_view_path_data_t *data;
    egui_color_t fill_color;
    egui_color_t stroke_color;
    egui_color_t accent_color;
    egui_dim_t stroke_width;
};

void egui_view_path_init(egui_view_t *self);
void egui_view_path_set_data(egui_view_t *self, const egui_view_path_data_t *data);
const egui_view_path_data_t *egui_view_path_get_data(egui_view_t *self);
const egui_view_path_data_t *egui_view_path_get_shield_data(void);
const egui_view_path_data_t *egui_view_path_get_curve_data(void);
const egui_view_path_data_t *egui_view_path_get_line_data(void);
const egui_view_path_data_t *egui_view_path_get_bookmark_data(void);
void egui_view_path_set_palette(egui_view_t *self, egui_color_t fill_color, egui_color_t stroke_color, egui_color_t accent_color);
void egui_view_path_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width);
egui_dim_t egui_view_path_get_stroke_width(egui_view_t *self);
void egui_view_path_apply_standard_style(egui_view_t *self);
void egui_view_path_apply_accent_style(egui_view_t *self);
void egui_view_path_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_PATH_H_ */
