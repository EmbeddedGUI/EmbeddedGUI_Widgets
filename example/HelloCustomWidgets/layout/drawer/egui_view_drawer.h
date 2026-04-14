#ifndef _EGUI_VIEW_DRAWER_H_
#define _EGUI_VIEW_DRAWER_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_DRAWER_ANCHOR_START 0
#define EGUI_VIEW_DRAWER_ANCHOR_END   1

#define EGUI_VIEW_DRAWER_MODE_INLINE  0
#define EGUI_VIEW_DRAWER_MODE_OVERLAY 1

#define EGUI_VIEW_DRAWER_PART_NONE   0
#define EGUI_VIEW_DRAWER_PART_TOGGLE 1
#define EGUI_VIEW_DRAWER_PART_CLOSE  2

typedef void (*egui_view_on_drawer_open_changed_listener_t)(egui_view_t *self, uint8_t is_open);

typedef struct egui_view_drawer egui_view_drawer_t;
struct egui_view_drawer
{
    egui_view_t base;
    const char *eyebrow;
    const char *title;
    const char *body_primary;
    const char *body_secondary;
    const char *footer;
    const char *tag;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_drawer_open_changed_listener_t on_open_changed;
    egui_color_t surface_color;
    egui_color_t overlay_color;
    egui_color_t border_color;
    egui_color_t section_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t shadow_color;
    uint8_t anchor;
    uint8_t presentation_mode;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t open;
    uint8_t pressed_part;
};

void egui_view_drawer_init(egui_view_t *self);
void egui_view_drawer_set_eyebrow(egui_view_t *self, const char *text);
void egui_view_drawer_set_title(egui_view_t *self, const char *text);
void egui_view_drawer_set_body_primary(egui_view_t *self, const char *text);
void egui_view_drawer_set_body_secondary(egui_view_t *self, const char *text);
void egui_view_drawer_set_footer(egui_view_t *self, const char *text);
void egui_view_drawer_set_tag(egui_view_t *self, const char *text);
void egui_view_drawer_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_drawer_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_drawer_set_anchor(egui_view_t *self, uint8_t anchor);
uint8_t egui_view_drawer_get_anchor(egui_view_t *self);
void egui_view_drawer_set_presentation_mode(egui_view_t *self, uint8_t presentation_mode);
uint8_t egui_view_drawer_get_presentation_mode(egui_view_t *self);
void egui_view_drawer_set_open(egui_view_t *self, uint8_t is_open);
uint8_t egui_view_drawer_get_open(egui_view_t *self);
void egui_view_drawer_set_on_open_changed_listener(egui_view_t *self, egui_view_on_drawer_open_changed_listener_t listener);
void egui_view_drawer_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_drawer_get_compact_mode(egui_view_t *self);
void egui_view_drawer_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_drawer_get_read_only_mode(egui_view_t *self);
void egui_view_drawer_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t overlay_color, egui_color_t border_color,
                                  egui_color_t section_color, egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                  egui_color_t shadow_color);
uint8_t egui_view_drawer_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region);
void egui_view_drawer_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_DRAWER_H_ */
