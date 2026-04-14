#include "egui.h"
#include "core/egui_timer.h"
#include "egui_view_activity_ring.h"

#define HCW_ACTIVITY_RING_STATE_CAPACITY         12
#define HCW_ACTIVITY_RING_TIMER_MS               80
#define HCW_ACTIVITY_RING_PHASE_COUNT            24
#define HCW_ACTIVITY_RING_ROTATION_STEP_DEGREES  15
#define HCW_ACTIVITY_RING_INDETERMINATE_MIN      22
#define HCW_ACTIVITY_RING_INDETERMINATE_MAX      36

typedef struct hcw_activity_ring_state hcw_activity_ring_state_t;
struct hcw_activity_ring_state
{
    egui_view_t *self;
    egui_timer_t anim_timer;
    uint8_t timer_started;
    uint8_t indeterminate_mode;
    uint8_t phase;
    uint8_t determinate_value;
    int16_t base_start_angle;
};

static egui_view_activity_ring_t *hcw_activity_ring_local(egui_view_t *self)
{
    return (egui_view_activity_ring_t *)self;
}

static hcw_activity_ring_state_t g_hcw_activity_ring_states[HCW_ACTIVITY_RING_STATE_CAPACITY];

static uint8_t hcw_activity_ring_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static hcw_activity_ring_state_t *hcw_activity_ring_find_state(egui_view_t *self)
{
    uint8_t i;

    for (i = 0; i < HCW_ACTIVITY_RING_STATE_CAPACITY; i++)
    {
        if (g_hcw_activity_ring_states[i].self == self)
        {
            return &g_hcw_activity_ring_states[i];
        }
    }

    return NULL;
}

static void hcw_activity_ring_tick(egui_timer_t *timer);

static hcw_activity_ring_state_t *hcw_activity_ring_alloc_state(egui_view_t *self)
{
    uint8_t i;

    for (i = 0; i < HCW_ACTIVITY_RING_STATE_CAPACITY; i++)
    {
        if (g_hcw_activity_ring_states[i].self == NULL)
        {
            g_hcw_activity_ring_states[i].self = self;
            g_hcw_activity_ring_states[i].phase = 0;
            g_hcw_activity_ring_states[i].timer_started = 0;
            g_hcw_activity_ring_states[i].indeterminate_mode = 0;
            g_hcw_activity_ring_states[i].determinate_value = egui_view_activity_ring_get_value(self, 0);
            g_hcw_activity_ring_states[i].base_start_angle = hcw_activity_ring_local(self)->start_angle;
            egui_timer_init_timer(&g_hcw_activity_ring_states[i].anim_timer, self, hcw_activity_ring_tick);
            return &g_hcw_activity_ring_states[i];
        }
    }

    return NULL;
}

static void hcw_activity_ring_stop_timer_state(hcw_activity_ring_state_t *state)
{
    if (state == NULL || !state->timer_started)
    {
        return;
    }

    egui_timer_stop_timer(&state->anim_timer);
    state->timer_started = 0;
}

static void hcw_activity_ring_stop_timer(egui_view_t *self)
{
    hcw_activity_ring_stop_timer_state(hcw_activity_ring_find_state(self));
}

static void hcw_activity_ring_start_timer(egui_view_t *self)
{
    hcw_activity_ring_state_t *state = hcw_activity_ring_find_state(self);

    if (state == NULL || state->timer_started || !state->indeterminate_mode || !self->is_attached_to_window)
    {
        return;
    }

    egui_timer_start_timer(&state->anim_timer, HCW_ACTIVITY_RING_TIMER_MS, HCW_ACTIVITY_RING_TIMER_MS);
    state->timer_started = 1;
}

static void hcw_activity_ring_on_attach(egui_view_t *self);
static void hcw_activity_ring_on_detach(egui_view_t *self);

static const egui_view_api_t hcw_activity_ring_api = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
        .on_touch_event = egui_view_on_touch_event,
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = hcw_activity_ring_on_attach,
        .on_draw = egui_view_activity_ring_on_draw,
        .on_detach_from_window = hcw_activity_ring_on_detach,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

static hcw_activity_ring_state_t *hcw_activity_ring_ensure_state(egui_view_t *self)
{
    hcw_activity_ring_state_t *state = hcw_activity_ring_find_state(self);

    if (state == NULL)
    {
        state = hcw_activity_ring_alloc_state(self);
    }
    if (state != NULL)
    {
        self->api = &hcw_activity_ring_api;
    }

    return state;
}

