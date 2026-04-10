#include "egui_view_auto_suggest_box.h"

static uint8_t hcw_auto_suggest_box_clear_interaction_state(egui_view_t *self)
{
    egui_view_combobox_t *local = (egui_view_combobox_t *)self;
    uint8_t had_state = self->is_pressed || local->pressed_index != EGUI_VIEW_COMBOBOX_PRESSED_NONE || local->pressed_is_header || local->is_expanded;

    if (self->is_pressed)
    {
        egui_view_set_pressed(self, false);
    }
    local->pressed_index = EGUI_VIEW_COMBOBOX_PRESSED_NONE;
    local->pressed_is_header = 0;
    if (local->is_expanded)
    {
        egui_view_autocomplete_collapse(self);
    }
    else if (had_state)
    {
        egui_view_invalidate(self);
    }

    return had_state;
}

static void hcw_auto_suggest_box_apply_palette(egui_view_t *self, egui_dim_t collapsed_height, egui_dim_t item_height, uint8_t max_visible_items,
                                               egui_color_t bg_color, egui_color_t border_color, egui_color_t text_color, egui_color_t arrow_color,
                                               egui_color_t highlight_color)
{
    egui_view_combobox_t *local = (egui_view_combobox_t *)self;

    hcw_auto_suggest_box_clear_interaction_state(self);
    local->collapsed_height = collapsed_height;
    local->item_height = item_height;
    local->bg_color = bg_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->arrow_color = arrow_color;
    local->highlight_color = highlight_color;
    egui_view_autocomplete_set_max_visible_items(self, max_visible_items);
    egui_view_invalidate(self);
}

void hcw_auto_suggest_box_apply_standard_style(egui_view_t *self)
{
    hcw_auto_suggest_box_apply_palette(self, 34, 24, 4, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89),
                                       EGUI_COLOR_HEX(0xEAF3FB));
}

void hcw_auto_suggest_box_apply_compact_style(egui_view_t *self)
{
    hcw_auto_suggest_box_apply_palette(self, 28, 21, 3, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89),
                                       EGUI_COLOR_HEX(0xEAF3FB));
}

void hcw_auto_suggest_box_apply_read_only_style(egui_view_t *self)
{
    hcw_auto_suggest_box_apply_palette(self, 28, 21, 3, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x7A8796),
                                       EGUI_COLOR_HEX(0xEEF3F8));
}

void hcw_auto_suggest_box_set_suggestions(egui_view_t *self, const char **suggestions, uint8_t count)
{
    hcw_auto_suggest_box_clear_interaction_state(self);
    egui_view_autocomplete_set_suggestions(self, suggestions, count);
}

void hcw_auto_suggest_box_set_current_index(egui_view_t *self, uint8_t index)
{
    hcw_auto_suggest_box_clear_interaction_state(self);
    egui_view_autocomplete_set_current_index(self, index);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_auto_suggest_box_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_auto_suggest_box_clear_interaction_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_auto_suggest_box_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_auto_suggest_box_clear_interaction_state(self);
    return 1;
}
#endif

void hcw_auto_suggest_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_auto_suggest_box_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_auto_suggest_box_on_static_key_event;
#endif
}
