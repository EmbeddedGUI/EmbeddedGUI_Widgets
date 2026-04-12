#include "egui_view_tool_tip.h"

#define EGUI_VIEW_TOOL_TIP_DELAY_DEFAULT 420
#define EGUI_VIEW_TOOL_TIP_DELAY_MIN     120

#define EGUI_VIEW_TOOL_TIP_STANDARD_RADIUS        10
#define EGUI_VIEW_TOOL_TIP_STANDARD_FILL_ALPHA    90
#define EGUI_VIEW_TOOL_TIP_STANDARD_BORDER_ALPHA  52
#define EGUI_VIEW_TOOL_TIP_STANDARD_PAD_X         10
#define EGUI_VIEW_TOOL_TIP_STANDARD_PAD_Y         9
#define EGUI_VIEW_TOOL_TIP_STANDARD_TARGET_HEIGHT 24
#define EGUI_VIEW_TOOL_TIP_STANDARD_TARGET_GAP    8
#define EGUI_VIEW_TOOL_TIP_STANDARD_BUBBLE_HEIGHT 68
#define EGUI_VIEW_TOOL_TIP_STANDARD_BUBBLE_RADIUS 8
#define EGUI_VIEW_TOOL_TIP_STANDARD_ARROW_WIDTH   14
#define EGUI_VIEW_TOOL_TIP_STANDARD_ARROW_HEIGHT  8

#define EGUI_VIEW_TOOL_TIP_COMPACT_RADIUS        8
#define EGUI_VIEW_TOOL_TIP_COMPACT_FILL_ALPHA    88
#define EGUI_VIEW_TOOL_TIP_COMPACT_BORDER_ALPHA  48
#define EGUI_VIEW_TOOL_TIP_COMPACT_PAD_X         8
#define EGUI_VIEW_TOOL_TIP_COMPACT_PAD_Y         7
#define EGUI_VIEW_TOOL_TIP_COMPACT_TARGET_HEIGHT 18
#define EGUI_VIEW_TOOL_TIP_COMPACT_TARGET_GAP    5
#define EGUI_VIEW_TOOL_TIP_COMPACT_BUBBLE_HEIGHT 42
#define EGUI_VIEW_TOOL_TIP_COMPACT_BUBBLE_RADIUS 6
#define EGUI_VIEW_TOOL_TIP_COMPACT_ARROW_WIDTH   10
#define EGUI_VIEW_TOOL_TIP_COMPACT_ARROW_HEIGHT  5

typedef struct egui_view_tool_tip_metrics egui_view_tool_tip_metrics_t;
struct egui_view_tool_tip_metrics
{
    egui_region_t region;
    egui_region_t target_region;
    egui_region_t bubble_region;
    egui_region_t hint_region;
    egui_region_t title_region;
    egui_region_t body_region;
    egui_dim_t arrow_center_x;
    uint8_t show_bubble;
    uint8_t show_hint;
    uint8_t show_body;
};

static egui_view_tool_tip_t *egui_view_tool_tip_local(egui_view_t *self)
{
    return (egui_view_tool_tip_t *)self;
}

static const egui_view_tool_tip_snapshot_t *egui_view_tool_tip_get_snapshot(egui_view_tool_tip_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }

    return &local->snapshots[local->current_snapshot];
}

static uint8_t egui_view_tool_tip_has_text(const char *text)
{
    return (text != NULL && text[0] != '\0') ? 1 : 0;
}

static uint8_t egui_view_tool_tip_text_len(const char *text)
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

static uint8_t egui_view_tool_tip_clamp_snapshot_count(uint8_t count)
{
    if (count > EGUI_VIEW_TOOL_TIP_MAX_SNAPSHOTS)
    {
        return EGUI_VIEW_TOOL_TIP_MAX_SNAPSHOTS;
    }

    return count;
}

static uint16_t egui_view_tool_tip_resolve_delay(uint16_t show_delay_ms)
{
    if (show_delay_ms == 0)
    {
        return EGUI_VIEW_TOOL_TIP_DELAY_DEFAULT;
    }
    if (show_delay_ms < EGUI_VIEW_TOOL_TIP_DELAY_MIN)
    {
        return EGUI_VIEW_TOOL_TIP_DELAY_MIN;
    }

    return show_delay_ms;
}

static egui_color_t egui_view_tool_tip_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x7D8894), 60);
}

static egui_color_t egui_view_tool_tip_tone_color(egui_view_tool_tip_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_TOOL_TIP_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_TOOL_TIP_TONE_NEUTRAL:
        return local->neutral_color;
    case EGUI_VIEW_TOOL_TIP_TONE_ACCENT:
    default:
        return local->accent_color;
    }
}

