#include "egui_view_grid_splitter.h"

#define EGUI_VIEW_GRID_SPLITTER_STANDARD_RADIUS       10
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_OUTER_PAD_X  8
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_OUTER_PAD_Y  8
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_INNER_PAD_X  8
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_INNER_PAD_Y  7
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_BADGE_H      11
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_BADGE_GAP    5
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_TITLE_H      12
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_SUMMARY_H    10
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_TITLE_GAP    2
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_SHELL_GAP    7
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_SHELL_RADIUS 8
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_HANDLE_W     12
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_FOOTER_H     11
#define EGUI_VIEW_GRID_SPLITTER_STANDARD_FOOTER_GAP   5

#define EGUI_VIEW_GRID_SPLITTER_COMPACT_RADIUS       8
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_OUTER_PAD_X  6
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_OUTER_PAD_Y  6
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_INNER_PAD_X  6
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_INNER_PAD_Y  5
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_BADGE_H      8
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_BADGE_GAP    4
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_TITLE_H      10
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_SUMMARY_H    0
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_TITLE_GAP    0
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_SHELL_GAP    4
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_SHELL_RADIUS 6
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_HANDLE_W     8
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_FOOTER_H     0
#define EGUI_VIEW_GRID_SPLITTER_COMPACT_FOOTER_GAP   0

typedef struct egui_view_grid_splitter_metrics egui_view_grid_splitter_metrics_t;
struct egui_view_grid_splitter_metrics
{
    egui_region_t content_region;
    egui_region_t badge_region;
    egui_region_t title_region;
    egui_region_t summary_region;
    egui_region_t shell_region;
    egui_region_t left_region;
    egui_region_t handle_region;
    egui_region_t right_region;
    egui_region_t footer_region;
};

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_grid_splitter_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_grid_splitter_on_static_key_event(egui_view_t *self, egui_key_event_t *event);
#endif

static uint8_t grid_splitter_clamp_snapshot_count(uint8_t count)
{
    return count > EGUI_VIEW_GRID_SPLITTER_MAX_SNAPSHOTS ? EGUI_VIEW_GRID_SPLITTER_MAX_SNAPSHOTS : count;
}

static uint8_t grid_splitter_clamp_ratio(uint8_t ratio)
{
    if (ratio < EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_MIN)
    {
        return EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_MIN;
    }
    if (ratio > EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_MAX)
    {
        return EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_MAX;
    }
    return ratio;
}

static uint8_t grid_splitter_text_len(const char *text)
{
    uint8_t len = 0;

    if (text == NULL)
    {
        return 0;
    }
    while (text[len] != '\0')
    {
        len++;
    }
    return len;
}

static uint8_t grid_splitter_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static egui_color_t grid_splitter_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static uint8_t grid_splitter_region_contains_point(const egui_region_t *region, egui_dim_t x, egui_dim_t y)
{
    if (region == NULL || region->size.width <= 0 || region->size.height <= 0)
    {
        return 0;
    }
    return x >= region->location.x && x < region->location.x + region->size.width && y >= region->location.y &&
           y < region->location.y + region->size.height;
}

static const egui_view_grid_splitter_snapshot_t *grid_splitter_get_snapshot(egui_view_grid_splitter_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }
    return &local->snapshots[local->current_snapshot];
}

static uint8_t grid_splitter_resolve_default_ratio(const egui_view_grid_splitter_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_DEFAULT;
    }
    return grid_splitter_clamp_ratio(snapshot->split_ratio);
}

