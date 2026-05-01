#include "egui_view_persona_group.h"

#define EGUI_VIEW_PERSONA_GROUP_STANDARD_PAD_X         8
#define EGUI_VIEW_PERSONA_GROUP_STANDARD_PAD_Y         7
#define EGUI_VIEW_PERSONA_GROUP_STANDARD_RADIUS        10
#define EGUI_VIEW_PERSONA_GROUP_STANDARD_AVATAR_SIZE   20
#define EGUI_VIEW_PERSONA_GROUP_STANDARD_AVATAR_STEP   15
#define EGUI_VIEW_PERSONA_GROUP_STANDARD_HEADER_HEIGHT 8
#define EGUI_VIEW_PERSONA_GROUP_STANDARD_TITLE_HEIGHT  11
#define EGUI_VIEW_PERSONA_GROUP_STANDARD_FOOTER_HEIGHT 11

#define EGUI_VIEW_PERSONA_GROUP_DENSE_PAD_X         6
#define EGUI_VIEW_PERSONA_GROUP_DENSE_PAD_Y         5
#define EGUI_VIEW_PERSONA_GROUP_DENSE_RADIUS        8
#define EGUI_VIEW_PERSONA_GROUP_DENSE_AVATAR_SIZE   15
#define EGUI_VIEW_PERSONA_GROUP_DENSE_AVATAR_STEP   12
#define EGUI_VIEW_PERSONA_GROUP_DENSE_TITLE_HEIGHT  9
#define EGUI_VIEW_PERSONA_GROUP_DENSE_FOOTER_HEIGHT 10

typedef struct egui_view_persona_group_metrics egui_view_persona_group_metrics_t;
struct egui_view_persona_group_metrics
{
    egui_region_t content_region;
    egui_region_t avatar_regions[EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS];
    egui_region_t overflow_region;
    egui_region_t name_region;
    egui_region_t role_region;
    egui_region_t footer_region;
    egui_region_t eyebrow_region;
    egui_region_t title_region;
    uint8_t bubble_count;
    uint8_t dense_layout;
};

static const egui_color_t egui_view_persona_group_avatar_palette[] = {
        EGUI_COLOR_HEX(0x0F6CBD),
        EGUI_COLOR_HEX(0x0F7B45),
        EGUI_COLOR_HEX(0x9D5D00),
        EGUI_COLOR_HEX(0x7A8796),
};

static uint8_t egui_view_persona_group_clamp_snapshot_count(uint8_t count)
{
    if (count > EGUI_VIEW_PERSONA_GROUP_MAX_SNAPSHOTS)
    {
        return EGUI_VIEW_PERSONA_GROUP_MAX_SNAPSHOTS;
    }
    return count;
}

static uint8_t egui_view_persona_group_clamp_item_count(uint8_t count)
{
    if (count > EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS)
    {
        return EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS;
    }
    return count;
}

static uint8_t egui_view_persona_group_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static uint8_t egui_view_persona_group_text_len(const char *text)
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

static egui_dim_t egui_view_persona_group_measure_font_line_height(const egui_font_t *font)
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

static egui_dim_t egui_view_persona_group_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t text_width = 0;
    egui_dim_t dummy_height = 0;

    if (!egui_view_persona_group_has_text(text) || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, text, 0, 0, &text_width, &dummy_height);
    return text_width;
}

static void egui_view_persona_group_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
{
    uint8_t capacity;
    uint8_t allowed_chars;
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

    capacity = buffer_size - 1;
    length = egui_view_persona_group_text_len(text);
    if (length <= max_chars && length <= capacity)
    {
        for (index = 0; index < length; ++index)
        {
            buffer[index] = text[index];
        }
        buffer[length] = '\0';
        return;
    }

    allowed_chars = max_chars;
    if (allowed_chars > capacity)
    {
        allowed_chars = capacity;
    }
    if (allowed_chars == 0)
    {
        return;
    }

    if (allowed_chars <= 3)
    {
        for (index = 0; index < allowed_chars; ++index)
        {
            buffer[index] = '.';
        }
        buffer[allowed_chars] = '\0';
        return;
    }

    copy_length = allowed_chars - 3;
    for (index = 0; index < copy_length; ++index)
    {
        buffer[index] = text[index];
    }
    buffer[copy_length] = '.';
    buffer[copy_length + 1] = '.';
    buffer[copy_length + 2] = '.';
    buffer[copy_length + 3] = '\0';
}

