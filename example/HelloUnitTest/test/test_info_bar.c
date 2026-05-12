#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_info_bar.h"

#include "../../HelloCustomWidgets/feedback/info_bar/egui_view_info_bar.h"
#include "../../HelloCustomWidgets/feedback/info_bar/egui_view_info_bar.c"

typedef struct info_bar_preview_snapshot info_bar_preview_snapshot_t;
struct info_bar_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const egui_view_info_bar_snapshot_t *snapshots;
    egui_view_info_bar_action_listener_t on_action;
    egui_view_info_bar_open_changed_listener_t on_open_changed;
    const egui_font_t *font;
    const egui_font_t *meta_font;
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
    uint8_t opened;
    uint8_t pressed_part;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_info_bar_t test_info_bar;
static egui_view_info_bar_t preview_info_bar;
static egui_view_api_t preview_api;
static uint8_t g_last_action_snapshot;
static uint8_t g_action_count;
static uint8_t g_last_opened;
static uint8_t g_open_changed_count;

static const egui_view_info_bar_snapshot_t g_snapshots[] = {
        {"Sync complete", "All records are current.", "Details", 1, 1, 1},
        {"Policy update", "Review changes before publish.", "Review", 0, 1, 1},
        {"Storage warning", "Archive logs before next sync.", "Archive", 2, 1, 1},
        {"Sign-in required", "Reconnect account to continue.", "Sign in", 3, 1, 1},
};

static const egui_view_info_bar_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", "A", 0, 1, 1},
        {"B", "B", "B", 1, 1, 1},
        {"C", "C", "C", 2, 1, 1},
        {"D", "D", "D", 3, 1, 1},
        {"E", "E", "E", 0, 0, 0},
        {"F", "F", "F", 1, 0, 0},
};

static const egui_view_info_bar_snapshot_t g_preview_snapshots[] = {
        {"Compact note", "Policy synced.", "View", 0, 0, 1},
};

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void on_info_bar_action(egui_view_t *self, uint8_t snapshot_index)
{
    EGUI_UNUSED(self);
    g_last_action_snapshot = snapshot_index;
    g_action_count++;
}

static void on_info_bar_open_changed(egui_view_t *self, uint8_t opened)
{
    EGUI_UNUSED(self);
    g_last_opened = opened;
    g_open_changed_count++;
}

static void reset_listener_counts(void)
{
    g_last_action_snapshot = 0xFF;
    g_action_count = 0;
    g_last_opened = 0xFF;
    g_open_changed_count = 0;
}

static void setup_info_bar(void)
{
    egui_view_info_bar_init(EGUI_VIEW_OF(&test_info_bar));
    egui_view_set_size(EGUI_VIEW_OF(&test_info_bar), 198, 82);
    egui_view_info_bar_set_snapshots(EGUI_VIEW_OF(&test_info_bar), g_snapshots, 4);
    egui_view_info_bar_set_on_action_listener(EGUI_VIEW_OF(&test_info_bar), on_info_bar_action);
    egui_view_info_bar_set_on_open_changed_listener(EGUI_VIEW_OF(&test_info_bar), on_info_bar_open_changed);
    reset_listener_counts();
}

