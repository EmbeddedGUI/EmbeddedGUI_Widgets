#include "egui_view_flyout.h"

#define EGUI_VIEW_FLYOUT_STANDARD_RADIUS        10
#define EGUI_VIEW_FLYOUT_STANDARD_FILL_ALPHA    90
#define EGUI_VIEW_FLYOUT_STANDARD_BORDER_ALPHA  54
#define EGUI_VIEW_FLYOUT_STANDARD_PAD_X         10
#define EGUI_VIEW_FLYOUT_STANDARD_PAD_Y         8
#define EGUI_VIEW_FLYOUT_STANDARD_TARGET_HEIGHT 24
#define EGUI_VIEW_FLYOUT_STANDARD_TARGET_GAP    7
#define EGUI_VIEW_FLYOUT_STANDARD_BUBBLE_HEIGHT 86
#define EGUI_VIEW_FLYOUT_STANDARD_BUBBLE_RADIUS 8
#define EGUI_VIEW_FLYOUT_STANDARD_BUBBLE_PAD_X  10
#define EGUI_VIEW_FLYOUT_STANDARD_BUBBLE_PAD_Y  8
#define EGUI_VIEW_FLYOUT_STANDARD_ACTION_HEIGHT 18
#define EGUI_VIEW_FLYOUT_STANDARD_ACTION_GAP    6
#define EGUI_VIEW_FLYOUT_STANDARD_ARROW_WIDTH   14
#define EGUI_VIEW_FLYOUT_STANDARD_ARROW_HEIGHT  8

#define EGUI_VIEW_FLYOUT_COMPACT_RADIUS        8
#define EGUI_VIEW_FLYOUT_COMPACT_FILL_ALPHA    88
#define EGUI_VIEW_FLYOUT_COMPACT_BORDER_ALPHA  48
#define EGUI_VIEW_FLYOUT_COMPACT_PAD_X         8
#define EGUI_VIEW_FLYOUT_COMPACT_PAD_Y         7
#define EGUI_VIEW_FLYOUT_COMPACT_TARGET_HEIGHT 18
#define EGUI_VIEW_FLYOUT_COMPACT_TARGET_GAP    5
#define EGUI_VIEW_FLYOUT_COMPACT_BUBBLE_HEIGHT 48
#define EGUI_VIEW_FLYOUT_COMPACT_BUBBLE_RADIUS 6
#define EGUI_VIEW_FLYOUT_COMPACT_BUBBLE_PAD_X  6
#define EGUI_VIEW_FLYOUT_COMPACT_BUBBLE_PAD_Y  5
#define EGUI_VIEW_FLYOUT_COMPACT_ACTION_HEIGHT 14
#define EGUI_VIEW_FLYOUT_COMPACT_ACTION_GAP    4
#define EGUI_VIEW_FLYOUT_COMPACT_ARROW_WIDTH   10
#define EGUI_VIEW_FLYOUT_COMPACT_ARROW_HEIGHT  5

typedef struct egui_view_flyout_metrics egui_view_flyout_metrics_t;
struct egui_view_flyout_metrics
{
    egui_region_t region;
    egui_region_t content_region;
    egui_region_t target_region;
    egui_region_t bubble_region;
    egui_region_t hint_region;
    egui_region_t title_region;
    egui_region_t body_region;
    egui_region_t footer_region;
    egui_region_t primary_region;
    egui_region_t secondary_region;
    egui_dim_t arrow_center_x;
    uint8_t show_bubble;
    uint8_t show_hint;
    uint8_t show_body;
    uint8_t show_footer;
    uint8_t show_primary;
    uint8_t show_secondary;
};

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_flyout_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_flyout_on_static_key_event(egui_view_t *self, egui_key_event_t *event);
#endif

static uint8_t egui_view_flyout_clamp_snapshot_count(uint8_t count)
{
    if (count > EGUI_VIEW_FLYOUT_MAX_SNAPSHOTS)
    {
        return EGUI_VIEW_FLYOUT_MAX_SNAPSHOTS;
    }

    return count;
}

static const egui_view_flyout_snapshot_t *egui_view_flyout_get_snapshot(egui_view_flyout_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }

    return &local->snapshots[local->current_snapshot];
}

static uint8_t egui_view_flyout_text_len(const char *text)
{
    uint8_t length = 0;

    if (text == NULL)
    {
        return 0;
    }

    while (text[length] != '\0')
    {
        length++;
    }

    return length;
}

static uint8_t egui_view_flyout_has_text(const char *text)
{
    return (text != NULL && text[0] != '\0') ? 1 : 0;
}

static egui_color_t egui_view_flyout_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static egui_color_t egui_view_flyout_tone_color(egui_view_flyout_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_FLYOUT_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_FLYOUT_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_FLYOUT_TONE_NEUTRAL:
        return local->neutral_color;
    case EGUI_VIEW_FLYOUT_TONE_ACCENT:
    default:
        return local->accent_color;
    }
}

static uint8_t egui_view_flyout_clear_pressed_state(egui_view_t *self, egui_view_flyout_t *local)
{
    uint8_t was_pressed = self->is_pressed || local->pressed_part != EGUI_VIEW_FLYOUT_PART_NONE;

    if (!was_pressed)
    {
        return 0;
    }

    local->pressed_part = EGUI_VIEW_FLYOUT_PART_NONE;
    egui_view_set_pressed(self, 0);
    return 1;
}

static uint8_t egui_view_flyout_part_exists(const egui_view_flyout_snapshot_t *snapshot, uint8_t open_state, uint8_t part)
{
    if (snapshot == NULL)
    {
        return 0;
    }

    if (part == EGUI_VIEW_FLYOUT_PART_TARGET)
    {
        return 1;
    }

    if (!open_state)
    {
        return 0;
    }

    if (part == EGUI_VIEW_FLYOUT_PART_PRIMARY)
    {
        return egui_view_flyout_has_text(snapshot->primary_action) ? 1 : 0;
    }

    if (part == EGUI_VIEW_FLYOUT_PART_SECONDARY)
    {
        return egui_view_flyout_has_text(snapshot->secondary_action) ? 1 : 0;
    }

    return 0;
}

static uint8_t egui_view_flyout_part_data_enabled(const egui_view_flyout_snapshot_t *snapshot, uint8_t open_state, uint8_t part)
{
    if (!egui_view_flyout_part_exists(snapshot, open_state, part))
    {
        return 0;
    }

    switch (part)
    {
    case EGUI_VIEW_FLYOUT_PART_TARGET:
        return 1;
    case EGUI_VIEW_FLYOUT_PART_PRIMARY:
        return snapshot->primary_enabled ? 1 : 0;
    case EGUI_VIEW_FLYOUT_PART_SECONDARY:
        return snapshot->secondary_enabled ? 1 : 0;
    default:
        return 0;
    }
}

