#include "egui.h"
#include "core/egui_timer.h"
#include "egui_view_progress_bar.h"
#include "widget/egui_view_linear_value_helper.h"

#define HCW_PROGRESS_BAR_STATE_CAPACITY         12
#define HCW_PROGRESS_BAR_TIMER_MS               80
#define HCW_PROGRESS_BAR_PHASE_COUNT            24
#define HCW_PROGRESS_BAR_INDETERMINATE_MIN_PCT  24
#define HCW_PROGRESS_BAR_INDETERMINATE_MAX_PCT  42

typedef struct hcw_progress_bar_state hcw_progress_bar_state_t;
struct hcw_progress_bar_state
{
    egui_view_t *self;
    egui_timer_t anim_timer;
    uint8_t timer_started;
    uint8_t indeterminate_mode;
    uint8_t phase;
    uint8_t determinate_value;
};

static hcw_progress_bar_state_t g_hcw_progress_bar_states[HCW_PROGRESS_BAR_STATE_CAPACITY];

static egui_view_progress_bar_t *hcw_progress_bar_local(egui_view_t *self)
{
    return (egui_view_progress_bar_t *)self;
}

static uint8_t hcw_progress_bar_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static hcw_progress_bar_state_t *hcw_progress_bar_find_state(egui_view_t *self)
{
    uint8_t i;

    for (i = 0; i < HCW_PROGRESS_BAR_STATE_CAPACITY; i++)
    {
        if (g_hcw_progress_bar_states[i].self == self)
        {
            return &g_hcw_progress_bar_states[i];
        }
    }

    return NULL;
}

static void hcw_progress_bar_tick(egui_timer_t *timer);

static hcw_progress_bar_state_t *hcw_progress_bar_alloc_state(egui_view_t *self)
{
    uint8_t i;

    for (i = 0; i < HCW_PROGRESS_BAR_STATE_CAPACITY; i++)
    {
        if (g_hcw_progress_bar_states[i].self == NULL)
        {
            g_hcw_progress_bar_states[i].self = self;
            g_hcw_progress_bar_states[i].phase = 0;
            g_hcw_progress_bar_states[i].timer_started = 0;
            g_hcw_progress_bar_states[i].indeterminate_mode = 0;
            g_hcw_progress_bar_states[i].determinate_value = hcw_progress_bar_local(self)->process;
            egui_timer_init_timer(&g_hcw_progress_bar_states[i].anim_timer, self, hcw_progress_bar_tick);
            return &g_hcw_progress_bar_states[i];
        }
    }

    return NULL;
}

static void hcw_progress_bar_stop_timer_state(hcw_progress_bar_state_t *state)
{
    if (state == NULL || !state->timer_started)
    {
        return;
    }

    egui_timer_stop_timer(&state->anim_timer);
    state->timer_started = 0;
}

static void hcw_progress_bar_stop_timer(egui_view_t *self)
{
    hcw_progress_bar_stop_timer_state(hcw_progress_bar_find_state(self));
}

static void hcw_progress_bar_start_timer(egui_view_t *self)
{
    hcw_progress_bar_state_t *state = hcw_progress_bar_find_state(self);

    if (state == NULL || state->timer_started || !state->indeterminate_mode || !self->is_attached_to_window)
    {
        return;
    }

    egui_timer_start_timer(&state->anim_timer, HCW_PROGRESS_BAR_TIMER_MS, HCW_PROGRESS_BAR_TIMER_MS);
    state->timer_started = 1;
}

static void hcw_progress_bar_on_attach(egui_view_t *self);
static void hcw_progress_bar_on_detach(egui_view_t *self);
static void hcw_progress_bar_on_draw(egui_view_t *self);

static const egui_view_api_t hcw_progress_bar_api = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
        .on_touch_event = egui_view_on_touch_event,
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = hcw_progress_bar_on_attach,
        .on_draw = hcw_progress_bar_on_draw,
        .on_detach_from_window = hcw_progress_bar_on_detach,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

