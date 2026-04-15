#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_drop_down_button.h"

#include "../../HelloCustomWidgets/input/drop_down_button/egui_view_drop_down_button.h"
#include "../../HelloCustomWidgets/input/drop_down_button/egui_view_drop_down_button.c"

static egui_view_drop_down_button_t test_button;
static egui_view_drop_down_button_t preview_button;
static egui_view_api_t preview_api;
static int click_count;

typedef struct
{
    egui_region_t region_screen;
    const egui_view_drop_down_button_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_click_listener_t on_click_listener;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t danger_color;
    egui_color_t neutral_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    egui_alpha_t alpha;
} drop_down_button_preview_snapshot_t;

static const egui_view_drop_down_button_snapshot_t test_snapshots[] = {
        {"Standard", "SO", "Sort", "Sort by name", "v", EGUI_VIEW_DROP_DOWN_BUTTON_TONE_ACCENT, 1},
        {"Standard", "LY", "Layout", "Switch layout", "v", EGUI_VIEW_DROP_DOWN_BUTTON_TONE_NEUTRAL, 0},
        {"Standard", "TH", "Theme", "Choose theme", "v", EGUI_VIEW_DROP_DOWN_BUTTON_TONE_WARNING, 0},
        {"Standard", "FT", "Filter", "Open saved filters", "v", EGUI_VIEW_DROP_DOWN_BUTTON_TONE_SUCCESS, 0},
};

static const egui_view_drop_down_button_snapshot_t preview_snapshot = {
        "", "", "Quick", "", "", EGUI_VIEW_DROP_DOWN_BUTTON_TONE_ACCENT, 1};

static void on_button_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    click_count++;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_button(void)
{
    egui_view_drop_down_button_init(EGUI_VIEW_OF(&test_button));
    egui_view_set_size(EGUI_VIEW_OF(&test_button), 116, 76);
    egui_view_drop_down_button_set_snapshots(EGUI_VIEW_OF(&test_button), test_snapshots, EGUI_ARRAY_SIZE(test_snapshots));
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&test_button), on_button_click);
    click_count = 0;
}

static void layout_button(void)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = 116;
    region.size.height = 76;
    egui_view_layout(EGUI_VIEW_OF(&test_button), &region);
    egui_region_copy(&EGUI_VIEW_OF(&test_button)->region_screen, &region);
}

static void setup_preview_button(void)
{
    egui_view_drop_down_button_init(EGUI_VIEW_OF(&preview_button));
    egui_view_set_size(EGUI_VIEW_OF(&preview_button), 104, 44);
    egui_view_drop_down_button_set_snapshots(EGUI_VIEW_OF(&preview_button), &preview_snapshot, 1);
    egui_view_drop_down_button_set_compact_mode(EGUI_VIEW_OF(&preview_button), 1);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_button), on_button_click);
    egui_view_drop_down_button_override_static_preview_api(EGUI_VIEW_OF(&preview_button), &preview_api);
    click_count = 0;
}

static void layout_preview_button(void)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = 116;
    region.size.height = 44;
    egui_view_layout(EGUI_VIEW_OF(&preview_button), &region);
    egui_region_copy(&EGUI_VIEW_OF(&preview_button)->region_screen, &region);
}

static void capture_preview_snapshot(drop_down_button_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_button)->region_screen;
    snapshot->snapshots = preview_button.snapshots;
    snapshot->font = preview_button.font;
    snapshot->meta_font = preview_button.meta_font;
    snapshot->on_click_listener = EGUI_VIEW_OF(&preview_button)->on_click_listener;
    snapshot->surface_color = preview_button.surface_color;
    snapshot->border_color = preview_button.border_color;
    snapshot->text_color = preview_button.text_color;
    snapshot->muted_text_color = preview_button.muted_text_color;
    snapshot->accent_color = preview_button.accent_color;
    snapshot->success_color = preview_button.success_color;
    snapshot->warning_color = preview_button.warning_color;
    snapshot->danger_color = preview_button.danger_color;
    snapshot->neutral_color = preview_button.neutral_color;
    snapshot->snapshot_count = preview_button.snapshot_count;
    snapshot->current_snapshot = preview_button.current_snapshot;
    snapshot->compact_mode = preview_button.compact_mode;
    snapshot->read_only_mode = preview_button.read_only_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_button)->alpha;
}

static void assert_preview_state_unchanged(const drop_down_button_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_button)->region_screen);
    EGUI_TEST_ASSERT_TRUE(preview_button.snapshots == snapshot->snapshots);
    EGUI_TEST_ASSERT_TRUE(preview_button.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_button.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_button)->on_click_listener == snapshot->on_click_listener);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_button.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_button.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_button.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_button.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_button.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->success_color.full, preview_button.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_button.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->danger_color.full, preview_button.danger_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->neutral_color.full, preview_button.neutral_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->snapshot_count, preview_button.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_snapshot, preview_button.current_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_button.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_button.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_button)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_button)->is_pressed);
}

