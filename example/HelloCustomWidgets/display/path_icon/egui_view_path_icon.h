#ifndef _EGUI_VIEW_PATH_ICON_H_
#define _EGUI_VIEW_PATH_ICON_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    EGUI_VIEW_PATH_ICON_COMMAND_MOVE_TO = 0,
    EGUI_VIEW_PATH_ICON_COMMAND_LINE_TO,
    EGUI_VIEW_PATH_ICON_COMMAND_QUAD_TO,
    EGUI_VIEW_PATH_ICON_COMMAND_CUBIC_TO,
    EGUI_VIEW_PATH_ICON_COMMAND_CLOSE,
} egui_view_path_icon_command_type_t;

typedef struct egui_view_path_icon_command egui_view_path_icon_command_t;
struct egui_view_path_icon_command
{
    uint8_t type;
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
    int16_t x3;
    int16_t y3;
};

typedef struct egui_view_path_icon_data egui_view_path_icon_data_t;
struct egui_view_path_icon_data
{
    uint16_t viewport_width;
    uint16_t viewport_height;
    uint8_t command_count;
    const egui_view_path_icon_command_t *commands;
};

typedef struct egui_view_path_icon egui_view_path_icon_t;
struct egui_view_path_icon
{
    egui_view_t base;
    const egui_view_path_icon_data_t *data;
    egui_color_t icon_color;
};

void egui_view_path_icon_init(egui_view_t *self);
void egui_view_path_icon_apply_standard_style(egui_view_t *self);
void egui_view_path_icon_apply_subtle_style(egui_view_t *self);
void egui_view_path_icon_apply_accent_style(egui_view_t *self);
void egui_view_path_icon_set_data(egui_view_t *self, const egui_view_path_icon_data_t *data);
const egui_view_path_icon_data_t *egui_view_path_icon_get_data(egui_view_t *self);
const egui_view_path_icon_data_t *egui_view_path_icon_get_bookmark_data(void);
const egui_view_path_icon_data_t *egui_view_path_icon_get_heart_data(void);
const egui_view_path_icon_data_t *egui_view_path_icon_get_send_data(void);
void egui_view_path_icon_set_palette(egui_view_t *self, egui_color_t icon_color);
void egui_view_path_icon_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_PATH_ICON_H_ */
