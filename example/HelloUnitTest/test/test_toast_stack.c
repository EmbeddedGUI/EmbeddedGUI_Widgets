#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_toast_stack.h"

#include "../../HelloCustomWidgets/feedback/toast_stack/egui_view_toast_stack.h"
#include "../../HelloCustomWidgets/feedback/toast_stack/egui_view_toast_stack.c"

static egui_view_toast_stack_t test_stack;
static egui_view_toast_stack_t preview_stack;
static egui_view_api_t preview_api;
static int click_count;

static const egui_view_toast_stack_snapshot_t g_snapshots[] = {
        {"Backup ready", "Open the latest note.", "Open", "Now", "Shift starts", "Daily sync", 0, 1},
        {"Draft", "Team can review the build.", "Review", "1 min", "Queue", "Pinned", 1, 1},
        {"Storage low", "Archive logs before sync.", "Manage", "4 min", "Sync waiting", "Quota alert", 2, 1},
        {"Upload failed", "Reconnect to send report.", "Retry", "Offline", "Auth expired", "Queue paused", 3, 0},
};

static const egui_view_toast_stack_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", "A", "A", "A", "A", 0, 1},
        {"B", "B", "B", "B", "B", "B", 1, 1},
        {"C", "C", "C", "C", "C", "C", 2, 1},
        {"D", "D", "D", "D", "D", "D", 3, 1},
        {"E", "E", "E", "E", "E", "E", 0, 0},
        {"F", "F", "F", "F", "F", "F", 1, 0},
};

static void on_stack_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    click_count++;
}

static void reset_click_count(void)
{
    click_count = 0;
}

static void setup_stack(void)
{
    egui_view_toast_stack_init(EGUI_VIEW_OF(&test_stack));
    egui_view_set_size(EGUI_VIEW_OF(&test_stack), 196, 108);
    egui_view_toast_stack_set_snapshots(EGUI_VIEW_OF(&test_stack), g_snapshots, 4);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&test_stack), on_stack_click);
    reset_click_count();
}

static void setup_preview_stack(void)
{
    egui_view_toast_stack_init(EGUI_VIEW_OF(&preview_stack));
    egui_view_set_size(EGUI_VIEW_OF(&preview_stack), 104, 83);
    egui_view_toast_stack_set_snapshots(EGUI_VIEW_OF(&preview_stack), g_snapshots, 4);
    egui_view_toast_stack_set_current_snapshot(EGUI_VIEW_OF(&preview_stack), 1);
    egui_view_toast_stack_set_compact_mode(EGUI_VIEW_OF(&preview_stack), 1);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_stack), on_stack_click);
    egui_view_toast_stack_override_static_preview_api(EGUI_VIEW_OF(&preview_stack), &preview_api);
    reset_click_count();
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

static void layout_stack(void)
{
    layout_view(EGUI_VIEW_OF(&test_stack), 10, 20, 196, 108);
}

static void layout_preview_stack(void)
{
    layout_view(EGUI_VIEW_OF(&preview_stack), 12, 18, 104, 83);
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

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= view->api->on_key_event(view, &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= view->api->on_key_event(view, &event);
    return handled;
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_stack), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_stack), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_stack), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_stack), key_code);
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void get_view_outside_point(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x - 6;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void seed_pressed_state(egui_view_t *view, uint8_t visual_pressed)
{
    egui_view_set_pressed(view, visual_pressed ? true : false);
}

static void assert_pressed_cleared(egui_view_t *view)
{
    EGUI_TEST_ASSERT_FALSE(view->is_pressed);
}

static void test_toast_stack_set_snapshots_clamps_and_clears_pressed_state(void)
{
    setup_stack();

    test_stack.current_snapshot = EGUI_VIEW_TOAST_STACK_MAX_SNAPSHOTS;
    seed_pressed_state(EGUI_VIEW_OF(&test_stack), 1);
    egui_view_toast_stack_set_snapshots(EGUI_VIEW_OF(&test_stack), g_overflow_snapshots, 6);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOAST_STACK_MAX_SNAPSHOTS, test_stack.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_toast_stack_get_current_snapshot(EGUI_VIEW_OF(&test_stack)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));

    test_stack.current_snapshot = 2;
    seed_pressed_state(EGUI_VIEW_OF(&test_stack), 1);
    egui_view_toast_stack_set_snapshots(EGUI_VIEW_OF(&test_stack), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_stack.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_toast_stack_get_current_snapshot(EGUI_VIEW_OF(&test_stack)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));
}