static uint8_t grid_splitter_clear_pressed_state(egui_view_t *self, egui_view_grid_splitter_t *local)
{
    uint8_t had_pressed = self->is_pressed || local->pressed_handle || local->handle_dragging;

    local->pressed_handle = 0;
    local->handle_dragging = 0;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static void grid_splitter_notify_ratio_changed(egui_view_t *self, egui_view_grid_splitter_t *local)
{
    if (local->on_ratio_changed != NULL)
    {
        local->on_ratio_changed(self, local->current_snapshot, local->split_ratio);
    }
}

static void grid_splitter_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                    egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !grid_splitter_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static egui_dim_t grid_splitter_pill_width(const char *text, uint8_t compact_mode, egui_dim_t min_width, egui_dim_t max_width)
{
    egui_dim_t width = min_width;

    if (grid_splitter_has_text(text))
    {
        width += grid_splitter_text_len(text) * (compact_mode ? 4 : 5);
    }
    if (width > max_width)
    {
        width = max_width;
    }
    return width;
}

static void grid_splitter_get_metrics(egui_view_grid_splitter_t *local, egui_view_t *self, egui_view_grid_splitter_metrics_t *metrics)
{
    const egui_view_grid_splitter_snapshot_t *snapshot = grid_splitter_get_snapshot(local);
    egui_region_t work_region;
    egui_dim_t outer_pad_x = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_OUTER_PAD_X : EGUI_VIEW_GRID_SPLITTER_STANDARD_OUTER_PAD_X;
    egui_dim_t outer_pad_y = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_OUTER_PAD_Y : EGUI_VIEW_GRID_SPLITTER_STANDARD_OUTER_PAD_Y;
    egui_dim_t inner_pad_x = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_INNER_PAD_X : EGUI_VIEW_GRID_SPLITTER_STANDARD_INNER_PAD_X;
    egui_dim_t inner_pad_y = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_INNER_PAD_Y : EGUI_VIEW_GRID_SPLITTER_STANDARD_INNER_PAD_Y;
    egui_dim_t badge_h = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_BADGE_H : EGUI_VIEW_GRID_SPLITTER_STANDARD_BADGE_H;
    egui_dim_t badge_gap = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_BADGE_GAP : EGUI_VIEW_GRID_SPLITTER_STANDARD_BADGE_GAP;
    egui_dim_t title_h = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_TITLE_H : EGUI_VIEW_GRID_SPLITTER_STANDARD_TITLE_H;
    egui_dim_t summary_h = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_SUMMARY_H : EGUI_VIEW_GRID_SPLITTER_STANDARD_SUMMARY_H;
    egui_dim_t title_gap = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_TITLE_GAP : EGUI_VIEW_GRID_SPLITTER_STANDARD_TITLE_GAP;
    egui_dim_t shell_gap = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_SHELL_GAP : EGUI_VIEW_GRID_SPLITTER_STANDARD_SHELL_GAP;
    egui_dim_t handle_w = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_HANDLE_W : EGUI_VIEW_GRID_SPLITTER_STANDARD_HANDLE_W;
    egui_dim_t footer_h = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_FOOTER_H : EGUI_VIEW_GRID_SPLITTER_STANDARD_FOOTER_H;
    egui_dim_t footer_gap = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_FOOTER_GAP : EGUI_VIEW_GRID_SPLITTER_STANDARD_FOOTER_GAP;
    egui_dim_t badge_w;
    egui_dim_t footer_w;
    egui_dim_t cursor_y;
    egui_dim_t shell_h;
    egui_dim_t shell_inner_x;
    egui_dim_t shell_inner_y;
    egui_dim_t shell_inner_w;
    egui_dim_t shell_inner_h;
    egui_dim_t available_w;
    egui_dim_t left_w;
    egui_dim_t right_w;
    egui_dim_t min_pane_w = local->compact_mode ? 18 : 28;

    egui_view_get_work_region(self, &work_region);
    metrics->content_region.location.x = work_region.location.x + outer_pad_x;
    metrics->content_region.location.y = work_region.location.y + outer_pad_y;
    metrics->content_region.size.width = work_region.size.width - outer_pad_x * 2;
    metrics->content_region.size.height = work_region.size.height - outer_pad_y * 2;

    metrics->badge_region.location.x = 0;
    metrics->badge_region.location.y = 0;
    metrics->badge_region.size.width = 0;
    metrics->badge_region.size.height = 0;
    metrics->title_region = metrics->badge_region;
    metrics->summary_region = metrics->badge_region;
    metrics->shell_region = metrics->badge_region;
    metrics->left_region = metrics->badge_region;
    metrics->handle_region = metrics->badge_region;
    metrics->right_region = metrics->badge_region;
    metrics->footer_region = metrics->badge_region;

    if (metrics->content_region.size.width <= 0 || metrics->content_region.size.height <= 0)
    {
        return;
    }

    cursor_y = metrics->content_region.location.y + inner_pad_y;
    if (snapshot != NULL && grid_splitter_has_text(snapshot->header))
    {
        badge_w = grid_splitter_pill_width(snapshot->header, local->compact_mode, local->compact_mode ? 18 : 24,
                                           metrics->content_region.size.width > inner_pad_x * 2 ? metrics->content_region.size.width - inner_pad_x * 2 : 18);
        metrics->badge_region.location.x = metrics->content_region.location.x + inner_pad_x;
        metrics->badge_region.location.y = cursor_y;
        metrics->badge_region.size.width = badge_w;
        metrics->badge_region.size.height = badge_h;
        cursor_y += badge_h + badge_gap;
    }

    metrics->title_region.location.x = metrics->content_region.location.x + inner_pad_x;
    metrics->title_region.location.y = cursor_y;
    metrics->title_region.size.width = metrics->content_region.size.width - inner_pad_x * 2;
    metrics->title_region.size.height = title_h;
    cursor_y += title_h;

    if (summary_h > 0 && snapshot != NULL && grid_splitter_has_text(snapshot->summary))
    {
        cursor_y += title_gap;
        metrics->summary_region.location.x = metrics->content_region.location.x + inner_pad_x;
        metrics->summary_region.location.y = cursor_y;
        metrics->summary_region.size.width = metrics->content_region.size.width - inner_pad_x * 2;
        metrics->summary_region.size.height = summary_h;
        cursor_y += summary_h;
    }

    cursor_y += shell_gap;

    if (footer_h > 0 && snapshot != NULL && grid_splitter_has_text(snapshot->footer))
    {
        footer_w = grid_splitter_pill_width(snapshot->footer, local->compact_mode, local->compact_mode ? 18 : 26,
                                            metrics->content_region.size.width > inner_pad_x * 2 ? metrics->content_region.size.width - inner_pad_x * 2 : 18);
        metrics->footer_region.size.width = footer_w;
        metrics->footer_region.size.height = footer_h;
        metrics->footer_region.location.x = metrics->content_region.location.x + inner_pad_x;
        metrics->footer_region.location.y = metrics->content_region.location.y + metrics->content_region.size.height - inner_pad_y - footer_h;
        shell_h = metrics->footer_region.location.y - footer_gap - cursor_y;
    }
    else
    {
        shell_h = metrics->content_region.location.y + metrics->content_region.size.height - inner_pad_y - cursor_y;
    }

    if (shell_h < (local->compact_mode ? 30 : 42))
    {
        shell_h = local->compact_mode ? 30 : 42;
    }

    metrics->shell_region.location.x = metrics->content_region.location.x + inner_pad_x;
    metrics->shell_region.location.y = cursor_y;
    metrics->shell_region.size.width = metrics->content_region.size.width - inner_pad_x * 2;
    metrics->shell_region.size.height = shell_h;

    shell_inner_x = metrics->shell_region.location.x + 2;
    shell_inner_y = metrics->shell_region.location.y + 2;
    shell_inner_w = metrics->shell_region.size.width - 4;
    shell_inner_h = metrics->shell_region.size.height - 4;

    if (shell_inner_w <= 0 || shell_inner_h <= 0)
    {
        return;
    }

    available_w = shell_inner_w - handle_w;
    if (available_w < min_pane_w * 2)
    {
        min_pane_w = available_w > 0 ? available_w / 2 : 0;
    }

    if (available_w <= 0)
    {
        return;
    }

    left_w = (egui_dim_t)(((int32_t)available_w * local->split_ratio) / 100);
    if (left_w < min_pane_w)
    {
        left_w = min_pane_w;
    }
    if (left_w > available_w - min_pane_w)
    {
        left_w = available_w - min_pane_w;
    }
    right_w = available_w - left_w;

    metrics->left_region.location.x = shell_inner_x;
    metrics->left_region.location.y = shell_inner_y;
    metrics->left_region.size.width = left_w;
    metrics->left_region.size.height = shell_inner_h;

    metrics->handle_region.location.x = shell_inner_x + left_w;
    metrics->handle_region.location.y = shell_inner_y;
    metrics->handle_region.size.width = handle_w;
    metrics->handle_region.size.height = shell_inner_h;

    metrics->right_region.location.x = metrics->handle_region.location.x + handle_w;
    metrics->right_region.location.y = shell_inner_y;
    metrics->right_region.size.width = right_w;
    metrics->right_region.size.height = shell_inner_h;
}

static void grid_splitter_draw_pane(egui_view_t *self, egui_view_grid_splitter_t *local, const egui_region_t *region, const char *title, const char *meta,
                                    const char *body, uint8_t emphasized)
{
    egui_color_t fill_color = egui_rgb_mix(local->surface_color, local->accent_color, emphasized ? 14 : 5);
    egui_color_t border_color = egui_rgb_mix(local->border_color, local->accent_color, emphasized ? 24 : 10);
    egui_color_t stripe_color = egui_rgb_mix(local->section_color, local->accent_color, emphasized ? 34 : 16);
    egui_color_t title_color = egui_rgb_mix(local->text_color, local->accent_color, emphasized ? 16 : 6);
    egui_color_t meta_color = egui_rgb_mix(local->muted_text_color, local->accent_color, emphasized ? 20 : 8);
    egui_color_t body_color = egui_rgb_mix(local->text_color, local->muted_text_color, local->compact_mode ? 20 : 12);
    egui_dim_t radius = local->compact_mode ? 5 : 6;
    egui_dim_t pad_x = local->compact_mode ? 5 : 6;
    egui_dim_t pad_y = local->compact_mode ? 4 : 5;
    egui_dim_t title_h = local->compact_mode ? 9 : 11;
    egui_dim_t meta_h = local->compact_mode ? 8 : 9;
    egui_dim_t body_h = local->compact_mode ? 8 : 9;
    egui_region_t stripe_region;
    egui_region_t title_region;
    egui_region_t meta_region;
    egui_region_t body_region;
    egui_region_t line_region;
    uint8_t i;

    if (region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        fill_color = egui_rgb_mix(fill_color, local->surface_color, 24);
        border_color = egui_rgb_mix(border_color, local->muted_text_color, 20);
        stripe_color = egui_rgb_mix(stripe_color, local->muted_text_color, 28);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 18);
        meta_color = egui_rgb_mix(meta_color, local->muted_text_color, 18);
        body_color = egui_rgb_mix(body_color, local->muted_text_color, 24);
    }
    if (!egui_view_get_enable(self))
    {
        fill_color = grid_splitter_mix_disabled(fill_color);
        border_color = grid_splitter_mix_disabled(border_color);
        stripe_color = grid_splitter_mix_disabled(stripe_color);
        title_color = grid_splitter_mix_disabled(title_color);
        meta_color = grid_splitter_mix_disabled(meta_color);
        body_color = grid_splitter_mix_disabled(body_color);
    }

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, radius, fill_color,
                                          egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, radius, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, 44));

    stripe_region.location.x = region->location.x + 1;
    stripe_region.location.y = region->location.y + 1;
    stripe_region.size.width = region->size.width - 2;
    stripe_region.size.height = local->compact_mode ? 3 : 4;
    if (stripe_region.size.width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(stripe_region.location.x, stripe_region.location.y, stripe_region.size.width, stripe_region.size.height, radius,
                                              stripe_color, egui_color_alpha_mix(self->alpha, emphasized ? 82 : 68));
    }

    title_region.location.x = region->location.x + pad_x;
    title_region.location.y = region->location.y + pad_y + (local->compact_mode ? 1 : 2);
    title_region.size.width = region->size.width - pad_x * 2;
    title_region.size.height = title_h;

    meta_region = title_region;
    meta_region.location.y += title_h;
    meta_region.size.height = meta_h;
    body_region = meta_region;
    body_region.location.y += meta_h + 2;
    body_region.size.height = body_h;

    grid_splitter_draw_text(local->font, self, title, &title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
    grid_splitter_draw_text(local->meta_font, self, meta, &meta_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, meta_color);
    grid_splitter_draw_text(local->meta_font, self, body, &body_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, body_color);

    line_region.location.x = region->location.x + pad_x;
    line_region.location.y = region->location.y + region->size.height - pad_y - (local->compact_mode ? 12 : 16);
    line_region.size.height = local->compact_mode ? 2 : 3;
    for (i = 0; i < 3; i++)
    {
        egui_dim_t shrink = (egui_dim_t)(i * (local->compact_mode ? 5 : 7));

        line_region.size.width = region->size.width - pad_x * 2 - shrink;
        if (line_region.size.width <= 0)
        {
            continue;
        }
        egui_canvas_draw_round_rectangle_fill(line_region.location.x, line_region.location.y + i * (local->compact_mode ? 4 : 5), line_region.size.width,
                                              line_region.size.height, 1, stripe_color, egui_color_alpha_mix(self->alpha, emphasized ? 54 : 42));
    }
}

