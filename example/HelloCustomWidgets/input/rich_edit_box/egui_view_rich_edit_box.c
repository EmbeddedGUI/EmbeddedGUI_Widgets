#include <stdio.h>
#include <string.h>

#include "egui_view_rich_edit_box.h"

#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_RADIUS           10
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PAD_X            10
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PAD_Y            8
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_BADGE_H          9
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_BADGE_GAP        4
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_TITLE_H          12
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_SUMMARY_H        10
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_TITLE_GAP        2
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_EDITOR_GAP       6
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_EDITOR_PAD_X     8
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_EDITOR_PAD_Y     7
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PRESET_H         19
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PRESET_GAP       4
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_FOOTER_H         10
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_FOOTER_GAP       5
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PILL_BASE_W      16
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PILL_CHAR_W      5
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PILL_MIN_W       32
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PILL_MAX_W       56
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_LINE_SPACE       3
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_CURSOR_WIDTH     1
#define EGUI_VIEW_RICH_EDIT_BOX_STANDARD_CHECKLIST_INDENT 10

#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_RADIUS           8
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PAD_X            6
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PAD_Y            6
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_BADGE_H          7
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_BADGE_GAP        3
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_TITLE_H          10
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_SUMMARY_H        0
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_TITLE_GAP        0
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_EDITOR_GAP       4
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_EDITOR_PAD_X     5
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_EDITOR_PAD_Y     5
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PRESET_H         14
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PRESET_GAP       3
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_FOOTER_H         0
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_FOOTER_GAP       0
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PILL_BASE_W      12
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PILL_CHAR_W      4
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PILL_MIN_W       22
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PILL_MAX_W       40
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_LINE_SPACE       2
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_CURSOR_WIDTH     1
#define EGUI_VIEW_RICH_EDIT_BOX_COMPACT_CHECKLIST_INDENT 8

typedef struct egui_view_rich_edit_box_metrics egui_view_rich_edit_box_metrics_t;
struct egui_view_rich_edit_box_metrics
{
    egui_region_t content_region;
    egui_region_t badge_region;
    egui_region_t title_region;
    egui_region_t summary_region;
    egui_region_t editor_region;
    egui_region_t editor_text_region;
    egui_region_t footer_region;
    egui_region_t preset_regions[EGUI_VIEW_RICH_EDIT_BOX_MAX_PRESETS];
    uint8_t preset_count;
    uint8_t show_summary;
    uint8_t show_footer;
};

typedef struct egui_view_rich_edit_box_text_result egui_view_rich_edit_box_text_result_t;
struct egui_view_rich_edit_box_text_result
{
    egui_dim_t cursor_x;
    egui_dim_t cursor_y;
    egui_dim_t cursor_height;
};

static egui_view_rich_edit_box_t *rich_edit_box_local(egui_view_t *self)
{
    return (egui_view_rich_edit_box_t *)self;
}

static uint8_t rich_edit_box_has_text(const char *text)
{
    return (text != NULL && text[0] != '\0') ? 1 : 0;
}

static uint8_t rich_edit_box_text_len(const char *text)
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

static void rich_edit_box_copy_text(char *dst, uint8_t capacity, const char *src)
{
    size_t len;

    if (dst == NULL || capacity == 0)
    {
        return;
    }
    if (src == NULL)
    {
        dst[0] = '\0';
        return;
    }

    len = strlen(src);
    if (len >= capacity)
    {
        len = capacity - 1;
    }
    if (len > 0)
    {
        memcpy(dst, src, len);
    }
    dst[len] = '\0';
}

static uint8_t rich_edit_box_clamp_document_count(uint8_t count)
{
    return count > EGUI_VIEW_RICH_EDIT_BOX_MAX_DOCUMENTS ? EGUI_VIEW_RICH_EDIT_BOX_MAX_DOCUMENTS : count;
}

static uint8_t rich_edit_box_clamp_preset_count(uint8_t count)
{
    return count > EGUI_VIEW_RICH_EDIT_BOX_MAX_PRESETS ? EGUI_VIEW_RICH_EDIT_BOX_MAX_PRESETS : count;
}

static uint8_t rich_edit_box_clamp_style(uint8_t style)
{
    switch (style)
    {
    case EGUI_VIEW_RICH_EDIT_BOX_STYLE_CALLOUT:
    case EGUI_VIEW_RICH_EDIT_BOX_STYLE_CHECKLIST:
        return style;
    case EGUI_VIEW_RICH_EDIT_BOX_STYLE_BODY:
    default:
        return EGUI_VIEW_RICH_EDIT_BOX_STYLE_BODY;
    }
}

static egui_color_t rich_edit_box_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static const egui_view_rich_edit_box_document_t *rich_edit_box_get_document(egui_view_rich_edit_box_t *local)
{
    if (local->documents == NULL || local->document_count == 0 || local->current_document >= local->document_count)
    {
        return NULL;
    }
    return &local->documents[local->current_document];
}

static uint8_t rich_edit_box_get_preset_count(const egui_view_rich_edit_box_document_t *document)
{
    if (document == NULL || document->presets == NULL)
    {
        return 0;
    }
    return rich_edit_box_clamp_preset_count(document->preset_count);
}

static const egui_view_rich_edit_box_preset_t *rich_edit_box_get_preset(const egui_view_rich_edit_box_document_t *document, uint8_t preset_index)
{
    if (preset_index >= rich_edit_box_get_preset_count(document))
    {
        return NULL;
    }
    return &document->presets[preset_index];
}

static uint8_t rich_edit_box_default_preset(const egui_view_rich_edit_box_document_t *document)
{
    uint8_t preset_count = rich_edit_box_get_preset_count(document);

    if (preset_count == 0)
    {
        return 0;
    }
    return document->selected_preset < preset_count ? document->selected_preset : 0;
}

static uint8_t rich_edit_box_preset_to_part(uint8_t preset_index)
{
    return (uint8_t)(EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_0 + preset_index);
}

static uint8_t rich_edit_box_part_to_preset(uint8_t part)
{
    if (part < EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_0 || part > EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_2)
    {
        return EGUI_VIEW_RICH_EDIT_BOX_MAX_PRESETS;
    }
    return (uint8_t)(part - EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_0);
}

