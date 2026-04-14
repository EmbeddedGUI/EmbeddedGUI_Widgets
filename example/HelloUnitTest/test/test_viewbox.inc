#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_viewbox.h"

#include "../../HelloCustomWidgets/layout/viewbox/egui_view_viewbox.h"
#include "../../HelloCustomWidgets/layout/viewbox/egui_view_viewbox.c"

static egui_view_viewbox_t viewbox_test_widget;
static egui_view_viewbox_t viewbox_preview_widget;
static egui_view_api_t viewbox_preview_api;
static uint8_t viewbox_action_count;
static uint8_t viewbox_action_snapshot;
static uint8_t viewbox_action_preset;
static uint8_t viewbox_action_mode;

static const egui_view_viewbox_preset_t viewbox_presets_0[] = {
        {"Uniform", "Keep ratio", EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM},
        {"Fill", "Stretch shell", EGUI_VIEW_VIEWBOX_STRETCH_FILL},
        {"Down only", "No upscale", EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY},
};

static const egui_view_viewbox_preset_t viewbox_presets_1[] = {
        {"Fit", "Keep ratio", EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM},
        {"Cover", "Stretch shell", EGUI_VIEW_VIEWBOX_STRETCH_FILL},
        {"Hold", "No upscale", EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY},
};

static const egui_view_viewbox_snapshot_t viewbox_snapshots[] = {
        {"OPS", "Device preview", "", "Telemetry deck", "420 x 240 baseline", "Footer stays attached.", "Uniform preset", viewbox_presets_0, 420, 240, 3, 1},
        {"QA", "Inspector thumb", "", "Audit chip", "96 x 64 surface", "Small source stays crisp.", "Downscale only", viewbox_presets_1, 96, 64, 3, 0},
};

static const egui_view_viewbox_preset_t viewbox_overflow_presets[] = {
        {"A", "A", EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM},
        {"B", "B", EGUI_VIEW_VIEWBOX_STRETCH_FILL},
        {"C", "C", EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY},
        {"D", "D", EGUI_VIEW_VIEWBOX_STRETCH_FILL},
};

static const egui_view_viewbox_snapshot_t viewbox_overflow_snapshots[] = {
        {"A", "A", "A", "A", "A", "A", "A", viewbox_overflow_presets, 640, 360, 4, 2},
        {"B", "B", "B", "B", "B", "B", "B", viewbox_presets_0, 420, 240, 3, 1},
        {"C", "C", "C", "C", "C", "C", "C", viewbox_presets_0, 420, 240, 3, 1},
        {"D", "D", "D", "D", "D", "D", "D", viewbox_presets_0, 420, 240, 3, 1},
        {"E", "E", "E", "E", "E", "E", "E", viewbox_presets_0, 420, 240, 3, 1},
        {"F", "F", "F", "F", "F", "F", "F", viewbox_presets_0, 420, 240, 3, 1},
        {"G", "G", "G", "G", "G", "G", "G", viewbox_presets_0, 420, 240, 3, 1},
};

static const egui_view_viewbox_snapshot_t viewbox_preview_snapshot = {
        "OPS", "Compact fit", "", "Tile", "240 x 160", "", "", viewbox_presets_0, 240, 160, 3, 0};

static void viewbox_on_action(egui_view_t *self, uint8_t snapshot_index, uint8_t preset_index, uint8_t stretch_mode)
{
    EGUI_UNUSED(self);
    viewbox_action_count++;
    viewbox_action_snapshot = snapshot_index;
    viewbox_action_preset = preset_index;
    viewbox_action_mode = stretch_mode;
}

static void viewbox_reset_action_state(void)
{
    viewbox_action_count = 0;
    viewbox_action_snapshot = 0xFF;
    viewbox_action_preset = EGUI_VIEW_VIEWBOX_PRESET_NONE;
    viewbox_action_mode = 0xFF;
}

static void viewbox_setup_widget(const egui_view_viewbox_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_viewbox_init(EGUI_VIEW_OF(&viewbox_test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&viewbox_test_widget), 196, 120);
    egui_view_viewbox_set_snapshots(EGUI_VIEW_OF(&viewbox_test_widget), snapshots, snapshot_count);
    egui_view_viewbox_set_font(EGUI_VIEW_OF(&viewbox_test_widget), NULL);
    egui_view_viewbox_set_meta_font(EGUI_VIEW_OF(&viewbox_test_widget), NULL);
    egui_view_viewbox_set_on_action_listener(EGUI_VIEW_OF(&viewbox_test_widget), viewbox_on_action);
    viewbox_reset_action_state();
}

