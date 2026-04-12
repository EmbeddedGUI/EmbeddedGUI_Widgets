#ifndef _HELLO_CUSTOM_WIDGETS_SWITCH_STYLE_H_
#define _HELLO_CUSTOM_WIDGETS_SWITCH_STYLE_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_switch.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_switch_apply_standard_style(egui_view_t *self);
void hcw_switch_apply_compact_style(egui_view_t *self);
void hcw_switch_apply_read_only_style(egui_view_t *self);
void hcw_switch_set_checked(egui_view_t *self, uint8_t is_checked);
void hcw_switch_set_state_icons(egui_view_t *self, const char *icon_on, const char *icon_off);
void hcw_switch_set_icon_font(egui_view_t *self, const egui_font_t *font);
void hcw_switch_override_interaction_api(egui_view_t *self, egui_view_api_t *api);
void hcw_switch_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);
int hcw_switch_on_key_event(egui_view_t *self, egui_key_event_t *event);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_SWITCH_STYLE_H_ */
