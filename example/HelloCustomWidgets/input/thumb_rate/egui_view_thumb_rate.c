#include "egui_view_thumb_rate.h"

#define THUMB_RATE_STD_RADIUS   10
#define THUMB_RATE_STD_PAD_X    8
#define THUMB_RATE_STD_PAD_Y    6
#define THUMB_RATE_STD_GAP      8
#define THUMB_RATE_STD_ICON     16
#define THUMB_RATE_STD_LABEL_H  10

#define THUMB_RATE_COMPACT_RADIUS   8
#define THUMB_RATE_COMPACT_PAD_X    5
#define THUMB_RATE_COMPACT_PAD_Y    5
#define THUMB_RATE_COMPACT_GAP      6
#define THUMB_RATE_COMPACT_ICON     14
#define THUMB_RATE_COMPACT_LABEL_H  0

typedef struct egui_view_thumb_rate_metrics egui_view_thumb_rate_metrics_t;
struct egui_view_thumb_rate_metrics
{
    egui_region_t surface_region;
    egui_region_t like_region;
    egui_region_t dislike_region;
    egui_region_t like_label_region;
    egui_region_t dislike_label_region;
    egui_dim_t icon_size;
    egui_dim_t part_radius;
    uint8_t show_labels;
};

static uint8_t egui_view_thumb_rate_has_text(const char *text)
{
    return (text != NULL && text[0] != '\0') ? 1 : 0;
}

static uint8_t egui_view_thumb_rate_state_is_valid(uint8_t state)
{
    return (state == EGUI_VIEW_THUMB_RATE_STATE_NONE || state == EGUI_VIEW_THUMB_RATE_STATE_LIKED ||
            state == EGUI_VIEW_THUMB_RATE_STATE_DISLIKED)
                   ? 1
                   : 0;
}

static uint8_t egui_view_thumb_rate_part_is_valid(uint8_t part)
{
    return (part == EGUI_VIEW_THUMB_RATE_PART_LIKE || part == EGUI_VIEW_THUMB_RATE_PART_DISLIKE) ? 1 : 0;
}

static uint8_t egui_view_thumb_rate_default_part(void)
{
    return EGUI_VIEW_THUMB_RATE_PART_LIKE;
}

static uint8_t egui_view_thumb_rate_part_to_state(uint8_t part)
{
    switch (part)
    {
    case EGUI_VIEW_THUMB_RATE_PART_LIKE:
        return EGUI_VIEW_THUMB_RATE_STATE_LIKED;
    case EGUI_VIEW_THUMB_RATE_PART_DISLIKE:
        return EGUI_VIEW_THUMB_RATE_STATE_DISLIKED;
    default:
        return EGUI_VIEW_THUMB_RATE_STATE_NONE;
    }
}

static uint8_t egui_view_thumb_rate_state_to_part(uint8_t state)
{
    switch (state)
    {
    case EGUI_VIEW_THUMB_RATE_STATE_LIKED:
        return EGUI_VIEW_THUMB_RATE_PART_LIKE;
    case EGUI_VIEW_THUMB_RATE_STATE_DISLIKED:
        return EGUI_VIEW_THUMB_RATE_PART_DISLIKE;
    default:
        return egui_view_thumb_rate_default_part();
    }
}

static void egui_view_thumb_rate_normalize_state(egui_view_thumb_rate_t *local)
{
    if (!egui_view_thumb_rate_state_is_valid(local->current_state))
    {
        local->current_state = EGUI_VIEW_THUMB_RATE_STATE_NONE;
    }
    if (!egui_view_thumb_rate_part_is_valid(local->current_part))
    {
        local->current_part = egui_view_thumb_rate_state_to_part(local->current_state);
    }
}

