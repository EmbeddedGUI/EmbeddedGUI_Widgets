#include <string.h>

#include "egui_view_rich_text_block.h"

typedef struct egui_view_rich_text_block_paragraph_layout egui_view_rich_text_block_paragraph_layout_t;
struct egui_view_rich_text_block_paragraph_layout
{
    egui_region_t box_region;
    egui_region_t strip_region;
    egui_region_t text_region;
    const egui_font_t *font;
    egui_color_t text_color;
    uint8_t style;
    uint8_t has_box;
};

static uint8_t egui_view_rich_text_block_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_color_t egui_view_rich_text_block_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 64);
}

static uint8_t egui_view_rich_text_block_clamp_count(uint8_t paragraph_count)
{
    if (paragraph_count > EGUI_VIEW_RICH_TEXT_BLOCK_MAX_PARAGRAPHS)
    {
        return EGUI_VIEW_RICH_TEXT_BLOCK_MAX_PARAGRAPHS;
    }
    return paragraph_count;
}

static uint8_t egui_view_rich_text_block_clamp_style(uint8_t style)
{
    if (style > EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION)
    {
        return EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY;
    }
    return style;
}

static const egui_font_t *egui_view_rich_text_block_resolve_font(egui_view_rich_text_block_t *local, uint8_t style)
{
    switch (egui_view_rich_text_block_clamp_style(style))
    {
    case EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_EMPHASIS:
    case EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_ACCENT:
        if (local->emphasis_font != NULL)
        {
            return local->emphasis_font;
        }
        return local->font;
    case EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION:
        if (local->caption_font != NULL)
        {
            return local->caption_font;
        }
        return local->font;
    default:
        return local->font;
    }
}

static egui_color_t egui_view_rich_text_block_resolve_color(egui_view_rich_text_block_t *local, uint8_t style)
{
    switch (egui_view_rich_text_block_clamp_style(style))
    {
    case EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_EMPHASIS:
        return egui_rgb_mix(local->text_color, local->accent_color, 18);
    case EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_ACCENT:
        return local->accent_color;
    case EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION:
        return local->muted_text_color;
    default:
        return local->text_color;
    }
}

static egui_dim_t egui_view_rich_text_block_get_line_height(const egui_font_t *font)
{
    egui_dim_t dummy_width = 0;
    egui_dim_t line_height = 0;

    if (font == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, "A", 0, 0, &dummy_width, &line_height);
    return line_height;
}

static void egui_view_rich_text_block_get_char_metrics(const egui_font_t *font, const char *text, int *out_bytes, egui_dim_t *out_width)
{
    char glyph_buf[8];
    int glyph_bytes = 0;
    egui_dim_t glyph_width = 0;
    egui_dim_t glyph_height = 0;
    uint32_t utf8_code = 0;

    if (out_bytes != NULL)
    {
        *out_bytes = 0;
    }
    if (out_width != NULL)
    {
        *out_width = 0;
    }
    if (font == NULL || text == NULL || text[0] == '\0')
    {
        return;
    }

    glyph_bytes = egui_font_get_utf8_code_fast(text, &utf8_code);
    if (glyph_bytes <= 0)
    {
        glyph_bytes = 1;
    }
    if (glyph_bytes >= (int)sizeof(glyph_buf))
    {
        glyph_bytes = (int)sizeof(glyph_buf) - 1;
    }

    egui_api_memcpy(glyph_buf, text, glyph_bytes);
    glyph_buf[glyph_bytes] = '\0';
    font->api->get_str_size(font, glyph_buf, 0, 0, &glyph_width, &glyph_height);

    if (out_bytes != NULL)
    {
        *out_bytes = glyph_bytes;
    }
    if (out_width != NULL)
    {
        *out_width = glyph_width;
    }
}

static const char *egui_view_rich_text_block_get_next_line(const egui_font_t *font, const char *text, egui_dim_t max_width, int *out_line_len,
                                                           egui_dim_t *out_line_width)
{
    const char *cursor = text;
    egui_dim_t line_width = 0;

    if (out_line_len != NULL)
    {
        *out_line_len = 0;
    }
    if (out_line_width != NULL)
    {
        *out_line_width = 0;
    }
    if (font == NULL || text == NULL)
    {
        return NULL;
    }

    while (*cursor != '\0')
    {
        int glyph_bytes = 0;
        egui_dim_t glyph_width = 0;

        if (*cursor == '\r')
        {
            cursor++;
            continue;
        }
        if (*cursor == '\n')
        {
            break;
        }

        egui_view_rich_text_block_get_char_metrics(font, cursor, &glyph_bytes, &glyph_width);
        if (glyph_bytes <= 0)
        {
            break;
        }

        if (max_width > 0 && line_width > 0 && (line_width + glyph_width) > max_width)
        {
            break;
        }

        line_width += glyph_width;
        cursor += glyph_bytes;
    }

    if (out_line_len != NULL)
    {
        *out_line_len = (int)(cursor - text);
    }
    if (out_line_width != NULL)
    {
        *out_line_width = line_width;
    }

    if (*cursor == '\n')
    {
        return cursor + 1;
    }
    if (*cursor == '\0')
    {
        return NULL;
    }
    return cursor;
}

