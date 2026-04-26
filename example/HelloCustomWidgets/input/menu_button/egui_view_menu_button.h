#ifndef _EGUI_VIEW_MENU_BUTTON_H_
#define _EGUI_VIEW_MENU_BUTTON_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_MENU_BUTTON_MAX_ITEMS       5
#define EGUI_VIEW_MENU_BUTTON_INDEX_NONE      0xFF
#define EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER  0xFE

#define EGUI_VIEW_MENU_BUTTON_TONE_ACCENT     0
#define EGUI_VIEW_MENU_BUTTON_TONE_SUCCESS    1
#define EGUI_VIEW_MENU_BUTTON_TONE_WARNING    2
#define EGUI_VIEW_MENU_BUTTON_TONE_DANGER     3
#define EGUI_VIEW_MENU_BUTTON_TONE_NEUTRAL    4

typedef struct egui_view_menu_button_item egui_view_menu_button_item_t;
struct egui_view_menu_button_item
{
    const char *label;
    const char *icon;
    const char *shortcut;
    uint8_t tone;
    uint8_t checked;
    uint8_t disabled;
};

typedef void (*egui_view_menu_button_action_listener_t)(egui_view_t *self, uint8_t index);

typedef struct egui_view_menu_button egui_view_menu_button_t;
struct egui_view_menu_button
{
    egui_view_t base;
    egui_view_menu_button_item_t items[EGUI_VIEW_MENU_BUTTON_MAX_ITEMS];
    const char *button_label;
    const char *button_icon;
    const char *menu_title;
    const egui_font_t *label_font;
    const egui_font_t *meta_font;
    const egui_font_t *icon_font;
    egui_view_menu_button_action_listener_t on_action;
    egui_color_t surface_color;
    egui_color_t menu_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t danger_color;
    egui_color_t neutral_color;
    uint8_t item_count;
    uint8_t selected_index;
    uint8_t focus_index;
    uint8_t active_target;
    uint8_t is_open;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_menu_button_init(egui_view_t *self);
void egui_view_menu_button_set_button(egui_view_t *self, const char *label, const char *icon);
void egui_view_menu_button_set_menu_title(egui_view_t *self, const char *title);
void egui_view_menu_button_set_items(egui_view_t *self, const egui_view_menu_button_item_t *items, uint8_t item_count);
void egui_view_menu_button_set_selected_index(egui_view_t *self, uint8_t index);
uint8_t egui_view_menu_button_get_selected_index(egui_view_t *self);
void egui_view_menu_button_set_open(egui_view_t *self, uint8_t is_open);
uint8_t egui_view_menu_button_get_open(egui_view_t *self);
void egui_view_menu_button_set_fonts(egui_view_t *self, const egui_font_t *label_font, const egui_font_t *meta_font,
                                     const egui_font_t *icon_font);
void egui_view_menu_button_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_menu_button_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_menu_button_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t menu_color, egui_color_t border_color,
                                       egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                       egui_color_t success_color, egui_color_t warning_color, egui_color_t danger_color,
                                       egui_color_t neutral_color);
void egui_view_menu_button_set_on_action_listener(egui_view_t *self, egui_view_menu_button_action_listener_t listener);
uint8_t egui_view_menu_button_activate_item(egui_view_t *self, uint8_t index);
uint8_t egui_view_menu_button_get_trigger_region(egui_view_t *self, egui_region_t *region);
uint8_t egui_view_menu_button_get_item_region(egui_view_t *self, uint8_t index, egui_region_t *region);
void egui_view_menu_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_MENU_BUTTON_H_ */
