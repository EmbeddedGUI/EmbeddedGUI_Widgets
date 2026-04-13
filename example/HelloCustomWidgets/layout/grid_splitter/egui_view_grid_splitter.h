#ifndef _EGUI_VIEW_GRID_SPLITTER_H_
#define _EGUI_VIEW_GRID_SPLITTER_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_GRID_SPLITTER_MAX_SNAPSHOTS 6

#define EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_MIN     20
#define EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_MAX     80
#define EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_DEFAULT 50
#define EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_STEP    5

#define EGUI_VIEW_GRID_SPLITTER_EMPHASIS_LEFT  0
#define EGUI_VIEW_GRID_SPLITTER_EMPHASIS_RIGHT 1
#define EGUI_VIEW_GRID_SPLITTER_EMPHASIS_NONE  2

typedef struct egui_view_grid_splitter_snapshot egui_view_grid_splitter_snapshot_t;
struct egui_view_grid_splitter_snapshot
{
    const char *header;
    const char *title;
    const char *summary;
    const char *left_title;
    const char *left_meta;
    const char *left_body;
    const char *right_title;
    const char *right_meta;
    const char *right_body;
    const char *footer;
    uint8_t split_ratio;
    uint8_t emphasis;
};

typedef void (*egui_view_on_grid_splitter_ratio_changed_listener_t)(egui_view_t *self, uint8_t snapshot_index, uint8_t split_ratio);

typedef struct egui_view_grid_splitter egui_view_grid_splitter_t;
struct egui_view_grid_splitter
{
    egui_view_t base;
    const egui_view_grid_splitter_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_grid_splitter_ratio_changed_listener_t on_ratio_changed;
    egui_color_t surface_color;
    egui_color_t section_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t split_ratio;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_handle;
    uint8_t handle_dragging;
};

void egui_view_grid_splitter_init(egui_view_t *self);
void egui_view_grid_splitter_set_snapshots(egui_view_t *self, const egui_view_grid_splitter_snapshot_t *snapshots, uint8_t snapshot_count);
void egui_view_grid_splitter_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index);
uint8_t egui_view_grid_splitter_get_current_snapshot(egui_view_t *self);
void egui_view_grid_splitter_set_split_ratio(egui_view_t *self, uint8_t split_ratio);
uint8_t egui_view_grid_splitter_get_split_ratio(egui_view_t *self);
void egui_view_grid_splitter_set_on_ratio_changed_listener(egui_view_t *self,
                                                           egui_view_on_grid_splitter_ratio_changed_listener_t listener);
void egui_view_grid_splitter_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_grid_splitter_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_grid_splitter_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_grid_splitter_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_grid_splitter_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                         egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color);
uint8_t egui_view_grid_splitter_get_handle_region(egui_view_t *self, egui_region_t *region);
void egui_view_grid_splitter_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_GRID_SPLITTER_H_ */
