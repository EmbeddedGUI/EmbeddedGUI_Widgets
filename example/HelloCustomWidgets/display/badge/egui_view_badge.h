#ifndef _HELLO_CUSTOM_WIDGETS_BADGE_H_
#define _HELLO_CUSTOM_WIDGETS_BADGE_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_BADGE_MAX_TEXT_LEN 31

typedef struct egui_view_badge egui_view_badge_t;
struct egui_view_badge
{
    egui_view_t base;
    const egui_font_t *font;
    const egui_font_t *icon_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t accent_color;
    char text[EGUI_VIEW_BADGE_MAX_TEXT_LEN + 1];
    const char *icon;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t outline_mode;
    uint8_t subtle_mode;
};

void egui_view_badge_init(egui_view_t *self);
void egui_view_badge_apply_filled_style(egui_view_t *self);
void egui_view_badge_apply_outline_style(egui_view_t *self);
void egui_view_badge_apply_subtle_style(egui_view_t *self);
void egui_view_badge_apply_read_only_style(egui_view_t *self);
void egui_view_badge_set_text(egui_view_t *self, const char *text);
void egui_view_badge_set_icon(egui_view_t *self, const char *icon);
void egui_view_badge_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_badge_set_icon_font(egui_view_t *self, const egui_font_t *font);
void egui_view_badge_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color, egui_color_t accent_color);
void egui_view_badge_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_badge_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_badge_get_icon_region(egui_view_t *self, egui_region_t *region);
void egui_view_badge_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_BADGE_H_ */
