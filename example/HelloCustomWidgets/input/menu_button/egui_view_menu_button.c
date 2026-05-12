#include "egui_view_menu_button.h"
#include "../../hcw_text_center.h"

#define EGUI_VIEW_MENU_BUTTON_RADIUS             9
#define EGUI_VIEW_MENU_BUTTON_COMPACT_RADIUS     7
#define EGUI_VIEW_MENU_BUTTON_PAD_X              8
#define EGUI_VIEW_MENU_BUTTON_PAD_Y              6
#define EGUI_VIEW_MENU_BUTTON_TRIGGER_HEIGHT     30
#define EGUI_VIEW_MENU_BUTTON_COMPACT_HEIGHT     24
#define EGUI_VIEW_MENU_BUTTON_MENU_GAP           5
#define EGUI_VIEW_MENU_BUTTON_MENU_PAD           5
#define EGUI_VIEW_MENU_BUTTON_MENU_TITLE_HEIGHT  12
#define EGUI_VIEW_MENU_BUTTON_ITEM_HEIGHT        20
#define EGUI_VIEW_MENU_BUTTON_ITEM_GAP           2
#define EGUI_VIEW_MENU_BUTTON_ICON_SIZE          16
#define EGUI_VIEW_MENU_BUTTON_SHORTCUT_WIDTH     34
#define EGUI_VIEW_MENU_BUTTON_TRIGGER_PAD_X      7
#define EGUI_VIEW_MENU_BUTTON_TRIGGER_TEXT_GAP   5
#define EGUI_VIEW_MENU_BUTTON_TRIGGER_END_GAP    4
#define EGUI_VIEW_MENU_BUTTON_CHEVRON_SIZE       16
#define EGUI_VIEW_MENU_BUTTON_STATUS_INSET_X     2
#define EGUI_VIEW_MENU_BUTTON_STATUS_ICON_SIZE   12
#define EGUI_VIEW_MENU_BUTTON_STATUS_ICON_SLOT   22
#define EGUI_VIEW_MENU_BUTTON_STATUS_END_GAP     4
#define EGUI_VIEW_MENU_BUTTON_ITEM_ICON_SLOT     26
#define EGUI_VIEW_MENU_BUTTON_ITEM_CHECK_SIZE    14
#define EGUI_VIEW_MENU_BUTTON_ITEM_CHECK_END_GAP 4
#define EGUI_VIEW_MENU_BUTTON_ITEM_TEXT_GAP      2

typedef struct egui_view_menu_button_metrics egui_view_menu_button_metrics_t;
struct egui_view_menu_button_metrics
{
    egui_region_t content_region;
    egui_region_t trigger_region;
    egui_region_t trigger_icon_region;
    egui_region_t trigger_label_region;
    egui_region_t trigger_chevron_region;
    egui_region_t menu_region;
    egui_region_t menu_title_region;
    egui_region_t item_regions[EGUI_VIEW_MENU_BUTTON_MAX_ITEMS];
    egui_region_t item_icon_regions[EGUI_VIEW_MENU_BUTTON_MAX_ITEMS];
    egui_region_t item_label_regions[EGUI_VIEW_MENU_BUTTON_MAX_ITEMS];
    egui_region_t item_shortcut_regions[EGUI_VIEW_MENU_BUTTON_MAX_ITEMS];
    egui_region_t item_check_regions[EGUI_VIEW_MENU_BUTTON_MAX_ITEMS];
    egui_region_t status_region;
    egui_region_t status_icon_region;
    egui_region_t status_label_region;
    uint8_t show_menu_title;
};

static uint8_t egui_view_menu_button_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static egui_color_t egui_view_menu_button_tone_color(egui_view_menu_button_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_MENU_BUTTON_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_MENU_BUTTON_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_MENU_BUTTON_TONE_DANGER:
        return local->danger_color;
    case EGUI_VIEW_MENU_BUTTON_TONE_NEUTRAL:
        return local->neutral_color;
    default:
        return local->accent_color;
    }
}

static egui_color_t egui_view_menu_button_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_DISABLED, EGUI_ALPHA_MAKE(52));
}

static uint8_t egui_view_menu_button_is_valid_index(egui_view_menu_button_t *local, uint8_t index)
{
    return (uint8_t)(index < local->item_count);
}

static uint8_t egui_view_menu_button_item_is_enabled(egui_view_menu_button_t *local, uint8_t index)
{
    return (uint8_t)(egui_view_menu_button_is_valid_index(local, index) && !local->items[index].disabled);
}

static uint8_t egui_view_menu_button_is_interactive(egui_view_menu_button_t *local, egui_view_t *self)
{
    return (uint8_t)(!local->read_only_mode && !local->compact_mode && egui_view_get_enable(self));
}

static uint8_t egui_view_menu_button_clear_active_state(egui_view_t *self, egui_view_menu_button_t *local)
{
    uint8_t had_active = (uint8_t)(self->is_pressed || local->active_target != EGUI_VIEW_MENU_BUTTON_INDEX_NONE);

    local->active_target = EGUI_VIEW_MENU_BUTTON_INDEX_NONE;
    egui_view_set_pressed(self, false);
    return had_active;
}