static uint8_t egui_view_flyout_part_is_interactive(egui_view_flyout_t *local, egui_view_t *self, const egui_view_flyout_snapshot_t *snapshot, uint8_t part)
{
    if (snapshot == NULL || local->compact_mode || local->disabled_mode || !egui_view_get_enable(self))
    {
        return 0;
    }

    return egui_view_flyout_part_data_enabled(snapshot, local->open_state, part);
}

static uint8_t egui_view_flyout_find_first_action(const egui_view_flyout_snapshot_t *snapshot, uint8_t open_state, uint8_t enabled_only)
{
    if (snapshot == NULL || !open_state)
    {
        return EGUI_VIEW_FLYOUT_PART_NONE;
    }

    if ((!enabled_only || egui_view_flyout_part_data_enabled(snapshot, open_state, EGUI_VIEW_FLYOUT_PART_PRIMARY)) &&
        egui_view_flyout_part_exists(snapshot, open_state, EGUI_VIEW_FLYOUT_PART_PRIMARY))
    {
        return EGUI_VIEW_FLYOUT_PART_PRIMARY;
    }

    if ((!enabled_only || egui_view_flyout_part_data_enabled(snapshot, open_state, EGUI_VIEW_FLYOUT_PART_SECONDARY)) &&
        egui_view_flyout_part_exists(snapshot, open_state, EGUI_VIEW_FLYOUT_PART_SECONDARY))
    {
        return EGUI_VIEW_FLYOUT_PART_SECONDARY;
    }

    return EGUI_VIEW_FLYOUT_PART_NONE;
}

static uint8_t egui_view_flyout_find_last_action(const egui_view_flyout_snapshot_t *snapshot, uint8_t open_state, uint8_t enabled_only)
{
    if (snapshot == NULL || !open_state)
    {
        return EGUI_VIEW_FLYOUT_PART_NONE;
    }

    if ((!enabled_only || egui_view_flyout_part_data_enabled(snapshot, open_state, EGUI_VIEW_FLYOUT_PART_SECONDARY)) &&
        egui_view_flyout_part_exists(snapshot, open_state, EGUI_VIEW_FLYOUT_PART_SECONDARY))
    {
        return EGUI_VIEW_FLYOUT_PART_SECONDARY;
    }

    if ((!enabled_only || egui_view_flyout_part_data_enabled(snapshot, open_state, EGUI_VIEW_FLYOUT_PART_PRIMARY)) &&
        egui_view_flyout_part_exists(snapshot, open_state, EGUI_VIEW_FLYOUT_PART_PRIMARY))
    {
        return EGUI_VIEW_FLYOUT_PART_PRIMARY;
    }

    return EGUI_VIEW_FLYOUT_PART_NONE;
}

static uint8_t egui_view_flyout_resolve_default_part(egui_view_flyout_t *local, const egui_view_flyout_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return EGUI_VIEW_FLYOUT_PART_NONE;
    }

    if (!local->open_state)
    {
        return EGUI_VIEW_FLYOUT_PART_TARGET;
    }

    if (egui_view_flyout_part_exists(snapshot, local->open_state, snapshot->focus_part) &&
        egui_view_flyout_part_data_enabled(snapshot, local->open_state, snapshot->focus_part))
    {
        return snapshot->focus_part;
    }

    if (egui_view_flyout_find_first_action(snapshot, local->open_state, 1) != EGUI_VIEW_FLYOUT_PART_NONE)
    {
        return egui_view_flyout_find_first_action(snapshot, local->open_state, 1);
    }

    if (egui_view_flyout_find_first_action(snapshot, local->open_state, 0) != EGUI_VIEW_FLYOUT_PART_NONE)
    {
        return egui_view_flyout_find_first_action(snapshot, local->open_state, 0);
    }

    return EGUI_VIEW_FLYOUT_PART_TARGET;
}

static void egui_view_flyout_sync_current_part(egui_view_flyout_t *local)
{
    const egui_view_flyout_snapshot_t *snapshot = egui_view_flyout_get_snapshot(local);

    if (!egui_view_flyout_part_exists(snapshot, local->open_state, local->current_part))
    {
        local->current_part = egui_view_flyout_resolve_default_part(local, snapshot);
    }
}

