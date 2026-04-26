#include "egui_view_snackbar.h"

#define EGUI_VIEW_SNACKBAR_PART_NONE   0
#define EGUI_VIEW_SNACKBAR_PART_ACTION 1
#define EGUI_VIEW_SNACKBAR_PART_CLOSE  2

typedef struct egui_view_snackbar_metrics egui_view_snackbar_metrics_t;
struct egui_view_snackbar_metrics
{
    egui_region_t content_region;
    egui_region_t icon_region;
    egui_region_t title_region;
    egui_region_t message_region;
    egui_region_t action_region;
    egui_region_t close_region;
    uint8_t show_action;
    uint8_t show_close;
};

static uint8_t egui_view_snackbar_clamp_snapshot_count(uint8_t count)
{
    if (count > EGUI_VIEW_SNACKBAR_MAX_SNAPSHOTS)
    {
        return EGUI_VIEW_SNACKBAR_MAX_SNAPSHOTS;
    }
    return count;
}

static uint8_t egui_view_snackbar_text_len(const char *text)
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

static uint8_t egui_view_snackbar_is_space_char(char c)
{
    return (uint8_t)(c == ' ' || c == '\t');
}

static uint8_t egui_view_snackbar_is_break_after_char(char c)
{
    return (uint8_t)(c == '-' || c == '/');
}

static uint8_t egui_view_snackbar_find_elide_boundary(const char *text, uint8_t visible_chars)
{
    uint8_t index;

    if (text == NULL || visible_chars == 0)
    {
        return 0;
    }

    for (index = visible_chars; index > 0; --index)
    {
        if (egui_view_snackbar_is_space_char(text[index - 1]))
        {
            return (uint8_t)(index - 1);
        }
    }

    for (index = visible_chars; index > 0; --index)
    {
        if (egui_view_snackbar_is_break_after_char(text[index - 1]))
        {
            return index;
        }
    }

    return visible_chars;
}

static void egui_view_snackbar_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
{
    uint8_t copy_length;
    uint8_t index;
    uint8_t length;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (text == NULL || max_chars == 0)
    {
        return;
    }

    length = egui_view_snackbar_text_len(text);
    if (length <= max_chars)
    {
        copy_length = length;
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (index = 0; index < copy_length; index++)
        {
            buffer[index] = text[index];
        }
        buffer[copy_length] = '\0';
        return;
    }

    if (max_chars <= 3)
    {
        copy_length = max_chars;
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (index = 0; index < copy_length; index++)
        {
            buffer[index] = '.';
        }
        buffer[copy_length] = '\0';
        return;
    }

    copy_length = egui_view_snackbar_find_elide_boundary(text, (uint8_t)(max_chars - 3));
    if (copy_length > buffer_size - 4)
    {
        copy_length = buffer_size - 4;
    }
    for (index = 0; index < copy_length; index++)
    {
        buffer[index] = text[index];
    }
    buffer[copy_length] = '.';
    buffer[copy_length + 1] = '.';
    buffer[copy_length + 2] = '.';
    buffer[copy_length + 3] = '\0';
}

static egui_dim_t egui_view_snackbar_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t text_width = 0;
    egui_dim_t dummy_height = 0;

    if (text == NULL || text[0] == '\0' || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, text, 0, 0, &text_width, &dummy_height);
    return text_width;
}

static void egui_view_snackbar_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width,
                                                 egui_dim_t fallback_char_width)
{
    uint8_t max_chars;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (text == NULL || text[0] == '\0' || max_width <= 0)
    {
        return;
    }

    max_chars = egui_view_snackbar_text_len(text);
    egui_view_snackbar_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t text_width = egui_view_snackbar_measure_text_width(font, buffer);

        if (text_width <= 0)
        {
            text_width = (egui_dim_t)egui_view_snackbar_text_len(buffer) * fallback_char_width;
        }
        if (text_width <= max_width)
        {
            break;
        }

        max_chars--;
        egui_view_snackbar_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_dim_t egui_view_snackbar_measure_font_line_height(const egui_font_t *font)
{
    egui_dim_t dummy_width = 0;
    egui_dim_t line_height = 0;

    if (font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, "A", 0, 0, &dummy_width, &line_height);
    return line_height;
}

static egui_dim_t egui_view_snackbar_resolve_line_height(const egui_font_t *font, egui_dim_t fallback)
{
    egui_dim_t line_height = egui_view_snackbar_measure_font_line_height(font);

    return line_height > fallback ? line_height : fallback;
}

static egui_color_t egui_view_snackbar_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 65);
}