static uint8_t egui_view_menu_button_first_enabled(egui_view_menu_button_t *local)
{
    uint8_t index;

    for (index = 0; index < local->item_count; ++index)
    {
        if (!local->items[index].disabled)
        {
            return index;
        }
    }
    return EGUI_VIEW_MENU_BUTTON_INDEX_NONE;
}

static uint8_t egui_view_menu_button_find_next_enabled(egui_view_menu_button_t *local, uint8_t start, int8_t direction)
{
    uint8_t step;
    uint8_t index;

    if (local->item_count == 0)
    {
        return EGUI_VIEW_MENU_BUTTON_INDEX_NONE;
    }
    if (start >= local->item_count)
    {
        start = direction > 0 ? (uint8_t)(local->item_count - 1U) : 0;
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
    return EGUI_VIEW_MENU_BUTTON_INDEX_NONE;
}

static void egui_view_menu_button_normalize_indices(egui_view_menu_button_t *local)
{
    uint8_t first_enabled;

    if (local->item_count == 0)
    {
        local->selected_index = EGUI_VIEW_MENU_BUTTON_INDEX_NONE;
        local->focus_index = EGUI_VIEW_MENU_BUTTON_INDEX_NONE;
        local->is_open = 0;
        return;
    }

    first_enabled = egui_view_menu_button_first_enabled(local);
    if (!egui_view_menu_button_item_is_enabled(local, local->selected_index))
    {
        local->selected_index = first_enabled;
    }
    if (!egui_view_menu_button_item_is_enabled(local, local->focus_index))
    {
        local->focus_index = local->selected_index;
    }
}

static void egui_view_menu_button_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                            egui_color_t color, egui_alpha_t alpha)
{
    egui_region_t draw_region = *region;

    if (!egui_view_menu_button_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    draw_region.location.y += hcw_text_center_get_delta(font, text, region, align);
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, egui_color_alpha_mix(self->alpha, alpha));
}

