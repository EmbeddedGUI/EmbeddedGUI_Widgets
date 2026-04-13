#ifndef _HELLO_CUSTOM_WIDGETS_GRID_H_
#define _HELLO_CUSTOM_WIDGETS_GRID_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_gridlayout.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_grid_apply_standard_style(egui_view_t *self);
void hcw_grid_apply_dense_style(egui_view_t *self);
void hcw_grid_apply_stack_style(egui_view_t *self);
void hcw_grid_set_columns(egui_view_t *self, uint8_t columns);
void hcw_grid_set_align_type(egui_view_t *self, uint8_t align_type);
void hcw_grid_layout_childs(egui_view_t *self);
void hcw_grid_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_GRID_H_ */