static void viewbox_setup_preview_widget(void)
{
    egui_view_viewbox_init(EGUI_VIEW_OF(&viewbox_preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&viewbox_preview_widget), 104, 78);
    egui_view_viewbox_set_snapshots(EGUI_VIEW_OF(&viewbox_preview_widget), &viewbox_preview_snapshot, 1);
    egui_view_viewbox_set_font(EGUI_VIEW_OF(&viewbox_preview_widget), NULL);
    egui_view_viewbox_set_meta_font(EGUI_VIEW_OF(&viewbox_preview_widget), NULL);
    egui_view_viewbox_set_compact_mode(EGUI_VIEW_OF(&viewbox_preview_widget), 1);
    egui_view_viewbox_override_static_preview_api(EGUI_VIEW_OF(&viewbox_preview_widget), &viewbox_preview_api);
    viewbox_reset_action_state();
}

static void viewbox_layout_view(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static void viewbox_layout_widget(void)
{
    viewbox_layout_view(EGUI_VIEW_OF(&viewbox_test_widget), 10, 20, 196, 120);
}

static void viewbox_layout_preview_widget(void)
{
    viewbox_layout_view(EGUI_VIEW_OF(&viewbox_preview_widget), 12, 18, 104, 78);
}

static int viewbox_send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int viewbox_send_key_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int viewbox_send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return viewbox_send_touch_to_view(EGUI_VIEW_OF(&viewbox_test_widget), type, x, y);
}

static int viewbox_send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return viewbox_send_touch_to_view(EGUI_VIEW_OF(&viewbox_preview_widget), type, x, y);
}

static int viewbox_send_key_action(uint8_t type, uint8_t key_code)
{
    return viewbox_send_key_to_view(EGUI_VIEW_OF(&viewbox_test_widget), type, key_code);
}

static int viewbox_send_preview_key_action(uint8_t type, uint8_t key_code)
{
    return viewbox_send_key_to_view(EGUI_VIEW_OF(&viewbox_preview_widget), type, key_code);
}