static void egui_view_menu_button_draw_chevron(egui_view_t *self, const egui_region_t *region, egui_color_t color, uint8_t is_open)
{
    egui_dim_t cx = region->location.x + region->size.width / 2;
    egui_dim_t cy = region->location.y + region->size.height / 2;

    if (is_open)
    {
        egui_canvas_draw_line(&uicode_get_core()->canvas, cx - 3, cy + 1, cx, cy - 2, 1, color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
        egui_canvas_draw_line(&uicode_get_core()->canvas, cx, cy - 2, cx + 3, cy + 1, 1, color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
    }
    else
    {
        egui_canvas_draw_line(&uicode_get_core()->canvas, cx - 3, cy - 1, cx, cy + 2, 1, color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
        egui_canvas_draw_line(&uicode_get_core()->canvas, cx, cy + 2, cx + 3, cy - 1, 1, color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
    }
}

static void egui_view_menu_button_get_metrics(egui_view_menu_button_t *local, egui_view_t *self, egui_view_menu_button_metrics_t *metrics)
{
    egui_region_t region;
    egui_dim_t trigger_h = local->compact_mode ? EGUI_VIEW_MENU_BUTTON_COMPACT_HEIGHT : EGUI_VIEW_MENU_BUTTON_TRIGGER_HEIGHT;
    egui_dim_t row_y;
    egui_dim_t menu_y;
    egui_dim_t menu_h;
    egui_dim_t label_x;
    egui_dim_t label_right;
    egui_dim_t item_x;
    egui_dim_t item_w;
    uint8_t index;

    egui_view_get_work_region(self, &region);
    metrics->content_region.location.x = region.location.x + EGUI_VIEW_MENU_BUTTON_PAD_X;
    metrics->content_region.location.y = region.location.y + EGUI_VIEW_MENU_BUTTON_PAD_Y;
    metrics->content_region.size.width = region.size.width - EGUI_VIEW_MENU_BUTTON_PAD_X * 2;
    metrics->content_region.size.height = region.size.height - EGUI_VIEW_MENU_BUTTON_PAD_Y * 2;

    metrics->trigger_region = metrics->content_region;
    metrics->trigger_region.size.height = trigger_h;

    metrics->trigger_icon_region.location.x = metrics->trigger_region.location.x + EGUI_VIEW_MENU_BUTTON_TRIGGER_PAD_X;
    metrics->trigger_icon_region.location.y = metrics->trigger_region.location.y + (trigger_h - EGUI_VIEW_MENU_BUTTON_ICON_SIZE) / 2;
    metrics->trigger_icon_region.size.width = EGUI_VIEW_MENU_BUTTON_ICON_SIZE;
    metrics->trigger_icon_region.size.height = EGUI_VIEW_MENU_BUTTON_ICON_SIZE;

    metrics->trigger_chevron_region.size.width = EGUI_VIEW_MENU_BUTTON_CHEVRON_SIZE;
    metrics->trigger_chevron_region.size.height = EGUI_VIEW_MENU_BUTTON_CHEVRON_SIZE;
    metrics->trigger_chevron_region.location.x = metrics->trigger_region.location.x + metrics->trigger_region.size.width -
                                                 EGUI_VIEW_MENU_BUTTON_TRIGGER_PAD_X - metrics->trigger_chevron_region.size.width;
    metrics->trigger_chevron_region.location.y =
            metrics->trigger_region.location.y + (trigger_h - metrics->trigger_chevron_region.size.height) / 2;

    label_x = metrics->trigger_region.location.x + EGUI_VIEW_MENU_BUTTON_TRIGGER_PAD_X;
    if (egui_view_menu_button_has_text(local->button_icon) && !local->compact_mode)
    {
        label_x = metrics->trigger_icon_region.location.x + metrics->trigger_icon_region.size.width + EGUI_VIEW_MENU_BUTTON_TRIGGER_TEXT_GAP;
    }
    label_right = metrics->trigger_chevron_region.location.x - EGUI_VIEW_MENU_BUTTON_TRIGGER_END_GAP;
    if (label_right < label_x)
    {
        label_right = label_x;
    }
    metrics->trigger_label_region.location.x = label_x;
    metrics->trigger_label_region.location.y = metrics->trigger_region.location.y;
    metrics->trigger_label_region.size.width = label_right - label_x;
    metrics->trigger_label_region.size.height = trigger_h;

    metrics->show_menu_title = (uint8_t)(!local->compact_mode && egui_view_menu_button_has_text(local->menu_title));
    menu_y = metrics->trigger_region.location.y + metrics->trigger_region.size.height + EGUI_VIEW_MENU_BUTTON_MENU_GAP;
    menu_h = EGUI_VIEW_MENU_BUTTON_MENU_PAD * 2;
    if (metrics->show_menu_title)
    {
        menu_h += EGUI_VIEW_MENU_BUTTON_MENU_TITLE_HEIGHT + EGUI_VIEW_MENU_BUTTON_ITEM_GAP;
    }
    if (local->item_count > 0)
    {
        menu_h += (egui_dim_t)local->item_count * EGUI_VIEW_MENU_BUTTON_ITEM_HEIGHT +
                  (egui_dim_t)(local->item_count - 1U) * EGUI_VIEW_MENU_BUTTON_ITEM_GAP;
    }

    metrics->menu_region.location.x = metrics->trigger_region.location.x;
    metrics->menu_region.location.y = menu_y;
    metrics->menu_region.size.width = metrics->trigger_region.size.width;
    metrics->menu_region.size.height = menu_h;

    metrics->menu_title_region.location.x = metrics->menu_region.location.x + EGUI_VIEW_MENU_BUTTON_MENU_PAD;
    metrics->menu_title_region.location.y = metrics->menu_region.location.y + EGUI_VIEW_MENU_BUTTON_MENU_PAD;
    metrics->menu_title_region.size.width = metrics->menu_region.size.width - EGUI_VIEW_MENU_BUTTON_MENU_PAD * 2;
    metrics->menu_title_region.size.height = metrics->show_menu_title ? EGUI_VIEW_MENU_BUTTON_MENU_TITLE_HEIGHT : 0;

    row_y = metrics->menu_region.location.y + EGUI_VIEW_MENU_BUTTON_MENU_PAD;
    if (metrics->show_menu_title)
    {
        row_y += EGUI_VIEW_MENU_BUTTON_MENU_TITLE_HEIGHT + EGUI_VIEW_MENU_BUTTON_ITEM_GAP;
    }
    for (index = 0; index < EGUI_VIEW_MENU_BUTTON_MAX_ITEMS; ++index)
    {
        item_x = metrics->menu_region.location.x + EGUI_VIEW_MENU_BUTTON_MENU_PAD;
        item_w = metrics->menu_region.size.width - EGUI_VIEW_MENU_BUTTON_MENU_PAD * 2;
        metrics->item_regions[index].location.x = item_x;
        metrics->item_regions[index].location.y = row_y + index * (EGUI_VIEW_MENU_BUTTON_ITEM_HEIGHT + EGUI_VIEW_MENU_BUTTON_ITEM_GAP);
        metrics->item_regions[index].size.width = item_w;
        metrics->item_regions[index].size.height = index < local->item_count ? EGUI_VIEW_MENU_BUTTON_ITEM_HEIGHT : 0;

        metrics->item_icon_regions[index].location.x =
                item_x + (EGUI_VIEW_MENU_BUTTON_ITEM_ICON_SLOT - EGUI_VIEW_MENU_BUTTON_ICON_SIZE) / 2;
        metrics->item_icon_regions[index].location.y =
                metrics->item_regions[index].location.y + (EGUI_VIEW_MENU_BUTTON_ITEM_HEIGHT - EGUI_VIEW_MENU_BUTTON_ICON_SIZE) / 2;
        metrics->item_icon_regions[index].size.width = EGUI_VIEW_MENU_BUTTON_ICON_SIZE;
        metrics->item_icon_regions[index].size.height = index < local->item_count ? EGUI_VIEW_MENU_BUTTON_ICON_SIZE : 0;

        metrics->item_check_regions[index].size.width = EGUI_VIEW_MENU_BUTTON_ITEM_CHECK_SIZE;
        metrics->item_check_regions[index].size.height = index < local->item_count ? EGUI_VIEW_MENU_BUTTON_ITEM_CHECK_SIZE : 0;
        metrics->item_check_regions[index].location.x =
                item_x + item_w - EGUI_VIEW_MENU_BUTTON_ITEM_CHECK_END_GAP - metrics->item_check_regions[index].size.width;
        metrics->item_check_regions[index].location.y =
                metrics->item_regions[index].location.y +
                (EGUI_VIEW_MENU_BUTTON_ITEM_HEIGHT - metrics->item_check_regions[index].size.height) / 2;

        metrics->item_shortcut_regions[index].location.x =
                metrics->item_check_regions[index].location.x - EGUI_VIEW_MENU_BUTTON_SHORTCUT_WIDTH;
        metrics->item_shortcut_regions[index].location.y = metrics->item_regions[index].location.y;
        metrics->item_shortcut_regions[index].size.width =
                EGUI_VIEW_MENU_BUTTON_SHORTCUT_WIDTH > EGUI_VIEW_MENU_BUTTON_ITEM_TEXT_GAP
                        ? EGUI_VIEW_MENU_BUTTON_SHORTCUT_WIDTH - EGUI_VIEW_MENU_BUTTON_ITEM_TEXT_GAP
                        : EGUI_VIEW_MENU_BUTTON_SHORTCUT_WIDTH;
        metrics->item_shortcut_regions[index].size.height = metrics->item_regions[index].size.height;

        metrics->item_label_regions[index].location.x =
                item_x + EGUI_VIEW_MENU_BUTTON_ITEM_ICON_SLOT + EGUI_VIEW_MENU_BUTTON_ITEM_TEXT_GAP;
        metrics->item_label_regions[index].location.y = metrics->item_regions[index].location.y;
        label_right = metrics->item_shortcut_regions[index].location.x - EGUI_VIEW_MENU_BUTTON_ITEM_TEXT_GAP;
        if (label_right < metrics->item_label_regions[index].location.x)
        {
            label_right = metrics->item_label_regions[index].location.x;
        }
        metrics->item_label_regions[index].size.width = label_right - metrics->item_label_regions[index].location.x;
        metrics->item_label_regions[index].size.height = metrics->item_regions[index].size.height;
    }

    metrics->status_region.location.x = metrics->content_region.location.x + EGUI_VIEW_MENU_BUTTON_STATUS_INSET_X;
    metrics->status_region.location.y = metrics->trigger_region.location.y + metrics->trigger_region.size.height + 6;
    metrics->status_region.size.width = metrics->content_region.size.width - EGUI_VIEW_MENU_BUTTON_STATUS_INSET_X * 2;
    metrics->status_region.size.height = 18;
    metrics->status_icon_region.location.x =
            metrics->status_region.location.x + (EGUI_VIEW_MENU_BUTTON_STATUS_ICON_SLOT - EGUI_VIEW_MENU_BUTTON_STATUS_ICON_SIZE) / 2;
    metrics->status_icon_region.location.y =
            metrics->status_region.location.y + (metrics->status_region.size.height - EGUI_VIEW_MENU_BUTTON_STATUS_ICON_SIZE) / 2;
    metrics->status_icon_region.size.width = EGUI_VIEW_MENU_BUTTON_STATUS_ICON_SIZE;
    metrics->status_icon_region.size.height = EGUI_VIEW_MENU_BUTTON_STATUS_ICON_SIZE;
    metrics->status_label_region.location.x = metrics->status_region.location.x + EGUI_VIEW_MENU_BUTTON_STATUS_ICON_SLOT;
    metrics->status_label_region.location.y = metrics->status_region.location.y;
    label_right = metrics->status_region.location.x + metrics->status_region.size.width - EGUI_VIEW_MENU_BUTTON_STATUS_END_GAP;
    if (label_right < metrics->status_label_region.location.x)
    {
        label_right = metrics->status_label_region.location.x;
    }
    metrics->status_label_region.size.width = label_right - metrics->status_label_region.location.x;
    metrics->status_label_region.size.height = metrics->status_region.size.height;
}

static uint8_t egui_view_menu_button_hit_target(egui_view_menu_button_t *local, egui_view_t *self, egui_dim_t screen_x, egui_dim_t screen_y)
{
    egui_view_menu_button_metrics_t metrics;
    egui_dim_t local_x = screen_x - self->region_screen.location.x;
    egui_dim_t local_y = screen_y - self->region_screen.location.y;
    uint8_t index;

    egui_view_menu_button_get_metrics(local, self, &metrics);
    if (egui_region_pt_in_rect(&metrics.trigger_region, local_x, local_y))
    {
        return EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER;
    }
    if (local->is_open && !local->compact_mode)
    {
        for (index = 0; index < local->item_count; ++index)
        {
            if (egui_region_pt_in_rect(&metrics.item_regions[index], local_x, local_y))
            {
                return index;
            }
        }
    }
    return EGUI_VIEW_MENU_BUTTON_INDEX_NONE;
}

static uint8_t egui_view_menu_button_target_enabled(egui_view_menu_button_t *local, egui_view_t *self, uint8_t target)
{
    if (!egui_view_menu_button_is_interactive(local, self))
    {
        return 0;
    }
    if (target == EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER)
    {
        return 1;
    }
    return egui_view_menu_button_item_is_enabled(local, target);
}

void egui_view_menu_button_set_button(egui_view_t *self, const char *label, const char *icon)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);

    egui_view_menu_button_clear_active_state(self, local);
    local->button_label = label != NULL ? label : "";
    local->button_icon = icon;
    egui_view_invalidate(self);
}

void egui_view_menu_button_set_menu_title(egui_view_t *self, const char *title)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);

    egui_view_menu_button_clear_active_state(self, local);
    local->menu_title = title != NULL ? title : "";
    egui_view_invalidate(self);
}