static void egui_view_flyout_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                       egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (text == NULL || text[0] == '\0' || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static void egui_view_flyout_get_metrics(egui_view_flyout_t *local, egui_view_t *self, const egui_view_flyout_snapshot_t *snapshot,
                                         egui_view_flyout_metrics_t *metrics)
{
    egui_dim_t pad_x = local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_PAD_X : EGUI_VIEW_FLYOUT_STANDARD_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_PAD_Y : EGUI_VIEW_FLYOUT_STANDARD_PAD_Y;
    egui_dim_t target_h = local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_TARGET_HEIGHT : EGUI_VIEW_FLYOUT_STANDARD_TARGET_HEIGHT;
    egui_dim_t gap = local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_TARGET_GAP : EGUI_VIEW_FLYOUT_STANDARD_TARGET_GAP;
    egui_dim_t bubble_h = local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_BUBBLE_HEIGHT : EGUI_VIEW_FLYOUT_STANDARD_BUBBLE_HEIGHT;
    egui_dim_t inner_pad_x = local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_BUBBLE_PAD_X : EGUI_VIEW_FLYOUT_STANDARD_BUBBLE_PAD_X;
    egui_dim_t inner_pad_y = local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_BUBBLE_PAD_Y : EGUI_VIEW_FLYOUT_STANDARD_BUBBLE_PAD_Y;
    egui_dim_t action_h = local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_ACTION_HEIGHT : EGUI_VIEW_FLYOUT_STANDARD_ACTION_HEIGHT;
    egui_dim_t action_gap = local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_ACTION_GAP : EGUI_VIEW_FLYOUT_STANDARD_ACTION_GAP;
    egui_dim_t bubble_w;
    egui_dim_t target_w;
    egui_dim_t target_x;
    egui_dim_t target_y;
    egui_dim_t bubble_x;
    egui_dim_t bubble_y;
    egui_dim_t bubble_inner_w;
    egui_dim_t actions_y;
    egui_dim_t primary_w;
    egui_dim_t secondary_w;
    egui_dim_t bubble_shift = 0;
    egui_dim_t bubble_side_margin = local->compact_mode ? 0 : 6;
    egui_dim_t bubble_min_x;
    egui_dim_t bubble_max_x;
    egui_dim_t title_y;
    egui_dim_t footer_y;
    egui_dim_t body_y;
    egui_dim_t body_bottom;

    egui_view_get_work_region(self, &metrics->region);
    metrics->content_region = metrics->region;
    metrics->show_bubble = (snapshot != NULL && local->open_state) ? 1 : 0;
    metrics->show_hint = (snapshot != NULL && egui_view_flyout_has_text(snapshot->hint) && !local->compact_mode) ? 1 : 0;
    metrics->show_body = (snapshot != NULL && egui_view_flyout_has_text(snapshot->body) && !local->compact_mode) ? 1 : 0;
    metrics->show_footer = (snapshot != NULL && egui_view_flyout_has_text(snapshot->footer) && !local->compact_mode) ? 1 : 0;
    metrics->show_primary = (snapshot != NULL && egui_view_flyout_has_text(snapshot->primary_action)) ? 1 : 0;
    metrics->show_secondary = (snapshot != NULL && egui_view_flyout_has_text(snapshot->secondary_action)) ? 1 : 0;
    metrics->hint_region.size.width = 0;
    metrics->hint_region.size.height = 0;
    metrics->title_region.size.width = 0;
    metrics->title_region.size.height = 0;
    metrics->body_region.size.width = 0;
    metrics->body_region.size.height = 0;
    metrics->footer_region.size.width = 0;
    metrics->footer_region.size.height = 0;
    metrics->primary_region.size.width = 0;
    metrics->primary_region.size.height = 0;
    metrics->secondary_region.size.width = 0;
    metrics->secondary_region.size.height = 0;

    if (metrics->region.size.width <= 0 || metrics->region.size.height <= 0)
    {
        return;
    }

    bubble_w = metrics->region.size.width - pad_x * 2 - bubble_side_margin * 2;
    if (bubble_w < (local->compact_mode ? 64 : 116))
    {
        bubble_w = local->compact_mode ? 64 : 116;
    }

    target_w = (local->compact_mode ? 40 : 54) + egui_view_flyout_text_len(snapshot != NULL ? snapshot->target_label : NULL) *
                                                     (local->compact_mode ? 4 : 5);
    if (target_w < (local->compact_mode ? 48 : 62))
    {
        target_w = local->compact_mode ? 48 : 62;
    }
    if (target_w > bubble_w - (local->compact_mode ? 8 : 12))
    {
        target_w = bubble_w - (local->compact_mode ? 8 : 12);
    }

    target_x = metrics->region.location.x + (metrics->region.size.width - target_w) / 2;
    if (snapshot != NULL)
    {
        target_x += snapshot->target_offset_x;
    }
    if (target_x < metrics->region.location.x + pad_x)
    {
        target_x = metrics->region.location.x + pad_x;
    }
    if (target_x + target_w > metrics->region.location.x + metrics->region.size.width - pad_x)
    {
        target_x = metrics->region.location.x + metrics->region.size.width - pad_x - target_w;
    }

    if (metrics->show_bubble && snapshot != NULL && snapshot->placement == EGUI_VIEW_FLYOUT_PLACEMENT_TOP)
    {
        bubble_y = metrics->region.location.y + pad_y;
        target_y = bubble_y + bubble_h + gap;
    }
    else if (metrics->show_bubble)
    {
        target_y = metrics->region.location.y + pad_y + (local->compact_mode ? 1 : 3);
        bubble_y = target_y + target_h + gap;
    }
    else
    {
        target_y = metrics->region.location.y + (metrics->region.size.height - target_h) / 2;
        bubble_y = metrics->region.location.y + pad_y;
    }

    metrics->target_region.location.x = target_x;
    metrics->target_region.location.y = target_y;
    metrics->target_region.size.width = target_w;
    metrics->target_region.size.height = target_h;

    if (!local->compact_mode && snapshot != NULL)
    {
        bubble_shift = snapshot->target_offset_x / 3;
    }

    bubble_min_x = metrics->region.location.x + pad_x + bubble_side_margin;
    bubble_max_x = metrics->region.location.x + metrics->region.size.width - pad_x - bubble_side_margin - bubble_w;
    bubble_x = bubble_min_x + bubble_shift;
    if (bubble_x < bubble_min_x)
    {
        bubble_x = bubble_min_x;
    }
    if (bubble_x > bubble_max_x)
    {
        bubble_x = bubble_max_x;
    }

    metrics->bubble_region.location.x = bubble_x;
    metrics->bubble_region.location.y = bubble_y;
    metrics->bubble_region.size.width = bubble_w;
    metrics->bubble_region.size.height = metrics->show_bubble ? bubble_h : 0;

    metrics->arrow_center_x = metrics->target_region.location.x + metrics->target_region.size.width / 2;
    if (metrics->arrow_center_x < metrics->bubble_region.location.x + 14)
    {
        metrics->arrow_center_x = metrics->bubble_region.location.x + 14;
    }
    if (metrics->arrow_center_x > metrics->bubble_region.location.x + metrics->bubble_region.size.width - 14)
    {
        metrics->arrow_center_x = metrics->bubble_region.location.x + metrics->bubble_region.size.width - 14;
    }

    if (!metrics->show_bubble)
    {
        return;
    }

    bubble_inner_w = metrics->bubble_region.size.width - inner_pad_x * 2;
    metrics->hint_region.location.x = metrics->bubble_region.location.x + inner_pad_x;
    metrics->hint_region.location.y = metrics->bubble_region.location.y + inner_pad_y;
    metrics->hint_region.size.width = bubble_inner_w;
    metrics->hint_region.size.height = metrics->show_hint ? (local->compact_mode ? 0 : 8) : 0;

    if (local->compact_mode)
    {
        title_y = metrics->bubble_region.location.y + inner_pad_y + 2;
        metrics->title_region.location.x = metrics->bubble_region.location.x + inner_pad_x;
        metrics->title_region.location.y = title_y;
        metrics->title_region.size.width = bubble_inner_w;
        metrics->title_region.size.height = 10;
        actions_y = metrics->bubble_region.location.y + metrics->bubble_region.size.height - inner_pad_y - action_h;
    }
    else
    {
        title_y = metrics->bubble_region.location.y + inner_pad_y + (metrics->show_hint ? 10 : 2);
        metrics->title_region.location.x = metrics->bubble_region.location.x + inner_pad_x;
        metrics->title_region.location.y = title_y;
        metrics->title_region.size.width = bubble_inner_w;
        metrics->title_region.size.height = 11;
        actions_y = metrics->bubble_region.location.y + metrics->bubble_region.size.height - inner_pad_y - action_h;
    }

    if (metrics->show_secondary)
    {
        secondary_w = (local->compact_mode ? 16 : 22) +
                      egui_view_flyout_text_len(snapshot != NULL ? snapshot->secondary_action : NULL) * (local->compact_mode ? 4 : 5);
        if (secondary_w < (local->compact_mode ? 24 : 40))
        {
            secondary_w = local->compact_mode ? 24 : 40;
        }
        if (secondary_w > bubble_inner_w - action_gap - (local->compact_mode ? 28 : 52))
        {
            secondary_w = bubble_inner_w > action_gap ? (bubble_inner_w - action_gap) / 2 : 0;
        }

        metrics->secondary_region.location.x = metrics->bubble_region.location.x + inner_pad_x;
        metrics->secondary_region.location.y = actions_y;
        metrics->secondary_region.size.width = secondary_w;
        metrics->secondary_region.size.height = action_h;
    }

    if (metrics->show_primary)
    {
        if (metrics->show_secondary)
        {
            primary_w = bubble_inner_w - metrics->secondary_region.size.width - action_gap;
            metrics->primary_region.location.x = metrics->secondary_region.location.x + metrics->secondary_region.size.width + action_gap;
        }
        else
        {
            primary_w = bubble_inner_w;
            metrics->primary_region.location.x = metrics->bubble_region.location.x + inner_pad_x;
        }

        metrics->primary_region.location.y = actions_y;
        metrics->primary_region.size.width = primary_w;
        metrics->primary_region.size.height = action_h;
    }

    if (local->compact_mode)
    {
        return;
    }

    if (metrics->show_footer)
    {
        footer_y = actions_y - 12;
        metrics->footer_region.location.x = metrics->bubble_region.location.x + inner_pad_x;
        metrics->footer_region.location.y = footer_y;
        metrics->footer_region.size.width = bubble_inner_w;
        metrics->footer_region.size.height = 8;
    }

    body_y = metrics->title_region.location.y + metrics->title_region.size.height + 5;
    body_bottom = metrics->show_footer ? metrics->footer_region.location.y - 4 : actions_y - 6;
    if (body_bottom < body_y)
    {
        body_bottom = body_y;
    }
    metrics->body_region.location.x = metrics->bubble_region.location.x + inner_pad_x;
    metrics->body_region.location.y = body_y;
    metrics->body_region.size.width = bubble_inner_w;
    metrics->body_region.size.height = metrics->show_body ? (body_bottom - body_y) : 0;
}

static void egui_view_flyout_draw_focus_ring(egui_view_t *self, const egui_region_t *region, egui_dim_t radius, egui_color_t color, egui_alpha_t alpha)
{
    egui_canvas_draw_round_rectangle(region->location.x - 1, region->location.y - 1, region->size.width + 2, region->size.height + 2, radius + 1, 1, color,
                                     egui_color_alpha_mix(self->alpha, alpha));
}

static void egui_view_flyout_draw_arrow(egui_view_t *self, egui_view_flyout_t *local, const egui_view_flyout_snapshot_t *snapshot,
                                        const egui_view_flyout_metrics_t *metrics, egui_color_t fill_color, egui_color_t border_color)
{
    egui_dim_t arrow_w = local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_ARROW_WIDTH : EGUI_VIEW_FLYOUT_STANDARD_ARROW_WIDTH;
    egui_dim_t arrow_h = local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_ARROW_HEIGHT : EGUI_VIEW_FLYOUT_STANDARD_ARROW_HEIGHT;
    egui_dim_t center_x = metrics->arrow_center_x;

    if (snapshot == NULL || !metrics->show_bubble)
    {
        return;
    }

    if (snapshot->placement == EGUI_VIEW_FLYOUT_PLACEMENT_TOP)
    {
        egui_dim_t top_y = metrics->bubble_region.location.y + metrics->bubble_region.size.height;

        egui_canvas_draw_triangle_fill(center_x - arrow_w / 2, top_y - 1, center_x + arrow_w / 2, top_y - 1, center_x, top_y + arrow_h, fill_color,
                                       egui_color_alpha_mix(self->alpha, 94));
        egui_canvas_draw_triangle(center_x - arrow_w / 2, top_y - 1, center_x + arrow_w / 2, top_y - 1, center_x, top_y + arrow_h, border_color,
                                  egui_color_alpha_mix(self->alpha, 46));
    }
    else
    {
        egui_dim_t top_y = metrics->bubble_region.location.y;

        egui_canvas_draw_triangle_fill(center_x - arrow_w / 2, top_y + 1, center_x + arrow_w / 2, top_y + 1, center_x, top_y - arrow_h, fill_color,
                                       egui_color_alpha_mix(self->alpha, 94));
        egui_canvas_draw_triangle(center_x - arrow_w / 2, top_y + 1, center_x + arrow_w / 2, top_y + 1, center_x, top_y - arrow_h, border_color,
                                  egui_color_alpha_mix(self->alpha, 46));
    }
}

static void egui_view_flyout_draw_target(egui_view_t *self, egui_view_flyout_t *local, const egui_view_flyout_snapshot_t *snapshot,
                                         const egui_view_flyout_metrics_t *metrics, egui_color_t tone_color, egui_color_t text_color,
                                         egui_color_t border_color)
{
    egui_color_t fill = egui_rgb_mix(local->target_fill_color, tone_color, local->open_state ? 10 : 4);
    egui_color_t outline = egui_rgb_mix(local->target_border_color, tone_color, local->open_state ? 16 : 8);
    egui_color_t label = egui_rgb_mix(text_color, tone_color, local->open_state ? 10 : 4);
    egui_color_t focus = egui_rgb_mix(tone_color, EGUI_COLOR_WHITE, 10);
    egui_dim_t radius = local->compact_mode ? 6 : 8;
    egui_region_t text_region;
    egui_dim_t dot_x;
    egui_dim_t dot_y;

    if (local->current_part == EGUI_VIEW_FLYOUT_PART_TARGET)
    {
        fill = egui_rgb_mix(fill, tone_color, 12);
        outline = egui_rgb_mix(outline, tone_color, 18);
    }
    if (local->pressed_part == EGUI_VIEW_FLYOUT_PART_TARGET)
    {
        fill = egui_rgb_mix(fill, tone_color, 12);
    }

    if (local->disabled_mode)
    {
        fill = egui_rgb_mix(fill, local->surface_color, 28);
        outline = egui_rgb_mix(outline, local->muted_text_color, 26);
        label = egui_rgb_mix(label, local->muted_text_color, 28);
    }
    if (!egui_view_get_enable(self))
    {
        fill = egui_view_flyout_mix_disabled(fill);
        outline = egui_view_flyout_mix_disabled(outline);
        label = egui_view_flyout_mix_disabled(label);
        tone_color = egui_view_flyout_mix_disabled(tone_color);
    }

    if (local->current_part == EGUI_VIEW_FLYOUT_PART_TARGET && egui_view_flyout_part_is_interactive(local, self, snapshot, EGUI_VIEW_FLYOUT_PART_TARGET))
    {
        egui_view_flyout_draw_focus_ring(self, &metrics->target_region, radius, focus, 42);
    }

    egui_canvas_draw_round_rectangle_fill(metrics->target_region.location.x, metrics->target_region.location.y, metrics->target_region.size.width,
                                          metrics->target_region.size.height, radius, fill,
                                          egui_color_alpha_mix(self->alpha, local->disabled_mode ? 84 : 94));
    egui_canvas_draw_round_rectangle(metrics->target_region.location.x, metrics->target_region.location.y, metrics->target_region.size.width,
                                     metrics->target_region.size.height, radius, 1, outline, egui_color_alpha_mix(self->alpha, 46));

    dot_x = metrics->target_region.location.x + (local->compact_mode ? 8 : 10);
    dot_y = metrics->target_region.location.y + metrics->target_region.size.height / 2;
    egui_canvas_draw_circle_fill(dot_x, dot_y, local->compact_mode ? 2 : 3, tone_color, egui_color_alpha_mix(self->alpha, 78));

    if (local->open_state && metrics->target_region.size.width > 18)
    {
        egui_canvas_draw_round_rectangle_fill(metrics->target_region.location.x + 8, metrics->target_region.location.y + metrics->target_region.size.height - 4,
                                              metrics->target_region.size.width - 16, 2, 1, tone_color, egui_color_alpha_mix(self->alpha, 62));
    }

    text_region.location.x = dot_x + (local->compact_mode ? 6 : 8);
    text_region.location.y = metrics->target_region.location.y;
    text_region.size.width = metrics->target_region.size.width - (text_region.location.x - metrics->target_region.location.x) - 8;
    text_region.size.height = metrics->target_region.size.height;
    egui_view_flyout_draw_text(local->font, self, snapshot != NULL ? snapshot->target_label : "", &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, label);
}

static void egui_view_flyout_draw_action(egui_view_t *self, egui_view_flyout_t *local, const egui_region_t *region, const char *label,
                                         egui_color_t tone_color, egui_color_t border_color, egui_color_t text_color, uint8_t emphasized, uint8_t focused,
                                         uint8_t pressed, uint8_t enabled)
{
    egui_color_t fill = emphasized ? egui_rgb_mix(tone_color, local->surface_color, local->compact_mode ? 34 : 26)
                                   : egui_rgb_mix(local->surface_color, tone_color, focused ? 12 : 6);
    egui_color_t outline = emphasized ? egui_rgb_mix(border_color, tone_color, 20) : egui_rgb_mix(border_color, tone_color, focused ? 18 : 12);
    egui_color_t label_color = emphasized ? egui_rgb_mix(EGUI_COLOR_WHITE, text_color, 12) : (focused ? tone_color : text_color);
    egui_color_t focus_color = emphasized ? egui_rgb_mix(EGUI_COLOR_WHITE, tone_color, 18) : egui_rgb_mix(tone_color, EGUI_COLOR_WHITE, 8);

    if (!enabled || local->disabled_mode)
    {
        fill = egui_rgb_mix(fill, local->surface_color, 34);
        outline = egui_rgb_mix(outline, local->muted_text_color, 28);
        label_color = egui_rgb_mix(label_color, local->muted_text_color, 34);
    }
    if (pressed && enabled && !local->disabled_mode)
    {
        fill = egui_rgb_mix(fill, tone_color, 12);
    }
    if (!egui_view_get_enable(self))
    {
        fill = egui_view_flyout_mix_disabled(fill);
        outline = egui_view_flyout_mix_disabled(outline);
        label_color = egui_view_flyout_mix_disabled(label_color);
        focus_color = egui_view_flyout_mix_disabled(focus_color);
    }

    if (focused && enabled && !local->disabled_mode && egui_view_get_enable(self))
    {
        egui_view_flyout_draw_focus_ring(self, region, local->compact_mode ? 5 : 6, focus_color, emphasized ? 48 : 42);
    }

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, local->compact_mode ? 5 : 6, fill,
                                          egui_color_alpha_mix(self->alpha, emphasized ? 90 : 80));
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, local->compact_mode ? 5 : 6, 1, outline,
                                     egui_color_alpha_mix(self->alpha, 40));
    egui_view_flyout_draw_text(local->meta_font, self, label, region, EGUI_ALIGN_CENTER, label_color);
}