static hcw_progress_bar_state_t *hcw_progress_bar_ensure_state(egui_view_t *self)
{
    hcw_progress_bar_state_t *state = hcw_progress_bar_find_state(self);

    if (state == NULL)
    {
        state = hcw_progress_bar_alloc_state(self);
    }
    if (state != NULL)
    {
        self->api = &hcw_progress_bar_api;
    }

    return state;
}

static hcw_progress_bar_state_t *hcw_progress_bar_reset_state(egui_view_t *self)
{
    hcw_progress_bar_state_t *state = hcw_progress_bar_find_state(self);

    if (state == NULL)
    {
        state = hcw_progress_bar_alloc_state(self);
    }
    if (state == NULL)
    {
        return NULL;
    }

    hcw_progress_bar_stop_timer_state(state);
    state->self = self;
    state->phase = 0;
    state->timer_started = 0;
    state->indeterminate_mode = 0;
    state->determinate_value = hcw_progress_bar_local(self)->process;
    egui_timer_init_timer(&state->anim_timer, self, hcw_progress_bar_tick);
    self->api = &hcw_progress_bar_api;
    return state;
}

static egui_dim_t hcw_progress_bar_get_indeterminate_width(const egui_view_linear_value_metrics_t *metrics, uint8_t phase)
{
    uint8_t local_phase = (uint8_t)(phase % HCW_PROGRESS_BAR_PHASE_COUNT);
    uint8_t up_phase_count = HCW_PROGRESS_BAR_PHASE_COUNT / 2;
    egui_dim_t min_width;
    egui_dim_t max_width;
    egui_dim_t min_visual_width = (egui_dim_t)(metrics->track_height * 3);
    egui_dim_t width;

    min_width = (egui_dim_t)((uint32_t)metrics->usable_width * HCW_PROGRESS_BAR_INDETERMINATE_MIN_PCT / 100);
    max_width = (egui_dim_t)((uint32_t)metrics->usable_width * HCW_PROGRESS_BAR_INDETERMINATE_MAX_PCT / 100);

    if (min_width < min_visual_width)
    {
        min_width = min_visual_width;
    }
    if (max_width < min_width)
    {
        max_width = min_width;
    }
    if (max_width > metrics->usable_width)
    {
        max_width = metrics->usable_width;
    }

    if (up_phase_count <= 1 || max_width <= min_width)
    {
        return max_width;
    }

    if (local_phase < up_phase_count)
    {
        width = (egui_dim_t)(min_width + ((uint32_t)(max_width - min_width) * local_phase) / (up_phase_count - 1));
    }
    else
    {
        local_phase = (uint8_t)(local_phase - up_phase_count);
        width = (egui_dim_t)(max_width - ((uint32_t)(max_width - min_width) * local_phase) / (up_phase_count - 1));
    }

    return width;
}

