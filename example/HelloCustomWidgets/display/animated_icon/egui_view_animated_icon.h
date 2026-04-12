#ifndef _EGUI_VIEW_ANIMATED_ICON_H_
#define _EGUI_VIEW_ANIMATED_ICON_H_

#include "core/egui_timer.h"
#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    EGUI_VIEW_ANIMATED_ICON_VISUAL_BACK = 0,
    EGUI_VIEW_ANIMATED_ICON_VISUAL_CHEVRON_DOWN_SMALL,
} egui_view_animated_icon_visual_t;

typedef struct egui_view_animated_icon_source egui_view_animated_icon_source_t;
struct egui_view_animated_icon_source
{
    uint8_t visual;
    const char *name;
    const char *default_state;
    const char *default_fallback_glyph;
};

typedef struct egui_view_animated_icon egui_view_animated_icon_t;
struct egui_view_animated_icon
{
    egui_view_t base;
    egui_timer_t anim_timer;
    const egui_view_animated_icon_source_t *source;
    const egui_font_t *icon_font;
    const char *fallback_glyph;
    egui_color_t icon_color;
    int16_t current_progress;
    int16_t from_progress;
    int16_t to_progress;
    uint8_t current_state;
    uint8_t animation_enabled;
    uint8_t fallback_overridden;
    uint8_t anim_step;
    uint8_t anim_steps;
    uint8_t timer_started;
};

void egui_view_animated_icon_init(egui_view_t *self);
void egui_view_animated_icon_apply_standard_style(egui_view_t *self);
void egui_view_animated_icon_apply_subtle_style(egui_view_t *self);
void egui_view_animated_icon_apply_accent_style(egui_view_t *self);
void egui_view_animated_icon_set_source(egui_view_t *self, const egui_view_animated_icon_source_t *source);
const egui_view_animated_icon_source_t *egui_view_animated_icon_get_source(egui_view_t *self);
void egui_view_animated_icon_set_state(egui_view_t *self, const char *state_name);
const char *egui_view_animated_icon_get_state(egui_view_t *self);
void egui_view_animated_icon_set_fallback_glyph(egui_view_t *self, const char *glyph);
const char *egui_view_animated_icon_get_fallback_glyph(egui_view_t *self);
void egui_view_animated_icon_set_icon_font(egui_view_t *self, const egui_font_t *font);
void egui_view_animated_icon_set_palette(egui_view_t *self, egui_color_t icon_color);
void egui_view_animated_icon_set_animation_enabled(egui_view_t *self, uint8_t enabled);
uint8_t egui_view_animated_icon_get_animation_enabled(egui_view_t *self);
const egui_view_animated_icon_source_t *egui_view_animated_icon_get_back_source(void);
const egui_view_animated_icon_source_t *egui_view_animated_icon_get_chevron_down_small_source(void);
void egui_view_animated_icon_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_ANIMATED_ICON_H_ */
