#ifndef _EGUI_VIEW_REFERENCE_LIST_H_
#define _EGUI_VIEW_REFERENCE_LIST_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_REFERENCE_LIST_MAX_ITEMS  5
#define EGUI_VIEW_REFERENCE_LIST_INDEX_NONE EGUI_VIEW_REFERENCE_LIST_MAX_ITEMS

#define EGUI_VIEW_REFERENCE_LIST_TONE_ACCENT  0
#define EGUI_VIEW_REFERENCE_LIST_TONE_SUCCESS 1
#define EGUI_VIEW_REFERENCE_LIST_TONE_WARNING 2
#define EGUI_VIEW_REFERENCE_LIST_TONE_NEUTRAL 3

typedef struct egui_view_reference_list_item egui_view_reference_list_item_t;
struct egui_view_reference_list_item
{
    const char *title;
    const char *meta;
    const char *badge;
    uint8_t tone;
    uint8_t emphasized;
};

typedef void (*egui_view_on_reference_list_selection_changed_listener_t)(egui_view_t *self, uint8_t index);

typedef struct egui_view_reference_list egui_view_reference_list_t;
struct egui_view_reference_list
{
    egui_view_t base;
    const egui_view_reference_list_item_t *items;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_reference_list_selection_changed_listener_t on_selection_changed;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t neutral_color;
    uint8_t item_count;
    uint8_t current_index;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_index;
};

void egui_view_reference_list_init(egui_view_t *self);
void egui_view_reference_list_set_items(egui_view_t *self, const egui_view_reference_list_item_t *items, uint8_t item_count);
uint8_t egui_view_reference_list_get_item_count(egui_view_t *self);
void egui_view_reference_list_set_current_index(egui_view_t *self, uint8_t index);
uint8_t egui_view_reference_list_get_current_index(egui_view_t *self);
void egui_view_reference_list_set_on_selection_changed_listener(egui_view_t *self,
                                                                egui_view_on_reference_list_selection_changed_listener_t listener);
void egui_view_reference_list_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_reference_list_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_reference_list_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_reference_list_get_compact_mode(egui_view_t *self);
void egui_view_reference_list_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_reference_list_get_read_only_mode(egui_view_t *self);
void egui_view_reference_list_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                          egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t success_color,
                                          egui_color_t warning_color, egui_color_t neutral_color);
uint8_t egui_view_reference_list_get_item_region(egui_view_t *self, uint8_t index, egui_region_t *region);
void egui_view_reference_list_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_REFERENCE_LIST_H_ */
