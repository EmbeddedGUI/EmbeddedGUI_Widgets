#ifndef _HELLO_CUSTOM_WIDGETS_SEARCH_BOX_H_
#define _HELLO_CUSTOM_WIDGETS_SEARCH_BOX_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_textinput.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct egui_view_search_box egui_view_search_box_t;
struct egui_view_search_box
{
    egui_view_textinput_t textinput;
    egui_view_api_t api;
    const egui_font_t *icon_font;
    egui_color_t icon_color;
    egui_color_t clear_fill_color;
    egui_color_t clear_fill_pressed_color;
    egui_color_t clear_icon_color;
    uint8_t clear_pressed;
};

void egui_view_search_box_init(egui_view_t *self);
void egui_view_search_box_apply_standard_style(egui_view_t *self);
void egui_view_search_box_apply_compact_style(egui_view_t *self);
void egui_view_search_box_apply_read_only_style(egui_view_t *self);
void egui_view_search_box_set_text(egui_view_t *self, const char *text);
void egui_view_search_box_set_placeholder(egui_view_t *self, const char *placeholder);
void egui_view_search_box_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_search_box_set_icon_font(egui_view_t *self, const egui_font_t *font);
void egui_view_search_box_set_max_length(egui_view_t *self, uint8_t max_length);
uint8_t egui_view_search_box_get_clear_region(egui_view_t *self, egui_region_t *region);
void egui_view_search_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_SEARCH_BOX_H_ */
