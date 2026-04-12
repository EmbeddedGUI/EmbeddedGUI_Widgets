#ifndef _EGUI_VIEW_REPEAT_BUTTON_H_
#define _EGUI_VIEW_REPEAT_BUTTON_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct egui_view_repeat_button egui_view_repeat_button_t;
struct egui_view_repeat_button
{
    egui_view_button_t base;
    egui_timer_t repeat_timer;
    uint16_t initial_delay_ms;
    uint16_t repeat_interval_ms;
    uint8_t timer_started;
    uint8_t touch_active;
    uint8_t key_active;
};

void egui_view_repeat_button_init(egui_view_t *self);
void egui_view_repeat_button_apply_standard_style(egui_view_t *self);
void egui_view_repeat_button_apply_compact_style(egui_view_t *self);
void egui_view_repeat_button_apply_disabled_style(egui_view_t *self);
void egui_view_repeat_button_set_text(egui_view_t *self, const char *text);
void egui_view_repeat_button_set_icon(egui_view_t *self, const char *icon);
void egui_view_repeat_button_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_repeat_button_set_icon_font(egui_view_t *self, const egui_font_t *font);
void egui_view_repeat_button_set_icon_text_gap(egui_view_t *self, egui_dim_t gap);
void egui_view_repeat_button_set_repeat_timing(egui_view_t *self, uint16_t initial_delay_ms, uint16_t repeat_interval_ms);
void egui_view_repeat_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_REPEAT_BUTTON_H_ */