static uint8_t egui_view_thumb_rate_clear_pressed_state(egui_view_t *self, egui_view_thumb_rate_t *local)
{
    uint8_t had_pressed = self->is_pressed ? 1 : 0;
    uint8_t had_part = local->pressed_part != EGUI_VIEW_THUMB_RATE_PART_NONE ? 1 : 0;

    if (!had_pressed && !had_part)
    {
        return 0;
    }

    local->pressed_part = EGUI_VIEW_THUMB_RATE_PART_NONE;
    if (had_pressed)
    {
        egui_view_set_pressed(self, false);
    }
    else
    {
        egui_view_invalidate(self);
    }
    return 1;
}

static void egui_view_thumb_rate_get_metrics(egui_view_thumb_rate_t *local, egui_view_t *self, egui_view_thumb_rate_metrics_t *metrics)
{
    egui_region_t region;
    egui_dim_t pad_x = local->compact_mode ? THUMB_RATE_COMPACT_PAD_X : THUMB_RATE_STD_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? THUMB_RATE_COMPACT_PAD_Y : THUMB_RATE_STD_PAD_Y;
    egui_dim_t gap = local->compact_mode ? THUMB_RATE_COMPACT_GAP : THUMB_RATE_STD_GAP;
    egui_dim_t label_height = local->compact_mode ? THUMB_RATE_COMPACT_LABEL_H : THUMB_RATE_STD_LABEL_H;
    egui_dim_t part_width;
    egui_dim_t part_height;

    egui_view_get_work_region(self, &region);
    metrics->surface_region = region;
    metrics->show_labels = (!local->compact_mode && (egui_view_thumb_rate_has_text(local->like_label) || egui_view_thumb_rate_has_text(local->dislike_label))) ? 1 : 0;
    metrics->icon_size = local->compact_mode ? THUMB_RATE_COMPACT_ICON : THUMB_RATE_STD_ICON;
    metrics->part_radius = local->compact_mode ? THUMB_RATE_COMPACT_RADIUS : THUMB_RATE_STD_RADIUS;

    part_width = (region.size.width - pad_x * 2 - gap) / 2;
    if (part_width < metrics->icon_size + 12)
    {
        part_width = metrics->icon_size + 12;
    }
    part_height = region.size.height - pad_y * 2;
    if (part_height < metrics->icon_size + 8)
    {
        part_height = metrics->icon_size + 8;
    }

    metrics->like_region.location.x = region.location.x + pad_x;
    metrics->like_region.location.y = region.location.y + pad_y;
    metrics->like_region.size.width = part_width;
    metrics->like_region.size.height = part_height;

    metrics->dislike_region.location.x = metrics->like_region.location.x + part_width + gap;
    metrics->dislike_region.location.y = metrics->like_region.location.y;
    metrics->dislike_region.size.width = region.location.x + region.size.width - pad_x - metrics->dislike_region.location.x;
    metrics->dislike_region.size.height = part_height;
    if (metrics->dislike_region.size.width < part_width)
    {
        metrics->dislike_region.size.width = part_width;
    }

    metrics->like_label_region.location.x = metrics->like_region.location.x + 4;
    metrics->like_label_region.location.y = metrics->like_region.location.y + metrics->like_region.size.height - label_height - 4;
    metrics->like_label_region.size.width = metrics->like_region.size.width - 8;
    metrics->like_label_region.size.height = metrics->show_labels ? label_height : 0;

    metrics->dislike_label_region.location.x = metrics->dislike_region.location.x + 4;
    metrics->dislike_label_region.location.y = metrics->dislike_region.location.y + metrics->dislike_region.size.height - label_height - 4;
    metrics->dislike_label_region.size.width = metrics->dislike_region.size.width - 8;
    metrics->dislike_label_region.size.height = metrics->show_labels ? label_height : 0;
}

static void egui_view_thumb_rate_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (!egui_view_thumb_rate_has_text(text) || font == NULL || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    egui_canvas_draw_text_in_rect(font, text, &draw_region, EGUI_ALIGN_CENTER, color, self->alpha);
}