static void test_toast_stack_snapshot_and_setters_clear_pressed_state(void)
{
    egui_color_t surface = EGUI_COLOR_HEX(0x101112);
    egui_color_t border = EGUI_COLOR_HEX(0x202122);
    egui_color_t text = EGUI_COLOR_HEX(0x303132);
    egui_color_t muted = EGUI_COLOR_HEX(0x404142);
    egui_color_t accent = EGUI_COLOR_HEX(0x505152);
    egui_color_t info = EGUI_COLOR_HEX(0x606162);
    egui_color_t success = EGUI_COLOR_HEX(0x707172);
    egui_color_t warning = EGUI_COLOR_HEX(0x808182);
    egui_color_t error = EGUI_COLOR_HEX(0x909192);

    setup_stack();

    egui_view_toast_stack_set_current_snapshot(EGUI_VIEW_OF(&test_stack), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_toast_stack_get_current_snapshot(EGUI_VIEW_OF(&test_stack)));

    seed_pressed_state(EGUI_VIEW_OF(&test_stack), 1);
    egui_view_toast_stack_set_current_snapshot(EGUI_VIEW_OF(&test_stack), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_toast_stack_get_current_snapshot(EGUI_VIEW_OF(&test_stack)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));

    seed_pressed_state(EGUI_VIEW_OF(&test_stack), 1);
    egui_view_toast_stack_set_current_snapshot(EGUI_VIEW_OF(&test_stack), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_toast_stack_get_current_snapshot(EGUI_VIEW_OF(&test_stack)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));

    seed_pressed_state(EGUI_VIEW_OF(&test_stack), 1);
    egui_view_toast_stack_set_font(EGUI_VIEW_OF(&test_stack), NULL);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));
    EGUI_TEST_ASSERT_TRUE(test_stack.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(EGUI_VIEW_OF(&test_stack), 1);
    egui_view_toast_stack_set_meta_font(EGUI_VIEW_OF(&test_stack), NULL);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));
    EGUI_TEST_ASSERT_TRUE(test_stack.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(EGUI_VIEW_OF(&test_stack), 1);
    egui_view_toast_stack_set_palette(EGUI_VIEW_OF(&test_stack), surface, border, text, muted, accent, info, success, warning, error);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));
    EGUI_TEST_ASSERT_EQUAL_INT(surface.full, test_stack.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(border.full, test_stack.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(text.full, test_stack.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(muted.full, test_stack.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(accent.full, test_stack.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(info.full, test_stack.info_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(success.full, test_stack.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(warning.full, test_stack.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(error.full, test_stack.error_color.full);

    seed_pressed_state(EGUI_VIEW_OF(&test_stack), 1);
    egui_view_toast_stack_set_compact_mode(EGUI_VIEW_OF(&test_stack), 2);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_stack.compact_mode);

    seed_pressed_state(EGUI_VIEW_OF(&test_stack), 1);
    egui_view_toast_stack_set_read_only_mode(EGUI_VIEW_OF(&test_stack), 3);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_stack.read_only_mode);
}

static void test_toast_stack_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_stack();
    layout_stack();
    get_view_center(EGUI_VIEW_OF(&test_stack), &inside_x, &inside_y);
    get_view_outside_point(EGUI_VIEW_OF(&test_stack), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_stack)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_stack)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_stack)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_stack)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));
}

