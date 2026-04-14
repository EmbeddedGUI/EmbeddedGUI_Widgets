#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_message_bar.h"

#include "../../HelloCustomWidgets/feedback/message_bar/egui_view_message_bar.h"
#include "../../HelloCustomWidgets/feedback/message_bar/egui_view_message_bar.c"

static egui_view_message_bar_t test_bar;
static egui_view_message_bar_t preview_bar;
static egui_view_api_t preview_api;
static int click_count;

static const egui_view_message_bar_snapshot_t g_snapshots[] = {
        {"Updates ready", "Open latest release notes.", "View notes", 0, 1, 1},
        {"Settings saved", "New defaults are active.", "Open panel", 1, 1, 1},
        {"Storage almost full", "Clear logs before next sync.", "View logs", 2, 1, 1},
        {"Connection lost", "Uploads pause. Link is down.", "Retry now", 3, 1, 1},
};

static const egui_view_message_bar_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", "A", 0, 1, 1},
        {"B", "B", "B", 1, 1, 1},
        {"C", "C", "C", 2, 1, 1},
        {"D", "D", "D", 3, 1, 1},
        {"E", "E", "E", 0, 0, 0},
        {"F", "F", "F", 1, 0, 0},
        {"G", "G", "G", 2, 0, 0},
};

static void on_bar_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    click_count++;
}

static void reset_click_count(void)
{
    click_count = 0;
}

static void setup_bar(void)
{
    egui_view_message_bar_init(EGUI_VIEW_OF(&test_bar));
    egui_view_set_size(EGUI_VIEW_OF(&test_bar), 196, 96);
    egui_view_message_bar_set_snapshots(EGUI_VIEW_OF(&test_bar), g_snapshots, 4);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&test_bar), on_bar_click);
    reset_click_count();
}

static void setup_preview_bar(void)
{
    egui_view_message_bar_init(EGUI_VIEW_OF(&preview_bar));
    egui_view_set_size(EGUI_VIEW_OF(&preview_bar), 104, 82);
    egui_view_message_bar_set_snapshots(EGUI_VIEW_OF(&preview_bar), g_snapshots, 4);
    egui_view_message_bar_set_current_snapshot(EGUI_VIEW_OF(&preview_bar), 0);
    egui_view_message_bar_set_compact_mode(EGUI_VIEW_OF(&preview_bar), 1);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_bar), on_bar_click);
    egui_view_message_bar_override_static_preview_api(EGUI_VIEW_OF(&preview_bar), &preview_api);
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

static void layout_bar(void)
{
    layout_view(EGUI_VIEW_OF(&test_bar), 10, 20, 196, 96);
}

static void layout_preview_bar(void)
{
    layout_view(EGUI_VIEW_OF(&preview_bar), 12, 18, 104, 82);
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
    return send_touch_to_view(EGUI_VIEW_OF(&test_bar), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_bar), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_bar), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_bar), key_code);
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

