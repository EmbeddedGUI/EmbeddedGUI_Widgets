#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_two_pane_view.h"

#include "../../HelloCustomWidgets/layout/two_pane_view/egui_view_two_pane_view.h"
#include "../../HelloCustomWidgets/layout/two_pane_view/egui_view_two_pane_view.c"

static egui_view_two_pane_view_t test_two_pane_view;
static egui_view_two_pane_view_t preview_two_pane_view;
static egui_view_api_t preview_api;
static uint8_t g_layout_changed_count;
static uint8_t g_last_layout_mode;
static uint8_t g_pane_changed_count;
static uint8_t g_last_pane;

typedef struct
{
    egui_view_two_pane_view_metrics_t metrics;
    uint8_t layout_mode;
    uint8_t single_pane;
    uint8_t compact_mode;
    uint8_t read_only_mode;
} two_pane_view_preview_snapshot_t;

static const egui_view_two_pane_view_pane_t g_first_pane = {
        "Primary pane",
        "Inbox timeline",
        "Wide / Tall",
        "Messages and filters stay visible",
        "The left pane keeps the scan path stable",
        "Open",
        EGUI_VIEW_TWO_PANE_VIEW_TONE_ACCENT,
        1,
};

static const egui_view_two_pane_view_pane_t g_second_pane = {
        "Secondary pane",
        "Reading surface",
        "Single pane ready",
        "Selected content can take the full area",
        "The second pane keeps context nearby",
        "Read",
        EGUI_VIEW_TWO_PANE_VIEW_TONE_SUCCESS,
        0,
};

static void on_layout_changed(egui_view_t *self, uint8_t layout_mode)
{
    EGUI_UNUSED(self);
    g_layout_changed_count++;
    g_last_layout_mode = layout_mode;
}

static void on_pane_changed(egui_view_t *self, uint8_t pane)
{
    EGUI_UNUSED(self);
    g_pane_changed_count++;
    g_last_pane = pane;
}

static void reset_listener_state(void)
{
    g_layout_changed_count = 0;
    g_last_layout_mode = 0xFF;
    g_pane_changed_count = 0;
    g_last_pane = 0xFF;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_two_pane_view(void)
{
    egui_view_two_pane_view_init(EGUI_VIEW_OF(&test_two_pane_view));
    egui_view_set_size(EGUI_VIEW_OF(&test_two_pane_view), 196, 106);
    egui_view_two_pane_view_set_panes(EGUI_VIEW_OF(&test_two_pane_view), &g_first_pane, &g_second_pane);
    egui_view_two_pane_view_set_on_layout_changed_listener(EGUI_VIEW_OF(&test_two_pane_view), on_layout_changed);
    egui_view_two_pane_view_set_on_pane_changed_listener(EGUI_VIEW_OF(&test_two_pane_view), on_pane_changed);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&test_two_pane_view), 1);
#endif
    reset_listener_state();
}

static void setup_preview_two_pane_view(void)
{
    egui_view_two_pane_view_init(EGUI_VIEW_OF(&preview_two_pane_view));
    egui_view_set_size(EGUI_VIEW_OF(&preview_two_pane_view), 104, 74);
    egui_view_two_pane_view_set_panes(EGUI_VIEW_OF(&preview_two_pane_view), &g_first_pane, &g_second_pane);
    egui_view_two_pane_view_set_compact_mode(EGUI_VIEW_OF(&preview_two_pane_view), 1);
    egui_view_two_pane_view_set_single_pane(EGUI_VIEW_OF(&preview_two_pane_view), EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND);
    egui_view_two_pane_view_set_layout_mode(EGUI_VIEW_OF(&preview_two_pane_view), EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE);
    egui_view_two_pane_view_set_on_layout_changed_listener(EGUI_VIEW_OF(&preview_two_pane_view), on_layout_changed);
    egui_view_two_pane_view_set_on_pane_changed_listener(EGUI_VIEW_OF(&preview_two_pane_view), on_pane_changed);
    egui_view_two_pane_view_override_static_preview_api(EGUI_VIEW_OF(&preview_two_pane_view), &preview_api);
    reset_listener_state();
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

static void layout_two_pane_view(egui_dim_t width, egui_dim_t height)
{
    layout_view(EGUI_VIEW_OF(&test_two_pane_view), 10, 20, width, height);
}

static void layout_preview_two_pane_view(void)
{
    layout_view(EGUI_VIEW_OF(&preview_two_pane_view), 12, 18, 104, 74);
}

static int send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = view->region_screen.location.x + x;
    event.location.y = view->region_screen.location.y + y;
    return view->api->on_touch_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= view->api->dispatch_key_event(view, &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= view->api->dispatch_key_event(view, &event);
    return handled;
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_two_pane_view), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_two_pane_view), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_two_pane_view), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_two_pane_view), key_code);
}

