#ifndef _EGUI_VIEW_DATA_GRID_H_
#define _EGUI_VIEW_DATA_GRID_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_DATA_GRID_MAX_SNAPSHOTS 6
#define EGUI_VIEW_DATA_GRID_MAX_ROWS      6
#define EGUI_VIEW_DATA_GRID_MAX_COLS      4
#define EGUI_VIEW_DATA_GRID_ROW_NONE      0xFF

#define EGUI_VIEW_DATA_GRID_ALIGN_LEFT   0
#define EGUI_VIEW_DATA_GRID_ALIGN_CENTER 1
#define EGUI_VIEW_DATA_GRID_ALIGN_RIGHT  2

#define EGUI_VIEW_DATA_GRID_TONE_ACCENT  0
#define EGUI_VIEW_DATA_GRID_TONE_SUCCESS 1
#define EGUI_VIEW_DATA_GRID_TONE_WARNING 2
#define EGUI_VIEW_DATA_GRID_TONE_NEUTRAL 3

typedef struct egui_view_data_grid_column egui_view_data_grid_column_t;
struct egui_view_data_grid_column
{
    const char *title;
    uint8_t align;
};

typedef struct egui_view_data_grid_row egui_view_data_grid_row_t;
struct egui_view_data_grid_row
{
    const char *cells[EGUI_VIEW_DATA_GRID_MAX_COLS];
    uint8_t tone;
    uint8_t emphasized;
};

typedef struct egui_view_data_grid_snapshot egui_view_data_grid_snapshot_t;
struct egui_view_data_grid_snapshot
{
    const char *header;
    const char *title;
    const char *summary;
    const char *footer;
    const egui_view_data_grid_column_t *columns;
    const egui_view_data_grid_row_t *rows;
    uint8_t column_count;
    uint8_t row_count;
    uint8_t selected_row;
};

typedef void (*egui_view_on_data_grid_action_listener_t)(egui_view_t *self, uint8_t snapshot_index, uint8_t row_index);

typedef struct egui_view_data_grid egui_view_data_grid_t;
struct egui_view_data_grid
{
    egui_view_t base;
    const egui_view_data_grid_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_data_grid_action_listener_t on_action;
    egui_color_t surface_color;
    egui_color_t section_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t neutral_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_row;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_row;
};

void egui_view_data_grid_init(egui_view_t *self);
void egui_view_data_grid_set_snapshots(egui_view_t *self, const egui_view_data_grid_snapshot_t *snapshots, uint8_t snapshot_count);
void egui_view_data_grid_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index);
uint8_t egui_view_data_grid_get_current_snapshot(egui_view_t *self);
void egui_view_data_grid_set_current_row(egui_view_t *self, uint8_t row_index);
uint8_t egui_view_data_grid_get_current_row(egui_view_t *self);
uint8_t egui_view_data_grid_activate_current_row(egui_view_t *self);
void egui_view_data_grid_set_on_action_listener(egui_view_t *self, egui_view_on_data_grid_action_listener_t listener);
void egui_view_data_grid_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_data_grid_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_data_grid_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_data_grid_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_data_grid_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                     egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                     egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color);
uint8_t egui_view_data_grid_get_row_region(egui_view_t *self, uint8_t row_index, egui_region_t *region);
void egui_view_data_grid_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_DATA_GRID_H_ */