static void egui_view_flyout_draw_bubble(egui_view_t *self, egui_view_flyout_t *local, const egui_view_flyout_snapshot_t *snapshot,
                                         const egui_view_flyout_metrics_t *metrics, egui_color_t tone_color, egui_color_t text_color,
                                         egui_color_t muted_text_color, egui_color_t border_color, egui_color_t shadow_color)
{
    egui_color_t bubble_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 8 : 10);
    egui_color_t bubble_border = egui_rgb_mix(border_color, tone_color, local->compact_mode ? 12 : 16);
    egui_color_t body_color = egui_rgb_mix(text_color, muted_text_color, local->compact_mode ? 60 : 56);
    egui_color_t footer_color = egui_rgb_mix(muted_text_color, tone_color, 8);
    egui_color_t divider_color = egui_rgb_mix(bubble_border, tone_color, 8);
    egui_dim_t bubble_radius = local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_BUBBLE_RADIUS : EGUI_VIEW_FLYOUT_STANDARD_BUBBLE_RADIUS;

    if (!metrics->show_bubble || snapshot == NULL)
    {
        return;
    }

    if (local->disabled_mode)
    {
        bubble_fill = egui_rgb_mix(bubble_fill, local->surface_color, 24);
        bubble_border = egui_rgb_mix(bubble_border, local->muted_text_color, 26);
        body_color = egui_rgb_mix(body_color, local->muted_text_color, 24);
        footer_color = egui_rgb_mix(footer_color, local->muted_text_color, 24);
        shadow_color = egui_rgb_mix(shadow_color, local->surface_color, 32);
    }
    if (!egui_view_get_enable(self))
    {
        bubble_fill = egui_view_flyout_mix_disabled(bubble_fill);
        bubble_border = egui_view_flyout_mix_disabled(bubble_border);
        body_color = egui_view_flyout_mix_disabled(body_color);
        footer_color = egui_view_flyout_mix_disabled(footer_color);
        shadow_color = egui_view_flyout_mix_disabled(shadow_color);
    }

    egui_canvas_draw_round_rectangle_fill(metrics->bubble_region.location.x + 1, metrics->bubble_region.location.y + 2, metrics->bubble_region.size.width,
                                          metrics->bubble_region.size.height, bubble_radius, shadow_color,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 10 : 18));
    egui_view_flyout_draw_arrow(self, local, snapshot, metrics, bubble_fill, bubble_border);
    egui_canvas_draw_round_rectangle_fill(metrics->bubble_region.location.x, metrics->bubble_region.location.y, metrics->bubble_region.size.width,
                                          metrics->bubble_region.size.height, bubble_radius, bubble_fill, egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(metrics->bubble_region.location.x, metrics->bubble_region.location.y, metrics->bubble_region.size.width,
                                     metrics->bubble_region.size.height, bubble_radius, 1, bubble_border, egui_color_alpha_mix(self->alpha, 52));
    egui_canvas_draw_round_rectangle_fill(metrics->bubble_region.location.x + (local->compact_mode ? 6 : 10), metrics->bubble_region.location.y + 6,
                                          local->compact_mode ? 12 : 20, local->compact_mode ? 2 : 3, 2, tone_color,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 56 : 70));

    if (metrics->show_hint)
    {
        egui_view_flyout_draw_text(local->meta_font, self, snapshot->hint, &metrics->hint_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, tone_color);
    }
    egui_view_flyout_draw_text(local->font, self, snapshot->title, &metrics->title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color);
    if (metrics->show_body)
    {
        egui_view_flyout_draw_text(local->meta_font, self, snapshot->body, &metrics->body_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP, body_color);
    }
    if (metrics->show_footer)
    {
        egui_view_flyout_draw_text(local->meta_font, self, snapshot->footer, &metrics->footer_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, footer_color);
    }
    if (!local->compact_mode && (metrics->show_primary || metrics->show_secondary))
    {
        egui_dim_t divider_y = metrics->show_footer ? (metrics->footer_region.location.y - 4) : (metrics->primary_region.location.y - 4);
        egui_dim_t divider_x1 = metrics->bubble_region.location.x + 10;
        egui_dim_t divider_x2 = metrics->bubble_region.location.x + metrics->bubble_region.size.width - 11;

        egui_canvas_draw_line(divider_x1, divider_y, divider_x2, divider_y, 1, divider_color, egui_color_alpha_mix(self->alpha, 20));
    }
    if (metrics->show_primary)
    {
        egui_view_flyout_draw_action(self, local, &metrics->primary_region, snapshot->primary_action, tone_color, border_color, text_color, 1,
                                     local->current_part == EGUI_VIEW_FLYOUT_PART_PRIMARY ? 1 : 0,
                                     local->pressed_part == EGUI_VIEW_FLYOUT_PART_PRIMARY ? 1 : 0,
                                     snapshot->primary_enabled ? 1 : 0);
    }
    if (metrics->show_secondary)
    {
        egui_view_flyout_draw_action(self, local, &metrics->secondary_region, snapshot->secondary_action, tone_color, border_color, text_color, 0,
                                     local->current_part == EGUI_VIEW_FLYOUT_PART_SECONDARY ? 1 : 0,
                                     local->pressed_part == EGUI_VIEW_FLYOUT_PART_SECONDARY ? 1 : 0,
                                     snapshot->secondary_enabled ? 1 : 0);
    }
}

