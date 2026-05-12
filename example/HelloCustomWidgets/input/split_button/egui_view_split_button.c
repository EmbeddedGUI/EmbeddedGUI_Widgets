#include "egui_view_split_button.h"

#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_RADIUS               10
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_FILL_ALPHA           EGUI_ALPHA_MAKE(100)
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_BORDER_ALPHA         EGUI_ALPHA_MAKE(100)
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_CONTENT_PAD_X        10
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_CONTENT_PAD_Y        8
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_TITLE_HEIGHT         10
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_TITLE_GAP            4
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_ROW_HEIGHT           30
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_HELPER_GAP           5
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_HELPER_HEIGHT        10
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_MENU_WIDTH           28
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_SEGMENT_RADIUS       7
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_SEGMENT_FILL_ALPHA   EGUI_ALPHA_MAKE(100)
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_SEGMENT_BORDER_ALPHA EGUI_ALPHA_MAKE(100)
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_GLYPH_WIDTH          16
#define EGUI_VIEW_SPLIT_BUTTON_STANDARD_GLYPH_HEIGHT         14

#define EGUI_VIEW_SPLIT_BUTTON_COMPACT_RADIUS               8
#define EGUI_VIEW_SPLIT_BUTTON_COMPACT_FILL_ALPHA           EGUI_ALPHA_MAKE(100)
#define EGUI_VIEW_SPLIT_BUTTON_COMPACT_BORDER_ALPHA         EGUI_ALPHA_MAKE(100)
#define EGUI_VIEW_SPLIT_BUTTON_COMPACT_CONTENT_PAD_X        7
#define EGUI_VIEW_SPLIT_BUTTON_COMPACT_CONTENT_PAD_Y        6
#define EGUI_VIEW_SPLIT_BUTTON_COMPACT_TITLE_HEIGHT         9
#define EGUI_VIEW_SPLIT_BUTTON_COMPACT_TITLE_GAP            3
#define EGUI_VIEW_SPLIT_BUTTON_COMPACT_ROW_HEIGHT           22
#define EGUI_VIEW_SPLIT_BUTTON_COMPACT_MENU_WIDTH           20
#define EGUI_VIEW_SPLIT_BUTTON_COMPACT_SEGMENT_RADIUS       5
#define EGUI_VIEW_SPLIT_BUTTON_COMPACT_SEGMENT_FILL_ALPHA   EGUI_ALPHA_MAKE(100)
#define EGUI_VIEW_SPLIT_BUTTON_COMPACT_SEGMENT_BORDER_ALPHA EGUI_ALPHA_MAKE(100)

typedef struct egui_view_split_button_metrics egui_view_split_button_metrics_t;
struct egui_view_split_button_metrics
{
    egui_region_t content_region;
    egui_region_t title_region;
    egui_region_t row_region;
    egui_region_t primary_region;
    egui_region_t menu_region;
    egui_region_t helper_region;
    uint8_t show_title;
    uint8_t show_helper;
};

static uint8_t egui_view_split_button_clamp_snapshot_count(uint8_t count)
{
    if (count > EGUI_VIEW_SPLIT_BUTTON_MAX_SNAPSHOTS)
    {
        return EGUI_VIEW_SPLIT_BUTTON_MAX_SNAPSHOTS;
    }
    return count;
}

static const egui_view_split_button_snapshot_t *egui_view_split_button_get_snapshot(egui_view_split_button_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }

    return &local->snapshots[local->current_snapshot];
}

static uint8_t egui_view_split_button_text_len(const char *text)
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

static egui_dim_t egui_view_split_button_measure_font_line_height(const egui_font_t *font)
{
    egui_dim_t width = 0;
    egui_dim_t height = 0;

    if (font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, "A", 0, 0, &width, &height);
    return height;
}

static egui_dim_t egui_view_split_button_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t width = 0;
    egui_dim_t height = 0;

    if (font == NULL || text == NULL || text[0] == '\0' || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, text, 0, 0, &width, &height);
    return width;
}

static egui_color_t egui_view_split_button_tone_color(egui_view_split_button_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_SPLIT_BUTTON_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_SPLIT_BUTTON_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_SPLIT_BUTTON_TONE_DANGER:
        return local->danger_color;
    case EGUI_VIEW_SPLIT_BUTTON_TONE_NEUTRAL:
        return local->neutral_color;
    default:
        return local->accent_color;
    }
}

static egui_color_t egui_view_split_button_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(30));
}

