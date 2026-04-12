#ifndef _HELLO_CUSTOM_WIDGETS_RADIO_BUTTON_STYLE_H_
#define _HELLO_CUSTOM_WIDGETS_RADIO_BUTTON_STYLE_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_radio_button.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_radio_button_apply_standard_style(egui_view_t *self);
void hcw_radio_button_apply_compact_style(egui_view_t *self);
void hcw_radio_button_apply_read_only_style(egui_view_t *self);
void hcw_radio_button_set_checked(egui_view_t *self, uint8_t is_checked);
void hcw_radio_button_set_text(egui_view_t *self, const char *text);
void hcw_radio_button_set_font(egui_view_t *self, const egui_font_t *font);
void hcw_radio_button_set_text_color(egui_view_t *self, egui_color_t color);
void hcw_radio_button_set_mark_style(egui_view_t *self, egui_view_radio_button_mark_style_t style);
void hcw_radio_button_set_mark_icon(egui_view_t *self, const char *icon);
void hcw_radio_button_set_icon_font(egui_view_t *self, const egui_font_t *font);
void hcw_radio_button_set_icon_text_gap(egui_view_t *self, egui_dim_t gap);
void hcw_radio_button_override_interaction_api(egui_view_t *self, egui_view_api_t *api);
void hcw_radio_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);
int hcw_radio_button_on_key_event(egui_view_t *self, egui_key_event_t *event);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_RADIO_BUTTON_STYLE_H_ */
