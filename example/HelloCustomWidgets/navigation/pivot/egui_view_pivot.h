#ifndef _HELLO_CUSTOM_WIDGETS_PIVOT_H_
#define _HELLO_CUSTOM_WIDGETS_PIVOT_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HCW_PIVOT_MAX_ITEMS 6
#define HCW_PIVOT_INDEX_NONE 0xFF

typedef enum
{
    HCW_PIVOT_TONE_NEUTRAL = 0,
    HCW_PIVOT_TONE_ACCENT,
    HCW_PIVOT_TONE_WARM,
    HCW_PIVOT_TONE_SUCCESS,
} hcw_pivot_tone_t;

typedef struct hcw_pivot_item hcw_pivot_item_t;
struct hcw_pivot_item
{
    const char *header;
    const char *eyebrow;
    const char *title;
    const char *body;
    const char *meta;
    uint8_t tone;
};

typedef void (*hcw_pivot_on_changed_listener_t)(egui_view_t *self, uint8_t index);

typedef struct hcw_pivot hcw_pivot_t;
struct hcw_pivot
{
    egui_view_t base;
    hcw_pivot_on_changed_listener_t on_changed;
    const hcw_pivot_item_t *items;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t card_surface_color;
    uint8_t item_count;
    uint8_t current_index;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_index;
};

void hcw_pivot_init(egui_view_t *self);
void hcw_pivot_apply_standard_style(egui_view_t *self);
void hcw_pivot_apply_compact_style(egui_view_t *self);
void hcw_pivot_apply_read_only_style(egui_view_t *self);
void hcw_pivot_set_items(egui_view_t *self, const hcw_pivot_item_t *items, uint8_t item_count);
void hcw_pivot_set_current_index(egui_view_t *self, uint8_t index);
uint8_t hcw_pivot_get_current_index(egui_view_t *self);
void hcw_pivot_set_on_changed_listener(egui_view_t *self, hcw_pivot_on_changed_listener_t listener);
void hcw_pivot_set_font(egui_view_t *self, const egui_font_t *font);
void hcw_pivot_set_meta_font(egui_view_t *self, const egui_font_t *font);
void hcw_pivot_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                           egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t card_surface_color);
void hcw_pivot_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void hcw_pivot_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t hcw_pivot_get_header_region(egui_view_t *self, uint8_t index, egui_region_t *region);
void hcw_pivot_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_PIVOT_H_ */