static void grid_splitter_set_current_snapshot_inner(egui_view_t *self, uint8_t snapshot_index, uint8_t notify_ratio)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    uint8_t had_pressed = grid_splitter_clear_pressed_state(self, local);
    uint8_t next_snapshot = 0;
    uint8_t next_ratio = EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_DEFAULT;
    uint8_t changed;

    if (local->snapshot_count > 0)
    {
        next_snapshot = snapshot_index >= local->snapshot_count ? (local->snapshot_count - 1) : snapshot_index;
        next_ratio = grid_splitter_resolve_default_ratio(&local->snapshots[next_snapshot]);
    }

    changed = next_snapshot != local->current_snapshot || next_ratio != local->split_ratio;
    local->current_snapshot = next_snapshot;
    local->split_ratio = next_ratio;

    if (notify_ratio && changed)
    {
        grid_splitter_notify_ratio_changed(self, local);
    }
    if (changed || had_pressed)
    {
        egui_view_invalidate(self);
    }
}

static void grid_splitter_set_split_ratio_inner(egui_view_t *self, uint8_t split_ratio, uint8_t notify, uint8_t clear_pressed)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    uint8_t had_pressed = 0;
    uint8_t next_ratio = grid_splitter_clamp_ratio(split_ratio);
    uint8_t changed;

    if (clear_pressed)
    {
        had_pressed = grid_splitter_clear_pressed_state(self, local);
    }

    changed = next_ratio != local->split_ratio;
    local->split_ratio = next_ratio;

    if (notify && changed)
    {
        grid_splitter_notify_ratio_changed(self, local);
    }
    if (changed || had_pressed)
    {
        egui_view_invalidate(self);
    }
}

