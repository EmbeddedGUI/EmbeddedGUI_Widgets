#include "egui_view_auto_suggest_box.h"

#include <string.h>

#include "egui.h"
#include "../../../../sdk/EmbeddedGUI/src/resource/egui_icon_material_symbols.h"
#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_icon_font.h"

#define HCW_AUTO_SUGGEST_BOX_STANDARD_RADIUS 10
#define HCW_AUTO_SUGGEST_BOX_COMPACT_RADIUS  8
#define HCW_AUTO_SUGGEST_BOX_TEXT_INSET_X    2
#define HCW_AUTO_SUGGEST_BOX_TEXT_INSET_Y    1
#define HCW_AUTO_SUGGEST_BOX_POPUP_INSET_X   2
#define HCW_AUTO_SUGGEST_BOX_POPUP_INSET_Y   2

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_auto_suggest_box_standard_bg_normal_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100,
                                                        HCW_AUTO_SUGGEST_BOX_STANDARD_RADIUS, 1, EGUI_COLOR_HEX(0xD5DCE4), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_auto_suggest_box_standard_bg_pressed_param, EGUI_COLOR_HEX(0xF8FBFE), EGUI_ALPHA_100,
                                                        HCW_AUTO_SUGGEST_BOX_STANDARD_RADIUS, 1, EGUI_COLOR_HEX(0xC4D5E7), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_auto_suggest_box_standard_bg_disabled_param, EGUI_COLOR_HEX(0xF1F4F7), EGUI_ALPHA_100,
                                                        HCW_AUTO_SUGGEST_BOX_STANDARD_RADIUS, 1, EGUI_COLOR_HEX(0xD8E0E7), EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_auto_suggest_box_standard_bg_focused_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100,
                                                        HCW_AUTO_SUGGEST_BOX_STANDARD_RADIUS, 2, EGUI_COLOR_HEX(0x0F6CBD), EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_auto_suggest_box_standard_bg_params, &hcw_auto_suggest_box_standard_bg_normal_param,
                                      &hcw_auto_suggest_box_standard_bg_pressed_param, &hcw_auto_suggest_box_standard_bg_disabled_param,
                                      &hcw_auto_suggest_box_standard_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_auto_suggest_box_standard_bg_params, &hcw_auto_suggest_box_standard_bg_normal_param,
                           &hcw_auto_suggest_box_standard_bg_pressed_param, &hcw_auto_suggest_box_standard_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_auto_suggest_box_standard_background, &hcw_auto_suggest_box_standard_bg_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_auto_suggest_box_compact_bg_normal_param, EGUI_COLOR_HEX(0xF7FBFB), EGUI_ALPHA_100,
                                                        HCW_AUTO_SUGGEST_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xC9D9D7), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_auto_suggest_box_compact_bg_pressed_param, EGUI_COLOR_HEX(0xEEF7F5), EGUI_ALPHA_100,
                                                        HCW_AUTO_SUGGEST_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xBDD0CD), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_auto_suggest_box_compact_bg_disabled_param, EGUI_COLOR_HEX(0xEDF4F3), EGUI_ALPHA_100,
                                                        HCW_AUTO_SUGGEST_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xD4DFDE), EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_auto_suggest_box_compact_bg_focused_param, EGUI_COLOR_HEX(0xF7FBFB), EGUI_ALPHA_100,
                                                        HCW_AUTO_SUGGEST_BOX_COMPACT_RADIUS, 2, EGUI_COLOR_HEX(0x0C7C73), EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_auto_suggest_box_compact_bg_params, &hcw_auto_suggest_box_compact_bg_normal_param,
                                      &hcw_auto_suggest_box_compact_bg_pressed_param, &hcw_auto_suggest_box_compact_bg_disabled_param,
                                      &hcw_auto_suggest_box_compact_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_auto_suggest_box_compact_bg_params, &hcw_auto_suggest_box_compact_bg_normal_param,
                           &hcw_auto_suggest_box_compact_bg_pressed_param, &hcw_auto_suggest_box_compact_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_auto_suggest_box_compact_background, &hcw_auto_suggest_box_compact_bg_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_auto_suggest_box_read_only_bg_normal_param, EGUI_COLOR_HEX(0xF5F7FA), EGUI_ALPHA_100,
                                                        HCW_AUTO_SUGGEST_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xD7DEE6), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_auto_suggest_box_read_only_bg_pressed_param, EGUI_COLOR_HEX(0xF5F7FA), EGUI_ALPHA_100,
                                                        HCW_AUTO_SUGGEST_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xD7DEE6), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_auto_suggest_box_read_only_bg_disabled_param, EGUI_COLOR_HEX(0xF5F7FA), EGUI_ALPHA_100,
                                                        HCW_AUTO_SUGGEST_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xD7DEE6), EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_auto_suggest_box_read_only_bg_focused_param, EGUI_COLOR_HEX(0xF5F7FA), EGUI_ALPHA_100,
                                                        HCW_AUTO_SUGGEST_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xD7DEE6), EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_auto_suggest_box_read_only_bg_params, &hcw_auto_suggest_box_read_only_bg_normal_param,
                                      &hcw_auto_suggest_box_read_only_bg_pressed_param, &hcw_auto_suggest_box_read_only_bg_disabled_param,
                                      &hcw_auto_suggest_box_read_only_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_auto_suggest_box_read_only_bg_params, &hcw_auto_suggest_box_read_only_bg_normal_param,
                           &hcw_auto_suggest_box_read_only_bg_pressed_param, &hcw_auto_suggest_box_read_only_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_auto_suggest_box_read_only_background, &hcw_auto_suggest_box_read_only_bg_params);

static char auto_suggest_box_to_lower(char ch)
{
    if (ch >= 'A' && ch <= 'Z')
    {
        return (char)(ch - 'A' + 'a');
    }
    return ch;
}