static void setup_preview_info_bar(void)
{
    egui_view_info_bar_init(EGUI_VIEW_OF(&preview_info_bar));
    egui_view_set_size(EGUI_VIEW_OF(&preview_info_bar), 104, 54);
    egui_view_info_bar_set_snapshots(EGUI_VIEW_OF(&preview_info_bar), g_preview_snapshots, 1);
    egui_view_info_bar_set_font(EGUI_VIEW_OF(&preview_info_bar), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_info_bar_set_meta_font(EGUI_VIEW_OF(&preview_info_bar), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_info_bar_set_compact_mode(EGUI_VIEW_OF(&preview_info_bar), 1);
    egui_view_info_bar_set_on_action_listener(EGUI_VIEW_OF(&preview_info_bar), on_info_bar_action);
    egui_view_info_bar_set_on_open_changed_listener(EGUI_VIEW_OF(&preview_info_bar), on_info_bar_open_changed);
    egui_view_info_bar_override_static_preview_api(EGUI_VIEW_OF(&preview_info_bar), &preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&preview_info_bar), 0);
#endif
    reset_listener_counts();
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

static void layout_info_bar(void)
{
    layout_view(EGUI_VIEW_OF(&test_info_bar), 10, 20, 198, 82);
}

static void layout_preview_info_bar(void)
{
    layout_view(EGUI_VIEW_OF(&preview_info_bar), 12, 18, 104, 54);
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
    return send_touch_to_view(EGUI_VIEW_OF(&test_info_bar), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_info_bar), key_code);
}

static void get_metrics(egui_view_info_bar_metrics_t *metrics)
{
    egui_view_info_bar_get_metrics(&test_info_bar, EGUI_VIEW_OF(&test_info_bar), metrics);
}

static void get_region_center(const egui_region_t *region, egui_dim_t *x, egui_dim_t *y)
{
    *x = region->location.x + region->size.width / 2;
    *y = region->location.y + region->size.height / 2;
}

static void get_action_center(egui_dim_t *x, egui_dim_t *y)
{
    egui_view_info_bar_metrics_t metrics;

    get_metrics(&metrics);
    EGUI_TEST_ASSERT_TRUE(metrics.show_action);
    get_region_center(&metrics.action_region, x, y);
}

static void get_close_center(egui_dim_t *x, egui_dim_t *y)
{
    egui_view_info_bar_metrics_t metrics;

    get_metrics(&metrics);
    EGUI_TEST_ASSERT_TRUE(metrics.show_close);
    get_region_center(&metrics.close_region, x, y);
}

static void get_view_outside_point(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x - 6;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void seed_pressed_state(egui_view_t *view, uint8_t pressed_part)
{
    egui_view_set_pressed(view, 1);
    ((egui_view_info_bar_t *)view)->pressed_part = pressed_part;
}

static void assert_pressed_cleared(egui_view_info_bar_t *info_bar)
{
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(info_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_INFO_BAR_PART_NONE, info_bar->pressed_part);
}

static void capture_preview_snapshot(info_bar_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_info_bar)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_info_bar)->background;
    snapshot->snapshots = preview_info_bar.snapshots;
    snapshot->on_action = preview_info_bar.on_action;
    snapshot->on_open_changed = preview_info_bar.on_open_changed;
    snapshot->font = preview_info_bar.font;
    snapshot->meta_font = preview_info_bar.meta_font;
    snapshot->api = EGUI_VIEW_OF(&preview_info_bar)->api;
    snapshot->surface_color = preview_info_bar.surface_color;
    snapshot->border_color = preview_info_bar.border_color;
    snapshot->text_color = preview_info_bar.text_color;
    snapshot->muted_text_color = preview_info_bar.muted_text_color;
    snapshot->accent_color = preview_info_bar.accent_color;
    snapshot->info_color = preview_info_bar.info_color;
    snapshot->success_color = preview_info_bar.success_color;
    snapshot->warning_color = preview_info_bar.warning_color;
    snapshot->error_color = preview_info_bar.error_color;
    snapshot->snapshot_count = preview_info_bar.snapshot_count;
    snapshot->current_snapshot = preview_info_bar.current_snapshot;
    snapshot->compact_mode = preview_info_bar.compact_mode;
    snapshot->read_only_mode = preview_info_bar.read_only_mode;
    snapshot->opened = preview_info_bar.opened;
    snapshot->pressed_part = preview_info_bar.pressed_part;
    snapshot->alpha = EGUI_VIEW_OF(&preview_info_bar)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_info_bar));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_info_bar)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_info_bar)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_info_bar)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_info_bar)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_info_bar)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_info_bar)->padding.bottom;
}

