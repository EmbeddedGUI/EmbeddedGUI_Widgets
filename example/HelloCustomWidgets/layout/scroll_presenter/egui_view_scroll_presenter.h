#ifndef _EGUI_VIEW_SCROLL_PRESENTER_H_
#define _EGUI_VIEW_SCROLL_PRESENTER_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_SCROLL_PRESENTER_MAX_ITEMS     5
#define EGUI_VIEW_SCROLL_PRESENTER_MAX_SNAPSHOTS 6

#define EGUI_VIEW_SCROLL_PRESENTER_TONE_ACCENT  0
#define EGUI_VIEW_SCROLL_PRESENTER_TONE_SUCCESS 1
#define EGUI_VIEW_SCROLL_PRESENTER_TONE_WARNING 2
#define EGUI_VIEW_SCROLL_PRESENTER_TONE_NEUTRAL 3

#define EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE 0
#define EGUI_VIEW_SCROLL_PRESENTER_PART_NONE    0xFF

typedef struct egui_view_scroll_presenter_item egui_view_scroll_presenter_item_t;
struct egui_view_scroll_presenter_item
{
    const char *badge;
    const char *title;
    const char *meta;
    egui_dim_t origin_x;
    egui_dim_t origin_y;
    egui_dim_t width;
    egui_dim_t height;
    uint8_t tone;
    uint8_t emphasized;
};

typedef struct egui_view_scroll_presenter_snapshot egui_view_scroll_presenter_snapshot_t;
struct egui_view_scroll_presenter_snapshot
{
    const char *eyebrow;
    const char *title;
    const char *summary;
    const char *helper;
    const char *footer;
    const egui_view_scroll_presenter_item_t *items;
    uint8_t item_count;
    egui_dim_t content_width;
    egui_dim_t content_height;
    egui_dim_t viewport_width;
    egui_dim_t viewport_height;
    egui_dim_t vertical_offset;
    egui_dim_t horizontal_offset;
};

typedef void (*egui_view_on_scroll_presenter_changed_listener_t)(egui_view_t *self, uint8_t snapshot_index, egui_dim_t vertical_offset,
                                                                 egui_dim_t horizontal_offset, uint8_t part);

typedef struct egui_view_scroll_presenter egui_view_scroll_presenter_t;
struct egui_view_scroll_presenter
{
    egui_view_t base;
    const egui_view_scroll_presenter_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_scroll_presenter_changed_listener_t on_view_changed;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t viewport_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t preview_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_part;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_part;
    uint8_t surface_dragging;
    egui_dim_t vertical_offset;
    egui_dim_t horizontal_offset;
    egui_dim_t drag_anchor_x;
    egui_dim_t drag_anchor_y;
    egui_dim_t drag_anchor_vertical_offset;
    egui_dim_t drag_anchor_horizontal_offset;
};

void egui_view_scroll_presenter_init(egui_view_t *self);
void egui_view_scroll_presenter_set_snapshots(egui_view_t *self, const egui_view_scroll_presenter_snapshot_t *snapshots, uint8_t snapshot_count);
void egui_view_scroll_presenter_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index);
uint8_t egui_view_scroll_presenter_get_current_snapshot(egui_view_t *self);
void egui_view_scroll_presenter_set_vertical_offset(egui_view_t *self, egui_dim_t offset);
egui_dim_t egui_view_scroll_presenter_get_vertical_offset(egui_view_t *self);
egui_dim_t egui_view_scroll_presenter_get_max_vertical_offset(egui_view_t *self);
void egui_view_scroll_presenter_set_horizontal_offset(egui_view_t *self, egui_dim_t offset);
egui_dim_t egui_view_scroll_presenter_get_horizontal_offset(egui_view_t *self);
egui_dim_t egui_view_scroll_presenter_get_max_horizontal_offset(egui_view_t *self);
void egui_view_scroll_presenter_scroll_line(egui_view_t *self, egui_dim_t delta_lines);
void egui_view_scroll_presenter_scroll_page(egui_view_t *self, egui_dim_t delta_pages);
void egui_view_scroll_presenter_activate_current_snapshot(egui_view_t *self);
void egui_view_scroll_presenter_set_on_view_changed_listener(egui_view_t *self, egui_view_on_scroll_presenter_changed_listener_t listener);
void egui_view_scroll_presenter_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_scroll_presenter_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_scroll_presenter_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_scroll_presenter_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_scroll_presenter_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                            egui_color_t viewport_color, egui_color_t text_color, egui_color_t muted_text_color,
                                            egui_color_t accent_color, egui_color_t preview_color);
uint8_t egui_view_scroll_presenter_get_surface_region(egui_view_t *self, egui_region_t *region);
uint8_t egui_view_scroll_presenter_handle_navigation_key(egui_view_t *self, uint8_t key_code);
void egui_view_scroll_presenter_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_SCROLL_PRESENTER_H_ */