static void grid_splitter_update_ratio_from_x(egui_view_t *self, egui_dim_t x)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    egui_view_grid_splitter_metrics_t metrics;
    egui_dim_t handle_center_x;
    egui_dim_t left_w;
    egui_dim_t available_w;
    uint8_t next_ratio;

    grid_splitter_get_metrics(local, self, &metrics);
    available_w = metrics.left_region.size.width + metrics.right_region.size.width;
    if (available_w <= 0)
    {
        return;
    }

    handle_center_x = x - metrics.handle_region.size.width / 2;
    if (handle_center_x < metrics.shell_region.location.x + 2)
    {
        handle_center_x = metrics.shell_region.location.x + 2;
    }
    if (handle_center_x > metrics.shell_region.location.x + metrics.shell_region.size.width - metrics.handle_region.size.width - 2)
    {
        handle_center_x = metrics.shell_region.location.x + metrics.shell_region.size.width - metrics.handle_region.size.width - 2;
    }
    left_w = handle_center_x - (metrics.shell_region.location.x + 2);
    next_ratio = (uint8_t)(((int32_t)left_w * 100 + available_w / 2) / available_w);
    grid_splitter_set_split_ratio_inner(self, next_ratio, 1, 0);
}

void egui_view_grid_splitter_set_snapshots(egui_view_t *self, const egui_view_grid_splitter_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    uint8_t had_pressed = grid_splitter_clear_pressed_state(self, local);
    uint8_t next_count = snapshots == NULL || snapshot_count == 0 ? 0 : grid_splitter_clamp_snapshot_count(snapshot_count);
    uint8_t next_snapshot = 0;
    uint8_t next_ratio = EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_DEFAULT;
    uint8_t changed;

    if (next_count > 0)
    {
        next_snapshot = local->current_snapshot < next_count ? local->current_snapshot : 0;
        next_ratio = grid_splitter_resolve_default_ratio(&snapshots[next_snapshot]);
    }

    changed = local->snapshots != snapshots || local->snapshot_count != next_count || local->current_snapshot != next_snapshot ||
              local->split_ratio != next_ratio;
    local->snapshots = snapshots;
    local->snapshot_count = next_count;
    local->current_snapshot = next_snapshot;
    local->split_ratio = next_ratio;

    if (changed || had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_grid_splitter_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    grid_splitter_set_current_snapshot_inner(self, snapshot_index, 1);
}

uint8_t egui_view_grid_splitter_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    return local->current_snapshot;
}

