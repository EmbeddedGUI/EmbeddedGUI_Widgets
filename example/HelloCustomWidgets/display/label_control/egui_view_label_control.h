#ifndef _HELLO_CUSTOM_WIDGETS_LABEL_CONTROL_H_
#define _HELLO_CUSTOM_WIDGETS_LABEL_CONTROL_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_LABEL_CONTROL_MAX_TEXT_LEN 31
#define EGUI_VIEW_LABEL_CONTROL_MAX_HINT_LEN 31
#define EGUI_VIEW_LABEL_CONTROL_ACCESS_NONE  0xFF

typedef struct egui_view_label_control egui_view_label_control_t;
struct egui_view_label_control
{
    egui_view_t base;
    const egui_font_t *text_font;
    const egui_font_t *hint_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t hint_color;
    egui_color_t accent_color;
    egui_color_t required_color;
    char text[EGUI_VIEW_LABEL_CONTROL_MAX_TEXT_LEN + 1];
    char target_hint[EGUI_VIEW_LABEL_CONTROL_MAX_HINT_LEN + 1];
    uint8_t align_type;
    uint8_t access_key_index;
    uint8_t required;
    uint8_t target_highlighted;
};

void egui_view_label_control_init(egui_view_t *self);
void egui_view_label_control_set_text(egui_view_t *self, const char *text);
const char *egui_view_label_control_get_text(egui_view_t *self);
void egui_view_label_control_set_target_hint(egui_view_t *self, const char *hint);
const char *egui_view_label_control_get_target_hint(egui_view_t *self);
void egui_view_label_control_set_fonts(egui_view_t *self, const egui_font_t *text_font, const egui_font_t *hint_font);
void egui_view_label_control_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                         egui_color_t text_color, egui_color_t hint_color, egui_color_t accent_color,
                                         egui_color_t required_color);
void egui_view_label_control_set_align_type(egui_view_t *self, uint8_t align_type);
uint8_t egui_view_label_control_get_align_type(egui_view_t *self);
void egui_view_label_control_set_access_key_index(egui_view_t *self, uint8_t access_key_index);
uint8_t egui_view_label_control_get_access_key_index(egui_view_t *self);
void egui_view_label_control_set_required(egui_view_t *self, uint8_t required);
uint8_t egui_view_label_control_get_required(egui_view_t *self);
void egui_view_label_control_set_target_highlighted(egui_view_t *self, uint8_t target_highlighted);
uint8_t egui_view_label_control_get_target_highlighted(egui_view_t *self);
void egui_view_label_control_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_LABEL_CONTROL_H_ */