static void egui_view_thumb_rate_draw_focus(egui_view_t *self, const egui_region_t *region, egui_dim_t radius, egui_color_t color, egui_alpha_t alpha)
{
    egui_canvas_draw_round_rectangle(region->location.x - 1, region->location.y - 1, region->size.width + 2, region->size.height + 2, radius, 1, color,
                                     egui_color_alpha_mix(self->alpha, alpha));
}

static void egui_view_thumb_rate_draw_thumb_icon(egui_view_t *self, egui_dim_t x, egui_dim_t y, egui_dim_t size, uint8_t is_down, egui_color_t fill_color,
                                                 egui_color_t border_color, egui_alpha_t fill_alpha, egui_alpha_t border_alpha)
{
    static const int8_t template_points[13][2] = {
            {0, 12},
            {4, 12},
            {4, 6},
            {8, 1},
            {11, 1},
            {12, 2},
            {11, 7},
            {19, 7},
            {20, 8},
            {20, 16},
            {5, 16},
            {5, 20},
            {0, 20},
    };
    egui_dim_t points[26];
    uint8_t index;

    if (size <= 0)
    {
        return;
    }

    for (index = 0; index < 13; index++)
    {
        egui_dim_t px = template_points[index][0];
        egui_dim_t py = template_points[index][1];

        if (is_down)
        {
            py = 21 - py;
        }
        points[index * 2] = x + (px * size) / 20;
        points[index * 2 + 1] = y + (py * size) / 20;
    }

    egui_canvas_draw_polygon_fill(points, 13, fill_color, egui_color_alpha_mix(self->alpha, fill_alpha));
    egui_canvas_draw_polygon(points, 13, 1, border_color, egui_color_alpha_mix(self->alpha, border_alpha));
}

static void egui_view_thumb_rate_draw_part(egui_view_t *self, egui_view_thumb_rate_t *local, egui_view_thumb_rate_metrics_t *metrics, uint8_t part,
                                           egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color, egui_color_t muted_text_color)
{
    egui_region_t *part_region = part == EGUI_VIEW_THUMB_RATE_PART_LIKE ? &metrics->like_region : &metrics->dislike_region;
    egui_region_t *label_region = part == EGUI_VIEW_THUMB_RATE_PART_LIKE ? &metrics->like_label_region : &metrics->dislike_label_region;
    const char *label = part == EGUI_VIEW_THUMB_RATE_PART_LIKE ? local->like_label : local->dislike_label;
    egui_color_t accent = part == EGUI_VIEW_THUMB_RATE_PART_LIKE ? local->like_color : local->dislike_color;
    uint8_t enabled = (egui_view_get_enable(self) && !local->read_only_mode) ? 1 : 0;
    uint8_t focused = (self->is_focused && local->current_part == part && enabled) ? 1 : 0;
    uint8_t selected = local->current_state == egui_view_thumb_rate_part_to_state(part) ? 1 : 0;
    uint8_t pressed = (local->pressed_part == part && self->is_pressed) ? 1 : 0;
    egui_color_t fill_color;
    egui_color_t part_border_color;
    egui_color_t icon_color;
    egui_color_t label_color;
    egui_dim_t icon_x;
    egui_dim_t icon_y;

    fill_color = selected ? accent : egui_rgb_mix(surface_color, accent, focused ? 12 : (pressed ? 10 : 4));
    part_border_color = selected ? egui_rgb_mix(accent, EGUI_COLOR_WHITE, 10) : egui_rgb_mix(border_color, accent, focused ? 26 : 10);
    icon_color = selected ? EGUI_COLOR_WHITE : egui_rgb_mix(accent, text_color, 18);
    label_color = selected ? EGUI_COLOR_WHITE : egui_rgb_mix(text_color, muted_text_color, 18);

    if (pressed)
    {
        fill_color = egui_rgb_mix(fill_color, accent, selected ? 14 : 18);
        part_border_color = egui_rgb_mix(part_border_color, accent, 18);
    }
    if (focused)
    {
        egui_view_thumb_rate_draw_focus(self, part_region, metrics->part_radius, egui_rgb_mix(accent, EGUI_COLOR_WHITE, 12), local->compact_mode ? 44 : 54);
    }

    egui_canvas_draw_round_rectangle_fill(part_region->location.x, part_region->location.y, part_region->size.width, part_region->size.height, metrics->part_radius,
                                          fill_color, egui_color_alpha_mix(self->alpha, selected ? 96 : 94));
    egui_canvas_draw_round_rectangle(part_region->location.x, part_region->location.y, part_region->size.width, part_region->size.height, metrics->part_radius, 1,
                                     part_border_color, egui_color_alpha_mix(self->alpha, selected ? 82 : 58));

    icon_x = part_region->location.x + (part_region->size.width - metrics->icon_size) / 2;
    if (metrics->show_labels)
    {
        icon_y = part_region->location.y + 7;
    }
    else
    {
        icon_y = part_region->location.y + (part_region->size.height - metrics->icon_size) / 2;
    }

    egui_view_thumb_rate_draw_thumb_icon(self, icon_x, icon_y, metrics->icon_size, part == EGUI_VIEW_THUMB_RATE_PART_DISLIKE, icon_color,
                                         egui_rgb_mix(part_border_color, icon_color, 28), selected ? 100 : 96, selected ? 92 : 76);

    if (metrics->show_labels)
    {
        egui_view_thumb_rate_draw_text(local->font, self, label, label_region, label_color);
    }
}