static int send_touch_at(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&test_button)->api->on_touch_event(EGUI_VIEW_OF(&test_button), &event);
}

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_touch(uint8_t type)
{
    return send_touch_at(type, 40, 40);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int send_key_event(uint8_t type, uint8_t key_code)
{
    return dispatch_key_event_to_view(EGUI_VIEW_OF(&test_button), type, key_code);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_button), key_code);
}

static int send_preview_touch(uint8_t type)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = 40;
    event.location.y = 40;
    return EGUI_VIEW_OF(&preview_button)->api->on_touch_event(EGUI_VIEW_OF(&preview_button), &event);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_button), key_code);
}

static void test_drop_down_button_snapshot_switching_clears_pressed_state(void)
{
    setup_button();
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_drop_down_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
    egui_view_drop_down_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_drop_down_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));

    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    egui_view_drop_down_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    egui_view_drop_down_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_drop_down_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    test_button.current_snapshot = 2;
    egui_view_drop_down_button_set_snapshots(EGUI_VIEW_OF(&test_button), test_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_drop_down_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
}

static void test_drop_down_button_setters_clear_pressed_state(void)
{
    setup_button();

    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    egui_view_drop_down_button_set_font(EGUI_VIEW_OF(&test_button), NULL);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    egui_view_drop_down_button_set_meta_font(EGUI_VIEW_OF(&test_button), NULL);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    egui_view_drop_down_button_set_palette(EGUI_VIEW_OF(&test_button), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD6DEE7), EGUI_COLOR_HEX(0x1D2630),
                                           EGUI_COLOR_HEX(0x6F7B89), EGUI_COLOR_HEX(0x2563EB), EGUI_COLOR_HEX(0x178454), EGUI_COLOR_HEX(0xB87A16),
                                           EGUI_COLOR_HEX(0xB13A35), EGUI_COLOR_HEX(0x7A8795));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
}

static void test_drop_down_button_touch_click_listener(void)
{
    setup_button();
    layout_button();
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
    EGUI_TEST_ASSERT_FALSE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
}

static void test_drop_down_button_keyboard_enter_click_listener(void)
{
    setup_button();
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_drop_down_button_same_target_release_requires_return_to_origin(void)
{
    egui_dim_t inside_x = 40;
    egui_dim_t inside_y = 40;
    egui_dim_t outside_x = 4;
    egui_dim_t outside_y = 4;

    setup_button();
    layout_button();

    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_touch_at(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_drop_down_button_touch_cancel_and_key_guard_clear_pressed_state(void)
{
    setup_button();
    layout_button();

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
}

static void test_drop_down_button_compact_mode_clears_pressed_and_keeps_click_behavior(void)
{
    setup_button();
    layout_button();
    egui_view_drop_down_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 2);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);

    egui_view_drop_down_button_set_compact_mode(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_button.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_drop_down_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_drop_down_button_read_only_clears_pressed_and_ignores_input(void)
{
    setup_button();
    layout_button();

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);

    egui_view_drop_down_button_set_read_only_mode(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_button.read_only_mode);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP));
    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    egui_view_drop_down_button_set_read_only_mode(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_button.read_only_mode);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_drop_down_button_disabled_ignores_input_and_clears_pressed_state(void)
{
    setup_button();
    layout_button();

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);

    egui_view_set_enable(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_drop_down_button_static_preview_consumes_input_and_keeps_state(void)
{
    drop_down_button_preview_snapshot_t initial_snapshot;

    setup_preview_button();
    layout_preview_button();
    capture_preview_snapshot(&initial_snapshot);

    EGUI_VIEW_OF(&preview_button)->is_pressed = true;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    assert_preview_state_unchanged(&initial_snapshot);

    EGUI_VIEW_OF(&preview_button)->is_pressed = true;
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_drop_down_button_run(void)
{
    EGUI_TEST_SUITE_BEGIN(drop_down_button);
    EGUI_TEST_RUN(test_drop_down_button_snapshot_switching_clears_pressed_state);
    EGUI_TEST_RUN(test_drop_down_button_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_drop_down_button_touch_click_listener);
    EGUI_TEST_RUN(test_drop_down_button_keyboard_enter_click_listener);
    EGUI_TEST_RUN(test_drop_down_button_same_target_release_requires_return_to_origin);
    EGUI_TEST_RUN(test_drop_down_button_touch_cancel_and_key_guard_clear_pressed_state);
    EGUI_TEST_RUN(test_drop_down_button_compact_mode_clears_pressed_and_keeps_click_behavior);
    EGUI_TEST_RUN(test_drop_down_button_read_only_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_drop_down_button_disabled_ignores_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_drop_down_button_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