static void egui_view_flyout_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    const egui_view_flyout_snapshot_t *snapshot = egui_view_flyout_get_snapshot(local);
    egui_view_flyout_metrics_t metrics;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_color_t muted_text_color = local->muted_text_color;
    egui_color_t shadow_color = local->shadow_color;
    egui_color_t tone_color;
    egui_dim_t panel_radius = local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_RADIUS : EGUI_VIEW_FLYOUT_STANDARD_RADIUS;

    egui_view_get_work_region(self, &metrics.region);
    if (metrics.region.size.width <= 0 || metrics.region.size.height <= 0 || snapshot == NULL)
    {
        return;
    }

    tone_color = egui_view_flyout_tone_color(local, snapshot->tone);
    if (local->disabled_mode)
    {
        tone_color = egui_rgb_mix(tone_color, muted_text_color, 52);
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFBFCFD), 14);
        border_color = egui_rgb_mix(border_color, muted_text_color, 12);
        text_color = egui_rgb_mix(text_color, muted_text_color, 22);
        muted_text_color = egui_rgb_mix(muted_text_color, surface_color, 14);
        shadow_color = egui_rgb_mix(shadow_color, surface_color, 34);
    }
    if (!egui_view_get_enable(self))
    {
        tone_color = egui_view_flyout_mix_disabled(tone_color);
        surface_color = egui_view_flyout_mix_disabled(surface_color);
        border_color = egui_view_flyout_mix_disabled(border_color);
        text_color = egui_view_flyout_mix_disabled(text_color);
        muted_text_color = egui_view_flyout_mix_disabled(muted_text_color);
        shadow_color = egui_view_flyout_mix_disabled(shadow_color);
    }

    egui_view_flyout_get_metrics(local, self, snapshot, &metrics);
    egui_canvas_draw_round_rectangle_fill(metrics.region.location.x, metrics.region.location.y, metrics.region.size.width, metrics.region.size.height, panel_radius,
                                          surface_color,
                                          egui_color_alpha_mix(self->alpha,
                                                               local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_FILL_ALPHA
                                                                                  : EGUI_VIEW_FLYOUT_STANDARD_FILL_ALPHA));
    egui_canvas_draw_round_rectangle(metrics.region.location.x, metrics.region.location.y, metrics.region.size.width, metrics.region.size.height, panel_radius, 1,
                                     border_color,
                                     egui_color_alpha_mix(self->alpha,
                                                          local->compact_mode ? EGUI_VIEW_FLYOUT_COMPACT_BORDER_ALPHA
                                                                             : EGUI_VIEW_FLYOUT_STANDARD_BORDER_ALPHA));

    egui_view_flyout_draw_target(self, local, snapshot, &metrics, tone_color, text_color, border_color);
    egui_view_flyout_draw_bubble(self, local, snapshot, &metrics, tone_color, text_color, muted_text_color, border_color, shadow_color);
}

