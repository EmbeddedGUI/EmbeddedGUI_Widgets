#ifndef _EGUI_VIEW_TOOLBAR_H_
#define _EGUI_VIEW_TOOLBAR_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_TOOLBAR_MAX_ITEMS   5
#define EGUI_VIEW_TOOLBAR_INDEX_NONE  0xFF

#define EGUI_VIEW_TOOLBAR_ITEM_BUTTON 0
#define EGUI_VIEW_TOOLBAR_ITEM_TOGGLE 1

typedef struct egui_view_toolbar_item egui_view_toolbar_item_t;
struct egui_view_toolbar_item
{
    const char *label;
    const char *icon;
    uint8_t kind;
    uint8_t checked;
    uint8_t disabled;
};

typedef void (*egui_view_toolbar_action_listener_t)(egui_view_t *self, uint8_t index);

typedef struct egui_view_toolbar egui_view_toolbar_t;
struct egui_view_toolbar
{
    egui_view_t base;
    egui_view_toolbar_item_t items[EGUI_VIEW_TOOLBAR_MAX_ITEMS];
    const egui_font_t *label_font;
    const egui_font_t *icon_font;
    egui_view_toolbar_action_listener_t on_action;
    egui_color_t surface_color;
    egui_color_t item_color;
    egui_color_t checked_color;
    egui_color_t pressed_color;
    egui_color_t border_color;
    egui_color_t focus_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t icon_color;
    egui_color_t checked_icon_color;
    uint8_t item_count;
    uint8_t current_index;
    uint8_t pressed_index;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_toolbar_init(egui_view_t *self);
void egui_view_toolbar_set_items(egui_view_t *self, const egui_view_toolbar_item_t *items, uint8_t item_count);
void egui_view_toolbar_set_current_index(egui_view_t *self, uint8_t index);
void egui_view_toolbar_set_item_checked(egui_view_t *self, uint8_t index, uint8_t checked);
void egui_view_toolbar_set_item_disabled(egui_view_t *self, uint8_t index, uint8_t disabled);
void egui_view_toolbar_set_fonts(egui_view_t *self, const egui_font_t *label_font, const egui_font_t *icon_font);
void egui_view_toolbar_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_toolbar_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_toolbar_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t item_color, egui_color_t checked_color,
                                   egui_color_t pressed_color, egui_color_t border_color, egui_color_t focus_color, egui_color_t text_color,
                                   egui_color_t muted_text_color, egui_color_t icon_color, egui_color_t checked_icon_color);
void egui_view_toolbar_set_on_action_listener(egui_view_t *self, egui_view_toolbar_action_listener_t listener);
uint8_t egui_view_toolbar_activate_item(egui_view_t *self, uint8_t index);
uint8_t egui_view_toolbar_get_item_region(egui_view_t *self, uint8_t index, egui_region_t *region);
void egui_view_toolbar_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_TOOLBAR_H_ */
