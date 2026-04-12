#include <string.h>

#include "egui_view_animated_icon.h"

#include "resource/egui_icon_material_symbols.h"
#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_icon_font.h"

#define EGUI_VIEW_ANIMATED_ICON_PROGRESS_MAX       1024
#define EGUI_VIEW_ANIMATED_ICON_PROGRESS_POINTER    576
#define EGUI_VIEW_ANIMATED_ICON_PROGRESS_POINTER_2  640
#define EGUI_VIEW_ANIMATED_ICON_ANIM_STEPS           10
#define EGUI_VIEW_ANIMATED_ICON_TIMER_MS             18

enum
{
    EGUI_VIEW_ANIMATED_ICON_STATE_NORMAL = 0,
    EGUI_VIEW_ANIMATED_ICON_STATE_POINTER_OVER,
    EGUI_VIEW_ANIMATED_ICON_STATE_PRESSED,
};

typedef struct egui_view_animated_icon_transition egui_view_animated_icon_transition_t;
struct egui_view_animated_icon_transition
{
    const char *name;
    uint8_t from_state;
    uint8_t to_state;
};

static const char *const egui_view_animated_icon_state_names[] = {
        "Normal",
        "PointerOver",
        "Pressed",
};

static const egui_view_animated_icon_transition_t egui_view_animated_icon_transitions[] = {
        { "NormalToPointerOver", EGUI_VIEW_ANIMATED_ICON_STATE_NORMAL, EGUI_VIEW_ANIMATED_ICON_STATE_POINTER_OVER },
        { "NormalToPressed", EGUI_VIEW_ANIMATED_ICON_STATE_NORMAL, EGUI_VIEW_ANIMATED_ICON_STATE_PRESSED },
        { "PointerOverToNormal", EGUI_VIEW_ANIMATED_ICON_STATE_POINTER_OVER, EGUI_VIEW_ANIMATED_ICON_STATE_NORMAL },
        { "PointerOverToPressed", EGUI_VIEW_ANIMATED_ICON_STATE_POINTER_OVER, EGUI_VIEW_ANIMATED_ICON_STATE_PRESSED },
        { "PressedToNormal", EGUI_VIEW_ANIMATED_ICON_STATE_PRESSED, EGUI_VIEW_ANIMATED_ICON_STATE_NORMAL },
        { "PressedToPointerOver", EGUI_VIEW_ANIMATED_ICON_STATE_PRESSED, EGUI_VIEW_ANIMATED_ICON_STATE_POINTER_OVER },
};

static const egui_view_animated_icon_source_t egui_view_animated_icon_back_source = {
        .visual = EGUI_VIEW_ANIMATED_ICON_VISUAL_BACK,
        .name = "AnimatedBackVisualSource",
        .default_state = "Normal",
        .default_fallback_glyph = EGUI_ICON_MS_ARROW_BACK,
};

static const egui_view_animated_icon_source_t egui_view_animated_icon_chevron_down_small_source = {
        .visual = EGUI_VIEW_ANIMATED_ICON_VISUAL_CHEVRON_DOWN_SMALL,
        .name = "AnimatedChevronDownSmallVisualSource",
        .default_state = "Normal",
        .default_fallback_glyph = EGUI_ICON_MS_EXPAND_MORE,
};

static egui_view_animated_icon_t *egui_view_animated_icon_local(egui_view_t *self)
{
    return (egui_view_animated_icon_t *)self;
}

static uint8_t egui_view_animated_icon_source_is_valid(const egui_view_animated_icon_source_t *source)
{
    if (source == NULL)
    {
        return 0;
    }

    return (source->visual == EGUI_VIEW_ANIMATED_ICON_VISUAL_BACK || source->visual == EGUI_VIEW_ANIMATED_ICON_VISUAL_CHEVRON_DOWN_SMALL) ? 1 : 0;
}