static uint8_t egui_view_split_button_clear_pressed_state(egui_view_t *self, egui_view_split_button_t *local)
{
    uint8_t was_pressed = self->is_pressed ? 1 : 0;
    uint8_t had_pressed = was_pressed || local->pressed_part != EGUI_VIEW_SPLIT_BUTTON_PART_NONE;

    if (!had_pressed)
    {
        return 0;
    }

    local->pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_NONE;
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

static uint8_t egui_view_split_button_part_is_enabled(egui_view_split_button_t *local, egui_view_t *self, const egui_view_split_button_snapshot_t *snapshot,
                                                      uint8_t part)
{
    if (snapshot == NULL || !egui_view_get_enable(self) || local->disabled_mode)
    {
        return 0;
    }

    if (part == EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY)
    {
        return snapshot->primary_enabled ? 1 : 0;
    }

    if (part == EGUI_VIEW_SPLIT_BUTTON_PART_MENU)
    {
        return snapshot->menu_enabled ? 1 : 0;
    }

    return 0;
}

static uint8_t egui_view_split_button_resolve_default_part(egui_view_split_button_t *local, egui_view_t *self,
                                                           const egui_view_split_button_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return EGUI_VIEW_SPLIT_BUTTON_PART_NONE;
    }

    if (egui_view_split_button_part_is_enabled(local, self, snapshot, snapshot->focus_part))
    {
        return snapshot->focus_part;
    }

    if (egui_view_split_button_part_is_enabled(local, self, snapshot, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY))
    {
        return EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY;
    }

    if (egui_view_split_button_part_is_enabled(local, self, snapshot, EGUI_VIEW_SPLIT_BUTTON_PART_MENU))
    {
        return EGUI_VIEW_SPLIT_BUTTON_PART_MENU;
    }

    return EGUI_VIEW_SPLIT_BUTTON_PART_NONE;
}

static void egui_view_split_button_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                             egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (text == NULL || text[0] == '\0')
    {
        return;
    }

    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, self->alpha);
}

static void egui_view_split_button_draw_chevron(egui_view_t *self, const egui_region_t *region, egui_color_t color, uint8_t alpha_percent)
{
    egui_dim_t cx = region->location.x + region->size.width / 2;
    egui_dim_t cy = region->location.y + region->size.height / 2;
    egui_alpha_t mixed_alpha = egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(alpha_percent));

    egui_canvas_draw_line(&uicode_get_core()->canvas, cx - 3, cy - 1, cx, cy + 2, 1, color, mixed_alpha);
    egui_canvas_draw_line(&uicode_get_core()->canvas, cx, cy + 2, cx + 3, cy - 1, 1, color, mixed_alpha);
}