static void egui_view_persona_group_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width,
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

    max_chars = egui_view_persona_group_text_len(text);
    egui_view_persona_group_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t text_width = egui_view_persona_group_measure_text_width(font, buffer);

        if (text_width <= 0)
        {
            text_width = (egui_dim_t)egui_view_persona_group_text_len(buffer) * fallback_char_width;
        }
        if (text_width <= max_width)
        {
            break;
        }

        max_chars--;
        egui_view_persona_group_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_dim_t egui_view_persona_group_get_eyebrow_height(egui_view_persona_group_t *local)
{
    egui_dim_t eyebrow_h = egui_view_persona_group_measure_font_line_height(local->meta_font);

    return eyebrow_h > EGUI_VIEW_PERSONA_GROUP_STANDARD_HEADER_HEIGHT ? eyebrow_h : EGUI_VIEW_PERSONA_GROUP_STANDARD_HEADER_HEIGHT;
}

static uint8_t egui_view_persona_group_should_use_dense_layout(const egui_region_t *work_region)
{
    return (uint8_t)(work_region->size.width <= 132 || work_region->size.height <= 90);
}

static egui_dim_t egui_view_persona_group_get_title_height(egui_view_persona_group_t *local, uint8_t dense_layout)
{
    egui_dim_t title_h = egui_view_persona_group_measure_font_line_height(local->font);
    egui_dim_t min_h = dense_layout ? EGUI_VIEW_PERSONA_GROUP_DENSE_TITLE_HEIGHT : EGUI_VIEW_PERSONA_GROUP_STANDARD_TITLE_HEIGHT;

    return title_h > min_h ? title_h : min_h;
}

static egui_dim_t egui_view_persona_group_get_name_height(egui_view_persona_group_t *local, uint8_t dense_layout)
{
    egui_dim_t name_h = egui_view_persona_group_measure_font_line_height(local->font);
    egui_dim_t min_h = dense_layout ? 9 : 10;

    return name_h > min_h ? name_h : min_h;
}

static egui_dim_t egui_view_persona_group_get_role_height(egui_view_persona_group_t *local, uint8_t dense_layout)
{
    egui_dim_t role_h;

    if (dense_layout)
    {
        return 0;
    }

    role_h = egui_view_persona_group_measure_font_line_height(local->meta_font);
    return role_h > 8 ? role_h : 8;
}

static egui_dim_t egui_view_persona_group_get_footer_height(egui_view_persona_group_t *local, uint8_t dense_layout)
{
    egui_dim_t footer_h = egui_view_persona_group_measure_font_line_height(local->meta_font);
    egui_dim_t min_h = dense_layout ? EGUI_VIEW_PERSONA_GROUP_DENSE_FOOTER_HEIGHT : EGUI_VIEW_PERSONA_GROUP_STANDARD_FOOTER_HEIGHT;

    return footer_h > min_h ? footer_h : min_h;
}

static egui_color_t egui_view_persona_group_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 66);
}

static uint8_t egui_view_persona_group_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_group_t);
    uint8_t was_pressed = self->is_pressed ? 1 : 0;
    uint8_t had_pressed = (uint8_t)(was_pressed || local->pressed_index != EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS);

    if (!had_pressed)
    {
        return 0;
    }

    local->pressed_index = EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS;
    if (was_pressed)
    {
        egui_view_set_pressed(self, false);
    }
    else
    {
        egui_view_invalidate(self);
    }
    return 1;
}

static egui_color_t egui_view_persona_group_presence_color(egui_view_persona_group_t *local, uint8_t presence)
{
    switch (presence)
    {
    case EGUI_VIEW_PERSONA_GROUP_PRESENCE_BUSY:
        return local->warning_color;
    case EGUI_VIEW_PERSONA_GROUP_PRESENCE_AWAY:
        return egui_rgb_mix(local->warning_color, local->accent_color, 24);
    case EGUI_VIEW_PERSONA_GROUP_PRESENCE_IDLE:
        return local->neutral_color;
    default:
        return local->success_color;
    }
}

static const egui_view_persona_group_snapshot_t *egui_view_persona_group_get_snapshot(egui_view_persona_group_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }

    return &local->snapshots[local->current_snapshot];
}

static uint8_t egui_view_persona_group_focus_index(const egui_view_persona_group_snapshot_t *snapshot, uint8_t item_count)
{
    if (snapshot == NULL || item_count == 0 || snapshot->focus_index >= item_count)
    {
        return 0;
    }
    return snapshot->focus_index;
}

static const egui_view_persona_group_item_t *egui_view_persona_group_get_item(egui_view_persona_group_t *local)
{
    const egui_view_persona_group_snapshot_t *snapshot = egui_view_persona_group_get_snapshot(local);

    if (snapshot == NULL || snapshot->items == NULL || snapshot->item_count == 0)
    {
        return NULL;
    }
    if (local->current_index >= snapshot->item_count)
    {
        return NULL;
    }

    return &snapshot->items[local->current_index];
}