static egui_color_t egui_view_snackbar_severity_color(egui_view_snackbar_t *local, uint8_t severity)
{
    switch (severity)
    {
    case 1:
        return local->success_color;
    case 2:
        return local->warning_color;
    case 3:
        return local->error_color;
    default:
        return local->info_color;
    }
}

static const char *egui_view_snackbar_severity_glyph(uint8_t severity)
{
    switch (severity)
    {
    case 1:
        return "+";
    case 2:
        return "!";
    case 3:
        return "x";
    default:
        return "i";
    }
}

static uint8_t egui_view_snackbar_clear_pressed_state(egui_view_t *self, egui_view_snackbar_t *local)
{
    uint8_t had_state = (uint8_t)(self->is_pressed || local->pressed_part != EGUI_VIEW_SNACKBAR_PART_NONE);

    local->pressed_part = EGUI_VIEW_SNACKBAR_PART_NONE;
    if (self->is_pressed)
    {
        egui_view_set_pressed(self, 0);
    }
    else if (had_state)
    {
        egui_view_invalidate(self);
    }

    return had_state;
}

static const egui_view_snackbar_snapshot_t *egui_view_snackbar_get_snapshot(egui_view_snackbar_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }
    return &local->snapshots[local->current_snapshot];
}

static uint8_t egui_view_snackbar_snapshot_has_action(const egui_view_snackbar_snapshot_t *snapshot)
{
    return (uint8_t)(snapshot != NULL && snapshot->show_action && snapshot->action != NULL && snapshot->action[0] != '\0');
}

static uint8_t egui_view_snackbar_is_interactive(egui_view_t *self, egui_view_snackbar_t *local)
{
    return (uint8_t)(local->opened && !local->read_only_mode && egui_view_get_enable(self));
}

static void egui_view_snackbar_emit_open_changed(egui_view_t *self, egui_view_snackbar_t *local)
{
    if (local->on_open_changed != NULL)
    {
        local->on_open_changed(self, local->opened);
    }
}

static void egui_view_snackbar_set_opened_inner(egui_view_t *self, egui_view_snackbar_t *local, uint8_t opened, uint8_t notify)
{
    uint8_t had_pressed = egui_view_snackbar_clear_pressed_state(self, local);

    opened = opened ? 1 : 0;
    if (local->opened == opened)
    {
        return;
    }

    local->opened = opened;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
    if (notify)
    {
        egui_view_snackbar_emit_open_changed(self, local);
    }
}

static void egui_view_snackbar_emit_action(egui_view_t *self, egui_view_snackbar_t *local)
{
    if (local->on_action != NULL)
    {
        local->on_action(self, local->current_snapshot);
    }
}

void egui_view_snackbar_set_snapshots(egui_view_t *self, const egui_view_snackbar_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    uint8_t had_pressed = egui_view_snackbar_clear_pressed_state(self, local);

    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : egui_view_snackbar_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }
    local->opened = local->snapshot_count > 0 ? 1 : 0;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_snackbar_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    uint8_t had_pressed = egui_view_snackbar_clear_pressed_state(self, local);

    if (local->snapshot_count == 0 || snapshot_index >= local->snapshot_count)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_snapshot = snapshot_index;
    local->opened = 1;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

uint8_t egui_view_snackbar_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    return local->current_snapshot;
}

void egui_view_snackbar_set_opened(egui_view_t *self, uint8_t opened)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    egui_view_snackbar_set_opened_inner(self, local, opened, 0);
}

uint8_t egui_view_snackbar_get_opened(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    return local->opened;
}

