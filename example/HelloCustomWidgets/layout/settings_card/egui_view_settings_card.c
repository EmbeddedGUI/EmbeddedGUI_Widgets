#include "egui_view_settings_card.h"

#define EGUI_VIEW_SETTINGS_CARD_STANDARD_RADIUS           10
#define EGUI_VIEW_SETTINGS_CARD_STANDARD_PAD_X            8
#define EGUI_VIEW_SETTINGS_CARD_STANDARD_PAD_Y            8
#define EGUI_VIEW_SETTINGS_CARD_STANDARD_BADGE_H          11
#define EGUI_VIEW_SETTINGS_CARD_STANDARD_ICON_SIZE        14
#define EGUI_VIEW_SETTINGS_CARD_STANDARD_TITLE_H          12
#define EGUI_VIEW_SETTINGS_CARD_STANDARD_DESCRIPTION_H    10
#define EGUI_VIEW_SETTINGS_CARD_STANDARD_FOOTER_H         11
#define EGUI_VIEW_SETTINGS_CARD_STANDARD_SWITCH_W         24
#define EGUI_VIEW_SETTINGS_CARD_STANDARD_SWITCH_H         12
#define EGUI_VIEW_SETTINGS_CARD_STANDARD_TRAILING_GAP     8
#define EGUI_VIEW_SETTINGS_CARD_STANDARD_CONTENT_TOP_GAP  5
#define EGUI_VIEW_SETTINGS_CARD_STANDARD_TEXT_LEFT_GAP    8

#define EGUI_VIEW_SETTINGS_CARD_COMPACT_RADIUS           8
#define EGUI_VIEW_SETTINGS_CARD_COMPACT_PAD_X            6
#define EGUI_VIEW_SETTINGS_CARD_COMPACT_PAD_Y            6
#define EGUI_VIEW_SETTINGS_CARD_COMPACT_BADGE_H          9
#define EGUI_VIEW_SETTINGS_CARD_COMPACT_ICON_SIZE        11
#define EGUI_VIEW_SETTINGS_CARD_COMPACT_TITLE_H          10
#define EGUI_VIEW_SETTINGS_CARD_COMPACT_DESCRIPTION_H    0
#define EGUI_VIEW_SETTINGS_CARD_COMPACT_FOOTER_H         0
#define EGUI_VIEW_SETTINGS_CARD_COMPACT_SWITCH_W         20
#define EGUI_VIEW_SETTINGS_CARD_COMPACT_SWITCH_H         10
#define EGUI_VIEW_SETTINGS_CARD_COMPACT_TRAILING_GAP     6
#define EGUI_VIEW_SETTINGS_CARD_COMPACT_CONTENT_TOP_GAP  4
#define EGUI_VIEW_SETTINGS_CARD_COMPACT_TEXT_LEFT_GAP    6

typedef struct egui_view_settings_card_metrics egui_view_settings_card_metrics_t;
struct egui_view_settings_card_metrics
{
    egui_region_t region;
    egui_region_t content_region;
    egui_region_t card_region;
    egui_region_t badge_region;
    egui_region_t icon_region;
    egui_region_t title_region;
    egui_region_t description_region;
    egui_region_t trailing_region;
    egui_region_t footer_region;
};

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_settings_card_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_settings_card_on_static_key_event(egui_view_t *self, egui_key_event_t *event);
#endif

static uint8_t egui_view_settings_card_clamp_snapshot_count(uint8_t count)
{
    return count > EGUI_VIEW_SETTINGS_CARD_MAX_SNAPSHOTS ? EGUI_VIEW_SETTINGS_CARD_MAX_SNAPSHOTS : count;
}

static uint8_t egui_view_settings_card_text_len(const char *text)
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

static uint8_t egui_view_settings_card_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static egui_color_t egui_view_settings_card_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static const egui_view_settings_card_snapshot_t *egui_view_settings_card_get_snapshot(egui_view_settings_card_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }

    return &local->snapshots[local->current_snapshot];
}

static egui_color_t egui_view_settings_card_tone_color(egui_view_settings_card_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_SETTINGS_CARD_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_SETTINGS_CARD_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_SETTINGS_CARD_TONE_NEUTRAL:
        return local->neutral_color;
    case EGUI_VIEW_SETTINGS_CARD_TONE_ACCENT:
    default:
        return local->accent_color;
    }
}