static void egui_view_persona_group_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                              egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (text == NULL || text[0] == '\0')
    {
        return;
    }

    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, self->alpha);
}

static egui_dim_t egui_view_persona_group_footer_width(const egui_font_t *font, const char *text, uint8_t dense_layout, egui_dim_t max_width)
{
    egui_dim_t min_width = dense_layout ? 18 : 24;
    egui_dim_t fallback_char_width = dense_layout ? 4 : 5;
    egui_dim_t width = min_width + egui_view_persona_group_measure_text_width(font, text);

    if (width <= min_width)
    {
        width = min_width + egui_view_persona_group_text_len(text) * fallback_char_width;
    }
    if (width > max_width)
    {
        width = max_width;
    }

    return width;
}

static void egui_view_persona_group_notify_change(egui_view_t *self, egui_view_persona_group_t *local)
{
    if (local->on_focus_changed)
    {
        local->on_focus_changed(self, local->current_snapshot, local->current_index);
    }
}

static void egui_view_persona_group_set_current_index_inner(egui_view_t *self, uint8_t item_index, uint8_t notify);
static void egui_view_persona_group_get_metrics(egui_view_persona_group_t *local, egui_view_t *self, egui_view_persona_group_metrics_t *metrics)
{
    const egui_view_persona_group_snapshot_t *snapshot = egui_view_persona_group_get_snapshot(local);
    egui_region_t work_region;
    uint8_t dense_layout;
    egui_dim_t pad_x;
    egui_dim_t pad_y;
    egui_dim_t avatar_size;
    egui_dim_t avatar_step;
    egui_dim_t bubble_count;
    egui_dim_t row_width;
    egui_dim_t start_x;
    egui_dim_t row_y;
    egui_dim_t eyebrow_h;
    egui_dim_t title_h;
    egui_dim_t name_h;
    egui_dim_t role_h;
    egui_dim_t footer_h;
    egui_dim_t overflow_size;
    uint8_t item_count = 0;
    uint8_t i;

    egui_view_get_work_region(self, &work_region);
    dense_layout = egui_view_persona_group_should_use_dense_layout(&work_region);
    pad_x = dense_layout ? EGUI_VIEW_PERSONA_GROUP_DENSE_PAD_X : EGUI_VIEW_PERSONA_GROUP_STANDARD_PAD_X;
    pad_y = dense_layout ? EGUI_VIEW_PERSONA_GROUP_DENSE_PAD_Y : EGUI_VIEW_PERSONA_GROUP_STANDARD_PAD_Y;
    avatar_size = dense_layout ? EGUI_VIEW_PERSONA_GROUP_DENSE_AVATAR_SIZE : EGUI_VIEW_PERSONA_GROUP_STANDARD_AVATAR_SIZE;
    avatar_step = dense_layout ? EGUI_VIEW_PERSONA_GROUP_DENSE_AVATAR_STEP : EGUI_VIEW_PERSONA_GROUP_STANDARD_AVATAR_STEP;
    eyebrow_h = dense_layout ? 0 : egui_view_persona_group_get_eyebrow_height(local);
    title_h = egui_view_persona_group_get_title_height(local, dense_layout);
    name_h = egui_view_persona_group_get_name_height(local, dense_layout);
    role_h = egui_view_persona_group_get_role_height(local, dense_layout);
    footer_h = egui_view_persona_group_get_footer_height(local, dense_layout);
    overflow_size = avatar_size - (dense_layout ? 1 : 2);

    metrics->dense_layout = dense_layout;
    metrics->content_region.location.x = work_region.location.x + pad_x;
    metrics->content_region.location.y = work_region.location.y + pad_y;
    metrics->content_region.size.width = work_region.size.width - pad_x * 2;
    metrics->content_region.size.height = work_region.size.height - pad_y * 2;

    for (i = 0; i < EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS; i++)
    {
        metrics->avatar_regions[i].location.x = 0;
        metrics->avatar_regions[i].location.y = 0;
        metrics->avatar_regions[i].size.width = 0;
        metrics->avatar_regions[i].size.height = 0;
    }

    metrics->overflow_region.location.x = 0;
    metrics->overflow_region.location.y = 0;
    metrics->overflow_region.size.width = 0;
    metrics->overflow_region.size.height = 0;

    if (snapshot != NULL)
    {
        item_count = egui_view_persona_group_clamp_item_count(snapshot->item_count);
    }

    metrics->bubble_count = item_count;
    if (snapshot != NULL && snapshot->overflow_count > 0)
    {
        metrics->bubble_count++;
    }

    bubble_count = metrics->bubble_count > 0 ? metrics->bubble_count : 1;
    row_width = avatar_size + (bubble_count - 1) * avatar_step;
    if (row_width > metrics->content_region.size.width)
    {
        row_width = metrics->content_region.size.width;
    }
    start_x = metrics->content_region.location.x + (metrics->content_region.size.width - row_width) / 2;

    metrics->eyebrow_region.location.x = metrics->content_region.location.x + 2;
    metrics->eyebrow_region.location.y = metrics->content_region.location.y + 1;
    metrics->eyebrow_region.size.width = metrics->content_region.size.width - 4;
    metrics->eyebrow_region.size.height = eyebrow_h;

    metrics->title_region.location.x = metrics->content_region.location.x + 2;
    metrics->title_region.location.y = dense_layout ? (metrics->content_region.location.y + 4) : (metrics->eyebrow_region.location.y + eyebrow_h + 3);
    metrics->title_region.size.width = metrics->content_region.size.width - 4;
    metrics->title_region.size.height = title_h;

    row_y = metrics->title_region.location.y + title_h + 6;

    for (i = 0; i < item_count; i++)
    {
        metrics->avatar_regions[i].location.x = start_x + i * avatar_step;
        metrics->avatar_regions[i].location.y = row_y;
        metrics->avatar_regions[i].size.width = avatar_size;
        metrics->avatar_regions[i].size.height = avatar_size;
    }

    if (snapshot != NULL && snapshot->overflow_count > 0)
    {
        metrics->overflow_region.location.x = start_x + item_count * avatar_step + (avatar_size - overflow_size) / 2;
        metrics->overflow_region.location.y = row_y + (avatar_size - overflow_size) / 2;
        metrics->overflow_region.size.width = overflow_size;
        metrics->overflow_region.size.height = overflow_size;
    }

    metrics->name_region.location.x = metrics->content_region.location.x + 4;
    metrics->name_region.location.y = row_y + avatar_size + (dense_layout ? 5 : 7);
    metrics->name_region.size.width = metrics->content_region.size.width - 8;
    metrics->name_region.size.height = name_h;

    metrics->role_region.location.x = metrics->content_region.location.x + 4;
    metrics->role_region.location.y = metrics->name_region.location.y + metrics->name_region.size.height + 1;
    metrics->role_region.size.width = metrics->content_region.size.width - 8;
    metrics->role_region.size.height = role_h;

    metrics->footer_region.size.height = footer_h;
    metrics->footer_region.location.y = metrics->content_region.location.y + metrics->content_region.size.height - footer_h;
    metrics->footer_region.location.x = metrics->content_region.location.x + 4;
    metrics->footer_region.size.width = metrics->content_region.size.width - 8;
}

