#include "egui_view_segmented_control.h"

static egui_view_segmented_control_t *hcw_segmented_control_local(egui_view_t *self)
{
    return (egui_view_segmented_control_t *)self;
}

static uint8_t hcw_segmented_control_clamp_count(uint8_t count)
{
    if (count > EGUI_VIEW_SEGMENTED_CONTROL_MAX_SEGMENTS)
    {
        return EGUI_VIEW_SEGMENTED_CONTROL_MAX_SEGMENTS;
    }
    return count;
}

static uint8_t hcw_segmented_control_clear_pressed_state(egui_view_t *self)
{
    egui_view_segmented_control_t *local = hcw_segmented_control_local(self);
    uint8_t had_pressed = egui_view_get_pressed(self) || local->pressed_index != EGUI_VIEW_SEGMENTED_CONTROL_PRESSED_NONE;

    if (had_pressed)
    {
        local->pressed_index = EGUI_VIEW_SEGMENTED_CONTROL_PRESSED_NONE;
        egui_view_set_pressed(self, 0);
        egui_view_invalidate(self);
    }
    return had_pressed;
}

static void hcw_segmented_control_apply_style(egui_view_t *self, uint8_t corner_radius, uint8_t segment_gap, uint8_t horizontal_padding, egui_color_t bg_color,
                                              egui_color_t selected_bg_color, egui_color_t text_color, egui_color_t selected_text_color,
                                              egui_color_t border_color)
{
    egui_view_segmented_control_t *local = hcw_segmented_control_local(self);

    hcw_segmented_control_clear_pressed_state(self);
    local->corner_radius = corner_radius;
    local->segment_gap = segment_gap;
    local->horizontal_padding = horizontal_padding;
    local->bg_color = bg_color;
    local->selected_bg_color = selected_bg_color;
    local->text_color = text_color;
    local->selected_text_color = selected_text_color;
    local->border_color = border_color;
    egui_view_invalidate(self);
}

static int hcw_segmented_control_resolve_segment_gap(int content_width, uint8_t count, int gap)
{
    if (count <= 1)
    {
        return 0;
    }
    while (gap > 0 && (content_width - gap * (count - 1)) < count)
    {
        gap--;
    }
    return gap;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static uint8_t hcw_segmented_control_get_hit_index(egui_view_t *self, egui_dim_t local_x, egui_dim_t local_y)
{
    egui_view_segmented_control_t *local = hcw_segmented_control_local(self);
    egui_region_t work_region;
    egui_dim_t padding;
    egui_dim_t gap;
    egui_dim_t content_x;
    egui_dim_t content_y;
    egui_dim_t content_width;
    egui_dim_t content_height;
    egui_dim_t available_width;
    egui_dim_t base_width;
    egui_dim_t remainder;
    egui_dim_t cursor_x;
    uint8_t count;
    uint8_t i;

    egui_view_get_work_region(self, &work_region);
    if (work_region.size.width <= 0 || work_region.size.height <= 0)
    {
        return EGUI_VIEW_SEGMENTED_CONTROL_PRESSED_NONE;
    }

    count = hcw_segmented_control_clamp_count(local->segment_count);
    if (count == 0)
    {
        return EGUI_VIEW_SEGMENTED_CONTROL_PRESSED_NONE;
    }

    padding = local->horizontal_padding;
    if (padding * 2 >= work_region.size.width || padding * 2 >= work_region.size.height)
    {
        padding = 0;
    }

    content_x = work_region.location.x + padding;
    content_y = work_region.location.y + padding;
    content_width = work_region.size.width - padding * 2;
    content_height = work_region.size.height - padding * 2;
    if (content_width <= 0 || content_height <= 0)
    {
        return EGUI_VIEW_SEGMENTED_CONTROL_PRESSED_NONE;
    }

    if (local_x < content_x || local_y < content_y || local_x >= content_x + content_width || local_y >= content_y + content_height)
    {
        return EGUI_VIEW_SEGMENTED_CONTROL_PRESSED_NONE;
    }

    gap = hcw_segmented_control_resolve_segment_gap(content_width, count, local->segment_gap);
    available_width = content_width - gap * (count - 1);
    if (available_width < count)
    {
        return EGUI_VIEW_SEGMENTED_CONTROL_PRESSED_NONE;
    }

    base_width = available_width / count;
    remainder = available_width % count;
    cursor_x = content_x;
    for (i = 0; i < count; i++)
    {
        egui_dim_t segment_width = base_width;

        if (remainder > 0)
        {
            segment_width++;
            remainder--;
        }
        if (local_x >= cursor_x && local_x < cursor_x + segment_width)
        {
            return i;
        }
        cursor_x += segment_width + gap;
    }

    return EGUI_VIEW_SEGMENTED_CONTROL_PRESSED_NONE;
}

static int hcw_segmented_control_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    egui_view_segmented_control_t *local = hcw_segmented_control_local(self);
    egui_dim_t local_x;
    egui_dim_t local_y;
    uint8_t hit_index;
    uint8_t had_pressed;
    int is_inside_view;

    if (!egui_view_get_enable(self) || local->segment_count == 0 || (local->segment_texts == NULL && local->segment_icons == NULL))
    {
        hcw_segmented_control_clear_pressed_state(self);
        return 0;
    }

    if (event->type == EGUI_MOTION_EVENT_ACTION_CANCEL)
    {
        return hcw_segmented_control_clear_pressed_state(self);
    }

    local_x = event->location.x - self->region_screen.location.x;
    local_y = event->location.y - self->region_screen.location.y;
    hit_index = hcw_segmented_control_get_hit_index(self, local_x, local_y);
    is_inside_view = egui_region_pt_in_rect(&self->region_screen, event->location.x, event->location.y);

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        if (hit_index == EGUI_VIEW_SEGMENTED_CONTROL_PRESSED_NONE)
        {
            hcw_segmented_control_clear_pressed_state(self);
            return is_inside_view;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        if (local->pressed_index != hit_index)
        {
            local->pressed_index = hit_index;
            egui_view_invalidate(self);
        }
        egui_view_set_pressed(self, 1);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_index == EGUI_VIEW_SEGMENTED_CONTROL_PRESSED_NONE)
        {
            hcw_segmented_control_clear_pressed_state(self);
            return is_inside_view;
        }
        egui_view_set_pressed(self, local->pressed_index == hit_index);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
        had_pressed = egui_view_get_pressed(self) || local->pressed_index != EGUI_VIEW_SEGMENTED_CONTROL_PRESSED_NONE;
        if (egui_view_get_pressed(self) && local->pressed_index != EGUI_VIEW_SEGMENTED_CONTROL_PRESSED_NONE && local->pressed_index == hit_index)
        {
            hcw_segmented_control_set_current_index(self, hit_index);
        }
        else
        {
            hcw_segmented_control_clear_pressed_state(self);
        }
        return had_pressed || is_inside_view;
    default:
        return 0;
    }
}

