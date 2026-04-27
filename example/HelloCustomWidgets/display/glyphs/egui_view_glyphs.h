#ifndef _HELLO_CUSTOM_WIDGETS_GLYPHS_H_
#define _HELLO_CUSTOM_WIDGETS_GLYPHS_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct egui_view_glyphs egui_view_glyphs_t;
struct egui_view_glyphs
{
    egui_view_t base;
    const char *unicode_string;
    const egui_font_t *font;
    egui_color_t fill_color;
    egui_color_t accent_color;
    uint8_t font_rendering_em_size;
    uint8_t origin_x_percent;
    uint8_t origin_y_percent;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_glyphs_init(egui_view_t *self);
void egui_view_glyphs_set_unicode_string(egui_view_t *self, const char *unicode_string);
const char *egui_view_glyphs_get_unicode_string(egui_view_t *self);
void egui_view_glyphs_set_font(egui_view_t *self, const egui_font_t *font, uint8_t font_rendering_em_size);
const egui_font_t *egui_view_glyphs_get_font(egui_view_t *self);
uint8_t egui_view_glyphs_get_font_rendering_em_size(egui_view_t *self);
void egui_view_glyphs_set_fill(egui_view_t *self, egui_color_t fill_color, egui_color_t accent_color);
void egui_view_glyphs_set_origin(egui_view_t *self, uint8_t origin_x_percent, uint8_t origin_y_percent);
void egui_view_glyphs_get_origin(egui_view_t *self, uint8_t *origin_x_percent, uint8_t *origin_y_percent);
void egui_view_glyphs_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_glyphs_get_compact_mode(egui_view_t *self);
void egui_view_glyphs_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_glyphs_get_read_only_mode(egui_view_t *self);
void egui_view_glyphs_apply_standard_style(egui_view_t *self);
void egui_view_glyphs_apply_accent_style(egui_view_t *self);
void egui_view_glyphs_apply_compact_style(egui_view_t *self);
void egui_view_glyphs_apply_read_only_style(egui_view_t *self);
void egui_view_glyphs_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_GLYPHS_H_ */
