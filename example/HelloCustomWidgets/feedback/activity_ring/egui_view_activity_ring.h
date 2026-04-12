#ifndef _HELLO_CUSTOM_WIDGETS_ACTIVITY_RING_STYLE_H_
#define _HELLO_CUSTOM_WIDGETS_ACTIVITY_RING_STYLE_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_activity_ring.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_activity_ring_apply_standard_style(egui_view_t *self);
void hcw_activity_ring_apply_compact_style(egui_view_t *self);
void hcw_activity_ring_apply_paused_style(egui_view_t *self);
void hcw_activity_ring_set_value(egui_view_t *self, uint8_t value);
void hcw_activity_ring_set_palette(egui_view_t *self, egui_color_t ring_color, egui_color_t ring_bg_color);
void hcw_activity_ring_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_ACTIVITY_RING_STYLE_H_ */