static uint8_t egui_view_flyout_resolve_hit(egui_view_flyout_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_flyout_metrics_t metrics;
    const egui_view_flyout_snapshot_t *snapshot = egui_view_flyout_get_snapshot(local);

    egui_view_flyout_get_metrics(local, self, snapshot, &metrics);
    if (metrics.target_region.size.width > 0 && egui_region_pt_in_rect(&metrics.target_region, x, y))
    {
        return EGUI_VIEW_FLYOUT_PART_TARGET;
    }
    if (!metrics.show_bubble)
    {
        return EGUI_VIEW_FLYOUT_PART_NONE;
    }
    if (metrics.show_primary && metrics.primary_region.size.width > 0 && egui_region_pt_in_rect(&metrics.primary_region, x, y))
    {
        return EGUI_VIEW_FLYOUT_PART_PRIMARY;
    }
    if (metrics.show_secondary && metrics.secondary_region.size.width > 0 && egui_region_pt_in_rect(&metrics.secondary_region, x, y))
    {
        return EGUI_VIEW_FLYOUT_PART_SECONDARY;
    }

    return EGUI_VIEW_FLYOUT_PART_NONE;
}

static void egui_view_flyout_set_current_part_inner(egui_view_t *self, uint8_t part, uint8_t clear_pressed)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    const egui_view_flyout_snapshot_t *snapshot = egui_view_flyout_get_snapshot(local);
    uint8_t had_pressed = 0;

    if (clear_pressed)
    {
        had_pressed = egui_view_flyout_clear_pressed_state(self, local);
    }

    if (!egui_view_flyout_part_exists(snapshot, local->open_state, part))
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    if (local->current_part == part)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_part = part;
    egui_view_invalidate(self);
}

static uint8_t egui_view_flyout_get_active_parts(egui_view_flyout_t *local, const egui_view_flyout_snapshot_t *snapshot, uint8_t *parts)
{
    uint8_t count = 0;

    if (snapshot == NULL)
    {
        return 0;
    }

    parts[count++] = EGUI_VIEW_FLYOUT_PART_TARGET;
    if (!local->open_state)
    {
        return count;
    }

    if (egui_view_flyout_part_data_enabled(snapshot, local->open_state, EGUI_VIEW_FLYOUT_PART_PRIMARY))
    {
        parts[count++] = EGUI_VIEW_FLYOUT_PART_PRIMARY;
    }
    if (egui_view_flyout_part_data_enabled(snapshot, local->open_state, EGUI_VIEW_FLYOUT_PART_SECONDARY))
    {
        parts[count++] = EGUI_VIEW_FLYOUT_PART_SECONDARY;
    }

    return count;
}