static void test_message_bar_set_snapshots_clamps_and_clears_pressed_state(void)
{
    setup_bar();

    test_bar.current_snapshot = EGUI_VIEW_MESSAGE_BAR_MAX_SNAPSHOTS;
    seed_pressed_state(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_message_bar_set_snapshots(EGUI_VIEW_OF(&test_bar), g_overflow_snapshots, 7);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MESSAGE_BAR_MAX_SNAPSHOTS, test_bar.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_message_bar_get_current_snapshot(EGUI_VIEW_OF(&test_bar)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));

    test_bar.current_snapshot = 2;
    seed_pressed_state(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_message_bar_set_snapshots(EGUI_VIEW_OF(&test_bar), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_message_bar_get_current_snapshot(EGUI_VIEW_OF(&test_bar)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));
}

static void test_message_bar_snapshot_and_setters_clear_pressed_state(void)
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

    setup_bar();

    egui_view_message_bar_set_current_snapshot(EGUI_VIEW_OF(&test_bar), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_message_bar_get_current_snapshot(EGUI_VIEW_OF(&test_bar)));

    seed_pressed_state(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_message_bar_set_current_snapshot(EGUI_VIEW_OF(&test_bar), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_message_bar_get_current_snapshot(EGUI_VIEW_OF(&test_bar)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));

    seed_pressed_state(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_message_bar_set_current_snapshot(EGUI_VIEW_OF(&test_bar), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_message_bar_get_current_snapshot(EGUI_VIEW_OF(&test_bar)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));

    seed_pressed_state(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_message_bar_set_font(EGUI_VIEW_OF(&test_bar), NULL);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));
    EGUI_TEST_ASSERT_TRUE(test_bar.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_message_bar_set_palette(EGUI_VIEW_OF(&test_bar), surface, border, text, muted, accent, info, success, warning, error);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));
    EGUI_TEST_ASSERT_EQUAL_INT(surface.full, test_bar.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(border.full, test_bar.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(text.full, test_bar.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(muted.full, test_bar.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(accent.full, test_bar.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(info.full, test_bar.info_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(success.full, test_bar.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(warning.full, test_bar.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(error.full, test_bar.error_color.full);

    seed_pressed_state(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_message_bar_set_compact_mode(EGUI_VIEW_OF(&test_bar), 2);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_bar.compact_mode);

    seed_pressed_state(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_message_bar_set_read_only_mode(EGUI_VIEW_OF(&test_bar), 3);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_bar.read_only_mode);
}

static void test_message_bar_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_bar();
    layout_bar();
    get_view_center(EGUI_VIEW_OF(&test_bar), &inside_x, &inside_y);
    get_view_outside_point(EGUI_VIEW_OF(&test_bar), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));
}

static void test_message_bar_keyboard_click_listener(void)
{
    setup_bar();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_message_bar_compact_mode_clears_pressed_and_keeps_click_behavior(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_bar();
    layout_bar();
    get_view_center(EGUI_VIEW_OF(&test_bar), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    egui_view_message_bar_set_compact_mode(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_bar.compact_mode);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_message_bar_read_only_and_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_bar();
    layout_bar();
    get_view_center(EGUI_VIEW_OF(&test_bar), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    egui_view_message_bar_set_read_only_mode(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_bar.read_only_mode);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));

    seed_pressed_state(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));

    seed_pressed_state(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    egui_view_message_bar_set_read_only_mode(EGUI_VIEW_OF(&test_bar), 0);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);

    setup_bar();
    layout_bar();
    get_view_center(EGUI_VIEW_OF(&test_bar), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    egui_view_set_enable(EGUI_VIEW_OF(&test_bar), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));

    seed_pressed_state(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_bar));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_message_bar_static_preview_consumes_input_and_keeps_snapshot(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_bar();
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_message_bar_get_current_snapshot(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_bar.compact_mode);
    layout_preview_bar();
    get_view_center(EGUI_VIEW_OF(&preview_bar), &x, &y);

    seed_pressed_state(EGUI_VIEW_OF(&preview_bar), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_bar));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_message_bar_get_current_snapshot(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    seed_pressed_state(EGUI_VIEW_OF(&preview_bar), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_bar));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_message_bar_get_current_snapshot(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
}

static void test_message_bar_internal_helpers_cover_severity_and_text(void)
{
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);
    egui_color_t mixed = egui_view_message_bar_mix_disabled(sample);

    setup_bar();
    egui_view_message_bar_set_palette(EGUI_VIEW_OF(&test_bar), EGUI_COLOR_HEX(0x111111), EGUI_COLOR_HEX(0x222222), EGUI_COLOR_HEX(0x333333),
                                      EGUI_COLOR_HEX(0x444444), EGUI_COLOR_HEX(0x555555), EGUI_COLOR_HEX(0x666666), EGUI_COLOR_HEX(0x777777),
                                      EGUI_COLOR_HEX(0x888888), EGUI_COLOR_HEX(0x999999));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MESSAGE_BAR_MAX_SNAPSHOTS, egui_view_message_bar_clamp_snapshot_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_message_bar_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_message_bar_text_len("Retry"));
    EGUI_TEST_ASSERT_TRUE(strcmp("i", egui_view_message_bar_severity_glyph(0)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("+", egui_view_message_bar_severity_glyph(1)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("!", egui_view_message_bar_severity_glyph(2)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("x", egui_view_message_bar_severity_glyph(3)) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full, egui_view_message_bar_severity_color(&test_bar, 0).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x777777).full, egui_view_message_bar_severity_color(&test_bar, 1).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x888888).full, egui_view_message_bar_severity_color(&test_bar, 2).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x999999).full, egui_view_message_bar_severity_color(&test_bar, 3).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full, egui_view_message_bar_severity_color(&test_bar, 9).full);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 65).full, mixed.full);
}

void test_message_bar_run(void)
{
    EGUI_TEST_SUITE_BEGIN(message_bar);
    EGUI_TEST_RUN(test_message_bar_set_snapshots_clamps_and_clears_pressed_state);
    EGUI_TEST_RUN(test_message_bar_snapshot_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_message_bar_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_message_bar_keyboard_click_listener);
    EGUI_TEST_RUN(test_message_bar_compact_mode_clears_pressed_and_keeps_click_behavior);
    EGUI_TEST_RUN(test_message_bar_read_only_and_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_message_bar_static_preview_consumes_input_and_keeps_snapshot);
    EGUI_TEST_RUN(test_message_bar_internal_helpers_cover_severity_and_text);
    EGUI_TEST_SUITE_END();
}