static void egui_view_rich_text_block_measure_wrapped_text(const egui_font_t *font, const char *text, egui_dim_t width, egui_dim_t line_space, egui_dim_t *out_width,
                                                           egui_dim_t *out_height)
{
    const char *cursor = text;
    egui_dim_t line_height = egui_view_rich_text_block_get_line_height(font);
    egui_dim_t line_count = 0;
    egui_dim_t max_line_width = 0;
    size_t text_len = 0;

    if (out_width != NULL)
    {
        *out_width = 0;
    }
    if (out_height != NULL)
    {
        *out_height = 0;
    }
    if (font == NULL || text == NULL || text[0] == '\0' || width <= 0)
    {
        return;
    }

    text_len = strlen(text);
    while (cursor != NULL && *cursor != '\0')
    {
        int line_len = 0;
        egui_dim_t line_width = 0;
        const char *next = egui_view_rich_text_block_get_next_line(font, cursor, width, &line_len, &line_width);

        if (line_width > max_line_width)
        {
            max_line_width = line_width;
        }
        line_count++;

        if (next == cursor)
        {
            break;
        }
        cursor = next;
    }

    if (cursor != NULL && *cursor == '\0' && text_len > 0 && text[text_len - 1] == '\n')
    {
        line_count++;
    }

    if (out_width != NULL)
    {
        *out_width = max_line_width;
    }
    if (out_height != NULL && line_count > 0)
    {
        *out_height = (egui_dim_t)(line_count * line_height + (line_count - 1) * line_space);
    }
}

static void egui_view_rich_text_block_draw_line(const egui_font_t *font, const char *text, int text_len, egui_dim_t x, egui_dim_t y, egui_color_t color,
                                                egui_alpha_t alpha)
{
    int offset = 0;

    while (offset < text_len)
    {
        char glyph_buf[8];
        int glyph_bytes = 0;
        egui_dim_t glyph_width = 0;

        if (text[offset] == '\r' || text[offset] == '\n')
        {
            offset++;
            continue;
        }

        egui_view_rich_text_block_get_char_metrics(font, &text[offset], &glyph_bytes, &glyph_width);
        if (glyph_bytes <= 0)
        {
            break;
        }
        if (glyph_bytes >= (int)sizeof(glyph_buf))
        {
            glyph_bytes = (int)sizeof(glyph_buf) - 1;
        }

        egui_api_memcpy(glyph_buf, &text[offset], glyph_bytes);
        glyph_buf[glyph_bytes] = '\0';
        egui_canvas_draw_text(font, glyph_buf, x, y, color, alpha);

        x += glyph_width;
        offset += glyph_bytes;
    }
}

static void egui_view_rich_text_block_draw_wrapped_text(const egui_font_t *font, const char *text, const egui_region_t *region, egui_dim_t line_space,
                                                        egui_color_t color, egui_alpha_t alpha)
{
    const char *cursor = text;
    egui_dim_t line_height = egui_view_rich_text_block_get_line_height(font);
    egui_dim_t y = region->location.y;

    if (font == NULL || text == NULL || text[0] == '\0')
    {
        return;
    }

    while (cursor != NULL && *cursor != '\0')
    {
        int line_len = 0;
        egui_dim_t line_width = 0;
        const char *next = egui_view_rich_text_block_get_next_line(font, cursor, region->size.width, &line_len, &line_width);

        if ((y + line_height) > (region->location.y + region->size.height))
        {
            break;
        }

        egui_view_rich_text_block_draw_line(font, cursor, line_len, region->location.x, y, color, alpha);
        y += line_height + line_space;

        if (next == cursor)
        {
            break;
        }
        cursor = next;
    }
}

static void egui_view_rich_text_block_get_work_region(egui_view_t *self, egui_region_t *work_region)
{
    egui_view_get_work_region(self, work_region);
    if (work_region->size.width < 0)
    {
        work_region->size.width = 0;
    }
    if (work_region->size.height < 0)
    {
        work_region->size.height = 0;
    }
}