static uint8_t rich_edit_box_part_exists(const egui_view_rich_edit_box_document_t *document, uint8_t part)
{
    if (part == EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR)
    {
        return 1;
    }
    return rich_edit_box_part_to_preset(part) < rich_edit_box_get_preset_count(document) ? 1 : 0;
}

static uint8_t rich_edit_box_clear_pressed_state(egui_view_t *self, egui_view_rich_edit_box_t *local)
{
    uint8_t was_pressed = self->is_pressed ? 1 : 0;
    uint8_t had_pressed = (uint8_t)(was_pressed || local->pressed_part != EGUI_VIEW_RICH_EDIT_BOX_PART_NONE);

    if (!had_pressed)
    {
        return 0;
    }

    local->pressed_part = EGUI_VIEW_RICH_EDIT_BOX_PART_NONE;
    if (was_pressed)
    {
        egui_view_set_pressed(self, 0);
    }
    else
    {
        egui_view_invalidate(self);
    }
    return 1;
}

static void rich_edit_box_sync_state(egui_view_rich_edit_box_t *local)
{
    const egui_view_rich_edit_box_document_t *document = rich_edit_box_get_document(local);
    uint8_t preset_count = rich_edit_box_get_preset_count(document);

    if (document == NULL)
    {
        local->current_part = EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR;
        local->current_preset = 0;
        local->applied_preset = 0;
        return;
    }

    if (preset_count == 0)
    {
        local->current_preset = 0;
        local->applied_preset = 0;
    }
    else
    {
        if (local->current_preset >= preset_count)
        {
            local->current_preset = rich_edit_box_default_preset(document);
        }
        if (local->applied_preset >= preset_count)
        {
            local->applied_preset = local->current_preset;
        }
    }

    if (!rich_edit_box_part_exists(document, local->current_part))
    {
        local->current_part = EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR;
    }
    if (!rich_edit_box_part_exists(document, local->pressed_part))
    {
        local->pressed_part = EGUI_VIEW_RICH_EDIT_BOX_PART_NONE;
    }
}

static void rich_edit_box_load_current_document(egui_view_rich_edit_box_t *local)
{
    const egui_view_rich_edit_box_document_t *document = rich_edit_box_get_document(local);

    if (document == NULL)
    {
        local->editor_text[0] = '\0';
        local->text_len = 0;
        local->current_part = EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR;
        local->current_preset = 0;
        local->applied_preset = 0;
        return;
    }

    rich_edit_box_copy_text(local->editor_text, sizeof(local->editor_text), document->draft_text);
    local->text_len = rich_edit_box_text_len(local->editor_text);
    local->current_part = EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR;
    local->current_preset = rich_edit_box_default_preset(document);
    local->applied_preset = local->current_preset;
    rich_edit_box_sync_state(local);
}

static void rich_edit_box_set_current_document_inner(egui_view_t *self, uint8_t document_index)
{
    egui_view_rich_edit_box_t *local = rich_edit_box_local(self);

    if (local->documents == NULL || local->document_count == 0)
    {
        local->current_document = 0;
        rich_edit_box_load_current_document(local);
        egui_view_invalidate(self);
        return;
    }

    if (document_index >= local->document_count)
    {
        document_index = 0;
    }
    local->current_document = document_index;
    rich_edit_box_load_current_document(local);
    egui_view_invalidate(self);
}

static void rich_edit_box_select_preset_inner(egui_view_t *self, uint8_t preset_index, uint8_t apply_preset)
{
    egui_view_rich_edit_box_t *local = rich_edit_box_local(self);
    const egui_view_rich_edit_box_document_t *document = rich_edit_box_get_document(local);
    uint8_t preset_count = rich_edit_box_get_preset_count(document);

    if (preset_count == 0)
    {
        local->current_preset = 0;
        local->applied_preset = 0;
        egui_view_invalidate(self);
        return;
    }

    if (preset_index >= preset_count)
    {
        preset_index = rich_edit_box_default_preset(document);
    }
    local->current_preset = preset_index;
    local->current_part = rich_edit_box_preset_to_part(preset_index);
    if (apply_preset)
    {
        local->applied_preset = preset_index;
    }
    egui_view_invalidate(self);
}

static uint8_t rich_edit_box_active_style(egui_view_rich_edit_box_t *local)
{
    const egui_view_rich_edit_box_document_t *document = rich_edit_box_get_document(local);
    const egui_view_rich_edit_box_preset_t *preset = rich_edit_box_get_preset(document, local->applied_preset);

    if (preset == NULL)
    {
        return EGUI_VIEW_RICH_EDIT_BOX_STYLE_BODY;
    }
    return rich_edit_box_clamp_style(preset->style);
}

static void rich_edit_box_focus_editor(egui_view_rich_edit_box_t *local)
{
    local->current_part = EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR;
}

static uint8_t rich_edit_box_append_char(egui_view_t *self, char ch)
{
    egui_view_rich_edit_box_t *local = rich_edit_box_local(self);

    if (local->text_len >= EGUI_CONFIG_TEXTINPUT_MAX_LENGTH)
    {
        return 1;
    }

    rich_edit_box_focus_editor(local);
    local->editor_text[local->text_len++] = ch;
    local->editor_text[local->text_len] = '\0';
    egui_view_invalidate(self);
    return 1;
}

static uint8_t rich_edit_box_pop_char(egui_view_t *self)
{
    egui_view_rich_edit_box_t *local = rich_edit_box_local(self);

    rich_edit_box_focus_editor(local);
    if (local->text_len == 0)
    {
        return 1;
    }

    local->text_len--;
    local->editor_text[local->text_len] = '\0';
    egui_view_invalidate(self);
    return 1;
}

static egui_dim_t rich_edit_box_get_line_height(const egui_font_t *font)
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

static void rich_edit_box_get_char_metrics(const egui_font_t *font, const char *text, int *out_bytes, egui_dim_t *out_width)
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

    memcpy(glyph_buf, text, glyph_bytes);
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

