#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_toast_stack.h"

#include "../../HelloCustomWidgets/feedback/toast_stack/egui_view_toast_stack.h"
#include "../../HelloCustomWidgets/feedback/toast_stack/egui_view_toast_stack.c"

typedef struct toast_stack_preview_snapshot toast_stack_preview_snapshot_t;
struct toast_stack_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const egui_view_toast_stack_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_click_listener_t on_click_listener;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t info_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t error_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

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

static const egui_view_toast_stack_snapshot_t g_preview_snapshots[] = {
        {"Quota alert", "Archive.", "Manage", "Soon", "Sync", "Pinned note", 2, 0},
};

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

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
    egui_view_toast_stack_set_snapshots(EGUI_VIEW_OF(&preview_stack), g_preview_snapshots, 1);
    egui_view_toast_stack_set_current_snapshot(EGUI_VIEW_OF(&preview_stack), 0);
    egui_view_toast_stack_set_font(EGUI_VIEW_OF(&preview_stack), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_toast_stack_set_meta_font(EGUI_VIEW_OF(&preview_stack), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_toast_stack_set_compact_mode(EGUI_VIEW_OF(&preview_stack), 1);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_stack), on_stack_click);
    egui_view_toast_stack_override_static_preview_api(EGUI_VIEW_OF(&preview_stack), &preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&preview_stack), 0);
#endif
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
    return view->api->dispatch_touch_event(view, &event);
}

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_UP, key_code);
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

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void capture_preview_snapshot(toast_stack_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_stack)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_stack)->background;
    snapshot->snapshots = preview_stack.snapshots;
    snapshot->font = preview_stack.font;
    snapshot->meta_font = preview_stack.meta_font;
    snapshot->on_click_listener = EGUI_VIEW_OF(&preview_stack)->on_click_listener;
    snapshot->api = EGUI_VIEW_OF(&preview_stack)->api;
    snapshot->surface_color = preview_stack.surface_color;
    snapshot->border_color = preview_stack.border_color;
    snapshot->text_color = preview_stack.text_color;
    snapshot->muted_text_color = preview_stack.muted_text_color;
    snapshot->accent_color = preview_stack.accent_color;
    snapshot->info_color = preview_stack.info_color;
    snapshot->success_color = preview_stack.success_color;
    snapshot->warning_color = preview_stack.warning_color;
    snapshot->error_color = preview_stack.error_color;
    snapshot->snapshot_count = preview_stack.snapshot_count;
    snapshot->current_snapshot = preview_stack.current_snapshot;
    snapshot->compact_mode = preview_stack.compact_mode;
    snapshot->read_only_mode = preview_stack.read_only_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_stack)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_stack));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_stack)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_stack)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_stack)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_stack)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_stack)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_stack)->padding.bottom;
}

static void assert_preview_state_unchanged(const toast_stack_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_stack)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_stack)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_stack.snapshots == snapshot->snapshots);
    EGUI_TEST_ASSERT_TRUE(preview_stack.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_stack.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_stack)->on_click_listener == snapshot->on_click_listener);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_stack)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_stack.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_stack.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_stack.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_stack.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_stack.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->info_color.full, preview_stack.info_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->success_color.full, preview_stack.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_stack.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->error_color.full, preview_stack.error_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->snapshot_count, preview_stack.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_snapshot, preview_stack.current_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_stack.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_stack.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_stack)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_stack)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_stack)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_stack)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_stack)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_stack)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_stack)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_stack)->padding.bottom);
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

static void test_toast_stack_static_preview_consumes_input_and_keeps_state(void)
{
    toast_stack_preview_snapshot_t initial_snapshot;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_stack();
    layout_preview_stack();
    get_view_center(EGUI_VIEW_OF(&preview_stack), &x, &y);
    capture_preview_snapshot(&initial_snapshot);

    seed_pressed_state(EGUI_VIEW_OF(&preview_stack), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    seed_pressed_state(EGUI_VIEW_OF(&preview_stack), 1);
    EGUI_TEST_ASSERT_TRUE(dispatch_key_event_to_view(EGUI_VIEW_OF(&preview_stack), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(dispatch_key_event_to_view(EGUI_VIEW_OF(&preview_stack), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
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
    EGUI_TEST_RUN(test_toast_stack_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_RUN(test_toast_stack_internal_helpers_cover_severity_and_text);
    EGUI_TEST_SUITE_END();
}