static void egui_view_split_button_get_metrics(egui_view_split_button_t *local, egui_view_t *self, const egui_view_split_button_snapshot_t *snapshot,
                                               egui_view_split_button_metrics_t *metrics)
{
    egui_region_t region;
    egui_dim_t pad_x = local->compact_mode ? EGUI_VIEW_SPLIT_BUTTON_COMPACT_CONTENT_PAD_X : EGUI_VIEW_SPLIT_BUTTON_STANDARD_CONTENT_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? EGUI_VIEW_SPLIT_BUTTON_COMPACT_CONTENT_PAD_Y : EGUI_VIEW_SPLIT_BUTTON_STANDARD_CONTENT_PAD_Y;
    egui_dim_t title_h = local->compact_mode ? EGUI_VIEW_SPLIT_BUTTON_COMPACT_TITLE_HEIGHT : EGUI_VIEW_SPLIT_BUTTON_STANDARD_TITLE_HEIGHT;
    egui_dim_t title_gap = local->compact_mode ? EGUI_VIEW_SPLIT_BUTTON_COMPACT_TITLE_GAP : EGUI_VIEW_SPLIT_BUTTON_STANDARD_TITLE_GAP;
    egui_dim_t row_h = local->compact_mode ? EGUI_VIEW_SPLIT_BUTTON_COMPACT_ROW_HEIGHT : EGUI_VIEW_SPLIT_BUTTON_STANDARD_ROW_HEIGHT;
    egui_dim_t helper_gap = EGUI_VIEW_SPLIT_BUTTON_STANDARD_HELPER_GAP;
    egui_dim_t helper_h = EGUI_VIEW_SPLIT_BUTTON_STANDARD_HELPER_HEIGHT;
    egui_dim_t menu_w = local->compact_mode ? EGUI_VIEW_SPLIT_BUTTON_COMPACT_MENU_WIDTH : EGUI_VIEW_SPLIT_BUTTON_STANDARD_MENU_WIDTH;
    egui_dim_t row_y;
    egui_dim_t block_h = row_h;
    egui_dim_t block_y;
    egui_dim_t meta_line_height = egui_view_split_button_measure_font_line_height(local->meta_font);

    egui_view_get_work_region(self, &region);
    metrics->show_title = (snapshot != NULL && snapshot->title != NULL && snapshot->title[0] != '\0') ? 1 : 0;
    metrics->show_helper = (!local->compact_mode && snapshot != NULL && snapshot->helper != NULL && snapshot->helper[0] != '\0') ? 1 : 0;
    metrics->content_region.location.x = region.location.x + pad_x;
    metrics->content_region.size.width = region.size.width - pad_x * 2;
    metrics->content_region.location.y = region.location.y + pad_y;
    metrics->content_region.size.height = region.size.height - pad_y * 2;

    if (meta_line_height > title_h)
    {
        title_h = meta_line_height;
    }
    if (meta_line_height > helper_h)
    {
        helper_h = meta_line_height;
    }

    if (metrics->show_title)
    {
        block_h += title_h + title_gap;
    }
    if (metrics->show_helper)
    {
        block_h += helper_h + helper_gap;
    }
    if (block_h > metrics->content_region.size.height && region.size.height >= block_h)
    {
        pad_y = (region.size.height - block_h) / 2;
        metrics->content_region.location.y = region.location.y + pad_y;
        metrics->content_region.size.height = region.size.height - pad_y * 2;
    }

    block_y = metrics->content_region.location.y;
    if (metrics->content_region.size.height > block_h)
    {
        block_y += (metrics->content_region.size.height - block_h) / 2;
    }

    metrics->title_region.location.x = metrics->content_region.location.x;
    metrics->title_region.location.y = block_y;
    metrics->title_region.size.width = metrics->content_region.size.width;
    metrics->title_region.size.height = title_h;

    if (metrics->show_title)
    {
        row_y = block_y + title_h + title_gap;
    }
    else
    {
        row_y = block_y;
    }

    metrics->row_region.location.x = metrics->content_region.location.x;
    metrics->row_region.location.y = row_y;
    metrics->row_region.size.width = metrics->content_region.size.width;
    metrics->row_region.size.height = row_h;

    metrics->primary_region.location.x = metrics->row_region.location.x;
    metrics->primary_region.location.y = metrics->row_region.location.y;
    metrics->primary_region.size.width = metrics->row_region.size.width - menu_w;
    metrics->primary_region.size.height = row_h;

    metrics->menu_region.location.x = metrics->primary_region.location.x + metrics->primary_region.size.width;
    metrics->menu_region.location.y = metrics->row_region.location.y;
    metrics->menu_region.size.width = menu_w;
    metrics->menu_region.size.height = row_h;

    metrics->helper_region.location.x = metrics->content_region.location.x;
    metrics->helper_region.location.y = metrics->row_region.location.y + row_h + helper_gap;
    metrics->helper_region.size.width = metrics->content_region.size.width;
    metrics->helper_region.size.height = helper_h;
}

static uint8_t egui_view_split_button_hit_part(egui_view_split_button_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_split_button_metrics_t metrics;
    const egui_view_split_button_snapshot_t *snapshot = egui_view_split_button_get_snapshot(local);

    egui_view_split_button_get_metrics(local, self, snapshot, &metrics);
    if (egui_region_pt_in_rect(&metrics.primary_region, x, y))
    {
        return EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY;
    }
    if (egui_region_pt_in_rect(&metrics.menu_region, x, y))
    {
        return EGUI_VIEW_SPLIT_BUTTON_PART_MENU;
    }

    return EGUI_VIEW_SPLIT_BUTTON_PART_NONE;
}

static void egui_view_split_button_screen_to_local(egui_view_t *self, const egui_motion_event_t *event, egui_dim_t *x, egui_dim_t *y)
{
    *x = event->location.x - self->region_screen.location.x;
    *y = event->location.y - self->region_screen.location.y;
}

static void egui_view_split_button_set_current_part_inner(egui_view_t *self, uint8_t part, uint8_t notify, uint8_t clear_pressed)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    const egui_view_split_button_snapshot_t *snapshot = egui_view_split_button_get_snapshot(local);
    uint8_t had_pressed = 0;

    if (clear_pressed)
    {
        had_pressed = egui_view_split_button_clear_pressed_state(self, local);
    }

    if (!egui_view_split_button_part_is_enabled(local, self, snapshot, part))
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    if (local->current_part == part)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_part = part;
    if (notify && local->on_part_changed)
    {
        local->on_part_changed(self, part);
    }
    egui_view_invalidate(self);
}