static uint8_t auto_suggest_box_text_matches_query(const char *text, const char *query)
{
    if (query == NULL || query[0] == '\0')
    {
        return 1;
    }
    if (text == NULL)
    {
        return 0;
    }
    while (*query != '\0' && *text != '\0')
    {
        if (auto_suggest_box_to_lower(*text) != auto_suggest_box_to_lower(*query))
        {
            return 0;
        }
        text++;
        query++;
    }
    return *query == '\0';
}

static uint8_t auto_suggest_box_get_filtered_count_inner(const egui_view_auto_suggest_box_t *local)
{
    uint8_t index;
    uint8_t count = 0;
    const char *query = local->textinput.text;

    for (index = 0; index < local->suggestion_count; index++)
    {
        if (auto_suggest_box_text_matches_query(local->suggestions[index], query))
        {
            count++;
        }
    }
    return count;
}

static uint8_t auto_suggest_box_find_first_match(const egui_view_auto_suggest_box_t *local)
{
    uint8_t index;

    for (index = 0; index < local->suggestion_count; index++)
    {
        if (auto_suggest_box_text_matches_query(local->suggestions[index], local->textinput.text))
        {
            return index;
        }
    }
    return EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;
}

static uint8_t auto_suggest_box_find_last_match(const egui_view_auto_suggest_box_t *local)
{
    int index;

    for (index = (int)local->suggestion_count - 1; index >= 0; index--)
    {
        if (auto_suggest_box_text_matches_query(local->suggestions[index], local->textinput.text))
        {
            return (uint8_t)index;
        }
    }
    return EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;
}

static uint8_t auto_suggest_box_get_filtered_row_for_index(const egui_view_auto_suggest_box_t *local, uint8_t index)
{
    uint8_t scan;
    uint8_t row = 0;

    if (index >= local->suggestion_count || !auto_suggest_box_text_matches_query(local->suggestions[index], local->textinput.text))
    {
        return EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;
    }

    for (scan = 0; scan < index; scan++)
    {
        if (auto_suggest_box_text_matches_query(local->suggestions[scan], local->textinput.text))
        {
            row++;
        }
    }
    return row;
}

static uint8_t auto_suggest_box_get_source_index_for_filtered_row(const egui_view_auto_suggest_box_t *local, uint8_t row)
{
    uint8_t index;
    uint8_t current_row = 0;

    for (index = 0; index < local->suggestion_count; index++)
    {
        if (!auto_suggest_box_text_matches_query(local->suggestions[index], local->textinput.text))
        {
            continue;
        }
        if (current_row == row)
        {
            return index;
        }
        current_row++;
    }
    return EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;
}

static uint8_t auto_suggest_box_get_visible_count_for_height(const egui_view_auto_suggest_box_t *local, egui_dim_t total_height, uint8_t max_visible_count)
{
    uint8_t fit_count;
    egui_dim_t item_space;

    if (max_visible_count == 0 || local->item_height <= 0 || total_height <= local->collapsed_height)
    {
        return 0;
    }

    item_space = total_height - local->collapsed_height;
    fit_count = (uint8_t)(item_space / local->item_height);
    if (fit_count > max_visible_count)
    {
        fit_count = max_visible_count;
    }
    return fit_count;
}

static egui_dim_t auto_suggest_box_get_available_bottom(egui_view_t *self)
{
    egui_dim_t available_bottom = EGUI_CONFIG_SCEEN_HEIGHT;
    egui_view_t *parent = (egui_view_t *)self->parent;

    while (parent != NULL)
    {
        egui_dim_t parent_bottom = parent->region_screen.location.y + parent->padding.top;

        if (parent->region_screen.size.height > parent->padding.top + parent->padding.bottom)
        {
            parent_bottom += parent->region_screen.size.height - parent->padding.top - parent->padding.bottom;
        }
        if (parent_bottom < available_bottom)
        {
            available_bottom = parent_bottom;
        }

        parent = (egui_view_t *)parent->parent;
    }

    return available_bottom;
}

static void auto_suggest_box_local_region_to_screen(egui_view_t *self, const egui_region_t *local_region, egui_region_t *screen_region)
{
    screen_region->location.x = self->region_screen.location.x + local_region->location.x;
    screen_region->location.y = self->region_screen.location.y + local_region->location.y;
    screen_region->size.width = local_region->size.width;
    screen_region->size.height = local_region->size.height;
}

static egui_dim_t auto_suggest_box_scale_metric(egui_dim_t value, egui_dim_t scaled_height, egui_dim_t base_height)
{
    if (value <= 0 || scaled_height <= 0 || base_height <= 0)
    {
        return value;
    }
    return (egui_dim_t)(((int32_t)value * scaled_height + base_height / 2) / base_height);
}

static void auto_suggest_box_sync_external_scale(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    egui_dim_t scaled_height = self->region.size.height;
    egui_dim_t base_height = local->collapsed_height;

    if (local->is_expanded || scaled_height <= 0 || base_height <= 0 || scaled_height == base_height)
    {
        return;
    }

    local->item_height = EGUI_MAX((egui_dim_t)1, auto_suggest_box_scale_metric(local->item_height, scaled_height, base_height));
    local->icon_text_gap = EGUI_MAX((egui_dim_t)1, auto_suggest_box_scale_metric(local->icon_text_gap, scaled_height, base_height));
    local->collapsed_height = scaled_height;
    if (self->region_screen.size.height != scaled_height)
    {
        self->region_screen.size.height = scaled_height;
    }
}

static void auto_suggest_box_inset_region(egui_region_t *region, egui_dim_t left, egui_dim_t right, egui_dim_t top, egui_dim_t bottom)
{
    region->location.x += left;
    region->location.y += top;
    region->size.width -= left + right;
    region->size.height -= top + bottom;
    if (region->size.width < 0)
    {
        region->size.width = 0;
    }
    if (region->size.height < 0)
    {
        region->size.height = 0;
    }
}

