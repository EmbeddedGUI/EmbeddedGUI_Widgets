#ifndef _HELLO_CUSTOM_WIDGETS_SPINNER_H_
#define _HELLO_CUSTOM_WIDGETS_SPINNER_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_spinner.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_spinner_apply_standard_style(egui_view_t *self);
void hcw_spinner_apply_compact_style(egui_view_t *self);
void hcw_spinner_apply_muted_style(egui_view_t *self);
void hcw_spinner_set_palette(egui_view_t *self, egui_color_t color);
void hcw_spinner_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width);
void hcw_spinner_set_arc_length(egui_view_t *self, int16_t arc_length);
void hcw_spinner_set_rotation_angle(egui_view_t *self, int16_t rotation_angle);
void hcw_spinner_set_spinning(egui_view_t *self, uint8_t spinning);
void hcw_spinner_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_SPINNER_H_ */