static uint8_t egui_view_persona_group_hit_index(egui_view_persona_group_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    const egui_view_persona_group_snapshot_t *snapshot = egui_view_persona_group_get_snapshot(local);
    egui_view_persona_group_metrics_t metrics;
    uint8_t item_count;
    uint8_t i;

    if (snapshot == NULL)
    {
        return EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS;
    }

    item_count = egui_view_persona_group_clamp_item_count(snapshot->item_count);
    egui_view_persona_group_get_metrics(local, self, &metrics);

    for (i = 0; i < item_count; i++)
    {
        if (egui_region_pt_in_rect(&metrics.avatar_regions[i], x, y))
        {
            return i;
        }
    }

    return EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS;
}

static void egui_view_persona_group_set_current_snapshot_inner(egui_view_t *self, uint8_t snapshot_index, uint8_t notify)
{
    EGUI_LOCAL_INIT(egui_view_persona_group_t);
    const egui_view_persona_group_snapshot_t *snapshot;
    uint8_t item_count;

    if (local->snapshots == NULL || local->snapshot_count == 0 || snapshot_index >= local->snapshot_count)
    {
        if (egui_view_persona_group_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }
    if (local->current_snapshot == snapshot_index)
    {
        if (egui_view_persona_group_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_snapshot = snapshot_index;
    snapshot = egui_view_persona_group_get_snapshot(local);
    item_count = snapshot == NULL ? 0 : egui_view_persona_group_clamp_item_count(snapshot->item_count);
    if (snapshot != NULL && snapshot->items != NULL && item_count > 0)
    {
        local->current_index = egui_view_persona_group_focus_index(snapshot, item_count);
    }
    else
    {
        local->current_index = 0;
    }
    egui_view_persona_group_clear_pressed_state(self);
    if (notify)
    {
        egui_view_persona_group_notify_change(self, local);
    }
    egui_view_invalidate(self);
}

static void egui_view_persona_group_set_current_index_inner(egui_view_t *self, uint8_t item_index, uint8_t notify)
{
    EGUI_LOCAL_INIT(egui_view_persona_group_t);
    const egui_view_persona_group_snapshot_t *snapshot = egui_view_persona_group_get_snapshot(local);
    uint8_t item_count = snapshot == NULL ? 0 : egui_view_persona_group_clamp_item_count(snapshot->item_count);

    if (snapshot == NULL || snapshot->items == NULL || item_count == 0)
    {
        local->current_index = 0;
        if (egui_view_persona_group_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }
    if (item_index >= item_count)
    {
        if (egui_view_persona_group_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }
    if (local->current_index == item_index)
    {
        if (egui_view_persona_group_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_index = item_index;
    egui_view_persona_group_clear_pressed_state(self);
    if (notify)
    {
        egui_view_persona_group_notify_change(self, local);
    }
    egui_view_invalidate(self);
}

void egui_view_persona_group_set_snapshots(egui_view_t *self, const egui_view_persona_group_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_persona_group_t);
    const egui_view_persona_group_snapshot_t *snapshot;
    uint8_t item_count;

    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : egui_view_persona_group_clamp_snapshot_count(snapshot_count);
    local->current_snapshot = 0;
    local->current_index = 0;
    egui_view_persona_group_clear_pressed_state(self);

    snapshot = egui_view_persona_group_get_snapshot(local);
    item_count = snapshot == NULL ? 0 : egui_view_persona_group_clamp_item_count(snapshot->item_count);
    if (snapshot != NULL && snapshot->items != NULL && item_count > 0)
    {
        local->current_index = egui_view_persona_group_focus_index(snapshot, item_count);
    }

    egui_view_invalidate(self);
}

void egui_view_persona_group_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    egui_view_persona_group_set_current_snapshot_inner(self, snapshot_index, 1);
}

uint8_t egui_view_persona_group_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_group_t);
    return local->current_snapshot;
}

void egui_view_persona_group_set_current_index(egui_view_t *self, uint8_t item_index)
{
    egui_view_persona_group_set_current_index_inner(self, item_index, 1);
}

uint8_t egui_view_persona_group_get_current_index(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_group_t);
    return local->current_index;
}

void egui_view_persona_group_set_on_focus_changed_listener(egui_view_t *self, egui_view_on_persona_group_focus_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_persona_group_t);
    local->on_focus_changed = listener;
}

void egui_view_persona_group_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_persona_group_t);
    egui_view_persona_group_clear_pressed_state(self);
    local->font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_persona_group_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_persona_group_t);
    egui_view_persona_group_clear_pressed_state(self);
    local->meta_font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_persona_group_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t section_color,
                                         egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t success_color,
                                         egui_color_t warning_color, egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_persona_group_t);
    egui_view_persona_group_clear_pressed_state(self);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->section_color = section_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    egui_view_invalidate(self);
}
static void egui_view_persona_group_draw_avatar(egui_view_t *self, egui_view_persona_group_t *local, const egui_view_persona_group_item_t *item,
                                                const egui_region_t *region, uint8_t selected, uint8_t pressed)
{
    egui_region_t text_region;
    egui_color_t fill_color = egui_view_persona_group_avatar_palette[item->tone % 4];
    egui_color_t ring_color = selected ? egui_rgb_mix(local->border_color, local->accent_color, 38) : egui_rgb_mix(local->border_color, fill_color, 10);
    egui_color_t text_color = EGUI_COLOR_HEX(0xFFFFFF);
    egui_color_t presence_color = egui_view_persona_group_presence_color(local, item->presence);
    egui_dim_t center_x = region->location.x + region->size.width / 2;
    egui_dim_t center_y = region->location.y + region->size.height / 2;
    egui_dim_t radius = region->size.width / 2;

    if (item->emphasized)
    {
        fill_color = egui_rgb_mix(fill_color, local->surface_color, 18);
    }
    if (pressed)
    {
        fill_color = egui_rgb_mix(fill_color, EGUI_COLOR_BLACK, 10);
    }
    if (!egui_view_get_enable(self))
    {
        fill_color = egui_view_persona_group_mix_disabled(fill_color);
        ring_color = egui_view_persona_group_mix_disabled(ring_color);
        text_color = egui_view_persona_group_mix_disabled(text_color);
        presence_color = egui_view_persona_group_mix_disabled(presence_color);
    }

    if (selected)
    {
        egui_canvas_draw_circle_basic(&uicode_get_core()->canvas, center_x, center_y, radius + 2, 1, ring_color, egui_color_alpha_mix(self->alpha, 56));
    }

    egui_canvas_draw_circle_fill_basic(&uicode_get_core()->canvas, center_x, center_y, radius, fill_color, egui_color_alpha_mix(self->alpha, 84));
    egui_canvas_draw_circle_basic(&uicode_get_core()->canvas, center_x, center_y, radius, 1, egui_rgb_mix(fill_color, local->surface_color, 24), egui_color_alpha_mix(self->alpha, 24));

    text_region.location.x = region->location.x;
    text_region.location.y = region->location.y;
    text_region.size.width = region->size.width;
    text_region.size.height = region->size.height;
    egui_view_persona_group_draw_text(local->meta_font, self, item->initials, &text_region, EGUI_ALIGN_CENTER, text_color);

    egui_canvas_draw_circle_fill_basic(&uicode_get_core()->canvas, region->location.x + region->size.width - 4, region->location.y + region->size.height - 4, 3, EGUI_COLOR_HEX(0xFFFFFF),
                                       egui_color_alpha_mix(self->alpha, 88));
    egui_canvas_draw_circle_fill_basic(&uicode_get_core()->canvas, region->location.x + region->size.width - 4, region->location.y + region->size.height - 4, 2, presence_color,
                                       egui_color_alpha_mix(self->alpha, 86));
}

