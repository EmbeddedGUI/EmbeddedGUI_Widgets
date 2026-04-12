#ifndef _EGUI_VIEW_THUMB_RATE_H_
#define _EGUI_VIEW_THUMB_RATE_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_THUMB_RATE_STATE_NONE      0
#define EGUI_VIEW_THUMB_RATE_STATE_LIKED     1
#define EGUI_VIEW_THUMB_RATE_STATE_DISLIKED  2

#define EGUI_VIEW_THUMB_RATE_PART_LIKE     1
#define EGUI_VIEW_THUMB_RATE_PART_DISLIKE  2
#define EGUI_VIEW_THUMB_RATE_PART_NONE     0xFF

typedef void (*egui_view_on_thumb_rate_changed_listener_t)(egui_view_t *self, uint8_t state, uint8_t part);

typedef struct egui_view_thumb_rate egui_view_thumb_rate_t;
struct egui_view_thumb_rate
{
    egui_view_t base;
    const egui_font_t *font;
    egui_view_on_thumb_rate_changed_listener_t on_changed;
    const char *like_label;
    const char *dislike_label;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t like_color;
    egui_color_t dislike_color;
    egui_color_t shadow_color;
    uint8_t current_state;
    uint8_t current_part;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_part;
};

void egui_view_thumb_rate_init(egui_view_t *self);
void egui_view_thumb_rate_apply_standard_style(egui_view_t *self);
void egui_view_thumb_rate_apply_compact_style(egui_view_t *self);
void egui_view_thumb_rate_apply_read_only_style(egui_view_t *self);
void egui_view_thumb_rate_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_thumb_rate_set_labels(egui_view_t *self, const char *like_label, const char *dislike_label);
void egui_view_thumb_rate_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                      egui_color_t muted_text_color, egui_color_t like_color, egui_color_t dislike_color, egui_color_t shadow_color);
void egui_view_thumb_rate_set_state(egui_view_t *self, uint8_t state);
uint8_t egui_view_thumb_rate_get_state(egui_view_t *self);
void egui_view_thumb_rate_set_current_part(egui_view_t *self, uint8_t part);
uint8_t egui_view_thumb_rate_get_current_part(egui_view_t *self);
void egui_view_thumb_rate_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_thumb_rate_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_thumb_rate_set_on_changed_listener(egui_view_t *self, egui_view_on_thumb_rate_changed_listener_t listener);
uint8_t egui_view_thumb_rate_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region);
uint8_t egui_view_thumb_rate_handle_navigation_key(egui_view_t *self, uint8_t key_code);
void egui_view_thumb_rate_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_THUMB_RATE_H_ */
