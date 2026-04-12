#ifndef _HELLO_CUSTOM_WIDGETS_PROGRESS_BAR_STYLE_H_
#define _HELLO_CUSTOM_WIDGETS_PROGRESS_BAR_STYLE_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_progress_bar.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_progress_bar_apply_standard_style(egui_view_t *self);
void hcw_progress_bar_apply_paused_style(egui_view_t *self);
void hcw_progress_bar_apply_error_style(egui_view_t *self);
void hcw_progress_bar_set_value(egui_view_t *self, uint8_t value);
void hcw_progress_bar_set_palette(egui_view_t *self, egui_color_t track_color, egui_color_t fill_color, egui_color_t control_color);
void hcw_progress_bar_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_PROGRESS_BAR_STYLE_H_ */