static void get_target_center(const egui_region_t *region, egui_dim_t *x, egui_dim_t *y)
{
    *x = region->location.x + region->size.width / 2;
    *y = region->location.y + region->size.height / 2;
}

static void get_view_outside_point(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    EGUI_UNUSED(view);
    *x = -4;
    *y = -4;
}

static void capture_preview_snapshot(two_pane_view_preview_snapshot_t *snapshot)
{
    tpv_get_metrics(&preview_two_pane_view, EGUI_VIEW_OF(&preview_two_pane_view), &snapshot->metrics);
    snapshot->layout_mode = egui_view_two_pane_view_get_layout_mode(EGUI_VIEW_OF(&preview_two_pane_view));
    snapshot->single_pane = egui_view_two_pane_view_get_single_pane(EGUI_VIEW_OF(&preview_two_pane_view));
    snapshot->compact_mode = preview_two_pane_view.compact_mode;
    snapshot->read_only_mode = preview_two_pane_view.read_only_mode;
}

static void assert_preview_state_unchanged(const two_pane_view_preview_snapshot_t *snapshot)
{
    egui_view_two_pane_view_metrics_t metrics;

    tpv_get_metrics(&preview_two_pane_view, EGUI_VIEW_OF(&preview_two_pane_view), &metrics);
    assert_region_equal(&snapshot->metrics.content, &metrics.content);
    assert_region_equal(&snapshot->metrics.first_pane, &metrics.first_pane);
    assert_region_equal(&snapshot->metrics.second_pane, &metrics.second_pane);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->metrics.show_first_pane, metrics.show_first_pane);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->metrics.show_second_pane, metrics.show_second_pane);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->layout_mode, egui_view_two_pane_view_get_layout_mode(EGUI_VIEW_OF(&preview_two_pane_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->single_pane, egui_view_two_pane_view_get_single_pane(EGUI_VIEW_OF(&preview_two_pane_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_two_pane_view.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_two_pane_view.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(TPV_HIT_NONE, preview_two_pane_view.pressed_target);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_two_pane_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_layout_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_pane_changed_count);
}

static void test_two_pane_view_defaults_helpers_and_setters(void)
{
    char label[12];
    egui_color_t sample = EGUI_COLOR_HEX(0x336699);

    setup_two_pane_view();

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE, egui_view_two_pane_view_get_layout_mode(EGUI_VIEW_OF(&test_two_pane_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST, egui_view_two_pane_view_get_single_pane(EGUI_VIEW_OF(&test_two_pane_view)));
    EGUI_TEST_ASSERT_TRUE(test_two_pane_view.first_pane == &g_first_pane);
    EGUI_TEST_ASSERT_TRUE(test_two_pane_view.second_pane == &g_second_pane);
    EGUI_TEST_ASSERT_EQUAL_INT(TPV_HIT_NONE, test_two_pane_view.pressed_target);
    EGUI_TEST_ASSERT_TRUE(tpv_layout_label(EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE, 0)[0] == 'W');
    EGUI_TEST_ASSERT_TRUE(tpv_layout_label(EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL, 1)[0] == 'T');
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE, tpv_clamp_layout(99));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST, tpv_clamp_pane(99));
    EGUI_TEST_ASSERT_EQUAL_INT(14, tpv_text_len("Inbox timeline"));

    tpv_copy_elided(label, sizeof(label), "Reading surface", 8);
    EGUI_TEST_ASSERT_TRUE(strcmp("Readi...", label) == 0);
    tpv_fit_text_to_width(NULL, "Reading surface", label, sizeof(label), 15, 5);
    EGUI_TEST_ASSERT_TRUE(strcmp("...", label) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, EGUI_ALPHA_MAKE(52)).full, tpv_mix_disabled(sample).full);

    egui_view_two_pane_view_set_font(EGUI_VIEW_OF(&test_two_pane_view), NULL);
    egui_view_two_pane_view_set_meta_font(EGUI_VIEW_OF(&test_two_pane_view), NULL);
    EGUI_TEST_ASSERT_TRUE(test_two_pane_view.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(test_two_pane_view.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    egui_view_two_pane_view_set_layout_mode(EGUI_VIEW_OF(&test_two_pane_view), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE, egui_view_two_pane_view_get_layout_mode(EGUI_VIEW_OF(&test_two_pane_view)));
    egui_view_two_pane_view_set_single_pane(EGUI_VIEW_OF(&test_two_pane_view), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST, egui_view_two_pane_view_get_single_pane(EGUI_VIEW_OF(&test_two_pane_view)));
    egui_view_two_pane_view_toggle_single_pane(EGUI_VIEW_OF(&test_two_pane_view));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND, egui_view_two_pane_view_get_single_pane(EGUI_VIEW_OF(&test_two_pane_view)));

    test_two_pane_view.pressed_target = TPV_HIT_LAYOUT_BASE + 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_two_pane_view), true);
    egui_view_two_pane_view_set_compact_mode(EGUI_VIEW_OF(&test_two_pane_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_two_pane_view.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(TPV_HIT_NONE, test_two_pane_view.pressed_target);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_two_pane_view)->is_pressed);
    egui_view_two_pane_view_set_read_only_mode(EGUI_VIEW_OF(&test_two_pane_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_two_pane_view.read_only_mode);

    egui_view_two_pane_view_set_palette(EGUI_VIEW_OF(&test_two_pane_view), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122),
                                        EGUI_COLOR_HEX(0x303132), EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152),
                                        EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172), EGUI_COLOR_HEX(0x808182),
                                        EGUI_COLOR_HEX(0x909192));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_two_pane_view.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, tpv_tone_color(&test_two_pane_view, EGUI_VIEW_TWO_PANE_VIEW_TONE_ACCENT).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x707172).full, tpv_tone_color(&test_two_pane_view, EGUI_VIEW_TWO_PANE_VIEW_TONE_SUCCESS).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x808182).full, tpv_tone_color(&test_two_pane_view, EGUI_VIEW_TWO_PANE_VIEW_TONE_WARNING).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x909192).full, tpv_tone_color(&test_two_pane_view, EGUI_VIEW_TWO_PANE_VIEW_TONE_NEUTRAL).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, tpv_tone_color(&test_two_pane_view, 99).full);
}

