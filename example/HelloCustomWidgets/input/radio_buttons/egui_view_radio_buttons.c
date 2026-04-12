#include "egui_view_radio_buttons.h"

#define EGUI_VIEW_RADIO_BUTTONS_STANDARD_CONTENT_PAD_X 10
#define EGUI_VIEW_RADIO_BUTTONS_STANDARD_CONTENT_PAD_Y 10
#define EGUI_VIEW_RADIO_BUTTONS_STANDARD_ITEM_HEIGHT   26
#define EGUI_VIEW_RADIO_BUTTONS_STANDARD_ITEM_GAP      6
#define EGUI_VIEW_RADIO_BUTTONS_STANDARD_ITEM_RADIUS   9
#define EGUI_VIEW_RADIO_BUTTONS_STANDARD_INDICATOR     16
#define EGUI_VIEW_RADIO_BUTTONS_STANDARD_TEXT_GAP      8
#define EGUI_VIEW_RADIO_BUTTONS_STANDARD_PANEL_RADIUS  12

#define EGUI_VIEW_RADIO_BUTTONS_COMPACT_CONTENT_PAD_X 8
#define EGUI_VIEW_RADIO_BUTTONS_COMPACT_CONTENT_PAD_Y 8
#define EGUI_VIEW_RADIO_BUTTONS_COMPACT_ITEM_HEIGHT   22
#define EGUI_VIEW_RADIO_BUTTONS_COMPACT_ITEM_GAP      4
#define EGUI_VIEW_RADIO_BUTTONS_COMPACT_ITEM_RADIUS   8
#define EGUI_VIEW_RADIO_BUTTONS_COMPACT_INDICATOR     14
#define EGUI_VIEW_RADIO_BUTTONS_COMPACT_TEXT_GAP      6
#define EGUI_VIEW_RADIO_BUTTONS_COMPACT_PANEL_RADIUS  10

typedef struct egui_view_radio_buttons_layout_item egui_view_radio_buttons_layout_item_t;
struct egui_view_radio_buttons_layout_item
{
    egui_region_t region;
};

static uint8_t egui_view_radio_buttons_clamp_count(uint8_t count)
{
    if (count > EGUI_VIEW_RADIO_BUTTONS_MAX_ITEMS)
    {
        return EGUI_VIEW_RADIO_BUTTONS_MAX_ITEMS;
    }
    return count;
}

static egui_color_t egui_view_radio_buttons_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 64);
}

static uint8_t egui_view_radio_buttons_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_radio_buttons_t);
    uint8_t had_pressed = (uint8_t)(self->is_pressed || local->pressed_index != EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE);

    if (!had_pressed)
    {
        return 0;
    }

    local->pressed_index = EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE;
    if (self->is_pressed)
    {
        egui_view_set_pressed(self, false);
    }
    else
    {
        egui_view_invalidate(self);
    }
    return 1;
}

static egui_dim_t egui_view_radio_buttons_content_pad_x(uint8_t compact_mode)
{
    return compact_mode ? EGUI_VIEW_RADIO_BUTTONS_COMPACT_CONTENT_PAD_X : EGUI_VIEW_RADIO_BUTTONS_STANDARD_CONTENT_PAD_X;
}

static egui_dim_t egui_view_radio_buttons_content_pad_y(uint8_t compact_mode)
{
    return compact_mode ? EGUI_VIEW_RADIO_BUTTONS_COMPACT_CONTENT_PAD_Y : EGUI_VIEW_RADIO_BUTTONS_STANDARD_CONTENT_PAD_Y;
}

static egui_dim_t egui_view_radio_buttons_item_height(uint8_t compact_mode)
{
    return compact_mode ? EGUI_VIEW_RADIO_BUTTONS_COMPACT_ITEM_HEIGHT : EGUI_VIEW_RADIO_BUTTONS_STANDARD_ITEM_HEIGHT;
}

