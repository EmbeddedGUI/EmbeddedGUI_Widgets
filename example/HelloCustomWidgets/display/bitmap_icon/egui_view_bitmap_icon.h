#ifndef _EGUI_VIEW_BITMAP_ICON_H_
#define _EGUI_VIEW_BITMAP_ICON_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef egui_view_image_t egui_view_bitmap_icon_t;

void egui_view_bitmap_icon_init(egui_view_t *self);
void egui_view_bitmap_icon_apply_standard_style(egui_view_t *self);
void egui_view_bitmap_icon_apply_subtle_style(egui_view_t *self);
void egui_view_bitmap_icon_apply_accent_style(egui_view_t *self);
void egui_view_bitmap_icon_set_image(egui_view_t *self, const egui_image_t *image);
const egui_image_t *egui_view_bitmap_icon_get_image(egui_view_t *self);
const egui_image_t *egui_view_bitmap_icon_get_document_image(void);
const egui_image_t *egui_view_bitmap_icon_get_mail_image(void);
const egui_image_t *egui_view_bitmap_icon_get_alert_image(void);
void egui_view_bitmap_icon_set_palette(egui_view_t *self, egui_color_t icon_color);
void egui_view_bitmap_icon_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_BITMAP_ICON_H_ */