static void egui_view_rich_text_block_prepare_layout(egui_view_rich_text_block_t *local, egui_view_t *self, const egui_view_rich_text_block_paragraph_t *paragraph,
                                                     egui_dim_t y, egui_view_rich_text_block_paragraph_layout_t *layout)
{
    egui_region_t work_region;
    const egui_font_t *font;
    egui_dim_t text_width;
    egui_dim_t text_height = 0;
    egui_dim_t text_x;
    egui_dim_t text_y;
    egui_dim_t accent_inset = 0;
    uint8_t style;

    egui_view_rich_text_block_get_work_region(self, &work_region);

    style = egui_view_rich_text_block_clamp_style(paragraph == NULL ? EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY : paragraph->style);
    font = egui_view_rich_text_block_resolve_font(local, style);
    text_width = work_region.size.width;

    layout->box_region.location.x = 0;
    layout->box_region.location.y = y;
    layout->box_region.size.width = 0;
    layout->box_region.size.height = 0;
    layout->strip_region.location.x = 0;
    layout->strip_region.location.y = y;
    layout->strip_region.size.width = 0;
    layout->strip_region.size.height = 0;
    layout->text_region.location.x = work_region.location.x;
    layout->text_region.location.y = y;
    layout->text_region.size.width = text_width;
    layout->text_region.size.height = 0;
    layout->font = font;
    layout->text_color = egui_view_rich_text_block_resolve_color(local, style);
    layout->style = style;
    layout->has_box = 0;

    if (style == EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_ACCENT && work_region.size.width > 14)
    {
        accent_inset = local->compact_mode ? 10 : 14;
        if (accent_inset >= work_region.size.width)
        {
            accent_inset = 0;
        }
        layout->has_box = 1;
        layout->box_region.location.x = work_region.location.x;
        layout->box_region.size.width = work_region.size.width;
        layout->strip_region.location.x = work_region.location.x + (local->compact_mode ? 4 : 5);
        layout->strip_region.location.y = y + 4;
        layout->strip_region.size.width = local->compact_mode ? 2 : 3;
    }

    text_x = work_region.location.x + accent_inset;
    text_y = y + (layout->has_box ? 5 : 0);
    text_width = work_region.size.width - accent_inset;
    if (text_width < 0)
    {
        text_width = 0;
    }

    egui_view_rich_text_block_measure_wrapped_text(font, paragraph == NULL ? NULL : paragraph->text, text_width, local->line_space, NULL, &text_height);

    if (text_height <= 0 && font != NULL)
    {
        egui_dim_t dummy_w = 0;
        font->api->get_str_size(font, "A", 0, 0, &dummy_w, &text_height);
    }

    layout->text_region.location.x = text_x;
    layout->text_region.location.y = text_y;
    layout->text_region.size.width = text_width;
    layout->text_region.size.height = text_height;

    if (layout->has_box)
    {
        layout->box_region.size.height = text_height + 10;
        layout->strip_region.size.height = layout->box_region.size.height - 8;
    }
}

static egui_dim_t egui_view_rich_text_block_measure_content_height(egui_view_rich_text_block_t *local, egui_view_t *self)
{
    egui_dim_t height = 0;
    uint8_t i;

    for (i = 0; i < local->paragraph_count; ++i)
    {
        egui_view_rich_text_block_paragraph_layout_t layout;

        egui_view_rich_text_block_prepare_layout(local, self, &local->paragraphs[i], height, &layout);
        height += layout.has_box ? layout.box_region.size.height : layout.text_region.size.height;
        if ((i + 1) < local->paragraph_count)
        {
            height += local->paragraph_gap;
        }
    }

    return height;
}

