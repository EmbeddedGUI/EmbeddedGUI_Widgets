#ifndef _EGUI_VIEW_SPIN_BUTTON_H_
#define _EGUI_VIEW_SPIN_BUTTON_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_SPIN_BUTTON_PART_NONE      0
#define EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT 1
#define EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT 2
#define EGUI_VIEW_SPIN_BUTTON_PART_FIELD     3

typedef void (*egui_view_spin_button_value_changed_listener_t)(egui_view_t *self, int16_t value);

typedef struct egui_view_spin_button egui_view_spin_button_t;
struct egui_view_spin_button
{
    egui_view_t base;
    egui_view_spin_button_value_changed_listener_t on_value_changed;
    const egui_font_t *value_font;
    const egui_font_t *meta_font;
    const char *label;
    const char *suffix;
    const char *helper;
    egui_color_t surface_color;
    egui_color_t field_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    int16_t value;
    int16_t min_value;
    int16_t max_value;
    int16_t step;
    int16_t large_step;
    uint8_t active_part;
    uint8_t focus_part;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    char value_buffer[24];
};

void egui_view_spin_button_init(egui_view_t *self);
void egui_view_spin_button_set_value(egui_view_t *self, int16_t value);
int16_t egui_view_spin_button_get_value(egui_view_t *self);
void egui_view_spin_button_set_range(egui_view_t *self, int16_t min_value, int16_t max_value);
void egui_view_spin_button_set_step(egui_view_t *self, int16_t step);
void egui_view_spin_button_set_large_step(egui_view_t *self, int16_t large_step);
void egui_view_spin_button_set_texts(egui_view_t *self, const char *label, const char *suffix, const char *helper);
void egui_view_spin_button_set_fonts(egui_view_t *self, const egui_font_t *value_font, const egui_font_t *meta_font);
void egui_view_spin_button_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_spin_button_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_spin_button_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t field_color, egui_color_t border_color,
                                       egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color);
void egui_view_spin_button_set_on_value_changed_listener(egui_view_t *self, egui_view_spin_button_value_changed_listener_t listener);
uint8_t egui_view_spin_button_adjust(egui_view_t *self, int16_t delta);
uint8_t egui_view_spin_button_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region);
void egui_view_spin_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_SPIN_BUTTON_H_ */