static void egui_view_flyout_move_linear(egui_view_t *self, int8_t step, uint8_t wrap)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    const egui_view_flyout_snapshot_t *snapshot = egui_view_flyout_get_snapshot(local);
    uint8_t parts[3];
    uint8_t count = egui_view_flyout_get_active_parts(local, snapshot, parts);
    int16_t current = 0;
    int16_t next;

    if (count == 0)
    {
        return;
    }

    while (current < count && parts[current] != local->current_part)
    {
        current++;
    }

    if (current >= count)
    {
        local->current_part = parts[0];
        egui_view_invalidate(self);
        return;
    }

    next = current + step;
    if (wrap)
    {
        if (next < 0)
        {
            next = count - 1;
        }
        else if (next >= count)
        {
            next = 0;
        }
    }
    else if (next < 0 || next >= count)
    {
        return;
    }

    if (parts[next] != local->current_part)
    {
        local->current_part = parts[next];
        egui_view_invalidate(self);
    }
}

static void egui_view_flyout_move_vertical(egui_view_t *self, int8_t direction)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    const egui_view_flyout_snapshot_t *snapshot = egui_view_flyout_get_snapshot(local);
    uint8_t toward_bubble;
    uint8_t toward_target;
    uint8_t target = local->current_part;
    uint8_t first_action;

    if (snapshot == NULL || !local->open_state)
    {
        return;
    }

    toward_bubble = snapshot->placement == EGUI_VIEW_FLYOUT_PLACEMENT_TOP ? 0 : 1;
    toward_target = toward_bubble ? 0 : 1;
    first_action = egui_view_flyout_find_first_action(snapshot, local->open_state, 1);
    if (first_action == EGUI_VIEW_FLYOUT_PART_NONE)
    {
        first_action = egui_view_flyout_find_first_action(snapshot, local->open_state, 0);
    }

    if ((direction > 0 ? 1 : 0) == toward_bubble)
    {
        if (local->current_part == EGUI_VIEW_FLYOUT_PART_TARGET)
        {
            target = first_action != EGUI_VIEW_FLYOUT_PART_NONE ? first_action : local->current_part;
        }
        else if (local->current_part == EGUI_VIEW_FLYOUT_PART_PRIMARY &&
                 egui_view_flyout_part_data_enabled(snapshot, local->open_state, EGUI_VIEW_FLYOUT_PART_SECONDARY))
        {
            target = EGUI_VIEW_FLYOUT_PART_SECONDARY;
        }
    }
    else if ((direction > 0 ? 1 : 0) == toward_target)
    {
        if (local->current_part == EGUI_VIEW_FLYOUT_PART_PRIMARY || local->current_part == EGUI_VIEW_FLYOUT_PART_SECONDARY)
        {
            target = EGUI_VIEW_FLYOUT_PART_TARGET;
        }
    }

    if (target != local->current_part && target != EGUI_VIEW_FLYOUT_PART_NONE)
    {
        local->current_part = target;
        egui_view_invalidate(self);
    }
}

void egui_view_flyout_set_snapshots(egui_view_t *self, const egui_view_flyout_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    const egui_view_flyout_snapshot_t *snapshot;

    egui_view_flyout_clear_pressed_state(self, local);
    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : egui_view_flyout_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }

    snapshot = egui_view_flyout_get_snapshot(local);
    local->open_state = snapshot != NULL && snapshot->open ? 1 : 0;
    local->current_part = egui_view_flyout_resolve_default_part(local, snapshot);
    egui_view_invalidate(self);
}

void egui_view_flyout_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    const egui_view_flyout_snapshot_t *snapshot;
    uint8_t had_pressed = egui_view_flyout_clear_pressed_state(self, local);

    if (local->snapshot_count == 0 || snapshot_index >= local->snapshot_count)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    if (local->current_snapshot == snapshot_index)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_snapshot = snapshot_index;
    snapshot = egui_view_flyout_get_snapshot(local);
    local->open_state = snapshot != NULL && snapshot->open ? 1 : 0;
    local->current_part = egui_view_flyout_resolve_default_part(local, snapshot);
    egui_view_invalidate(self);
}

uint8_t egui_view_flyout_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    return local->current_snapshot;
}

void egui_view_flyout_set_open(egui_view_t *self, uint8_t open)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    const egui_view_flyout_snapshot_t *snapshot = egui_view_flyout_get_snapshot(local);
    uint8_t had_pressed = egui_view_flyout_clear_pressed_state(self, local);

    open = open ? 1 : 0;
    if (snapshot == NULL)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    if (local->open_state == open)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->open_state = open;
    local->current_part = egui_view_flyout_resolve_default_part(local, snapshot);
    egui_view_invalidate(self);
}

uint8_t egui_view_flyout_get_open(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    return local->open_state;
}

void egui_view_flyout_set_current_part(egui_view_t *self, uint8_t part)
{
    egui_view_flyout_set_current_part_inner(self, part, 1);
}

uint8_t egui_view_flyout_get_current_part(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    egui_view_flyout_sync_current_part(local);
    return local->current_part;
}

uint8_t egui_view_flyout_activate_current_part(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    const egui_view_flyout_snapshot_t *snapshot = egui_view_flyout_get_snapshot(local);
    uint8_t action_part;

    egui_view_flyout_sync_current_part(local);
    if (!egui_view_flyout_part_is_interactive(local, self, snapshot, local->current_part))
    {
        return 0;
    }

    if (local->current_part == EGUI_VIEW_FLYOUT_PART_TARGET)
    {
        local->open_state = local->open_state ? 0 : 1;
        local->current_part = egui_view_flyout_resolve_default_part(local, snapshot);
        egui_view_invalidate(self);
        return 1;
    }

    action_part = local->current_part;
    local->open_state = 0;
    local->current_part = EGUI_VIEW_FLYOUT_PART_TARGET;
    if (local->on_action != NULL)
    {
        local->on_action(self, local->current_snapshot, action_part);
    }
    egui_view_invalidate(self);
    return 1;
}

void egui_view_flyout_set_on_action_listener(egui_view_t *self, egui_view_on_flyout_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    local->on_action = listener;
}

