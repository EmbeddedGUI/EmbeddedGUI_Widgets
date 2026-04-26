#include "egui_view_toolbar.h"

#define EGUI_VIEW_TOOLBAR_RADIUS          8
#define EGUI_VIEW_TOOLBAR_COMPACT_RADIUS  7
#define EGUI_VIEW_TOOLBAR_PAD_X           5
#define EGUI_VIEW_TOOLBAR_PAD_Y           5
#define EGUI_VIEW_TOOLBAR_ITEM_GAP        4
#define EGUI_VIEW_TOOLBAR_ICON_SIZE       18
#define EGUI_VIEW_TOOLBAR_COMPACT_ICON    16
#define EGUI_VIEW_TOOLBAR_LABEL_GAP       4

typedef struct egui_view_toolbar_metrics egui_view_toolbar_metrics_t;
struct egui_view_toolbar_metrics
{
    egui_region_t region;
    egui_region_t item_regions[EGUI_VIEW_TOOLBAR_MAX_ITEMS];
};

static uint8_t egui_view_toolbar_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static uint8_t egui_view_toolbar_text_len(const char *text)
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

static uint8_t egui_view_toolbar_is_space_char(char c)
{
    return (uint8_t)(c == ' ' || c == '\t');
}

static uint8_t egui_view_toolbar_is_break_after_char(char c)
{
    return (uint8_t)(c == '-' || c == '/');
}

static uint8_t egui_view_toolbar_find_elide_boundary(const char *text, uint8_t visible_chars)
{
    uint8_t index;

    if (text == NULL || visible_chars == 0)
    {
        return 0;
    }

    for (index = visible_chars; index > 0; --index)
    {
        if (egui_view_toolbar_is_space_char(text[index - 1]))
        {
            return (uint8_t)(index - 1);
        }
    }
    for (index = visible_chars; index > 0; --index)
    {
        if (egui_view_toolbar_is_break_after_char(text[index - 1]))
        {
            return index;
        }
    }
    return visible_chars;
}

static void egui_view_toolbar_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
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

    length = egui_view_toolbar_text_len(text);
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

    copy_length = egui_view_toolbar_find_elide_boundary(text, (uint8_t)(max_chars - 3));
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

static egui_dim_t egui_view_toolbar_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t width = 0;
    egui_dim_t height = 0;

    if (!egui_view_toolbar_has_text(text) || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }
    font->api->get_str_size(font, text, 0, 0, &width, &height);
    return width;
}

static void egui_view_toolbar_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width,
                                                egui_dim_t fallback_char_width)
{
    uint8_t max_chars;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }
    buffer[0] = '\0';
    if (!egui_view_toolbar_has_text(text) || max_width <= 0)
    {
        return;
    }

    max_chars = egui_view_toolbar_text_len(text);
    egui_view_toolbar_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t width = egui_view_toolbar_measure_text_width(font, buffer);

        if (width <= 0)
        {
            width = (egui_dim_t)egui_view_toolbar_text_len(buffer) * fallback_char_width;
        }
        if (width <= max_width)
        {
            break;
        }
        max_chars--;
        egui_view_toolbar_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_color_t egui_view_toolbar_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 66);
}

static uint8_t egui_view_toolbar_is_valid_index(egui_view_toolbar_t *local, uint8_t index)
{
    return (uint8_t)(index < local->item_count);
}