static egui_dim_t egui_view_radio_buttons_min_item_height(uint8_t compact_mode)
{
    return compact_mode ? 14 : 16;
}

static egui_dim_t egui_view_radio_buttons_item_gap(uint8_t compact_mode)
{
    return compact_mode ? EGUI_VIEW_RADIO_BUTTONS_COMPACT_ITEM_GAP : EGUI_VIEW_RADIO_BUTTONS_STANDARD_ITEM_GAP;
}

static egui_dim_t egui_view_radio_buttons_indicator_size(uint8_t compact_mode)
{
    return compact_mode ? EGUI_VIEW_RADIO_BUTTONS_COMPACT_INDICATOR : EGUI_VIEW_RADIO_BUTTONS_STANDARD_INDICATOR;
}

static egui_dim_t egui_view_radio_buttons_item_radius(uint8_t compact_mode)
{
    return compact_mode ? EGUI_VIEW_RADIO_BUTTONS_COMPACT_ITEM_RADIUS : EGUI_VIEW_RADIO_BUTTONS_STANDARD_ITEM_RADIUS;
}

static egui_dim_t egui_view_radio_buttons_panel_radius(uint8_t compact_mode)
{
    return compact_mode ? EGUI_VIEW_RADIO_BUTTONS_COMPACT_PANEL_RADIUS : EGUI_VIEW_RADIO_BUTTONS_STANDARD_PANEL_RADIUS;
}

static egui_dim_t egui_view_radio_buttons_text_gap(uint8_t compact_mode)
{
    return compact_mode ? EGUI_VIEW_RADIO_BUTTONS_COMPACT_TEXT_GAP : EGUI_VIEW_RADIO_BUTTONS_STANDARD_TEXT_GAP;
}

static uint8_t egui_view_radio_buttons_prepare_layout(egui_view_radio_buttons_t *local, egui_view_t *self,
                                                      egui_view_radio_buttons_layout_item_t *items)
{
    egui_region_t work_region;
    egui_dim_t content_x;
    egui_dim_t content_y;
    egui_dim_t content_w;
    egui_dim_t content_h;
    egui_dim_t item_h;
    egui_dim_t gap;
    egui_dim_t total_h;
    egui_dim_t min_item_h;
    egui_dim_t cursor_y;
    uint8_t count;
    uint8_t i;

    count = egui_view_radio_buttons_clamp_count(local->item_count);
    if (count == 0 || local->items == NULL)
    {
        return 0;
    }

    egui_view_get_work_region(self, &work_region);
    content_x = work_region.location.x + egui_view_radio_buttons_content_pad_x(local->compact_mode);
    content_y = work_region.location.y + egui_view_radio_buttons_content_pad_y(local->compact_mode);
    content_w = work_region.size.width - egui_view_radio_buttons_content_pad_x(local->compact_mode) * 2;
    content_h = work_region.size.height - egui_view_radio_buttons_content_pad_y(local->compact_mode) * 2;
    if (content_w <= 0 || content_h <= 0)
    {
        return 0;
    }

    item_h = egui_view_radio_buttons_item_height(local->compact_mode);
    gap = egui_view_radio_buttons_item_gap(local->compact_mode);
    min_item_h = egui_view_radio_buttons_min_item_height(local->compact_mode);
    total_h = item_h * count + gap * (count - 1);

    if (total_h > content_h)
    {
        egui_dim_t available = content_h - gap * (count - 1);
        if (available < 0)
        {
            available = 0;
        }

        item_h = count > 0 ? available / count : 0;
        if (item_h < min_item_h)
        {
            item_h = min_item_h;
        }
        total_h = item_h * count + gap * (count - 1);

        while (total_h > content_h && gap > 0)
        {
            gap--;
            total_h = item_h * count + gap * (count - 1);
        }

        if (total_h > content_h)
        {
            item_h = content_h / count;
            if (item_h < min_item_h)
            {
                item_h = min_item_h;
            }
            total_h = item_h * count + gap * (count - 1);
        }
    }

    cursor_y = content_y + (content_h - total_h) / 2;
    for (i = 0; i < count; i++)
    {
        items[i].region.location.x = content_x;
        items[i].region.location.y = cursor_y;
        items[i].region.size.width = content_w;
        items[i].region.size.height = item_h;
        cursor_y += item_h + gap;
    }

    return count;
}