static const char *egui_view_animated_icon_state_name_from_id(uint8_t state_id)
{
    if (state_id >= EGUI_ARRAY_SIZE(egui_view_animated_icon_state_names))
    {
        return egui_view_animated_icon_state_names[EGUI_VIEW_ANIMATED_ICON_STATE_NORMAL];
    }
    return egui_view_animated_icon_state_names[state_id];
}

static uint8_t egui_view_animated_icon_resolve_state_id(const char *state_name, uint8_t *state_id)
{
    uint8_t i;

    if (state_name == NULL)
    {
        *state_id = EGUI_VIEW_ANIMATED_ICON_STATE_NORMAL;
        return 1;
    }

    for (i = 0; i < EGUI_ARRAY_SIZE(egui_view_animated_icon_state_names); ++i)
    {
        if (strcmp(state_name, egui_view_animated_icon_state_names[i]) == 0)
        {
            *state_id = i;
            return 1;
        }
    }

    return 0;
}

static uint8_t egui_view_animated_icon_resolve_transition(const char *state_name, uint8_t *from_state, uint8_t *to_state)
{
    uint8_t i;

    if (state_name == NULL)
    {
        return 0;
    }

    for (i = 0; i < EGUI_ARRAY_SIZE(egui_view_animated_icon_transitions); ++i)
    {
        if (strcmp(state_name, egui_view_animated_icon_transitions[i].name) == 0)
        {
            *from_state = egui_view_animated_icon_transitions[i].from_state;
            *to_state = egui_view_animated_icon_transitions[i].to_state;
            return 1;
        }
    }

    return 0;
}

static const char *egui_view_animated_icon_default_fallback_glyph(const egui_view_animated_icon_source_t *source)
{
    if (source != NULL && EGUI_VIEW_ICON_TEXT_VALID(source->default_fallback_glyph))
    {
        return source->default_fallback_glyph;
    }
    return EGUI_ICON_MS_ARROW_BACK;
}

static const egui_font_t *egui_view_animated_icon_default_icon_font(egui_dim_t area_size)
{
    if (area_size <= 18)
    {
        return EGUI_FONT_ICON_MS_16;
    }
    if (area_size <= 24)
    {
        return EGUI_FONT_ICON_MS_20;
    }
    return EGUI_FONT_ICON_MS_24;
}

static int16_t egui_view_animated_icon_progress_for_state(const egui_view_animated_icon_source_t *source, uint8_t state)
{
    uint8_t visual = egui_view_animated_icon_source_is_valid(source) ? source->visual : EGUI_VIEW_ANIMATED_ICON_VISUAL_BACK;

    if (state > EGUI_VIEW_ANIMATED_ICON_STATE_PRESSED)
    {
        state = EGUI_VIEW_ANIMATED_ICON_STATE_NORMAL;
    }

    if (visual == EGUI_VIEW_ANIMATED_ICON_VISUAL_CHEVRON_DOWN_SMALL)
    {
        switch (state)
        {
        case EGUI_VIEW_ANIMATED_ICON_STATE_POINTER_OVER:
            return EGUI_VIEW_ANIMATED_ICON_PROGRESS_POINTER_2;
        case EGUI_VIEW_ANIMATED_ICON_STATE_PRESSED:
            return EGUI_VIEW_ANIMATED_ICON_PROGRESS_MAX;
        default:
            return 0;
        }
    }

    switch (state)
    {
    case EGUI_VIEW_ANIMATED_ICON_STATE_POINTER_OVER:
        return EGUI_VIEW_ANIMATED_ICON_PROGRESS_POINTER;
    case EGUI_VIEW_ANIMATED_ICON_STATE_PRESSED:
        return EGUI_VIEW_ANIMATED_ICON_PROGRESS_MAX;
    default:
        return 0;
    }
}

static uint8_t egui_view_animated_icon_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static int32_t egui_view_animated_icon_lerp_i32(int32_t from, int32_t to, int32_t q10)
{
    return from + ((to - from) * q10 + 512) / 1024;
}