void egui_view_grid_splitter_set_split_ratio(egui_view_t *self, uint8_t split_ratio)
{
    grid_splitter_set_split_ratio_inner(self, split_ratio, 1, 1);
}

uint8_t egui_view_grid_splitter_get_split_ratio(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    return local->split_ratio;
}

void egui_view_grid_splitter_set_on_ratio_changed_listener(egui_view_t *self,
                                                           egui_view_on_grid_splitter_ratio_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    local->on_ratio_changed = listener;
}

void egui_view_grid_splitter_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    uint8_t had_pressed = grid_splitter_clear_pressed_state(self, local);
    const egui_font_t *next_font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;

    if (local->font != next_font || had_pressed)
    {
        local->font = next_font;
        egui_view_invalidate(self);
    }
}

void egui_view_grid_splitter_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    uint8_t had_pressed = grid_splitter_clear_pressed_state(self, local);
    const egui_font_t *next_font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;

    if (local->meta_font != next_font || had_pressed)
    {
        local->meta_font = next_font;
        egui_view_invalidate(self);
    }
}

void egui_view_grid_splitter_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    uint8_t had_pressed = grid_splitter_clear_pressed_state(self, local);
    uint8_t next_mode = compact_mode ? 1 : 0;

    if (local->compact_mode != next_mode || had_pressed)
    {
        local->compact_mode = next_mode;
        egui_view_invalidate(self);
    }
}

