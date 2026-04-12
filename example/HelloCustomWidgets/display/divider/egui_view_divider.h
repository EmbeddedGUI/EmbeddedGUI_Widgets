#ifndef _HELLO_CUSTOM_WIDGETS_DIVIDER_STYLE_H_
#define _HELLO_CUSTOM_WIDGETS_DIVIDER_STYLE_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_divider.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_divider_apply_standard_style(egui_view_t *self);
void hcw_divider_apply_subtle_style(egui_view_t *self);
void hcw_divider_apply_accent_style(egui_view_t *self);
void hcw_divider_set_palette(egui_view_t *self, egui_color_t color, egui_alpha_t alpha);
void hcw_divider_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_DIVIDER_STYLE_H_ */