void egui_view_snackbar_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    uint8_t had_pressed = egui_view_snackbar_clear_pressed_state(self, local);

    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_snackbar_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    uint8_t had_pressed = egui_view_snackbar_clear_pressed_state(self, local);

    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_snackbar_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    uint8_t had_pressed = egui_view_snackbar_clear_pressed_state(self, local);

    local->compact_mode = compact_mode ? 1 : 0;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_snackbar_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    uint8_t had_pressed = egui_view_snackbar_clear_pressed_state(self, local);

    local->read_only_mode = read_only_mode ? 1 : 0;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_snackbar_set_on_action_listener(egui_view_t *self, egui_view_snackbar_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    local->on_action = listener;
}

void egui_view_snackbar_set_on_open_changed_listener(egui_view_t *self, egui_view_snackbar_open_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    local->on_open_changed = listener;
}

void egui_view_snackbar_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                    egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t info_color, egui_color_t success_color,
                                    egui_color_t warning_color, egui_color_t error_color)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    uint8_t had_pressed = egui_view_snackbar_clear_pressed_state(self, local);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->info_color = info_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->error_color = error_color;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

static void egui_view_snackbar_get_metrics(egui_view_snackbar_t *local, egui_view_t *self, egui_view_snackbar_metrics_t *metrics)
{
    const egui_view_snackbar_snapshot_t *snapshot = egui_view_snackbar_get_snapshot(local);
    egui_region_t region;
    egui_dim_t pad_x;
    egui_dim_t pad_y;
    egui_dim_t icon_size;
    egui_dim_t close_size;
    egui_dim_t action_w;
    egui_dim_t action_h;
    egui_dim_t text_x;
    egui_dim_t text_right;
    egui_dim_t title_h;
    egui_dim_t message_h;

    egui_view_get_work_region(self, &region);
    pad_x = local->compact_mode ? 8 : 10;
    pad_y = local->compact_mode ? 7 : 9;
    icon_size = local->compact_mode ? 12 : 14;
    close_size = local->compact_mode ? 0 : 12;
    action_h = local->compact_mode ? 13 : 15;
    action_w = local->compact_mode ? 38 : 50;

    metrics->content_region.location.x = region.location.x + pad_x;
    metrics->content_region.location.y = region.location.y + pad_y;
    metrics->content_region.size.width = region.size.width - pad_x * 2;
    metrics->content_region.size.height = region.size.height - pad_y * 2;

    metrics->icon_region.location.x = metrics->content_region.location.x;
    metrics->icon_region.location.y = metrics->content_region.location.y + (local->compact_mode ? 1 : 2);
    metrics->icon_region.size.width = icon_size;
    metrics->icon_region.size.height = icon_size;

    metrics->show_close = (uint8_t)(snapshot != NULL && snapshot->closable && !local->compact_mode && !local->read_only_mode && egui_view_get_enable(self));
    metrics->close_region.location.x = metrics->content_region.location.x + metrics->content_region.size.width - close_size;
    metrics->close_region.location.y = metrics->content_region.location.y + 1;
    metrics->close_region.size.width = metrics->show_close ? close_size : 0;
    metrics->close_region.size.height = metrics->show_close ? close_size : 0;

    metrics->show_action = (uint8_t)(egui_view_snackbar_snapshot_has_action(snapshot) && !local->read_only_mode && egui_view_get_enable(self));
    if (snapshot != NULL && snapshot->action != NULL)
    {
        egui_dim_t text_w = egui_view_snackbar_measure_text_width(local->font, snapshot->action);

        if (text_w <= 0)
        {
            text_w = (egui_dim_t)egui_view_snackbar_text_len(snapshot->action) * (local->compact_mode ? 4 : 5);
        }
        if (text_w + 14 > action_w)
        {
            action_w = text_w + 14;
        }
    }
    if (action_w > metrics->content_region.size.width / 2)
    {
        action_w = metrics->content_region.size.width / 2;
    }
    metrics->action_region.size.width = metrics->show_action ? action_w : 0;
    metrics->action_region.size.height = metrics->show_action ? action_h : 0;
    metrics->action_region.location.x = metrics->content_region.location.x + metrics->content_region.size.width - action_w;
    metrics->action_region.location.y = metrics->content_region.location.y + metrics->content_region.size.height - action_h;

    text_x = metrics->icon_region.location.x + icon_size + (local->compact_mode ? 5 : 7);
    text_right = metrics->content_region.location.x + metrics->content_region.size.width - (metrics->show_close ? close_size + 4 : 0);
    if (metrics->show_action && metrics->action_region.location.y < metrics->content_region.location.y + metrics->content_region.size.height)
    {
        text_right = metrics->action_region.location.x - 5;
    }
    if (text_right <= text_x)
    {
        text_right = metrics->content_region.location.x + metrics->content_region.size.width;
    }

    title_h = egui_view_snackbar_resolve_line_height(local->font, local->compact_mode ? 10 : 12);
    message_h = egui_view_snackbar_resolve_line_height(local->meta_font, local->compact_mode ? 9 : 11);
    metrics->title_region.location.x = text_x;
    metrics->title_region.location.y = metrics->content_region.location.y;
    metrics->title_region.size.width = text_right - text_x;
    metrics->title_region.size.height = title_h;
    metrics->message_region.location.x = text_x;
    metrics->message_region.location.y = metrics->title_region.location.y + title_h + (local->compact_mode ? 2 : 3);
    metrics->message_region.size.width = text_right - text_x;
    metrics->message_region.size.height = message_h;
}

