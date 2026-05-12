#include <stdlib.h>

#include "egui_view_skeleton.h"

static uint8_t egui_view_skeleton_clamp_snapshot_count(uint8_t count)
{
    if (count > EGUI_VIEW_SKELETON_MAX_SNAPSHOTS)
    {
        return EGUI_VIEW_SKELETON_MAX_SNAPSHOTS;
    }
    return count;
}

static uint8_t egui_view_skeleton_clamp_block_count(uint8_t count)
{
    if (count > EGUI_VIEW_SKELETON_MAX_BLOCKS)
    {
        return EGUI_VIEW_SKELETON_MAX_BLOCKS;
    }
    return count;
}

static egui_color_t egui_view_skeleton_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(44));
}

static uint8_t egui_view_skeleton_clear_pressed_state(egui_view_t *self)
{
    if (!self->is_pressed)
    {
        return 0;
    }
    egui_view_set_pressed(self, false);
    return 1;
}

static egui_dim_t egui_view_skeleton_measure_font_line_height(const egui_font_t *font)
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

static egui_dim_t egui_view_skeleton_measure_text_width(const egui_font_t *font, const char *text)
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

static uint8_t egui_view_skeleton_text_len(const char *text)
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

static uint8_t egui_view_skeleton_is_space_char(char c)
{
    return (uint8_t)(c == ' ' || c == '\t');
}

static uint8_t egui_view_skeleton_is_break_after_char(char c)
{
    return (uint8_t)(c == '-' || c == '/');
}

static uint8_t egui_view_skeleton_find_elide_boundary(const char *text, uint8_t visible_chars)
{
    uint8_t index;

    if (text == NULL || visible_chars == 0)
    {
        return 0;
    }

    for (index = visible_chars; index > 0; --index)
    {
        if (egui_view_skeleton_is_space_char(text[index - 1]))
        {
            return (uint8_t)(index - 1);
        }
    }

    for (index = visible_chars; index > 0; --index)
    {
        if (egui_view_skeleton_is_break_after_char(text[index - 1]))
        {
            return index;
        }
    }

    return visible_chars;
}

static void egui_view_skeleton_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
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

    length = egui_view_skeleton_text_len(text);
    if (length <= max_chars)
    {
        copy_length = length;
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (index = 0; index < copy_length; ++index)
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
        for (index = 0; index < copy_length; ++index)
        {
            buffer[index] = '.';
        }
        buffer[copy_length] = '\0';
        return;
    }

    copy_length = egui_view_skeleton_find_elide_boundary(text, (uint8_t)(max_chars - 3));
    if (copy_length > buffer_size - 4)
    {
        copy_length = buffer_size - 4;
    }
    for (index = 0; index < copy_length; ++index)
    {
        buffer[index] = text[index];
    }
    buffer[copy_length] = '.';
    buffer[copy_length + 1] = '.';
    buffer[copy_length + 2] = '.';
    buffer[copy_length + 3] = '\0';
}

static void egui_view_skeleton_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width,
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

    max_chars = egui_view_skeleton_text_len(text);
    egui_view_skeleton_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t text_width = egui_view_skeleton_measure_text_width(font, buffer);

        if (text_width <= 0)
        {
            text_width = (egui_dim_t)egui_view_skeleton_text_len(buffer) * fallback_char_width;
        }
        if (text_width <= max_width)
        {
            break;
        }

        max_chars--;
        egui_view_skeleton_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_dim_t egui_view_skeleton_resolve_line_height(const egui_font_t *font, egui_dim_t fallback)
{
    egui_dim_t line_height = egui_view_skeleton_measure_font_line_height(font);

    return line_height > fallback ? line_height : fallback;
}

static void egui_view_skeleton_tick(egui_timer_t *timer)
{
    egui_view_t *self = (egui_view_t *)timer->user_data;
    EGUI_LOCAL_INIT(egui_view_skeleton_t);

    local->anim_phase = (uint8_t)((local->anim_phase + 1) % 24);
    egui_view_invalidate(self);
}

static void egui_view_skeleton_start_timer(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);

    if (local->timer_started || local->animation_mode == EGUI_VIEW_SKELETON_ANIM_NONE || local->read_only_mode)
    {
        return;
    }

    egui_timer_start_timer(uicode_get_core(), &local->anim_timer, 80, 80);
    local->timer_started = 1;
}