void egui_view_flyout_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    uint8_t had_pressed = egui_view_flyout_clear_pressed_state(self, local);

    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_flyout_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    uint8_t had_pressed = egui_view_flyout_clear_pressed_state(self, local);

    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_flyout_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    uint8_t had_pressed = egui_view_flyout_clear_pressed_state(self, local);

    compact_mode = compact_mode ? 1 : 0;
    if (local->compact_mode != compact_mode)
    {
        local->compact_mode = compact_mode;
        local->current_part = egui_view_flyout_resolve_default_part(local, egui_view_flyout_get_snapshot(local));
        egui_view_invalidate(self);
        return;
    }

    if (had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_flyout_set_disabled_mode(egui_view_t *self, uint8_t disabled_mode)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    uint8_t had_pressed = egui_view_flyout_clear_pressed_state(self, local);

    disabled_mode = disabled_mode ? 1 : 0;
    if (local->disabled_mode != disabled_mode)
    {
        local->disabled_mode = disabled_mode;
        egui_view_invalidate(self);
        return;
    }

    if (had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_flyout_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                  egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t success_color, egui_color_t warning_color,
                                  egui_color_t neutral_color, egui_color_t shadow_color, egui_color_t target_fill_color,
                                  egui_color_t target_border_color)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    uint8_t had_pressed = egui_view_flyout_clear_pressed_state(self, local);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    local->shadow_color = shadow_color;
    local->target_fill_color = target_fill_color;
    local->target_border_color = target_border_color;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t egui_view_flyout_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    egui_view_flyout_metrics_t metrics;
    const egui_view_flyout_snapshot_t *snapshot = egui_view_flyout_get_snapshot(local);

    if (region == NULL)
    {
        return 0;
    }

    egui_view_flyout_get_metrics(local, self, snapshot, &metrics);
    switch (part)
    {
    case EGUI_VIEW_FLYOUT_PART_TARGET:
        *region = metrics.target_region;
        return 1;
    case EGUI_VIEW_FLYOUT_PART_PRIMARY:
        if (!metrics.show_bubble || !metrics.show_primary)
        {
            return 0;
        }
        *region = metrics.primary_region;
        return 1;
    case EGUI_VIEW_FLYOUT_PART_SECONDARY:
        if (!metrics.show_bubble || !metrics.show_secondary)
        {
            return 0;
        }
        *region = metrics.secondary_region;
        return 1;
    default:
        return 0;
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_flyout_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    const egui_view_flyout_snapshot_t *snapshot = egui_view_flyout_get_snapshot(local);
    uint8_t hit_part;

    if (snapshot == NULL || local->compact_mode || local->disabled_mode || !egui_view_get_enable(self))
    {
        if (egui_view_flyout_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_part = egui_view_flyout_resolve_hit(local, self, event->location.x, event->location.y);
        if (!egui_view_flyout_part_is_interactive(local, self, snapshot, hit_part))
        {
            if (egui_view_flyout_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return 0;
        }
        local->pressed_part = hit_part;
        local->current_part = hit_part;
        egui_view_set_pressed(self, 1);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_part == EGUI_VIEW_FLYOUT_PART_NONE)
        {
            return 0;
        }
        hit_part = egui_view_flyout_resolve_hit(local, self, event->location.x, event->location.y);
        egui_view_set_pressed(self, hit_part == local->pressed_part &&
                                            egui_view_flyout_part_is_interactive(local, self, snapshot, local->pressed_part));
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled;

        hit_part = egui_view_flyout_resolve_hit(local, self, event->location.x, event->location.y);
        if (hit_part == local->pressed_part && egui_view_flyout_part_is_interactive(local, self, snapshot, hit_part))
        {
            local->current_part = hit_part;
            egui_view_flyout_activate_current_part(self);
        }
        handled = egui_view_flyout_clear_pressed_state(self, local);
        if (handled)
        {
            egui_view_invalidate(self);
        }
        return handled || hit_part != EGUI_VIEW_FLYOUT_PART_NONE;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (egui_view_flyout_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_flyout_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    EGUI_UNUSED(event);

    if (egui_view_flyout_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_flyout_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    const egui_view_flyout_snapshot_t *snapshot = egui_view_flyout_get_snapshot(local);

    if (snapshot == NULL || local->compact_mode || local->disabled_mode || !egui_view_get_enable(self))
    {
        if (egui_view_flyout_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            if (!egui_view_flyout_part_is_interactive(local, self, snapshot, local->current_part))
            {
                return 0;
            }
            local->pressed_part = local->current_part;
            egui_view_set_pressed(self, 1);
            egui_view_invalidate(self);
            return 1;
        }

        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            uint8_t handled = 0;

            if (local->pressed_part != EGUI_VIEW_FLYOUT_PART_NONE && local->pressed_part == local->current_part &&
                egui_view_flyout_part_is_interactive(local, self, snapshot, local->pressed_part))
            {
                handled = egui_view_flyout_activate_current_part(self);
            }
            if (egui_view_flyout_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return handled;
        }

        return 0;
    }

    if (egui_view_flyout_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }

    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_LEFT:
        egui_view_flyout_move_linear(self, -1, 0);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
        egui_view_flyout_move_linear(self, 1, 0);
        return 1;
    case EGUI_KEY_CODE_TAB:
        egui_view_flyout_move_linear(self, 1, 1);
        return 1;
    case EGUI_KEY_CODE_UP:
        egui_view_flyout_move_vertical(self, -1);
        return 1;
    case EGUI_KEY_CODE_DOWN:
        egui_view_flyout_move_vertical(self, 1);
        return 1;
    case EGUI_KEY_CODE_HOME:
        egui_view_flyout_set_current_part_inner(self, EGUI_VIEW_FLYOUT_PART_TARGET, 0);
        return 1;
    case EGUI_KEY_CODE_END:
    {
        uint8_t target = egui_view_flyout_find_last_action(snapshot, local->open_state, 1);

        if (target == EGUI_VIEW_FLYOUT_PART_NONE)
        {
            target = egui_view_flyout_find_last_action(snapshot, local->open_state, 0);
        }
        if (target == EGUI_VIEW_FLYOUT_PART_NONE)
        {
            target = EGUI_VIEW_FLYOUT_PART_TARGET;
        }
        egui_view_flyout_set_current_part_inner(self, target, 0);
        return 1;
    }
    case EGUI_KEY_CODE_ESCAPE:
        if (local->open_state)
        {
            egui_view_flyout_set_open(self, 0);
            return 1;
        }
        return 0;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_flyout_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_flyout_t);
    EGUI_UNUSED(event);

    if (egui_view_flyout_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_flyout_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_flyout_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_flyout_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_flyout_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_flyout_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_flyout_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_flyout_on_key_event,
#endif
};

void egui_view_flyout_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_flyout_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_flyout_t);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 1);
#endif

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_action = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD5DDE5);
    local->text_color = EGUI_COLOR_HEX(0x1E2C39);
    local->muted_text_color = EGUI_COLOR_HEX(0x6C7A88);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->success_color = EGUI_COLOR_HEX(0x0F7B45);
    local->warning_color = EGUI_COLOR_HEX(0x9D5D00);
    local->neutral_color = EGUI_COLOR_HEX(0x748190);
    local->shadow_color = EGUI_COLOR_HEX(0xDDE5EB);
    local->target_fill_color = EGUI_COLOR_HEX(0xF5F8FB);
    local->target_border_color = EGUI_COLOR_HEX(0xD0D9E2);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_part = EGUI_VIEW_FLYOUT_PART_TARGET;
    local->open_state = 0;
    local->compact_mode = 0;
    local->disabled_mode = 0;
    local->pressed_part = EGUI_VIEW_FLYOUT_PART_NONE;

    egui_view_set_view_name(self, "egui_view_flyout");
}