static int viewbox_send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= viewbox_send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= viewbox_send_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int viewbox_send_preview_key(uint8_t key_code)
{
    int handled = 0;

    handled |= viewbox_send_preview_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= viewbox_send_preview_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static uint8_t viewbox_get_preset_center(egui_view_t *view, uint8_t preset_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_viewbox_get_preset_region(view, preset_index, &region))
    {
        return 0;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void viewbox_get_view_outside_point(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x - 4;
    *y = view->region_screen.location.y - 4;
}

static void viewbox_seed_pressed_state(egui_view_viewbox_t *widget, uint8_t preset_index, uint8_t visual_pressed)
{
    widget->pressed_preset = preset_index;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void viewbox_assert_pressed_cleared(egui_view_viewbox_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_VIEWBOX_PRESET_NONE, widget->pressed_preset);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void viewbox_assert_widget_state(egui_view_viewbox_t *widget, uint8_t snapshot, uint8_t preset, uint8_t stretch_mode, uint8_t compact_mode,
                                        uint8_t read_only_mode)
{
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot, egui_view_viewbox_get_current_snapshot(EGUI_VIEW_OF(widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(preset, egui_view_viewbox_get_current_preset(EGUI_VIEW_OF(widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(stretch_mode, egui_view_viewbox_get_stretch_mode(EGUI_VIEW_OF(widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(compact_mode, widget->compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(read_only_mode, widget->read_only_mode);
}

static void viewbox_assert_action_state(uint8_t count, uint8_t snapshot, uint8_t preset, uint8_t stretch_mode)
{
    EGUI_TEST_ASSERT_EQUAL_INT(count, viewbox_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot, viewbox_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(preset, viewbox_action_preset);
    EGUI_TEST_ASSERT_EQUAL_INT(stretch_mode, viewbox_action_mode);
}

static void test_viewbox_set_snapshots_clamp_and_defaults(void)
{
    viewbox_setup_widget(viewbox_overflow_snapshots, EGUI_ARRAY_SIZE(viewbox_overflow_snapshots));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_VIEWBOX_MAX_SNAPSHOTS, viewbox_test_widget.snapshot_count);
    viewbox_assert_widget_state(&viewbox_test_widget, 0, 2, EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY, 0, 0);

    viewbox_seed_pressed_state(&viewbox_test_widget, 1, 1);
    viewbox_test_widget.current_snapshot = 5;
    egui_view_viewbox_set_snapshots(EGUI_VIEW_OF(&viewbox_test_widget), viewbox_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, viewbox_test_widget.snapshot_count);
    viewbox_assert_widget_state(&viewbox_test_widget, 0, 1, EGUI_VIEW_VIEWBOX_STRETCH_FILL, 0, 0);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);

    egui_view_viewbox_set_current_preset(EGUI_VIEW_OF(&viewbox_test_widget), EGUI_VIEW_VIEWBOX_PRESET_NONE);
    viewbox_assert_widget_state(&viewbox_test_widget, 0, 1, EGUI_VIEW_VIEWBOX_STRETCH_FILL, 0, 0);

    egui_view_viewbox_set_snapshots(EGUI_VIEW_OF(&viewbox_test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, viewbox_test_widget.snapshot_count);
    viewbox_assert_widget_state(&viewbox_test_widget, 0, EGUI_VIEW_VIEWBOX_PRESET_NONE, EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM, 0, 0);
}

static void test_viewbox_setters_clear_pressed_and_update_state(void)
{
    viewbox_setup_widget(viewbox_snapshots, EGUI_ARRAY_SIZE(viewbox_snapshots));

    viewbox_seed_pressed_state(&viewbox_test_widget, 1, 1);
    egui_view_viewbox_set_font(EGUI_VIEW_OF(&viewbox_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(viewbox_test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);

    viewbox_seed_pressed_state(&viewbox_test_widget, 1, 1);
    egui_view_viewbox_set_meta_font(EGUI_VIEW_OF(&viewbox_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(viewbox_test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);

    viewbox_seed_pressed_state(&viewbox_test_widget, 1, 1);
    egui_view_viewbox_set_compact_mode(EGUI_VIEW_OF(&viewbox_test_widget), 2);
    viewbox_assert_widget_state(&viewbox_test_widget, 0, 1, EGUI_VIEW_VIEWBOX_STRETCH_FILL, 1, 0);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);
    egui_view_viewbox_set_compact_mode(EGUI_VIEW_OF(&viewbox_test_widget), 0);

    viewbox_seed_pressed_state(&viewbox_test_widget, 1, 1);
    egui_view_viewbox_set_read_only_mode(EGUI_VIEW_OF(&viewbox_test_widget), 3);
    viewbox_assert_widget_state(&viewbox_test_widget, 0, 1, EGUI_VIEW_VIEWBOX_STRETCH_FILL, 0, 1);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);
    egui_view_viewbox_set_read_only_mode(EGUI_VIEW_OF(&viewbox_test_widget), 0);

    viewbox_seed_pressed_state(&viewbox_test_widget, 1, 1);
    egui_view_viewbox_set_palette(EGUI_VIEW_OF(&viewbox_test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                  EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, viewbox_test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, viewbox_test_widget.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, viewbox_test_widget.border_color.full);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);

    egui_view_viewbox_set_current_snapshot(EGUI_VIEW_OF(&viewbox_test_widget), 1);
    viewbox_assert_widget_state(&viewbox_test_widget, 1, 0, EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM, 0, 0);

    viewbox_seed_pressed_state(&viewbox_test_widget, 0, 1);
    egui_view_viewbox_set_current_preset(EGUI_VIEW_OF(&viewbox_test_widget), 2);
    viewbox_assert_widget_state(&viewbox_test_widget, 1, 2, EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY, 0, 0);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);

    viewbox_seed_pressed_state(&viewbox_test_widget, 2, 1);
    egui_view_viewbox_set_stretch_mode(EGUI_VIEW_OF(&viewbox_test_widget), EGUI_VIEW_VIEWBOX_STRETCH_FILL);
    viewbox_assert_widget_state(&viewbox_test_widget, 1, 1, EGUI_VIEW_VIEWBOX_STRETCH_FILL, 0, 0);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);
}

static void test_viewbox_regions_activate_and_listener(void)
{
    egui_dim_t x;
    egui_dim_t y;

    viewbox_setup_widget(viewbox_snapshots, EGUI_ARRAY_SIZE(viewbox_snapshots));
    viewbox_layout_widget();

    EGUI_TEST_ASSERT_TRUE(viewbox_get_preset_center(EGUI_VIEW_OF(&viewbox_test_widget), 2, &x, &y));
    egui_view_viewbox_set_current_preset(EGUI_VIEW_OF(&viewbox_test_widget), 2);
    EGUI_TEST_ASSERT_TRUE(egui_view_viewbox_activate_current_preset(EGUI_VIEW_OF(&viewbox_test_widget)));
    viewbox_assert_widget_state(&viewbox_test_widget, 0, 2, EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY, 0, 0);
    viewbox_assert_action_state(1, 0, 2, EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY);

    viewbox_reset_action_state();
    egui_view_viewbox_set_on_action_listener(EGUI_VIEW_OF(&viewbox_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_viewbox_activate_current_preset(EGUI_VIEW_OF(&viewbox_test_widget)));
    viewbox_assert_action_state(0, 0xFF, EGUI_VIEW_VIEWBOX_PRESET_NONE, 0xFF);
}

static void test_viewbox_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t preset_x;
    egui_dim_t preset_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    viewbox_setup_widget(viewbox_snapshots, EGUI_ARRAY_SIZE(viewbox_snapshots));
    viewbox_layout_widget();
    EGUI_TEST_ASSERT_TRUE(viewbox_get_preset_center(EGUI_VIEW_OF(&viewbox_test_widget), 0, &preset_x, &preset_y));
    viewbox_get_view_outside_point(EGUI_VIEW_OF(&viewbox_test_widget), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(viewbox_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, preset_x, preset_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, viewbox_test_widget.pressed_preset);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&viewbox_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&viewbox_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    viewbox_assert_widget_state(&viewbox_test_widget, 0, 1, EGUI_VIEW_VIEWBOX_STRETCH_FILL, 0, 0);
    viewbox_assert_action_state(0, 0xFF, EGUI_VIEW_VIEWBOX_PRESET_NONE, 0xFF);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);

    EGUI_TEST_ASSERT_TRUE(viewbox_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, preset_x, preset_y));
    EGUI_TEST_ASSERT_TRUE(viewbox_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(viewbox_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, preset_x, preset_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&viewbox_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_touch(EGUI_MOTION_EVENT_ACTION_UP, preset_x, preset_y));
    viewbox_assert_widget_state(&viewbox_test_widget, 0, 0, EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM, 0, 0);
    viewbox_assert_action_state(1, 0, 0, EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);

    viewbox_reset_action_state();
    EGUI_TEST_ASSERT_TRUE(viewbox_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, preset_x, preset_y));
    EGUI_TEST_ASSERT_TRUE(viewbox_send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, preset_x, preset_y));
    viewbox_assert_action_state(0, 0xFF, EGUI_VIEW_VIEWBOX_PRESET_NONE, 0xFF);
    EGUI_TEST_ASSERT_EQUAL_INT(0, viewbox_action_count);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);
}

static void test_viewbox_key_navigation_activate_and_tab(void)
{
    viewbox_setup_widget(viewbox_snapshots, EGUI_ARRAY_SIZE(viewbox_snapshots));

    EGUI_TEST_ASSERT_TRUE(viewbox_send_key(EGUI_KEY_CODE_HOME));
    viewbox_assert_widget_state(&viewbox_test_widget, 0, 0, EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM, 0, 0);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_key(EGUI_KEY_CODE_RIGHT));
    viewbox_assert_widget_state(&viewbox_test_widget, 0, 1, EGUI_VIEW_VIEWBOX_STRETCH_FILL, 0, 0);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_key(EGUI_KEY_CODE_END));
    viewbox_assert_widget_state(&viewbox_test_widget, 0, 2, EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY, 0, 0);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_key(EGUI_KEY_CODE_LEFT));
    viewbox_assert_widget_state(&viewbox_test_widget, 0, 1, EGUI_VIEW_VIEWBOX_STRETCH_FILL, 0, 0);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_key(EGUI_KEY_CODE_TAB));
    viewbox_assert_widget_state(&viewbox_test_widget, 0, 2, EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY, 0, 0);

    egui_view_viewbox_set_current_preset(EGUI_VIEW_OF(&viewbox_test_widget), 2);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_key(EGUI_KEY_CODE_TAB));
    viewbox_assert_widget_state(&viewbox_test_widget, 1, 0, EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM, 0, 0);

    EGUI_TEST_ASSERT_TRUE(viewbox_send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(0, viewbox_test_widget.pressed_preset);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&viewbox_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    viewbox_assert_widget_state(&viewbox_test_widget, 1, 0, EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM, 0, 0);
    viewbox_assert_action_state(1, 1, 0, EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);
}

static void test_viewbox_read_only_and_disabled_guards(void)
{
    egui_dim_t preset_x;
    egui_dim_t preset_y;
    uint8_t initial_snapshot;
    uint8_t initial_preset;
    uint8_t initial_mode;

    viewbox_setup_widget(viewbox_snapshots, EGUI_ARRAY_SIZE(viewbox_snapshots));
    viewbox_layout_widget();
    EGUI_TEST_ASSERT_TRUE(viewbox_get_preset_center(EGUI_VIEW_OF(&viewbox_test_widget), 2, &preset_x, &preset_y));

    initial_snapshot = egui_view_viewbox_get_current_snapshot(EGUI_VIEW_OF(&viewbox_test_widget));
    initial_preset = egui_view_viewbox_get_current_preset(EGUI_VIEW_OF(&viewbox_test_widget));
    initial_mode = egui_view_viewbox_get_stretch_mode(EGUI_VIEW_OF(&viewbox_test_widget));
    viewbox_assert_widget_state(&viewbox_test_widget, initial_snapshot, initial_preset, initial_mode, 0, 0);
    viewbox_assert_action_state(0, 0xFF, EGUI_VIEW_VIEWBOX_PRESET_NONE, 0xFF);

    viewbox_seed_pressed_state(&viewbox_test_widget, 1, 1);
    egui_view_viewbox_set_read_only_mode(EGUI_VIEW_OF(&viewbox_test_widget), 1);
    viewbox_assert_widget_state(&viewbox_test_widget, initial_snapshot, initial_preset, initial_mode, 0, 1);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);
    EGUI_TEST_ASSERT_FALSE(viewbox_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, preset_x, preset_y));
    EGUI_TEST_ASSERT_FALSE(viewbox_send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(viewbox_send_key(EGUI_KEY_CODE_ENTER));
    viewbox_assert_widget_state(&viewbox_test_widget, initial_snapshot, initial_preset, initial_mode, 0, 1);
    viewbox_assert_action_state(0, 0xFF, EGUI_VIEW_VIEWBOX_PRESET_NONE, 0xFF);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);

    egui_view_viewbox_set_read_only_mode(EGUI_VIEW_OF(&viewbox_test_widget), 0);
    viewbox_assert_widget_state(&viewbox_test_widget, initial_snapshot, initial_preset, initial_mode, 0, 0);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_key(EGUI_KEY_CODE_RIGHT));
    viewbox_assert_widget_state(&viewbox_test_widget, initial_snapshot, 2, EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY, 0, 0);
    viewbox_assert_action_state(0, 0xFF, EGUI_VIEW_VIEWBOX_PRESET_NONE, 0xFF);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_key(EGUI_KEY_CODE_ENTER));
    viewbox_assert_widget_state(&viewbox_test_widget, initial_snapshot, 2, EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY, 0, 0);
    viewbox_assert_action_state(1, initial_snapshot, 2, EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY);
    viewbox_reset_action_state();

    egui_view_viewbox_set_current_snapshot(EGUI_VIEW_OF(&viewbox_test_widget), 1);
    egui_view_viewbox_set_current_preset(EGUI_VIEW_OF(&viewbox_test_widget), 1);
    initial_snapshot = egui_view_viewbox_get_current_snapshot(EGUI_VIEW_OF(&viewbox_test_widget));
    initial_preset = egui_view_viewbox_get_current_preset(EGUI_VIEW_OF(&viewbox_test_widget));
    initial_mode = egui_view_viewbox_get_stretch_mode(EGUI_VIEW_OF(&viewbox_test_widget));
    viewbox_assert_widget_state(&viewbox_test_widget, initial_snapshot, initial_preset, initial_mode, 0, 0);

    viewbox_seed_pressed_state(&viewbox_test_widget, 1, 1);
    egui_view_set_enable(EGUI_VIEW_OF(&viewbox_test_widget), 0);
    viewbox_assert_widget_state(&viewbox_test_widget, initial_snapshot, initial_preset, initial_mode, 0, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, viewbox_test_widget.pressed_preset);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&viewbox_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(viewbox_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, preset_x, preset_y));
    EGUI_TEST_ASSERT_FALSE(viewbox_send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(viewbox_send_key(EGUI_KEY_CODE_SPACE));
    viewbox_assert_widget_state(&viewbox_test_widget, initial_snapshot, initial_preset, initial_mode, 0, 0);
    viewbox_assert_action_state(0, 0xFF, EGUI_VIEW_VIEWBOX_PRESET_NONE, 0xFF);
    viewbox_assert_pressed_cleared(&viewbox_test_widget);

    egui_view_set_enable(EGUI_VIEW_OF(&viewbox_test_widget), 1);
    viewbox_assert_widget_state(&viewbox_test_widget, initial_snapshot, initial_preset, initial_mode, 0, 0);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_key(EGUI_KEY_CODE_RIGHT));
    viewbox_assert_widget_state(&viewbox_test_widget, initial_snapshot, 2, EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY, 0, 0);
    viewbox_assert_action_state(0, 0xFF, EGUI_VIEW_VIEWBOX_PRESET_NONE, 0xFF);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_key(EGUI_KEY_CODE_SPACE));
    viewbox_assert_widget_state(&viewbox_test_widget, initial_snapshot, 2, EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY, 0, 0);
    viewbox_assert_action_state(1, initial_snapshot, 2, EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY);
}

