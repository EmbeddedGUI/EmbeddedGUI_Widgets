#include "egui_view_card_panel.h"

static uint8_t egui_view_card_panel_clamp_snapshot_count(uint8_t count)
{
    if (count > EGUI_VIEW_CARD_PANEL_MAX_SNAPSHOTS)
    {
        return EGUI_VIEW_CARD_PANEL_MAX_SNAPSHOTS;
    }
    return count;
}

static uint8_t egui_view_card_panel_text_len(const char *text)
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

static egui_color_t egui_view_card_panel_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static void egui_view_card_panel_clear_pressed_state(egui_view_t *self)
{
    egui_view_set_pressed(self, false);
}

static egui_color_t egui_view_card_panel_tone_color(egui_view_card_panel_t *local, uint8_t tone)
{
    switch (tone)
    {
    case 1:
        return local->success_color;
    case 2:
        return local->warning_color;
    case 3:
        return local->neutral_color;
    default:
        return local->accent_color;
    }
}

static void egui_view_card_panel_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                           egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (text == NULL || text[0] == '\0')
    {
        return;
    }
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static egui_dim_t egui_view_card_panel_pill_width(const char *text, uint8_t compact_mode, egui_dim_t min_w, egui_dim_t max_w)
{
    egui_dim_t width = min_w + egui_view_card_panel_text_len(text) * (compact_mode ? 4 : 5);

    if (width < min_w)
    {
        width = min_w;
    }
    if (width > max_w)
    {
        width = max_w;
    }
    return width;
}

static void egui_view_card_panel_draw_pill(const egui_font_t *font, egui_view_t *self, const char *text, egui_dim_t x, egui_dim_t y, egui_dim_t width,
                                           egui_dim_t height, egui_dim_t radius, egui_color_t fill_color, egui_alpha_t fill_alpha, egui_color_t border_color,
                                           egui_alpha_t border_alpha, egui_color_t text_color)
{
    egui_region_t text_region;

    if (text == NULL || text[0] == '\0' || width <= 0 || height <= 0)
    {
        return;
    }

    egui_canvas_draw_round_rectangle_fill(x, y, width, height, radius, fill_color, egui_color_alpha_mix(self->alpha, fill_alpha));
    egui_canvas_draw_round_rectangle(x, y, width, height, radius, 1, border_color, egui_color_alpha_mix(self->alpha, border_alpha));

    text_region.location.x = x;
    text_region.location.y = y;
    text_region.size.width = width;
    text_region.size.height = height;
    egui_canvas_draw_text_in_rect(font, text, &text_region, EGUI_ALIGN_CENTER, text_color, self->alpha);
}

void egui_view_card_panel_set_snapshots(egui_view_t *self, const egui_view_card_panel_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_card_panel_t);
    local->snapshots = snapshots;
    local->snapshot_count = egui_view_card_panel_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }
    egui_view_card_panel_clear_pressed_state(self);
    egui_view_invalidate(self);
}

void egui_view_card_panel_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    EGUI_LOCAL_INIT(egui_view_card_panel_t);
    if (local->snapshot_count == 0 || snapshot_index >= local->snapshot_count)
    {
        return;
    }
    if (local->current_snapshot == snapshot_index)
    {
        if (self->is_pressed)
        {
            egui_view_card_panel_clear_pressed_state(self);
            egui_view_invalidate(self);
        }
        return;
    }
    local->current_snapshot = snapshot_index;
    egui_view_card_panel_clear_pressed_state(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_card_panel_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_card_panel_t);
    return local->current_snapshot;
}

void egui_view_card_panel_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_card_panel_t);
    local->font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_card_panel_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_card_panel_t);
    local->meta_font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_card_panel_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_card_panel_t);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_card_panel_clear_pressed_state(self);
    egui_view_invalidate(self);
}

void egui_view_card_panel_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_card_panel_t);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_card_panel_clear_pressed_state(self);
    egui_view_invalidate(self);
}

void egui_view_card_panel_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                      egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t success_color, egui_color_t warning_color,
                                      egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_card_panel_t);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    egui_view_invalidate(self);
}