void egui_view_split_button_set_snapshots(egui_view_t *self, const egui_view_split_button_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    const egui_view_split_button_snapshot_t *snapshot;

    egui_view_split_button_clear_pressed_state(self, local);
    local->snapshots = snapshots;
    local->snapshot_count = egui_view_split_button_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }

    snapshot = egui_view_split_button_get_snapshot(local);
    local->current_part = egui_view_split_button_resolve_default_part(local, self, snapshot);
    egui_view_invalidate(self);
}

void egui_view_split_button_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    const egui_view_split_button_snapshot_t *snapshot;
    uint8_t had_pressed = egui_view_split_button_clear_pressed_state(self, local);

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
    snapshot = egui_view_split_button_get_snapshot(local);
    local->current_part = egui_view_split_button_resolve_default_part(local, self, snapshot);
    egui_view_invalidate(self);
}

uint8_t egui_view_split_button_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    return local->current_snapshot;
}

void egui_view_split_button_set_current_part(egui_view_t *self, uint8_t part)
{
    egui_view_split_button_set_current_part_inner(self, part, 1, 1);
}

uint8_t egui_view_split_button_get_current_part(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    return local->current_part;
}

void egui_view_split_button_set_on_part_changed_listener(egui_view_t *self, egui_view_on_split_button_part_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    local->on_part_changed = listener;
}

