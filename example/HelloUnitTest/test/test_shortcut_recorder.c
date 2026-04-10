#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_shortcut_recorder.h"

#include "../../HelloCustomWidgets/input/shortcut_recorder/egui_view_shortcut_recorder.h"
#include "../../HelloCustomWidgets/input/shortcut_recorder/egui_view_shortcut_recorder.c"

static egui_view_shortcut_recorder_t test_recorder;
static egui_view_shortcut_recorder_t preview_recorder;
static egui_view_api_t preview_api;
static uint8_t changed_count;
static uint8_t last_part;
static uint8_t last_preset;

static const egui_view_shortcut_recorder_preset_t test_presets[] = {
        {"Search files", "Workspace", EGUI_KEY_CODE_F, 1, 1},
        {"Command bar", "Quick command", EGUI_KEY_CODE_P, 1, 1},
        {"Pin focus", "One tap", EGUI_KEY_CODE_1, 0, 1},
};

static void on_changed(egui_view_t *self, uint8_t part, uint8_t preset_index)
{
    EGUI_UNUSED(self);
    changed_count++;
    last_part = part;
    last_preset = preset_index;
}

static void reset_changed_state(void)
{
    changed_count = 0;
    last_part = EGUI_VIEW_SHORTCUT_RECORDER_PART_NONE;
    last_preset = 0;
}

static void setup_recorder(void)
{
    egui_view_shortcut_recorder_init(EGUI_VIEW_OF(&test_recorder));
    egui_view_set_size(EGUI_VIEW_OF(&test_recorder), 160, 96);
    egui_view_shortcut_recorder_set_presets(EGUI_VIEW_OF(&test_recorder), test_presets, EGUI_ARRAY_SIZE(test_presets));
    egui_view_shortcut_recorder_set_binding(EGUI_VIEW_OF(&test_recorder), EGUI_KEY_CODE_K, 0, 1);
    egui_view_shortcut_recorder_set_on_changed_listener(EGUI_VIEW_OF(&test_recorder), on_changed);
    reset_changed_state();
}

static void setup_preview_recorder(void)
{
    egui_view_shortcut_recorder_init(EGUI_VIEW_OF(&preview_recorder));
    egui_view_set_size(EGUI_VIEW_OF(&preview_recorder), 106, 82);
    egui_view_shortcut_recorder_set_presets(EGUI_VIEW_OF(&preview_recorder), test_presets, EGUI_ARRAY_SIZE(test_presets));
    egui_view_shortcut_recorder_set_binding(EGUI_VIEW_OF(&preview_recorder), EGUI_KEY_CODE_P, 1, 1);
    egui_view_shortcut_recorder_set_current_part(EGUI_VIEW_OF(&preview_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD);
    egui_view_shortcut_recorder_set_current_preset(EGUI_VIEW_OF(&preview_recorder), 0);
    egui_view_shortcut_recorder_set_compact_mode(EGUI_VIEW_OF(&preview_recorder), 1);
    egui_view_shortcut_recorder_override_static_preview_api(EGUI_VIEW_OF(&preview_recorder), &preview_api);
}

static void layout_view(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static void layout_recorder(void)
{
    layout_view(EGUI_VIEW_OF(&test_recorder), 10, 20, 160, 96);
}

static void layout_preview_recorder(void)
{
    layout_view(EGUI_VIEW_OF(&preview_recorder), 12, 18, 106, 82);
}

static void get_part_center(egui_view_t *view, uint8_t part, uint8_t preset_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    EGUI_TEST_ASSERT_TRUE(egui_view_shortcut_recorder_get_part_region(view, part, preset_index, &region));
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
}

static int send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t type, uint8_t key_code, uint8_t is_shift, uint8_t is_ctrl)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    event.is_shift = is_shift ? 1 : 0;
    event.is_ctrl = is_ctrl ? 1 : 0;
    return view->api->on_key_event(view, &event);
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_recorder), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_recorder), type, x, y);
}