static void auto_suggest_box_get_text_region(egui_view_t *self, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);

    auto_suggest_box_sync_external_scale(self);
    region->location.x = self->padding.left;
    region->location.y = self->padding.top;
    region->size.width = self->region.size.width - (self->padding.left + self->padding.right);
    region->size.height = local->collapsed_height - (self->padding.top + self->padding.bottom);
    auto_suggest_box_inset_region(region, HCW_AUTO_SUGGEST_BOX_TEXT_INSET_X, HCW_AUTO_SUGGEST_BOX_TEXT_INSET_X, HCW_AUTO_SUGGEST_BOX_TEXT_INSET_Y,
                                  HCW_AUTO_SUGGEST_BOX_TEXT_INSET_Y);
}

static void auto_suggest_box_get_search_icon_region(egui_view_t *self, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);

    auto_suggest_box_sync_external_scale(self);
    egui_dim_t size = EGUI_MIN(self->padding.left - local->icon_text_gap, local->collapsed_height - 10);

    if (size < 12)
    {
        size = 12;
    }

    region->size.width = size;
    region->size.height = size;
    region->location.x = 8;
    region->location.y = (local->collapsed_height - size) / 2;
}

static uint8_t auto_suggest_box_is_valid_index(const egui_view_auto_suggest_box_t *local, uint8_t index)
{
    return (local->suggestions != NULL && index < local->suggestion_count) ? 1 : 0;
}

static uint8_t auto_suggest_box_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    uint8_t had_state = self->is_pressed || local->pressed_part != EGUI_VIEW_AUTO_SUGGEST_BOX_PART_NONE;

    if (!had_state)
    {
        return 0;
    }

    local->pressed_part = EGUI_VIEW_AUTO_SUGGEST_BOX_PART_NONE;
    local->pressed_row = EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;
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

static void auto_suggest_box_set_expanded_height(egui_view_t *self, egui_dim_t expanded_height)
{
    egui_view_update_region_dirty(self, &self->region_screen);
    self->region.size.height = expanded_height;
    self->region_screen.size.height = expanded_height;
    egui_view_update_region_dirty(self, &self->region_screen);
    egui_view_invalidate(self);
}

static uint8_t auto_suggest_box_get_expand_fit_count(egui_view_t *self, const egui_view_auto_suggest_box_t *local)
{
    uint8_t max_visible_count = auto_suggest_box_get_filtered_count_inner(local);
    egui_dim_t available_height = 0;
    egui_dim_t available_bottom;

    auto_suggest_box_sync_external_scale(self);
    if (max_visible_count > local->max_visible_items)
    {
        max_visible_count = local->max_visible_items;
    }
    if (max_visible_count == 0)
    {
        return 0;
    }

    available_bottom = auto_suggest_box_get_available_bottom(self);
    if (self->region_screen.location.y < available_bottom)
    {
        available_height = available_bottom - self->region_screen.location.y;
    }

    return auto_suggest_box_get_visible_count_for_height(local, available_height, max_visible_count);
}

static uint8_t auto_suggest_box_get_current_visible_count(egui_view_t *self, const egui_view_auto_suggest_box_t *local)
{
    uint8_t max_visible_count = auto_suggest_box_get_filtered_count_inner(local);

    auto_suggest_box_sync_external_scale(self);
    if (max_visible_count > local->max_visible_items)
    {
        max_visible_count = local->max_visible_items;
    }
    return auto_suggest_box_get_visible_count_for_height(local, self->region.size.height, max_visible_count);
}

static uint8_t auto_suggest_box_get_visible_start_row(egui_view_t *self, const egui_view_auto_suggest_box_t *local, uint8_t visible_count)
{
    uint8_t filtered_count = auto_suggest_box_get_filtered_count_inner(local);
    uint8_t current_row = auto_suggest_box_get_filtered_row_for_index(local, local->current_index);
    uint8_t start_row = 0;

    EGUI_UNUSED(self);

    if (visible_count == 0 || filtered_count <= visible_count || current_row == EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE)
    {
        return 0;
    }

    if (current_row >= visible_count)
    {
        start_row = (uint8_t)(current_row + 1 - visible_count);
    }
    if ((uint16_t)start_row + visible_count > filtered_count)
    {
        start_row = (uint8_t)(filtered_count - visible_count);
    }
    return start_row;
}

static void auto_suggest_box_sync_current_index(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    uint8_t first_match;

    if (local->suggestion_count == 0 || local->suggestions == NULL)
    {
        local->current_index = EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;
        return;
    }

    if (local->textinput.text_len == 0)
    {
        if (!auto_suggest_box_is_valid_index(local, local->current_index))
        {
            local->current_index = 0;
        }
        return;
    }

    if (auto_suggest_box_is_valid_index(local, local->current_index) &&
        auto_suggest_box_text_matches_query(local->suggestions[local->current_index], local->textinput.text))
    {
        return;
    }

    first_match = auto_suggest_box_find_first_match(local);
    local->current_index = first_match;
}

static void auto_suggest_box_refresh_expanded_height(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    uint8_t visible_count;
    egui_dim_t expanded_height;

    if (!local->is_expanded)
    {
        return;
    }

    visible_count = auto_suggest_box_get_expand_fit_count(self, local);
    if (visible_count == 0)
    {
        egui_view_auto_suggest_box_collapse(self);
        return;
    }

    expanded_height = local->collapsed_height + visible_count * local->item_height;
    if (self->region.size.height != expanded_height || self->region_screen.size.height != expanded_height)
    {
        auto_suggest_box_set_expanded_height(self, expanded_height);
    }
    else
    {
        egui_view_invalidate(self);
    }
}