static void egui_view_skeleton_stop_timer(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);

    if (!local->timer_started)
    {
        return;
    }

    egui_timer_stop_timer(uicode_get_core(), &local->anim_timer);
    local->timer_started = 0;
}

void egui_view_skeleton_set_snapshots(egui_view_t *self, const egui_view_skeleton_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);
    uint8_t had_pressed = egui_view_skeleton_clear_pressed_state(self);

    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : egui_view_skeleton_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_skeleton_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);

    if (local->snapshot_count == 0 || snapshot_index >= local->snapshot_count)
    {
        if (egui_view_skeleton_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }
    if (local->current_snapshot == snapshot_index)
    {
        if (egui_view_skeleton_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_snapshot = snapshot_index;
    egui_view_skeleton_clear_pressed_state(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_skeleton_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);
    return local->current_snapshot;
}

void egui_view_skeleton_set_emphasis_block(egui_view_t *self, uint8_t block_index)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);
    uint8_t had_pressed = egui_view_skeleton_clear_pressed_state(self);

    local->emphasis_block = block_index;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_skeleton_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);
    uint8_t had_pressed = egui_view_skeleton_clear_pressed_state(self);

    local->font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_skeleton_set_show_footer(egui_view_t *self, uint8_t show_footer)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);
    uint8_t had_pressed = egui_view_skeleton_clear_pressed_state(self);

    local->show_footer = show_footer ? 1 : 0;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_skeleton_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);
    uint8_t had_pressed = egui_view_skeleton_clear_pressed_state(self);

    local->compact_mode = compact_mode ? 1 : 0;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_skeleton_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);
    uint8_t had_pressed = egui_view_skeleton_clear_pressed_state(self);

    local->read_only_mode = read_only_mode ? 1 : 0;
    if (local->read_only_mode)
    {
        egui_view_skeleton_stop_timer(self);
    }
    else
    {
        egui_view_skeleton_start_timer(self);
    }
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_skeleton_set_animation_mode(egui_view_t *self, uint8_t animation_mode)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);
    uint8_t had_pressed = egui_view_skeleton_clear_pressed_state(self);

    if (animation_mode > EGUI_VIEW_SKELETON_ANIM_PULSE)
    {
        animation_mode = EGUI_VIEW_SKELETON_ANIM_PULSE;
    }
    local->animation_mode = animation_mode;
    if (local->animation_mode == EGUI_VIEW_SKELETON_ANIM_NONE || local->read_only_mode)
    {
        egui_view_skeleton_stop_timer(self);
    }
    else
    {
        egui_view_skeleton_start_timer(self);
    }
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_skeleton_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t block_color, egui_color_t text_color,
                                    egui_color_t muted_text_color, egui_color_t accent_color)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);
    uint8_t had_pressed = egui_view_skeleton_clear_pressed_state(self);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->block_color = block_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

static uint8_t egui_view_skeleton_get_pulse_mix(uint8_t phase)
{
    uint8_t local_phase = phase % 12;

    if (local_phase > 6)
    {
        local_phase = 12 - local_phase;
    }
    return 12 + local_phase * 5;
}

static void egui_view_skeleton_draw_footer(egui_view_skeleton_t *local, egui_view_t *self, const char *text, egui_dim_t x, egui_dim_t y, egui_dim_t width,
                                           egui_dim_t height, egui_color_t color)
{
    char footer_text[48];
    egui_region_t text_region;

    if (local->font == NULL || text == NULL || width <= 0 || height <= 0)
    {
        return;
    }

    egui_view_skeleton_fit_text_to_width(local->font, text, footer_text, sizeof(footer_text), width, local->compact_mode ? 4 : 5);
    text_region.location.x = x;
    text_region.location.y = y;
    text_region.size.width = width;
    text_region.size.height = height;
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, local->font, footer_text, &text_region, EGUI_ALIGN_LEFT, color, self->alpha);
}

