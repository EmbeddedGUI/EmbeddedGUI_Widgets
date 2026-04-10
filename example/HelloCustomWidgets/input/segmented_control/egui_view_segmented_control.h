#ifndef _HELLO_CUSTOM_WIDGETS_SEGMENTED_CONTROL_STYLE_H_
#define _HELLO_CUSTOM_WIDGETS_SEGMENTED_CONTROL_STYLE_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_segmented_control.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_segmented_control_apply_standard_style(egui_view_t *self);
void hcw_segmented_control_apply_compact_style(egui_view_t *self);
void hcw_segmented_control_apply_read_only_style(egui_view_t *self);
void hcw_segmented_control_set_segments(egui_view_t *self, const char **segment_texts, uint8_t segment_count);
void hcw_segmented_control_set_current_index(egui_view_t *self, uint8_t index);
void hcw_segmented_control_override_interaction_api(egui_view_t *self, egui_view_api_t *api);
void hcw_segmented_control_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_SEGMENTED_CONTROL_STYLE_H_ */
