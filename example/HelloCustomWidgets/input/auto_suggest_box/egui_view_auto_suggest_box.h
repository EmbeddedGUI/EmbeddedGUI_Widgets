#ifndef _HELLO_CUSTOM_WIDGETS_AUTO_SUGGEST_BOX_STYLE_H_
#define _HELLO_CUSTOM_WIDGETS_AUTO_SUGGEST_BOX_STYLE_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_autocomplete.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_auto_suggest_box_apply_standard_style(egui_view_t *self);
void hcw_auto_suggest_box_apply_compact_style(egui_view_t *self);
void hcw_auto_suggest_box_apply_read_only_style(egui_view_t *self);
void hcw_auto_suggest_box_set_suggestions(egui_view_t *self, const char **suggestions, uint8_t count);
void hcw_auto_suggest_box_set_current_index(egui_view_t *self, uint8_t index);
void hcw_auto_suggest_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_AUTO_SUGGEST_BOX_STYLE_H_ */