static const char *rich_edit_box_get_next_line(const egui_font_t *font, const char *text, egui_dim_t max_width, int *out_line_len,
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

        rich_edit_box_get_char_metrics(font, cursor, &glyph_bytes, &glyph_width);
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

static void rich_edit_box_draw_focus_ring(egui_view_t *self, const egui_region_t *region, egui_dim_t radius, egui_color_t color)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (!self->is_focused)
    {
        return;
    }
    egui_canvas_draw_round_rectangle(region->location.x - 2, region->location.y - 2, region->size.width + 4, region->size.height + 4, radius + 2, 1, color,
                                     self->alpha);
#else
    EGUI_UNUSED(self);
    EGUI_UNUSED(region);
    EGUI_UNUSED(radius);
    EGUI_UNUSED(color);
#endif
}

static void rich_edit_box_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align, egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !rich_edit_box_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static egui_dim_t rich_edit_box_measure_pill_width(egui_view_rich_edit_box_t *local, const char *text)
{
    egui_dim_t width = (egui_dim_t)(rich_edit_box_text_len(text) *
                                    (local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PILL_CHAR_W : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PILL_CHAR_W));

    width += local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PILL_BASE_W : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PILL_BASE_W;
    if (local->compact_mode)
    {
        if (width < EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PILL_MIN_W)
        {
            width = EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PILL_MIN_W;
        }
        if (width > EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PILL_MAX_W)
        {
            width = EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PILL_MAX_W;
        }
    }
    else
    {
        if (width < EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PILL_MIN_W)
        {
            width = EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PILL_MIN_W;
        }
        if (width > EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PILL_MAX_W)
        {
            width = EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PILL_MAX_W;
        }
    }
    return width;
}

static void rich_edit_box_get_metrics(egui_view_rich_edit_box_t *local, egui_view_t *self, egui_view_rich_edit_box_metrics_t *metrics)
{
    const egui_view_rich_edit_box_document_t *document = rich_edit_box_get_document(local);
    egui_dim_t pad_x = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PAD_X : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PAD_Y : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PAD_Y;
    egui_dim_t badge_h = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_BADGE_H : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_BADGE_H;
    egui_dim_t badge_gap = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_BADGE_GAP : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_BADGE_GAP;
    egui_dim_t title_h = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_TITLE_H : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_TITLE_H;
    egui_dim_t summary_h = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_SUMMARY_H : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_SUMMARY_H;
    egui_dim_t title_gap = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_TITLE_GAP : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_TITLE_GAP;
    egui_dim_t editor_gap = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_EDITOR_GAP : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_EDITOR_GAP;
    egui_dim_t editor_pad_x = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_EDITOR_PAD_X : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_EDITOR_PAD_X;
    egui_dim_t editor_pad_y = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_EDITOR_PAD_Y : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_EDITOR_PAD_Y;
    egui_dim_t preset_h = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PRESET_H : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PRESET_H;
    egui_dim_t preset_gap = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_PRESET_GAP : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_PRESET_GAP;
    egui_dim_t footer_h = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_FOOTER_H : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_FOOTER_H;
    egui_dim_t footer_gap = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_FOOTER_GAP : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_FOOTER_GAP;
    egui_dim_t inner_x;
    egui_dim_t inner_y;
    egui_dim_t inner_w;
    egui_dim_t bottom_y;
    egui_dim_t total_preset_w = 0;
    egui_dim_t preset_x;
    uint8_t i;

    *metrics = (egui_view_rich_edit_box_metrics_t){0};
    egui_view_get_work_region(self, &metrics->content_region);
    if (metrics->content_region.size.width <= 0 || metrics->content_region.size.height <= 0)
    {
        return;
    }

    metrics->preset_count = rich_edit_box_get_preset_count(document);
    metrics->show_summary = (!local->compact_mode && document != NULL && rich_edit_box_has_text(document->summary)) ? 1 : 0;
    metrics->show_footer = (!local->compact_mode && document != NULL && rich_edit_box_has_text(document->footer)) ? 1 : 0;

    inner_x = metrics->content_region.location.x + pad_x;
    inner_y = metrics->content_region.location.y + pad_y;
    inner_w = metrics->content_region.size.width > pad_x * 2 ? (egui_dim_t)(metrics->content_region.size.width - pad_x * 2) : 0;
    if (inner_w <= 0)
    {
        return;
    }

    if (document != NULL && rich_edit_box_has_text(document->header))
    {
        metrics->badge_region.location.x = inner_x;
        metrics->badge_region.location.y = inner_y;
        metrics->badge_region.size.height = badge_h;
        metrics->badge_region.size.width = rich_edit_box_measure_pill_width(local, document->header);
        inner_y += badge_h + badge_gap;
    }

    metrics->title_region.location.x = inner_x;
    metrics->title_region.location.y = inner_y;
    metrics->title_region.size.width = inner_w;
    metrics->title_region.size.height = title_h;
    inner_y += title_h;

    if (metrics->show_summary)
    {
        inner_y += title_gap;
        metrics->summary_region.location.x = inner_x;
        metrics->summary_region.location.y = inner_y;
        metrics->summary_region.size.width = inner_w;
        metrics->summary_region.size.height = summary_h;
        inner_y += summary_h;
    }

    inner_y += editor_gap;
    bottom_y = metrics->content_region.location.y + metrics->content_region.size.height - pad_y;
    if (metrics->preset_count > 0)
    {
        bottom_y -= preset_h;
    }
    if (metrics->show_footer)
    {
        bottom_y -= footer_gap;
        metrics->footer_region.location.x = inner_x;
        metrics->footer_region.location.y = bottom_y - footer_h;
        metrics->footer_region.size.width = inner_w;
        metrics->footer_region.size.height = footer_h;
        bottom_y -= footer_h;
    }
    bottom_y -= editor_gap;

    metrics->editor_region.location.x = inner_x;
    metrics->editor_region.location.y = inner_y;
    metrics->editor_region.size.width = inner_w;
    metrics->editor_region.size.height = bottom_y > inner_y ? (egui_dim_t)(bottom_y - inner_y) : 24;
    metrics->editor_text_region.location.x = metrics->editor_region.location.x + editor_pad_x;
    metrics->editor_text_region.location.y = metrics->editor_region.location.y + editor_pad_y;
    metrics->editor_text_region.size.width =
            metrics->editor_region.size.width > editor_pad_x * 2 ? (egui_dim_t)(metrics->editor_region.size.width - editor_pad_x * 2) : 0;
    metrics->editor_text_region.size.height =
            metrics->editor_region.size.height > editor_pad_y * 2 ? (egui_dim_t)(metrics->editor_region.size.height - editor_pad_y * 2) : 0;

    if (metrics->preset_count == 0)
    {
        return;
    }

    for (i = 0; i < metrics->preset_count; ++i)
    {
        total_preset_w += rich_edit_box_measure_pill_width(local, document->presets[i].label);
    }
    total_preset_w += (egui_dim_t)((metrics->preset_count - 1) * preset_gap);
    if (total_preset_w > inner_w)
    {
        egui_dim_t equal_w = (egui_dim_t)((inner_w - (metrics->preset_count - 1) * preset_gap) / metrics->preset_count);

        preset_x = inner_x;
        for (i = 0; i < metrics->preset_count; ++i)
        {
            metrics->preset_regions[i].location.x = preset_x;
            metrics->preset_regions[i].location.y = metrics->content_region.location.y + metrics->content_region.size.height - pad_y - preset_h;
            metrics->preset_regions[i].size.width = equal_w;
            metrics->preset_regions[i].size.height = preset_h;
            preset_x += equal_w + preset_gap;
        }
        return;
    }

    preset_x = inner_x + (inner_w - total_preset_w) / 2;
    for (i = 0; i < metrics->preset_count; ++i)
    {
        egui_dim_t pill_w = rich_edit_box_measure_pill_width(local, document->presets[i].label);

        metrics->preset_regions[i].location.x = preset_x;
        metrics->preset_regions[i].location.y = metrics->content_region.location.y + metrics->content_region.size.height - pad_y - preset_h;
        metrics->preset_regions[i].size.width = pill_w;
        metrics->preset_regions[i].size.height = preset_h;
        preset_x += pill_w + preset_gap;
    }
}

