#ifndef _HELLO_CUSTOM_WIDGETS_IMAGE_CONTROL_H_
#define _HELLO_CUSTOM_WIDGETS_IMAGE_CONTROL_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_IMAGE_CONTROL_STRETCH_NONE    0
#define EGUI_VIEW_IMAGE_CONTROL_STRETCH_FILL    1
#define EGUI_VIEW_IMAGE_CONTROL_STRETCH_UNIFORM 2

typedef struct egui_view_image_control egui_view_image_control_t;
struct egui_view_image_control
{
    egui_view_t base;
    const egui_image_t *image;
    const char *source_name;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t placeholder_color;
    egui_color_t muted_color;
    uint8_t stretch;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_image_control_init(egui_view_t *self);
void egui_view_image_control_set_source(egui_view_t *self, const egui_image_t *image, const char *source_name);
const egui_image_t *egui_view_image_control_get_source(egui_view_t *self);
const char *egui_view_image_control_get_source_name(egui_view_t *self);
void egui_view_image_control_set_stretch(egui_view_t *self, uint8_t stretch);
uint8_t egui_view_image_control_get_stretch(egui_view_t *self);
void egui_view_image_control_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_image_control_get_compact_mode(egui_view_t *self);
void egui_view_image_control_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_image_control_get_read_only_mode(egui_view_t *self);
void egui_view_image_control_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                         egui_color_t placeholder_color, egui_color_t muted_color);
void egui_view_image_control_apply_standard_style(egui_view_t *self);
void egui_view_image_control_apply_compact_style(egui_view_t *self);
void egui_view_image_control_apply_read_only_style(egui_view_t *self);
void egui_view_image_control_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

const egui_image_t *egui_view_image_control_get_landscape_image(void);
const egui_image_t *egui_view_image_control_get_portrait_image(void);
const egui_image_t *egui_view_image_control_get_square_image(void);
const egui_image_t *egui_view_image_control_get_default_image(void);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_IMAGE_CONTROL_H_ */