static uint8_t egui_view_toolbar_clear_pressed_state(egui_view_t *self, egui_view_toolbar_t *local)
{
    uint8_t had_pressed = (uint8_t)(self->is_pressed || local->pressed_index != EGUI_VIEW_TOOLBAR_INDEX_NONE);

    local->pressed_index = EGUI_VIEW_TOOLBAR_INDEX_NONE;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static uint8_t egui_view_toolbar_is_interactive(egui_view_toolbar_t *local, egui_view_t *self)
{
    return (uint8_t)(!local->read_only_mode && egui_view_get_enable(self));
}

static uint8_t egui_view_toolbar_find_next_enabled(egui_view_toolbar_t *local, uint8_t start, int8_t direction)
{
    uint8_t step;
    uint8_t index;

    if (local->item_count == 0)
    {
        return EGUI_VIEW_TOOLBAR_INDEX_NONE;
    }

    index = start;
    for (step = 0; step < local->item_count; ++step)
    {
        if (direction > 0)
        {
            index = (uint8_t)((index + 1U) % local->item_count);
        }
        else
        {
            index = index == 0 ? (uint8_t)(local->item_count - 1U) : (uint8_t)(index - 1U);
        }
        if (!local->items[index].disabled)
        {
            return index;
        }
    }
    return start;
}

static uint8_t egui_view_toolbar_first_enabled(egui_view_toolbar_t *local)
{
    uint8_t index;

    for (index = 0; index < local->item_count; ++index)
    {
        if (!local->items[index].disabled)
        {
            return index;
        }
    }
    return local->item_count > 0 ? 0 : EGUI_VIEW_TOOLBAR_INDEX_NONE;
}

static void egui_view_toolbar_normalize_current_index(egui_view_toolbar_t *local)
{
    uint8_t index;

    if (local->item_count == 0)
    {
        local->current_index = EGUI_VIEW_TOOLBAR_INDEX_NONE;
        return;
    }
    if (local->current_index < local->item_count && !local->items[local->current_index].disabled)
    {
        return;
    }
    for (index = 0; index < local->item_count; ++index)
    {
        if (local->items[index].checked && !local->items[index].disabled)
        {
            local->current_index = index;
            return;
        }
    }
    local->current_index = egui_view_toolbar_first_enabled(local);
}

static void egui_view_toolbar_get_metrics(egui_view_toolbar_t *local, egui_view_t *self, egui_view_toolbar_metrics_t *metrics)
{
    egui_dim_t pad_x = EGUI_VIEW_TOOLBAR_PAD_X;
    egui_dim_t pad_y = EGUI_VIEW_TOOLBAR_PAD_Y;
    egui_dim_t inner_width;
    egui_dim_t item_width;
    uint8_t index;

    egui_view_get_work_region(self, &metrics->region);
    for (index = 0; index < EGUI_VIEW_TOOLBAR_MAX_ITEMS; ++index)
    {
        metrics->item_regions[index] = metrics->region;
        metrics->item_regions[index].size.width = 0;
        metrics->item_regions[index].size.height = 0;
    }

    if (metrics->region.size.width <= pad_x * 2 || metrics->region.size.height <= pad_y * 2 || local->item_count == 0)
    {
        return;
    }

    inner_width = metrics->region.size.width - pad_x * 2 - EGUI_VIEW_TOOLBAR_ITEM_GAP * (egui_dim_t)(local->item_count - 1U);
    item_width = inner_width / local->item_count;
    if (item_width <= 0)
    {
        item_width = 1;
    }

    for (index = 0; index < local->item_count; ++index)
    {
        metrics->item_regions[index].location.x = metrics->region.location.x + pad_x + index * (item_width + EGUI_VIEW_TOOLBAR_ITEM_GAP);
        metrics->item_regions[index].location.y = metrics->region.location.y + pad_y;
        metrics->item_regions[index].size.width = item_width;
        metrics->item_regions[index].size.height = metrics->region.size.height - pad_y * 2;
    }
}

static int8_t egui_view_toolbar_hit_test(egui_view_t *self, const egui_view_toolbar_metrics_t *metrics, egui_dim_t screen_x, egui_dim_t screen_y,
                                         uint8_t item_count)
{
    egui_dim_t local_x = screen_x - self->region_screen.location.x;
    egui_dim_t local_y = screen_y - self->region_screen.location.y;
    uint8_t index;

    for (index = 0; index < item_count; ++index)
    {
        if (egui_region_pt_in_rect(&metrics->item_regions[index], local_x, local_y))
        {
            return (int8_t)index;
        }
    }
    return -1;
}

static void egui_view_toolbar_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                        egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !egui_view_toolbar_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, self->alpha);
}