static void egui_view_tool_tip_stop_timer(egui_view_t *self)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    if (!local->timer_started)
    {
        return;
    }

    egui_timer_stop_timer(&local->show_timer);
    local->timer_started = 0;
}

static void egui_view_tool_tip_start_timer(egui_view_t *self)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    if (local->timer_started || !local->pending_show || local->open || !self->is_attached_to_window || !egui_view_get_enable(self) || local->read_only_mode)
    {
        return;
    }

    egui_timer_start_timer(&local->show_timer, local->show_delay_ms, 0);
    local->timer_started = 1;
}

static uint8_t egui_view_tool_tip_clear_interaction_state(egui_view_t *self)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);
    uint8_t had_state = (uint8_t)(self->is_pressed || local->touch_active || local->key_active || local->timer_started || local->pending_show ||
                                  local->toggle_on_release || local->current_part != EGUI_VIEW_TOOL_TIP_PART_NONE);

    egui_view_tool_tip_stop_timer(self);
    local->pending_show = 0;
    local->touch_active = 0;
    local->key_active = 0;
    local->toggle_on_release = 0;
    local->current_part = EGUI_VIEW_TOOL_TIP_PART_NONE;
    egui_view_set_pressed(self, 0);
    return had_state;
}

static void egui_view_tool_tip_request_focus_from_touch(egui_view_t *self, uint8_t is_inside)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (is_inside && self->is_focusable)
    {
        egui_view_request_focus(self);
    }
    else if (is_inside && !self->is_no_focus_clear)
    {
        egui_focus_manager_clear_focus();
    }
#else
    EGUI_UNUSED(self);
    EGUI_UNUSED(is_inside);
#endif
}