static void egui_view_snackbar_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align, egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (text == NULL || text[0] == '\0' || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, self->alpha);
}

static void egui_view_snackbar_draw_close(egui_view_t *self, const egui_region_t *region, egui_color_t color)
{
    egui_dim_t x0;
    egui_dim_t y0;
    egui_dim_t x1;
    egui_dim_t y1;

    if (region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    x0 = region->location.x + 3;
    y0 = region->location.y + 3;
    x1 = region->location.x + region->size.width - 4;
    y1 = region->location.y + region->size.height - 4;
    egui_canvas_draw_line(&uicode_get_core()->canvas, x0, y0, x1, y1, 1, color, egui_color_alpha_mix(self->alpha, 72));
    egui_canvas_draw_line(&uicode_get_core()->canvas, x1, y0, x0, y1, 1, color, egui_color_alpha_mix(self->alpha, 72));
}

static void egui_view_snackbar_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    const egui_view_snackbar_snapshot_t *snapshot;
    egui_view_snackbar_metrics_t metrics;
    egui_region_t region;
    char title_label[36];
    char message_label[48];
    char action_label[24];
    egui_color_t severity_color;
    egui_color_t fill_color;
    egui_color_t border_color;
    egui_color_t title_color;
    egui_color_t message_color;
    egui_color_t glyph_color;
    egui_color_t action_fill;
    egui_color_t action_border;
    egui_color_t action_text;
    egui_dim_t radius;
    egui_dim_t icon_radius;

    egui_view_get_work_region(self, &region);
    snapshot = egui_view_snackbar_get_snapshot(local);
    if (!local->opened || region.size.width <= 0 || region.size.height <= 0 || snapshot == NULL)
    {
        return;
    }

    severity_color = egui_view_snackbar_severity_color(local, snapshot->severity);
    fill_color = egui_rgb_mix(local->surface_color, severity_color, local->compact_mode ? 2 : 4);
    border_color = egui_rgb_mix(local->border_color, severity_color, local->compact_mode ? 3 : 6);
    title_color = local->text_color;
    message_color = local->muted_text_color;
    glyph_color = EGUI_COLOR_WHITE;

    if (!egui_view_get_enable(self))
    {
        severity_color = egui_view_snackbar_mix_disabled(severity_color);
        fill_color = egui_view_snackbar_mix_disabled(fill_color);
        border_color = egui_view_snackbar_mix_disabled(border_color);
        title_color = egui_view_snackbar_mix_disabled(title_color);
        message_color = egui_view_snackbar_mix_disabled(message_color);
        glyph_color = egui_view_snackbar_mix_disabled(glyph_color);
    }
    else if (local->read_only_mode)
    {
        severity_color = egui_rgb_mix(severity_color, local->muted_text_color, 60);
        fill_color = egui_rgb_mix(fill_color, local->surface_color, 26);
        border_color = egui_rgb_mix(border_color, local->muted_text_color, 32);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 34);
        message_color = egui_rgb_mix(message_color, local->text_color, 8);
        glyph_color = egui_rgb_mix(glyph_color, local->muted_text_color, 34);
    }

    radius = local->compact_mode ? 7 : 9;
    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius, fill_color,
                                          egui_color_alpha_mix(self->alpha, 94));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, 86));
    egui_canvas_draw_line(&uicode_get_core()->canvas, region.location.x + 8, region.location.y + region.size.height - 1, region.location.x + region.size.width - 8,
                          region.location.y + region.size.height - 1, 1, egui_rgb_mix(border_color, EGUI_COLOR_WHITE, 20), egui_color_alpha_mix(self->alpha, 30));

    egui_view_snackbar_get_metrics(local, self, &metrics);
    icon_radius = metrics.icon_region.size.width / 2;
    egui_canvas_draw_circle_fill(&uicode_get_core()->canvas, metrics.icon_region.location.x + icon_radius, metrics.icon_region.location.y + icon_radius, icon_radius,
                                 severity_color, egui_color_alpha_mix(self->alpha, local->read_only_mode ? 42 : 82));
    egui_view_snackbar_draw_text(local->font, self, egui_view_snackbar_severity_glyph(snapshot->severity), &metrics.icon_region, EGUI_ALIGN_CENTER, glyph_color);

    egui_view_snackbar_fit_text_to_width(local->font, snapshot->title, title_label, sizeof(title_label), metrics.title_region.size.width, local->compact_mode ? 4 : 5);
    egui_view_snackbar_draw_text(local->font, self, title_label, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);

    egui_view_snackbar_fit_text_to_width(local->meta_font, snapshot->message, message_label, sizeof(message_label), metrics.message_region.size.width,
                                         local->compact_mode ? 4 : 5);
    egui_view_snackbar_draw_text(local->meta_font, self, message_label, &metrics.message_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, message_color);

    if (metrics.show_action)
    {
        action_fill = egui_rgb_mix(local->surface_color, local->accent_color, self->is_pressed && local->pressed_part == EGUI_VIEW_SNACKBAR_PART_ACTION ? 12 : 4);
        action_border = egui_rgb_mix(local->border_color, local->accent_color, self->is_pressed && local->pressed_part == EGUI_VIEW_SNACKBAR_PART_ACTION ? 20 : 8);
        action_text = egui_rgb_mix(local->accent_color, local->text_color, 18);
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.action_region.location.x, metrics.action_region.location.y,
                                              metrics.action_region.size.width, metrics.action_region.size.height, 6, action_fill,
                                              egui_color_alpha_mix(self->alpha, 42));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.action_region.location.x, metrics.action_region.location.y, metrics.action_region.size.width,
                                         metrics.action_region.size.height, 6, 1, action_border, egui_color_alpha_mix(self->alpha, 52));
        egui_view_snackbar_fit_text_to_width(local->font, snapshot->action, action_label, sizeof(action_label), metrics.action_region.size.width - 6,
                                             local->compact_mode ? 4 : 5);
        egui_view_snackbar_draw_text(local->font, self, action_label, &metrics.action_region, EGUI_ALIGN_CENTER, action_text);
    }

    if (metrics.show_close)
    {
        if (self->is_pressed && local->pressed_part == EGUI_VIEW_SNACKBAR_PART_CLOSE)
        {
            egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.close_region.location.x, metrics.close_region.location.y,
                                                  metrics.close_region.size.width, metrics.close_region.size.height, 4, egui_rgb_mix(fill_color, severity_color, 10),
                                                  egui_color_alpha_mix(self->alpha, 44));
        }
        egui_view_snackbar_draw_close(self, &metrics.close_region, message_color);
    }
}