void egui_view_grid_splitter_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    uint8_t had_pressed = grid_splitter_clear_pressed_state(self, local);
    uint8_t next_mode = read_only_mode ? 1 : 0;

    if (local->read_only_mode != next_mode || had_pressed)
    {
        local->read_only_mode = next_mode;
        egui_view_invalidate(self);
    }
}

void egui_view_grid_splitter_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                         egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    uint8_t had_pressed = grid_splitter_clear_pressed_state(self, local);

    local->surface_color = surface_color;
    local->section_color = section_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

uint8_t egui_view_grid_splitter_get_handle_region(egui_view_t *self, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    egui_view_grid_splitter_metrics_t metrics;

    if (region == NULL || grid_splitter_get_snapshot(local) == NULL)
    {
        return 0;
    }

    grid_splitter_get_metrics(local, self, &metrics);
    if (metrics.handle_region.size.width <= 0 || metrics.handle_region.size.height <= 0)
    {
        return 0;
    }
    *region = metrics.handle_region;
    return 1;
}

static void egui_view_grid_splitter_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    const egui_view_grid_splitter_snapshot_t *snapshot = grid_splitter_get_snapshot(local);
    egui_view_grid_splitter_metrics_t metrics;
    egui_color_t card_fill = local->surface_color;
    egui_color_t card_border = local->border_color;
    egui_color_t badge_fill = egui_rgb_mix(local->section_color, local->accent_color, 26);
    egui_color_t badge_text = egui_rgb_mix(local->text_color, local->accent_color, 28);
    egui_color_t title_color = local->text_color;
    egui_color_t summary_color = local->muted_text_color;
    egui_color_t shell_fill = egui_rgb_mix(local->section_color, local->accent_color, 8);
    egui_color_t shell_border = egui_rgb_mix(local->border_color, local->accent_color, 10);
    egui_color_t handle_fill = egui_rgb_mix(local->section_color, local->accent_color, self->is_pressed ? 34 : (self->is_focused ? 24 : 16));
    egui_color_t handle_line = egui_rgb_mix(local->border_color, local->accent_color, self->is_pressed ? 46 : 22);
    egui_color_t footer_fill = egui_rgb_mix(local->surface_color, local->accent_color, 18);
    egui_color_t footer_border = egui_rgb_mix(local->border_color, local->accent_color, 20);
    egui_color_t footer_text = egui_rgb_mix(local->muted_text_color, local->accent_color, 18);
    egui_color_t focus_color = local->accent_color;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_RADIUS : EGUI_VIEW_GRID_SPLITTER_STANDARD_RADIUS;
    egui_dim_t shell_radius = local->compact_mode ? EGUI_VIEW_GRID_SPLITTER_COMPACT_SHELL_RADIUS : EGUI_VIEW_GRID_SPLITTER_STANDARD_SHELL_RADIUS;
    uint8_t emphasized_left = snapshot != NULL && snapshot->emphasis == EGUI_VIEW_GRID_SPLITTER_EMPHASIS_LEFT ? 1 : 0;
    uint8_t emphasized_right = snapshot != NULL && snapshot->emphasis == EGUI_VIEW_GRID_SPLITTER_EMPHASIS_RIGHT ? 1 : 0;
    uint8_t i;

    grid_splitter_get_metrics(local, self, &metrics);
    if (metrics.content_region.size.width <= 0 || metrics.content_region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, 26);
        badge_text = egui_rgb_mix(badge_text, local->muted_text_color, 18);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 12);
        summary_color = egui_rgb_mix(summary_color, local->muted_text_color, 18);
        shell_fill = egui_rgb_mix(shell_fill, local->surface_color, 24);
        shell_border = egui_rgb_mix(shell_border, local->muted_text_color, 16);
        handle_fill = egui_rgb_mix(handle_fill, local->surface_color, 18);
        handle_line = egui_rgb_mix(handle_line, local->muted_text_color, 20);
        footer_fill = egui_rgb_mix(footer_fill, local->surface_color, 18);
        footer_border = egui_rgb_mix(footer_border, local->muted_text_color, 16);
        footer_text = egui_rgb_mix(footer_text, local->muted_text_color, 18);
        focus_color = egui_rgb_mix(focus_color, local->muted_text_color, 16);
    }
    if (!egui_view_get_enable(self))
    {
        card_fill = grid_splitter_mix_disabled(card_fill);
        card_border = grid_splitter_mix_disabled(card_border);
        badge_fill = grid_splitter_mix_disabled(badge_fill);
        badge_text = grid_splitter_mix_disabled(badge_text);
        title_color = grid_splitter_mix_disabled(title_color);
        summary_color = grid_splitter_mix_disabled(summary_color);
        shell_fill = grid_splitter_mix_disabled(shell_fill);
        shell_border = grid_splitter_mix_disabled(shell_border);
        handle_fill = grid_splitter_mix_disabled(handle_fill);
        handle_line = grid_splitter_mix_disabled(handle_line);
        footer_fill = grid_splitter_mix_disabled(footer_fill);
        footer_border = grid_splitter_mix_disabled(footer_border);
        footer_text = grid_splitter_mix_disabled(footer_text);
        focus_color = grid_splitter_mix_disabled(focus_color);
    }

    egui_canvas_draw_round_rectangle_fill(metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                          metrics.content_region.size.height, radius, card_fill, egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                     metrics.content_region.size.height, radius, 1, card_border, egui_color_alpha_mix(self->alpha, 54));

    if (self->is_focused && egui_view_get_enable(self) && !local->read_only_mode)
    {
        egui_canvas_draw_round_rectangle(metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                         metrics.content_region.size.height, radius, 2, focus_color, egui_color_alpha_mix(self->alpha, 92));
    }

    if (snapshot != NULL && metrics.badge_region.size.width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                              metrics.badge_region.size.height, metrics.badge_region.size.height / 2, badge_fill,
                                              egui_color_alpha_mix(self->alpha, 98));
        grid_splitter_draw_text(local->meta_font, self, snapshot->header, &metrics.badge_region, EGUI_ALIGN_CENTER, badge_text);
    }

    if (snapshot != NULL)
    {
        grid_splitter_draw_text(local->font, self, snapshot->title, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
        grid_splitter_draw_text(local->meta_font, self, snapshot->summary, &metrics.summary_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, summary_color);
    }

    if (metrics.shell_region.size.width > 0 && metrics.shell_region.size.height > 0)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.shell_region.location.x, metrics.shell_region.location.y, metrics.shell_region.size.width,
                                              metrics.shell_region.size.height, shell_radius, shell_fill, egui_color_alpha_mix(self->alpha, 98));
        egui_canvas_draw_round_rectangle(metrics.shell_region.location.x, metrics.shell_region.location.y, metrics.shell_region.size.width,
                                         metrics.shell_region.size.height, shell_radius, 1, shell_border, egui_color_alpha_mix(self->alpha, 40));
    }

    if (snapshot != NULL)
    {
        grid_splitter_draw_pane(self, local, &metrics.left_region, snapshot->left_title, snapshot->left_meta, snapshot->left_body, emphasized_left);
        grid_splitter_draw_pane(self, local, &metrics.right_region, snapshot->right_title, snapshot->right_meta, snapshot->right_body, emphasized_right);
    }

    if (metrics.handle_region.size.width > 0 && metrics.handle_region.size.height > 0)
    {
        egui_dim_t handle_x = metrics.handle_region.location.x + metrics.handle_region.size.width / 2 - (local->compact_mode ? 2 : 3);
        egui_dim_t handle_y = metrics.handle_region.location.y + 6;
        egui_dim_t handle_h = metrics.handle_region.size.height - 12;

        egui_canvas_draw_round_rectangle_fill(metrics.handle_region.location.x, metrics.handle_region.location.y, metrics.handle_region.size.width,
                                              metrics.handle_region.size.height, metrics.handle_region.size.width / 2, handle_fill,
                                              egui_color_alpha_mix(self->alpha, 92));
        egui_canvas_draw_round_rectangle(metrics.handle_region.location.x, metrics.handle_region.location.y, metrics.handle_region.size.width,
                                         metrics.handle_region.size.height, metrics.handle_region.size.width / 2, 1, handle_line,
                                         egui_color_alpha_mix(self->alpha, 54));

        if (self->is_focused && egui_view_get_enable(self) && !local->read_only_mode)
        {
            egui_canvas_draw_round_rectangle(metrics.handle_region.location.x - 1, metrics.handle_region.location.y - 1, metrics.handle_region.size.width + 2,
                                             metrics.handle_region.size.height + 2, metrics.handle_region.size.width / 2, 1, focus_color,
                                             egui_color_alpha_mix(self->alpha, 70));
        }

        if (handle_h > 0)
        {
            egui_canvas_draw_round_rectangle_fill(handle_x, handle_y, local->compact_mode ? 4 : 6, handle_h, 2, handle_line,
                                                  egui_color_alpha_mix(self->alpha, 84));
            for (i = 0; i < 3; i++)
            {
                egui_dim_t dot_y = metrics.handle_region.location.y + metrics.handle_region.size.height / 2 - (local->compact_mode ? 4 : 6) +
                                   i * (local->compact_mode ? 4 : 5);
                egui_canvas_draw_round_rectangle_fill(metrics.handle_region.location.x + metrics.handle_region.size.width / 2 - 1, dot_y, 2, 2, 1,
                                                      local->surface_color, egui_color_alpha_mix(self->alpha, 98));
            }
        }
    }

    if (snapshot != NULL && metrics.footer_region.size.width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                              metrics.footer_region.size.height, metrics.footer_region.size.height / 2, footer_fill,
                                              egui_color_alpha_mix(self->alpha, 96));
        egui_canvas_draw_round_rectangle(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                         metrics.footer_region.size.height, metrics.footer_region.size.height / 2, 1, footer_border,
                                         egui_color_alpha_mix(self->alpha, 36));
        grid_splitter_draw_text(local->meta_font, self, snapshot->footer, &metrics.footer_region, EGUI_ALIGN_CENTER, footer_text);
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_grid_splitter_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    egui_view_grid_splitter_metrics_t metrics;

    if (grid_splitter_get_snapshot(local) == NULL || !egui_view_get_enable(self) || local->read_only_mode)
    {
        if (grid_splitter_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        grid_splitter_get_metrics(local, self, &metrics);
        if (!grid_splitter_region_contains_point(&metrics.handle_region, event->location.x, event->location.y))
        {
            if (grid_splitter_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        local->pressed_handle = 1;
        local->handle_dragging = 1;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (!local->handle_dragging)
        {
            return 0;
        }
        grid_splitter_update_ratio_from_x(self, event->location.x);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
        if (!local->handle_dragging)
        {
            return 0;
        }
        grid_splitter_update_ratio_from_x(self, event->location.x);
        if (grid_splitter_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (grid_splitter_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_grid_splitter_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    const egui_view_grid_splitter_snapshot_t *snapshot = grid_splitter_get_snapshot(local);
    uint8_t next_snapshot;

    if (snapshot == NULL || !egui_view_get_enable(self) || local->read_only_mode)
    {
        if (grid_splitter_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->pressed_handle = 1;
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            grid_splitter_set_split_ratio_inner(self, grid_splitter_resolve_default_ratio(snapshot), 1, 1);
            return 1;
        }
        return 0;
    }

    if (grid_splitter_clear_pressed_state(self, local))
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
        if (local->split_ratio > EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_STEP)
        {
            grid_splitter_set_split_ratio_inner(self, (uint8_t)(local->split_ratio - EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_STEP), 1, 0);
        }
        else
        {
            grid_splitter_set_split_ratio_inner(self, EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_MIN, 1, 0);
        }
        return 1;
    case EGUI_KEY_CODE_RIGHT:
        grid_splitter_set_split_ratio_inner(self, (uint8_t)(local->split_ratio + EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_STEP), 1, 0);
        return 1;
    case EGUI_KEY_CODE_HOME:
        grid_splitter_set_split_ratio_inner(self, EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_MIN, 1, 0);
        return 1;
    case EGUI_KEY_CODE_END:
        grid_splitter_set_split_ratio_inner(self, EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_MAX, 1, 0);
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_snapshot = local->current_snapshot + 1;
        if (next_snapshot >= local->snapshot_count)
        {
            next_snapshot = 0;
        }
        grid_splitter_set_current_snapshot_inner(self, next_snapshot, 1);
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_grid_splitter_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    EGUI_UNUSED(event);

    if (grid_splitter_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_grid_splitter_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_grid_splitter_t);
    EGUI_UNUSED(event);

    if (grid_splitter_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_grid_splitter_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_grid_splitter_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_grid_splitter_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_grid_splitter_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_grid_splitter_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_grid_splitter_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_grid_splitter_on_key_event,
#endif
};

void egui_view_grid_splitter_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_grid_splitter_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_grid_splitter_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_ratio_changed = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->section_color = EGUI_COLOR_HEX(0xF5F8FA);
    local->border_color = EGUI_COLOR_HEX(0xD4DDE6);
    local->text_color = EGUI_COLOR_HEX(0x1B2834);
    local->muted_text_color = EGUI_COLOR_HEX(0x6D7C8A);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->split_ratio = EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_DEFAULT;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_handle = 0;
    local->handle_dragging = 0;

    egui_view_set_view_name(self, "egui_view_grid_splitter");
}