static int32_t egui_view_animated_icon_phase_between(int32_t value, int32_t start, int32_t end)
{
    if (value <= start)
    {
        return 0;
    }
    if (value >= end)
    {
        return 1024;
    }
    return ((value - start) * 1024 + (end - start) / 2) / (end - start);
}

static const egui_font_t *egui_view_animated_icon_resolve_font(egui_view_animated_icon_t *local, egui_dim_t area_size)
{
    if (local->icon_font != NULL)
    {
        return local->icon_font;
    }
    return egui_view_animated_icon_default_icon_font(area_size);
}

static void egui_view_animated_icon_stop_timer(egui_view_t *self)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);

    if (!local->timer_started)
    {
        return;
    }

    egui_timer_stop_timer(&local->anim_timer);
    local->timer_started = 0;
}

static void egui_view_animated_icon_start_timer(egui_view_t *self)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);

    if (local->timer_started || !local->animation_enabled || !self->is_attached_to_window || !egui_view_animated_icon_source_is_valid(local->source) ||
        local->anim_steps == 0 || local->current_progress == local->to_progress)
    {
        return;
    }

    egui_timer_start_timer(&local->anim_timer, EGUI_VIEW_ANIMATED_ICON_TIMER_MS, EGUI_VIEW_ANIMATED_ICON_TIMER_MS);
    local->timer_started = 1;
}

static void egui_view_animated_icon_tick(egui_timer_t *timer)
{
    egui_view_t *self = (egui_view_t *)timer->user_data;
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);

    if (local->anim_steps == 0)
    {
        egui_view_animated_icon_stop_timer(self);
        return;
    }

    local->anim_step++;
    if (local->anim_step >= local->anim_steps)
    {
        local->current_progress = local->to_progress;
        egui_view_animated_icon_stop_timer(self);
    }
    else
    {
        local->current_progress =
                (int16_t)(local->from_progress + (((local->to_progress - local->from_progress) * local->anim_step) + local->anim_steps / 2) / local->anim_steps);
    }

    egui_view_invalidate(self);
}

static void egui_view_animated_icon_apply_color(egui_view_t *self, egui_color_t icon_color)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);

    egui_view_animated_icon_clear_pressed_state(self);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
    local->icon_color = icon_color;
    egui_view_invalidate(self);
}

void egui_view_animated_icon_apply_standard_style(egui_view_t *self)
{
    egui_view_animated_icon_apply_color(self, EGUI_COLOR_HEX(0x0F6CBD));
}

void egui_view_animated_icon_apply_subtle_style(egui_view_t *self)
{
    egui_view_animated_icon_apply_color(self, EGUI_COLOR_HEX(0x6F7C8A));
}

void egui_view_animated_icon_apply_accent_style(egui_view_t *self)
{
    egui_view_animated_icon_apply_color(self, EGUI_COLOR_HEX(0xA15C00));
}

void egui_view_animated_icon_set_source(egui_view_t *self, const egui_view_animated_icon_source_t *source)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);

    egui_view_animated_icon_clear_pressed_state(self);
    egui_view_animated_icon_stop_timer(self);

    if (!egui_view_animated_icon_source_is_valid(source))
    {
        local->source = NULL;
    }
    else
    {
        local->source = source;
    }

    if (!local->fallback_overridden)
    {
        local->fallback_glyph = egui_view_animated_icon_default_fallback_glyph(local->source);
    }

    local->current_progress = egui_view_animated_icon_progress_for_state(local->source, local->current_state);
    local->from_progress = local->current_progress;
    local->to_progress = local->current_progress;
    local->anim_step = 0;
    local->anim_steps = 0;
    egui_view_invalidate(self);
}

const egui_view_animated_icon_source_t *egui_view_animated_icon_get_source(egui_view_t *self)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);

    return local->source;
}