static void egui_view_thumb_rate_notify_changed(egui_view_t *self, egui_view_thumb_rate_t *local, uint8_t state, uint8_t part)
{
    if (local->on_changed != NULL)
    {
        local->on_changed(self, state, part);
    }
}

static void egui_view_thumb_rate_set_state_inner(egui_view_t *self, uint8_t state, uint8_t notify, uint8_t trigger_part)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);
    uint8_t normalized_state = egui_view_thumb_rate_state_is_valid(state) ? state : EGUI_VIEW_THUMB_RATE_STATE_NONE;
    uint8_t old_state = local->current_state;
    uint8_t old_part = local->current_part;

    if (egui_view_thumb_rate_part_is_valid(trigger_part))
    {
        local->current_part = trigger_part;
    }
    else if (normalized_state != EGUI_VIEW_THUMB_RATE_STATE_NONE)
    {
        local->current_part = egui_view_thumb_rate_state_to_part(normalized_state);
    }

    local->current_state = normalized_state;
    egui_view_thumb_rate_normalize_state(local);
    if (old_state == local->current_state && old_part == local->current_part)
    {
        return;
    }

    if (notify && old_state != local->current_state)
    {
        egui_view_thumb_rate_notify_changed(self, local, local->current_state, local->current_part);
    }
    egui_view_invalidate(self);
}

static void egui_view_thumb_rate_commit(egui_view_t *self, uint8_t part)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);
    uint8_t next_state;

    if (!egui_view_thumb_rate_part_is_valid(part) || !egui_view_get_enable(self) || local->read_only_mode)
    {
        return;
    }

    next_state = egui_view_thumb_rate_part_to_state(part);
    if (local->current_state == next_state)
    {
        next_state = EGUI_VIEW_THUMB_RATE_STATE_NONE;
    }
    egui_view_thumb_rate_set_state_inner(self, next_state, 1, part);
}

static uint8_t egui_view_thumb_rate_is_navigation_key(uint8_t key_code)
{
    switch (key_code)
    {
    case EGUI_KEY_CODE_LEFT:
    case EGUI_KEY_CODE_RIGHT:
    case EGUI_KEY_CODE_UP:
    case EGUI_KEY_CODE_DOWN:
    case EGUI_KEY_CODE_HOME:
    case EGUI_KEY_CODE_END:
    case EGUI_KEY_CODE_TAB:
    case EGUI_KEY_CODE_ESCAPE:
        return 1;
    default:
        return 0;
    }
}