static uint8_t egui_view_settings_card_clear_pressed_state(egui_view_t *self, egui_view_settings_card_t *local)
{
    uint8_t had_pressed = self->is_pressed || local->pressed_part != EGUI_VIEW_SETTINGS_CARD_PART_NONE;

    local->pressed_part = EGUI_VIEW_SETTINGS_CARD_PART_NONE;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static uint8_t egui_view_settings_card_part_exists(const egui_view_settings_card_snapshot_t *snapshot, uint8_t part)
{
    return snapshot != NULL && part == EGUI_VIEW_SETTINGS_CARD_PART_CARD ? 1 : 0;
}

static uint8_t egui_view_settings_card_part_is_interactive(egui_view_settings_card_t *local, egui_view_t *self,
                                                           const egui_view_settings_card_snapshot_t *snapshot, uint8_t part)
{
    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        return 0;
    }

    return egui_view_settings_card_part_exists(snapshot, part);
}

static uint8_t egui_view_settings_card_resolve_default_part(const egui_view_settings_card_snapshot_t *snapshot)
{
    return snapshot == NULL ? EGUI_VIEW_SETTINGS_CARD_PART_NONE : EGUI_VIEW_SETTINGS_CARD_PART_CARD;
}

static void egui_view_settings_card_sync_current_part(egui_view_settings_card_t *local)
{
    const egui_view_settings_card_snapshot_t *snapshot = egui_view_settings_card_get_snapshot(local);

    if (!egui_view_settings_card_part_exists(snapshot, local->current_part))
    {
        local->current_part = egui_view_settings_card_resolve_default_part(snapshot);
    }
}

static egui_dim_t egui_view_settings_card_pill_width(const char *text, uint8_t compact_mode, egui_dim_t min_width, egui_dim_t max_width)
{
    egui_dim_t width = min_width;

    if (egui_view_settings_card_has_text(text))
    {
        width += egui_view_settings_card_text_len(text) * (compact_mode ? 4 : 5);
    }

    if (width > max_width)
    {
        width = max_width;
    }

    return width;
}