void egui_view_animated_icon_set_state(egui_view_t *self, const char *state_name)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);
    uint8_t from_state = local->current_state;
    uint8_t to_state = EGUI_VIEW_ANIMATED_ICON_STATE_NORMAL;
    uint8_t has_transition = 0;

    egui_view_animated_icon_clear_pressed_state(self);

    has_transition = egui_view_animated_icon_resolve_transition(state_name, &from_state, &to_state);
    if (!has_transition && !egui_view_animated_icon_resolve_state_id(state_name, &to_state))
    {
        to_state = EGUI_VIEW_ANIMATED_ICON_STATE_NORMAL;
    }

    local->current_state = to_state;
    egui_view_animated_icon_stop_timer(self);

    if (!egui_view_animated_icon_source_is_valid(local->source) || !local->animation_enabled || !self->is_attached_to_window)
    {
        local->current_progress = egui_view_animated_icon_progress_for_state(local->source, to_state);
        local->from_progress = local->current_progress;
        local->to_progress = local->current_progress;
        local->anim_step = 0;
        local->anim_steps = 0;
        egui_view_invalidate(self);
        return;
    }

    local->from_progress = has_transition ? egui_view_animated_icon_progress_for_state(local->source, from_state) : local->current_progress;
    local->to_progress = egui_view_animated_icon_progress_for_state(local->source, to_state);
    local->current_progress = local->from_progress;
    local->anim_step = 0;
    local->anim_steps = EGUI_VIEW_ANIMATED_ICON_ANIM_STEPS;

    if (local->from_progress == local->to_progress)
    {
        local->current_progress = local->to_progress;
        local->anim_steps = 0;
    }
    else
    {
        egui_view_animated_icon_start_timer(self);
    }

    egui_view_invalidate(self);
}

const char *egui_view_animated_icon_get_state(egui_view_t *self)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);

    return egui_view_animated_icon_state_name_from_id(local->current_state);
}

void egui_view_animated_icon_set_fallback_glyph(egui_view_t *self, const char *glyph)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);

    egui_view_animated_icon_clear_pressed_state(self);
    if (EGUI_VIEW_ICON_TEXT_VALID(glyph))
    {
        local->fallback_glyph = glyph;
        local->fallback_overridden = 1;
    }
    else
    {
        local->fallback_glyph = egui_view_animated_icon_default_fallback_glyph(local->source);
        local->fallback_overridden = 0;
    }
    egui_view_invalidate(self);
}

const char *egui_view_animated_icon_get_fallback_glyph(egui_view_t *self)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);

    if (!EGUI_VIEW_ICON_TEXT_VALID(local->fallback_glyph))
    {
        local->fallback_glyph = egui_view_animated_icon_default_fallback_glyph(local->source);
        local->fallback_overridden = 0;
    }
    return local->fallback_glyph;
}

void egui_view_animated_icon_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);

    egui_view_animated_icon_clear_pressed_state(self);
    local->icon_font = font;
    egui_view_invalidate(self);
}

void egui_view_animated_icon_set_palette(egui_view_t *self, egui_color_t icon_color)
{
    egui_view_animated_icon_apply_color(self, icon_color);
}

void egui_view_animated_icon_set_animation_enabled(egui_view_t *self, uint8_t enabled)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);

    egui_view_animated_icon_clear_pressed_state(self);
    local->animation_enabled = enabled ? 1 : 0;

    if (!local->animation_enabled)
    {
        egui_view_animated_icon_stop_timer(self);
        local->current_progress = egui_view_animated_icon_progress_for_state(local->source, local->current_state);
        local->from_progress = local->current_progress;
        local->to_progress = local->current_progress;
        local->anim_step = 0;
        local->anim_steps = 0;
    }

    egui_view_invalidate(self);
}

uint8_t egui_view_animated_icon_get_animation_enabled(egui_view_t *self)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);

    return local->animation_enabled;
}

const egui_view_animated_icon_source_t *egui_view_animated_icon_get_back_source(void)
{
    return &egui_view_animated_icon_back_source;
}

const egui_view_animated_icon_source_t *egui_view_animated_icon_get_chevron_down_small_source(void)
{
    return &egui_view_animated_icon_chevron_down_small_source;
}