static uint8_t egui_view_snackbar_hit_part(egui_view_snackbar_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_snackbar_metrics_t metrics;

    if (!local->opened)
    {
        return EGUI_VIEW_SNACKBAR_PART_NONE;
    }

    egui_view_snackbar_get_metrics(local, self, &metrics);
    if (metrics.show_action && egui_region_pt_in_rect(&metrics.action_region, x, y))
    {
        return EGUI_VIEW_SNACKBAR_PART_ACTION;
    }
    if (metrics.show_close && egui_region_pt_in_rect(&metrics.close_region, x, y))
    {
        return EGUI_VIEW_SNACKBAR_PART_CLOSE;
    }

    return EGUI_VIEW_SNACKBAR_PART_NONE;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_snackbar_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    uint8_t hit_part;

    if (!egui_view_snackbar_is_interactive(self, local))
    {
        egui_view_snackbar_clear_pressed_state(self, local);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_part = egui_view_snackbar_hit_part(local, self, event->location.x, event->location.y);
        if (hit_part == EGUI_VIEW_SNACKBAR_PART_NONE)
        {
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        local->pressed_part = hit_part;
        egui_view_set_pressed(self, 1);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_part != EGUI_VIEW_SNACKBAR_PART_NONE)
        {
            uint8_t is_pressed = 0;

            hit_part = egui_view_snackbar_hit_part(local, self, event->location.x, event->location.y);
            if (hit_part == local->pressed_part)
            {
                is_pressed = 1;
            }
            if (self->is_pressed != is_pressed)
            {
                egui_view_set_pressed(self, is_pressed);
                egui_view_invalidate(self);
            }
            return 1;
        }
        return 0;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t pressed_part = local->pressed_part;
        uint8_t was_pressed = self->is_pressed;

        hit_part = egui_view_snackbar_hit_part(local, self, event->location.x, event->location.y);
        if (was_pressed && pressed_part != EGUI_VIEW_SNACKBAR_PART_NONE && pressed_part == hit_part)
        {
            if (pressed_part == EGUI_VIEW_SNACKBAR_PART_ACTION)
            {
                egui_view_snackbar_emit_action(self, local);
            }
            else if (pressed_part == EGUI_VIEW_SNACKBAR_PART_CLOSE)
            {
                egui_view_snackbar_set_opened_inner(self, local, 0, 1);
            }
        }
        egui_view_snackbar_clear_pressed_state(self, local);
        return (uint8_t)(hit_part != EGUI_VIEW_SNACKBAR_PART_NONE || pressed_part != EGUI_VIEW_SNACKBAR_PART_NONE);
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return egui_view_snackbar_clear_pressed_state(self, local);
    default:
        return 0;
    }
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_snackbar_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);
    const egui_view_snackbar_snapshot_t *snapshot;

    if (!egui_view_snackbar_is_interactive(self, local))
    {
        egui_view_snackbar_clear_pressed_state(self, local);
        return 0;
    }

    snapshot = egui_view_snackbar_get_snapshot(local);
    egui_view_snackbar_clear_pressed_state(self, local);
    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        switch (event->key_code)
        {
        case EGUI_KEY_CODE_ENTER:
        case EGUI_KEY_CODE_SPACE:
        case EGUI_KEY_CODE_ESCAPE:
            return 1;
        default:
            return 0;
        }
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (egui_view_snackbar_snapshot_has_action(snapshot))
        {
            egui_view_snackbar_emit_action(self, local);
            return 1;
        }
        return 0;
    case EGUI_KEY_CODE_ESCAPE:
        if (snapshot != NULL && snapshot->closable)
        {
            egui_view_snackbar_set_opened_inner(self, local, 0, 1);
            return 1;
        }
        return 0;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_snackbar_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);

    EGUI_UNUSED(event);
    egui_view_snackbar_clear_pressed_state(self, local);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_snackbar_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_snackbar_t);

    EGUI_UNUSED(event);
    egui_view_snackbar_clear_pressed_state(self, local);
    return 1;
}
#endif

void egui_view_snackbar_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_snackbar_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_snackbar_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_snackbar_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_snackbar_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_snackbar_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_snackbar_on_key_event,
#endif
};

void egui_view_snackbar_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_snackbar_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_snackbar_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 1);
#endif

    local->snapshots = NULL;
    local->on_action = NULL;
    local->on_open_changed = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD4DCE5);
    local->text_color = EGUI_COLOR_HEX(0x182331);
    local->muted_text_color = EGUI_COLOR_HEX(0x5F6F7F);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->info_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->success_color = EGUI_COLOR_HEX(0x107C41);
    local->warning_color = EGUI_COLOR_HEX(0xA15C07);
    local->error_color = EGUI_COLOR_HEX(0xC42B1C);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->opened = 0;
    local->pressed_part = EGUI_VIEW_SNACKBAR_PART_NONE;
    egui_view_set_view_name(self, "egui_view_snackbar");
}