void egui_view_menu_button_set_items(egui_view_t *self, const egui_view_menu_button_item_t *items, uint8_t item_count)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);
    uint8_t index;

    egui_view_menu_button_clear_active_state(self, local);
    if (items == NULL)
    {
        item_count = 0;
    }
    if (item_count > EGUI_VIEW_MENU_BUTTON_MAX_ITEMS)
    {
        item_count = EGUI_VIEW_MENU_BUTTON_MAX_ITEMS;
    }
    for (index = 0; index < EGUI_VIEW_MENU_BUTTON_MAX_ITEMS; ++index)
    {
        local->items[index].label = "";
        local->items[index].icon = NULL;
        local->items[index].shortcut = "";
        local->items[index].tone = EGUI_VIEW_MENU_BUTTON_TONE_NEUTRAL;
        local->items[index].checked = 0;
        local->items[index].disabled = 0;
    }
    for (index = 0; index < item_count; ++index)
    {
        local->items[index] = items[index];
        local->items[index].label = local->items[index].label != NULL ? local->items[index].label : "";
        local->items[index].shortcut = local->items[index].shortcut != NULL ? local->items[index].shortcut : "";
        local->items[index].checked = local->items[index].checked ? 1 : 0;
        local->items[index].disabled = local->items[index].disabled ? 1 : 0;
        if (local->items[index].tone > EGUI_VIEW_MENU_BUTTON_TONE_NEUTRAL)
        {
            local->items[index].tone = EGUI_VIEW_MENU_BUTTON_TONE_NEUTRAL;
        }
    }
    local->item_count = item_count;
    local->selected_index = EGUI_VIEW_MENU_BUTTON_INDEX_NONE;
    for (index = 0; index < local->item_count; ++index)
    {
        if (local->items[index].checked && !local->items[index].disabled)
        {
            local->selected_index = index;
            break;
        }
    }
    egui_view_menu_button_normalize_indices(local);
    egui_view_invalidate(self);
}

