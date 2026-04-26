#ifndef _EGUI_VIEW_COMPOUND_BUTTON_H_
#define _EGUI_VIEW_COMPOUND_BUTTON_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_COMPOUND_BUTTON_STYLE_DEFAULT 0
#define EGUI_VIEW_COMPOUND_BUTTON_STYLE_PRIMARY 1
#define EGUI_VIEW_COMPOUND_BUTTON_STYLE_SUBTLE  2

#define EGUI_VIEW_COMPOUND_BUTTON_TARGET_NONE 0
#define EGUI_VIEW_COMPOUND_BUTTON_TARGET_BODY 1

typedef void (*egui_view_compound_button_action_listener_t)(egui_view_t *self);

typedef struct egui_view_compound_button egui_view_compound_button_t;
struct egui_view_compound_button
{
    egui_view_t base;
    const char *title;
    const char *subtitle;
    const char *icon;
    const egui_font_t *title_font;
    const egui_font_t *subtitle_font;
    const egui_font_t *icon_font;
    egui_view_compound_button_action_listener_t on_action;
    egui_color_t surface_color;
    egui_color_t pressed_color;
    egui_color_t border_color;
    egui_color_t focus_color;
    egui_color_t title_color;
    egui_color_t subtitle_color;
    egui_color_t icon_color;
    uint8_t style;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_target;
};

void egui_view_compound_button_init(egui_view_t *self);
void egui_view_compound_button_set_content(egui_view_t *self, const char *title, const char *subtitle, const char *icon);
void egui_view_compound_button_set_title(egui_view_t *self, const char *title);
void egui_view_compound_button_set_subtitle(egui_view_t *self, const char *subtitle);
void egui_view_compound_button_set_icon(egui_view_t *self, const char *icon);
void egui_view_compound_button_set_fonts(egui_view_t *self, const egui_font_t *title_font, const egui_font_t *subtitle_font, const egui_font_t *icon_font);
void egui_view_compound_button_set_style(egui_view_t *self, uint8_t style);
void egui_view_compound_button_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_compound_button_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_compound_button_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t pressed_color, egui_color_t border_color,
                                           egui_color_t focus_color, egui_color_t title_color, egui_color_t subtitle_color, egui_color_t icon_color);
void egui_view_compound_button_set_on_action_listener(egui_view_t *self, egui_view_compound_button_action_listener_t listener);
uint8_t egui_view_compound_button_activate(egui_view_t *self);
uint8_t egui_view_compound_button_get_button_region(egui_view_t *self, egui_region_t *region);
void egui_view_compound_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_COMPOUND_BUTTON_H_ */