static void assert_preview_state_unchanged(const info_bar_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_info_bar)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_info_bar)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_info_bar.snapshots == snapshot->snapshots);
    EGUI_TEST_ASSERT_TRUE(preview_info_bar.on_action == snapshot->on_action);
    EGUI_TEST_ASSERT_TRUE(preview_info_bar.on_open_changed == snapshot->on_open_changed);
    EGUI_TEST_ASSERT_TRUE(preview_info_bar.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_info_bar.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_info_bar)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_info_bar.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_info_bar.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_info_bar.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_info_bar.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_info_bar.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->info_color.full, preview_info_bar.info_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->success_color.full, preview_info_bar.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_info_bar.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->error_color.full, preview_info_bar.error_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->snapshot_count, preview_info_bar.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_snapshot, preview_info_bar.current_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_info_bar.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_info_bar.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->opened, preview_info_bar.opened);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_part, preview_info_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_info_bar)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_info_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_info_bar)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_info_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_info_bar)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_info_bar)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_info_bar)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_info_bar)->padding.bottom);
}

static void test_info_bar_set_snapshots_clamps_and_resets_state(void)
{
    setup_info_bar();

    test_info_bar.current_snapshot = EGUI_VIEW_INFO_BAR_MAX_SNAPSHOTS;
    seed_pressed_state(EGUI_VIEW_OF(&test_info_bar), EGUI_VIEW_INFO_BAR_PART_ACTION);
    egui_view_info_bar_set_snapshots(EGUI_VIEW_OF(&test_info_bar), g_overflow_snapshots, 6);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_INFO_BAR_MAX_SNAPSHOTS, test_info_bar.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_info_bar_get_current_snapshot(EGUI_VIEW_OF(&test_info_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_info_bar_get_opened(EGUI_VIEW_OF(&test_info_bar)));
    assert_pressed_cleared(&test_info_bar);

    test_info_bar.current_snapshot = 2;
    seed_pressed_state(EGUI_VIEW_OF(&test_info_bar), EGUI_VIEW_INFO_BAR_PART_CLOSE);
    egui_view_info_bar_set_snapshots(EGUI_VIEW_OF(&test_info_bar), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_info_bar.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_info_bar_get_current_snapshot(EGUI_VIEW_OF(&test_info_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_info_bar_get_opened(EGUI_VIEW_OF(&test_info_bar)));
    assert_pressed_cleared(&test_info_bar);
}

static void test_info_bar_setters_clear_pressed_state_and_apply_palette(void)
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

    setup_info_bar();

    egui_view_info_bar_set_current_snapshot(EGUI_VIEW_OF(&test_info_bar), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_info_bar_get_current_snapshot(EGUI_VIEW_OF(&test_info_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_info_bar_get_opened(EGUI_VIEW_OF(&test_info_bar)));

    seed_pressed_state(EGUI_VIEW_OF(&test_info_bar), EGUI_VIEW_INFO_BAR_PART_ACTION);
    egui_view_info_bar_set_current_snapshot(EGUI_VIEW_OF(&test_info_bar), 8);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_info_bar_get_current_snapshot(EGUI_VIEW_OF(&test_info_bar)));
    assert_pressed_cleared(&test_info_bar);

    seed_pressed_state(EGUI_VIEW_OF(&test_info_bar), EGUI_VIEW_INFO_BAR_PART_CLOSE);
    egui_view_info_bar_set_opened(EGUI_VIEW_OF(&test_info_bar), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_info_bar_get_opened(EGUI_VIEW_OF(&test_info_bar)));
    assert_pressed_cleared(&test_info_bar);

    egui_view_info_bar_set_opened(EGUI_VIEW_OF(&test_info_bar), 1);
    seed_pressed_state(EGUI_VIEW_OF(&test_info_bar), EGUI_VIEW_INFO_BAR_PART_ACTION);
    egui_view_info_bar_set_font(EGUI_VIEW_OF(&test_info_bar), NULL);
    EGUI_TEST_ASSERT_TRUE(test_info_bar.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_info_bar);

    seed_pressed_state(EGUI_VIEW_OF(&test_info_bar), EGUI_VIEW_INFO_BAR_PART_ACTION);
    egui_view_info_bar_set_meta_font(EGUI_VIEW_OF(&test_info_bar), NULL);
    EGUI_TEST_ASSERT_TRUE(test_info_bar.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_info_bar);

    seed_pressed_state(EGUI_VIEW_OF(&test_info_bar), EGUI_VIEW_INFO_BAR_PART_ACTION);
    egui_view_info_bar_set_compact_mode(EGUI_VIEW_OF(&test_info_bar), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_info_bar.compact_mode);
    assert_pressed_cleared(&test_info_bar);

    seed_pressed_state(EGUI_VIEW_OF(&test_info_bar), EGUI_VIEW_INFO_BAR_PART_ACTION);
    egui_view_info_bar_set_read_only_mode(EGUI_VIEW_OF(&test_info_bar), 4);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_info_bar.read_only_mode);
    assert_pressed_cleared(&test_info_bar);

    seed_pressed_state(EGUI_VIEW_OF(&test_info_bar), EGUI_VIEW_INFO_BAR_PART_ACTION);
    egui_view_info_bar_set_palette(EGUI_VIEW_OF(&test_info_bar), surface, border, text, muted, accent, info, success, warning, error);
    assert_pressed_cleared(&test_info_bar);
    EGUI_TEST_ASSERT_EQUAL_INT(surface.full, test_info_bar.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(border.full, test_info_bar.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(text.full, test_info_bar.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(muted.full, test_info_bar.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(accent.full, test_info_bar.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(info.full, test_info_bar.info_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(success.full, test_info_bar.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(warning.full, test_info_bar.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(error.full, test_info_bar.error_color.full);
}

static void test_info_bar_action_uses_same_target_release(void)
{
    egui_dim_t action_x;
    egui_dim_t action_y;
    egui_dim_t close_x;
    egui_dim_t close_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_info_bar();
    layout_info_bar();
    get_action_center(&action_x, &action_y);
    get_close_center(&close_x, &close_y);
    get_view_outside_point(EGUI_VIEW_OF(&test_info_bar), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, action_x, action_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_INFO_BAR_PART_ACTION, test_info_bar.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, close_x, close_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_info_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, close_x, close_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_info_bar);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, action_x, action_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_info_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, action_x, action_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_info_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, action_x, action_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_action_snapshot);
    assert_pressed_cleared(&test_info_bar);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, action_x, action_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, action_x, action_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    assert_pressed_cleared(&test_info_bar);
}

static void test_info_bar_close_uses_same_target_release_and_notifies(void)
{
    egui_dim_t action_x;
    egui_dim_t action_y;
    egui_dim_t close_x;
    egui_dim_t close_y;

    setup_info_bar();
    layout_info_bar();
    get_action_center(&action_x, &action_y);
    get_close_center(&close_x, &close_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, close_x, close_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, action_x, action_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_info_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, action_x, action_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_info_bar_get_opened(EGUI_VIEW_OF(&test_info_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_changed_count);
    assert_pressed_cleared(&test_info_bar);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, close_x, close_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, close_x, close_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_info_bar_get_opened(EGUI_VIEW_OF(&test_info_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_opened);
    assert_pressed_cleared(&test_info_bar);
}

static void test_info_bar_keyboard_action_and_escape_close(void)
{
    setup_info_bar();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_action_snapshot);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_action_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_info_bar_get_opened(EGUI_VIEW_OF(&test_info_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_opened);
}

static void test_info_bar_guards_reject_input_and_clear_pressed(void)
{
    egui_dim_t action_x;
    egui_dim_t action_y;

    setup_info_bar();
    layout_info_bar();
    get_action_center(&action_x, &action_y);

    seed_pressed_state(EGUI_VIEW_OF(&test_info_bar), EGUI_VIEW_INFO_BAR_PART_ACTION);
    egui_view_info_bar_set_read_only_mode(EGUI_VIEW_OF(&test_info_bar), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, action_x, action_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_info_bar);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    egui_view_info_bar_set_read_only_mode(EGUI_VIEW_OF(&test_info_bar), 0);
    seed_pressed_state(EGUI_VIEW_OF(&test_info_bar), EGUI_VIEW_INFO_BAR_PART_ACTION);
    egui_view_set_enable(EGUI_VIEW_OF(&test_info_bar), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, action_x, action_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_info_bar);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_info_bar), 1);
    egui_view_info_bar_set_opened(EGUI_VIEW_OF(&test_info_bar), 0);
    seed_pressed_state(EGUI_VIEW_OF(&test_info_bar), EGUI_VIEW_INFO_BAR_PART_ACTION);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, action_x, action_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_info_bar);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_info_bar_static_preview_consumes_input_and_keeps_state(void)
{
    info_bar_preview_snapshot_t initial_snapshot;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_info_bar();
    layout_preview_info_bar();
    x = EGUI_VIEW_OF(&preview_info_bar)->region_screen.location.x + EGUI_VIEW_OF(&preview_info_bar)->region_screen.size.width / 2;
    y = EGUI_VIEW_OF(&preview_info_bar)->region_screen.location.y + EGUI_VIEW_OF(&preview_info_bar)->region_screen.size.height / 2;
    capture_preview_snapshot(&initial_snapshot);

    seed_pressed_state(EGUI_VIEW_OF(&preview_info_bar), EGUI_VIEW_INFO_BAR_PART_ACTION);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_info_bar), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_info_bar), EGUI_MOTION_EVENT_ACTION_UP, x, y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_changed_count);

    seed_pressed_state(EGUI_VIEW_OF(&preview_info_bar), EGUI_VIEW_INFO_BAR_PART_ACTION);
    EGUI_TEST_ASSERT_TRUE(dispatch_key_event_to_view(EGUI_VIEW_OF(&preview_info_bar), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(dispatch_key_event_to_view(EGUI_VIEW_OF(&preview_info_bar), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_changed_count);
}

static void test_info_bar_internal_helpers_cover_text_severity_and_metrics(void)
{
    char label[24];
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);
    egui_color_t mixed = egui_view_info_bar_mix_disabled(sample);
    egui_view_info_bar_metrics_t metrics;

    setup_info_bar();
    layout_info_bar();
    egui_view_info_bar_set_palette(EGUI_VIEW_OF(&test_info_bar), EGUI_COLOR_HEX(0x111111), EGUI_COLOR_HEX(0x222222), EGUI_COLOR_HEX(0x333333),
                                   EGUI_COLOR_HEX(0x444444), EGUI_COLOR_HEX(0x555555), EGUI_COLOR_HEX(0x666666), EGUI_COLOR_HEX(0x777777),
                                   EGUI_COLOR_HEX(0x888888), EGUI_COLOR_HEX(0x999999));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_INFO_BAR_MAX_SNAPSHOTS, egui_view_info_bar_clamp_snapshot_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_info_bar_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_info_bar_text_len("Retry"));
    EGUI_TEST_ASSERT_TRUE(egui_view_info_bar_is_space_char(' '));
    EGUI_TEST_ASSERT_FALSE(egui_view_info_bar_is_space_char('x'));
    EGUI_TEST_ASSERT_TRUE(egui_view_info_bar_is_break_after_char('-'));
    EGUI_TEST_ASSERT_TRUE(egui_view_info_bar_is_break_after_char('/'));
    EGUI_TEST_ASSERT_FALSE(egui_view_info_bar_is_break_after_char('x'));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_info_bar_find_elide_boundary("Open latest release notes.", 7));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_info_bar_find_elide_boundary("scan-first review", 5));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_info_bar_measure_text_width(NULL, "Retry"));
    egui_view_info_bar_copy_elided(label, sizeof(label), "Documents", 6);
    EGUI_TEST_ASSERT_TRUE(strcmp("Doc...", label) == 0);
    egui_view_info_bar_copy_elided(label, sizeof(label), "Open latest release notes.", 8);
    EGUI_TEST_ASSERT_TRUE(strcmp("Open...", label) == 0);
    egui_view_info_bar_copy_elided(label, sizeof(label), "scan-first review", 10);
    EGUI_TEST_ASSERT_TRUE(strcmp("scan-...", label) == 0);
    egui_view_info_bar_copy_elided(label, sizeof(label), "View", 3);
    EGUI_TEST_ASSERT_TRUE(strcmp("...", label) == 0);
    egui_view_info_bar_fit_text_to_width(NULL, "Open latest release notes.", label, sizeof(label), 28, 4);
    EGUI_TEST_ASSERT_TRUE(strcmp("Open...", label) == 0);
    egui_view_info_bar_fit_text_to_width(NULL, "scan-first review", label, sizeof(label), 32, 4);
    EGUI_TEST_ASSERT_TRUE(strcmp("scan-...", label) == 0);
    egui_view_info_bar_fit_text_to_width(NULL, "Retry", label, sizeof(label), 24, 4);
    EGUI_TEST_ASSERT_TRUE(strcmp("Retry", label) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("i", egui_view_info_bar_severity_glyph(0)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("+", egui_view_info_bar_severity_glyph(1)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("!", egui_view_info_bar_severity_glyph(2)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("x", egui_view_info_bar_severity_glyph(3)) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full, egui_view_info_bar_severity_color(&test_info_bar, 0).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x777777).full, egui_view_info_bar_severity_color(&test_info_bar, 1).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x888888).full, egui_view_info_bar_severity_color(&test_info_bar, 2).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x999999).full, egui_view_info_bar_severity_color(&test_info_bar, 3).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full, egui_view_info_bar_severity_color(&test_info_bar, 9).full);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, EGUI_ALPHA_MAKE(52)).full, mixed.full);

    egui_view_info_bar_get_metrics(&test_info_bar, EGUI_VIEW_OF(&test_info_bar), &metrics);
    EGUI_TEST_ASSERT_TRUE(metrics.show_action);
    EGUI_TEST_ASSERT_TRUE(metrics.show_close);
    EGUI_TEST_ASSERT_TRUE(metrics.action_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.close_region.size.width > 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_INFO_BAR_PART_ACTION,
                               egui_view_info_bar_hit_part(&test_info_bar, EGUI_VIEW_OF(&test_info_bar),
                                                           metrics.action_region.location.x + metrics.action_region.size.width / 2,
                                                           metrics.action_region.location.y + metrics.action_region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_INFO_BAR_PART_CLOSE,
                               egui_view_info_bar_hit_part(&test_info_bar, EGUI_VIEW_OF(&test_info_bar),
                                                           metrics.close_region.location.x + metrics.close_region.size.width / 2,
                                                           metrics.close_region.location.y + metrics.close_region.size.height / 2));
}

void test_info_bar_run(void)
{
    EGUI_TEST_SUITE_BEGIN(info_bar);
    EGUI_TEST_RUN(test_info_bar_set_snapshots_clamps_and_resets_state);
    EGUI_TEST_RUN(test_info_bar_setters_clear_pressed_state_and_apply_palette);
    EGUI_TEST_RUN(test_info_bar_action_uses_same_target_release);
    EGUI_TEST_RUN(test_info_bar_close_uses_same_target_release_and_notifies);
    EGUI_TEST_RUN(test_info_bar_keyboard_action_and_escape_close);
    EGUI_TEST_RUN(test_info_bar_guards_reject_input_and_clear_pressed);
    EGUI_TEST_RUN(test_info_bar_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_RUN(test_info_bar_internal_helpers_cover_text_severity_and_metrics);
    EGUI_TEST_SUITE_END();
}
