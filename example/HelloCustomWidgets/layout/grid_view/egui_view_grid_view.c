#include "egui_view_grid_view.h"

void hcw_grid_view_init(egui_view_t *self)
{
    egui_view_items_repeater_init(self);
}

void hcw_grid_view_set_snapshots(egui_view_t *self, const hcw_grid_view_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_items_repeater_set_snapshots(self, snapshots, snapshot_count);
}

void hcw_grid_view_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    egui_view_items_repeater_set_current_snapshot(self, snapshot_index);
}

uint8_t hcw_grid_view_get_current_snapshot(egui_view_t *self)
{
    return egui_view_items_repeater_get_current_snapshot(self);
}

void hcw_grid_view_set_current_item(egui_view_t *self, uint8_t item_index)
{
    egui_view_items_repeater_set_current_item(self, item_index);
}

uint8_t hcw_grid_view_get_current_item(egui_view_t *self)
{
    return egui_view_items_repeater_get_current_item(self);
}

void hcw_grid_view_set_layout_mode(egui_view_t *self, uint8_t layout_mode)
{
    egui_view_items_repeater_set_layout_mode(self, layout_mode);
}

uint8_t hcw_grid_view_get_layout_mode(egui_view_t *self)
{
    return egui_view_items_repeater_get_layout_mode(self);
}

uint8_t hcw_grid_view_activate_current_item(egui_view_t *self)
{
    return egui_view_items_repeater_activate_current_item(self);
}

void hcw_grid_view_set_on_action_listener(egui_view_t *self, hcw_on_grid_view_action_listener_t listener)
{
    egui_view_items_repeater_set_on_action_listener(self, listener);
}

void hcw_grid_view_set_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_items_repeater_set_font(self, font);
}

void hcw_grid_view_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_items_repeater_set_meta_font(self, font);
}

void hcw_grid_view_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_items_repeater_set_compact_mode(self, compact_mode);
}

void hcw_grid_view_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_items_repeater_set_read_only_mode(self, read_only_mode);
}

void hcw_grid_view_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                               egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                               egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color)
{
    egui_view_items_repeater_set_palette(self, surface_color, section_color, border_color, text_color, muted_text_color, accent_color, success_color,
                                         warning_color, neutral_color);
}

uint8_t hcw_grid_view_get_item_region(egui_view_t *self, uint8_t item_index, egui_region_t *region)
{
    return egui_view_items_repeater_get_item_region(self, item_index, region);
}

uint8_t hcw_grid_view_handle_navigation_key(egui_view_t *self, uint8_t key_code)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    egui_key_event_t event = {0};
    int handled = 0;

    if (self == NULL || self->api == NULL || self->api->on_key_event == NULL)
    {
        return 0;
    }

    event.key_code = key_code;
    if (key_code == EGUI_KEY_CODE_ENTER || key_code == EGUI_KEY_CODE_SPACE)
    {
        event.type = EGUI_KEY_EVENT_ACTION_DOWN;
        handled |= self->api->on_key_event(self, &event);
        event.type = EGUI_KEY_EVENT_ACTION_UP;
        handled |= self->api->on_key_event(self, &event);
        return handled ? 1 : 0;
    }

    event.type = EGUI_KEY_EVENT_ACTION_UP;
    return self->api->on_key_event(self, &event) ? 1 : 0;
#else
    EGUI_UNUSED(self);
    EGUI_UNUSED(key_code);
    return 0;
#endif
}

void hcw_grid_view_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_items_repeater_override_static_preview_api(self, api);
}