static void egui_view_settings_card_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region,
                                              uint8_t align, egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (!egui_view_settings_card_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static void egui_view_settings_card_draw_switch(egui_view_t *self, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height,
                                                uint8_t checked, egui_color_t tone_color, egui_color_t muted_color)
{
    egui_color_t track_color = checked ? egui_rgb_mix(EGUI_COLOR_WHITE, tone_color, 56) : egui_rgb_mix(EGUI_COLOR_WHITE, muted_color, 26);
    egui_color_t border_color = checked ? egui_rgb_mix(muted_color, tone_color, 36) : egui_rgb_mix(muted_color, EGUI_COLOR_WHITE, 10);
    egui_color_t thumb_color = checked ? EGUI_COLOR_WHITE : egui_rgb_mix(EGUI_COLOR_WHITE, muted_color, 18);
    egui_dim_t radius = height / 2;
    egui_dim_t thumb_r = radius - 2;
    egui_dim_t thumb_x = checked ? (x + width - radius) : (x + radius);
    egui_dim_t thumb_y = y + radius;

    if (thumb_r < 1)
    {
        thumb_r = 1;
    }

    egui_canvas_draw_round_rectangle_fill(x, y, width, height, radius, track_color, egui_color_alpha_mix(self->alpha, 88));
    egui_canvas_draw_round_rectangle(x, y, width, height, radius, 1, border_color, egui_color_alpha_mix(self->alpha, 36));
    egui_canvas_draw_circle_fill(thumb_x, thumb_y, thumb_r, thumb_color, egui_color_alpha_mix(self->alpha, 100));
}

static void egui_view_settings_card_draw_chevron(egui_view_t *self, const egui_region_t *region, egui_color_t color)
{
    egui_dim_t x;
    egui_dim_t y;
    egui_dim_t width;
    egui_dim_t height;

    if (region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    x = region->location.x + 1;
    y = region->location.y + 1;
    width = region->size.width - 2;
    height = region->size.height - 2;
    if (width < 4 || height < 4)
    {
        return;
    }

    egui_canvas_draw_triangle_fill(x, y, x, y + height, x + width, y + height / 2, color, egui_color_alpha_mix(self->alpha, 92));
}

static void egui_view_settings_card_get_metrics(egui_view_settings_card_t *local, egui_view_t *self,
                                                const egui_view_settings_card_snapshot_t *snapshot, egui_view_settings_card_metrics_t *metrics)
{
    egui_dim_t pad_x = local->compact_mode ? EGUI_VIEW_SETTINGS_CARD_COMPACT_PAD_X : EGUI_VIEW_SETTINGS_CARD_STANDARD_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? EGUI_VIEW_SETTINGS_CARD_COMPACT_PAD_Y : EGUI_VIEW_SETTINGS_CARD_STANDARD_PAD_Y;
    egui_dim_t badge_h = local->compact_mode ? EGUI_VIEW_SETTINGS_CARD_COMPACT_BADGE_H : EGUI_VIEW_SETTINGS_CARD_STANDARD_BADGE_H;
    egui_dim_t icon_size = local->compact_mode ? EGUI_VIEW_SETTINGS_CARD_COMPACT_ICON_SIZE : EGUI_VIEW_SETTINGS_CARD_STANDARD_ICON_SIZE;
    egui_dim_t title_h = local->compact_mode ? EGUI_VIEW_SETTINGS_CARD_COMPACT_TITLE_H : EGUI_VIEW_SETTINGS_CARD_STANDARD_TITLE_H;
    egui_dim_t description_h = local->compact_mode ? EGUI_VIEW_SETTINGS_CARD_COMPACT_DESCRIPTION_H : EGUI_VIEW_SETTINGS_CARD_STANDARD_DESCRIPTION_H;
    egui_dim_t footer_h = local->compact_mode ? EGUI_VIEW_SETTINGS_CARD_COMPACT_FOOTER_H : EGUI_VIEW_SETTINGS_CARD_STANDARD_FOOTER_H;
    egui_dim_t content_top_gap = local->compact_mode ? EGUI_VIEW_SETTINGS_CARD_COMPACT_CONTENT_TOP_GAP
                                                     : EGUI_VIEW_SETTINGS_CARD_STANDARD_CONTENT_TOP_GAP;
    egui_dim_t text_left_gap = local->compact_mode ? EGUI_VIEW_SETTINGS_CARD_COMPACT_TEXT_LEFT_GAP
                                                   : EGUI_VIEW_SETTINGS_CARD_STANDARD_TEXT_LEFT_GAP;
    egui_dim_t trailing_gap = local->compact_mode ? EGUI_VIEW_SETTINGS_CARD_COMPACT_TRAILING_GAP
                                                  : EGUI_VIEW_SETTINGS_CARD_STANDARD_TRAILING_GAP;
    egui_dim_t trailing_w = 0;
    egui_dim_t trailing_h = title_h;
    egui_dim_t inner_x;
    egui_dim_t inner_y;
    egui_dim_t inner_w;
    egui_dim_t footer_y;

    egui_view_get_work_region(self, &metrics->region);
    metrics->content_region = metrics->region;
    metrics->card_region = metrics->region;
    metrics->badge_region.size.width = 0;
    metrics->badge_region.size.height = 0;
    metrics->icon_region.size.width = 0;
    metrics->icon_region.size.height = 0;
    metrics->title_region.size.width = 0;
    metrics->title_region.size.height = 0;
    metrics->description_region.size.width = 0;
    metrics->description_region.size.height = 0;
    metrics->trailing_region.size.width = 0;
    metrics->trailing_region.size.height = 0;
    metrics->footer_region.size.width = 0;
    metrics->footer_region.size.height = 0;

    if (metrics->region.size.width <= 0 || metrics->region.size.height <= 0)
    {
        return;
    }

    metrics->content_region.location.x = metrics->region.location.x + pad_x;
    metrics->content_region.location.y = metrics->region.location.y + pad_y;
    metrics->content_region.size.width = metrics->region.size.width - pad_x * 2;
    metrics->content_region.size.height = metrics->region.size.height - pad_y * 2;
    metrics->card_region = metrics->content_region;

    if (metrics->content_region.size.width <= 0 || metrics->content_region.size.height <= 0 || snapshot == NULL)
    {
        return;
    }

    inner_x = metrics->card_region.location.x + pad_x;
    inner_y = metrics->card_region.location.y + pad_y;
    inner_w = metrics->card_region.size.width - pad_x * 2;
    if (inner_w <= 0)
    {
        return;
    }

    if (egui_view_settings_card_has_text(snapshot->eyebrow))
    {
        metrics->badge_region.location.x = inner_x;
        metrics->badge_region.location.y = inner_y;
        metrics->badge_region.size.width =
                egui_view_settings_card_pill_width(snapshot->eyebrow, local->compact_mode, local->compact_mode ? 24 : 30, inner_w / 2);
        metrics->badge_region.size.height = badge_h;
    }

    if (footer_h > 0 && egui_view_settings_card_has_text(snapshot->footer))
    {
        footer_y = metrics->card_region.location.y + metrics->card_region.size.height - pad_y - footer_h;
        if (footer_y > inner_y)
        {
            metrics->footer_region.location.x = inner_x;
            metrics->footer_region.location.y = footer_y;
            metrics->footer_region.size.width = inner_w;
            metrics->footer_region.size.height = footer_h;
        }
    }

    inner_y += metrics->badge_region.size.height;
    if (metrics->badge_region.size.height > 0)
    {
        inner_y += content_top_gap;
    }

    if (egui_view_settings_card_has_text(snapshot->icon_text))
    {
        metrics->icon_region.location.x = inner_x;
        metrics->icon_region.location.y = inner_y + (local->compact_mode ? 0 : 1);
        metrics->icon_region.size.width = icon_size;
        metrics->icon_region.size.height = icon_size;
    }

    switch (snapshot->trailing_kind)
    {
    case EGUI_VIEW_SETTINGS_CARD_TRAILING_SWITCH_ON:
    case EGUI_VIEW_SETTINGS_CARD_TRAILING_SWITCH_OFF:
        trailing_w = local->compact_mode ? EGUI_VIEW_SETTINGS_CARD_COMPACT_SWITCH_W : EGUI_VIEW_SETTINGS_CARD_STANDARD_SWITCH_W;
        trailing_h = local->compact_mode ? EGUI_VIEW_SETTINGS_CARD_COMPACT_SWITCH_H : EGUI_VIEW_SETTINGS_CARD_STANDARD_SWITCH_H;
        break;
    case EGUI_VIEW_SETTINGS_CARD_TRAILING_CHEVRON:
        trailing_w = local->compact_mode ? 7 : 9;
        trailing_h = local->compact_mode ? 9 : 11;
        break;
    case EGUI_VIEW_SETTINGS_CARD_TRAILING_VALUE:
    default:
        if (egui_view_settings_card_has_text(snapshot->value))
        {
            trailing_w = egui_view_settings_card_pill_width(snapshot->value, local->compact_mode, local->compact_mode ? 18 : 24, inner_w / 3);
            trailing_h = local->compact_mode ? 10 : 12;
        }
        break;
    }

    if (trailing_w > 0)
    {
        metrics->trailing_region.location.x = inner_x + inner_w - trailing_w;
        metrics->trailing_region.location.y = inner_y + (icon_size - trailing_h) / 2;
        metrics->trailing_region.size.width = trailing_w;
        metrics->trailing_region.size.height = trailing_h;
    }

    metrics->title_region.location.x = metrics->icon_region.size.width > 0 ? (metrics->icon_region.location.x + metrics->icon_region.size.width + text_left_gap) : inner_x;
    metrics->title_region.location.y = inner_y;
    metrics->title_region.size.width = inner_x + inner_w - metrics->title_region.location.x;
    if (metrics->trailing_region.size.width > 0)
    {
        metrics->title_region.size.width = metrics->trailing_region.location.x - metrics->title_region.location.x - trailing_gap;
    }
    metrics->title_region.size.height = title_h;
    if (metrics->title_region.size.width < 0)
    {
        metrics->title_region.size.width = 0;
    }

    if (description_h > 0 && egui_view_settings_card_has_text(snapshot->description))
    {
        metrics->description_region.location.x = metrics->title_region.location.x;
        metrics->description_region.location.y = metrics->title_region.location.y + metrics->title_region.size.height + 2;
        metrics->description_region.size.width = metrics->title_region.size.width;
        metrics->description_region.size.height = description_h;
        if (metrics->footer_region.size.height > 0 &&
            metrics->description_region.location.y + metrics->description_region.size.height > metrics->footer_region.location.y - 2)
        {
            metrics->description_region.size.width = 0;
            metrics->description_region.size.height = 0;
        }
    }
}

static uint8_t egui_view_settings_card_resolve_hit(egui_view_settings_card_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_settings_card_metrics_t metrics;
    const egui_view_settings_card_snapshot_t *snapshot = egui_view_settings_card_get_snapshot(local);

    egui_view_settings_card_get_metrics(local, self, snapshot, &metrics);
    if (egui_region_pt_in_rect(&metrics.card_region, x, y))
    {
        return EGUI_VIEW_SETTINGS_CARD_PART_CARD;
    }

    return EGUI_VIEW_SETTINGS_CARD_PART_NONE;
}

static void egui_view_settings_card_set_current_part_inner(egui_view_t *self, uint8_t part, uint8_t invalidate_on_clear)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);
    const egui_view_settings_card_snapshot_t *snapshot = egui_view_settings_card_get_snapshot(local);
    uint8_t had_pressed = egui_view_settings_card_clear_pressed_state(self, local);

    if (!egui_view_settings_card_part_exists(snapshot, part))
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    if (local->current_part == part)
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_part = part;
    egui_view_invalidate(self);
}