static void auto_suggest_box_notify_selected(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);

    if (local->on_selected != NULL && auto_suggest_box_is_valid_index(local, local->current_index))
    {
        local->on_selected(self, local->current_index);
    }
}

static void auto_suggest_box_commit_current_index(egui_view_t *self, uint8_t index, uint8_t notify)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);

    if (!auto_suggest_box_is_valid_index(local, index))
    {
        return;
    }

    local->current_index = index;
    egui_view_textinput_set_text(self, local->suggestions[index]);
    egui_view_auto_suggest_box_collapse(self);
    if (notify)
    {
        auto_suggest_box_notify_selected(self);
    }
}

static uint8_t auto_suggest_box_step_match(const egui_view_auto_suggest_box_t *local, int delta)
{
    uint8_t filtered_count = auto_suggest_box_get_filtered_count_inner(local);
    uint8_t current_row = auto_suggest_box_get_filtered_row_for_index(local, local->current_index);

    if (filtered_count == 0)
    {
        return EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;
    }
    if (current_row == EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE)
    {
        return delta >= 0 ? auto_suggest_box_find_first_match(local) : auto_suggest_box_find_last_match(local);
    }
    if (delta > 0 && current_row + 1 < filtered_count)
    {
        current_row++;
    }
    else if (delta < 0 && current_row > 0)
    {
        current_row--;
    }
    return auto_suggest_box_get_source_index_for_filtered_row(local, current_row);
}

static void auto_suggest_box_update_query_state(egui_view_t *self, uint8_t auto_expand)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    uint8_t filtered_count;

    auto_suggest_box_sync_current_index(self);
    filtered_count = auto_suggest_box_get_filtered_count_inner(local);

    if (filtered_count == 0 || local->read_only_mode)
    {
        egui_view_auto_suggest_box_collapse(self);
        egui_view_invalidate(self);
        return;
    }

    if (auto_expand && local->textinput.text_len > 0 && egui_view_get_enable(self))
    {
        egui_view_auto_suggest_box_expand(self);
        return;
    }

    auto_suggest_box_refresh_expanded_height(self);
    egui_view_invalidate(self);
}

static int auto_suggest_box_get_text_cursor_region(egui_view_t *self, egui_region_t *cursor_region)
{
    egui_view_textinput_t *input = (egui_view_textinput_t *)self;
    egui_region_t text_region;
    egui_dim_t text_width = 0;
    egui_dim_t text_height = 0;
    char tmp[EGUI_CONFIG_TEXTINPUT_MAX_LENGTH + 1];
    uint8_t len = input->cursor_pos;

    if (input->font == NULL || cursor_region == NULL)
    {
        return 0;
    }

    auto_suggest_box_get_text_region(self, &text_region);
    if (len > input->text_len)
    {
        len = input->text_len;
    }
    memcpy(tmp, input->text, len);
    tmp[len] = '\0';
    input->font->api->get_str_size(input->font, tmp, 0, 0, &text_width, &text_height);

    cursor_region->location.x = text_region.location.x + text_width - input->scroll_offset_x;
    cursor_region->location.y = text_region.location.y + (text_region.size.height - text_height) / 2;
    cursor_region->size.width = 1;
    cursor_region->size.height = text_height;
    return !egui_region_is_empty(cursor_region);
}

static void auto_suggest_box_draw_icon(egui_view_t *self, const egui_region_t *region, const char *icon, egui_color_t color)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    egui_canvas_t *canvas = egui_view_get_canvas(self);
    egui_region_t screen_region;
    egui_region_t draw_region;
    const egui_font_t *icon_font;

    if (!EGUI_VIEW_ICON_TEXT_VALID(icon))
    {
        return;
    }

    auto_suggest_box_local_region_to_screen(self, region, &screen_region);
    if (!egui_canvas_is_region_active(canvas, &screen_region))
    {
        return;
    }

    icon_font = EGUI_VIEW_ICON_FONT_RESOLVE(local->icon_font, EGUI_MIN(region->size.width, region->size.height), 18, 20);
    if (icon_font == NULL)
    {
        return;
    }

    draw_region = *region;
    egui_canvas_draw_text_in_rect(canvas, icon_font, icon, &draw_region, EGUI_ALIGN_CENTER, color, EGUI_ALPHA_100);
}

static egui_dim_t auto_suggest_box_get_border_radius(const egui_view_auto_suggest_box_t *local)
{
    if (local->collapsed_height <= 30)
    {
        return HCW_AUTO_SUGGEST_BOX_COMPACT_RADIUS;
    }
    return HCW_AUTO_SUGGEST_BOX_STANDARD_RADIUS;
}