static uint8_t egui_view_thumb_rate_is_commit_key(uint8_t key_code)
{
    return (key_code == EGUI_KEY_CODE_ENTER || key_code == EGUI_KEY_CODE_SPACE) ? 1 : 0;
}

static uint8_t egui_view_thumb_rate_handle_navigation_key_inner(egui_view_t *self, uint8_t key_code)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        egui_view_thumb_rate_clear_pressed_state(self, local);
        return 0;
    }

    egui_view_thumb_rate_clear_pressed_state(self, local);
    egui_view_thumb_rate_normalize_state(local);

    switch (key_code)
    {
    case EGUI_KEY_CODE_LEFT:
    case EGUI_KEY_CODE_UP:
    case EGUI_KEY_CODE_HOME:
        egui_view_thumb_rate_set_current_part(self, EGUI_VIEW_THUMB_RATE_PART_LIKE);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
    case EGUI_KEY_CODE_DOWN:
    case EGUI_KEY_CODE_END:
        egui_view_thumb_rate_set_current_part(self, EGUI_VIEW_THUMB_RATE_PART_DISLIKE);
        return 1;
    case EGUI_KEY_CODE_TAB:
        egui_view_thumb_rate_set_current_part(self, local->current_part == EGUI_VIEW_THUMB_RATE_PART_DISLIKE ? EGUI_VIEW_THUMB_RATE_PART_LIKE
                                                                                                               : EGUI_VIEW_THUMB_RATE_PART_DISLIKE);
        return 1;
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        egui_view_thumb_rate_commit(self, local->current_part);
        return 1;
    case EGUI_KEY_CODE_ESCAPE:
        if (local->current_state != EGUI_VIEW_THUMB_RATE_STATE_NONE)
        {
            egui_view_thumb_rate_set_state_inner(self, EGUI_VIEW_THUMB_RATE_STATE_NONE, 1, local->current_part);
            return 1;
        }
        egui_view_thumb_rate_set_current_part(self, EGUI_VIEW_THUMB_RATE_PART_LIKE);
        return 1;
    default:
        return 0;
    }
}

void egui_view_thumb_rate_apply_standard_style(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);
    uint8_t had_pressed = egui_view_thumb_rate_clear_pressed_state(self, local);

    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD7DFE7);
    local->text_color = EGUI_COLOR_HEX(0x233140);
    local->muted_text_color = EGUI_COLOR_HEX(0x6F7C8A);
    local->like_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->dislike_color = EGUI_COLOR_HEX(0xC42B1C);
    local->shadow_color = EGUI_COLOR_HEX(0xDBE3EB);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
    egui_view_set_enable(self, 1);
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_thumb_rate_apply_compact_style(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);
    uint8_t had_pressed = egui_view_thumb_rate_clear_pressed_state(self, local);

    local->compact_mode = 1;
    local->read_only_mode = 0;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD7DFE7);
    local->text_color = EGUI_COLOR_HEX(0x263242);
    local->muted_text_color = EGUI_COLOR_HEX(0x73808D);
    local->like_color = EGUI_COLOR_HEX(0x0F766E);
    local->dislike_color = EGUI_COLOR_HEX(0xC42B1C);
    local->shadow_color = EGUI_COLOR_HEX(0xE1E8EF);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
    egui_view_set_enable(self, 1);
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_thumb_rate_apply_read_only_style(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);
    uint8_t had_pressed = egui_view_thumb_rate_clear_pressed_state(self, local);

    local->compact_mode = 1;
    local->read_only_mode = 1;
    local->surface_color = EGUI_COLOR_HEX(0xFBFCFD);
    local->border_color = EGUI_COLOR_HEX(0xDCE4EB);
    local->text_color = EGUI_COLOR_HEX(0x617180);
    local->muted_text_color = EGUI_COLOR_HEX(0x8B97A3);
    local->like_color = EGUI_COLOR_HEX(0x9FB3C1);
    local->dislike_color = EGUI_COLOR_HEX(0xC3A59F);
    local->shadow_color = EGUI_COLOR_HEX(0xE5EBF0);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
    egui_view_set_enable(self, 1);
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_thumb_rate_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);

    egui_view_thumb_rate_clear_pressed_state(self, local);
    local->font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    egui_view_invalidate(self);
}

