#ifndef _HELLO_CUSTOM_WIDGETS_COMBO_BOX_STYLE_H_
#define _HELLO_CUSTOM_WIDGETS_COMBO_BOX_STYLE_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_combobox.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_combo_box_apply_standard_style(egui_view_t *self);
void hcw_combo_box_apply_compact_style(egui_view_t *self);
void hcw_combo_box_apply_read_only_style(egui_view_t *self);
void hcw_combo_box_set_items(egui_view_t *self, const char **items, uint8_t count);
void hcw_combo_box_set_current_index(egui_view_t *self, uint8_t index);
void hcw_combo_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_COMBO_BOX_STYLE_H_ */