static void egui_view_settings_card_draw_trailing(egui_view_t *self, egui_view_settings_card_t *local,
                                                  const egui_view_settings_card_snapshot_t *snapshot, const egui_region_t *region,
                                                  egui_color_t tone_color, egui_color_t muted_color, egui_color_t border_color,
                                                  egui_color_t text_color)
{
    egui_region_t text_region;

    if (region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    switch (snapshot->trailing_kind)
    {
    case EGUI_VIEW_SETTINGS_CARD_TRAILING_SWITCH_ON:
        egui_view_settings_card_draw_switch(self, region->location.x, region->location.y, region->size.width, region->size.height, 1, tone_color, muted_color);
        break;
    case EGUI_VIEW_SETTINGS_CARD_TRAILING_SWITCH_OFF:
        egui_view_settings_card_draw_switch(self, region->location.x, region->location.y, region->size.width, region->size.height, 0, tone_color, muted_color);
        break;
    case EGUI_VIEW_SETTINGS_CARD_TRAILING_CHEVRON:
        egui_view_settings_card_draw_chevron(self, region, text_color);
        break;
    case EGUI_VIEW_SETTINGS_CARD_TRAILING_VALUE:
    default:
        if (!egui_view_settings_card_has_text(snapshot->value))
        {
            break;
        }

        egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, region->size.height / 2,
                                              egui_rgb_mix(local->surface_color, tone_color, 10), egui_color_alpha_mix(self->alpha, 84));
        egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, region->size.height / 2, 1,
                                         egui_rgb_mix(border_color, tone_color, 12), egui_color_alpha_mix(self->alpha, 32));
        text_region = *region;
        egui_view_settings_card_draw_text(local->meta_font, self, snapshot->value, &text_region, EGUI_ALIGN_CENTER, text_color);
        break;
    }
}

