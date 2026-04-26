#ifndef _HELLO_CUSTOM_WIDGETS_BULLET_DECORATOR_H_
#define _HELLO_CUSTOM_WIDGETS_BULLET_DECORATOR_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_BULLET_DECORATOR_MAX_TEXT_LEN        47
#define EGUI_VIEW_BULLET_DECORATOR_MAX_BULLET_TEXT_LEN 7

typedef enum
{
    EGUI_VIEW_BULLET_DECORATOR_BULLET_DOT = 0,
    EGUI_VIEW_BULLET_DECORATOR_BULLET_SQUARE,
    EGUI_VIEW_BULLET_DECORATOR_BULLET_TEXT,
} egui_view_bullet_decorator_bullet_t;

typedef struct egui_view_bullet_decorator egui_view_bullet_decorator_t;
struct egui_view_bullet_decorator
{
    egui_view_t base;
    const egui_font_t *text_font;
    const egui_font_t *bullet_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t bullet_color;
    egui_color_t text_color;
    egui_color_t accent_color;
    char content_text[EGUI_VIEW_BULLET_DECORATOR_MAX_TEXT_LEN + 1];
    char bullet_text[EGUI_VIEW_BULLET_DECORATOR_MAX_BULLET_TEXT_LEN + 1];
    egui_dim_t bullet_slot_width;
    egui_dim_t bullet_gap;
    egui_dim_t bullet_size;
    uint8_t bullet_kind;
    uint8_t content_align_type;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_bullet_decorator_init(egui_view_t *self);
void egui_view_bullet_decorator_set_content_text(egui_view_t *self, const char *text);
const char *egui_view_bullet_decorator_get_content_text(egui_view_t *self);
void egui_view_bullet_decorator_set_bullet_text(egui_view_t *self, const char *text);
const char *egui_view_bullet_decorator_get_bullet_text(egui_view_t *self);
void egui_view_bullet_decorator_set_bullet_kind(egui_view_t *self, uint8_t bullet_kind);
uint8_t egui_view_bullet_decorator_get_bullet_kind(egui_view_t *self);
void egui_view_bullet_decorator_set_fonts(egui_view_t *self, const egui_font_t *text_font, const egui_font_t *bullet_font);
void egui_view_bullet_decorator_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                            egui_color_t bullet_color, egui_color_t text_color, egui_color_t accent_color);
void egui_view_bullet_decorator_set_metrics(egui_view_t *self, egui_dim_t bullet_slot_width, egui_dim_t bullet_gap, egui_dim_t bullet_size);
egui_dim_t egui_view_bullet_decorator_get_bullet_slot_width(egui_view_t *self);
egui_dim_t egui_view_bullet_decorator_get_bullet_gap(egui_view_t *self);
egui_dim_t egui_view_bullet_decorator_get_bullet_size(egui_view_t *self);
void egui_view_bullet_decorator_get_regions(egui_view_t *self, egui_region_t *bullet_region, egui_region_t *content_region);
void egui_view_bullet_decorator_set_content_align_type(egui_view_t *self, uint8_t align_type);
uint8_t egui_view_bullet_decorator_get_content_align_type(egui_view_t *self);
void egui_view_bullet_decorator_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_bullet_decorator_get_compact_mode(egui_view_t *self);
void egui_view_bullet_decorator_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_bullet_decorator_get_read_only_mode(egui_view_t *self);
void egui_view_bullet_decorator_apply_standard_style(egui_view_t *self);
void egui_view_bullet_decorator_apply_accent_style(egui_view_t *self);
void egui_view_bullet_decorator_apply_compact_style(egui_view_t *self);
void egui_view_bullet_decorator_apply_read_only_style(egui_view_t *self);
void egui_view_bullet_decorator_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_BULLET_DECORATOR_H_ */