static void egui_view_rich_text_block_draw_accent_box(egui_view_rich_text_block_t *local, egui_view_t *self, const egui_view_rich_text_block_paragraph_layout_t *layout)
{
    egui_color_t box_fill = egui_rgb_mix(local->surface_color, local->accent_color, local->compact_mode ? 10 : 14);
    egui_color_t box_border = egui_rgb_mix(local->border_color, local->accent_color, 28);
    egui_color_t strip_color = local->accent_color;

    if (local->read_only_mode)
    {
        box_fill = egui_rgb_mix(box_fill, local->surface_color, 36);
        box_border = egui_rgb_mix(box_border, local->muted_text_color, 34);
        strip_color = egui_rgb_mix(strip_color, local->muted_text_color, 30);
    }
    if (!egui_view_get_enable(self))
    {
        box_fill = egui_view_rich_text_block_mix_disabled(box_fill);
        box_border = egui_view_rich_text_block_mix_disabled(box_border);
        strip_color = egui_view_rich_text_block_mix_disabled(strip_color);
    }

    egui_canvas_draw_round_rectangle_fill(layout->box_region.location.x, layout->box_region.location.y, layout->box_region.size.width, layout->box_region.size.height,
                                          local->compact_mode ? 6 : 8, box_fill, egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(layout->box_region.location.x, layout->box_region.location.y, layout->box_region.size.width, layout->box_region.size.height,
                                     local->compact_mode ? 6 : 8, 1, box_border, egui_color_alpha_mix(self->alpha, 42));
    egui_canvas_draw_round_rectangle_fill(layout->strip_region.location.x, layout->strip_region.location.y, layout->strip_region.size.width,
                                          layout->strip_region.size.height, 1, strip_color, egui_color_alpha_mix(self->alpha, 92));
}

static void egui_view_rich_text_block_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_rich_text_block_t);
    egui_dim_t y = 0;
    uint8_t i;

    if (local->paragraphs == NULL || local->paragraph_count == 0 || local->font == NULL)
    {
        return;
    }
    if (egui_view_rich_text_block_measure_content_height(local, self) <= 0)
    {
        return;
    }

    for (i = 0; i < local->paragraph_count; ++i)
    {
        egui_view_rich_text_block_paragraph_layout_t layout;
        egui_color_t text_color;

        egui_view_rich_text_block_prepare_layout(local, self, &local->paragraphs[i], y, &layout);
        text_color = layout.text_color;

        if (local->read_only_mode)
        {
            text_color = egui_rgb_mix(text_color, local->muted_text_color, layout.style == EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION ? 12 : 28);
        }
        if (!egui_view_get_enable(self))
        {
            text_color = egui_view_rich_text_block_mix_disabled(text_color);
        }

        if (layout.has_box)
        {
            egui_view_rich_text_block_draw_accent_box(local, self, &layout);
        }

        egui_view_rich_text_block_draw_wrapped_text(layout.font, local->paragraphs[i].text, &layout.text_region, local->line_space, text_color,
                                                    egui_color_alpha_mix(self->alpha, 100));

        y += layout.has_box ? layout.box_region.size.height : layout.text_region.size.height;
        if ((i + 1) < local->paragraph_count)
        {
            y += local->paragraph_gap;
        }
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_rich_text_block_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_rich_text_block_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_rich_text_block_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_rich_text_block_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_rich_text_block_set_paragraphs(egui_view_t *self, const egui_view_rich_text_block_paragraph_t *paragraphs, uint8_t paragraph_count)
{
    EGUI_LOCAL_INIT(egui_view_rich_text_block_t);
    egui_view_rich_text_block_clear_pressed_state(self);
    local->paragraphs = paragraphs;
    local->paragraph_count = paragraphs == NULL ? 0 : egui_view_rich_text_block_clamp_count(paragraph_count);
    egui_view_invalidate(self);
}

uint8_t egui_view_rich_text_block_get_paragraph_count(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_rich_text_block_t);
    return local->paragraph_count;
}

void egui_view_rich_text_block_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_rich_text_block_t);
    egui_view_rich_text_block_clear_pressed_state(self);
    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_rich_text_block_set_emphasis_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_rich_text_block_t);
    egui_view_rich_text_block_clear_pressed_state(self);
    local->emphasis_font = font;
    egui_view_invalidate(self);
}

void egui_view_rich_text_block_set_caption_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_rich_text_block_t);
    egui_view_rich_text_block_clear_pressed_state(self);
    local->caption_font = font;
    egui_view_invalidate(self);
}

void egui_view_rich_text_block_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_rich_text_block_t);
    egui_view_rich_text_block_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    local->paragraph_gap = local->compact_mode ? 4 : 6;
    egui_view_invalidate(self);
}

uint8_t egui_view_rich_text_block_get_compact_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_rich_text_block_t);
    return local->compact_mode;
}

void egui_view_rich_text_block_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_rich_text_block_t);
    egui_view_rich_text_block_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_rich_text_block_get_read_only_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_rich_text_block_t);
    return local->read_only_mode;
}

void egui_view_rich_text_block_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                           egui_color_t muted_text_color, egui_color_t accent_color)
{
    EGUI_LOCAL_INIT(egui_view_rich_text_block_t);
    egui_view_rich_text_block_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_rich_text_block_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_rich_text_block_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_rich_text_block_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_rich_text_block_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
        .on_touch_event = egui_view_on_touch_event,
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_rich_text_block_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_rich_text_block_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_rich_text_block_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_rich_text_block_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->paragraphs = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->emphasis_font = NULL;
    local->caption_font = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xDCE3E8);
    local->text_color = EGUI_COLOR_HEX(0x1E2A36);
    local->muted_text_color = EGUI_COLOR_HEX(0x627181);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->line_space = 2;
    local->paragraph_gap = 6;
    local->paragraph_count = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;

    egui_view_set_view_name(self, "egui_view_rich_text_block");
}