static void egui_view_settings_card_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);
    const egui_view_settings_card_snapshot_t *snapshot = egui_view_settings_card_get_snapshot(local);
    egui_view_settings_card_metrics_t metrics;
    egui_region_t text_region;
    egui_color_t tone_color;
    egui_color_t card_fill;
    egui_color_t card_border;
    egui_color_t accent_line;
    egui_color_t title_color;
    egui_color_t description_color;
    egui_color_t footer_fill;
    egui_color_t footer_border;
    egui_color_t footer_color;
    egui_color_t badge_fill;
    egui_color_t badge_border;
    egui_color_t icon_fill;
    egui_color_t icon_text;
    egui_color_t trailing_text;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_SETTINGS_CARD_COMPACT_RADIUS : EGUI_VIEW_SETTINGS_CARD_STANDARD_RADIUS;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    uint8_t is_focused = self->is_focused ? 1 : 0;
#else
    uint8_t is_focused = 0;
#endif
    uint8_t pressed = local->pressed_part == EGUI_VIEW_SETTINGS_CARD_PART_CARD && self->is_pressed ? 1 : 0;

    if (snapshot == NULL)
    {
        return;
    }

    egui_view_settings_card_get_metrics(local, self, snapshot, &metrics);
    if (metrics.card_region.size.width <= 0 || metrics.card_region.size.height <= 0)
    {
        return;
    }

    tone_color = egui_view_settings_card_tone_color(local, snapshot->tone);
    card_fill = egui_rgb_mix(local->surface_color, tone_color, snapshot->emphasized ? (local->compact_mode ? 6 : 8) : (local->compact_mode ? 3 : 4));
    card_border = egui_rgb_mix(local->border_color, tone_color, pressed ? 26 : (is_focused ? 22 : 14));
    accent_line = egui_rgb_mix(local->surface_color, tone_color, snapshot->emphasized ? 24 : 16);
    title_color = pressed ? egui_rgb_mix(local->text_color, tone_color, 12) : local->text_color;
    description_color = egui_rgb_mix(local->muted_text_color, tone_color, pressed ? 18 : 10);
    footer_fill = egui_rgb_mix(local->section_color, tone_color, 10);
    footer_border = egui_rgb_mix(local->border_color, tone_color, 16);
    footer_color = egui_rgb_mix(local->muted_text_color, tone_color, 18);
    badge_fill = egui_rgb_mix(local->surface_color, tone_color, 10);
    badge_border = egui_rgb_mix(local->border_color, tone_color, 16);
    icon_fill = egui_rgb_mix(local->section_color, tone_color, pressed ? 16 : 10);
    icon_text = pressed ? egui_rgb_mix(local->text_color, tone_color, 20) : egui_rgb_mix(local->text_color, tone_color, 10);
    trailing_text = pressed ? egui_rgb_mix(local->text_color, tone_color, 16) : egui_rgb_mix(local->muted_text_color, tone_color, 16);

    if (local->read_only_mode)
    {
        tone_color = egui_rgb_mix(tone_color, local->muted_text_color, 82);
        card_fill = egui_rgb_mix(card_fill, local->surface_color, 24);
        card_border = egui_rgb_mix(card_border, local->muted_text_color, 18);
        accent_line = egui_rgb_mix(accent_line, local->surface_color, 36);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 24);
        description_color = egui_rgb_mix(description_color, local->muted_text_color, 28);
        footer_fill = egui_rgb_mix(footer_fill, local->surface_color, 24);
        footer_border = egui_rgb_mix(footer_border, local->muted_text_color, 18);
        footer_color = egui_rgb_mix(footer_color, local->muted_text_color, 36);
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, 24);
        badge_border = egui_rgb_mix(badge_border, local->muted_text_color, 18);
        icon_fill = egui_rgb_mix(icon_fill, local->surface_color, 26);
        icon_text = egui_rgb_mix(icon_text, local->muted_text_color, 28);
        trailing_text = egui_rgb_mix(trailing_text, local->muted_text_color, 30);
    }
    if (!egui_view_get_enable(self))
    {
        tone_color = egui_view_settings_card_mix_disabled(tone_color);
        card_fill = egui_view_settings_card_mix_disabled(card_fill);
        card_border = egui_view_settings_card_mix_disabled(card_border);
        accent_line = egui_view_settings_card_mix_disabled(accent_line);
        title_color = egui_view_settings_card_mix_disabled(title_color);
        description_color = egui_view_settings_card_mix_disabled(description_color);
        footer_fill = egui_view_settings_card_mix_disabled(footer_fill);
        footer_border = egui_view_settings_card_mix_disabled(footer_border);
        footer_color = egui_view_settings_card_mix_disabled(footer_color);
        badge_fill = egui_view_settings_card_mix_disabled(badge_fill);
        badge_border = egui_view_settings_card_mix_disabled(badge_border);
        icon_fill = egui_view_settings_card_mix_disabled(icon_fill);
        icon_text = egui_view_settings_card_mix_disabled(icon_text);
        trailing_text = egui_view_settings_card_mix_disabled(trailing_text);
    }

    egui_canvas_draw_round_rectangle_fill(metrics.card_region.location.x, metrics.card_region.location.y, metrics.card_region.size.width,
                                          metrics.card_region.size.height, radius, card_fill, egui_color_alpha_mix(self->alpha, 100));
    egui_canvas_draw_round_rectangle(metrics.card_region.location.x, metrics.card_region.location.y, metrics.card_region.size.width,
                                     metrics.card_region.size.height, radius, 1, card_border, egui_color_alpha_mix(self->alpha, 58));
    egui_canvas_draw_round_rectangle_fill(metrics.card_region.location.x + 2, metrics.card_region.location.y + 2, metrics.card_region.size.width - 4,
                                          local->compact_mode ? 2 : 3, radius - 2, accent_line,
                                          egui_color_alpha_mix(self->alpha, local->read_only_mode ? 10 : (snapshot->emphasized ? 20 : 14)));

    if (is_focused && egui_view_get_enable(self) && !local->read_only_mode)
    {
        egui_canvas_draw_round_rectangle(metrics.card_region.location.x, metrics.card_region.location.y, metrics.card_region.size.width,
                                         metrics.card_region.size.height, radius, 2, tone_color, egui_color_alpha_mix(self->alpha, 92));
    }

    if (metrics.badge_region.size.width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                              metrics.badge_region.size.height, metrics.badge_region.size.height / 2, badge_fill,
                                              egui_color_alpha_mix(self->alpha, 84));
        egui_canvas_draw_round_rectangle(metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                         metrics.badge_region.size.height, metrics.badge_region.size.height / 2, 1, badge_border,
                                         egui_color_alpha_mix(self->alpha, 34));
        text_region = metrics.badge_region;
        egui_view_settings_card_draw_text(local->meta_font, self, snapshot->eyebrow, &text_region, EGUI_ALIGN_CENTER, tone_color);
    }

    if (metrics.icon_region.size.width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.icon_region.location.x, metrics.icon_region.location.y, metrics.icon_region.size.width,
                                              metrics.icon_region.size.height, local->compact_mode ? 4 : 5, icon_fill,
                                              egui_color_alpha_mix(self->alpha, 90));
        text_region = metrics.icon_region;
        egui_view_settings_card_draw_text(local->meta_font, self, snapshot->icon_text, &text_region, EGUI_ALIGN_CENTER, icon_text);
    }

    if (metrics.title_region.size.width > 0)
    {
        text_region = metrics.title_region;
        egui_view_settings_card_draw_text(local->font, self, snapshot->title, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
    }
    if (metrics.description_region.size.width > 0)
    {
        text_region = metrics.description_region;
        egui_view_settings_card_draw_text(local->meta_font, self, snapshot->description, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                          description_color);
    }
    if (metrics.trailing_region.size.width > 0)
    {
        egui_view_settings_card_draw_trailing(self, local, snapshot, &metrics.trailing_region, tone_color, local->muted_text_color, local->border_color,
                                              trailing_text);
    }
    if (metrics.footer_region.size.width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                              metrics.footer_region.size.height, metrics.footer_region.size.height / 2, footer_fill,
                                              egui_color_alpha_mix(self->alpha, 82));
        egui_canvas_draw_round_rectangle(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                         metrics.footer_region.size.height, metrics.footer_region.size.height / 2, 1, footer_border,
                                         egui_color_alpha_mix(self->alpha, 28));
        text_region.location.x = metrics.footer_region.location.x + 5;
        text_region.location.y = metrics.footer_region.location.y;
        text_region.size.width = metrics.footer_region.size.width - 10;
        text_region.size.height = metrics.footer_region.size.height;
        egui_view_settings_card_draw_text(local->meta_font, self, snapshot->footer, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, footer_color);
    }
}