static int send_key(uint8_t key_code, uint8_t is_shift, uint8_t is_ctrl)
{
    int handled = 0;

    handled |= send_key_to_view(EGUI_VIEW_OF(&test_recorder), EGUI_KEY_EVENT_ACTION_DOWN, key_code, is_shift, is_ctrl);
    handled |= send_key_to_view(EGUI_VIEW_OF(&test_recorder), EGUI_KEY_EVENT_ACTION_UP, key_code, is_shift, is_ctrl);
    return handled;
}

static int send_preview_key(uint8_t type, uint8_t key_code, uint8_t is_shift, uint8_t is_ctrl)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_recorder), type, key_code, is_shift, is_ctrl);
}

static void seed_pressed_state(uint8_t part, uint8_t preset_index)
{
    EGUI_VIEW_OF(&test_recorder)->is_pressed = 1;
    test_recorder.pressed_part = part;
    test_recorder.pressed_preset = preset_index;
}

static void assert_pressed_cleared(void)
{
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_recorder)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SHORTCUT_RECORDER_PART_NONE, test_recorder.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_recorder.pressed_preset);
}

static void test_shortcut_recorder_enter_listening_and_capture_binding(void)
{
    setup_recorder();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER, 0, 0));
    EGUI_TEST_ASSERT_TRUE(egui_view_shortcut_recorder_is_listening(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_P, 1, 1));
    EGUI_TEST_ASSERT_FALSE(egui_view_shortcut_recorder_is_listening(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_TRUE(egui_view_shortcut_recorder_has_binding(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_KEY_CODE_P, egui_view_shortcut_recorder_get_key_code(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_shortcut_recorder_get_is_shift(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_shortcut_recorder_get_is_ctrl(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_TRUE(changed_count >= 2);
}

static void test_shortcut_recorder_tab_and_apply_preset(void)
{
    setup_recorder();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB, 0, 0));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, egui_view_shortcut_recorder_get_current_part(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN, 0, 0));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_shortcut_recorder_get_current_preset(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE, 0, 0));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_KEY_CODE_P, egui_view_shortcut_recorder_get_key_code(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_shortcut_recorder_get_is_shift(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_shortcut_recorder_get_is_ctrl(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, last_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_preset);
}

static void test_shortcut_recorder_touch_preset_and_clear_binding(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_recorder();
    layout_recorder();

    get_part_center(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, 2, &x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_KEY_CODE_1, egui_view_shortcut_recorder_get_key_code(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_shortcut_recorder_get_is_ctrl(EGUI_VIEW_OF(&test_recorder)));

    get_part_center(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_CLEAR, 0, &x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_FALSE(egui_view_shortcut_recorder_has_binding(EGUI_VIEW_OF(&test_recorder)));
}

static void test_shortcut_recorder_escape_cancels_listening(void)
{
    setup_recorder();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER, 0, 0));
    EGUI_TEST_ASSERT_TRUE(egui_view_shortcut_recorder_is_listening(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ESCAPE, 0, 0));
    EGUI_TEST_ASSERT_FALSE(egui_view_shortcut_recorder_is_listening(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_KEY_CODE_K, egui_view_shortcut_recorder_get_key_code(EGUI_VIEW_OF(&test_recorder)));
}

static void test_shortcut_recorder_setters_clear_pressed_state(void)
{
    setup_recorder();

    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, 1);
    egui_view_shortcut_recorder_set_header(EGUI_VIEW_OF(&test_recorder), "Quick launch", "Capture a shortcut", "Ready");
    assert_pressed_cleared();

    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, 1);
    egui_view_shortcut_recorder_set_palette(EGUI_VIEW_OF(&test_recorder), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD9E0E7), EGUI_COLOR_HEX(0x1F2933),
                                            EGUI_COLOR_HEX(0x708090), EGUI_COLOR_HEX(0x2563EB), EGUI_COLOR_HEX(0xD97706), EGUI_COLOR_HEX(0x0F766E),
                                            EGUI_COLOR_HEX(0xBE5168));
    assert_pressed_cleared();

    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, 2);
    egui_view_shortcut_recorder_set_presets(EGUI_VIEW_OF(&test_recorder), test_presets, 2);
    assert_pressed_cleared();

    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0);
    egui_view_shortcut_recorder_set_binding(EGUI_VIEW_OF(&test_recorder), EGUI_KEY_CODE_F, 1, 1);
    assert_pressed_cleared();

    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_CLEAR, 0);
    egui_view_shortcut_recorder_clear_binding(EGUI_VIEW_OF(&test_recorder));
    assert_pressed_cleared();

    egui_view_shortcut_recorder_set_binding(EGUI_VIEW_OF(&test_recorder), EGUI_KEY_CODE_K, 0, 1);
    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0);
    egui_view_shortcut_recorder_set_listening(EGUI_VIEW_OF(&test_recorder), 1);
    assert_pressed_cleared();

    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, 1);
    egui_view_shortcut_recorder_apply_preset(EGUI_VIEW_OF(&test_recorder), 1);
    assert_pressed_cleared();

    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, 1);
    egui_view_shortcut_recorder_set_current_part(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD);
    assert_pressed_cleared();

    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, 1);
    egui_view_shortcut_recorder_set_current_preset(EGUI_VIEW_OF(&test_recorder), 1);
    assert_pressed_cleared();

    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, 1);
    egui_view_shortcut_recorder_set_compact_mode(EGUI_VIEW_OF(&test_recorder), 1);
    assert_pressed_cleared();

    setup_recorder();
    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, 1);
    egui_view_shortcut_recorder_set_read_only_mode(EGUI_VIEW_OF(&test_recorder), 1);
    assert_pressed_cleared();
}