static hcw_activity_ring_state_t *hcw_activity_ring_reset_state(egui_view_t *self)
{
    hcw_activity_ring_state_t *state = hcw_activity_ring_find_state(self);

    if (state == NULL)
    {
        state = hcw_activity_ring_alloc_state(self);
    }
    if (state == NULL)
    {
        return NULL;
    }

    hcw_activity_ring_stop_timer_state(state);
    state->self = self;
    state->indeterminate_mode = 0;
    state->phase = 0;
    state->timer_started = 0;
    state->determinate_value = egui_view_activity_ring_get_value(self, 0);
    state->base_start_angle = hcw_activity_ring_local(self)->start_angle;
    egui_timer_init_timer(&state->anim_timer, self, hcw_activity_ring_tick);
    self->api = &hcw_activity_ring_api;
    return state;
}

static uint8_t hcw_activity_ring_get_indeterminate_value(uint8_t phase)
{
    uint8_t local_phase = (uint8_t)(phase % HCW_ACTIVITY_RING_PHASE_COUNT);
    uint8_t up_phase_count = HCW_ACTIVITY_RING_PHASE_COUNT / 2;
    uint8_t range = HCW_ACTIVITY_RING_INDETERMINATE_MAX - HCW_ACTIVITY_RING_INDETERMINATE_MIN;

    if (local_phase < up_phase_count)
    {
        return (uint8_t)(HCW_ACTIVITY_RING_INDETERMINATE_MIN + ((uint16_t)range * local_phase) / (up_phase_count - 1));
    }

    local_phase = (uint8_t)(local_phase - up_phase_count);
    return (uint8_t)(HCW_ACTIVITY_RING_INDETERMINATE_MAX - ((uint16_t)range * local_phase) / (up_phase_count - 1));
}

static void hcw_activity_ring_apply_indeterminate_frame(egui_view_t *self, hcw_activity_ring_state_t *state)
{
    egui_view_activity_ring_t *local;
    uint8_t phase;

    if (state == NULL)
    {
        return;
    }

    local = hcw_activity_ring_local(self);
    phase = (uint8_t)(state->phase % HCW_ACTIVITY_RING_PHASE_COUNT);
    local->ring_count = 1;
    local->values[0] = hcw_activity_ring_get_indeterminate_value(phase);
    local->values[1] = 0;
    local->values[2] = 0;
    local->start_angle = (int16_t)(state->base_start_angle + phase * HCW_ACTIVITY_RING_ROTATION_STEP_DEGREES);
    egui_view_invalidate(self);
}

static void hcw_activity_ring_restore_determinate_state(egui_view_t *self, hcw_activity_ring_state_t *state)
{
    egui_view_activity_ring_t *local;

    if (state == NULL)
    {
        return;
    }

    local = hcw_activity_ring_local(self);
    local->start_angle = state->base_start_angle;
    egui_view_activity_ring_set_value(self, 0, state->determinate_value);
}

static void hcw_activity_ring_tick(egui_timer_t *timer)
{
    egui_view_t *self = (egui_view_t *)timer->user_data;
    hcw_activity_ring_state_t *state = hcw_activity_ring_find_state(self);

    if (state == NULL || !state->indeterminate_mode || !self->is_attached_to_window)
    {
        hcw_activity_ring_stop_timer(self);
        return;
    }

    state->phase = (uint8_t)((state->phase + 1) % HCW_ACTIVITY_RING_PHASE_COUNT);
    hcw_activity_ring_apply_indeterminate_frame(self, state);
}

static void hcw_activity_ring_on_attach(egui_view_t *self)
{
    egui_view_on_attach_to_window(self);
    hcw_activity_ring_start_timer(self);
}

static void hcw_activity_ring_on_detach(egui_view_t *self)
{
    hcw_activity_ring_stop_timer(self);
    egui_view_on_detach_from_window(self);
}

