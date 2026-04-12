#ifndef _EGUI_VIEW_RADIO_BUTTONS_H_
#define _EGUI_VIEW_RADIO_BUTTONS_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_RADIO_BUTTONS_MAX_ITEMS  6
#define EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE 0xFF

typedef void (*egui_view_on_radio_buttons_changed_listener_t)(egui_view_t *self, uint8_t index);

typedef struct egui_view_radio_buttons egui_view_radio_buttons_t;
struct egui_view_radio_buttons
{
    egui_view_t base;
    egui_view_on_radio_buttons_changed_listener_t on_selection_changed;
    const char *const *items;
    const egui_font_t *font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    uint8_t item_count;
    uint8_t current_index;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_index;
};

void egui_view_radio_buttons_init(egui_view_t *self);
void egui_view_radio_buttons_set_items(egui_view_t *self, const char *const *items, uint8_t item_count);
void egui_view_radio_buttons_set_current_index(egui_view_t *self, uint8_t index);
uint8_t egui_view_radio_buttons_get_current_index(egui_view_t *self);
void egui_view_radio_buttons_set_on_selection_changed_listener(egui_view_t *self, egui_view_on_radio_buttons_changed_listener_t listener);
void egui_view_radio_buttons_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_radio_buttons_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_radio_buttons_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_radio_buttons_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                         egui_color_t muted_text_color, egui_color_t accent_color);
void egui_view_radio_buttons_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_RADIO_BUTTONS_H_ */
