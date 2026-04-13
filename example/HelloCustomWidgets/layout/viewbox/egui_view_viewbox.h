#ifndef _EGUI_VIEW_VIEWBOX_H_
#define _EGUI_VIEW_VIEWBOX_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_VIEWBOX_MAX_SNAPSHOTS 6
#define EGUI_VIEW_VIEWBOX_MAX_PRESETS   3
#define EGUI_VIEW_VIEWBOX_PRESET_NONE   0xFF

#define EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM        0
#define EGUI_VIEW_VIEWBOX_STRETCH_FILL           1
#define EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY 2

typedef struct egui_view_viewbox_preset egui_view_viewbox_preset_t;
struct egui_view_viewbox_preset
{
    const char *label;
    const char *meta;
    uint8_t stretch_mode;
};

typedef struct egui_view_viewbox_snapshot egui_view_viewbox_snapshot_t;
struct egui_view_viewbox_snapshot
{
    const char *header;
    const char *title;
    const char *summary;
    const char *content_title;
    const char *content_meta;
    const char *content_footer;
    const char *footer;
    const egui_view_viewbox_preset_t *presets;
    egui_dim_t source_width;
    egui_dim_t source_height;
    uint8_t preset_count;
    uint8_t selected_preset;
};

typedef void (*egui_view_on_viewbox_action_listener_t)(egui_view_t *self, uint8_t snapshot_index, uint8_t preset_index, uint8_t stretch_mode);

typedef struct egui_view_viewbox egui_view_viewbox_t;
struct egui_view_viewbox
{
    egui_view_t base;
    const egui_view_viewbox_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_viewbox_action_listener_t on_action;
    egui_color_t surface_color;
    egui_color_t section_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_preset;
    uint8_t stretch_mode;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_preset;
};

void egui_view_viewbox_init(egui_view_t *self);
void egui_view_viewbox_set_snapshots(egui_view_t *self, const egui_view_viewbox_snapshot_t *snapshots, uint8_t snapshot_count);
void egui_view_viewbox_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index);
uint8_t egui_view_viewbox_get_current_snapshot(egui_view_t *self);
void egui_view_viewbox_set_current_preset(egui_view_t *self, uint8_t preset_index);
uint8_t egui_view_viewbox_get_current_preset(egui_view_t *self);
void egui_view_viewbox_set_stretch_mode(egui_view_t *self, uint8_t stretch_mode);
uint8_t egui_view_viewbox_get_stretch_mode(egui_view_t *self);
uint8_t egui_view_viewbox_activate_current_preset(egui_view_t *self);
void egui_view_viewbox_set_on_action_listener(egui_view_t *self, egui_view_on_viewbox_action_listener_t listener);
void egui_view_viewbox_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_viewbox_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_viewbox_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_viewbox_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_viewbox_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                   egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color);
uint8_t egui_view_viewbox_get_preset_region(egui_view_t *self, uint8_t preset_index, egui_region_t *region);
void egui_view_viewbox_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_VIEWBOX_H_ */