static void auto_suggest_box_draw_popup(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    egui_canvas_t *canvas = egui_view_get_canvas(self);
    egui_region_t popup_region;
    egui_color_t text_color = local->text_color;
    uint8_t visible_count;
    uint8_t start_row;
    uint8_t row;

    if (!local->is_expanded)
    {
        return;
    }

    visible_count = auto_suggest_box_get_current_visible_count(self, local);
    if (visible_count == 0)
    {
        return;
    }

    popup_region.location.x = 1;
    popup_region.location.y = local->collapsed_height;
    popup_region.size.width = self->region.size.width - 2;
    popup_region.size.height = visible_count * local->item_height;

    if (popup_region.size.width <= 0 || popup_region.size.height <= 0)
    {
        return;
    }

    egui_canvas_draw_rectangle_fill(canvas, popup_region.location.x, popup_region.location.y, popup_region.size.width, popup_region.size.height, local->popup_color,
                                    EGUI_ALPHA_100);

    start_row = auto_suggest_box_get_visible_start_row(self, local, visible_count);
    for (row = 0; row < visible_count; row++)
    {
        egui_region_t item_region;
        egui_dim_t item_row_y = local->collapsed_height + row * local->item_height;
        uint8_t source_index = auto_suggest_box_get_source_index_for_filtered_row(local, (uint8_t)(start_row + row));
        egui_color_t item_text_color = text_color;

        item_region.location.x = self->padding.left;
        item_region.location.y = item_row_y;
        item_region.size.width = self->region.size.width - (self->padding.left + self->padding.right);
        item_region.size.height = local->item_height;
        auto_suggest_box_inset_region(&item_region, HCW_AUTO_SUGGEST_BOX_POPUP_INSET_X, HCW_AUTO_SUGGEST_BOX_POPUP_INSET_X, HCW_AUTO_SUGGEST_BOX_POPUP_INSET_Y,
                                      HCW_AUTO_SUGGEST_BOX_POPUP_INSET_Y);

        if (source_index == EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE)
        {
            continue;
        }
        if (source_index == local->current_index)
        {
            egui_canvas_draw_rectangle_fill(canvas, 1, item_row_y, self->region.size.width - 2, local->item_height, local->highlight_color, EGUI_ALPHA_100);
            item_text_color = EGUI_COLOR_WHITE;
        }

        egui_canvas_draw_text_in_rect(canvas, local->textinput.font, local->suggestions[source_index], &item_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                      item_text_color, self->alpha);
    }

}

static void auto_suggest_box_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    egui_view_textinput_t *input = &local->textinput;
    egui_canvas_t *canvas = egui_view_get_canvas(self);
    egui_region_t text_region;
    egui_region_t text_screen_region;
    egui_region_t icon_region;
    egui_dim_t border_radius;

    if (input->font == NULL)
    {
        return;
    }

    auto_suggest_box_sync_external_scale(self);
    auto_suggest_box_get_search_icon_region(self, &icon_region);
    auto_suggest_box_draw_icon(self, &icon_region, EGUI_ICON_MS_SEARCH, local->icon_color);

    auto_suggest_box_get_text_region(self, &text_region);
    auto_suggest_box_local_region_to_screen(self, &text_region, &text_screen_region);
    if (egui_canvas_is_region_active(canvas, &text_screen_region))
    {
        if (input->text_len == 0 && !self->is_focused && input->placeholder != NULL)
        {
            egui_canvas_draw_text_in_rect(canvas, input->font, input->placeholder, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, input->placeholder_color,
                                          input->placeholder_alpha);
        }
        else if (input->text_len > 0)
        {
            egui_dim_t text_width = 0;
            egui_dim_t text_height = 0;
            egui_dim_t text_x;
            egui_dim_t text_y;

            input->font->api->get_str_size(input->font, input->text, 0, 0, &text_width, &text_height);
            text_x = text_region.location.x - input->scroll_offset_x;
            text_y = text_region.location.y + (text_region.size.height - text_height) / 2;
            egui_canvas_draw_text(canvas, input->font, input->text, text_x, text_y, input->text_color, input->text_alpha);
        }

        if (self->is_enable && self->is_focused && input->cursor_visible)
        {
            egui_region_t cursor_region = {{0, 0}, {0, 0}};
            egui_region_t cursor_screen_region = {{0, 0}, {0, 0}};

            if (auto_suggest_box_get_text_cursor_region(self, &cursor_region))
            {
                auto_suggest_box_local_region_to_screen(self, &cursor_region, &cursor_screen_region);
                if (egui_canvas_is_region_active(canvas, &cursor_screen_region))
                {
                    egui_canvas_draw_rectangle_fill(canvas, cursor_region.location.x, cursor_region.location.y, cursor_region.size.width,
                                                    cursor_region.size.height, input->cursor_color, EGUI_ALPHA_100);
                }
            }
        }
    }

    auto_suggest_box_draw_popup(self);

    if (local->is_expanded)
    {
        border_radius = auto_suggest_box_get_border_radius(local);
        if (self->is_focused && egui_view_get_enable(self) && self->region.size.width > 4 && self->region.size.height > 4)
        {
            egui_canvas_draw_round_rectangle(canvas, 0, 0, self->region.size.width, self->region.size.height, border_radius, 2, local->border_color,
                                             egui_color_alpha_mix(self->alpha, 100));
            egui_canvas_draw_round_rectangle(canvas, 2, 2, self->region.size.width - 4, self->region.size.height - 4,
                                             border_radius > 2 ? (border_radius - 2) : border_radius, 1, local->border_color,
                                             egui_color_alpha_mix(self->alpha, 56));
        }
        else
        {
            egui_canvas_draw_round_rectangle(canvas, 0, 0, self->region.size.width, self->region.size.height, border_radius, 1, local->popup_border_color,
                                             self->alpha);
        }
    }
}