void egui_view_thumb_rate_set_labels(egui_view_t *self, const char *like_label, const char *dislike_label)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);

    egui_view_thumb_rate_clear_pressed_state(self, local);
    local->like_label = like_label;
    local->dislike_label = dislike_label;
    egui_view_invalidate(self);
}

void egui_view_thumb_rate_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                      egui_color_t muted_text_color, egui_color_t like_color, egui_color_t dislike_color, egui_color_t shadow_color)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);

    egui_view_thumb_rate_clear_pressed_state(self, local);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->like_color = like_color;
    local->dislike_color = dislike_color;
    local->shadow_color = shadow_color;
    egui_view_invalidate(self);
}

void egui_view_thumb_rate_set_state(egui_view_t *self, uint8_t state)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);

    egui_view_thumb_rate_clear_pressed_state(self, local);
    egui_view_thumb_rate_set_state_inner(self, state, 0, local->current_part);
}

uint8_t egui_view_thumb_rate_get_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);

    egui_view_thumb_rate_normalize_state(local);
    return local->current_state;
}

void egui_view_thumb_rate_set_current_part(egui_view_t *self, uint8_t part)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);

    egui_view_thumb_rate_clear_pressed_state(self, local);
    if (!egui_view_thumb_rate_part_is_valid(part))
    {
        return;
    }
    if (local->current_part == part)
    {
        return;
    }

    local->current_part = part;
    egui_view_invalidate(self);
}

uint8_t egui_view_thumb_rate_get_current_part(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);

    egui_view_thumb_rate_normalize_state(local);
    return local->current_part;
}

void egui_view_thumb_rate_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);
    uint8_t normalized = compact_mode ? 1 : 0;

    egui_view_thumb_rate_clear_pressed_state(self, local);
    if (local->compact_mode == normalized)
    {
        return;
    }

    local->compact_mode = normalized;
    egui_view_invalidate(self);
}

void egui_view_thumb_rate_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);
    uint8_t normalized = read_only_mode ? 1 : 0;

    egui_view_thumb_rate_clear_pressed_state(self, local);
    if (local->read_only_mode == normalized)
    {
        return;
    }

    local->read_only_mode = normalized;
    egui_view_invalidate(self);
}

void egui_view_thumb_rate_set_on_changed_listener(egui_view_t *self, egui_view_on_thumb_rate_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);
    local->on_changed = listener;
}

uint8_t egui_view_thumb_rate_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);
    egui_view_thumb_rate_metrics_t metrics;

    if (region == NULL || !egui_view_thumb_rate_part_is_valid(part))
    {
        return 0;
    }

    egui_view_thumb_rate_normalize_state(local);
    egui_view_thumb_rate_get_metrics(local, self, &metrics);
    *region = part == EGUI_VIEW_THUMB_RATE_PART_LIKE ? metrics.like_region : metrics.dislike_region;
    region->location.x += self->region_screen.location.x;
    region->location.y += self->region_screen.location.y;
    return 1;
}

uint8_t egui_view_thumb_rate_handle_navigation_key(egui_view_t *self, uint8_t key_code)
{
    return egui_view_thumb_rate_handle_navigation_key_inner(self, key_code);
}

