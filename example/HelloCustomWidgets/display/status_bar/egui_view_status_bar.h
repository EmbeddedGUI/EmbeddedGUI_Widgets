#ifndef _HELLO_CUSTOM_WIDGETS_STATUS_BAR_H_
#define _HELLO_CUSTOM_WIDGETS_STATUS_BAR_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_STATUS_BAR_MAX_ITEMS    4
#define EGUI_VIEW_STATUS_BAR_STATE_NORMAL 0
#define EGUI_VIEW_STATUS_BAR_STATE_INFO   1
#define EGUI_VIEW_STATUS_BAR_STATE_OK     2
#define EGUI_VIEW_STATUS_BAR_STATE_WARN   3

typedef struct egui_view_status_bar_item egui_view_status_bar_item_t;
struct egui_view_status_bar_item
{
    const char *label;
    const char *value;
    uint8_t weight;
    uint8_t state;
    uint8_t emphasized;
};

typedef struct egui_view_status_bar egui_view_status_bar_t;
struct egui_view_status_bar
{
    egui_view_t base;
    egui_view_status_bar_item_t items[EGUI_VIEW_STATUS_BAR_MAX_ITEMS];
    const egui_font_t *label_font;
    const egui_font_t *value_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t separator_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t ok_color;
    egui_color_t warn_color;
    uint8_t item_count;
};

void egui_view_status_bar_init(egui_view_t *self);
void egui_view_status_bar_set_items(egui_view_t *self, const egui_view_status_bar_item_t *items, uint8_t item_count);
void egui_view_status_bar_set_item(egui_view_t *self, uint8_t index, const egui_view_status_bar_item_t *item);
void egui_view_status_bar_set_fonts(egui_view_t *self, const egui_font_t *label_font, const egui_font_t *value_font);
void egui_view_status_bar_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                      egui_color_t separator_color, egui_color_t text_color, egui_color_t muted_text_color,
                                      egui_color_t accent_color, egui_color_t ok_color, egui_color_t warn_color);
uint8_t egui_view_status_bar_get_item_region(egui_view_t *self, uint8_t index, egui_region_t *region);
void egui_view_status_bar_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_STATUS_BAR_H_ */