static void egui_view_tool_tip_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                         egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (text == NULL || text[0] == '\0')
    {
        return;
    }

    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static void egui_view_tool_tip_get_metrics(egui_view_tool_tip_t *local, egui_view_t *self, const egui_view_tool_tip_snapshot_t *snapshot,
                                           egui_view_tool_tip_metrics_t *metrics)
{
    egui_dim_t pad_x = local->compact_mode ? EGUI_VIEW_TOOL_TIP_COMPACT_PAD_X : EGUI_VIEW_TOOL_TIP_STANDARD_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? EGUI_VIEW_TOOL_TIP_COMPACT_PAD_Y : EGUI_VIEW_TOOL_TIP_STANDARD_PAD_Y;
    egui_dim_t target_h = local->compact_mode ? EGUI_VIEW_TOOL_TIP_COMPACT_TARGET_HEIGHT : EGUI_VIEW_TOOL_TIP_STANDARD_TARGET_HEIGHT;
    egui_dim_t gap = local->compact_mode ? EGUI_VIEW_TOOL_TIP_COMPACT_TARGET_GAP : EGUI_VIEW_TOOL_TIP_STANDARD_TARGET_GAP;
    egui_dim_t bubble_h = local->compact_mode ? EGUI_VIEW_TOOL_TIP_COMPACT_BUBBLE_HEIGHT : EGUI_VIEW_TOOL_TIP_STANDARD_BUBBLE_HEIGHT;
    egui_dim_t bubble_w;
    egui_dim_t target_w;
    egui_dim_t target_x;
    egui_dim_t target_y;
    egui_dim_t bubble_y;
    egui_dim_t min_target_w = local->compact_mode ? 44 : 56;
    int16_t target_offset_x = (snapshot != NULL) ? snapshot->target_offset_x : 0;

    egui_view_get_work_region(self, &metrics->region);
    bubble_w = metrics->region.size.width - pad_x * 2;
    if (bubble_w < (local->compact_mode ? 56 : 92))
    {
        bubble_w = local->compact_mode ? 56 : 92;
    }

    target_w = (egui_dim_t)(min_target_w + egui_view_tool_tip_text_len(snapshot != NULL ? snapshot->target_label : "") * (local->compact_mode ? 4 : 5));
    if (target_w > bubble_w - (local->compact_mode ? 8 : 12))
    {
        target_w = bubble_w - (local->compact_mode ? 8 : 12);
    }
    if (target_w < min_target_w)
    {
        target_w = min_target_w;
    }

    target_x = metrics->region.location.x + (metrics->region.size.width - target_w) / 2 + target_offset_x;
    if (target_x < metrics->region.location.x + pad_x)
    {
        target_x = metrics->region.location.x + pad_x;
    }
    if (target_x + target_w > metrics->region.location.x + metrics->region.size.width - pad_x)
    {
        target_x = metrics->region.location.x + metrics->region.size.width - pad_x - target_w;
    }

    if (snapshot != NULL && snapshot->placement == EGUI_VIEW_TOOL_TIP_PLACEMENT_TOP)
    {
        bubble_y = metrics->region.location.y + pad_y;
        target_y = bubble_y + bubble_h + gap;
    }
    else
    {
        target_y = metrics->region.location.y + pad_y + (local->compact_mode ? 2 : 4);
        bubble_y = target_y + target_h + gap;
    }

    metrics->target_region.location.x = target_x;
    metrics->target_region.location.y = target_y;
    metrics->target_region.size.width = target_w;
    metrics->target_region.size.height = target_h;

    metrics->bubble_region.location.x = metrics->region.location.x + pad_x;
    metrics->bubble_region.location.y = bubble_y;
    metrics->bubble_region.size.width = bubble_w;
    metrics->bubble_region.size.height = bubble_h;
    metrics->arrow_center_x = metrics->target_region.location.x + metrics->target_region.size.width / 2;
    if (metrics->arrow_center_x < metrics->bubble_region.location.x + 12)
    {
        metrics->arrow_center_x = metrics->bubble_region.location.x + 12;
    }
    if (metrics->arrow_center_x > metrics->bubble_region.location.x + metrics->bubble_region.size.width - 12)
    {
        metrics->arrow_center_x = metrics->bubble_region.location.x + metrics->bubble_region.size.width - 12;
    }

    metrics->show_bubble = local->open ? 1 : 0;
    metrics->show_hint = (snapshot != NULL && egui_view_tool_tip_has_text(snapshot->hint)) ? 1 : 0;
    metrics->show_body = (snapshot != NULL && egui_view_tool_tip_has_text(snapshot->body) && !local->compact_mode) ? 1 : 0;

    metrics->hint_region.location.x = metrics->bubble_region.location.x + (local->compact_mode ? 8 : 10);
    metrics->hint_region.location.y = metrics->bubble_region.location.y + (local->compact_mode ? 6 : 8);
    metrics->hint_region.size.width = metrics->bubble_region.size.width - (local->compact_mode ? 16 : 20);
    metrics->hint_region.size.height = local->compact_mode ? 8 : 10;

    metrics->title_region.location.x = metrics->bubble_region.location.x + (local->compact_mode ? 8 : 10);
    metrics->title_region.location.y = metrics->bubble_region.location.y + (metrics->show_hint ? (local->compact_mode ? 16 : 20) : (local->compact_mode ? 10 : 12));
    metrics->title_region.size.width = metrics->bubble_region.size.width - (local->compact_mode ? 16 : 20);
    metrics->title_region.size.height = local->compact_mode ? 12 : 14;

    metrics->body_region.location.x = metrics->bubble_region.location.x + (local->compact_mode ? 8 : 10);
    metrics->body_region.location.y = metrics->title_region.location.y + metrics->title_region.size.height + (local->compact_mode ? 0 : 6);
    metrics->body_region.size.width = metrics->bubble_region.size.width - (local->compact_mode ? 16 : 20);
    metrics->body_region.size.height = metrics->bubble_region.location.y + metrics->bubble_region.size.height - metrics->body_region.location.y - (local->compact_mode ? 6 : 8);
}

static void egui_view_tool_tip_draw_focus_ring(const egui_region_t *region, egui_dim_t radius, egui_color_t color, egui_alpha_t alpha)
{
    egui_canvas_draw_round_rectangle(region->location.x - 2, region->location.y - 2, region->size.width + 4, region->size.height + 4, radius + 2, 1, color,
                                     alpha);
}