void egui_view_menu_button_set_selected_index(egui_view_t *self, uint8_t index)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);

    egui_view_menu_button_clear_active_state(self, local);
    if (egui_view_menu_button_item_is_enabled(local, index))
    {
        local->selected_index = index;
        local->focus_index = index;
    }
    egui_view_menu_button_normalize_indices(local);
    egui_view_invalidate(self);
}

uint8_t egui_view_menu_button_get_selected_index(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);
    return local->selected_index;
}

void egui_view_menu_button_set_open(egui_view_t *self, uint8_t is_open)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);

    egui_view_menu_button_clear_active_state(self, local);
    local->is_open = (uint8_t)(is_open && !local->compact_mode && local->item_count > 0);
    egui_view_menu_button_normalize_indices(local);
    egui_view_invalidate(self);
}

uint8_t egui_view_menu_button_get_open(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);
    return local->is_open;
}

void egui_view_menu_button_set_fonts(egui_view_t *self, const egui_font_t *label_font, const egui_font_t *meta_font, const egui_font_t *icon_font)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);

    egui_view_menu_button_clear_active_state(self, local);
    local->label_font = label_font != NULL ? label_font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = meta_font != NULL ? meta_font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->icon_font = icon_font != NULL ? icon_font : EGUI_FONT_ICON_MS_16;
    egui_view_invalidate(self);
}

void egui_view_menu_button_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);

    egui_view_menu_button_clear_active_state(self, local);
    local->compact_mode = compact_mode ? 1 : 0;
    if (local->compact_mode)
    {
        local->is_open = 0;
    }
    egui_view_invalidate(self);
}

void egui_view_menu_button_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);

    egui_view_menu_button_clear_active_state(self, local);
    local->read_only_mode = read_only_mode ? 1 : 0;
    if (local->read_only_mode)
    {
        local->is_open = 0;
    }
    egui_view_invalidate(self);
}

void egui_view_menu_button_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t menu_color, egui_color_t border_color,
                                       egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                       egui_color_t success_color, egui_color_t warning_color, egui_color_t danger_color,
                                       egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);

    egui_view_menu_button_clear_active_state(self, local);
    local->surface_color = surface_color;
    local->menu_color = menu_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->danger_color = danger_color;
    local->neutral_color = neutral_color;
    egui_view_invalidate(self);
}

void egui_view_menu_button_set_on_action_listener(egui_view_t *self, egui_view_menu_button_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);
    local->on_action = listener;
}

uint8_t egui_view_menu_button_activate_item(egui_view_t *self, uint8_t index)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);
    uint8_t item_index;

    if (!egui_view_menu_button_is_interactive(local, self) || !egui_view_menu_button_item_is_enabled(local, index))
    {
        egui_view_menu_button_clear_active_state(self, local);
        return 0;
    }

    local->selected_index = index;
    local->focus_index = index;
    local->is_open = 0;
    for (item_index = 0; item_index < local->item_count; ++item_index)
    {
        local->items[item_index].checked = (uint8_t)(item_index == index);
    }
    if (local->on_action != NULL)
    {
        local->on_action(self, index);
    }
    egui_view_menu_button_clear_active_state(self, local);
    egui_view_invalidate(self);
    return 1;
}