static void hcw_progress_bar_draw_indeterminate(egui_view_t *self, hcw_progress_bar_state_t *state)
{
    egui_view_progress_bar_t *local = hcw_progress_bar_local(self);
    egui_region_t region;
    egui_view_linear_value_metrics_t metrics;
    egui_dim_t segment_width;
    int32_t raw_start;
    int32_t raw_end;
    int32_t visible_start;
    int32_t visible_end;
    egui_dim_t visible_width;

    EGUI_UNUSED(state);
    egui_view_get_work_region(self, &region);

    if (!egui_view_linear_value_get_metrics(&region, 0, &metrics))
    {
        return;
    }

#if EGUI_CONFIG_WIDGET_ENHANCED_DRAW
    egui_canvas_draw_round_rectangle_fill(metrics.start_x, metrics.track_y, metrics.usable_width, metrics.track_height, metrics.track_radius, local->bk_color,
                                          EGUI_ALPHA_100);
#else
    egui_canvas_draw_round_rectangle_fill(metrics.start_x, metrics.track_y, metrics.usable_width, metrics.track_height, metrics.track_radius, local->bk_color,
                                          EGUI_ALPHA_100);
#endif

    segment_width = hcw_progress_bar_get_indeterminate_width(&metrics, state->phase);
    raw_start = (int32_t)metrics.start_x - segment_width +
                ((int32_t)(metrics.usable_width + segment_width) * state->phase) / (HCW_PROGRESS_BAR_PHASE_COUNT - 1);
    raw_end = raw_start + segment_width;
    visible_start = raw_start < metrics.start_x ? metrics.start_x : raw_start;
    visible_end = raw_end > ((int32_t)metrics.start_x + metrics.usable_width) ? ((int32_t)metrics.start_x + metrics.usable_width) : raw_end;

    if (visible_end <= visible_start)
    {
        return;
    }

    visible_width = (egui_dim_t)(visible_end - visible_start);
#if EGUI_CONFIG_WIDGET_ENHANCED_DRAW
    egui_canvas_draw_round_rectangle_fill((egui_dim_t)visible_start, metrics.track_y, visible_width, metrics.track_height, metrics.track_radius,
                                          local->progress_color, EGUI_ALPHA_100);
#else
    egui_canvas_draw_round_rectangle_fill((egui_dim_t)visible_start, metrics.track_y, visible_width, metrics.track_height, metrics.track_radius,
                                          local->progress_color, EGUI_ALPHA_100);
#endif
}

static void hcw_progress_bar_tick(egui_timer_t *timer)
{
    egui_view_t *self = (egui_view_t *)timer->user_data;
    hcw_progress_bar_state_t *state = hcw_progress_bar_find_state(self);

    if (state == NULL || !state->indeterminate_mode || !self->is_attached_to_window)
    {
        hcw_progress_bar_stop_timer(self);
        return;
    }

    state->phase = (uint8_t)((state->phase + 1) % HCW_PROGRESS_BAR_PHASE_COUNT);
    egui_view_invalidate(self);
}

static void hcw_progress_bar_on_attach(egui_view_t *self)
{
    egui_view_on_attach_to_window(self);
    hcw_progress_bar_start_timer(self);
}

static void hcw_progress_bar_on_detach(egui_view_t *self)
{
    hcw_progress_bar_stop_timer(self);
    egui_view_on_detach_from_window(self);
}

static void hcw_progress_bar_on_draw(egui_view_t *self)
{
    hcw_progress_bar_state_t *state = hcw_progress_bar_find_state(self);

    if (state != NULL && state->indeterminate_mode)
    {
        hcw_progress_bar_draw_indeterminate(self, state);
        return;
    }

    egui_view_progress_bar_on_draw(self);
}

static void hcw_progress_bar_apply_style(egui_view_t *self, egui_color_t track_color, egui_color_t fill_color, egui_color_t control_color)
{
    egui_view_progress_bar_t *local = hcw_progress_bar_local(self);
    hcw_progress_bar_state_t *state = hcw_progress_bar_ensure_state(self);

    hcw_progress_bar_clear_pressed_state(self);
    egui_view_set_shadow(self, NULL);
    egui_view_set_background(self, NULL);
    egui_view_set_enable(self, 1);
    self->padding.left = 0;
    self->padding.right = 0;
    self->padding.top = 0;
    self->padding.bottom = 0;
    local->bk_color = track_color;
    local->progress_color = fill_color;
    local->control_color = control_color;
    local->is_show_control = 0;
    if (state != NULL && !state->indeterminate_mode)
    {
        state->determinate_value = local->process;
    }
    egui_view_invalidate(self);
}

void hcw_progress_bar_init(egui_view_t *self)
{
    egui_view_progress_bar_init(self);
    hcw_progress_bar_reset_state(self);
}

void hcw_progress_bar_apply_standard_style(egui_view_t *self)
{
    hcw_progress_bar_apply_style(self, EGUI_COLOR_HEX(0xD8E1EA), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F6CBD));
    hcw_progress_bar_set_indeterminate_mode(self, 0);
}