static uint8_t auto_suggest_box_get_touch_target(egui_view_t *self, egui_dim_t touch_x, egui_dim_t touch_y, uint8_t *part, uint8_t *source_index,
                                                 uint8_t *filtered_row)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    egui_dim_t local_y;
    uint8_t visible_count;
    uint8_t start_row;
    uint8_t row;

    *part = EGUI_VIEW_AUTO_SUGGEST_BOX_PART_NONE;
    *source_index = EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;
    *filtered_row = EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;

    auto_suggest_box_sync_external_scale(self);
    if (!egui_region_pt_in_rect(&self->region_screen, touch_x, touch_y))
    {
        return 0;
    }

    local_y = touch_y - self->region_screen.location.y;
    if (local_y < local->collapsed_height)
    {
        *part = EGUI_VIEW_AUTO_SUGGEST_BOX_PART_FIELD;
        return 1;
    }
    if (!local->is_expanded)
    {
        return 0;
    }

    visible_count = auto_suggest_box_get_current_visible_count(self, local);
    start_row = auto_suggest_box_get_visible_start_row(self, local, visible_count);
    row = (uint8_t)((local_y - local->collapsed_height) / local->item_height);
    if (row >= visible_count)
    {
        return 0;
    }

    *part = EGUI_VIEW_AUTO_SUGGEST_BOX_PART_ITEM;
    *filtered_row = (uint8_t)(start_row + row);
    *source_index = auto_suggest_box_get_source_index_for_filtered_row(local, *filtered_row);
    return *source_index != EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int auto_suggest_box_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    uint8_t hit_part;
    uint8_t hit_index;
    uint8_t hit_row;
    uint8_t hit_valid;

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        auto_suggest_box_clear_pressed_state(self);
        return 0;
    }

    hit_valid = auto_suggest_box_get_touch_target(self, event->location.x, event->location.y, &hit_part, &hit_index, &hit_row);

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        if (!hit_valid)
        {
            return 0;
        }
        local->pressed_part = hit_part;
        local->pressed_row = hit_row;
        egui_view_set_pressed(self, true);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        if (hit_part == EGUI_VIEW_AUTO_SUGGEST_BOX_PART_FIELD)
        {
            local->api.on_touch_event(self, event);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_part == EGUI_VIEW_AUTO_SUGGEST_BOX_PART_NONE)
        {
            return 0;
        }
        egui_view_set_pressed(self, hit_valid && local->pressed_part == hit_part && local->pressed_row == hit_row);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
        if (local->pressed_part == EGUI_VIEW_AUTO_SUGGEST_BOX_PART_NONE)
        {
            return 0;
        }
        if (self->is_pressed && hit_valid && local->pressed_part == hit_part && local->pressed_row == hit_row)
        {
            if (hit_part == EGUI_VIEW_AUTO_SUGGEST_BOX_PART_FIELD)
            {
                if (local->is_expanded)
                {
                    egui_view_auto_suggest_box_collapse(self);
                }
                else
                {
                    egui_view_auto_suggest_box_expand(self);
                }
            }
            else if (hit_part == EGUI_VIEW_AUTO_SUGGEST_BOX_PART_ITEM)
            {
                auto_suggest_box_commit_current_index(self, hit_index, 1);
            }
        }
        auto_suggest_box_clear_pressed_state(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return auto_suggest_box_clear_pressed_state(self);
    default:
        return 0;
    }
}

static int auto_suggest_box_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    auto_suggest_box_clear_pressed_state(self);
    egui_view_auto_suggest_box_collapse(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int auto_suggest_box_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    char text_before[EGUI_CONFIG_TEXTINPUT_MAX_LENGTH + 1];
    uint8_t text_len_before = local->textinput.text_len;
    int handled;

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        auto_suggest_box_clear_pressed_state(self);
        return 0;
    }

    memcpy(text_before, local->textinput.text, sizeof(text_before));

    if (event->type == EGUI_KEY_EVENT_ACTION_UP)
    {
        switch (event->key_code)
        {
        case EGUI_KEY_CODE_DOWN:
            if (!local->is_expanded)
            {
                if (auto_suggest_box_get_expand_fit_count(self, local) == 0)
                {
                    return 0;
                }
                egui_view_auto_suggest_box_expand(self);
                return 1;
            }
            if (auto_suggest_box_get_filtered_count_inner(local) > 0)
            {
                uint8_t next_index = auto_suggest_box_step_match(local, 1);
                if (next_index != EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE)
                {
                    local->current_index = next_index;
                    auto_suggest_box_refresh_expanded_height(self);
                }
                return 1;
            }
            return 0;
        case EGUI_KEY_CODE_UP:
            if (!local->is_expanded)
            {
                if (auto_suggest_box_get_expand_fit_count(self, local) == 0)
                {
                    return 0;
                }
                egui_view_auto_suggest_box_expand(self);
                return 1;
            }
            if (auto_suggest_box_get_filtered_count_inner(local) > 0)
            {
                uint8_t next_index = auto_suggest_box_step_match(local, -1);
                if (next_index != EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE)
                {
                    local->current_index = next_index;
                    auto_suggest_box_refresh_expanded_height(self);
                }
                return 1;
            }
            return 0;
        case EGUI_KEY_CODE_HOME:
            if (local->is_expanded && auto_suggest_box_get_filtered_count_inner(local) > 0)
            {
                local->current_index = auto_suggest_box_find_first_match(local);
                auto_suggest_box_refresh_expanded_height(self);
                return 1;
            }
            break;
        case EGUI_KEY_CODE_END:
            if (local->is_expanded && auto_suggest_box_get_filtered_count_inner(local) > 0)
            {
                local->current_index = auto_suggest_box_find_last_match(local);
                auto_suggest_box_refresh_expanded_height(self);
                return 1;
            }
            break;
        case EGUI_KEY_CODE_ENTER:
            if (local->is_expanded)
            {
                if (local->current_index != EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE)
                {
                    auto_suggest_box_commit_current_index(self, local->current_index, 1);
                    return 1;
                }
            }
            return 0;
        case EGUI_KEY_CODE_ESCAPE:
            if (local->is_expanded)
            {
                egui_view_auto_suggest_box_collapse(self);
                return 1;
            }
            return 0;
        default:
            break;
        }
    }

    handled = local->api.on_key_event(self, event);
    if (!handled)
    {
        return 0;
    }

    if (strcmp(text_before, local->textinput.text) != 0 || text_len_before != local->textinput.text_len)
    {
        auto_suggest_box_update_query_state(self, 1);
    }
    return 1;
}

static int auto_suggest_box_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    auto_suggest_box_clear_pressed_state(self);
    egui_view_auto_suggest_box_collapse(self);
    return 1;
}
#endif