static void test_viewbox_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t preset_x;
    egui_dim_t preset_y;
    uint8_t initial_snapshot;
    uint8_t initial_preset;
    uint8_t initial_mode;

    viewbox_setup_preview_widget();
    viewbox_layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(viewbox_get_preset_center(EGUI_VIEW_OF(&viewbox_preview_widget), 0, &preset_x, &preset_y));

    initial_snapshot = egui_view_viewbox_get_current_snapshot(EGUI_VIEW_OF(&viewbox_preview_widget));
    initial_preset = egui_view_viewbox_get_current_preset(EGUI_VIEW_OF(&viewbox_preview_widget));
    initial_mode = egui_view_viewbox_get_stretch_mode(EGUI_VIEW_OF(&viewbox_preview_widget));
    viewbox_assert_widget_state(&viewbox_preview_widget, initial_snapshot, initial_preset, initial_mode, 1, 0);
    viewbox_assert_action_state(0, 0xFF, EGUI_VIEW_VIEWBOX_PRESET_NONE, 0xFF);

    viewbox_seed_pressed_state(&viewbox_preview_widget, 0, 1);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, preset_x, preset_y));
    viewbox_assert_widget_state(&viewbox_preview_widget, initial_snapshot, initial_preset, initial_mode, 1, 0);
    viewbox_assert_action_state(0, 0xFF, EGUI_VIEW_VIEWBOX_PRESET_NONE, 0xFF);
    viewbox_assert_pressed_cleared(&viewbox_preview_widget);

    viewbox_seed_pressed_state(&viewbox_preview_widget, 0, 1);
    EGUI_TEST_ASSERT_TRUE(viewbox_send_preview_key(EGUI_KEY_CODE_RIGHT));
    viewbox_assert_widget_state(&viewbox_preview_widget, initial_snapshot, initial_preset, initial_mode, 1, 0);
    viewbox_assert_action_state(0, 0xFF, EGUI_VIEW_VIEWBOX_PRESET_NONE, 0xFF);
    viewbox_assert_pressed_cleared(&viewbox_preview_widget);
}

void test_viewbox_run(void)
{
    EGUI_TEST_SUITE_BEGIN(viewbox);
    EGUI_TEST_RUN(test_viewbox_set_snapshots_clamp_and_defaults);
    EGUI_TEST_RUN(test_viewbox_setters_clear_pressed_and_update_state);
    EGUI_TEST_RUN(test_viewbox_regions_activate_and_listener);
    EGUI_TEST_RUN(test_viewbox_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_viewbox_key_navigation_activate_and_tab);
    EGUI_TEST_RUN(test_viewbox_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_viewbox_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