static void rich_edit_box_draw_wrapped_text(egui_view_rich_edit_box_t *local, egui_view_t *self, const egui_region_t *region, const char *text, uint8_t style,
                                            egui_color_t color, uint8_t show_cursor, egui_view_rich_edit_box_text_result_t *result)
{
    const egui_font_t *font = local->font;
    const char *cursor = text;
    egui_dim_t line_height = rich_edit_box_get_line_height(font);
    egui_dim_t line_space = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_LINE_SPACE : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_LINE_SPACE;
    egui_dim_t checklist_indent = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_CHECKLIST_INDENT : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_CHECKLIST_INDENT;
    egui_dim_t max_width = region->size.width;
    egui_dim_t draw_x = region->location.x;
    egui_dim_t draw_y = region->location.y;
    uint8_t is_checklist = style == EGUI_VIEW_RICH_EDIT_BOX_STYLE_CHECKLIST ? 1 : 0;

    if (result != NULL)
    {
        result->cursor_x = region->location.x;
        result->cursor_y = region->location.y;
        result->cursor_height = line_height;
    }
    if (font == NULL || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    if (is_checklist)
    {
        max_width = region->size.width > checklist_indent ? (egui_dim_t)(region->size.width - checklist_indent) : region->size.width;
    }
    if (!rich_edit_box_has_text(text))
    {
        if (show_cursor && result != NULL)
        {
            result->cursor_x = is_checklist ? (egui_dim_t)(region->location.x + checklist_indent) : region->location.x;
            result->cursor_y = region->location.y;
            result->cursor_height = line_height;
        }
        return;
    }

    while (cursor != NULL && *cursor != '\0' && draw_y + line_height <= region->location.y + region->size.height)
    {
        int line_len = 0;
        egui_dim_t line_width = 0;
        const char *next = rich_edit_box_get_next_line(font, cursor, max_width, &line_len, &line_width);
        char line_buf[EGUI_CONFIG_TEXTINPUT_MAX_LENGTH + 1];
        egui_region_t line_region;

        if (line_len > EGUI_CONFIG_TEXTINPUT_MAX_LENGTH)
        {
            line_len = EGUI_CONFIG_TEXTINPUT_MAX_LENGTH;
        }
        if (line_len > 0)
        {
            memcpy(line_buf, cursor, (size_t)line_len);
        }
        line_buf[line_len] = '\0';

        draw_x = region->location.x;
        if (is_checklist)
        {
            egui_dim_t bullet_size = local->compact_mode ? 3 : 4;
            egui_dim_t bullet_x = region->location.x;
            egui_dim_t bullet_y = draw_y + (line_height > bullet_size ? (line_height - bullet_size) / 2 : 0);

            egui_canvas_draw_round_rectangle_fill(bullet_x, bullet_y, bullet_size, bullet_size, bullet_size / 2, local->accent_color, self->alpha);
            draw_x += checklist_indent;
        }

        line_region.location.x = draw_x;
        line_region.location.y = draw_y;
        line_region.size.width = max_width;
        line_region.size.height = line_height;
        rich_edit_box_draw_text(font, self, line_buf, &line_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP, color);

        if (result != NULL)
        {
            result->cursor_x = draw_x + line_width;
            result->cursor_y = draw_y;
            result->cursor_height = line_height;
        }

        draw_y += line_height + line_space;
        if (next == cursor)
        {
            break;
        }
        cursor = next;
    }

    if (show_cursor && result != NULL && local->text_len > 0 && local->editor_text[local->text_len - 1] == '\n')
    {
        result->cursor_x = is_checklist ? (egui_dim_t)(region->location.x + checklist_indent) : region->location.x;
        result->cursor_y = draw_y;
        result->cursor_height = line_height;
    }
}

static void rich_edit_box_draw_editor(egui_view_rich_edit_box_t *local, egui_view_t *self, const egui_region_t *editor_region,
                                      const egui_region_t *text_region)
{
    uint8_t style = rich_edit_box_active_style(local);
    egui_color_t fill_color = local->editor_color;
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_RADIUS : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_RADIUS;
    egui_view_rich_edit_box_text_result_t text_result;

    if (style == EGUI_VIEW_RICH_EDIT_BOX_STYLE_CALLOUT)
    {
        fill_color = egui_rgb_mix(local->editor_color, local->accent_color, 16);
        border_color = egui_rgb_mix(local->border_color, local->accent_color, 18);
        text_color = egui_rgb_mix(local->text_color, local->accent_color, 12);
    }
    else if (style == EGUI_VIEW_RICH_EDIT_BOX_STYLE_CHECKLIST)
    {
        fill_color = egui_rgb_mix(local->surface_color, local->accent_color, 8);
        border_color = egui_rgb_mix(local->border_color, local->accent_color, 12);
    }

    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        fill_color = rich_edit_box_mix_disabled(fill_color);
        border_color = rich_edit_box_mix_disabled(border_color);
        text_color = rich_edit_box_mix_disabled(text_color);
    }

    egui_canvas_draw_round_rectangle_fill(editor_region->location.x, editor_region->location.y, editor_region->size.width, editor_region->size.height, radius,
                                          fill_color, self->alpha);
    egui_canvas_draw_round_rectangle(editor_region->location.x, editor_region->location.y, editor_region->size.width, editor_region->size.height, radius, 1,
                                     border_color, self->alpha);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (self->is_focused && self->is_enable && !local->read_only_mode && local->current_part == EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR)
    {
        rich_edit_box_draw_focus_ring(self, editor_region, radius, local->accent_color);
    }
#endif

    if (style == EGUI_VIEW_RICH_EDIT_BOX_STYLE_CALLOUT)
    {
        egui_dim_t strip_w = local->compact_mode ? 4 : 5;

        egui_canvas_draw_round_rectangle_fill(editor_region->location.x + 2, editor_region->location.y + 2, strip_w,
                                              editor_region->size.height > 4 ? (egui_dim_t)(editor_region->size.height - 4) : editor_region->size.height, strip_w / 2,
                                              local->accent_color, self->alpha);
    }

    if (!rich_edit_box_has_text(local->editor_text))
    {
        rich_edit_box_draw_text(local->font, self, "Start typing...", text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP, local->muted_text_color);
        return;
    }

    rich_edit_box_draw_wrapped_text(local, self, text_region, local->editor_text, style, text_color,
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
                                    (uint8_t)(self->is_focused && self->is_enable && !local->read_only_mode &&
                                              local->current_part == EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR),
#else
                                    0,
#endif
                                    &text_result);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (self->is_focused && self->is_enable && !local->read_only_mode && local->current_part == EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR)
    {
        egui_dim_t cursor_w = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_CURSOR_WIDTH : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_CURSOR_WIDTH;

        egui_canvas_draw_rectangle_fill(text_result.cursor_x, text_result.cursor_y, cursor_w, text_result.cursor_height, local->accent_color, self->alpha);
    }
#endif
}

