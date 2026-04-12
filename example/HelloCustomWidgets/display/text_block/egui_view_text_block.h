#ifndef _EGUI_VIEW_TEXT_BLOCK_H_
#define _EGUI_VIEW_TEXT_BLOCK_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    EGUI_VIEW_TEXT_BLOCK_STYLE_STANDARD = 0,
    EGUI_VIEW_TEXT_BLOCK_STYLE_SUBTLE,
    EGUI_VIEW_TEXT_BLOCK_STYLE_ACCENT,
} egui_view_text_block_style_t;

typedef struct egui_view_text_block egui_view_text_block_t;
struct egui_view_text_block
{
    egui_view_textblock_t base;
    egui_color_t standard_color;
    egui_color_t subtle_color;
    egui_color_t accent_color;
    egui_alpha_t text_alpha;
    uint8_t style;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_text_block_init(egui_view_t *self);
void egui_view_text_block_apply_standard_style(egui_view_t *self);
void egui_view_text_block_apply_subtle_style(egui_view_t *self);
void egui_view_text_block_apply_accent_style(egui_view_t *self);
void egui_view_text_block_set_text(egui_view_t *self, const char *text);
void egui_view_text_block_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_text_block_set_palette(egui_view_t *self, egui_color_t standard_color, egui_color_t subtle_color, egui_color_t accent_color,
                                      egui_alpha_t text_alpha);
void egui_view_text_block_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_text_block_get_compact_mode(egui_view_t *self);
void egui_view_text_block_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_text_block_get_read_only_mode(egui_view_t *self);
void egui_view_text_block_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_TEXT_BLOCK_H_ */