static void egui_view_persona_group_draw_overflow(egui_view_t *self, egui_view_persona_group_t *local, const egui_region_t *region, uint8_t overflow_count)
{
    egui_region_t text_region;
    egui_color_t fill_color = egui_rgb_mix(local->surface_color, local->section_color, 12);
    egui_color_t border_color = egui_rgb_mix(local->border_color, local->section_color, 12);
    egui_color_t text_color = local->muted_text_color;
    char text[4];
    egui_dim_t center_x = region->location.x + region->size.width / 2;
    egui_dim_t center_y = region->location.y + region->size.height / 2;
    egui_dim_t radius = region->size.width / 2;

    text[0] = '+';
    if (overflow_count > 9)
    {
        overflow_count = 9;
    }
    text[1] = (char)('0' + overflow_count);
    text[2] = '\0';

    if (!egui_view_get_enable(self))
    {
        fill_color = egui_view_persona_group_mix_disabled(fill_color);
        border_color = egui_view_persona_group_mix_disabled(border_color);
        text_color = egui_view_persona_group_mix_disabled(text_color);
    }

    egui_canvas_draw_circle_fill_basic(&uicode_get_core()->canvas, center_x, center_y, radius, fill_color, egui_color_alpha_mix(self->alpha, 84));
    egui_canvas_draw_circle_basic(&uicode_get_core()->canvas, center_x, center_y, radius, 1, border_color, egui_color_alpha_mix(self->alpha, 24));

    text_region.location.x = region->location.x;
    text_region.location.y = region->location.y;
    text_region.size.width = region->size.width;
    text_region.size.height = region->size.height;
    egui_view_persona_group_draw_text(local->meta_font, self, text, &text_region, EGUI_ALIGN_CENTER, text_color);
}