static uint8_t egui_view_radio_buttons_resolve_hit(egui_view_radio_buttons_t *local, egui_view_t *self, egui_dim_t screen_x, egui_dim_t screen_y)
{
    egui_region_t work_region;
    egui_view_radio_buttons_layout_item_t items[EGUI_VIEW_RADIO_BUTTONS_MAX_ITEMS];
    uint8_t count;
    uint8_t i;

    egui_view_get_work_region(self, &work_region);
    if (!egui_region_pt_in_rect(&work_region, screen_x, screen_y))
    {
        return EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE;
    }

    count = egui_view_radio_buttons_prepare_layout(local, self, items);
    for (i = 0; i < count; i++)
    {
        if (egui_region_pt_in_rect(&items[i].region, screen_x, screen_y))
        {
            return i;
        }
    }

    return EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE;
}

static void egui_view_radio_buttons_draw_indicator(egui_view_t *self, egui_view_radio_buttons_t *local, const egui_region_t *item_region, uint8_t is_selected,
                                                   egui_color_t ring_color, egui_color_t fill_color)
{
    egui_dim_t indicator_size = egui_view_radio_buttons_indicator_size(local->compact_mode);
    egui_dim_t stroke = local->compact_mode ? 1 : 2;
    egui_dim_t max_indicator = item_region->size.height - 4;
    egui_dim_t center_x;
    egui_dim_t center_y;
    egui_dim_t outer_radius;

    if (indicator_size > max_indicator)
    {
        indicator_size = max_indicator;
    }
    if (indicator_size < 8)
    {
        indicator_size = 8;
    }

    center_x = item_region->location.x + 4 + indicator_size / 2;
    center_y = item_region->location.y + item_region->size.height / 2;
    outer_radius = indicator_size / 2 - 1;
    if (outer_radius <= 0)
    {
        return;
    }

    egui_canvas_draw_circle_basic(center_x, center_y, outer_radius, stroke, ring_color, self->alpha);
    if (is_selected)
    {
        egui_dim_t inner_radius = local->compact_mode ? (outer_radius * 4) / 10 : (outer_radius * 5) / 10;
        if (inner_radius < 2)
        {
            inner_radius = 2;
        }
        egui_canvas_draw_circle_fill_basic(center_x, center_y, inner_radius, fill_color, self->alpha);
    }
}

