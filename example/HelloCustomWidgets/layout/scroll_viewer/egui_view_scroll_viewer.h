#ifndef _EGUI_VIEW_SCROLL_VIEWER_H_
#define _EGUI_VIEW_SCROLL_VIEWER_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_SCROLL_VIEWER_MAX_BLOCKS    4
#define EGUI_VIEW_SCROLL_VIEWER_MAX_SNAPSHOTS 6

#define EGUI_VIEW_SCROLL_VIEWER_TONE_ACCENT  0
#define EGUI_VIEW_SCROLL_VIEWER_TONE_SUCCESS 1
#define EGUI_VIEW_SCROLL_VIEWER_TONE_WARNING 2
#define EGUI_VIEW_SCROLL_VIEWER_TONE_NEUTRAL 3

#define EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE 0
#define EGUI_VIEW_SCROLL_VIEWER_PART_THUMB   1
#define EGUI_VIEW_SCROLL_VIEWER_PART_TRACK   2
#define EGUI_VIEW_SCROLL_VIEWER_PART_NONE    0xFF

typedef struct egui_view_scroll_viewer_block egui_view_scroll_viewer_block_t;
struct egui_view_scroll_viewer_block
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

typedef struct egui_view_scroll_viewer_snapshot egui_view_scroll_viewer_snapshot_t;
struct egui_view_scroll_viewer_snapshot
{
    const char *eyebrow;
    const char *title;
    const char *summary;
    const char *helper;
    const char *footer;
    const egui_view_scroll_viewer_block_t *blocks;
    uint8_t block_count;
    egui_dim_t content_width;
    egui_dim_t content_height;
    egui_dim_t viewport_width;
    egui_dim_t viewport_height;
    egui_dim_t vertical_offset;
    egui_dim_t horizontal_offset;
    uint8_t show_scrollbar;
};

typedef void (*egui_view_on_scroll_viewer_changed_listener_t)(egui_view_t *self, uint8_t snapshot_index, egui_dim_t vertical_offset,
                                                              egui_dim_t horizontal_offset, uint8_t part);

typedef struct egui_view_scroll_viewer egui_view_scroll_viewer_t;
struct egui_view_scroll_viewer
{
    egui_view_t base;
    const egui_view_scroll_viewer_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_scroll_viewer_changed_listener_t on_view_changed;
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
    uint8_t scrollbar_visibility;
    uint8_t pressed_part;
    uint8_t thumb_dragging;
    uint8_t track_direction;
    egui_dim_t vertical_offset;
    egui_dim_t horizontal_offset;
    egui_dim_t drag_anchor_y;
    egui_dim_t drag_anchor_offset;
};

void egui_view_scroll_viewer_init(egui_view_t *self);
void egui_view_scroll_viewer_set_snapshots(egui_view_t *self, const egui_view_scroll_viewer_snapshot_t *snapshots, uint8_t snapshot_count);
void egui_view_scroll_viewer_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index);
uint8_t egui_view_scroll_viewer_get_current_snapshot(egui_view_t *self);
void egui_view_scroll_viewer_set_vertical_offset(egui_view_t *self, egui_dim_t offset);
egui_dim_t egui_view_scroll_viewer_get_vertical_offset(egui_view_t *self);
egui_dim_t egui_view_scroll_viewer_get_max_vertical_offset(egui_view_t *self);
void egui_view_scroll_viewer_set_horizontal_offset(egui_view_t *self, egui_dim_t offset);
egui_dim_t egui_view_scroll_viewer_get_horizontal_offset(egui_view_t *self);
egui_dim_t egui_view_scroll_viewer_get_max_horizontal_offset(egui_view_t *self);
void egui_view_scroll_viewer_scroll_line(egui_view_t *self, egui_dim_t delta_lines);
void egui_view_scroll_viewer_scroll_page(egui_view_t *self, egui_dim_t delta_pages);
void egui_view_scroll_viewer_set_scrollbar_visibility(egui_view_t *self, uint8_t visible);
uint8_t egui_view_scroll_viewer_get_scrollbar_visibility(egui_view_t *self);
void egui_view_scroll_viewer_set_on_view_changed_listener(egui_view_t *self, egui_view_on_scroll_viewer_changed_listener_t listener);
void egui_view_scroll_viewer_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_scroll_viewer_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_scroll_viewer_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_scroll_viewer_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_scroll_viewer_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t viewport_color,
                                         egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                         egui_color_t preview_color);
uint8_t egui_view_scroll_viewer_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region);
uint8_t egui_view_scroll_viewer_handle_navigation_key(egui_view_t *self, uint8_t key_code);
void egui_view_scroll_viewer_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_SCROLL_VIEWER_H_ */