static void test_shortcut_recorder_touch_release_requires_same_target(void)
{
    egui_dim_t field_x;
    egui_dim_t field_y;
    egui_dim_t preset_x;
    egui_dim_t preset_y;
    egui_dim_t clear_x;
    egui_dim_t clear_y;

    setup_recorder();
    layout_recorder();
    get_part_center(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0, &field_x, &field_y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, field_x, field_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, field_x + 200, field_y + 120));
    EGUI_TEST_ASSERT_FALSE(egui_view_shortcut_recorder_is_listening(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    assert_pressed_cleared();

    setup_recorder();
    layout_recorder();
    get_part_center(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, 2, &preset_x, &preset_y);
    get_part_center(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0, &field_x, &field_y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, preset_x, preset_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, field_x, field_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_KEY_CODE_K, egui_view_shortcut_recorder_get_key_code(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    assert_pressed_cleared();

    setup_recorder();
    layout_recorder();
    get_part_center(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_CLEAR, 0, &clear_x, &clear_y);
    get_part_center(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0, &field_x, &field_y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, clear_x, clear_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, field_x, field_y));
    EGUI_TEST_ASSERT_TRUE(egui_view_shortcut_recorder_has_binding(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    assert_pressed_cleared();
}

static void test_shortcut_recorder_touch_cancel_clears_pressed_state_without_notify(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_recorder();
    layout_recorder();
    get_part_center(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, 1, &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_recorder)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, test_recorder.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_recorder.pressed_preset);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, x, y));
    assert_pressed_cleared();
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_KEY_CODE_K, egui_view_shortcut_recorder_get_key_code(EGUI_VIEW_OF(&test_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

static void test_shortcut_recorder_compact_read_only_and_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_recorder();
    egui_view_shortcut_recorder_set_compact_mode(EGUI_VIEW_OF(&test_recorder), 1);
    layout_recorder();
    get_part_center(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0, &x, &y);
    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER, 0, 0));
    assert_pressed_cleared();
    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared();

    setup_recorder();
    egui_view_shortcut_recorder_set_read_only_mode(EGUI_VIEW_OF(&test_recorder), 1);
    layout_recorder();
    get_part_center(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0, &x, &y);
    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_TAB, 0, 0));
    assert_pressed_cleared();
    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared();

    setup_recorder();
    layout_recorder();
    egui_view_set_enable(EGUI_VIEW_OF(&test_recorder), 0);
    get_part_center(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0, &x, &y);
    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER, 0, 0));
    assert_pressed_cleared();
    seed_pressed_state(EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared();
}