static void egui_view_radio_buttons_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_radio_buttons_t);
    egui_region_t work_region;
    egui_view_radio_buttons_layout_item_t items[EGUI_VIEW_RADIO_BUTTONS_MAX_ITEMS];
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    uint8_t count;
    uint8_t is_enabled;
    uint8_t i;

    count = egui_view_radio_buttons_prepare_layout(local, self, items);
    if (count == 0)
    {
        return;
    }

    egui_view_get_work_region(self, &work_region);
    surface_color = local->surface_color;
    border_color = local->border_color;
    text_color = local->text_color;
    muted_text_color = local->muted_text_color;
    accent_color = local->accent_color;
    is_enabled = egui_view_get_enable(self) ? 1 : 0;

    if (local->compact_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFBFCFD), 18);
    }

    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xEFF3F7), 26);
        border_color = egui_rgb_mix(border_color, EGUI_COLOR_HEX(0xA7B4C1), 20);
        text_color = egui_rgb_mix(text_color, muted_text_color, 18);
        muted_text_color = egui_rgb_mix(muted_text_color, border_color, 24);
        accent_color = egui_rgb_mix(accent_color, muted_text_color, 40);
    }

    if (!is_enabled)
    {
        surface_color = egui_view_radio_buttons_mix_disabled(surface_color);
        border_color = egui_view_radio_buttons_mix_disabled(border_color);
        text_color = egui_view_radio_buttons_mix_disabled(text_color);
        muted_text_color = egui_view_radio_buttons_mix_disabled(muted_text_color);
        accent_color = egui_view_radio_buttons_mix_disabled(accent_color);
    }

    egui_canvas_draw_round_rectangle_fill(work_region.location.x, work_region.location.y, work_region.size.width, work_region.size.height,
                                          egui_view_radio_buttons_panel_radius(local->compact_mode), surface_color,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 82 : 88));
    egui_canvas_draw_round_rectangle(work_region.location.x, work_region.location.y, work_region.size.width, work_region.size.height,
                                     egui_view_radio_buttons_panel_radius(local->compact_mode), 1, border_color,
                                     egui_color_alpha_mix(self->alpha, local->compact_mode ? 34 : 40));

    for (i = 0; i < count; i++)
    {
        egui_region_t item_region = items[i].region;
        egui_region_t text_region;
        egui_color_t ring_color = border_color;
        egui_color_t fill_color = accent_color;
        egui_color_t item_text_color = text_color;
        uint8_t is_selected = i == local->current_index;
        uint8_t is_pressed = (uint8_t)(is_enabled && !local->read_only_mode && local->pressed_index == i);

        if (is_selected)
        {
            egui_color_t active_fill = egui_rgb_mix(surface_color, accent_color, local->read_only_mode ? 3 : 7);
            egui_alpha_t active_alpha = local->read_only_mode ? 16 : 22;

            egui_canvas_draw_round_rectangle_fill(item_region.location.x, item_region.location.y, item_region.size.width, item_region.size.height,
                                                  egui_view_radio_buttons_item_radius(local->compact_mode), active_fill,
                                                  egui_color_alpha_mix(self->alpha, active_alpha));
            ring_color = accent_color;
            item_text_color = egui_rgb_mix(text_color, accent_color, local->read_only_mode ? 2 : 6);
        }
        else if (local->read_only_mode)
        {
            ring_color = egui_rgb_mix(border_color, muted_text_color, 18);
            item_text_color = egui_rgb_mix(text_color, muted_text_color, 24);
            fill_color = muted_text_color;
        }

        if (is_pressed)
        {
            egui_canvas_draw_round_rectangle_fill(item_region.location.x, item_region.location.y, item_region.size.width, item_region.size.height,
                                                  egui_view_radio_buttons_item_radius(local->compact_mode), EGUI_THEME_PRESS_OVERLAY,
                                                  EGUI_THEME_PRESS_OVERLAY_ALPHA);
        }

        egui_view_radio_buttons_draw_indicator(self, local, &item_region, is_selected, ring_color, fill_color);

        text_region.location.x = item_region.location.x + 8 + egui_view_radio_buttons_indicator_size(local->compact_mode) +
                                 egui_view_radio_buttons_text_gap(local->compact_mode);
        text_region.location.y = item_region.location.y;
        text_region.size.width = item_region.location.x + item_region.size.width - text_region.location.x - 6;
        text_region.size.height = item_region.size.height;
        if (text_region.size.width > 0 && local->font != NULL && local->items[i] != NULL)
        {
            egui_canvas_draw_text_in_rect(local->font, local->items[i], &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, item_text_color, self->alpha);
        }

        if (i + 1 < count)
        {
            egui_dim_t divider_y = item_region.location.y + item_region.size.height + egui_view_radio_buttons_item_gap(local->compact_mode) / 2;

            egui_canvas_draw_line(item_region.location.x + 2, divider_y, item_region.location.x + item_region.size.width - 2, divider_y, 1, border_color,
                                  egui_color_alpha_mix(self->alpha, local->compact_mode ? 14 : 18));
        }

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focused && is_enabled && !local->read_only_mode && i == local->current_index)
        {
            egui_canvas_draw_round_rectangle(item_region.location.x, item_region.location.y, item_region.size.width, item_region.size.height,
                                             egui_view_radio_buttons_item_radius(local->compact_mode), 1, EGUI_THEME_FOCUS,
                                             egui_color_alpha_mix(self->alpha, local->compact_mode ? 52 : 64));
        }
