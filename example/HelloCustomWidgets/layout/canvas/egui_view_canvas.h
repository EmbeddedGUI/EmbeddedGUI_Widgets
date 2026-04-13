#ifndef _HELLO_CUSTOM_WIDGETS_CANVAS_H_
#define _HELLO_CUSTOM_WIDGETS_CANVAS_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_group.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_canvas_apply_standard_style(egui_view_t *self);
void hcw_canvas_apply_compact_style(egui_view_t *self);
void hcw_canvas_set_child_origin(egui_view_t *child, egui_dim_t x, egui_dim_t y);
void hcw_canvas_layout_childs(egui_view_t *self);
void hcw_canvas_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_CANVAS_H_ */