static void hcw_auto_suggest_box_apply_style(egui_view_t *self, egui_background_t *background, egui_color_t popup_color, egui_color_t popup_border_color,
                                             egui_color_t text_color, egui_color_t muted_text_color, egui_color_t highlight_color, egui_color_t icon_color,
                                             egui_color_t cursor_color, egui_dim_t pad_left, egui_dim_t pad_right, egui_dim_t pad_y,
                                             egui_dim_t collapsed_height, egui_dim_t item_height, uint8_t max_visible_items, uint8_t is_enable,
                                             uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);

    auto_suggest_box_clear_pressed_state(self);
    egui_view_auto_suggest_box_collapse(self);
    egui_view_set_shadow(self, NULL);
    egui_view_set_background(self, background);
    egui_view_set_padding(self, pad_left, pad_right, pad_y, pad_y);
    egui_view_textinput_set_text_color(self, text_color, EGUI_ALPHA_100);
    egui_view_textinput_set_placeholder_color(self, muted_text_color, EGUI_ALPHA_100);
    egui_view_textinput_set_cursor_color(self, cursor_color);
    local->popup_color = popup_color;
    local->popup_border_color = popup_border_color;
    local->border_color = cursor_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->highlight_color = highlight_color;
    local->icon_color = icon_color;
    local->collapsed_height = collapsed_height;
    local->item_height = item_height;
    local->max_visible_items = max_visible_items == 0 ? 1 : max_visible_items;
    local->read_only_mode = read_only_mode ? 1 : 0;
    if (!local->is_expanded)
    {
        self->region.size.height = collapsed_height;
        self->region_screen.size.height = collapsed_height;
    }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (!is_enable)
    {
        egui_view_clear_focus(self);
        local->textinput.cursor_visible = 0;
    }
#endif
    egui_view_set_enable(self, is_enable);
    egui_view_invalidate(self);
}

void hcw_auto_suggest_box_apply_standard_style(egui_view_t *self)
{
    hcw_auto_suggest_box_apply_style(self, EGUI_BG_OF(&hcw_auto_suggest_box_standard_background), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xDCE3EB),
                                     EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x5B7FD6), EGUI_COLOR_HEX(0x5C6B79),
                                     EGUI_COLOR_HEX(0x0F6CBD), 28, 10, 8, 34, 24, 4, 1, 0);
}

void hcw_auto_suggest_box_apply_compact_style(egui_view_t *self)
{
    hcw_auto_suggest_box_apply_style(self, EGUI_BG_OF(&hcw_auto_suggest_box_compact_background), EGUI_COLOR_HEX(0xF7FBFB), EGUI_COLOR_HEX(0xD4E1DF),
                                     EGUI_COLOR_HEX(0x183235), EGUI_COLOR_HEX(0x66817E), EGUI_COLOR_HEX(0x0C7C73), EGUI_COLOR_HEX(0x55716D),
                                     EGUI_COLOR_HEX(0x0C7C73), 24, 8, 6, 28, 21, 3, 1, 0);
}

void hcw_auto_suggest_box_apply_read_only_style(egui_view_t *self)
{
    hcw_auto_suggest_box_apply_style(self, EGUI_BG_OF(&hcw_auto_suggest_box_read_only_background), EGUI_COLOR_HEX(0xF5F7FA), EGUI_COLOR_HEX(0xE0E6ED),
                                     EGUI_COLOR_HEX(0x7A8796), EGUI_COLOR_HEX(0x95A1AE), EGUI_COLOR_HEX(0xA9B7C4), EGUI_COLOR_HEX(0x8A97A5),
                                     EGUI_COLOR_HEX(0x7A8796), 24, 8, 6, 28, 21, 3, 0, 1);
}

void hcw_auto_suggest_box_set_suggestions(egui_view_t *self, const char **suggestions, uint8_t count)
{
    auto_suggest_box_clear_pressed_state(self);
    egui_view_auto_suggest_box_set_suggestions(self, suggestions, count);
}

void hcw_auto_suggest_box_set_query(egui_view_t *self, const char *query)
{
    auto_suggest_box_clear_pressed_state(self);
    egui_view_auto_suggest_box_set_query(self, query);
}

void hcw_auto_suggest_box_set_current_index(egui_view_t *self, uint8_t index)
{
    auto_suggest_box_clear_pressed_state(self);
    egui_view_auto_suggest_box_set_current_index(self, index);
}

void hcw_auto_suggest_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = auto_suggest_box_on_static_touch_event;
    api->on_touch = NULL;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = auto_suggest_box_on_static_key_event;
    api->on_key = NULL;
#endif
}

void egui_view_auto_suggest_box_set_suggestions(egui_view_t *self, const char **suggestions, uint8_t count)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);

    local->suggestions = suggestions;
    local->suggestion_count = (suggestions == NULL) ? 0 : count;
    if (local->suggestion_count == 0)
    {
        local->current_index = EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;
    }
    auto_suggest_box_update_query_state(self, 0);
}

uint8_t egui_view_auto_suggest_box_get_suggestion_count(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    return local->suggestion_count;
}

void egui_view_auto_suggest_box_set_query(egui_view_t *self, const char *query)
{
    egui_view_textinput_set_text(self, query);
    auto_suggest_box_update_query_state(self, 0);
}

const char *egui_view_auto_suggest_box_get_query(egui_view_t *self)
{
    return egui_view_textinput_get_text(self);
}

void egui_view_auto_suggest_box_set_current_index(egui_view_t *self, uint8_t index)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);

    if (!auto_suggest_box_is_valid_index(local, index))
    {
        return;
    }
    auto_suggest_box_commit_current_index(self, index, 0);
}

uint8_t egui_view_auto_suggest_box_get_current_index(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    return local->current_index;
}

const char *egui_view_auto_suggest_box_get_current_text(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);

    if (!auto_suggest_box_is_valid_index(local, local->current_index))
    {
        return NULL;
    }
    return local->suggestions[local->current_index];
}

