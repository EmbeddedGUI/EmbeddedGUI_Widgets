#ifndef _HELLO_CUSTOM_WIDGETS_INFO_BADGE_H_
#define _HELLO_CUSTOM_WIDGETS_INFO_BADGE_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_notification_badge.h"

#ifdef __cplusplus
extern "C" {
#endif

void hcw_info_badge_apply_count_style(egui_view_t *self);
void hcw_info_badge_apply_icon_style(egui_view_t *self);
void hcw_info_badge_apply_attention_style(egui_view_t *self);
void hcw_info_badge_set_count(egui_view_t *self, uint16_t count);
void hcw_info_badge_set_icon(egui_view_t *self, const char *icon);
void hcw_info_badge_set_palette(egui_view_t *self, egui_color_t badge_color, egui_color_t text_color);
void hcw_info_badge_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_INFO_BADGE_H_ */