static void egui_view_animated_icon_draw_back(const egui_region_t *region, int32_t progress_q10, egui_color_t color, egui_alpha_t alpha)
{
    int32_t hover_q10 = egui_view_animated_icon_phase_between(progress_q10, 0, EGUI_VIEW_ANIMATED_ICON_PROGRESS_POINTER);
    int32_t press_q10 = egui_view_animated_icon_phase_between(progress_q10, EGUI_VIEW_ANIMATED_ICON_PROGRESS_POINTER, EGUI_VIEW_ANIMATED_ICON_PROGRESS_MAX);
    int32_t size = EGUI_MIN(region->size.width, region->size.height);
    int32_t cx = region->location.x + region->size.width / 2;
    int32_t cy = region->location.y + region->size.height / 2;
    int32_t stroke = EGUI_MAX(1, size / 9);
    int32_t tip_x;
    int32_t join_x;
    int32_t tail_x2;
    int32_t arm_dy;
    int32_t center_y;

    stroke = egui_view_animated_icon_lerp_i32(stroke, stroke + 1, hover_q10 / 2);
    stroke = egui_view_animated_icon_lerp_i32(stroke, stroke + 1, press_q10);
    center_y = egui_view_animated_icon_lerp_i32(cy, cy + EGUI_MAX(1, size / 28), press_q10);

    tip_x = egui_view_animated_icon_lerp_i32(cx - (size * 6) / 18, cx - (size * 7) / 18, hover_q10);
    tip_x = egui_view_animated_icon_lerp_i32(tip_x, cx - (size * 13) / 36, press_q10);
    join_x = egui_view_animated_icon_lerp_i32(cx - size / 18, cx - (size * 3) / 18, hover_q10);
    join_x = egui_view_animated_icon_lerp_i32(join_x, cx - (size * 2) / 18, press_q10 / 2);
    tail_x2 = egui_view_animated_icon_lerp_i32(cx + (size * 5) / 18, cx + (size * 4) / 18, hover_q10 / 2);
    tail_x2 = egui_view_animated_icon_lerp_i32(tail_x2, cx + (size * 7) / 36, press_q10);
    arm_dy = egui_view_animated_icon_lerp_i32((size * 4) / 18, (size * 5) / 18, hover_q10);
    arm_dy = egui_view_animated_icon_lerp_i32(arm_dy, (size * 7) / 36, press_q10);

    egui_canvas_draw_line_round_cap_hq(join_x, center_y, tail_x2, center_y, stroke, color, alpha);
    egui_canvas_draw_line_round_cap_hq(tip_x, center_y, join_x, center_y - arm_dy, stroke, color, alpha);
    egui_canvas_draw_line_round_cap_hq(tip_x, center_y, join_x, center_y + arm_dy, stroke, color, alpha);
}

static void egui_view_animated_icon_draw_chevron_down_small(const egui_region_t *region, int32_t progress_q10, egui_color_t color, egui_alpha_t alpha)
{
    int32_t hover_q10 = egui_view_animated_icon_phase_between(progress_q10, 0, EGUI_VIEW_ANIMATED_ICON_PROGRESS_POINTER_2);
    int32_t press_q10 =
            egui_view_animated_icon_phase_between(progress_q10, EGUI_VIEW_ANIMATED_ICON_PROGRESS_POINTER_2, EGUI_VIEW_ANIMATED_ICON_PROGRESS_MAX);
    int32_t size = EGUI_MIN(region->size.width, region->size.height);
    int32_t cx = region->location.x + region->size.width / 2;
    int32_t cy = region->location.y + region->size.height / 2;
    int32_t stroke = EGUI_MAX(1, size / 10);
    int32_t half_width;
    int32_t top_y;
    int32_t bottom_y;

    stroke = egui_view_animated_icon_lerp_i32(stroke, stroke + 1, hover_q10 / 2);
    stroke = egui_view_animated_icon_lerp_i32(stroke, stroke + 1, press_q10);
    half_width = egui_view_animated_icon_lerp_i32((size * 4) / 18, (size * 5) / 18, hover_q10 / 2);
    top_y = egui_view_animated_icon_lerp_i32(cy - size / 12, cy - size / 16, press_q10);
    top_y = egui_view_animated_icon_lerp_i32(top_y, top_y + EGUI_MAX(1, size / 24), hover_q10 / 2);
    bottom_y = egui_view_animated_icon_lerp_i32(cy + size / 10, cy + size / 8, hover_q10);
    bottom_y = egui_view_animated_icon_lerp_i32(bottom_y, cy + size / 12, press_q10);

    egui_canvas_draw_line_round_cap_hq(cx - half_width, top_y, cx, bottom_y, stroke, color, alpha);
    egui_canvas_draw_line_round_cap_hq(cx, bottom_y, cx + half_width, top_y, stroke, color, alpha);
}