static void hcw_activity_ring_apply_style(egui_view_t *self, egui_dim_t stroke_width, egui_color_t ring_color, egui_color_t ring_bg_color)
{
    egui_view_activity_ring_t *local = hcw_activity_ring_local(self);
    hcw_activity_ring_state_t *state = hcw_activity_ring_ensure_state(self);

    hcw_activity_ring_clear_pressed_state(self);
    egui_view_set_shadow(self, NULL);
    egui_view_set_background(self, NULL);
    egui_view_set_enable(self, 1);
    self->padding.left = 0;
    self->padding.right = 0;
    self->padding.top = 0;
    self->padding.bottom = 0;
    local->ring_count = 1;
    local->stroke_width = stroke_width;
    local->ring_gap = 0;
    local->start_angle = -90;
    local->show_round_cap = 1;
    local->ring_colors[0] = ring_color;
    local->ring_bg_colors[0] = ring_bg_color;
    local->values[1] = 0;
    local->values[2] = 0;
    if (state != NULL)
    {
        state->base_start_angle = local->start_angle;
        if (!state->indeterminate_mode)
        {
            state->determinate_value = local->values[0];
        }
    }
    egui_view_invalidate(self);
}

void hcw_activity_ring_init(egui_view_t *self)
{
    egui_view_activity_ring_init(self);
    hcw_activity_ring_reset_state(self);
}

void hcw_activity_ring_apply_standard_style(egui_view_t *self)
{
    hcw_activity_ring_apply_style(self, 8, EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xD8E1EA));
    hcw_activity_ring_set_indeterminate_mode(self, 0);
}

void hcw_activity_ring_apply_compact_style(egui_view_t *self)
{
    hcw_activity_ring_apply_style(self, 6, EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xD8E1EA));
    hcw_activity_ring_set_indeterminate_mode(self, 0);
}

void hcw_activity_ring_apply_paused_style(egui_view_t *self)
{
    hcw_activity_ring_apply_style(self, 6, EGUI_COLOR_HEX(0xB95A00), EGUI_COLOR_HEX(0xE7DDCA));
    hcw_activity_ring_set_indeterminate_mode(self, 0);
}

void hcw_activity_ring_apply_indeterminate_style(egui_view_t *self)
{
    hcw_activity_ring_apply_style(self, 8, EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xD8E1EA));
    hcw_activity_ring_set_indeterminate_mode(self, 1);
}

void hcw_activity_ring_set_value(egui_view_t *self, uint8_t value)
{
    hcw_activity_ring_state_t *state = hcw_activity_ring_ensure_state(self);

    if (value > 100)
    {
        value = 100;
    }

    hcw_activity_ring_clear_pressed_state(self);
    if (state != NULL)
    {
        state->determinate_value = value;
        if (state->indeterminate_mode)
        {
            state->indeterminate_mode = 0;
            hcw_activity_ring_stop_timer_state(state);
            hcw_activity_ring_local(self)->start_angle = state->base_start_angle;
        }
    }
    egui_view_activity_ring_set_value(self, 0, value);
}

void hcw_activity_ring_set_palette(egui_view_t *self, egui_color_t ring_color, egui_color_t ring_bg_color)
{
    egui_view_activity_ring_t *local = hcw_activity_ring_local(self);

    hcw_activity_ring_ensure_state(self);
    hcw_activity_ring_clear_pressed_state(self);
    local->ring_colors[0] = ring_color;
    local->ring_bg_colors[0] = ring_bg_color;
    egui_view_invalidate(self);
}

void hcw_activity_ring_set_indeterminate_mode(egui_view_t *self, uint8_t enable)
{
    hcw_activity_ring_state_t *state = hcw_activity_ring_ensure_state(self);

    if (state == NULL)
    {
        return;
    }

    enable = (uint8_t)(enable ? 1 : 0);
    hcw_activity_ring_clear_pressed_state(self);

    if (enable == state->indeterminate_mode)
    {
        if (enable)
        {
            hcw_activity_ring_apply_indeterminate_frame(self, state);
            hcw_activity_ring_start_timer(self);
        }
        return;
    }

    if (enable)
    {
        state->determinate_value = egui_view_activity_ring_get_value(self, 0);
        state->phase = 0;
        state->indeterminate_mode = 1;
        hcw_activity_ring_apply_indeterminate_frame(self, state);
        hcw_activity_ring_start_timer(self);
        return;
    }

    state->indeterminate_mode = 0;
    hcw_activity_ring_stop_timer_state(state);
    hcw_activity_ring_restore_determinate_state(self, state);
}

uint8_t hcw_activity_ring_get_indeterminate_mode(egui_view_t *self)
{
    hcw_activity_ring_state_t *state = hcw_activity_ring_ensure_state(self);

    if (state == NULL)
    {
        return 0;
    }

    return state->indeterminate_mode;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_activity_ring_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_activity_ring_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_activity_ring_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_activity_ring_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_activity_ring_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    hcw_activity_ring_ensure_state(self);
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_activity_ring_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_activity_ring_on_static_key_event;
#endif
}