static void test_toast_stack_keyboard_click_listener(void)
{
    setup_stack();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_toast_stack_compact_mode_clears_pressed_and_keeps_click_behavior(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_stack();
    layout_stack();
    get_view_center(EGUI_VIEW_OF(&test_stack), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_stack)->is_pressed);

    egui_view_toast_stack_set_compact_mode(EGUI_VIEW_OF(&test_stack), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_stack.compact_mode);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_toast_stack_read_only_and_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_stack();
    layout_stack();
    get_view_center(EGUI_VIEW_OF(&test_stack), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_stack)->is_pressed);

    egui_view_toast_stack_set_read_only_mode(EGUI_VIEW_OF(&test_stack), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_stack.read_only_mode);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));

    seed_pressed_state(EGUI_VIEW_OF(&test_stack), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));

    seed_pressed_state(EGUI_VIEW_OF(&test_stack), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    egui_view_toast_stack_set_read_only_mode(EGUI_VIEW_OF(&test_stack), 0);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);

    setup_stack();
    layout_stack();
    get_view_center(EGUI_VIEW_OF(&test_stack), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_stack)->is_pressed);

    egui_view_set_enable(EGUI_VIEW_OF(&test_stack), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));

    seed_pressed_state(EGUI_VIEW_OF(&test_stack), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_stack));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_stack), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_toast_stack_static_preview_consumes_input_and_keeps_snapshot(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_stack();
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_toast_stack_get_current_snapshot(EGUI_VIEW_OF(&preview_stack)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_stack.compact_mode);
    layout_preview_stack();
    get_view_center(EGUI_VIEW_OF(&preview_stack), &x, &y);

    seed_pressed_state(EGUI_VIEW_OF(&preview_stack), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_stack));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_toast_stack_get_current_snapshot(EGUI_VIEW_OF(&preview_stack)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    seed_pressed_state(EGUI_VIEW_OF(&preview_stack), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_stack));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_toast_stack_get_current_snapshot(EGUI_VIEW_OF(&preview_stack)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
}

static void test_toast_stack_internal_helpers_cover_severity_and_text(void)
{
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);
    egui_color_t mixed = egui_view_toast_stack_mix_disabled(sample);

    setup_stack();
    egui_view_toast_stack_set_palette(EGUI_VIEW_OF(&test_stack), EGUI_COLOR_HEX(0x111111), EGUI_COLOR_HEX(0x222222), EGUI_COLOR_HEX(0x333333),
                                      EGUI_COLOR_HEX(0x444444), EGUI_COLOR_HEX(0x555555), EGUI_COLOR_HEX(0x666666), EGUI_COLOR_HEX(0x777777),
                                      EGUI_COLOR_HEX(0x888888), EGUI_COLOR_HEX(0x999999));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOAST_STACK_MAX_SNAPSHOTS, egui_view_toast_stack_clamp_snapshot_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_toast_stack_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_toast_stack_text_len("Retry"));
    EGUI_TEST_ASSERT_TRUE(strcmp("i", egui_view_toast_stack_severity_glyph(0)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("+", egui_view_toast_stack_severity_glyph(1)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("!", egui_view_toast_stack_severity_glyph(2)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("x", egui_view_toast_stack_severity_glyph(3)) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full, egui_view_toast_stack_severity_color(&test_stack, 0).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x777777).full, egui_view_toast_stack_severity_color(&test_stack, 1).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x888888).full, egui_view_toast_stack_severity_color(&test_stack, 2).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x999999).full, egui_view_toast_stack_severity_color(&test_stack, 3).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full, egui_view_toast_stack_severity_color(&test_stack, 9).full);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 68).full, mixed.full);
}

void test_toast_stack_run(void)
{
    EGUI_TEST_SUITE_BEGIN(toast_stack);
    EGUI_TEST_RUN(test_toast_stack_set_snapshots_clamps_and_clears_pressed_state);
    EGUI_TEST_RUN(test_toast_stack_snapshot_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_toast_stack_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_toast_stack_keyboard_click_listener);
    EGUI_TEST_RUN(test_toast_stack_compact_mode_clears_pressed_and_keeps_click_behavior);
    EGUI_TEST_RUN(test_toast_stack_read_only_and_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_toast_stack_static_preview_consumes_input_and_keeps_snapshot);
    EGUI_TEST_RUN(test_toast_stack_internal_helpers_cover_severity_and_text);
    EGUI_TEST_SUITE_END();
}