static void egui_view_skeleton_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);
    egui_region_t region;
    const egui_view_skeleton_snapshot_t *snapshot;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t block_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_dim_t radius;
    egui_dim_t content_x;
    egui_dim_t content_y;
    egui_dim_t content_width;
    egui_dim_t content_height;
    egui_dim_t footer_h = egui_view_skeleton_resolve_line_height(local->font, 10);
    egui_dim_t footer_y;
    uint8_t block_count;
    uint8_t i;
    uint8_t is_enabled;
    uint8_t pulse_mix = egui_view_skeleton_get_pulse_mix(local->anim_phase);

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0 || local->snapshots == NULL || local->snapshot_count == 0)
    {
        return;
    }

    snapshot = &local->snapshots[local->current_snapshot];
    block_count = egui_view_skeleton_clamp_block_count(snapshot->block_count);
    if (snapshot->blocks == NULL || block_count == 0)
    {
        return;
    }

    is_enabled = egui_view_get_enable(self) ? 1 : 0;
    surface_color = local->surface_color;
    border_color = local->border_color;
    block_color = local->block_color;
    text_color = local->text_color;
    muted_text_color = local->muted_text_color;
    accent_color = local->accent_color;
    radius = local->compact_mode ? 7 : 10;

    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(16));
        border_color = egui_rgb_mix(border_color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(24));
        block_color = egui_rgb_mix(block_color, HCW_COLOR_PANEL, EGUI_ALPHA_MAKE(20));
        text_color = egui_rgb_mix(text_color, muted_text_color, EGUI_ALPHA_MAKE(42));
        muted_text_color = egui_rgb_mix(muted_text_color, surface_color, EGUI_ALPHA_MAKE(12));
        accent_color = egui_rgb_mix(accent_color, muted_text_color, EGUI_ALPHA_MAKE(36));
    }
    else if (!local->compact_mode)
    {
        surface_color = egui_rgb_mix(surface_color, HCW_COLOR_SURFACE, EGUI_ALPHA_MAKE(10));
        border_color = egui_rgb_mix(border_color, HCW_COLOR_SURFACE, EGUI_ALPHA_MAKE(18));
    }

    if (!is_enabled)
    {
        surface_color = egui_view_skeleton_mix_disabled(surface_color);
        border_color = egui_view_skeleton_mix_disabled(border_color);
        block_color = egui_view_skeleton_mix_disabled(block_color);
        text_color = egui_view_skeleton_mix_disabled(text_color);
        muted_text_color = egui_view_skeleton_mix_disabled(muted_text_color);
        accent_color = egui_view_skeleton_mix_disabled(accent_color);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius, surface_color,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->compact_mode ? 94 : 96)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius, 1, border_color,
                                     egui_color_alpha_mix(self->alpha,
                                                          EGUI_ALPHA_MAKE(local->read_only_mode ? (local->compact_mode ? 64 : 70) : (local->compact_mode ? 82 : 88))));

    content_x = region.location.x + (local->compact_mode ? 8 : 10);
    content_y = region.location.y + (local->compact_mode ? 8 : 10);
    content_width = region.size.width - (local->compact_mode ? 16 : 20);
    content_height = region.size.height - (local->compact_mode ? 16 : 20);
    if (local->show_footer && !local->compact_mode)
    {
        content_height -= footer_h + 2;
    }
    if (content_width <= 0 || content_height <= 0)
    {
        return;
    }

    for (i = 0; i < block_count; i++)
    {
        const egui_view_skeleton_block_t *block = &snapshot->blocks[i];
        egui_dim_t x = content_x + block->x;
        egui_dim_t y = content_y + block->y;
        egui_dim_t w = block->width;
        egui_dim_t h = block->height;
        egui_dim_t block_right;
        egui_dim_t band_width;
        egui_dim_t band_x;
        egui_color_t fill_color;
        egui_color_t line_color;
        uint8_t is_emphasis;

        if (w <= 0 || h <= 0)
        {
            continue;
        }
        if (x + w > content_x + content_width || y + h > content_y + content_height)
        {
            continue;
        }

        is_emphasis = (i == snapshot->emphasis_block) || (i == local->emphasis_block);
        fill_color = egui_rgb_mix(block_color, accent_color, EGUI_ALPHA_MAKE(is_emphasis ? 18 : 8));
        line_color = egui_rgb_mix(border_color, accent_color, EGUI_ALPHA_MAKE(is_emphasis ? 30 : 14));

        if (local->animation_mode == EGUI_VIEW_SKELETON_ANIM_PULSE && is_emphasis && !local->read_only_mode)
        {
            fill_color = egui_rgb_mix(block_color, accent_color, EGUI_ALPHA_MAKE(18 + pulse_mix / 2));
            line_color = egui_rgb_mix(border_color, accent_color, EGUI_ALPHA_MAKE(26 + pulse_mix / 2));
        }

        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, x, y, w, h, block->radius, fill_color,
                                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? (local->compact_mode ? 54 : 64)
                                                                                                                       : (local->compact_mode ? 88 : 96))));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, x, y, w, h, block->radius, 1, line_color,
                                         egui_color_alpha_mix(self->alpha,
                                                              EGUI_ALPHA_MAKE(
                                                                      local->read_only_mode ? (local->compact_mode ? (is_emphasis ? 34 : 22) : (is_emphasis ? 42 : 30))
                                                                                            : (local->compact_mode ? (is_emphasis ? 70 : 46)
                                                                                                                   : (is_emphasis ? 82 : 58)))));

        if (local->animation_mode == EGUI_VIEW_SKELETON_ANIM_WAVE && !local->read_only_mode)
        {
            egui_dim_t overlap_x0;
            egui_dim_t overlap_x1;
            egui_color_t band_color = egui_rgb_mix(block_color, accent_color, EGUI_ALPHA_MAKE(is_emphasis ? 22 : 14));

            band_width = local->compact_mode ? 9 : 12;
            block_right = content_width + (local->compact_mode ? 16 : 24);
            band_x = content_x - band_width + ((egui_dim_t)local->anim_phase * block_right) / 24;
            overlap_x0 = EGUI_MAX(x + 1, band_x);
            overlap_x1 = EGUI_MIN(x + w - 1, band_x + band_width);
            if (overlap_x1 > overlap_x0)
            {
                egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, overlap_x0, y + 1, overlap_x1 - overlap_x0, h - 2, EGUI_MIN(block->radius, 3), band_color,
                                                      egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(is_emphasis ? 52 : 34)));
            }
        }
    }

    if (local->show_footer && !local->compact_mode)
    {
        footer_y = content_y + content_height + 3;
        egui_view_skeleton_draw_footer(local, self, snapshot->footer ? snapshot->footer : "Loading content", content_x, footer_y, content_width, footer_h,
                                       local->read_only_mode ? muted_text_color : text_color);
    }
}