static void egui_view_tool_tip_draw_arrow(egui_view_t *self, egui_view_tool_tip_t *local, const egui_view_tool_tip_snapshot_t *snapshot,
                                          const egui_view_tool_tip_metrics_t *metrics, egui_color_t fill_color, egui_color_t border_color)
{
    egui_dim_t arrow_w = local->compact_mode ? EGUI_VIEW_TOOL_TIP_COMPACT_ARROW_WIDTH : EGUI_VIEW_TOOL_TIP_STANDARD_ARROW_WIDTH;
    egui_dim_t arrow_h = local->compact_mode ? EGUI_VIEW_TOOL_TIP_COMPACT_ARROW_HEIGHT : EGUI_VIEW_TOOL_TIP_STANDARD_ARROW_HEIGHT;
    egui_dim_t center_x = metrics->arrow_center_x;

    if (snapshot == NULL || !metrics->show_bubble)
    {
        return;
    }

    if (snapshot->placement == EGUI_VIEW_TOOL_TIP_PLACEMENT_TOP)
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

static void egui_view_tool_tip_draw_target(egui_view_t *self, egui_view_tool_tip_t *local, const egui_view_tool_tip_snapshot_t *snapshot,
                                           const egui_view_tool_tip_metrics_t *metrics, egui_color_t tone_color, egui_color_t text_color,
                                           egui_color_t border_color)
{
    egui_color_t target_fill = egui_rgb_mix(local->target_fill_color, tone_color, local->open ? 10 : 4);
    egui_color_t target_outline = egui_rgb_mix(local->target_border_color, tone_color, local->open ? 14 : 6);
    egui_color_t target_text = egui_rgb_mix(text_color, tone_color, local->open ? 10 : 3);
    egui_color_t focus_color = egui_rgb_mix(tone_color, EGUI_COLOR_HEX(0xF4FAFF), 18);
    egui_alpha_t fill_alpha = egui_color_alpha_mix(self->alpha, local->read_only_mode ? 68 : (self->is_pressed ? 86 : 92));
    egui_dim_t radius = local->compact_mode ? 6 : 8;

    if (!egui_view_get_enable(self))
    {
        target_fill = egui_view_tool_tip_mix_disabled(target_fill);
        target_outline = egui_view_tool_tip_mix_disabled(target_outline);
        target_text = egui_view_tool_tip_mix_disabled(target_text);
    }
    else if (local->read_only_mode)
    {
        target_fill = egui_rgb_mix(target_fill, local->surface_color, 34);
        target_outline = egui_rgb_mix(target_outline, local->muted_text_color, 36);
        target_text = egui_rgb_mix(target_text, local->muted_text_color, 26);
    }

    if (self->is_focused && egui_view_get_enable(self))
    {
        egui_view_tool_tip_draw_focus_ring(&metrics->target_region, radius, focus_color, egui_color_alpha_mix(self->alpha, 54));
    }

    egui_canvas_draw_round_rectangle_fill(metrics->target_region.location.x, metrics->target_region.location.y, metrics->target_region.size.width,
                                          metrics->target_region.size.height, radius, target_fill, fill_alpha);
    egui_canvas_draw_round_rectangle(metrics->target_region.location.x, metrics->target_region.location.y, metrics->target_region.size.width,
                                     metrics->target_region.size.height, radius, 1, target_outline, egui_color_alpha_mix(self->alpha, 52));
    if (local->open && metrics->target_region.size.width > 18)
    {
        egui_canvas_draw_round_rectangle_fill(metrics->target_region.location.x + 8, metrics->target_region.location.y + metrics->target_region.size.height - 4,
                                              metrics->target_region.size.width - 16, 2, 1, tone_color, egui_color_alpha_mix(self->alpha, 70));
    }

    egui_view_tool_tip_draw_text(local->font, self, snapshot != NULL ? snapshot->target_label : "", &metrics->target_region, EGUI_ALIGN_CENTER, target_text);
}

static void egui_view_tool_tip_draw_bubble(egui_view_t *self, egui_view_tool_tip_t *local, const egui_view_tool_tip_snapshot_t *snapshot,
                                           const egui_view_tool_tip_metrics_t *metrics, egui_color_t tone_color, egui_color_t text_color,
                                           egui_color_t muted_text_color, egui_color_t border_color, egui_color_t shadow_color)
{
    egui_color_t bubble_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 6 : 8);
    egui_color_t bubble_border = egui_rgb_mix(border_color, tone_color, local->compact_mode ? 10 : 14);
    egui_color_t body_color = egui_rgb_mix(text_color, muted_text_color, local->compact_mode ? 62 : 56);
    egui_dim_t bubble_radius = local->compact_mode ? EGUI_VIEW_TOOL_TIP_COMPACT_BUBBLE_RADIUS : EGUI_VIEW_TOOL_TIP_STANDARD_BUBBLE_RADIUS;

    if (!metrics->show_bubble || snapshot == NULL)
    {
        return;
    }

    if (!egui_view_get_enable(self))
    {
        bubble_fill = egui_view_tool_tip_mix_disabled(bubble_fill);
        bubble_border = egui_view_tool_tip_mix_disabled(bubble_border);
        body_color = egui_view_tool_tip_mix_disabled(body_color);
        shadow_color = egui_view_tool_tip_mix_disabled(shadow_color);
    }
    else if (local->read_only_mode)
    {
        bubble_fill = egui_rgb_mix(bubble_fill, local->surface_color, 26);
        bubble_border = egui_rgb_mix(bubble_border, muted_text_color, 26);
        body_color = egui_rgb_mix(body_color, muted_text_color, 20);
        shadow_color = egui_rgb_mix(shadow_color, local->surface_color, 36);
    }

    egui_canvas_draw_round_rectangle_fill(metrics->bubble_region.location.x + 1, metrics->bubble_region.location.y + 2, metrics->bubble_region.size.width,
                                          metrics->bubble_region.size.height, bubble_radius, shadow_color,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 10 : 18));
    egui_view_tool_tip_draw_arrow(self, local, snapshot, metrics, bubble_fill, bubble_border);
    egui_canvas_draw_round_rectangle_fill(metrics->bubble_region.location.x, metrics->bubble_region.location.y, metrics->bubble_region.size.width,
                                          metrics->bubble_region.size.height, bubble_radius, bubble_fill, egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(metrics->bubble_region.location.x, metrics->bubble_region.location.y, metrics->bubble_region.size.width,
                                     metrics->bubble_region.size.height, bubble_radius, 1, bubble_border, egui_color_alpha_mix(self->alpha, 54));
    egui_canvas_draw_round_rectangle_fill(metrics->bubble_region.location.x + (local->compact_mode ? 8 : 10), metrics->bubble_region.location.y + 6,
                                          local->compact_mode ? 14 : 18, 2, 1, tone_color, egui_color_alpha_mix(self->alpha, local->compact_mode ? 58 : 70));

    if (metrics->show_hint)
    {
        egui_view_tool_tip_draw_text(local->meta_font, self, snapshot->hint, &metrics->hint_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, tone_color);
    }
    egui_view_tool_tip_draw_text(local->font, self, snapshot->title, &metrics->title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color);

    if (metrics->show_body)
    {
        egui_view_tool_tip_draw_text(local->meta_font, self, snapshot->body, &metrics->body_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP, body_color);
    }
    else if (local->compact_mode && egui_view_tool_tip_has_text(snapshot->body))
    {
        egui_region_t compact_body_region = metrics->title_region;

        compact_body_region.location.y += 11;
        compact_body_region.size.height = 10;
        egui_view_tool_tip_draw_text(local->meta_font, self, snapshot->body, &compact_body_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, body_color);
    }

    if (local->read_only_mode)
    {
        egui_region_t badge_region;
        egui_color_t badge_fill = egui_rgb_mix(local->surface_color, tone_color, 4);
        egui_color_t badge_text = egui_rgb_mix(muted_text_color, text_color, 18);

        badge_region.size.width = local->compact_mode ? 24 : 30;
        badge_region.size.height = local->compact_mode ? 10 : 11;
        badge_region.location.x = metrics->bubble_region.location.x + metrics->bubble_region.size.width - badge_region.size.width - (local->compact_mode ? 8 : 10);
        badge_region.location.y = metrics->bubble_region.location.y + metrics->bubble_region.size.height - badge_region.size.height - (local->compact_mode ? 6 : 8);
        egui_canvas_draw_round_rectangle_fill(badge_region.location.x, badge_region.location.y, badge_region.size.width, badge_region.size.height, 5, badge_fill,
                                              egui_color_alpha_mix(self->alpha, 46));
        egui_canvas_draw_round_rectangle(badge_region.location.x, badge_region.location.y, badge_region.size.width, badge_region.size.height, 5, 1, bubble_border,
                                         egui_color_alpha_mix(self->alpha, 28));
        egui_view_tool_tip_draw_text(local->meta_font, self, "Read", &badge_region, EGUI_ALIGN_CENTER, badge_text);
    }
}

