#ifndef _EGUI_VIEW_IMAGE_ICON_H_
#define _EGUI_VIEW_IMAGE_ICON_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef egui_view_image_t egui_view_image_icon_t;

void egui_view_image_icon_init(egui_view_t *self);
void egui_view_image_icon_set_image(egui_view_t *self, const egui_image_t *image);
const egui_image_t *egui_view_image_icon_get_image(egui_view_t *self);
const egui_image_t *egui_view_image_icon_get_default_image(void);
const egui_image_t *egui_view_image_icon_get_warm_image(void);
const egui_image_t *egui_view_image_icon_get_fresh_image(void);
void egui_view_image_icon_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_IMAGE_ICON_H_ */