static void egui_view_toolbar_draw_item(egui_view_t *self, egui_view_toolbar_t *local, const egui_view_toolbar_metrics_t *metrics, uint8_t index)
{
    egui_view_toolbar_item_t *item = &local->items[index];
    egui_region_t item_region = metrics->item_regions[index];
    egui_region_t icon_region;
    egui_region_t label_region;
    egui_color_t fill;
    egui_color_t border;
    egui_color_t text;
    egui_color_t icon;
    uint8_t checked = item->checked ? 1 : 0;
    uint8_t disabled = item->disabled ? 1 : 0;
    uint8_t is_current = (uint8_t)(index == local->current_index);
    uint8_t is_pressed = (uint8_t)(index == local->pressed_index && self->is_pressed);
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_TOOLBAR_COMPACT_RADIUS : EGUI_VIEW_TOOLBAR_RADIUS;
    char label[28];

    if (item_region.size.width <= 0 || item_region.size.height <= 0)
    {
        return;
    }

    fill = checked ? local->checked_color : local->item_color;
    if (is_pressed)
    {
        fill = local->pressed_color;
    }
    else if (is_current && !checked)
    {
        fill = egui_rgb_mix(fill, local->focus_color, 8);
    }
    border = is_current ? local->focus_color : local->border_color;
    text = checked ? local->text_color : local->muted_text_color;
    icon = checked ? local->checked_icon_color : local->icon_color;

    if (disabled || !egui_view_get_enable(self))
    {
        fill = egui_view_toolbar_mix_disabled(fill);
        border = egui_view_toolbar_mix_disabled(border);
        text = egui_view_toolbar_mix_disabled(text);
        icon = egui_view_toolbar_mix_disabled(icon);
    }
    else if (local->read_only_mode)
    {
        fill = egui_rgb_mix(fill, EGUI_COLOR_WHITE, 35);
        border = egui_rgb_mix(border, local->muted_text_color, 30);
        text = egui_rgb_mix(text, local->muted_text_color, 36);
        icon = egui_rgb_mix(icon, local->muted_text_color, 38);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, item_region.location.x, item_region.location.y, item_region.size.width,
                                          item_region.size.height, radius, fill, egui_color_alpha_mix(self->alpha, 94));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, item_region.location.x, item_region.location.y, item_region.size.width,
                                     item_region.size.height, radius, is_current && self->is_focused ? 2 : 1, border,
                                     egui_color_alpha_mix(self->alpha, is_current ? 76 : 48));

    if (checked)
    {
        egui_canvas_draw_rectangle_fill(&uicode_get_core()->canvas, item_region.location.x + 7, item_region.location.y + item_region.size.height - 3,
                                        item_region.size.width - 14, 2, local->checked_icon_color, egui_color_alpha_mix(self->alpha, 88));
    }

    if (local->compact_mode || item_region.size.width < 42)
    {
        icon_region = item_region;
        icon_region.size.width = EGUI_VIEW_TOOLBAR_COMPACT_ICON;
        icon_region.size.height = EGUI_VIEW_TOOLBAR_COMPACT_ICON;
        icon_region.location.x = item_region.location.x + (item_region.size.width - icon_region.size.width) / 2;
        icon_region.location.y = item_region.location.y + (item_region.size.height - icon_region.size.height) / 2;
        egui_view_toolbar_draw_text(local->icon_font, self, item->icon, &icon_region, EGUI_ALIGN_CENTER, icon);
        return;
    }

    icon_region = item_region;
    icon_region.location.x += 6;
    icon_region.location.y += (item_region.size.height - EGUI_VIEW_TOOLBAR_ICON_SIZE) / 2;
    icon_region.size.width = EGUI_VIEW_TOOLBAR_ICON_SIZE;
    icon_region.size.height = EGUI_VIEW_TOOLBAR_ICON_SIZE;
    egui_view_toolbar_draw_text(local->icon_font, self, item->icon, &icon_region, EGUI_ALIGN_CENTER, icon);

    label_region = item_region;
    label_region.location.x = icon_region.location.x + icon_region.size.width + EGUI_VIEW_TOOLBAR_LABEL_GAP;
    label_region.size.width = item_region.size.width - (label_region.location.x - item_region.location.x) - 5;
    label_region.location.y += 1;
    label_region.size.height -= 2;
    egui_view_toolbar_fit_text_to_width(local->label_font, item->label, label, sizeof(label), label_region.size.width, 5);
    egui_view_toolbar_draw_text(local->label_font, self, label, &label_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text);
}

