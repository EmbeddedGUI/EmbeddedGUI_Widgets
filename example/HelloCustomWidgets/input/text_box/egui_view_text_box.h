#ifndef _HELLO_CUSTOM_WIDGETS_TEXT_BOX_STYLE_H_
#define _HELLO_CUSTOM_WIDGETS_TEXT_BOX_STYLE_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_textinput.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_text_box_apply_standard_style(egui_view_t *self);
void hcw_text_box_apply_compact_style(egui_view_t *self);
void hcw_text_box_apply_read_only_style(egui_view_t *self);
void hcw_text_box_set_text(egui_view_t *self, const char *text);
void hcw_text_box_set_placeholder(egui_view_t *self, const char *placeholder);
void hcw_text_box_set_font(egui_view_t *self, const egui_font_t *font);
void hcw_text_box_set_max_length(egui_view_t *self, uint8_t max_length);
void hcw_text_box_override_interaction_api(egui_view_t *self, egui_view_api_t *api);
void hcw_text_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_TEXT_BOX_STYLE_H_ */