uint8_t egui_view_menu_button_get_trigger_region(egui_view_t *self, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);
    egui_view_menu_button_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }
    egui_view_menu_button_get_metrics(local, self, &metrics);
    *region = metrics.trigger_region;
    region->location.x += self->region_screen.location.x;
    region->location.y += self->region_screen.location.y;
    return (uint8_t)(region->size.width > 0 && region->size.height > 0);
}

uint8_t egui_view_menu_button_get_item_region(egui_view_t *self, uint8_t index, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);
    egui_view_menu_button_metrics_t metrics;

    if (region == NULL || !egui_view_menu_button_is_valid_index(local, index))
    {
        return 0;
    }
    egui_view_menu_button_get_metrics(local, self, &metrics);
    *region = metrics.item_regions[index];
    region->location.x += self->region_screen.location.x;
    region->location.y += self->region_screen.location.y;
    return (uint8_t)(region->size.width > 0 && region->size.height > 0);
}

static void egui_view_menu_button_draw_closed_status(egui_view_t *self, egui_view_menu_button_t *local, const egui_view_menu_button_metrics_t *metrics)
{
    const char *selected_label = "";
    egui_color_t status_fill = egui_rgb_mix(local->surface_color, local->accent_color, EGUI_ALPHA_MAKE(22));
    egui_color_t status_border = egui_rgb_mix(local->border_color, local->accent_color, EGUI_ALPHA_MAKE(38));
    egui_color_t status_text = egui_rgb_mix(local->text_color, local->accent_color, EGUI_ALPHA_MAKE(28));

    if (local->selected_index < local->item_count)
    {
        selected_label = local->items[local->selected_index].label;
    }

    if (local->read_only_mode)
    {
        status_fill = HCW_COLOR_SURFACE_DISABLED;
        status_border = egui_rgb_mix(local->border_color, local->muted_text_color, EGUI_ALPHA_MAKE(22));
        status_text = local->muted_text_color;
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics->status_region.location.x, metrics->status_region.location.y,
                                          metrics->status_region.size.width, metrics->status_region.size.height, 6, status_fill,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics->status_region.location.x, metrics->status_region.location.y,
                                     metrics->status_region.size.width, metrics->status_region.size.height, 6, 1, status_border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(86)));

    egui_view_menu_button_draw_text(local->icon_font, self, EGUI_ICON_MS_DONE, &metrics->status_icon_region, EGUI_ALIGN_CENTER, status_text,
                                    EGUI_ALPHA_100);
    egui_view_menu_button_draw_text(local->meta_font, self, selected_label, &metrics->status_label_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                    status_text, EGUI_ALPHA_100);
}

static void egui_view_menu_button_draw_trigger(egui_view_t *self, egui_view_menu_button_t *local, const egui_view_menu_button_metrics_t *metrics)
{
    egui_color_t fill = local->is_open ? egui_rgb_mix(local->accent_color, EGUI_COLOR_BLACK, EGUI_ALPHA_MAKE(10)) : local->accent_color;
    egui_color_t border = egui_rgb_mix(local->accent_color, EGUI_COLOR_BLACK, EGUI_ALPHA_MAKE(local->is_open ? 18 : 12));
    egui_color_t text = HCW_COLOR_ON_PRIMARY;
    egui_color_t icon = HCW_COLOR_ON_PRIMARY;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_MENU_BUTTON_COMPACT_RADIUS : EGUI_VIEW_MENU_BUTTON_RADIUS;
    uint8_t is_active = (uint8_t)(local->active_target == EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER && self->is_pressed);
    uint8_t is_focused = 0;

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    is_focused = self->is_focused ? 1 : 0;
#endif

    if (is_active)
    {
        fill = egui_rgb_mix(fill, EGUI_COLOR_BLACK, EGUI_ALPHA_MAKE(14));
        border = egui_rgb_mix(border, EGUI_COLOR_BLACK, EGUI_ALPHA_MAKE(18));
    }
    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        fill = HCW_COLOR_SURFACE_DISABLED;
        border = egui_rgb_mix(local->border_color, local->muted_text_color, EGUI_ALPHA_MAKE(24));
        text = local->muted_text_color;
        icon = local->muted_text_color;
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics->trigger_region.location.x, metrics->trigger_region.location.y,
                                          metrics->trigger_region.size.width, metrics->trigger_region.size.height, radius, fill,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics->trigger_region.location.x, metrics->trigger_region.location.y,
                                     metrics->trigger_region.size.width, metrics->trigger_region.size.height, radius, is_focused ? 2 : 1, border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));

    if (egui_view_menu_button_has_text(local->button_icon) && !local->compact_mode)
    {
        egui_view_menu_button_draw_text(local->icon_font, self, local->button_icon, &metrics->trigger_icon_region, EGUI_ALIGN_CENTER, icon,
                                        EGUI_ALPHA_100);
    }
    egui_view_menu_button_draw_text(local->label_font, self, local->button_label, &metrics->trigger_label_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                    text, EGUI_ALPHA_100);
    egui_view_menu_button_draw_chevron(self, &metrics->trigger_chevron_region, icon, local->is_open);
}