static void egui_view_tool_tip_on_draw(egui_view_t *self)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);
    const egui_view_tool_tip_snapshot_t *snapshot = egui_view_tool_tip_get_snapshot(local);
    egui_view_tool_tip_metrics_t metrics;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_color_t muted_text_color = local->muted_text_color;
    egui_color_t shadow_color = local->shadow_color;
    egui_color_t tone_color;
    egui_dim_t panel_radius;

    egui_view_get_work_region(self, &metrics.region);
    if (metrics.region.size.width <= 0 || metrics.region.size.height <= 0)
    {
        return;
    }

    tone_color = egui_view_tool_tip_tone_color(local, snapshot != NULL ? snapshot->tone : EGUI_VIEW_TOOL_TIP_TONE_ACCENT);
    if (local->read_only_mode)
    {
        tone_color = egui_rgb_mix(tone_color, muted_text_color, 44);
        border_color = egui_rgb_mix(border_color, muted_text_color, 12);
        text_color = egui_rgb_mix(text_color, muted_text_color, 20);
        muted_text_color = egui_rgb_mix(muted_text_color, surface_color, 10);
        shadow_color = egui_rgb_mix(shadow_color, surface_color, 36);
    }
    if (!egui_view_get_enable(self))
    {
        tone_color = egui_view_tool_tip_mix_disabled(tone_color);
        surface_color = egui_view_tool_tip_mix_disabled(surface_color);
        border_color = egui_view_tool_tip_mix_disabled(border_color);
        text_color = egui_view_tool_tip_mix_disabled(text_color);
        muted_text_color = egui_view_tool_tip_mix_disabled(muted_text_color);
        shadow_color = egui_view_tool_tip_mix_disabled(shadow_color);
    }

    egui_view_tool_tip_get_metrics(local, self, snapshot, &metrics);
    panel_radius = local->compact_mode ? EGUI_VIEW_TOOL_TIP_COMPACT_RADIUS : EGUI_VIEW_TOOL_TIP_STANDARD_RADIUS;
    egui_canvas_draw_round_rectangle_fill(metrics.region.location.x, metrics.region.location.y, metrics.region.size.width, metrics.region.size.height, panel_radius,
                                          surface_color, egui_color_alpha_mix(self->alpha,
                                                                              local->compact_mode ? EGUI_VIEW_TOOL_TIP_COMPACT_FILL_ALPHA
                                                                                                 : EGUI_VIEW_TOOL_TIP_STANDARD_FILL_ALPHA));
    egui_canvas_draw_round_rectangle(metrics.region.location.x, metrics.region.location.y, metrics.region.size.width, metrics.region.size.height, panel_radius, 1,
                                     border_color, egui_color_alpha_mix(self->alpha,
                                                                        local->compact_mode ? EGUI_VIEW_TOOL_TIP_COMPACT_BORDER_ALPHA
                                                                                           : EGUI_VIEW_TOOL_TIP_STANDARD_BORDER_ALPHA));

    egui_view_tool_tip_draw_target(self, local, snapshot, &metrics, tone_color, text_color, border_color);
    egui_view_tool_tip_draw_bubble(self, local, snapshot, &metrics, tone_color, text_color, muted_text_color, border_color, shadow_color);
}

