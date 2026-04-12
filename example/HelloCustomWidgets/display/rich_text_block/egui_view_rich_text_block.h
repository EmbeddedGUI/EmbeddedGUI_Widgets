#ifndef _EGUI_VIEW_RICH_TEXT_BLOCK_H_
#define _EGUI_VIEW_RICH_TEXT_BLOCK_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_RICH_TEXT_BLOCK_MAX_PARAGRAPHS 4

typedef enum
{
    EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY = 0,
    EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_EMPHASIS,
    EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_ACCENT,
    EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION,
} egui_view_rich_text_block_style_t;

typedef struct egui_view_rich_text_block_paragraph egui_view_rich_text_block_paragraph_t;
struct egui_view_rich_text_block_paragraph
{
    const char *text;
    uint8_t style;
};

typedef struct egui_view_rich_text_block egui_view_rich_text_block_t;
struct egui_view_rich_text_block
{
    egui_view_t base;
    const egui_view_rich_text_block_paragraph_t *paragraphs;
    const egui_font_t *font;
    const egui_font_t *emphasis_font;
    const egui_font_t *caption_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_dim_t line_space;
    egui_dim_t paragraph_gap;
    uint8_t paragraph_count;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_rich_text_block_init(egui_view_t *self);
void egui_view_rich_text_block_set_paragraphs(egui_view_t *self, const egui_view_rich_text_block_paragraph_t *paragraphs, uint8_t paragraph_count);
uint8_t egui_view_rich_text_block_get_paragraph_count(egui_view_t *self);
void egui_view_rich_text_block_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_rich_text_block_set_emphasis_font(egui_view_t *self, const egui_font_t *font);
void egui_view_rich_text_block_set_caption_font(egui_view_t *self, const egui_font_t *font);
void egui_view_rich_text_block_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_rich_text_block_get_compact_mode(egui_view_t *self);
void egui_view_rich_text_block_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_rich_text_block_get_read_only_mode(egui_view_t *self);
void egui_view_rich_text_block_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                           egui_color_t muted_text_color, egui_color_t accent_color);
void egui_view_rich_text_block_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_RICH_TEXT_BLOCK_H_ */