static void test_two_pane_view_metrics_and_hit_testing(void)
{
    egui_view_two_pane_view_metrics_t metrics;
    egui_dim_t x;
    egui_dim_t y;

    setup_two_pane_view();
    layout_two_pane_view(196, 106);

    tpv_get_metrics(&test_two_pane_view, EGUI_VIEW_OF(&test_two_pane_view), &metrics);
    EGUI_TEST_ASSERT_TRUE(metrics.content.size.width > 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, metrics.show_first_pane);
    EGUI_TEST_ASSERT_EQUAL_INT(1, metrics.show_second_pane);
    EGUI_TEST_ASSERT_TRUE(metrics.first_pane.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.second_pane.location.x > metrics.first_pane.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(1, metrics.divider.size.width);
    get_target_center(&metrics.layout_tabs[EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL], &x, &y);
    EGUI_TEST_ASSERT_EQUAL_INT(TPV_HIT_LAYOUT_BASE + EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL,
                               tpv_hit_target(&test_two_pane_view, EGUI_VIEW_OF(&test_two_pane_view), x, y));
    get_target_center(&metrics.pane_buttons[1], &x, &y);
    EGUI_TEST_ASSERT_EQUAL_INT(TPV_HIT_PANE_BASE + 1, tpv_hit_target(&test_two_pane_view, EGUI_VIEW_OF(&test_two_pane_view), x, y));

    egui_view_two_pane_view_set_layout_mode(EGUI_VIEW_OF(&test_two_pane_view), EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL);
    tpv_get_metrics(&test_two_pane_view, EGUI_VIEW_OF(&test_two_pane_view), &metrics);
    EGUI_TEST_ASSERT_TRUE(metrics.second_pane.location.y > metrics.first_pane.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(1, metrics.divider.size.height);

    egui_view_two_pane_view_set_single_pane(EGUI_VIEW_OF(&test_two_pane_view), EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND);
    egui_view_two_pane_view_set_layout_mode(EGUI_VIEW_OF(&test_two_pane_view), EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE);
    tpv_get_metrics(&test_two_pane_view, EGUI_VIEW_OF(&test_two_pane_view), &metrics);
    EGUI_TEST_ASSERT_EQUAL_INT(0, metrics.show_first_pane);
    EGUI_TEST_ASSERT_EQUAL_INT(1, metrics.show_second_pane);
    assert_region_equal(&metrics.pane_area, &metrics.second_pane);

    egui_view_two_pane_view_set_compact_mode(EGUI_VIEW_OF(&test_two_pane_view), 1);
    layout_two_pane_view(104, 74);
    tpv_get_metrics(&test_two_pane_view, EGUI_VIEW_OF(&test_two_pane_view), &metrics);
    EGUI_TEST_ASSERT_EQUAL_INT(TPV_COMPACT_CONTROL_W, metrics.layout_tabs[0].size.width);
    EGUI_TEST_ASSERT_TRUE(metrics.pane_buttons[1].location.x > metrics.layout_tabs[2].location.x);
}

static void test_two_pane_view_touch_release_semantics(void)
{
    egui_view_two_pane_view_metrics_t metrics;
    egui_dim_t tall_x;
    egui_dim_t tall_y;
    egui_dim_t single_x;
    egui_dim_t single_y;
    egui_dim_t pane2_x;
    egui_dim_t pane2_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_two_pane_view();
    layout_two_pane_view(196, 106);
    tpv_get_metrics(&test_two_pane_view, EGUI_VIEW_OF(&test_two_pane_view), &metrics);
    get_target_center(&metrics.layout_tabs[EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL], &tall_x, &tall_y);
    get_target_center(&metrics.layout_tabs[EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE], &single_x, &single_y);
    get_target_center(&metrics.pane_buttons[1], &pane2_x, &pane2_y);
    get_view_outside_point(EGUI_VIEW_OF(&test_two_pane_view), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, outside_x, outside_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, tall_x, tall_y));
    EGUI_TEST_ASSERT_EQUAL_INT(TPV_HIT_LAYOUT_BASE + EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL, test_two_pane_view.pressed_target);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_two_pane_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_two_pane_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE, egui_view_two_pane_view_get_layout_mode(EGUI_VIEW_OF(&test_two_pane_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_layout_changed_count);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, tall_x, tall_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, single_x, single_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_two_pane_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, tall_x, tall_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_two_pane_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, tall_x, tall_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL, egui_view_two_pane_view_get_layout_mode(EGUI_VIEW_OF(&test_two_pane_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_layout_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL, g_last_layout_mode);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, pane2_x, pane2_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, pane2_x, pane2_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND, egui_view_two_pane_view_get_single_pane(EGUI_VIEW_OF(&test_two_pane_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE, egui_view_two_pane_view_get_layout_mode(EGUI_VIEW_OF(&test_two_pane_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_layout_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_pane_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND, g_last_pane);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, tall_x, tall_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, tall_x, tall_y));
    EGUI_TEST_ASSERT_EQUAL_INT(TPV_HIT_NONE, test_two_pane_view.pressed_target);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_two_pane_view)->is_pressed);
}

static void test_two_pane_view_keyboard_read_only_and_disabled_guards(void)
{
    egui_view_two_pane_view_metrics_t metrics;
    egui_dim_t x;
    egui_dim_t y;

    setup_two_pane_view();
    layout_two_pane_view(196, 106);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL, egui_view_two_pane_view_get_layout_mode(EGUI_VIEW_OF(&test_two_pane_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_layout_changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE, egui_view_two_pane_view_get_layout_mode(EGUI_VIEW_OF(&test_two_pane_view)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND, egui_view_two_pane_view_get_single_pane(EGUI_VIEW_OF(&test_two_pane_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_pane_changed_count);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE, egui_view_two_pane_view_get_layout_mode(EGUI_VIEW_OF(&test_two_pane_view)));

    tpv_get_metrics(&test_two_pane_view, EGUI_VIEW_OF(&test_two_pane_view), &metrics);
    get_target_center(&metrics.layout_tabs[1], &x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_two_pane_view)->is_pressed);
    test_two_pane_view.pressed_target = TPV_HIT_LAYOUT_BASE + 1;
    egui_view_two_pane_view_set_read_only_mode(EGUI_VIEW_OF(&test_two_pane_view), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(TPV_HIT_NONE, test_two_pane_view.pressed_target);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_two_pane_view)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));

    egui_view_two_pane_view_set_read_only_mode(EGUI_VIEW_OF(&test_two_pane_view), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_two_pane_view), 0);
    test_two_pane_view.pressed_target = TPV_HIT_LAYOUT_BASE + 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_two_pane_view), true);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(TPV_HIT_NONE, test_two_pane_view.pressed_target);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_two_pane_view)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));

    egui_view_set_enable(EGUI_VIEW_OF(&test_two_pane_view), 1);
    reset_listener_state();
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL, egui_view_two_pane_view_get_layout_mode(EGUI_VIEW_OF(&test_two_pane_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_layout_changed_count);
}