static void egui_view_menu_button_draw_menu(egui_view_t *self, egui_view_menu_button_t *local, const egui_view_menu_button_metrics_t *metrics)
{
    egui_color_t menu_fill = egui_rgb_mix(local->menu_color, local->accent_color, EGUI_ALPHA_MAKE(26));
    egui_color_t menu_border = egui_rgb_mix(local->border_color, local->accent_color, EGUI_ALPHA_MAKE(52));
    egui_color_t title_text = egui_rgb_mix(local->text_color, local->accent_color, EGUI_ALPHA_MAKE(24));
    uint8_t index;

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics->menu_region.location.x, metrics->menu_region.location.y,
                                          metrics->menu_region.size.width, metrics->menu_region.size.height, 8, menu_fill,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics->menu_region.location.x, metrics->menu_region.location.y,
                                     metrics->menu_region.size.width, metrics->menu_region.size.height, 8, 1, menu_border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(90)));

    if (metrics->show_menu_title)
    {
        egui_view_menu_button_draw_text(local->meta_font, self, local->menu_title, &metrics->menu_title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                        title_text, EGUI_ALPHA_100);
    }

    for (index = 0; index < local->item_count; ++index)
    {
        egui_view_menu_button_item_t *item = &local->items[index];
        egui_region_t item_region = metrics->item_regions[index];
        egui_color_t tone = egui_view_menu_button_tone_color(local, item->tone);
        egui_color_t row_fill = index == local->focus_index ? egui_rgb_mix(local->surface_color, tone, EGUI_ALPHA_MAKE(42))
                                                            : egui_rgb_mix(local->surface_color, tone, EGUI_ALPHA_MAKE(26));
        egui_color_t row_border = index == local->focus_index ? egui_rgb_mix(local->border_color, tone, EGUI_ALPHA_MAKE(58))
                                                              : egui_rgb_mix(local->border_color, tone, EGUI_ALPHA_MAKE(38));
        egui_color_t label = index == local->selected_index ? tone : local->text_color;
        egui_color_t meta = egui_rgb_mix(local->muted_text_color, tone, EGUI_ALPHA_MAKE(item->checked ? 28 : 12));
        uint8_t item_pressed = (uint8_t)(local->active_target == index && self->is_pressed);

        if (item_pressed)
        {
            row_fill = egui_rgb_mix(row_fill, tone, EGUI_ALPHA_MAKE(32));
            row_border = egui_rgb_mix(row_border, tone, EGUI_ALPHA_MAKE(32));
        }
        if (item->disabled)
        {
            row_fill = egui_view_menu_button_mix_disabled(row_fill);
            row_border = egui_view_menu_button_mix_disabled(row_border);
            label = egui_view_menu_button_mix_disabled(label);
            meta = egui_view_menu_button_mix_disabled(meta);
        }

        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, item_region.location.x, item_region.location.y, item_region.size.width,
                                              item_region.size.height, 6, row_fill, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, item_region.location.x, item_region.location.y, item_region.size.width,
                                         item_region.size.height, 6, 1, row_border, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(92)));

        egui_view_menu_button_draw_text(local->icon_font, self, item->icon, &metrics->item_icon_regions[index], EGUI_ALIGN_CENTER, meta,
                                        EGUI_ALPHA_100);
        if (item->checked || index == local->selected_index)
        {
            egui_view_menu_button_draw_text(local->icon_font, self, EGUI_ICON_MS_DONE, &metrics->item_check_regions[index], EGUI_ALIGN_CENTER, tone,
                                            EGUI_ALPHA_100);
        }

        egui_view_menu_button_draw_text(local->meta_font, self, item->shortcut, &metrics->item_shortcut_regions[index],
                                        EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER, meta, EGUI_ALPHA_MAKE(96));
        egui_view_menu_button_draw_text(local->label_font, self, item->label, &metrics->item_label_regions[index],
                                        EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, label, EGUI_ALPHA_100);
    }
}