uint8_t egui_view_auto_suggest_box_get_filtered_count(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    return auto_suggest_box_get_filtered_count_inner(local);
}

void egui_view_auto_suggest_box_set_max_visible_items(egui_view_t *self, uint8_t max_items)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);

    if (max_items == 0)
    {
        max_items = 1;
    }
    if (local->max_visible_items == max_items)
    {
        return;
    }
    local->max_visible_items = max_items;
    auto_suggest_box_refresh_expanded_height(self);
    egui_view_invalidate(self);
}

void egui_view_auto_suggest_box_set_font(egui_view_t *self, const egui_font_t *font)
{
    if (font == NULL)
    {
        return;
    }
    egui_view_textinput_set_font(self, font);
}

void egui_view_auto_suggest_box_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);

    if (local->icon_font == font)
    {
        return;
    }
    local->icon_font = font;
    egui_view_invalidate(self);
}

void egui_view_auto_suggest_box_set_placeholder(egui_view_t *self, const char *placeholder)
{
    egui_view_textinput_set_placeholder(self, placeholder);
}

void egui_view_auto_suggest_box_set_on_selected_listener(egui_view_t *self, egui_view_on_auto_suggest_box_selected_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    local->on_selected = listener;
}

void egui_view_auto_suggest_box_expand(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    uint8_t visible_count;
    egui_dim_t expanded_height;

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        return;
    }

    auto_suggest_box_sync_current_index(self);
    visible_count = auto_suggest_box_get_expand_fit_count(self, local);
    if (visible_count == 0)
    {
        return;
    }

    expanded_height = local->collapsed_height + visible_count * local->item_height;
    if (!local->is_expanded)
    {
        local->is_expanded = 1;
#if EGUI_CONFIG_FUNCTION_SUPPORT_LAYER
        egui_view_set_layer(self, EGUI_VIEW_LAYER_TOP);
#endif
    }
    auto_suggest_box_set_expanded_height(self, expanded_height);
}

void egui_view_auto_suggest_box_collapse(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);

    if (!local->is_expanded)
    {
        if (self->region.size.height != local->collapsed_height || self->region_screen.size.height != local->collapsed_height)
        {
            self->region.size.height = local->collapsed_height;
            self->region_screen.size.height = local->collapsed_height;
            egui_view_invalidate(self);
        }
        return;
    }

    local->is_expanded = 0;
#if EGUI_CONFIG_FUNCTION_SUPPORT_LAYER
    egui_view_set_layer(self, EGUI_VIEW_LAYER_DEFAULT);
#endif
    auto_suggest_box_set_expanded_height(self, local->collapsed_height);
}

uint8_t egui_view_auto_suggest_box_is_expanded(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);
    return local->is_expanded;
}

void egui_view_auto_suggest_box_apply_params(egui_view_t *self, const egui_view_auto_suggest_box_params_t *params)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);

    self->region = params->region;
    local->collapsed_height = params->region.size.height;
    self->region.size.height = local->collapsed_height;
    self->region_screen.size.height = local->collapsed_height;

    egui_view_auto_suggest_box_set_suggestions(self, params->suggestions, params->suggestion_count);
    if (params->placeholder != NULL)
    {
        egui_view_auto_suggest_box_set_placeholder(self, params->placeholder);
    }
    if (params->query != NULL)
    {
        egui_view_auto_suggest_box_set_query(self, params->query);
    }
    else if (params->current_index != EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE && params->current_index < params->suggestion_count)
    {
        egui_view_auto_suggest_box_set_current_index(self, params->current_index);
    }
    else
    {
        egui_view_auto_suggest_box_set_query(self, NULL);
    }
    egui_view_auto_suggest_box_collapse(self);
}

void egui_view_auto_suggest_box_init_with_params(egui_view_t *self, egui_core_t *core, const egui_view_auto_suggest_box_params_t *params)
{
    egui_view_auto_suggest_box_init(self, core);
    egui_view_auto_suggest_box_apply_params(self, params);
}

void egui_view_auto_suggest_box_init(egui_view_t *self, egui_core_t *core)
{
    EGUI_LOCAL_INIT(egui_view_auto_suggest_box_t);

    egui_view_textinput_init(self, core);
    egui_view_copy_api(self, &local->api);
    local->custom_api = local->api;
    local->custom_api.on_draw = auto_suggest_box_on_draw;
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    local->custom_api.on_touch_event = auto_suggest_box_on_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    local->custom_api.on_key_event = auto_suggest_box_on_key_event;
#endif
    self->api = &local->custom_api;
    local->icon_font = NULL;
    local->on_selected = NULL;
    local->suggestions = NULL;
    local->suggestion_count = 0;
    local->current_index = EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;
    local->is_expanded = 0;
    local->max_visible_items = 4;
    local->read_only_mode = 0;
    local->pressed_part = EGUI_VIEW_AUTO_SUGGEST_BOX_PART_NONE;
    local->pressed_row = EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;
    local->collapsed_height = 34;
    local->item_height = 24;
    local->icon_text_gap = 6;
    local->popup_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->popup_border_color = EGUI_COLOR_HEX(0xDCE3EB);
    local->border_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->text_color = EGUI_COLOR_HEX(0x1A2734);
    local->muted_text_color = EGUI_COLOR_HEX(0x6B7A89);
    local->highlight_color = EGUI_COLOR_HEX(0x5B7FD6);
    local->icon_color = EGUI_COLOR_HEX(0x5C6B79);

    egui_view_textinput_set_font(self, (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    egui_view_textinput_set_placeholder(self, "Type to filter");
    egui_view_set_background(self, EGUI_BG_OF(&hcw_auto_suggest_box_standard_background));
    egui_view_set_padding(self, 28, 10, 8, 8);
    egui_view_set_shadow(self, NULL);
    egui_view_set_view_name(self, "egui_view_auto_suggest_box");
}