static uint8_t egui_view_thumb_rate_hit_part(egui_view_thumb_rate_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_thumb_rate_metrics_t metrics;
    egui_dim_t local_x = x - self->region_screen.location.x;
    egui_dim_t local_y = y - self->region_screen.location.y;

    egui_view_thumb_rate_get_metrics(local, self, &metrics);
    if (egui_region_pt_in_rect(&metrics.like_region, local_x, local_y))
    {
        return EGUI_VIEW_THUMB_RATE_PART_LIKE;
    }
    if (egui_region_pt_in_rect(&metrics.dislike_region, local_x, local_y))
    {
        return EGUI_VIEW_THUMB_RATE_PART_DISLIKE;
    }
    return EGUI_VIEW_THUMB_RATE_PART_NONE;
}

static void egui_view_thumb_rate_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);
    egui_view_thumb_rate_metrics_t metrics;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_color_t muted_text_color = local->muted_text_color;
    egui_color_t shadow_color = local->shadow_color;
    uint8_t enabled = egui_view_get_enable(self) ? 1 : 0;

    egui_view_thumb_rate_normalize_state(local);
    egui_view_thumb_rate_get_metrics(local, self, &metrics);
    if (metrics.surface_region.size.width <= 0 || metrics.surface_region.size.height <= 0)
    {
        return;
    }

    if (!enabled)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xF6F8FA), 34);
        border_color = egui_rgb_mix(border_color, muted_text_color, 28);
        text_color = egui_rgb_mix(text_color, muted_text_color, 48);
        muted_text_color = egui_rgb_mix(muted_text_color, surface_color, 16);
        shadow_color = egui_rgb_mix(shadow_color, surface_color, 40);
    }
    else if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFBFCFD), 24);
        border_color = egui_rgb_mix(border_color, muted_text_color, 20);
        text_color = egui_rgb_mix(text_color, muted_text_color, 32);
        muted_text_color = egui_rgb_mix(muted_text_color, surface_color, 12);
        shadow_color = egui_rgb_mix(shadow_color, surface_color, 36);
    }

    if (!local->compact_mode)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.surface_region.location.x, metrics.surface_region.location.y + 2, metrics.surface_region.size.width,
                                              metrics.surface_region.size.height, THUMB_RATE_STD_RADIUS + 1, shadow_color,
                                              egui_color_alpha_mix(self->alpha, enabled ? 18 : 10));
    }
    egui_canvas_draw_round_rectangle_fill(metrics.surface_region.location.x, metrics.surface_region.location.y, metrics.surface_region.size.width,
                                          metrics.surface_region.size.height, local->compact_mode ? THUMB_RATE_COMPACT_RADIUS : THUMB_RATE_STD_RADIUS, surface_color,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 94 : 96));
    egui_canvas_draw_round_rectangle(metrics.surface_region.location.x, metrics.surface_region.location.y, metrics.surface_region.size.width,
                                     metrics.surface_region.size.height, local->compact_mode ? THUMB_RATE_COMPACT_RADIUS : THUMB_RATE_STD_RADIUS, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, local->compact_mode ? 56 : 60));

    egui_view_thumb_rate_draw_part(self, local, &metrics, EGUI_VIEW_THUMB_RATE_PART_LIKE, surface_color, border_color, text_color, muted_text_color);
    egui_view_thumb_rate_draw_part(self, local, &metrics, EGUI_VIEW_THUMB_RATE_PART_DISLIKE, surface_color, border_color, text_color, muted_text_color);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_thumb_rate_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);
    uint8_t hit_part;
    uint8_t handled;

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        egui_view_thumb_rate_clear_pressed_state(self, local);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_part = egui_view_thumb_rate_hit_part(local, self, event->location.x, event->location.y);
        if (!egui_view_thumb_rate_part_is_valid(hit_part))
        {
            egui_view_thumb_rate_clear_pressed_state(self, local);
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
        if (!egui_view_thumb_rate_part_is_valid(local->pressed_part))
        {
            return 0;
        }
        hit_part = egui_view_thumb_rate_hit_part(local, self, event->location.x, event->location.y);
        handled = self->is_pressed ? 1 : 0;
        if (hit_part == local->pressed_part)
        {
            if (!self->is_pressed)
            {
                egui_view_set_pressed(self, true);
            }
        }
        else if (self->is_pressed)
        {
            egui_view_set_pressed(self, false);
        }
        return handled || hit_part != EGUI_VIEW_THUMB_RATE_PART_NONE;
    case EGUI_MOTION_EVENT_ACTION_UP:
        hit_part = egui_view_thumb_rate_hit_part(local, self, event->location.x, event->location.y);
        handled = egui_view_thumb_rate_part_is_valid(local->pressed_part) ? 1 : 0;
        if (handled && self->is_pressed && hit_part == local->pressed_part)
        {
            egui_view_thumb_rate_commit(self, local->pressed_part);
        }
        egui_view_thumb_rate_clear_pressed_state(self, local);
        return handled || hit_part != EGUI_VIEW_THUMB_RATE_PART_NONE;
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return egui_view_thumb_rate_clear_pressed_state(self, local);
    default:
        return 0;
    }
}

