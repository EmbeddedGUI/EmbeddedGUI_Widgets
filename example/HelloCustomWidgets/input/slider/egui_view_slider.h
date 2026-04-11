#ifndef _HELLO_CUSTOM_WIDGETS_SLIDER_STYLE_H_
#define _HELLO_CUSTOM_WIDGETS_SLIDER_STYLE_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_slider.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_slider_apply_standard_style(egui_view_t *self);
void hcw_slider_apply_compact_style(egui_view_t *self);
void hcw_slider_apply_read_only_style(egui_view_t *self);
void hcw_slider_set_value(egui_view_t *self, uint8_t value);
uint8_t hcw_slider_handle_navigation_key(egui_view_t *self, uint8_t key_code);
void hcw_slider_override_interaction_api(egui_view_t *self, egui_view_api_t *api);
void hcw_slider_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_SLIDER_STYLE_H_ */
