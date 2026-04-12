#ifndef _HELLO_CUSTOM_WIDGETS_HYPERLINK_BUTTON_STYLE_H_
#define _HELLO_CUSTOM_WIDGETS_HYPERLINK_BUTTON_STYLE_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_button.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_hyperlink_button_apply_standard_style(egui_view_t *self);
void hcw_hyperlink_button_apply_inline_style(egui_view_t *self);
void hcw_hyperlink_button_apply_disabled_style(egui_view_t *self);
void hcw_hyperlink_button_set_text(egui_view_t *self, const char *text);
void hcw_hyperlink_button_set_font(egui_view_t *self, const egui_font_t *font);
void hcw_hyperlink_button_override_interaction_api(egui_view_t *self, egui_view_api_t *api);
void hcw_hyperlink_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);
int hcw_hyperlink_button_on_key_event(egui_view_t *self, egui_key_event_t *event);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_HYPERLINK_BUTTON_STYLE_H_ */