static void egui_view_tool_tip_tick(egui_timer_t *timer)
{
    egui_view_t *self = (egui_view_t *)timer->user_data;
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    egui_timer_stop_timer(&local->show_timer);
    local->timer_started = 0;
    if (!self->is_attached_to_window || !local->pending_show || !egui_view_get_enable(self) || local->read_only_mode)
    {
        return;
    }

    local->pending_show = 0;
    local->open = 1;
    local->toggle_on_release = 0;
    egui_view_invalidate(self);
}

void egui_view_tool_tip_set_snapshots(egui_view_t *self, const egui_view_tool_tip_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    egui_view_tool_tip_clear_interaction_state(self);
    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : egui_view_tool_tip_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }
    egui_view_invalidate(self);
}

void egui_view_tool_tip_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    if (local->snapshot_count == 0 || snapshot_index >= local->snapshot_count)
    {
        if (egui_view_tool_tip_clear_interaction_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }

    egui_view_tool_tip_clear_interaction_state(self);
    local->current_snapshot = snapshot_index;
    egui_view_invalidate(self);
}

uint8_t egui_view_tool_tip_get_current_snapshot(egui_view_t *self)
{
    return egui_view_tool_tip_local(self)->current_snapshot;
}

void egui_view_tool_tip_set_open(egui_view_t *self, uint8_t is_open)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);
    uint8_t new_open = is_open ? 1 : 0;

    egui_view_tool_tip_clear_interaction_state(self);
    if (local->open == new_open)
    {
        egui_view_invalidate(self);
        return;
    }

    local->open = new_open;
    egui_view_invalidate(self);
}

uint8_t egui_view_tool_tip_get_open(egui_view_t *self)
{
    return egui_view_tool_tip_local(self)->open;
}

void egui_view_tool_tip_set_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    egui_view_tool_tip_clear_interaction_state(self);
    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_tool_tip_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    egui_view_tool_tip_clear_interaction_state(self);
    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_tool_tip_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    egui_view_tool_tip_clear_interaction_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_tool_tip_get_compact_mode(egui_view_t *self)
{
    return egui_view_tool_tip_local(self)->compact_mode;
}