static void egui_view_toolbar_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);
    egui_view_toolbar_metrics_t metrics;
    egui_color_t surface;
    egui_color_t border;
    uint8_t index;

    egui_view_toolbar_get_metrics(local, self, &metrics);
    if (metrics.region.size.width <= 0 || metrics.region.size.height <= 0)
    {
        return;
    }

    surface = local->surface_color;
    border = local->border_color;
    if (!egui_view_get_enable(self))
    {
        surface = egui_view_toolbar_mix_disabled(surface);
        border = egui_view_toolbar_mix_disabled(border);
    }
    else if (local->read_only_mode)
    {
        surface = egui_rgb_mix(surface, EGUI_COLOR_WHITE, 30);
        border = egui_rgb_mix(border, local->muted_text_color, 26);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.region.location.x, metrics.region.location.y, metrics.region.size.width,
                                          metrics.region.size.height, 12, surface, egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.region.location.x, metrics.region.location.y, metrics.region.size.width,
                                     metrics.region.size.height, 12, 1, border, egui_color_alpha_mix(self->alpha, 54));

    for (index = 0; index < local->item_count; ++index)
    {
        egui_view_toolbar_draw_item(self, local, &metrics, index);
    }
}

void egui_view_toolbar_set_items(egui_view_t *self, const egui_view_toolbar_item_t *items, uint8_t item_count)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);
    uint8_t index;
    uint8_t count = items == NULL ? 0 : item_count;

    egui_view_toolbar_clear_pressed_state(self, local);
    if (count > EGUI_VIEW_TOOLBAR_MAX_ITEMS)
    {
        count = EGUI_VIEW_TOOLBAR_MAX_ITEMS;
    }

    for (index = 0; index < EGUI_VIEW_TOOLBAR_MAX_ITEMS; ++index)
    {
        local->items[index].label = "";
        local->items[index].icon = NULL;
        local->items[index].kind = EGUI_VIEW_TOOLBAR_ITEM_BUTTON;
        local->items[index].checked = 0;
        local->items[index].disabled = 0;
    }

    for (index = 0; index < count; ++index)
    {
        local->items[index] = items[index];
        if (local->items[index].label == NULL)
        {
            local->items[index].label = "";
        }
        if (local->items[index].kind != EGUI_VIEW_TOOLBAR_ITEM_TOGGLE)
        {
            local->items[index].kind = EGUI_VIEW_TOOLBAR_ITEM_BUTTON;
            local->items[index].checked = 0;
        }
        local->items[index].disabled = local->items[index].disabled ? 1 : 0;
        local->items[index].checked = local->items[index].checked ? 1 : 0;
    }

    local->item_count = count;
    local->current_index = EGUI_VIEW_TOOLBAR_INDEX_NONE;
    for (index = 0; index < local->item_count; ++index)
    {
        if (local->items[index].checked && !local->items[index].disabled)
        {
            local->current_index = index;
            break;
        }
    }
    egui_view_toolbar_normalize_current_index(local);
    egui_view_invalidate(self);
}

void egui_view_toolbar_set_current_index(egui_view_t *self, uint8_t index)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);

    egui_view_toolbar_clear_pressed_state(self, local);
    if (egui_view_toolbar_is_valid_index(local, index))
    {
        local->current_index = index;
    }
    egui_view_toolbar_normalize_current_index(local);
    egui_view_invalidate(self);
}

void egui_view_toolbar_set_item_checked(egui_view_t *self, uint8_t index, uint8_t checked)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);
    uint8_t item_index;

    if (!egui_view_toolbar_is_valid_index(local, index))
    {
        return;
    }

    egui_view_toolbar_clear_pressed_state(self, local);
    if (local->items[index].kind == EGUI_VIEW_TOOLBAR_ITEM_TOGGLE)
    {
        for (item_index = 0; item_index < local->item_count; ++item_index)
        {
            if (local->items[item_index].kind == EGUI_VIEW_TOOLBAR_ITEM_TOGGLE)
            {
                local->items[item_index].checked = 0;
            }
        }
        local->items[index].checked = checked ? 1 : 0;
        if (checked)
        {
            local->current_index = index;
        }
    }
    egui_view_toolbar_normalize_current_index(local);
    egui_view_invalidate(self);
}