#endif
    }
}

void egui_view_radio_buttons_set_items(egui_view_t *self, const char *const *items, uint8_t item_count)
{
    EGUI_LOCAL_INIT(egui_view_radio_buttons_t);
    uint8_t had_pressed = egui_view_radio_buttons_clear_pressed_state(self);

    local->items = items;
    local->item_count = items != NULL ? egui_view_radio_buttons_clamp_count(item_count) : 0;
    if (local->item_count == 0)
    {
        local->current_index = EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE;
    }
    else if (local->current_index != EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE && local->current_index >= local->item_count)
    {
        local->current_index = 0;
    }

    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_radio_buttons_set_current_index(egui_view_t *self, uint8_t index)
{
    EGUI_LOCAL_INIT(egui_view_radio_buttons_t);
    uint8_t had_pressed;

    if (local->item_count == 0)
    {
        egui_view_radio_buttons_clear_pressed_state(self);
        local->current_index = EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE;
        return;
    }
    if (index != EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE && index >= local->item_count)
    {
        egui_view_radio_buttons_clear_pressed_state(self);
        return;
    }
    if (local->current_index == index)
    {
        egui_view_radio_buttons_clear_pressed_state(self);
        return;
    }

    had_pressed = egui_view_radio_buttons_clear_pressed_state(self);
    local->current_index = index;
    if (local->on_selection_changed != NULL && index != EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE)
    {
        local->on_selection_changed(self, index);
    }
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t egui_view_radio_buttons_get_current_index(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_radio_buttons_t);
    return local->current_index;
}

void egui_view_radio_buttons_set_on_selection_changed_listener(egui_view_t *self, egui_view_on_radio_buttons_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_radio_buttons_t);
    local->on_selection_changed = listener;
}