static void rich_edit_box_draw_preset(egui_view_rich_edit_box_t *local, egui_view_t *self, const egui_region_t *region, const char *label, uint8_t preset_index)
{
    egui_color_t fill_color = egui_rgb_mix(local->surface_color, local->accent_color, 6);
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->muted_text_color;
    egui_dim_t radius = region->size.height / 2;
    uint8_t is_current = local->current_preset == preset_index ? 1 : 0;
    uint8_t is_applied = local->applied_preset == preset_index ? 1 : 0;
    uint8_t is_pressed = local->pressed_part == rich_edit_box_preset_to_part(preset_index) && self->is_pressed ? 1 : 0;

    if (is_applied)
    {
        fill_color = egui_rgb_mix(local->surface_color, local->accent_color, 18);
        border_color = local->accent_color;
        text_color = local->text_color;
    }
    else if (is_current)
    {
        fill_color = egui_rgb_mix(local->surface_color, local->accent_color, 11);
        border_color = egui_rgb_mix(local->border_color, local->accent_color, 18);
    }
    if (is_pressed)
    {
        fill_color = egui_rgb_mix(fill_color, local->accent_color, 22);
    }
    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        fill_color = rich_edit_box_mix_disabled(fill_color);
        border_color = rich_edit_box_mix_disabled(border_color);
        text_color = rich_edit_box_mix_disabled(text_color);
    }

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, radius, fill_color, self->alpha);
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, radius, 1, border_color, self->alpha);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (self->is_focused && self->is_enable && !local->read_only_mode && local->current_part == rich_edit_box_preset_to_part(preset_index))
    {
        rich_edit_box_draw_focus_ring(self, region, radius, local->accent_color);
    }
#endif

    rich_edit_box_draw_text(local->meta_font != NULL ? local->meta_font : local->font, self, label, region, EGUI_ALIGN_CENTER, text_color);
}

static uint8_t rich_edit_box_hit_part(egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_rich_edit_box_t *local = rich_edit_box_local(self);
    egui_view_rich_edit_box_metrics_t metrics;
    uint8_t preset_index;

    rich_edit_box_get_metrics(local, self, &metrics);
    for (preset_index = 0; preset_index < metrics.preset_count; ++preset_index)
    {
        if (egui_region_pt_in_rect(&metrics.preset_regions[preset_index], x, y))
        {
            return rich_edit_box_preset_to_part(preset_index);
        }
    }
    if (egui_region_pt_in_rect(&metrics.editor_region, x, y))
    {
        return EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR;
    }
    return EGUI_VIEW_RICH_EDIT_BOX_PART_NONE;
}

static uint8_t rich_edit_box_activate_current_inner(egui_view_t *self)
{
    egui_view_rich_edit_box_t *local = rich_edit_box_local(self);
    const egui_view_rich_edit_box_document_t *document = rich_edit_box_get_document(local);
    const egui_view_rich_edit_box_preset_t *preset = rich_edit_box_get_preset(document, local->current_preset);

    if (preset == NULL)
    {
        return 0;
    }

    local->applied_preset = local->current_preset;
    if (local->on_action != NULL)
    {
        local->on_action(self, local->current_document, local->current_preset, rich_edit_box_clamp_style(preset->style), local->text_len);
    }
    egui_view_invalidate(self);
    return 1;
}

