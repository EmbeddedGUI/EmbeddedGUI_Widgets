#ifndef _HELLO_CUSTOM_WIDGETS_COUNTER_BADGE_H_
#define _HELLO_CUSTOM_WIDGETS_COUNTER_BADGE_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct egui_view_counter_badge egui_view_counter_badge_t;
struct egui_view_counter_badge
{
    egui_view_t base;
    uint16_t count;
    uint8_t max_display;
    egui_color_t badge_color;
    egui_color_t text_color;
    const egui_font_t *font;
    uint8_t content_style;
    const char *icon;
    const egui_font_t *icon_font;
    char text_buffer[8];
    egui_color_t outline_color;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t dot_mode;
};

void egui_view_counter_badge_init(egui_view_t *self);
void egui_view_counter_badge_set_count(egui_view_t *self, uint16_t count);
uint16_t egui_view_counter_badge_get_count(egui_view_t *self);
void egui_view_counter_badge_set_max_display(egui_view_t *self, uint8_t max_display);
uint8_t egui_view_counter_badge_get_max_display(egui_view_t *self);
void egui_view_counter_badge_set_dot_mode(egui_view_t *self, uint8_t dot_mode);
uint8_t egui_view_counter_badge_get_dot_mode(egui_view_t *self);
void egui_view_counter_badge_set_palette(egui_view_t *self, egui_color_t badge_color, egui_color_t text_color, egui_color_t outline_color);
void egui_view_counter_badge_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_counter_badge_get_compact_mode(egui_view_t *self);
void egui_view_counter_badge_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_counter_badge_get_read_only_mode(egui_view_t *self);
uint8_t egui_view_counter_badge_get_badge_region(egui_view_t *self, egui_region_t *region);
void egui_view_counter_badge_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_COUNTER_BADGE_H_ */
