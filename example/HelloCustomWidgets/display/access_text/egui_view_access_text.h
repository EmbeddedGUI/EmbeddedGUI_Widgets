#ifndef _HELLO_CUSTOM_WIDGETS_ACCESS_TEXT_H_
#define _HELLO_CUSTOM_WIDGETS_ACCESS_TEXT_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_ACCESS_TEXT_MAX_MARKUP_LEN 63
#define EGUI_VIEW_ACCESS_TEXT_MAX_TEXT_LEN   63
#define EGUI_VIEW_ACCESS_TEXT_ACCESS_NONE    0xFF

typedef struct egui_view_access_text egui_view_access_text_t;
struct egui_view_access_text
{
    egui_view_t base;
    const egui_font_t *text_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t accent_color;
    egui_color_t cue_color;
    char markup_text[EGUI_VIEW_ACCESS_TEXT_MAX_MARKUP_LEN + 1];
    char display_text[EGUI_VIEW_ACCESS_TEXT_MAX_TEXT_LEN + 1];
    uint8_t align_type;
    uint8_t access_key_index;
    uint8_t keyboard_cue_visible;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_access_text_init(egui_view_t *self);
void egui_view_access_text_set_markup_text(egui_view_t *self, const char *markup_text);
const char *egui_view_access_text_get_markup_text(egui_view_t *self);
void egui_view_access_text_set_plain_text(egui_view_t *self, const char *plain_text);
const char *egui_view_access_text_get_display_text(egui_view_t *self);
uint8_t egui_view_access_text_get_access_key_index(egui_view_t *self);
char egui_view_access_text_get_access_key_char(egui_view_t *self);
void egui_view_access_text_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_access_text_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                       egui_color_t text_color, egui_color_t accent_color, egui_color_t cue_color);
void egui_view_access_text_set_align_type(egui_view_t *self, uint8_t align_type);
uint8_t egui_view_access_text_get_align_type(egui_view_t *self);
void egui_view_access_text_set_keyboard_cue_visible(egui_view_t *self, uint8_t visible);
uint8_t egui_view_access_text_get_keyboard_cue_visible(egui_view_t *self);
void egui_view_access_text_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_access_text_get_compact_mode(egui_view_t *self);
void egui_view_access_text_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_access_text_get_read_only_mode(egui_view_t *self);
void egui_view_access_text_apply_standard_style(egui_view_t *self);
void egui_view_access_text_apply_accent_style(egui_view_t *self);
void egui_view_access_text_apply_compact_style(egui_view_t *self);
void egui_view_access_text_apply_read_only_style(egui_view_t *self);
void egui_view_access_text_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_ACCESS_TEXT_H_ */