static uint8_t rich_edit_box_cycle_part(egui_view_t *self)
{
    egui_view_rich_edit_box_t *local = rich_edit_box_local(self);
    const egui_view_rich_edit_box_document_t *document = rich_edit_box_get_document(local);
    uint8_t preset_count = rich_edit_box_get_preset_count(document);

    if (preset_count == 0)
    {
        local->current_part = EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR;
        egui_view_invalidate(self);
        return 1;
    }

    if (local->current_part == EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR)
    {
        local->current_preset = 0;
        local->current_part = EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_0;
    }
    else if (local->current_preset + 1 < preset_count)
    {
        local->current_preset++;
        local->current_part = rich_edit_box_preset_to_part(local->current_preset);
    }
    else
    {
        local->current_part = EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR;
    }
    egui_view_invalidate(self);
    return 1;
}

static uint8_t rich_edit_box_handle_key_up(egui_view_t *self, egui_key_event_t *event)
{
    egui_view_rich_edit_box_t *local = rich_edit_box_local(self);
    const egui_view_rich_edit_box_document_t *document = rich_edit_box_get_document(local);
    uint8_t preset_count = rich_edit_box_get_preset_count(document);
    char ch;

    if (preset_count > 0)
    {
        switch (event->key_code)
        {
        case EGUI_KEY_CODE_LEFT:
            if (local->current_preset > 0)
            {
                rich_edit_box_select_preset_inner(self, (uint8_t)(local->current_preset - 1), 0);
            }
            else
            {
                rich_edit_box_select_preset_inner(self, 0, 0);
            }
            return 1;
        case EGUI_KEY_CODE_RIGHT:
            if (local->current_preset + 1 < preset_count)
            {
                rich_edit_box_select_preset_inner(self, (uint8_t)(local->current_preset + 1), 0);
            }
            else
            {
                rich_edit_box_select_preset_inner(self, (uint8_t)(preset_count - 1), 0);
            }
            return 1;
        case EGUI_KEY_CODE_HOME:
            rich_edit_box_select_preset_inner(self, 0, 0);
            return 1;
        case EGUI_KEY_CODE_END:
            rich_edit_box_select_preset_inner(self, (uint8_t)(preset_count - 1), 0);
            return 1;
        default:
            break;
        }
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_TAB:
        return rich_edit_box_cycle_part(self);
    case EGUI_KEY_CODE_ENTER:
        if (local->current_part != EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR)
        {
            return rich_edit_box_activate_current_inner(self);
        }
        return rich_edit_box_append_char(self, '\n');
    case EGUI_KEY_CODE_SPACE:
        if (local->current_part != EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR)
        {
            return rich_edit_box_activate_current_inner(self);
        }
        return rich_edit_box_append_char(self, ' ');
    case EGUI_KEY_CODE_BACKSPACE:
    case EGUI_KEY_CODE_DELETE:
        return rich_edit_box_pop_char(self);
    default:
        ch = egui_key_event_to_char(event);
        if (ch != 0)
        {
            return rich_edit_box_append_char(self, ch);
        }
        return egui_view_on_key_event(self, event);
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_rich_edit_box_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);
    uint8_t hit_part;

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        rich_edit_box_clear_pressed_state(self, local);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        rich_edit_box_clear_pressed_state(self, local);
        hit_part = rich_edit_box_hit_part(self, event->location.x, event->location.y);
        if (hit_part == EGUI_VIEW_RICH_EDIT_BOX_PART_NONE)
        {
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        if (hit_part == EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR)
        {
            local->current_part = hit_part;
        }
        local->pressed_part = hit_part;
        egui_view_set_pressed(self, 1);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_part == EGUI_VIEW_RICH_EDIT_BOX_PART_NONE)
        {
            return 0;
        }
        hit_part = rich_edit_box_hit_part(self, event->location.x, event->location.y);
        if ((self->is_pressed ? 1 : 0) != (local->pressed_part == hit_part ? 1 : 0))
        {
            egui_view_set_pressed(self, local->pressed_part == hit_part ? 1 : 0);
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled = 0;
        uint8_t pressed_part = local->pressed_part;
        uint8_t was_pressed = self->is_pressed ? 1 : 0;

        hit_part = rich_edit_box_hit_part(self, event->location.x, event->location.y);
        if (pressed_part == EGUI_VIEW_RICH_EDIT_BOX_PART_NONE)
        {
            return rich_edit_box_clear_pressed_state(self, local) ? 1 : 0;
        }

        if (was_pressed && pressed_part == hit_part)
        {
            local->current_part = hit_part;
            if (hit_part == EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR)
            {
                handled = 1;
            }
            else
            {
                local->current_preset = rich_edit_box_part_to_preset(hit_part);
                handled = rich_edit_box_activate_current_inner(self);
            }
        }
        rich_edit_box_clear_pressed_state(self, local);
        if (hit_part != EGUI_VIEW_RICH_EDIT_BOX_PART_NONE)
        {
            egui_view_invalidate(self);
            return 1;
        }
        return (uint8_t)(handled || pressed_part != EGUI_VIEW_RICH_EDIT_BOX_PART_NONE);
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return rich_edit_box_clear_pressed_state(self, local);
    default:
        return 0;
    }
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_rich_edit_box_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);
    char ch;

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        rich_edit_box_clear_pressed_state(self, local);
        return 0;
    }

    rich_edit_box_sync_state(local);

    if ((event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE) &&
        local->current_part != EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->pressed_part = local->current_part;
            egui_view_set_pressed(self, 1);
            egui_view_invalidate(self);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            uint8_t handled = 0;

            if (local->pressed_part == local->current_part)
            {
                handled = rich_edit_box_activate_current_inner(self);
            }
            rich_edit_box_clear_pressed_state(self, local);
            return handled;
        }
        return 0;
    }

    if (rich_edit_box_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }

    switch (event->type)
    {
    case EGUI_KEY_EVENT_ACTION_DOWN:
        switch (event->key_code)
        {
        case EGUI_KEY_CODE_LEFT:
        case EGUI_KEY_CODE_RIGHT:
        case EGUI_KEY_CODE_HOME:
        case EGUI_KEY_CODE_END:
        case EGUI_KEY_CODE_TAB:
        case EGUI_KEY_CODE_BACKSPACE:
        case EGUI_KEY_CODE_DELETE:
        case EGUI_KEY_CODE_ENTER:
        case EGUI_KEY_CODE_SPACE:
            return 1;
        default:
            ch = egui_key_event_to_char(event);
            return ch != 0 ? 1 : 0;
        }
    case EGUI_KEY_EVENT_ACTION_UP:
        return rich_edit_box_handle_key_up(self, event);
    default:
        return 0;
    }
}