static int hcw_segmented_control_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_segmented_control_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_segmented_control_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    egui_view_segmented_control_t *local = hcw_segmented_control_local(self);
    uint8_t count = hcw_segmented_control_clamp_count(local->segment_count);
    uint8_t next_index;

    hcw_segmented_control_clear_pressed_state(self);
    if (!egui_view_get_enable(self) || count == 0 || (local->segment_texts == NULL && local->segment_icons == NULL))
    {
        return 0;
    }

    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        return (event->type == EGUI_KEY_EVENT_ACTION_DOWN || event->type == EGUI_KEY_EVENT_ACTION_UP) ? 1 : 0;
    }

    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    next_index = local->current_index;
    if (next_index >= count)
    {
        next_index = 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_LEFT:
    case EGUI_KEY_CODE_UP:
        if (next_index > 0)
        {
            next_index--;
        }
        hcw_segmented_control_set_current_index(self, next_index);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
    case EGUI_KEY_CODE_DOWN:
        if (next_index + 1 < count)
        {
            next_index++;
        }
        hcw_segmented_control_set_current_index(self, next_index);
        return 1;
    case EGUI_KEY_CODE_HOME:
        hcw_segmented_control_set_current_index(self, 0);
        return 1;
    case EGUI_KEY_CODE_END:
        hcw_segmented_control_set_current_index(self, count - 1);
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_index++;
        if (next_index >= count)
        {
            next_index = 0;
        }
        hcw_segmented_control_set_current_index(self, next_index);
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int hcw_segmented_control_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_segmented_control_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_segmented_control_apply_standard_style(egui_view_t *self)
{
    hcw_segmented_control_apply_style(self, 10, 2, 2, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0x2563EB), EGUI_COLOR_HEX(0x1E2933), EGUI_COLOR_HEX(0xFFFFFF),
                                      EGUI_COLOR_HEX(0xD7DEE7));
}

void hcw_segmented_control_apply_compact_style(egui_view_t *self)
{
    hcw_segmented_control_apply_style(self, 8, 1, 1, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0x0C7C73), EGUI_COLOR_HEX(0x203039), EGUI_COLOR_HEX(0xFFFFFF),
                                      EGUI_COLOR_HEX(0xD3DEDA));
}

void hcw_segmented_control_apply_read_only_style(egui_view_t *self)
{
    hcw_segmented_control_apply_style(self, 8, 1, 1, EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0x97A4B4), EGUI_COLOR_HEX(0x5B6976), EGUI_COLOR_HEX(0xFFFFFF),
                                      EGUI_COLOR_HEX(0xDBE2E8));
}

void hcw_segmented_control_set_segments(egui_view_t *self, const char **segment_texts, uint8_t segment_count)
{
    hcw_segmented_control_clear_pressed_state(self);
    egui_view_segmented_control_set_segments(self, segment_texts, segment_count);
}

void hcw_segmented_control_set_current_index(egui_view_t *self, uint8_t index)
{
    hcw_segmented_control_clear_pressed_state(self);
    egui_view_segmented_control_set_current_index(self, index);
}

void hcw_segmented_control_override_interaction_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_segmented_control_on_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_segmented_control_on_key_event;
#endif
}

void hcw_segmented_control_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_segmented_control_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_segmented_control_on_static_key_event;
#endif
}