void egui_view_settings_card_set_snapshots(egui_view_t *self, const egui_view_settings_card_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);

    egui_view_settings_card_clear_pressed_state(self, local);
    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : egui_view_settings_card_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }
    local->current_part = egui_view_settings_card_resolve_default_part(egui_view_settings_card_get_snapshot(local));
    egui_view_invalidate(self);
}

void egui_view_settings_card_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);
    uint8_t had_pressed = egui_view_settings_card_clear_pressed_state(self, local);

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
    local->current_part = egui_view_settings_card_resolve_default_part(egui_view_settings_card_get_snapshot(local));
    egui_view_invalidate(self);
}

uint8_t egui_view_settings_card_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);
    return local->current_snapshot;
}

void egui_view_settings_card_set_current_part(egui_view_t *self, uint8_t part)
{
    egui_view_settings_card_set_current_part_inner(self, part, 1);
}

uint8_t egui_view_settings_card_get_current_part(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);
    egui_view_settings_card_sync_current_part(local);
    return local->current_part;
}

uint8_t egui_view_settings_card_activate_current_part(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);
    const egui_view_settings_card_snapshot_t *snapshot = egui_view_settings_card_get_snapshot(local);
    uint8_t action_part;

    egui_view_settings_card_sync_current_part(local);
    if (!egui_view_settings_card_part_is_interactive(local, self, snapshot, local->current_part))
    {
        return 0;
    }

    action_part = local->current_part;
    if (local->on_action != NULL)
    {
        local->on_action(self, local->current_snapshot, action_part);
    }
    egui_view_invalidate(self);
    return 1;
}