static int egui_view_rich_edit_box_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);
    EGUI_UNUSED(event);

    rich_edit_box_clear_pressed_state(self, local);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_rich_edit_box_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);
    EGUI_UNUSED(event);

    rich_edit_box_clear_pressed_state(self, local);
    return 1;
}
#endif

void egui_view_rich_edit_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_rich_edit_box_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_rich_edit_box_on_static_key_event;
#endif
}

static void egui_view_rich_edit_box_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);
    const egui_view_rich_edit_box_document_t *document = rich_edit_box_get_document(local);
    const egui_view_rich_edit_box_preset_t *preset;
    egui_view_rich_edit_box_metrics_t metrics;
    egui_color_t card_fill = local->surface_color;
    egui_color_t card_border = local->border_color;
    egui_color_t badge_fill = egui_rgb_mix(local->surface_color, local->accent_color, 14);
    egui_color_t badge_text = egui_rgb_mix(local->text_color, local->accent_color, 26);
    egui_color_t title_color = local->text_color;
    egui_color_t summary_color = local->muted_text_color;
    egui_color_t footer_fill = egui_rgb_mix(local->surface_color, local->accent_color, 10);
    egui_color_t footer_border = egui_rgb_mix(local->border_color, local->accent_color, 14);
    egui_color_t footer_text = egui_rgb_mix(local->muted_text_color, local->accent_color, 12);
    egui_color_t shadow_color = local->shadow_color;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_RICH_EDIT_BOX_COMPACT_RADIUS : EGUI_VIEW_RICH_EDIT_BOX_STANDARD_RADIUS;
    char char_count_text[24];
    egui_region_t footer_left_region;
    egui_region_t footer_right_region;
    const char *title_text = document != NULL && rich_edit_box_has_text(document->title) ? document->title : "Rich edit";
    const char *summary_text = document != NULL ? document->summary : "No document";
    const char *footer_meta_text = document != NULL ? document->footer : "No document";
    uint8_t preset_index;

    rich_edit_box_sync_state(local);
    rich_edit_box_get_metrics(local, self, &metrics);
    if (metrics.content_region.size.width <= 0 || metrics.content_region.size.height <= 0)
    {
        return;
    }

    preset = rich_edit_box_get_preset(document, local->applied_preset);
    if (preset != NULL && rich_edit_box_has_text(preset->meta))
    {
        footer_meta_text = preset->meta;
    }

    if (local->read_only_mode)
    {
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, 26);
        badge_text = egui_rgb_mix(badge_text, local->muted_text_color, 18);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 12);
        summary_color = egui_rgb_mix(summary_color, local->muted_text_color, 18);
        footer_fill = egui_rgb_mix(footer_fill, local->surface_color, 20);
        footer_border = egui_rgb_mix(footer_border, local->muted_text_color, 18);
        footer_text = egui_rgb_mix(footer_text, local->muted_text_color, 18);
        shadow_color = egui_rgb_mix(shadow_color, local->surface_color, 42);
    }
    if (!egui_view_get_enable(self))
    {
        card_fill = rich_edit_box_mix_disabled(card_fill);
        card_border = rich_edit_box_mix_disabled(card_border);
        badge_fill = rich_edit_box_mix_disabled(badge_fill);
        badge_text = rich_edit_box_mix_disabled(badge_text);
        title_color = rich_edit_box_mix_disabled(title_color);
        summary_color = rich_edit_box_mix_disabled(summary_color);
        footer_fill = rich_edit_box_mix_disabled(footer_fill);
        footer_border = rich_edit_box_mix_disabled(footer_border);
        footer_text = rich_edit_box_mix_disabled(footer_text);
        shadow_color = rich_edit_box_mix_disabled(shadow_color);
    }

    egui_canvas_draw_round_rectangle_fill(metrics.content_region.location.x, metrics.content_region.location.y + 2, metrics.content_region.size.width,
                                          metrics.content_region.size.height, radius, shadow_color, egui_color_alpha_mix(self->alpha, 20));
    egui_canvas_draw_round_rectangle_fill(metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                          metrics.content_region.size.height, radius, card_fill, egui_color_alpha_mix(self->alpha, 97));
    egui_canvas_draw_round_rectangle(metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                     metrics.content_region.size.height, radius, 1, card_border, egui_color_alpha_mix(self->alpha, 54));

    if (document != NULL && metrics.badge_region.size.width > 0 && metrics.badge_region.size.height > 0)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                              metrics.badge_region.size.height, metrics.badge_region.size.height / 2, badge_fill,
                                              egui_color_alpha_mix(self->alpha, 98));
        rich_edit_box_draw_text(local->meta_font != NULL ? local->meta_font : local->font, self, document->header, &metrics.badge_region, EGUI_ALIGN_CENTER,
                                badge_text);
    }

    rich_edit_box_draw_text(local->font, self, title_text, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
    if (metrics.show_summary)
    {
        rich_edit_box_draw_text(local->meta_font != NULL ? local->meta_font : local->font, self, summary_text, &metrics.summary_region,
                                EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, summary_color);
    }

    rich_edit_box_draw_editor(local, self, &metrics.editor_region, &metrics.editor_text_region);

    for (preset_index = 0; preset_index < metrics.preset_count; ++preset_index)
    {
        const egui_view_rich_edit_box_preset_t *current_preset = rich_edit_box_get_preset(document, preset_index);

        if (current_preset != NULL)
        {
            rich_edit_box_draw_preset(local, self, &metrics.preset_regions[preset_index], current_preset->label, preset_index);
        }
    }

    if (metrics.show_footer && metrics.footer_region.size.width > 0 && metrics.footer_region.size.height > 0)
    {
        snprintf(char_count_text, sizeof(char_count_text), "%u chars", (unsigned)local->text_len);
        footer_left_region = metrics.footer_region;
        footer_right_region = metrics.footer_region;
        footer_left_region.location.x += local->compact_mode ? 4 : 6;
        footer_left_region.size.width -= local->compact_mode ? 8 : 12;
        footer_right_region.location.x += local->compact_mode ? 4 : 6;
        footer_right_region.size.width -= local->compact_mode ? 8 : 12;

        egui_canvas_draw_round_rectangle_fill(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                              metrics.footer_region.size.height, metrics.footer_region.size.height / 2, footer_fill,
                                              egui_color_alpha_mix(self->alpha, 96));
        egui_canvas_draw_round_rectangle(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                         metrics.footer_region.size.height, metrics.footer_region.size.height / 2, 1, footer_border,
                                         egui_color_alpha_mix(self->alpha, 36));
        rich_edit_box_draw_text(local->meta_font != NULL ? local->meta_font : local->font, self, footer_meta_text, &footer_left_region,
                                EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, footer_text);
        rich_edit_box_draw_text(local->meta_font != NULL ? local->meta_font : local->font, self, char_count_text, &footer_right_region,
                                EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER, footer_text);
    }
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_rich_edit_box_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_rich_edit_box_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_rich_edit_box_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_rich_edit_box_on_key_event,
#endif
};