static void test_shortcut_recorder_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_recorder();
    layout_preview_recorder();
    get_part_center(EGUI_VIEW_OF(&preview_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0, &x, &y);

    EGUI_VIEW_OF(&preview_recorder)->is_pressed = 1;
    preview_recorder.pressed_part = EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD;
    preview_recorder.pressed_preset = 0;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_recorder)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SHORTCUT_RECORDER_PART_NONE, preview_recorder.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_recorder.pressed_preset);
    EGUI_TEST_ASSERT_FALSE(egui_view_shortcut_recorder_is_listening(EGUI_VIEW_OF(&preview_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_KEY_CODE_P, egui_view_shortcut_recorder_get_key_code(EGUI_VIEW_OF(&preview_recorder)));

    egui_view_shortcut_recorder_set_current_part(EGUI_VIEW_OF(&preview_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET);
    egui_view_shortcut_recorder_set_current_preset(EGUI_VIEW_OF(&preview_recorder), 0);
    EGUI_VIEW_OF(&preview_recorder)->is_pressed = 1;
    preview_recorder.pressed_part = EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET;
    preview_recorder.pressed_preset = 0;
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_TAB, 0, 0));
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_TAB, 0, 0));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_recorder)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SHORTCUT_RECORDER_PART_NONE, preview_recorder.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_recorder.pressed_preset);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, egui_view_shortcut_recorder_get_current_part(EGUI_VIEW_OF(&preview_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_shortcut_recorder_get_current_preset(EGUI_VIEW_OF(&preview_recorder)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_KEY_CODE_P, egui_view_shortcut_recorder_get_key_code(EGUI_VIEW_OF(&preview_recorder)));
    EGUI_TEST_ASSERT_FALSE(egui_view_shortcut_recorder_is_listening(EGUI_VIEW_OF(&preview_recorder)));
}

static void test_shortcut_recorder_regions_follow_state(void)
{
    egui_region_t region;

    setup_recorder();
    layout_recorder();
    EGUI_TEST_ASSERT_TRUE(egui_view_shortcut_recorder_get_part_region(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0, &region));
    EGUI_TEST_ASSERT_TRUE(region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(egui_view_shortcut_recorder_get_part_region(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, 0, &region));
    EGUI_TEST_ASSERT_TRUE(egui_view_shortcut_recorder_get_part_region(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_CLEAR, 0, &region));

    egui_view_shortcut_recorder_clear_binding(EGUI_VIEW_OF(&test_recorder));
    EGUI_TEST_ASSERT_FALSE(egui_view_shortcut_recorder_get_part_region(EGUI_VIEW_OF(&test_recorder), EGUI_VIEW_SHORTCUT_RECORDER_PART_CLEAR, 0, &region));
}

void test_shortcut_recorder_run(void)
{
    EGUI_TEST_SUITE_BEGIN(shortcut_recorder);
    EGUI_TEST_RUN(test_shortcut_recorder_enter_listening_and_capture_binding);
    EGUI_TEST_RUN(test_shortcut_recorder_tab_and_apply_preset);
    EGUI_TEST_RUN(test_shortcut_recorder_touch_preset_and_clear_binding);
    EGUI_TEST_RUN(test_shortcut_recorder_escape_cancels_listening);
    EGUI_TEST_RUN(test_shortcut_recorder_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_shortcut_recorder_touch_release_requires_same_target);
    EGUI_TEST_RUN(test_shortcut_recorder_touch_cancel_clears_pressed_state_without_notify);
    EGUI_TEST_RUN(test_shortcut_recorder_compact_read_only_and_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_shortcut_recorder_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_shortcut_recorder_regions_follow_state);
    EGUI_TEST_SUITE_END();
}
