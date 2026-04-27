#ifndef _HELLO_CUSTOM_WIDGETS_TICK_BAR_H_
#define _HELLO_CUSTOM_WIDGETS_TICK_BAR_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum egui_view_tick_bar_placement egui_view_tick_bar_placement_t;
enum egui_view_tick_bar_placement
{
    EGUI_VIEW_TICK_BAR_PLACEMENT_BOTTOM = 0,
    EGUI_VIEW_TICK_BAR_PLACEMENT_TOP,
    EGUI_VIEW_TICK_BAR_PLACEMENT_LEFT,
    EGUI_VIEW_TICK_BAR_PLACEMENT_RIGHT,
};

typedef struct egui_view_tick_bar egui_view_tick_bar_t;
struct egui_view_tick_bar
{
    egui_view_t base;
    int16_t minimum;
    int16_t maximum;
    int16_t value;
    int16_t selection_start;
    int16_t selection_end;
    uint8_t tick_frequency;
    uint8_t placement;
    uint8_t reversed;
    uint8_t show_selected_range;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    egui_color_t rail_color;
    egui_color_t tick_color;
    egui_color_t selected_tick_color;
    egui_color_t value_color;
};

void egui_view_tick_bar_init(egui_view_t *self);
void egui_view_tick_bar_set_range(egui_view_t *self, int16_t minimum, int16_t maximum);
int16_t egui_view_tick_bar_get_minimum(egui_view_t *self);
int16_t egui_view_tick_bar_get_maximum(egui_view_t *self);
void egui_view_tick_bar_set_value(egui_view_t *self, int16_t value);
int16_t egui_view_tick_bar_get_value(egui_view_t *self);
void egui_view_tick_bar_set_selection_range(egui_view_t *self, int16_t selection_start, int16_t selection_end);
void egui_view_tick_bar_get_selection_range(egui_view_t *self, int16_t *selection_start, int16_t *selection_end);
void egui_view_tick_bar_set_tick_frequency(egui_view_t *self, uint8_t tick_frequency);
uint8_t egui_view_tick_bar_get_tick_frequency(egui_view_t *self);
void egui_view_tick_bar_set_placement(egui_view_t *self, uint8_t placement);
uint8_t egui_view_tick_bar_get_placement(egui_view_t *self);
void egui_view_tick_bar_set_reversed(egui_view_t *self, uint8_t reversed);
uint8_t egui_view_tick_bar_get_reversed(egui_view_t *self);
void egui_view_tick_bar_set_show_selected_range(egui_view_t *self, uint8_t show_selected_range);
uint8_t egui_view_tick_bar_get_show_selected_range(egui_view_t *self);
void egui_view_tick_bar_set_colors(egui_view_t *self, egui_color_t rail_color, egui_color_t tick_color, egui_color_t selected_tick_color,
                                   egui_color_t value_color);
void egui_view_tick_bar_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_tick_bar_get_compact_mode(egui_view_t *self);
void egui_view_tick_bar_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_tick_bar_get_read_only_mode(egui_view_t *self);
void egui_view_tick_bar_apply_standard_style(egui_view_t *self);
void egui_view_tick_bar_apply_accent_style(egui_view_t *self);
void egui_view_tick_bar_apply_vertical_style(egui_view_t *self);
void egui_view_tick_bar_apply_compact_style(egui_view_t *self);
void egui_view_tick_bar_apply_read_only_style(egui_view_t *self);
void egui_view_tick_bar_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_TICK_BAR_H_ */
