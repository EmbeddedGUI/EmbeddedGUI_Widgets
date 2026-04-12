#ifndef _EGUI_VIEW_FONT_ICON_H_
#define _EGUI_VIEW_FONT_ICON_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef egui_view_label_t egui_view_font_icon_t;

void egui_view_font_icon_init(egui_view_t *self);
void egui_view_font_icon_apply_standard_style(egui_view_t *self);
void egui_view_font_icon_apply_subtle_style(egui_view_t *self);
void egui_view_font_icon_apply_accent_style(egui_view_t *self);
void egui_view_font_icon_set_glyph(egui_view_t *self, const char *glyph);
const char *egui_view_font_icon_get_glyph(egui_view_t *self);
void egui_view_font_icon_set_icon_font(egui_view_t *self, const egui_font_t *font);
void egui_view_font_icon_set_palette(egui_view_t *self, egui_color_t icon_color);
void egui_view_font_icon_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_FONT_ICON_H_ */