static void egui_view_animated_icon_draw_fallback(egui_view_t *self, egui_view_animated_icon_t *local, const egui_region_t *region, egui_color_t color)
{
    const char *glyph = egui_view_animated_icon_get_fallback_glyph(self);
    const egui_font_t *font;
    egui_region_t draw_region;

    if (!EGUI_VIEW_ICON_TEXT_VALID(glyph))
    {
        return;
    }

    font = egui_view_animated_icon_resolve_font(local, EGUI_MIN(region->size.width, region->size.height));
    if (font == NULL)
    {
        return;
    }

    draw_region = *region;
    egui_canvas_draw_text_in_rect(font, glyph, &draw_region, EGUI_ALIGN_CENTER, color, self->alpha);
}

static void egui_view_animated_icon_on_draw(egui_view_t *self)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);
    egui_region_t region;
    egui_color_t color = local->icon_color;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    if (!egui_view_get_enable(self))
    {
        color = egui_rgb_mix(color, EGUI_COLOR_HEX(0x97A4B1), 58);
    }

    if (!egui_view_animated_icon_source_is_valid(local->source))
    {
        egui_view_animated_icon_draw_fallback(self, local, &region, color);
        return;
    }

    switch (local->source->visual)
    {
    case EGUI_VIEW_ANIMATED_ICON_VISUAL_CHEVRON_DOWN_SMALL:
        egui_view_animated_icon_draw_chevron_down_small(&region, local->current_progress, color, self->alpha);
        break;

    case EGUI_VIEW_ANIMATED_ICON_VISUAL_BACK:
    default:
        egui_view_animated_icon_draw_back(&region, local->current_progress, color, self->alpha);
        break;
    }
}

static void egui_view_animated_icon_on_attach(egui_view_t *self)
{
    egui_view_on_attach_to_window(self);
}

static void egui_view_animated_icon_on_detach(egui_view_t *self)
{
    egui_view_animated_icon_stop_timer(self);
    egui_view_on_detach_from_window(self);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_animated_icon_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_animated_icon_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_animated_icon_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_animated_icon_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_animated_icon_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_animated_icon_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_animated_icon_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_animated_icon_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
        .on_touch_event = egui_view_on_touch_event,
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_animated_icon_on_attach,
        .on_draw = egui_view_animated_icon_on_draw,
        .on_detach_from_window = egui_view_animated_icon_on_detach,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_animated_icon_init(egui_view_t *self)
{
    egui_view_animated_icon_t *local = egui_view_animated_icon_local(self);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_animated_icon_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->source = &egui_view_animated_icon_back_source;
    local->icon_font = NULL;
    local->fallback_glyph = egui_view_animated_icon_default_fallback_glyph(local->source);
    local->icon_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->current_state = EGUI_VIEW_ANIMATED_ICON_STATE_NORMAL;
    local->current_progress = 0;
    local->from_progress = 0;
    local->to_progress = 0;
    local->animation_enabled = 1;
    local->fallback_overridden = 0;
    local->anim_step = 0;
    local->anim_steps = 0;
    local->timer_started = 0;
    egui_timer_init_timer(&local->anim_timer, self, egui_view_animated_icon_tick);

    egui_view_set_view_name(self, "egui_view_animated_icon");
}