void egui_view_tool_tip_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    egui_view_tool_tip_clear_interaction_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_tool_tip_get_read_only_mode(egui_view_t *self)
{
    return egui_view_tool_tip_local(self)->read_only_mode;
}

void egui_view_tool_tip_set_show_delay(egui_view_t *self, uint16_t show_delay_ms)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    egui_view_tool_tip_clear_interaction_state(self);
    local->show_delay_ms = egui_view_tool_tip_resolve_delay(show_delay_ms);
    egui_view_invalidate(self);
}

uint16_t egui_view_tool_tip_get_show_delay(egui_view_t *self)
{
    return egui_view_tool_tip_local(self)->show_delay_ms;
}

void egui_view_tool_tip_begin_show_delay(egui_view_t *self)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    if (local->open || local->read_only_mode || !egui_view_get_enable(self))
    {
        return;
    }

    egui_view_tool_tip_stop_timer(self);
    local->pending_show = 1;
    local->toggle_on_release = 0;
    local->current_part = EGUI_VIEW_TOOL_TIP_PART_TARGET;
    egui_view_tool_tip_start_timer(self);
    egui_view_invalidate(self);
}

void egui_view_tool_tip_cancel_show_delay(egui_view_t *self)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    egui_view_tool_tip_stop_timer(self);
    if (!local->pending_show)
    {
        if (!local->open && !local->touch_active && !local->key_active)
        {
            local->current_part = EGUI_VIEW_TOOL_TIP_PART_NONE;
            egui_view_invalidate(self);
        }
        return;
    }

    local->pending_show = 0;
    if (!local->open && !local->touch_active && !local->key_active)
    {
        local->current_part = EGUI_VIEW_TOOL_TIP_PART_NONE;
    }
    egui_view_invalidate(self);
}

void egui_view_tool_tip_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                    egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t warning_color, egui_color_t neutral_color,
                                    egui_color_t shadow_color, egui_color_t target_fill_color, egui_color_t target_border_color)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    egui_view_tool_tip_clear_interaction_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    local->shadow_color = shadow_color;
    local->target_fill_color = target_fill_color;
    local->target_border_color = target_border_color;
    egui_view_invalidate(self);
}

uint8_t egui_view_tool_tip_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);
    const egui_view_tool_tip_snapshot_t *snapshot = egui_view_tool_tip_get_snapshot(local);
    egui_view_tool_tip_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }

    egui_view_tool_tip_get_metrics(local, self, snapshot, &metrics);
    switch (part)
    {
    case EGUI_VIEW_TOOL_TIP_PART_TARGET:
        *region = metrics.target_region;
        return 1;
    case EGUI_VIEW_TOOL_TIP_PART_BUBBLE:
        if (!metrics.show_bubble)
        {
            return 0;
        }
        *region = metrics.bubble_region;
        return 1;
    default:
        return 0;
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_tool_tip_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_tool_tip_clear_interaction_state(self);
    return 1;
}

static int egui_view_tool_tip_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);
    egui_region_t target_region;
    uint8_t is_inside_target;
    uint8_t was_pressed;

    if (!egui_view_get_clickable(self))
    {
        return 0;
    }

    egui_view_tool_tip_get_part_region(self, EGUI_VIEW_TOOL_TIP_PART_TARGET, &target_region);
    is_inside_target = egui_region_pt_in_rect(&target_region, event->location.x, event->location.y) ? 1 : 0;
    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        if (egui_view_tool_tip_clear_interaction_state(self))
        {
            egui_view_invalidate(self);
        }
        return 1;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        if (!is_inside_target)
        {
            return 0;
        }
        local->key_active = 0;
        local->touch_active = 1;
        local->toggle_on_release = local->open ? 1 : 0;
        local->current_part = EGUI_VIEW_TOOL_TIP_PART_TARGET;
        egui_view_tool_tip_cancel_show_delay(self);
        egui_view_set_pressed(self, 1);
        egui_view_tool_tip_request_focus_from_touch(self, 1);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (!local->touch_active)
        {
            return 0;
        }
        egui_view_set_pressed(self, is_inside_target ? 1 : 0);
        local->current_part = is_inside_target ? EGUI_VIEW_TOOL_TIP_PART_TARGET : EGUI_VIEW_TOOL_TIP_PART_NONE;
        if (!is_inside_target)
        {
            local->toggle_on_release = 0;
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (!local->touch_active)
        {
            return is_inside_target ? 1 : 0;
        }
        was_pressed = self->is_pressed ? 1 : 0;
        local->touch_active = 0;
        egui_view_set_pressed(self, 0);
        if (!is_inside_target && !local->open)
        {
            local->current_part = EGUI_VIEW_TOOL_TIP_PART_NONE;
        }
        if (event->type == EGUI_MOTION_EVENT_ACTION_CANCEL)
        {
            local->toggle_on_release = 0;
            local->current_part = local->open ? EGUI_VIEW_TOOL_TIP_PART_TARGET : EGUI_VIEW_TOOL_TIP_PART_NONE;
            egui_view_tool_tip_cancel_show_delay(self);
            egui_view_invalidate(self);
            return 1;
        }
        if (was_pressed && is_inside_target)
        {
            if (local->toggle_on_release)
            {
                local->toggle_on_release = 0;
                egui_view_tool_tip_set_open(self, 0);
                return 1;
            }
            egui_view_tool_tip_begin_show_delay(self);
            return 1;
        }
        local->toggle_on_release = 0;
        egui_view_tool_tip_cancel_show_delay(self);
        egui_view_invalidate(self);
        return 1;
    default:
        return 1;
    }
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_tool_tip_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_tool_tip_clear_interaction_state(self);
    return 1;
}