static void test_two_pane_view_static_preview_consumes_input_and_keeps_state(void)
{
    two_pane_view_preview_snapshot_t initial_snapshot;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_two_pane_view();
    layout_preview_two_pane_view();
    capture_preview_snapshot(&initial_snapshot);
    get_target_center(&initial_snapshot.metrics.layout_tabs[EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE], &x, &y);

    EGUI_VIEW_OF(&preview_two_pane_view)->is_pressed = true;
    preview_two_pane_view.pressed_target = TPV_HIT_LAYOUT_BASE + EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    EGUI_VIEW_OF(&preview_two_pane_view)->is_pressed = true;
    preview_two_pane_view.pressed_target = TPV_HIT_PANE_BASE;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    EGUI_VIEW_OF(&preview_two_pane_view)->is_pressed = true;
    preview_two_pane_view.pressed_target = TPV_HIT_PANE_BASE + 1;
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_RIGHT));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_two_pane_view_run(void)
{
    EGUI_TEST_SUITE_BEGIN(two_pane_view);
    EGUI_TEST_RUN(test_two_pane_view_defaults_helpers_and_setters);
    EGUI_TEST_RUN(test_two_pane_view_metrics_and_hit_testing);
    EGUI_TEST_RUN(test_two_pane_view_touch_release_semantics);
    EGUI_TEST_RUN(test_two_pane_view_keyboard_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_two_pane_view_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