void hcw_progress_bar_apply_paused_style(egui_view_t *self)
{
    hcw_progress_bar_apply_style(self, EGUI_COLOR_HEX(0xE7DDCA), EGUI_COLOR_HEX(0xB95A00), EGUI_COLOR_HEX(0xB95A00));
    hcw_progress_bar_set_indeterminate_mode(self, 0);
}

void hcw_progress_bar_apply_error_style(egui_view_t *self)
{
    hcw_progress_bar_apply_style(self, EGUI_COLOR_HEX(0xEED6DA), EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0xC42B1C));
    hcw_progress_bar_set_indeterminate_mode(self, 0);
}

void hcw_progress_bar_apply_indeterminate_style(egui_view_t *self)
{
    hcw_progress_bar_apply_style(self, EGUI_COLOR_HEX(0xD8E1EA), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F6CBD));
    hcw_progress_bar_set_indeterminate_mode(self, 1);
}

void hcw_progress_bar_set_value(egui_view_t *self, uint8_t value)
{
    hcw_progress_bar_state_t *state = hcw_progress_bar_ensure_state(self);
    uint8_t was_indeterminate = 0;

    if (value > 100)
    {
        value = 100;
    }

    hcw_progress_bar_clear_pressed_state(self);
    if (state != NULL)
    {
        was_indeterminate = state->indeterminate_mode;
        state->determinate_value = value;
        if (state->indeterminate_mode)
        {
            state->indeterminate_mode = 0;
            hcw_progress_bar_stop_timer_state(state);
        }
    }

    egui_view_progress_bar_set_process(self, value);
    if (state != NULL)
    {
        state->determinate_value = hcw_progress_bar_local(self)->process;
    }
    if (was_indeterminate)
    {
        egui_view_invalidate(self);
    }
}

void hcw_progress_bar_set_palette(egui_view_t *self, egui_color_t track_color, egui_color_t fill_color, egui_color_t control_color)
{
    egui_view_progress_bar_t *local = hcw_progress_bar_local(self);

    hcw_progress_bar_ensure_state(self);
    hcw_progress_bar_clear_pressed_state(self);
    local->bk_color = track_color;
    local->progress_color = fill_color;
    local->control_color = control_color;
    egui_view_invalidate(self);
}

void hcw_progress_bar_set_indeterminate_mode(egui_view_t *self, uint8_t enable)
{
    hcw_progress_bar_state_t *state = hcw_progress_bar_ensure_state(self);

    if (state == NULL)
    {
        return;
    }

    enable = (uint8_t)(enable ? 1 : 0);
    hcw_progress_bar_clear_pressed_state(self);

    if (enable == state->indeterminate_mode)
    {
        if (enable)
        {
            egui_view_invalidate(self);
            hcw_progress_bar_start_timer(self);
        }
        return;
    }

    if (enable)
    {
        state->determinate_value = hcw_progress_bar_local(self)->process;
        state->phase = 0;
        state->indeterminate_mode = 1;
        egui_view_invalidate(self);
        hcw_progress_bar_start_timer(self);
        return;
    }

    state->indeterminate_mode = 0;
    hcw_progress_bar_stop_timer_state(state);
    egui_view_progress_bar_set_process(self, state->determinate_value);
    egui_view_invalidate(self);
}

uint8_t hcw_progress_bar_get_indeterminate_mode(egui_view_t *self)
{
    hcw_progress_bar_state_t *state = hcw_progress_bar_ensure_state(self);

    if (state == NULL)
    {
        return 0;
    }

    return state->indeterminate_mode;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_progress_bar_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_progress_bar_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_progress_bar_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_progress_bar_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_progress_bar_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    hcw_progress_bar_ensure_state(self);
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_progress_bar_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_progress_bar_on_static_key_event;
#endif
}