static int egui_view_tool_tip_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        if (egui_view_tool_tip_clear_interaction_state(self))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            if (!local->key_active)
            {
                local->touch_active = 0;
                local->key_active = 1;
                local->toggle_on_release = local->open ? 1 : 0;
                local->current_part = EGUI_VIEW_TOOL_TIP_PART_TARGET;
                egui_view_tool_tip_cancel_show_delay(self);
                egui_view_set_pressed(self, 1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
                egui_view_request_focus(self);
#endif
            }
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            local->key_active = 0;
            egui_view_set_pressed(self, 0);
            if (local->toggle_on_release)
            {
                local->toggle_on_release = 0;
                egui_view_tool_tip_set_open(self, 0);
                return 1;
            }
            egui_view_tool_tip_begin_show_delay(self);
            return 1;
        }
        return 0;
    case EGUI_KEY_CODE_ESCAPE:
        if (local->open || local->pending_show)
        {
            egui_view_tool_tip_set_open(self, 0);
            return 1;
        }
        if (egui_view_tool_tip_clear_interaction_state(self))
        {
            egui_view_invalidate(self);
        }
        return 1;
    default:
        if (egui_view_tool_tip_clear_interaction_state(self))
        {
            egui_view_invalidate(self);
        }
        return egui_view_on_key_event(self, event);
    }
}
#endif

static void egui_view_tool_tip_on_attach(egui_view_t *self)
{
    egui_view_on_attach_to_window(self);
    egui_view_tool_tip_start_timer(self);
}

static void egui_view_tool_tip_on_detach(egui_view_t *self)
{
    egui_view_tool_tip_stop_timer(self);
    egui_view_on_detach_from_window(self);
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_tool_tip_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_tool_tip_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_tool_tip_on_attach,
        .on_draw = egui_view_tool_tip_on_draw,
        .on_detach_from_window = egui_view_tool_tip_on_detach,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_tool_tip_on_key_event,
#endif
};

void egui_view_tool_tip_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_tool_tip_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_tool_tip_on_static_key_event;
#endif
}

void egui_view_tool_tip_init(egui_view_t *self)
{
    egui_view_tool_tip_t *local = egui_view_tool_tip_local(self);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_tool_tip_t);

    egui_timer_init_timer(&local->show_timer, self, egui_view_tool_tip_tick);
    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD5DDE5);
    local->text_color = EGUI_COLOR_HEX(0x1E2C39);
    local->muted_text_color = EGUI_COLOR_HEX(0x6C7A88);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->warning_color = EGUI_COLOR_HEX(0x9D5D00);
    local->neutral_color = EGUI_COLOR_HEX(0x6A7785);
    local->shadow_color = EGUI_COLOR_HEX(0xDDE5EB);
    local->target_fill_color = EGUI_COLOR_HEX(0xF5F8FB);
    local->target_border_color = EGUI_COLOR_HEX(0xD0D9E2);
    local->show_delay_ms = EGUI_VIEW_TOOL_TIP_DELAY_DEFAULT;
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_part = EGUI_VIEW_TOOL_TIP_PART_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->open = 0;
    local->timer_started = 0;
    local->pending_show = 0;
    local->touch_active = 0;
    local->key_active = 0;
    local->toggle_on_release = 0;

    egui_view_set_clickable(self, 1);
    egui_view_set_focusable(self, 1);
    egui_view_set_view_name(self, "egui_view_tool_tip");
}
