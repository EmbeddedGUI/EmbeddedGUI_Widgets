#ifndef _HELLO_CUSTOM_WIDGETS_TAG_H_
#define _HELLO_CUSTOM_WIDGETS_TAG_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_TAG_MAX_TEXT_LEN           31
#define EGUI_VIEW_TAG_MAX_SECONDARY_TEXT_LEN 23

typedef void (*egui_view_on_tag_dismiss_listener_t)(egui_view_t *self);

typedef struct egui_view_tag egui_view_tag_t;
struct egui_view_tag
{
    egui_view_t base;
    const egui_font_t *font;
    const egui_font_t *secondary_font;
    const egui_font_t *icon_font;
    egui_view_on_tag_dismiss_listener_t on_dismiss;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t secondary_color;
    egui_color_t accent_color;
    char text[EGUI_VIEW_TAG_MAX_TEXT_LEN + 1];
    char secondary_text[EGUI_VIEW_TAG_MAX_SECONDARY_TEXT_LEN + 1];
    uint8_t dismissible;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t dismiss_pressed;
};

void egui_view_tag_init(egui_view_t *self);
void egui_view_tag_apply_standard_style(egui_view_t *self);
void egui_view_tag_apply_compact_style(egui_view_t *self);
void egui_view_tag_apply_read_only_style(egui_view_t *self);
void egui_view_tag_set_text(egui_view_t *self, const char *text);
void egui_view_tag_set_secondary_text(egui_view_t *self, const char *text);
void egui_view_tag_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_tag_set_secondary_font(egui_view_t *self, const egui_font_t *font);
void egui_view_tag_set_icon_font(egui_view_t *self, const egui_font_t *font);
void egui_view_tag_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                               egui_color_t secondary_color, egui_color_t accent_color);
void egui_view_tag_set_dismissible(egui_view_t *self, uint8_t dismissible);
void egui_view_tag_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_tag_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_tag_set_on_dismiss_listener(egui_view_t *self, egui_view_on_tag_dismiss_listener_t listener);
uint8_t egui_view_tag_get_dismiss_region(egui_view_t *self, egui_region_t *region);
void egui_view_tag_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_TAG_H_ */