void egui_view_split_button_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    uint8_t had_pressed = egui_view_split_button_clear_pressed_state(self, local);
    local->font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_split_button_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    uint8_t had_pressed = egui_view_split_button_clear_pressed_state(self, local);
    local->meta_font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_split_button_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    uint8_t changed = 0;
    uint8_t had_pressed;

    compact_mode = compact_mode ? 1 : 0;
    had_pressed = egui_view_split_button_clear_pressed_state(self, local);
    if (local->compact_mode != compact_mode)
    {
        local->compact_mode = compact_mode;
        changed = 1;
    }
    if (changed || had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_split_button_set_disabled_mode(egui_view_t *self, uint8_t disabled_mode)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    uint8_t changed = 0;
    uint8_t had_pressed;

    disabled_mode = disabled_mode ? 1 : 0;
    had_pressed = egui_view_split_button_clear_pressed_state(self, local);
    if (local->disabled_mode != disabled_mode)
    {
        local->disabled_mode = disabled_mode;
        changed = 1;
    }
    if (changed || had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_split_button_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                        egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t success_color, egui_color_t warning_color,
                                        egui_color_t danger_color, egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    uint8_t had_pressed = egui_view_split_button_clear_pressed_state(self, local);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->danger_color = danger_color;
    local->neutral_color = neutral_color;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

static void egui_view_split_button_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    const egui_view_split_button_snapshot_t *snapshot = egui_view_split_button_get_snapshot(local);
    egui_view_split_button_metrics_t metrics;
    egui_region_t text_region;
    egui_region_t primary_fill_region;
    egui_region_t menu_fill_region;
    egui_color_t tone_color;
    egui_color_t card_fill;
    egui_color_t card_border;
    egui_color_t title_color;
    egui_color_t helper_color;
    egui_color_t row_fill;
    egui_color_t row_border;
    egui_color_t primary_fill;
    egui_color_t primary_border;
    egui_color_t primary_text;
    egui_color_t menu_fill;
    egui_color_t menu_border;
    egui_color_t menu_text;
    egui_color_t divider_color;
    egui_color_t glyph_fill;
    uint8_t primary_enabled;
    uint8_t menu_enabled;
    uint8_t show_glyph;
    egui_dim_t label_width;
    egui_dim_t radius;
    egui_dim_t segment_radius;

    if (snapshot == NULL)
    {
        return;
    }

    local->current_part = egui_view_split_button_resolve_default_part(local, self, snapshot);
    egui_view_split_button_get_metrics(local, self, snapshot, &metrics);
    if (metrics.content_region.size.width <= 0 || metrics.content_region.size.height <= 0)
    {
        return;
    }

    tone_color = egui_view_split_button_tone_color(local, snapshot->tone);
    card_fill = HCW_COLOR_PANEL;
    card_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(local->compact_mode ? 18 : 22));
    title_color = egui_rgb_mix(local->text_color, tone_color, EGUI_ALPHA_MAKE(local->compact_mode ? 8 : 10));
    helper_color = egui_rgb_mix(local->text_color, local->muted_text_color, EGUI_ALPHA_MAKE(local->compact_mode ? 16 : 20));
    row_fill = HCW_COLOR_PANEL;
    row_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(local->compact_mode ? 18 : 22));
    primary_fill = snapshot->emphasized ? egui_rgb_mix(HCW_COLOR_PANEL, tone_color, EGUI_ALPHA_MAKE(6))
                                        : egui_rgb_mix(local->surface_color, tone_color,
                                                       EGUI_ALPHA_MAKE(local->current_part == EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY ? 6 : 2));
    primary_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(local->current_part == EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY ? 20 : 16));
    primary_text = snapshot->emphasized ? egui_rgb_mix(local->text_color, tone_color, EGUI_ALPHA_MAKE(12))
                                        : (snapshot->tone == EGUI_VIEW_SPLIT_BUTTON_TONE_DANGER ? tone_color
                                                                                                 : egui_rgb_mix(local->text_color, tone_color, EGUI_ALPHA_MAKE(8)));
    menu_fill = egui_rgb_mix(HCW_COLOR_PANEL, tone_color, EGUI_ALPHA_MAKE(local->current_part == EGUI_VIEW_SPLIT_BUTTON_PART_MENU ? 6 : 2));
    menu_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(local->current_part == EGUI_VIEW_SPLIT_BUTTON_PART_MENU ? 20 : 16));
    menu_text = local->current_part == EGUI_VIEW_SPLIT_BUTTON_PART_MENU ? tone_color : egui_rgb_mix(local->text_color, tone_color, EGUI_ALPHA_MAKE(14));
    divider_color = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(16));
    glyph_fill = snapshot->emphasized ? egui_rgb_mix(HCW_COLOR_PANEL, tone_color, EGUI_ALPHA_MAKE(6))
                                      : egui_rgb_mix(HCW_COLOR_PANEL, tone_color, EGUI_ALPHA_MAKE(4));
    primary_enabled = egui_view_split_button_part_is_enabled(local, self, snapshot, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY);
    menu_enabled = egui_view_split_button_part_is_enabled(local, self, snapshot, EGUI_VIEW_SPLIT_BUTTON_PART_MENU);
    radius = local->compact_mode ? EGUI_VIEW_SPLIT_BUTTON_COMPACT_RADIUS : EGUI_VIEW_SPLIT_BUTTON_STANDARD_RADIUS;
    segment_radius = local->compact_mode ? EGUI_VIEW_SPLIT_BUTTON_COMPACT_SEGMENT_RADIUS : EGUI_VIEW_SPLIT_BUTTON_STANDARD_SEGMENT_RADIUS;

    if (local->pressed_part == EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY && primary_enabled)
    {
        primary_fill = egui_rgb_mix(primary_fill, tone_color, EGUI_ALPHA_MAKE(10));
    }
    if (local->pressed_part == EGUI_VIEW_SPLIT_BUTTON_PART_MENU && menu_enabled)
    {
        menu_fill = egui_rgb_mix(menu_fill, tone_color, EGUI_ALPHA_MAKE(10));
    }

    if (!primary_enabled)
    {
        primary_fill = egui_rgb_mix(primary_fill, row_fill, EGUI_ALPHA_MAKE(12));
        primary_border = egui_rgb_mix(primary_border, local->muted_text_color, EGUI_ALPHA_MAKE(20));
        primary_text = egui_rgb_mix(primary_text, local->muted_text_color, EGUI_ALPHA_MAKE(12));
        glyph_fill = egui_rgb_mix(glyph_fill, row_fill, EGUI_ALPHA_MAKE(12));
    }
    if (!menu_enabled)
    {
        menu_fill = egui_rgb_mix(menu_fill, row_fill, EGUI_ALPHA_MAKE(12));
        menu_border = egui_rgb_mix(menu_border, local->muted_text_color, EGUI_ALPHA_MAKE(20));
        menu_text = egui_rgb_mix(menu_text, local->muted_text_color, EGUI_ALPHA_MAKE(12));
    }

    if (local->disabled_mode)
    {
        card_fill = egui_rgb_mix(card_fill, local->border_color, EGUI_ALPHA_MAKE(8));
        card_border = egui_rgb_mix(card_border, local->border_color, EGUI_ALPHA_MAKE(36));
        title_color = egui_rgb_mix(title_color, local->text_color, EGUI_ALPHA_MAKE(38));
        helper_color = egui_rgb_mix(helper_color, local->text_color, EGUI_ALPHA_MAKE(32));
        row_fill = egui_rgb_mix(row_fill, local->border_color, EGUI_ALPHA_MAKE(10));
        row_border = egui_rgb_mix(row_border, local->border_color, EGUI_ALPHA_MAKE(38));
        primary_fill = egui_rgb_mix(primary_fill, local->border_color, EGUI_ALPHA_MAKE(10));
        primary_border = egui_rgb_mix(primary_border, local->border_color, EGUI_ALPHA_MAKE(40));
        primary_text = egui_rgb_mix(primary_text, local->text_color, EGUI_ALPHA_MAKE(42));
        menu_fill = egui_rgb_mix(menu_fill, local->border_color, EGUI_ALPHA_MAKE(10));
        menu_border = egui_rgb_mix(menu_border, local->border_color, EGUI_ALPHA_MAKE(40));
        menu_text = egui_rgb_mix(menu_text, local->text_color, EGUI_ALPHA_MAKE(42));
        divider_color = egui_rgb_mix(divider_color, local->border_color, EGUI_ALPHA_MAKE(42));
        glyph_fill = egui_rgb_mix(glyph_fill, local->border_color, EGUI_ALPHA_MAKE(10));
    }

    if (!egui_view_get_enable(self))
    {
        card_fill = egui_view_split_button_mix_disabled(card_fill);
        card_border = egui_view_split_button_mix_disabled(card_border);
        title_color = egui_view_split_button_mix_disabled(title_color);
        helper_color = egui_view_split_button_mix_disabled(helper_color);
        row_fill = egui_view_split_button_mix_disabled(row_fill);
        row_border = egui_view_split_button_mix_disabled(row_border);
        primary_fill = egui_view_split_button_mix_disabled(primary_fill);
        primary_border = egui_view_split_button_mix_disabled(primary_border);
        primary_text = egui_view_split_button_mix_disabled(primary_text);
        menu_fill = egui_view_split_button_mix_disabled(menu_fill);
        menu_border = egui_view_split_button_mix_disabled(menu_border);
        menu_text = egui_view_split_button_mix_disabled(menu_text);
        divider_color = egui_view_split_button_mix_disabled(divider_color);
        glyph_fill = egui_view_split_button_mix_disabled(glyph_fill);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas,
            metrics.content_region.location.x - 2, metrics.content_region.location.y - 2, metrics.content_region.size.width + 4,
            metrics.content_region.size.height + 4, radius, card_fill,
            egui_color_alpha_mix(self->alpha, local->compact_mode ? EGUI_VIEW_SPLIT_BUTTON_COMPACT_FILL_ALPHA : EGUI_VIEW_SPLIT_BUTTON_STANDARD_FILL_ALPHA));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.content_region.location.x - 2, metrics.content_region.location.y - 2, metrics.content_region.size.width + 4,
                                     metrics.content_region.size.height + 4, radius, 1, card_border,
                                     egui_color_alpha_mix(self->alpha, local->compact_mode ? EGUI_VIEW_SPLIT_BUTTON_COMPACT_BORDER_ALPHA
                                                                                           : EGUI_VIEW_SPLIT_BUTTON_STANDARD_BORDER_ALPHA));

    if (metrics.show_title)
    {
        text_region.location.x = metrics.title_region.location.x;
        text_region.location.y = metrics.title_region.location.y;
        text_region.size.width = metrics.title_region.size.width;
        text_region.size.height = metrics.title_region.size.height;
        egui_view_split_button_draw_text(local->meta_font, self, snapshot->title, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.row_region.location.x, metrics.row_region.location.y, metrics.row_region.size.width,
                                          metrics.row_region.size.height, segment_radius + 1, row_fill,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? EGUI_VIEW_SPLIT_BUTTON_COMPACT_SEGMENT_FILL_ALPHA
                                                                                                : EGUI_VIEW_SPLIT_BUTTON_STANDARD_SEGMENT_FILL_ALPHA));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.row_region.location.x, metrics.row_region.location.y, metrics.row_region.size.width,
                                     metrics.row_region.size.height, segment_radius + 1, 1, row_border,
                                     egui_color_alpha_mix(self->alpha, local->compact_mode ? EGUI_VIEW_SPLIT_BUTTON_COMPACT_SEGMENT_BORDER_ALPHA
                                                                                           : EGUI_VIEW_SPLIT_BUTTON_STANDARD_SEGMENT_BORDER_ALPHA));

    primary_fill_region.location.x = metrics.primary_region.location.x + 1;
    primary_fill_region.location.y = metrics.primary_region.location.y + 1;
    primary_fill_region.size.width = metrics.primary_region.size.width - 2;
    primary_fill_region.size.height = metrics.primary_region.size.height - 2;
    menu_fill_region.location.x = metrics.menu_region.location.x + 1;
    menu_fill_region.location.y = metrics.menu_region.location.y + 1;
    menu_fill_region.size.width = metrics.menu_region.size.width - 2;
    menu_fill_region.size.height = metrics.menu_region.size.height - 2;

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, primary_fill_region.location.x, primary_fill_region.location.y, primary_fill_region.size.width,
                                          primary_fill_region.size.height, segment_radius, primary_fill, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(100)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, primary_fill_region.location.x, primary_fill_region.location.y, primary_fill_region.size.width,
                                     primary_fill_region.size.height, segment_radius, 1, primary_border, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(98)));

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, menu_fill_region.location.x, menu_fill_region.location.y, menu_fill_region.size.width, menu_fill_region.size.height,
                                          segment_radius, menu_fill, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(100)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, menu_fill_region.location.x, menu_fill_region.location.y, menu_fill_region.size.width, menu_fill_region.size.height,
                                     segment_radius, 1, menu_border, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(98)));

    egui_canvas_draw_line(&uicode_get_core()->canvas, metrics.menu_region.location.x, metrics.row_region.location.y + 4, metrics.menu_region.location.x,
                          metrics.row_region.location.y + metrics.row_region.size.height - 4, 1, divider_color,
                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(94)));

    label_width = egui_view_split_button_measure_text_width(local->font, snapshot->label);
    if (label_width <= 0 && snapshot->label != NULL && snapshot->label[0] != '\0')
    {
        label_width = (egui_dim_t)egui_view_split_button_text_len(snapshot->label) * 5;
    }
    show_glyph = (!local->compact_mode && snapshot->glyph != NULL && snapshot->glyph[0] != '\0' &&
                  primary_fill_region.size.width > label_width + 33)
                         ? 1
                         : 0;
    if (show_glyph)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, primary_fill_region.location.x + 5,
                                              primary_fill_region.location.y +
                                                      (primary_fill_region.size.height - EGUI_VIEW_SPLIT_BUTTON_STANDARD_GLYPH_HEIGHT) / 2,
                                              EGUI_VIEW_SPLIT_BUTTON_STANDARD_GLYPH_WIDTH, EGUI_VIEW_SPLIT_BUTTON_STANDARD_GLYPH_HEIGHT, 4, glyph_fill,
                                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(96)));

        text_region.location.x = primary_fill_region.location.x + 5;
        text_region.location.y = primary_fill_region.location.y + (primary_fill_region.size.height - EGUI_VIEW_SPLIT_BUTTON_STANDARD_GLYPH_HEIGHT) / 2;
        text_region.size.width = EGUI_VIEW_SPLIT_BUTTON_STANDARD_GLYPH_WIDTH;
        text_region.size.height = EGUI_VIEW_SPLIT_BUTTON_STANDARD_GLYPH_HEIGHT;
        egui_view_split_button_draw_text(local->meta_font, self, snapshot->glyph, &text_region, EGUI_ALIGN_CENTER,
                                         snapshot->emphasized ? EGUI_COLOR_WHITE : primary_text);

        text_region.location.x = primary_fill_region.location.x + 5 + EGUI_VIEW_SPLIT_BUTTON_STANDARD_GLYPH_WIDTH + 6;
        text_region.location.y = primary_fill_region.location.y;
        text_region.size.width = primary_fill_region.size.width - (5 + EGUI_VIEW_SPLIT_BUTTON_STANDARD_GLYPH_WIDTH + 12);
        text_region.size.height = primary_fill_region.size.height;
        egui_view_split_button_draw_text(local->font, self, snapshot->label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, primary_text);
    }
    else
    {
        text_region.location.x = primary_fill_region.location.x + 4;
        text_region.location.y = primary_fill_region.location.y;
        text_region.size.width = primary_fill_region.size.width - 8;
        text_region.size.height = primary_fill_region.size.height;
        egui_view_split_button_draw_text(local->font, self, snapshot->label, &text_region, EGUI_ALIGN_CENTER, primary_text);
    }

    egui_view_split_button_draw_chevron(self, &menu_fill_region, menu_text, 94);

    if (metrics.show_helper)
    {
        text_region.location.x = metrics.helper_region.location.x;
        text_region.location.y = metrics.helper_region.location.y;
        text_region.size.width = metrics.helper_region.size.width;
        text_region.size.height = metrics.helper_region.size.height;
        egui_view_split_button_draw_text(local->meta_font, self, snapshot->helper, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, helper_color);
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_split_button_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    const egui_view_split_button_snapshot_t *snapshot = egui_view_split_button_get_snapshot(local);
    uint8_t hit_part;
    egui_dim_t local_x;
    egui_dim_t local_y;

    if (snapshot == NULL || !egui_view_get_enable(self) || local->disabled_mode || local->compact_mode)
    {
        if (egui_view_split_button_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    egui_view_split_button_screen_to_local(self, event, &local_x, &local_y);

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_part = egui_view_split_button_hit_part(local, self, local_x, local_y);
        if (!egui_view_split_button_part_is_enabled(local, self, snapshot, hit_part))
        {
            if (egui_view_split_button_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return 0;
        }
        local->pressed_part = hit_part;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_part == EGUI_VIEW_SPLIT_BUTTON_PART_NONE)
        {
            return 0;
        }
        hit_part = egui_view_split_button_hit_part(local, self, local_x, local_y);
        egui_view_set_pressed(self, hit_part == local->pressed_part &&
                                            egui_view_split_button_part_is_enabled(local, self, snapshot, local->pressed_part));
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled;

        hit_part = egui_view_split_button_hit_part(local, self, local_x, local_y);
        if (local->pressed_part == hit_part && egui_view_split_button_part_is_enabled(local, self, snapshot, hit_part))
        {
            egui_view_split_button_set_current_part_inner(self, hit_part, 1, 0);
        }
        handled = egui_view_split_button_clear_pressed_state(self, local);
        if (handled)
        {
            egui_view_invalidate(self);
        }
        return handled || hit_part != EGUI_VIEW_SPLIT_BUTTON_PART_NONE;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (egui_view_split_button_clear_pressed_state(self, local))
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
static int egui_view_split_button_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);
    const egui_view_split_button_snapshot_t *snapshot = egui_view_split_button_get_snapshot(local);
    uint8_t current_part;
    uint8_t next_part;

    if (egui_view_split_button_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    if (snapshot == NULL || !egui_view_get_enable(self) || local->disabled_mode || local->compact_mode || event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    current_part = local->current_part;
    switch (event->key_code)
    {
    case EGUI_KEY_CODE_LEFT:
    case EGUI_KEY_CODE_HOME:
        if (egui_view_split_button_part_is_enabled(local, self, snapshot, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY))
        {
            egui_view_split_button_set_current_part_inner(self, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, 1, 0);
        }
        return 1;
    case EGUI_KEY_CODE_RIGHT:
    case EGUI_KEY_CODE_END:
        if (egui_view_split_button_part_is_enabled(local, self, snapshot, EGUI_VIEW_SPLIT_BUTTON_PART_MENU))
        {
            egui_view_split_button_set_current_part_inner(self, EGUI_VIEW_SPLIT_BUTTON_PART_MENU, 1, 0);
        }
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_part = current_part == EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY ? EGUI_VIEW_SPLIT_BUTTON_PART_MENU : EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY;
        if (egui_view_split_button_part_is_enabled(local, self, snapshot, next_part))
        {
            egui_view_split_button_set_current_part_inner(self, next_part, 1, 0);
        }
        else if (egui_view_split_button_part_is_enabled(local, self, snapshot, current_part))
        {
            egui_view_split_button_set_current_part_inner(self, current_part, 1, 0);
        }
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_split_button_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);

    EGUI_UNUSED(event);
    egui_view_split_button_clear_pressed_state(self, local);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_split_button_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_split_button_t);

    EGUI_UNUSED(event);
    egui_view_split_button_clear_pressed_state(self, local);
    return 1;
}
#endif

void egui_view_split_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_split_button_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_split_button_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_split_button_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_split_button_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_split_button_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_split_button_on_key_event,
#endif
};

void egui_view_split_button_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_split_button_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_split_button_t);
    egui_view_set_padding_all(self, 2);

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_part_changed = NULL;
    local->surface_color = HCW_COLOR_PANEL;
    local->border_color = HCW_COLOR_BORDER_STRONG;
    local->text_color = HCW_COLOR_TEXT_STRONG;
    local->muted_text_color = HCW_COLOR_TEXT_SOFT;
    local->accent_color = HCW_COLOR_PRIMARY_DARK;
    local->success_color = HCW_COLOR_SUCCESS;
    local->warning_color = HCW_COLOR_WARNING_DARK;
    local->danger_color = HCW_COLOR_DANGER_DARK;
    local->neutral_color = HCW_COLOR_TEXT_SOFT;
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_part = EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY;
    local->compact_mode = 0;
    local->disabled_mode = 0;
    local->pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_NONE;

    egui_view_set_view_name(self, "egui_view_split_button");
}
