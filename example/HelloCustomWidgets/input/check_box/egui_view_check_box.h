#ifndef _HELLO_CUSTOM_WIDGETS_CHECK_BOX_STYLE_H_
#define _HELLO_CUSTOM_WIDGETS_CHECK_BOX_STYLE_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_checkbox.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_check_box_apply_standard_style(egui_view_t *self);
void hcw_check_box_apply_compact_style(egui_view_t *self);
void hcw_check_box_apply_read_only_style(egui_view_t *self);
void hcw_check_box_set_checked(egui_view_t *self, uint8_t is_checked);
void hcw_check_box_set_text(egui_view_t *self, const char *text);
void hcw_check_box_set_mark_style(egui_view_t *self, egui_view_checkbox_mark_style_t style);
void hcw_check_box_set_mark_icon(egui_view_t *self, const char *icon);
void hcw_check_box_set_icon_font(egui_view_t *self, const egui_font_t *font);
void hcw_check_box_override_interaction_api(egui_view_t *self, egui_view_api_t *api);
void hcw_check_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);
int hcw_check_box_on_key_event(egui_view_t *self, egui_key_event_t *event);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_CHECK_BOX_STYLE_H_ */