static void egui_view_persona_group_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_group_t);
    char eyebrow_label[16];
    char title_label[24];
    char name_label[24];
    char role_label[24];
    char summary_label[24];
    const egui_view_persona_group_snapshot_t *snapshot = egui_view_persona_group_get_snapshot(local);
    const egui_view_persona_group_item_t *item = egui_view_persona_group_get_item(local);
    egui_view_persona_group_metrics_t metrics;
    egui_region_t text_region;
    egui_color_t card_fill;
    egui_color_t card_border;
    egui_color_t eyebrow_color;
    egui_color_t title_color;
    egui_color_t role_color;
    egui_color_t footer_fill;
    egui_color_t footer_border;
    egui_color_t footer_text;
    egui_dim_t footer_w;
    egui_dim_t card_radius;
    uint8_t item_count;
    uint8_t i;

    if (snapshot == NULL || item == NULL)
    {
        return;
    }

    egui_view_persona_group_get_metrics(local, self, &metrics);
    if (metrics.content_region.size.width <= 0 || metrics.content_region.size.height <= 0)
    {
        return;
    }

    item_count = egui_view_persona_group_clamp_item_count(snapshot->item_count);
    card_radius = metrics.dense_layout ? EGUI_VIEW_PERSONA_GROUP_DENSE_RADIUS : EGUI_VIEW_PERSONA_GROUP_STANDARD_RADIUS;
    card_fill = egui_rgb_mix(local->surface_color, local->section_color, metrics.dense_layout ? 4 : 6);
    card_border = egui_rgb_mix(local->border_color, local->section_color, metrics.dense_layout ? 14 : 18);
    eyebrow_color = egui_rgb_mix(local->muted_text_color, local->accent_color, metrics.dense_layout ? 14 : 18);
    title_color = local->text_color;
    role_color = local->muted_text_color;
    footer_fill = egui_rgb_mix(local->surface_color, local->accent_color, metrics.dense_layout ? 4 : 6);
    footer_border = egui_rgb_mix(local->border_color, local->accent_color, metrics.dense_layout ? 6 : 8);
    footer_text = egui_rgb_mix(local->muted_text_color, local->accent_color, metrics.dense_layout ? 12 : 16);
    if (!egui_view_get_enable(self))
    {
        card_fill = egui_view_persona_group_mix_disabled(card_fill);
        card_border = egui_view_persona_group_mix_disabled(card_border);
        eyebrow_color = egui_view_persona_group_mix_disabled(eyebrow_color);
        title_color = egui_view_persona_group_mix_disabled(title_color);
        role_color = egui_view_persona_group_mix_disabled(role_color);
        footer_fill = egui_view_persona_group_mix_disabled(footer_fill);
        footer_border = egui_view_persona_group_mix_disabled(footer_border);
        footer_text = egui_view_persona_group_mix_disabled(footer_text);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.content_region.location.x - 2, metrics.content_region.location.y - 2, metrics.content_region.size.width + 4,
                                          metrics.content_region.size.height + 4, card_radius, card_fill, egui_color_alpha_mix(self->alpha, 94));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.content_region.location.x - 2, metrics.content_region.location.y - 2, metrics.content_region.size.width + 4,
                                     metrics.content_region.size.height + 4, card_radius, 1, card_border, egui_color_alpha_mix(self->alpha, 34));

    if (!metrics.dense_layout)
    {
        egui_view_persona_group_fit_text_to_width(local->meta_font, snapshot->eyebrow, eyebrow_label, sizeof(eyebrow_label), metrics.eyebrow_region.size.width, 4);
        egui_view_persona_group_draw_text(local->meta_font, self, eyebrow_label, &metrics.eyebrow_region, EGUI_ALIGN_CENTER, eyebrow_color);
    }
    egui_view_persona_group_fit_text_to_width(local->font, snapshot->title, title_label, sizeof(title_label), metrics.title_region.size.width,
                                              metrics.dense_layout ? 4 : 5);
    egui_view_persona_group_draw_text(local->font, self, title_label, &metrics.title_region, EGUI_ALIGN_CENTER, title_color);

    if (snapshot->overflow_count > 0)
    {
        egui_view_persona_group_draw_overflow(self, local, &metrics.overflow_region, snapshot->overflow_count);
    }

    for (i = 0; i < item_count; i++)
    {
        if (i == local->current_index)
        {
            continue;
        }
        egui_view_persona_group_draw_avatar(self, local, &snapshot->items[i], &metrics.avatar_regions[i], 0,
                                            (uint8_t)(self->is_pressed && i == local->pressed_index));
    }
    if (local->current_index < item_count)
    {
        egui_view_persona_group_draw_avatar(self, local, &snapshot->items[local->current_index], &metrics.avatar_regions[local->current_index], 1,
                                            (uint8_t)(self->is_pressed && local->current_index == local->pressed_index));
    }

    egui_view_persona_group_fit_text_to_width(local->font, item->name, name_label, sizeof(name_label), metrics.name_region.size.width,
                                              metrics.dense_layout ? 4 : 5);
    egui_view_persona_group_draw_text(local->font, self, name_label, &metrics.name_region, EGUI_ALIGN_CENTER, title_color);

    if (!metrics.dense_layout)
    {
        egui_view_persona_group_fit_text_to_width(local->meta_font, item->role, role_label, sizeof(role_label), metrics.role_region.size.width, 4);
        egui_view_persona_group_draw_text(local->meta_font, self, role_label, &metrics.role_region, EGUI_ALIGN_CENTER, role_color);
    }

    footer_w = egui_view_persona_group_footer_width(local->meta_font, snapshot->summary, metrics.dense_layout, metrics.footer_region.size.width);
    text_region = metrics.footer_region;
    text_region.location.x = metrics.content_region.location.x + (metrics.content_region.size.width - footer_w) / 2;
    text_region.size.width = footer_w;
    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, text_region.location.x, text_region.location.y, text_region.size.width, text_region.size.height,
                                          text_region.size.height / 2, footer_fill, egui_color_alpha_mix(self->alpha, 80));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, text_region.location.x, text_region.location.y, text_region.size.width, text_region.size.height,
                                     text_region.size.height / 2, 1, footer_border, egui_color_alpha_mix(self->alpha, 20));
    egui_view_persona_group_fit_text_to_width(local->meta_font, snapshot->summary, summary_label, sizeof(summary_label), text_region.size.width - 4,
                                              metrics.dense_layout ? 4 : 5);
    egui_view_persona_group_draw_text(local->meta_font, self, summary_label, &text_region, EGUI_ALIGN_CENTER, footer_text);
}
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_persona_group_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_persona_group_t);
    const egui_view_persona_group_snapshot_t *snapshot = egui_view_persona_group_get_snapshot(local);
    uint8_t hit_index;
    uint8_t same_target;

    if (snapshot == NULL || !egui_view_get_enable(self))
    {
        egui_view_persona_group_clear_pressed_state(self);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_index = egui_view_persona_group_hit_index(local, self, event->location.x, event->location.y);
        if (hit_index >= EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS)
        {
            egui_view_persona_group_clear_pressed_state(self);
            return 0;
        }
        same_target = (uint8_t)(local->pressed_index == hit_index);
        if (same_target && self->is_pressed)
        {
            return 1;
        }
        local->pressed_index = hit_index;
        if (!self->is_pressed)
        {
            egui_view_set_pressed(self, true);
        }
        else
        {
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_index == EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS)
        {
            return 0;
        }
        hit_index = egui_view_persona_group_hit_index(local, self, event->location.x, event->location.y);
        if (hit_index == local->pressed_index)
        {
            if (!self->is_pressed)
            {
                egui_view_set_pressed(self, true);
            }
            return 1;
        }
        if (self->is_pressed)
        {
            egui_view_set_pressed(self, false);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled;

        hit_index = egui_view_persona_group_hit_index(local, self, event->location.x, event->location.y);
        handled = (uint8_t)((local->pressed_index != EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS) || hit_index < EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS);
        same_target = (uint8_t)(local->pressed_index != EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS && local->pressed_index == hit_index);
        if (same_target && self->is_pressed)
        {
            egui_view_persona_group_set_current_index_inner(self, hit_index, 1);
        }
        egui_view_persona_group_clear_pressed_state(self);
        return handled;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return egui_view_persona_group_clear_pressed_state(self);
    default:
        return 0;
    }
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_persona_group_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_persona_group_t);
    const egui_view_persona_group_snapshot_t *snapshot = egui_view_persona_group_get_snapshot(local);
    uint8_t item_count = snapshot == NULL ? 0 : egui_view_persona_group_clamp_item_count(snapshot->item_count);
    uint8_t next_index;

    if (snapshot == NULL || !egui_view_get_enable(self))
    {
        egui_view_persona_group_clear_pressed_state(self);
        return 0;
    }

    if (item_count == 0 || event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_LEFT:
    case EGUI_KEY_CODE_UP:
        next_index = local->current_index > 0 ? (local->current_index - 1) : 0;
        egui_view_persona_group_set_current_index_inner(self, next_index, 1);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
    case EGUI_KEY_CODE_DOWN:
        next_index = local->current_index + 1 < item_count ? (local->current_index + 1) : (item_count - 1);
        egui_view_persona_group_set_current_index_inner(self, next_index, 1);
        return 1;
    case EGUI_KEY_CODE_HOME:
        egui_view_persona_group_set_current_index_inner(self, 0, 1);
        return 1;
    case EGUI_KEY_CODE_END:
        egui_view_persona_group_set_current_index_inner(self, item_count - 1, 1);
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_index = local->current_index + 1;
        if (next_index >= item_count)
        {
            next_index = 0;
        }
        egui_view_persona_group_set_current_index_inner(self, next_index, 1);
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_persona_group_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_persona_group_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_persona_group_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_persona_group_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_persona_group_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_persona_group_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_persona_group_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_persona_group_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_persona_group_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_persona_group_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_persona_group_on_key_event,
#endif
};

void egui_view_persona_group_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_persona_group_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_persona_group_t);
    egui_view_set_padding_all(self, 2);

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_focus_changed = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD2DBE3);
    local->section_color = EGUI_COLOR_HEX(0xEEF3F7);
    local->text_color = EGUI_COLOR_HEX(0x1A2734);
    local->muted_text_color = EGUI_COLOR_HEX(0x6B7A89);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->success_color = EGUI_COLOR_HEX(0x0F7B45);
    local->warning_color = EGUI_COLOR_HEX(0x9D5D00);
    local->neutral_color = EGUI_COLOR_HEX(0x7A8796);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_index = 0;
    local->pressed_index = EGUI_VIEW_PERSONA_GROUP_MAX_ITEMS;

    egui_view_set_view_name(self, "egui_view_persona_group");
}