static void egui_view_skeleton_on_attach(egui_view_t *self)
{
    egui_view_on_attach_to_window(self);
    egui_view_skeleton_start_timer(self);
}

static void egui_view_skeleton_on_detach(egui_view_t *self)
{
    egui_view_skeleton_stop_timer(self);
    egui_view_on_detach_from_window(self);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_skeleton_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);

    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        if (egui_view_skeleton_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        EGUI_UNUSED(event);
        return 0;
    }

    return egui_view_on_touch_event(self, event);
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_skeleton_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_skeleton_t);

    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        if (egui_view_skeleton_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        EGUI_UNUSED(event);
        return 0;
    }

    return egui_view_on_key_event(self, event);
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_skeleton_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_skeleton_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_skeleton_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_skeleton_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_skeleton_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_skeleton_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_skeleton_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_skeleton_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_skeleton_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_skeleton_on_attach,
        .on_draw = egui_view_skeleton_on_draw,
        .on_detach_from_window = egui_view_skeleton_on_detach,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_skeleton_on_key_event,
#endif
};

void egui_view_skeleton_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_skeleton_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_skeleton_t);
    egui_view_set_padding_all(self, 2);

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->surface_color = HCW_COLOR_SURFACE;
    local->border_color = HCW_COLOR_BORDER;
    local->block_color = HCW_COLOR_SURFACE_DISABLED;
    local->text_color = HCW_COLOR_TEXT_MUTED;
    local->muted_text_color = HCW_COLOR_TEXT_SOFT;
    local->accent_color = HCW_COLOR_PRIMARY_LIGHT;
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->emphasis_block = 0xFF;
    local->show_footer = 1;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->animation_mode = EGUI_VIEW_SKELETON_ANIM_WAVE;
    local->anim_phase = 0;
    local->timer_started = 0;
    egui_timer_init_timer(&local->anim_timer, self, egui_view_skeleton_tick);
}