void egui_view_toolbar_set_item_disabled(egui_view_t *self, uint8_t index, uint8_t disabled)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);

    if (!egui_view_toolbar_is_valid_index(local, index))
    {
        return;
    }

    egui_view_toolbar_clear_pressed_state(self, local);
    local->items[index].disabled = disabled ? 1 : 0;
    egui_view_toolbar_normalize_current_index(local);
    egui_view_invalidate(self);
}

void egui_view_toolbar_set_fonts(egui_view_t *self, const egui_font_t *label_font, const egui_font_t *icon_font)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);

    egui_view_toolbar_clear_pressed_state(self, local);
    local->label_font = label_font != NULL ? label_font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->icon_font = icon_font != NULL ? icon_font : EGUI_FONT_ICON_MS_16;
    egui_view_invalidate(self);
}

void egui_view_toolbar_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);

    egui_view_toolbar_clear_pressed_state(self, local);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

void egui_view_toolbar_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);

    egui_view_toolbar_clear_pressed_state(self, local);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

void egui_view_toolbar_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t item_color, egui_color_t checked_color,
                                   egui_color_t pressed_color, egui_color_t border_color, egui_color_t focus_color, egui_color_t text_color,
                                   egui_color_t muted_text_color, egui_color_t icon_color, egui_color_t checked_icon_color)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);

    egui_view_toolbar_clear_pressed_state(self, local);
    local->surface_color = surface_color;
    local->item_color = item_color;
    local->checked_color = checked_color;
    local->pressed_color = pressed_color;
    local->border_color = border_color;
    local->focus_color = focus_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->icon_color = icon_color;
    local->checked_icon_color = checked_icon_color;
    egui_view_invalidate(self);
}

void egui_view_toolbar_set_on_action_listener(egui_view_t *self, egui_view_toolbar_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);
    local->on_action = listener;
}

uint8_t egui_view_toolbar_activate_item(egui_view_t *self, uint8_t index)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);
    uint8_t item_index;

    if (!egui_view_toolbar_is_interactive(local, self) || !egui_view_toolbar_is_valid_index(local, index) || local->items[index].disabled)
    {
        return 0;
    }

    local->current_index = index;
    if (local->items[index].kind == EGUI_VIEW_TOOLBAR_ITEM_TOGGLE)
    {
        for (item_index = 0; item_index < local->item_count; ++item_index)
        {
            if (local->items[item_index].kind == EGUI_VIEW_TOOLBAR_ITEM_TOGGLE)
            {
                local->items[item_index].checked = 0;
            }
        }
        local->items[index].checked = 1;
    }
    if (local->on_action != NULL)
    {
        local->on_action(self, index);
    }
    egui_view_invalidate(self);
    return 1;
}