void egui_view_rich_edit_box_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_rich_edit_box_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_rich_edit_box_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->documents = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_action = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD5DDE5);
    local->editor_color = EGUI_COLOR_HEX(0xFBFCFE);
    local->text_color = EGUI_COLOR_HEX(0x1B2834);
    local->muted_text_color = EGUI_COLOR_HEX(0x6C7A89);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->shadow_color = EGUI_COLOR_HEX(0xDCE5EE);
    local->document_count = 0;
    local->current_document = 0;
    local->current_part = EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR;
    local->current_preset = 0;
    local->applied_preset = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_part = EGUI_VIEW_RICH_EDIT_BOX_PART_NONE;
    local->text_len = 0;
    local->editor_text[0] = '\0';

    egui_view_set_view_name(self, "egui_view_rich_edit_box");
}

void egui_view_rich_edit_box_set_documents(egui_view_t *self, const egui_view_rich_edit_box_document_t *documents, uint8_t document_count)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    rich_edit_box_clear_pressed_state(self, local);
    local->documents = documents;
    local->document_count = documents == NULL ? 0 : rich_edit_box_clamp_document_count(document_count);
    if (local->current_document >= local->document_count)
    {
        local->current_document = 0;
    }
    rich_edit_box_load_current_document(local);
    egui_view_invalidate(self);
}

void egui_view_rich_edit_box_set_current_document(egui_view_t *self, uint8_t document_index)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    rich_edit_box_clear_pressed_state(self, local);
    rich_edit_box_set_current_document_inner(self, document_index);
}

uint8_t egui_view_rich_edit_box_get_current_document(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    if (local->document_count == 0)
    {
        return 0;
    }
    if (local->current_document >= local->document_count)
    {
        return 0;
    }
    return local->current_document;
}

void egui_view_rich_edit_box_set_current_preset(egui_view_t *self, uint8_t preset_index)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    rich_edit_box_clear_pressed_state(self, local);
    rich_edit_box_select_preset_inner(self, preset_index, 0);
}

uint8_t egui_view_rich_edit_box_get_current_preset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    rich_edit_box_sync_state(local);
    return local->current_preset;
}

uint8_t egui_view_rich_edit_box_get_applied_preset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    rich_edit_box_sync_state(local);
    return local->applied_preset;
}

uint8_t egui_view_rich_edit_box_apply_current_preset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    rich_edit_box_clear_pressed_state(self, local);
    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        return 0;
    }
    rich_edit_box_sync_state(local);
    return rich_edit_box_activate_current_inner(self);
}

void egui_view_rich_edit_box_set_on_action_listener(egui_view_t *self, egui_view_on_rich_edit_box_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    local->on_action = listener;
}

void egui_view_rich_edit_box_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    rich_edit_box_clear_pressed_state(self, local);
    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_rich_edit_box_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    rich_edit_box_clear_pressed_state(self, local);
    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_rich_edit_box_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    rich_edit_box_clear_pressed_state(self, local);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

void egui_view_rich_edit_box_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    rich_edit_box_clear_pressed_state(self, local);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

void egui_view_rich_edit_box_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t editor_color,
                                         egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                         egui_color_t shadow_color)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    rich_edit_box_clear_pressed_state(self, local);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->editor_color = editor_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->shadow_color = shadow_color;
    egui_view_invalidate(self);
}

const char *egui_view_rich_edit_box_get_text(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    return local->editor_text;
}

uint8_t egui_view_rich_edit_box_get_text_length(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);

    return local->text_len;
}

uint8_t egui_view_rich_edit_box_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_rich_edit_box_t);
    const egui_view_rich_edit_box_document_t *document = rich_edit_box_get_document(local);
    egui_view_rich_edit_box_metrics_t metrics;
    uint8_t preset_index;

    if (region == NULL || !rich_edit_box_part_exists(document, part))
    {
        return 0;
    }

    rich_edit_box_get_metrics(local, self, &metrics);
    if (part == EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR)
    {
        if (metrics.editor_region.size.width <= 0 || metrics.editor_region.size.height <= 0)
        {
            return 0;
        }
        *region = metrics.editor_region;
        return 1;
    }

    preset_index = rich_edit_box_part_to_preset(part);
    if (preset_index >= metrics.preset_count || metrics.preset_regions[preset_index].size.width <= 0 || metrics.preset_regions[preset_index].size.height <= 0)
    {
        return 0;
    }
    *region = metrics.preset_regions[preset_index];
    return 1;
}