static void egui_view_card_panel_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_card_panel_t);
    egui_region_t region;
    egui_region_t text_region;
    const egui_view_card_panel_snapshot_t *snapshot;
    egui_color_t tone_color;
    egui_color_t card_fill;
    egui_color_t card_border;
    egui_color_t title_color;
    egui_color_t body_color;
    egui_color_t footer_color;
    egui_color_t section_fill;
    egui_color_t section_border;
    egui_color_t badge_fill;
    egui_color_t badge_border;
    egui_color_t action_fill;
    egui_color_t action_border;
    egui_color_t value_fill;
    egui_color_t value_border;
    egui_color_t value_color;
    egui_dim_t x;
    egui_dim_t y;
    egui_dim_t w;
    egui_dim_t h;
    egui_dim_t padding;
    egui_dim_t radius;
    egui_dim_t badge_h;
    egui_dim_t badge_w;
    egui_dim_t action_h;
    egui_dim_t action_w;
    egui_dim_t action_x;
    egui_dim_t summary_w;
    egui_dim_t summary_h;
    egui_dim_t summary_x;
    egui_dim_t summary_y;
    egui_dim_t title_x;
    egui_dim_t title_y;
    egui_dim_t title_w;
    egui_dim_t title_h;
    egui_dim_t body_y;
    egui_dim_t body_h;
    egui_dim_t detail_h;
    egui_dim_t detail_y;
    egui_dim_t footer_y;
    egui_dim_t footer_h;
    uint8_t show_action;
    uint8_t is_enabled;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0 || local->snapshots == NULL || local->snapshot_count == 0)
    {
        return;
    }

    snapshot = &local->snapshots[local->current_snapshot];
    tone_color = egui_view_card_panel_tone_color(local, snapshot->tone);
    card_fill = egui_rgb_mix(local->surface_color, tone_color, snapshot->emphasized ? (local->compact_mode ? 5 : 8) : (local->compact_mode ? 3 : 4));
    card_border = egui_rgb_mix(local->border_color, tone_color, snapshot->emphasized ? (local->compact_mode ? 12 : 16) : (local->compact_mode ? 6 : 10));
    title_color = local->text_color;
    body_color = egui_rgb_mix(local->muted_text_color, local->text_color, local->compact_mode ? 12 : 18);
    footer_color = egui_rgb_mix(local->muted_text_color, tone_color, local->compact_mode ? 8 : 12);
    section_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 5 : 6);
    section_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 8 : 10);
    badge_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 8 : 10);
    badge_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 12 : 16);
    action_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 3 : 4);
    action_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 8 : 10);
    value_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 6 : 8);
    value_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 9 : 12);
    value_color = egui_rgb_mix(local->text_color, tone_color, snapshot->emphasized ? (local->compact_mode ? 18 : 24) : 8);

    is_enabled = egui_view_get_enable(self) ? 1 : 0;
    if (local->read_only_mode)
    {
        tone_color = egui_rgb_mix(tone_color, local->muted_text_color, 82);
        card_fill = egui_rgb_mix(card_fill, EGUI_COLOR_HEX(0xFCFDFE), 24);
        card_border = egui_rgb_mix(card_border, local->muted_text_color, 10);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 24);
        body_color = egui_rgb_mix(body_color, local->muted_text_color, 28);
        footer_color = egui_rgb_mix(footer_color, local->muted_text_color, 36);
        section_fill = egui_rgb_mix(section_fill, local->surface_color, 30);
        section_border = egui_rgb_mix(section_border, local->muted_text_color, 18);
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, 24);
        badge_border = egui_rgb_mix(badge_border, local->muted_text_color, 14);
        action_fill = egui_rgb_mix(action_fill, local->surface_color, 34);
        action_border = egui_rgb_mix(action_border, local->muted_text_color, 22);
        value_fill = egui_rgb_mix(value_fill, local->surface_color, 30);
        value_border = egui_rgb_mix(value_border, local->muted_text_color, 18);
        value_color = egui_rgb_mix(value_color, local->muted_text_color, 40);
    }

    if (!is_enabled)
    {
        tone_color = egui_view_card_panel_mix_disabled(tone_color);
        card_fill = egui_view_card_panel_mix_disabled(card_fill);
        card_border = egui_view_card_panel_mix_disabled(card_border);
        title_color = egui_view_card_panel_mix_disabled(title_color);
        body_color = egui_view_card_panel_mix_disabled(body_color);
        footer_color = egui_view_card_panel_mix_disabled(footer_color);
        section_fill = egui_view_card_panel_mix_disabled(section_fill);
        section_border = egui_view_card_panel_mix_disabled(section_border);
        badge_fill = egui_view_card_panel_mix_disabled(badge_fill);
        badge_border = egui_view_card_panel_mix_disabled(badge_border);
        action_fill = egui_view_card_panel_mix_disabled(action_fill);
        action_border = egui_view_card_panel_mix_disabled(action_border);
        value_fill = egui_view_card_panel_mix_disabled(value_fill);
        value_border = egui_view_card_panel_mix_disabled(value_border);
        value_color = egui_view_card_panel_mix_disabled(value_color);
    }

    x = region.location.x;
    y = region.location.y;
    w = region.size.width;
    h = region.size.height;
    padding = local->compact_mode ? 8 : 10;
    radius = local->compact_mode ? 7 : 9;
    badge_h = local->compact_mode ? 10 : 11;
    action_h = badge_h;
    badge_w = egui_view_card_panel_pill_width(snapshot->badge, local->compact_mode, local->compact_mode ? 34 : 42, local->compact_mode ? 64 : 78);
    action_w = egui_view_card_panel_pill_width(snapshot->action, local->compact_mode, local->compact_mode ? 22 : 36, local->compact_mode ? 42 : 66);
    summary_w = local->compact_mode ? 32 : 48;
    summary_h = local->compact_mode ? 26 : 44;
    summary_x = x + w - padding - summary_w;
    summary_y = y + padding + badge_h + (local->compact_mode ? 6 : 8);
    title_x = x + padding;
    title_y = y + padding + badge_h + (local->compact_mode ? 6 : 8);
    title_w = summary_x - title_x - (local->compact_mode ? 4 : 10);
    title_h = local->compact_mode ? 11 : 14;
    body_y = title_y + title_h + (local->compact_mode ? 2 : 4);
    detail_h = local->compact_mode ? 16 : 30;
    footer_h = local->compact_mode ? 10 : 11;
    footer_y = y + h - padding - footer_h;
    detail_y = footer_y - (local->compact_mode ? 4 : 6) - detail_h;
    body_h = detail_y - body_y - (local->compact_mode ? 4 : 6);
    if (body_h < 10)
    {
        body_h = 10;
    }

    egui_canvas_draw_round_rectangle_fill(x, y, w, h, radius, card_fill, egui_color_alpha_mix(self->alpha, local->compact_mode ? 97 : 100));
    egui_canvas_draw_round_rectangle(x, y, w, h, radius, 1, card_border, egui_color_alpha_mix(self->alpha, local->compact_mode ? 56 : 62));
    egui_canvas_draw_round_rectangle_fill(x + 2, y + 2, w - 4, local->compact_mode ? 2 : 3, radius - 2, tone_color,
                                          egui_color_alpha_mix(self->alpha, snapshot->emphasized ? 22 : 12));

    egui_view_card_panel_draw_pill(local->meta_font, self, snapshot->badge, x + padding, y + padding, badge_w, badge_h, 5, badge_fill,
                                   local->compact_mode ? 26 : 30, badge_border, local->compact_mode ? 34 : 40, tone_color);

    show_action = (snapshot->action != NULL && snapshot->action[0] != '\0' && !local->read_only_mode && is_enabled) ? 1 : 0;
    if (show_action)
    {
        action_x = x + w - padding - action_w;
        egui_view_card_panel_draw_pill(local->meta_font, self, snapshot->action, action_x, y + padding, action_w, action_h, 5, action_fill,
                                       local->compact_mode ? 18 : 22, action_border, local->compact_mode ? 28 : 32, tone_color);
    }

    text_region.location.x = title_x;
    text_region.location.y = title_y;
    text_region.size.width = title_w;
    text_region.size.height = title_h;
    egui_view_card_panel_draw_text(local->font, self, snapshot->title, &text_region, EGUI_ALIGN_LEFT, title_color);

    text_region.location.x = title_x;
    text_region.location.y = body_y;
    text_region.size.width = title_w;
    text_region.size.height = body_h;
    egui_view_card_panel_draw_text(local->font, self, snapshot->body, &text_region, EGUI_ALIGN_LEFT, body_color);

    egui_canvas_draw_round_rectangle_fill(summary_x, summary_y, summary_w, summary_h, local->compact_mode ? 6 : 8, value_fill,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 22 : 26));
    egui_canvas_draw_round_rectangle(summary_x, summary_y, summary_w, summary_h, local->compact_mode ? 6 : 8, 1, value_border,
                                     egui_color_alpha_mix(self->alpha, local->compact_mode ? 30 : 34));
    egui_canvas_draw_round_rectangle_fill(summary_x + 4, summary_y + summary_h - 6, summary_w - 8, 2, 1, tone_color,
                                          egui_color_alpha_mix(self->alpha, snapshot->emphasized ? 46 : 24));

    text_region.location.x = summary_x;
    text_region.location.y = summary_y + (local->compact_mode ? 3 : 7);
    text_region.size.width = summary_w;
    text_region.size.height = local->compact_mode ? 11 : 14;
    egui_view_card_panel_draw_text(local->font, self, snapshot->value, &text_region, EGUI_ALIGN_CENTER, value_color);

    text_region.location.x = summary_x + 2;
    text_region.location.y = summary_y + summary_h - (local->compact_mode ? 12 : 15);
    text_region.size.width = summary_w - 4;
    text_region.size.height = 10;
    egui_view_card_panel_draw_text(local->meta_font, self, snapshot->value_label, &text_region, EGUI_ALIGN_CENTER, body_color);

    egui_canvas_draw_round_rectangle_fill(x + padding, detail_y, w - padding * 2, detail_h, local->compact_mode ? 6 : 8, section_fill,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 16 : 20));
    egui_canvas_draw_round_rectangle(x + padding, detail_y, w - padding * 2, detail_h, local->compact_mode ? 6 : 8, 1, section_border,
                                     egui_color_alpha_mix(self->alpha, local->compact_mode ? 22 : 26));

    text_region.location.x = x + padding + 6;
    text_region.location.y = detail_y + (local->compact_mode ? 3 : 5);
    text_region.size.width = w - padding * 2 - 12;
    text_region.size.height = 10;
    egui_view_card_panel_draw_text(local->meta_font, self, snapshot->detail_title, &text_region, EGUI_ALIGN_LEFT, tone_color);

    if (!local->compact_mode)
    {
        text_region.location.x = x + padding + 6;
        text_region.location.y = detail_y + 14;
        text_region.size.width = w - padding * 2 - 12;
        text_region.size.height = detail_h - 16;
        egui_view_card_panel_draw_text(local->font, self, snapshot->detail_body, &text_region, EGUI_ALIGN_LEFT, body_color);
    }

    text_region.location.x = x + padding;
    text_region.location.y = footer_y;
    text_region.size.width = w - padding * 2;
    text_region.size.height = footer_h;
    egui_view_card_panel_draw_text(local->meta_font, self, snapshot->footer, &text_region, EGUI_ALIGN_LEFT, footer_color);

    if (local->read_only_mode || !is_enabled)
    {
        egui_canvas_draw_line(x + padding, footer_y - 2, x + w - padding, footer_y - 2, 1, card_border, egui_color_alpha_mix(self->alpha, 24));
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_card_panel_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_card_panel_t);

    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        if (self->is_pressed)
        {
            egui_view_card_panel_clear_pressed_state(self);
            egui_view_invalidate(self);
        }
        EGUI_UNUSED(event);
        return 0;
    }

    return egui_view_on_touch_event(self, event);
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_card_panel_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_card_panel_t);

    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        return 0;
    }

    return egui_view_on_key_event(self, event);
}
#endif

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_card_panel_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_card_panel_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_card_panel_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_card_panel_on_key_event,
#endif
};

void egui_view_card_panel_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_card_panel_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_card_panel_t);
    egui_view_set_padding_all(self, 2);

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD2DBE3);
    local->text_color = EGUI_COLOR_HEX(0x1A2734);
    local->muted_text_color = EGUI_COLOR_HEX(0x6B7A89);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->success_color = EGUI_COLOR_HEX(0x0F7B45);
    local->warning_color = EGUI_COLOR_HEX(0x9D5D00);
    local->neutral_color = EGUI_COLOR_HEX(0x7A8796);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;
}