static int egui_view_thumb_rate_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);

    EGUI_UNUSED(event);
    egui_view_thumb_rate_clear_pressed_state(self, local);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_thumb_rate_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);
    uint8_t was_pressed = self->is_pressed ? 1 : 0;

    if (egui_view_thumb_rate_is_commit_key(event->key_code))
    {
        if (!egui_view_get_enable(self) || local->read_only_mode)
        {
            egui_view_thumb_rate_clear_pressed_state(self, local);
            return event->type == EGUI_KEY_EVENT_ACTION_DOWN ? 1 : 0;
        }

        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            egui_view_set_pressed(self, true);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            egui_view_thumb_rate_clear_pressed_state(self, local);
            if (was_pressed)
            {
                egui_view_thumb_rate_commit(self, local->current_part);
            }
            return 1;
        }
        return 0;
    }

    if (egui_view_thumb_rate_is_navigation_key(event->key_code))
    {
        if (!egui_view_get_enable(self) || local->read_only_mode)
        {
            egui_view_thumb_rate_clear_pressed_state(self, local);
            return event->type == EGUI_KEY_EVENT_ACTION_DOWN ? 1 : 0;
        }

        egui_view_thumb_rate_clear_pressed_state(self, local);
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            return egui_view_thumb_rate_handle_navigation_key_inner(self, event->key_code);
        }
        return 0;
    }

    egui_view_thumb_rate_clear_pressed_state(self, local);
    return egui_view_on_key_event(self, event);
}

static int egui_view_thumb_rate_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_thumb_rate_t);

    EGUI_UNUSED(event);
    egui_view_thumb_rate_clear_pressed_state(self, local);
    return 1;
}
#endif

void egui_view_thumb_rate_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_thumb_rate_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_thumb_rate_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_thumb_rate_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_thumb_rate_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_thumb_rate_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_thumb_rate_on_key_event,
#endif
};

void egui_view_thumb_rate_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_thumb_rate_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_thumb_rate_t);
    egui_view_set_padding_all(self, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_changed = NULL;
    local->like_label = "Like";
    local->dislike_label = "Dislike";
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD7DFE7);
    local->text_color = EGUI_COLOR_HEX(0x233140);
    local->muted_text_color = EGUI_COLOR_HEX(0x6F7C8A);
    local->like_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->dislike_color = EGUI_COLOR_HEX(0xC42B1C);
    local->shadow_color = EGUI_COLOR_HEX(0xDBE3EB);
    local->current_state = EGUI_VIEW_THUMB_RATE_STATE_NONE;
    local->current_part = EGUI_VIEW_THUMB_RATE_PART_LIKE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_part = EGUI_VIEW_THUMB_RATE_PART_NONE;

    egui_view_set_view_name(self, "egui_view_thumb_rate");
}
