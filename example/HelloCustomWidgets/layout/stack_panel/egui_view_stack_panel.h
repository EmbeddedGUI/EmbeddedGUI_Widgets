#ifndef _HELLO_CUSTOM_WIDGETS_STACK_PANEL_H_
#define _HELLO_CUSTOM_WIDGETS_STACK_PANEL_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_linearlayout.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_stack_panel_apply_standard_style(egui_view_t *self);
void hcw_stack_panel_apply_horizontal_style(egui_view_t *self);
void hcw_stack_panel_apply_compact_style(egui_view_t *self);
void hcw_stack_panel_set_orientation(egui_view_t *self, uint8_t is_horizontal);
void hcw_stack_panel_set_align_type(egui_view_t *self, uint8_t align_type);
void hcw_stack_panel_layout_childs(egui_view_t *self);
void hcw_stack_panel_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_STACK_PANEL_H_ */