uint8_t egui_view_toolbar_get_item_region(egui_view_t *self, uint8_t index, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);
    egui_view_toolbar_metrics_t metrics;

    if (region == NULL || !egui_view_toolbar_is_valid_index(local, index))
    {
        return 0;
    }
    egui_view_toolbar_get_metrics(local, self, &metrics);
    *region = metrics.item_regions[index];
    region->location.x += self->region_screen.location.x;
    region->location.y += self->region_screen.location.y;
    return region->size.width > 0 && region->size.height > 0 ? 1 : 0;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_toolbar_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);
    egui_view_toolbar_metrics_t metrics;
    int8_t hit;

    if (!egui_view_toolbar_is_interactive(local, self))
    {
        if (egui_view_toolbar_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    egui_view_toolbar_get_metrics(local, self, &metrics);
    hit = egui_view_toolbar_hit_test(self, &metrics, event->location.x, event->location.y, local->item_count);
    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        if (hit < 0 || local->items[(uint8_t)hit].disabled)
        {
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        local->current_index = (uint8_t)hit;
        local->pressed_index = (uint8_t)hit;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_index == EGUI_VIEW_TOOLBAR_INDEX_NONE)
        {
            return 0;
        }
        egui_view_set_pressed(self, (hit >= 0 && (uint8_t)hit == local->pressed_index) ? true : false);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t pressed_index = local->pressed_index;
        uint8_t had_target = egui_view_toolbar_is_valid_index(local, pressed_index);

        if (had_target && self->is_pressed && hit >= 0 && (uint8_t)hit == pressed_index)
        {
            egui_view_toolbar_activate_item(self, pressed_index);
        }
        if (egui_view_toolbar_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return had_target || hit >= 0;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (egui_view_toolbar_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        break;
    }
    return 0;
}

static int egui_view_toolbar_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);
    EGUI_UNUSED(event);

    if (egui_view_toolbar_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_toolbar_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);
    uint8_t current = local->current_index;
    uint8_t was_pressed = (uint8_t)(local->pressed_index == current && self->is_pressed);

    if (!egui_view_toolbar_is_interactive(local, self))
    {
        if (egui_view_toolbar_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }
    if (local->item_count == 0)
    {
        return 0;
    }
    egui_view_toolbar_normalize_current_index(local);
    current = local->current_index;

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_LEFT:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->current_index = egui_view_toolbar_find_next_enabled(local, current, -1);
            egui_view_toolbar_clear_pressed_state(self, local);
            egui_view_invalidate(self);
            return 1;
        }
        return 1;
    case EGUI_KEY_CODE_RIGHT:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->current_index = egui_view_toolbar_find_next_enabled(local, current, 1);
            egui_view_toolbar_clear_pressed_state(self, local);
            egui_view_invalidate(self);
            return 1;
        }
        return 1;
    case EGUI_KEY_CODE_HOME:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->current_index = egui_view_toolbar_first_enabled(local);
            egui_view_toolbar_clear_pressed_state(self, local);
            egui_view_invalidate(self);
            return 1;
        }
        return 1;
    case EGUI_KEY_CODE_END:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->current_index = egui_view_toolbar_find_next_enabled(local, 0, -1);
            egui_view_toolbar_clear_pressed_state(self, local);
            egui_view_invalidate(self);
            return 1;
        }
        return 1;
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (!egui_view_toolbar_is_valid_index(local, current) || local->items[current].disabled)
        {
            egui_view_toolbar_clear_pressed_state(self, local);
            return 0;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->pressed_index = current;
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            if (was_pressed)
            {
                egui_view_toolbar_activate_item(self, current);
            }
            egui_view_toolbar_clear_pressed_state(self, local);
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        if (egui_view_toolbar_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_toolbar_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_toolbar_t);
    EGUI_UNUSED(event);

    if (egui_view_toolbar_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_toolbar_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_toolbar_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_toolbar_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_toolbar_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_toolbar_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_toolbar_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_toolbar_on_key_event,
#endif
};

void egui_view_toolbar_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_toolbar_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_toolbar_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->label_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->icon_font = EGUI_FONT_ICON_MS_16;
    local->on_action = NULL;
    local->item_count = 0;
    local->current_index = EGUI_VIEW_TOOLBAR_INDEX_NONE;
    local->pressed_index = EGUI_VIEW_TOOLBAR_INDEX_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->item_color = EGUI_COLOR_HEX(0xF7F9FC);
    local->checked_color = EGUI_COLOR_HEX(0xE7F1FB);
    local->pressed_color = EGUI_COLOR_HEX(0xD9EAFB);
    local->border_color = EGUI_COLOR_HEX(0xC9D3DD);
    local->focus_color = EGUI_COLOR_HEX(0x78B7F2);
    local->text_color = EGUI_COLOR_HEX(0x182331);
    local->muted_text_color = EGUI_COLOR_HEX(0x5E6B78);
    local->icon_color = EGUI_COLOR_HEX(0x5E6B78);
    local->checked_icon_color = EGUI_COLOR_HEX(0x0F6CBD);
    egui_view_set_view_name(self, "egui_view_toolbar");
}
