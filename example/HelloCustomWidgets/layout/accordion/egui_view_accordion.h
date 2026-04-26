#ifndef _EGUI_VIEW_ACCORDION_H_
#define _EGUI_VIEW_ACCORDION_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_ACCORDION_MAX_ITEMS  4
#define EGUI_VIEW_ACCORDION_INDEX_NONE 0xFF

#define EGUI_VIEW_ACCORDION_TONE_ACCENT  0
#define EGUI_VIEW_ACCORDION_TONE_SUCCESS 1
#define EGUI_VIEW_ACCORDION_TONE_WARNING 2
#define EGUI_VIEW_ACCORDION_TONE_NEUTRAL 3

typedef struct egui_view_accordion_item egui_view_accordion_item_t;
struct egui_view_accordion_item
{
    const char *title;
    const char *description;
    const char *body;
    const char *meta;
    uint8_t tone;
    uint8_t expanded;
};

typedef void (*egui_view_accordion_action_listener_t)(egui_view_t *self, uint8_t item_index, uint8_t expanded);

typedef struct egui_view_accordion egui_view_accordion_t;
struct egui_view_accordion
{
    egui_view_t base;
    const egui_view_accordion_item_t *items;
    egui_view_accordion_action_listener_t on_action;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_color_t surface_color;
    egui_color_t section_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t neutral_color;
    uint8_t item_count;
    uint8_t expanded_index;
    uint8_t focused_index;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_index;
};

void egui_view_accordion_init(egui_view_t *self);
void egui_view_accordion_set_items(egui_view_t *self, const egui_view_accordion_item_t *items, uint8_t item_count);
void egui_view_accordion_set_expanded_index(egui_view_t *self, uint8_t item_index);
uint8_t egui_view_accordion_get_expanded_index(egui_view_t *self);
void egui_view_accordion_set_focused_index(egui_view_t *self, uint8_t item_index);
uint8_t egui_view_accordion_get_focused_index(egui_view_t *self);
uint8_t egui_view_accordion_activate_focused(egui_view_t *self);
void egui_view_accordion_set_on_action_listener(egui_view_t *self, egui_view_accordion_action_listener_t listener);
void egui_view_accordion_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_accordion_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_accordion_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_accordion_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_accordion_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                     egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                     egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color);
uint8_t egui_view_accordion_get_item_region(egui_view_t *self, uint8_t item_index, egui_region_t *region);
void egui_view_accordion_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_ACCORDION_H_ */