static void egui_view_menu_button_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);
    egui_view_menu_button_metrics_t metrics;

    egui_view_menu_button_get_metrics(local, self, &metrics);
    if (metrics.content_region.size.width <= 0 || metrics.content_region.size.height <= 0)
    {
        return;
    }

    egui_view_menu_button_draw_trigger(self, local, &metrics);
    if (local->is_open && !local->compact_mode)
    {
        egui_view_menu_button_draw_menu(self, local, &metrics);
    }
    else if (!local->compact_mode)
    {
        egui_view_menu_button_draw_closed_status(self, local, &metrics);
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_menu_button_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);
    uint8_t hit_target;
    uint8_t handled;

    if (!egui_view_menu_button_is_interactive(local, self))
    {
        if (egui_view_menu_button_clear_active_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    hit_target = egui_view_menu_button_hit_target(local, self, event->location.x, event->location.y);
    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        if (hit_target == EGUI_VIEW_MENU_BUTTON_INDEX_NONE)
        {
            if (local->is_open)
            {
                local->is_open = 0;
                egui_view_menu_button_clear_active_state(self, local);
                egui_view_invalidate(self);
                return 1;
            }
            return 0;
        }
        if (!egui_view_menu_button_target_enabled(local, self, hit_target))
        {
            return 1;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        local->active_target = hit_target;
        if (hit_target != EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER)
        {
            local->focus_index = hit_target;
        }
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->active_target == EGUI_VIEW_MENU_BUTTON_INDEX_NONE)
        {
            return 0;
        }
        egui_view_set_pressed(self, (uint8_t)(hit_target == local->active_target &&
                                             egui_view_menu_button_target_enabled(local, self, local->active_target)));
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
        handled = (uint8_t)(local->active_target != EGUI_VIEW_MENU_BUTTON_INDEX_NONE);
        if (handled && self->is_pressed && hit_target == local->active_target)
        {
            if (hit_target == EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER)
            {
                local->is_open = local->is_open ? 0 : 1;
                egui_view_menu_button_normalize_indices(local);
            }
            else
            {
                egui_view_menu_button_activate_item(self, hit_target);
            }
        }
        if (egui_view_menu_button_clear_active_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return (uint8_t)(handled || hit_target != EGUI_VIEW_MENU_BUTTON_INDEX_NONE);
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (egui_view_menu_button_clear_active_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_menu_button_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);
    EGUI_UNUSED(event);

    if (egui_view_menu_button_clear_active_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_menu_button_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);
    uint8_t had_active = (uint8_t)(local->active_target != EGUI_VIEW_MENU_BUTTON_INDEX_NONE && self->is_pressed);

    if (!egui_view_menu_button_is_interactive(local, self))
    {
        if (egui_view_menu_button_clear_active_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_DOWN:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->is_open = 1;
            local->focus_index = egui_view_menu_button_item_is_enabled(local, local->focus_index)
                                         ? egui_view_menu_button_find_next_enabled(local, local->focus_index, 1)
                                         : egui_view_menu_button_first_enabled(local);
            egui_view_menu_button_clear_active_state(self, local);
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_KEY_CODE_UP:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->is_open = 1;
            local->focus_index = egui_view_menu_button_item_is_enabled(local, local->focus_index)
                                         ? egui_view_menu_button_find_next_enabled(local, local->focus_index, -1)
                                         : egui_view_menu_button_first_enabled(local);
            egui_view_menu_button_clear_active_state(self, local);
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_KEY_CODE_HOME:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->is_open = 1;
            local->focus_index = egui_view_menu_button_first_enabled(local);
            egui_view_menu_button_clear_active_state(self, local);
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_KEY_CODE_END:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->is_open = 1;
            local->focus_index = egui_view_menu_button_find_next_enabled(local, 0, -1);
            egui_view_menu_button_clear_active_state(self, local);
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_KEY_CODE_ESCAPE:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->is_open = 0;
            egui_view_menu_button_clear_active_state(self, local);
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->active_target = local->is_open ? local->focus_index : EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER;
            if (!egui_view_menu_button_target_enabled(local, self, local->active_target))
            {
                local->active_target = EGUI_VIEW_MENU_BUTTON_INDEX_NONE;
                return 0;
            }
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            if (had_active)
            {
                if (local->active_target == EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER)
                {
                    local->is_open = local->is_open ? 0 : 1;
                    egui_view_menu_button_normalize_indices(local);
                }
                else
                {
                    egui_view_menu_button_activate_item(self, local->active_target);
                }
            }
            egui_view_menu_button_clear_active_state(self, local);
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        if (egui_view_menu_button_clear_active_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_menu_button_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_menu_button_t);
    EGUI_UNUSED(event);

    if (egui_view_menu_button_clear_active_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_menu_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_menu_button_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_menu_button_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_menu_button_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_menu_button_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_menu_button_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_menu_button_on_key_event,
#endif
};

void egui_view_menu_button_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_menu_button_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_menu_button_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->button_label = "Menu";
    local->button_icon = EGUI_ICON_MS_SETTINGS;
    local->menu_title = "Actions";
    local->label_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->icon_font = EGUI_FONT_ICON_MS_16;
    local->on_action = NULL;
    local->surface_color = HCW_COLOR_SURFACE;
    local->menu_color = HCW_COLOR_SURFACE_SUBTLE;
    local->border_color = HCW_COLOR_BORDER_STRONG;
    local->text_color = HCW_COLOR_TEXT_STRONG;
    local->muted_text_color = HCW_COLOR_TEXT_SOFT;
    local->accent_color = HCW_COLOR_PRIMARY_DARK;
    local->success_color = HCW_COLOR_SUCCESS;
    local->warning_color = HCW_COLOR_WARNING;
    local->danger_color = HCW_COLOR_DANGER_DARK;
    local->neutral_color = HCW_COLOR_TEXT_SOFT;
    local->item_count = 0;
    local->selected_index = EGUI_VIEW_MENU_BUTTON_INDEX_NONE;
    local->focus_index = EGUI_VIEW_MENU_BUTTON_INDEX_NONE;
    local->active_target = EGUI_VIEW_MENU_BUTTON_INDEX_NONE;
    local->is_open = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;

    egui_view_set_view_name(self, "egui_view_menu_button");
}