void egui_view_radio_buttons_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_radio_buttons_t);
    uint8_t had_pressed = egui_view_radio_buttons_clear_pressed_state(self);

    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_radio_buttons_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_radio_buttons_t);
    uint8_t had_pressed = egui_view_radio_buttons_clear_pressed_state(self);

    local->compact_mode = compact_mode ? 1 : 0;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_radio_buttons_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_radio_buttons_t);
    uint8_t had_pressed = egui_view_radio_buttons_clear_pressed_state(self);

    local->read_only_mode = read_only_mode ? 1 : 0;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_radio_buttons_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                         egui_color_t muted_text_color, egui_color_t accent_color)
{
    EGUI_LOCAL_INIT(egui_view_radio_buttons_t);
    uint8_t had_pressed = egui_view_radio_buttons_clear_pressed_state(self);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_radio_buttons_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_radio_buttons_t);
    uint8_t hit_index;
    uint8_t handled;
    uint8_t same_target;

    if (!egui_view_get_enable(self) || local->read_only_mode || local->item_count == 0 || local->items == NULL)
    {
        egui_view_radio_buttons_clear_pressed_state(self);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_index = egui_view_radio_buttons_resolve_hit(local, self, event->location.x, event->location.y);
        if (hit_index == EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE)
        {
            egui_view_radio_buttons_clear_pressed_state(self);
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        same_target = (uint8_t)(local->pressed_index == hit_index);
        if (same_target && self->is_pressed)
        {
            return 1;
        }
        local->pressed_index = hit_index;
        if (!self->is_pressed)
        {
            egui_view_set_pressed(self, true);
        }
        else
        {
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_index == EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE)
        {
            return 0;
        }
        hit_index = egui_view_radio_buttons_resolve_hit(local, self, event->location.x, event->location.y);
        if (hit_index == local->pressed_index)
        {
            if (!self->is_pressed)
            {
                egui_view_set_pressed(self, true);
            }
            return 1;
        }
        if (self->is_pressed)
        {
            egui_view_set_pressed(self, false);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
        hit_index = egui_view_radio_buttons_resolve_hit(local, self, event->location.x, event->location.y);
        handled = (local->pressed_index != EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE) || hit_index != EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE;
        same_target = (uint8_t)(local->pressed_index != EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE && local->pressed_index == hit_index);
        if (same_target && self->is_pressed)
        {
            egui_view_radio_buttons_set_current_index(self, hit_index);
        }
        egui_view_radio_buttons_clear_pressed_state(self);
        return handled;
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return egui_view_radio_buttons_clear_pressed_state(self);
    default:
        return 0;
    }
}

static int egui_view_radio_buttons_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_radio_buttons_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_radio_buttons_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_radio_buttons_t);
    uint8_t next_index;
    uint8_t target_index;

    if (!egui_view_get_enable(self) || local->read_only_mode || local->item_count == 0 || local->items == NULL)
    {
        egui_view_radio_buttons_clear_pressed_state(self);
        return 0;
    }

    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            target_index = local->current_index;
            if (target_index == EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE)
            {
                target_index = 0;
            }
            local->pressed_index = target_index;
            egui_view_set_pressed(self, true);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            target_index = local->pressed_index;
            if (target_index == EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE)
            {
                target_index = local->current_index;
            }
            egui_view_radio_buttons_clear_pressed_state(self);
            if (target_index != EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE)
            {
                egui_view_radio_buttons_set_current_index(self, target_index);
            }
            return 1;
        }
        return 0;
    }

    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    next_index = local->current_index;
    if (next_index == EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE)
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
        egui_view_radio_buttons_set_current_index(self, next_index);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
    case EGUI_KEY_CODE_DOWN:
        if (next_index + 1 < local->item_count)
        {
            next_index++;
        }
        egui_view_radio_buttons_set_current_index(self, next_index);
        return 1;
    case EGUI_KEY_CODE_HOME:
        egui_view_radio_buttons_set_current_index(self, 0);
        return 1;
    case EGUI_KEY_CODE_END:
        egui_view_radio_buttons_set_current_index(self, (uint8_t)(local->item_count - 1));
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_index++;
        if (next_index >= local->item_count)
        {
            next_index = 0;
        }
        egui_view_radio_buttons_set_current_index(self, next_index);
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_radio_buttons_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_radio_buttons_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void egui_view_radio_buttons_on_focus_change(egui_view_t *self, int is_focused)
{
    EGUI_LOCAL_INIT(egui_view_radio_buttons_t);

    if (!is_focused)
    {
        egui_view_radio_buttons_clear_pressed_state(self);
        egui_view_invalidate(self);
        return;
    }

    if (local->current_index == EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE && local->item_count > 0)
    {
        egui_view_radio_buttons_set_current_index(self, 0);
        return;
    }

    egui_view_invalidate(self);
}
#endif

void egui_view_radio_buttons_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_radio_buttons_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_radio_buttons_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_radio_buttons_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_radio_buttons_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_radio_buttons_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_radio_buttons_on_key_event,
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        .on_focus_changed = egui_view_radio_buttons_on_focus_change,
#endif
};

void egui_view_radio_buttons_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_radio_buttons_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_radio_buttons_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif
    self->is_clickable = true;

    local->on_selection_changed = NULL;
    local->items = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD4DCE4);
    local->text_color = EGUI_COLOR_HEX(0x1A2734);
    local->muted_text_color = EGUI_COLOR_HEX(0x6B7A89);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->item_count = 0;
    local->current_index = EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_index = EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE;

    egui_view_set_view_name(self, "egui_view_radio_buttons");
}