void egui_view_settings_card_set_on_action_listener(egui_view_t *self, egui_view_on_settings_card_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);
    local->on_action = listener;
}

void egui_view_settings_card_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);

    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_settings_card_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_settings_card_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);

    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_settings_card_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_settings_card_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);

    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_settings_card_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_settings_card_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);

    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_settings_card_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_settings_card_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                         egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                         egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);

    local->surface_color = surface_color;
    local->section_color = section_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    egui_view_settings_card_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

uint8_t egui_view_settings_card_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);
    const egui_view_settings_card_snapshot_t *snapshot = egui_view_settings_card_get_snapshot(local);
    egui_view_settings_card_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }

    egui_view_settings_card_get_metrics(local, self, snapshot, &metrics);
    if (part == EGUI_VIEW_SETTINGS_CARD_PART_CARD)
    {
        *region = metrics.card_region;
        return metrics.card_region.size.width > 0 && metrics.card_region.size.height > 0 ? 1 : 0;
    }

    return 0;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_settings_card_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);
    const egui_view_settings_card_snapshot_t *snapshot = egui_view_settings_card_get_snapshot(local);
    uint8_t hit_part;

    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (egui_view_settings_card_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_part = egui_view_settings_card_resolve_hit(local, self, event->location.x, event->location.y);
        if (!egui_view_settings_card_part_is_interactive(local, self, snapshot, hit_part))
        {
            if (egui_view_settings_card_clear_pressed_state(self, local))
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
        local->pressed_part = hit_part;
        local->current_part = hit_part;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_part == EGUI_VIEW_SETTINGS_CARD_PART_NONE)
        {
            return 0;
        }
        hit_part = egui_view_settings_card_resolve_hit(local, self, event->location.x, event->location.y);
        egui_view_set_pressed(self, hit_part == local->pressed_part);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled;

        hit_part = egui_view_settings_card_resolve_hit(local, self, event->location.x, event->location.y);
        if (hit_part == local->pressed_part && egui_view_settings_card_part_is_interactive(local, self, snapshot, hit_part))
        {
            local->current_part = hit_part;
            egui_view_settings_card_activate_current_part(self);
        }
        handled = egui_view_settings_card_clear_pressed_state(self, local);
        if (handled)
        {
            egui_view_invalidate(self);
        }
        return handled || hit_part != EGUI_VIEW_SETTINGS_CARD_PART_NONE;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (egui_view_settings_card_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_settings_card_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);
    EGUI_UNUSED(event);

    if (egui_view_settings_card_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_settings_card_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);
    const egui_view_settings_card_snapshot_t *snapshot = egui_view_settings_card_get_snapshot(local);

    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (egui_view_settings_card_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    egui_view_settings_card_sync_current_part(local);
    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            if (!egui_view_settings_card_part_is_interactive(local, self, snapshot, local->current_part))
            {
                return 0;
            }
            local->pressed_part = local->current_part;
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        }

        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            uint8_t handled = 0;

            if (local->pressed_part != EGUI_VIEW_SETTINGS_CARD_PART_NONE && local->pressed_part == local->current_part &&
                egui_view_settings_card_part_is_interactive(local, self, snapshot, local->pressed_part))
            {
                handled = egui_view_settings_card_activate_current_part(self);
            }
            if (egui_view_settings_card_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return handled;
        }

        return 0;
    }

    if (egui_view_settings_card_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }

    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_TAB:
    case EGUI_KEY_CODE_HOME:
    case EGUI_KEY_CODE_END:
        egui_view_settings_card_set_current_part_inner(self, EGUI_VIEW_SETTINGS_CARD_PART_CARD, 0);
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_settings_card_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_settings_card_t);
    EGUI_UNUSED(event);

    if (egui_view_settings_card_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_settings_card_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_settings_card_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_settings_card_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_settings_card_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_settings_card_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_settings_card_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_settings_card_on_key_event,
#endif
};

void egui_view_settings_card_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_settings_card_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_settings_card_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_action = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->section_color = EGUI_COLOR_HEX(0xF7F9FC);
    local->border_color = EGUI_COLOR_HEX(0xD2DBE3);
    local->text_color = EGUI_COLOR_HEX(0x1A2734);
    local->muted_text_color = EGUI_COLOR_HEX(0x6B7A89);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->success_color = EGUI_COLOR_HEX(0x0F7B45);
    local->warning_color = EGUI_COLOR_HEX(0x9D5D00);
    local->neutral_color = EGUI_COLOR_HEX(0x7A8796);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_part = EGUI_VIEW_SETTINGS_CARD_PART_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_part = EGUI_VIEW_SETTINGS_CARD_PART_NONE;

    egui_view_set_view_name(self, "egui_view_settings_card");
}
