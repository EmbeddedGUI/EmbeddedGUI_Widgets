#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_tool_tip.h"

#include "../../HelloCustomWidgets/feedback/tool_tip/egui_view_tool_tip.h"
#include "../../HelloCustomWidgets/feedback/tool_tip/egui_view_tool_tip.c"

typedef struct tool_tip_preview_snapshot tool_tip_preview_snapshot_t;
struct tool_tip_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const egui_view_tool_tip_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_click_listener_t on_click_listener;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t warning_color;
    egui_color_t neutral_color;
    egui_color_t shadow_color;
    egui_color_t target_fill_color;
    egui_color_t target_border_color;
    uint16_t show_delay_ms;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_part;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t open;
    uint8_t timer_started;
    uint8_t timer_running;
    uint8_t pending_show;
    uint8_t touch_active;
    uint8_t key_active;
    uint8_t toggle_on_release;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_tool_tip_t test_widget;
static egui_view_tool_tip_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

static const egui_view_tool_tip_snapshot_t test_snapshots[] = {
        {"Save", "Hint", "Quick save", "Appears after a short dwell.", EGUI_VIEW_TOOL_TIP_TONE_ACCENT, EGUI_VIEW_TOOL_TIP_PLACEMENT_BOTTOM, -12},
        {"Search", "Shortcut", "Press slash to search", "Placement above the target.", EGUI_VIEW_TOOL_TIP_TONE_ACCENT, EGUI_VIEW_TOOL_TIP_PLACEMENT_TOP, 12},
};

static const egui_view_tool_tip_snapshot_t preview_snapshots[] = {
        {"Preview", "Read only", "Review only", "Static comparison.", EGUI_VIEW_TOOL_TIP_TONE_NEUTRAL, EGUI_VIEW_TOOL_TIP_PLACEMENT_BOTTOM, 0},
};

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void on_tool_tip_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void reset_click_count(void)
{
    g_click_count = 0;
}

static void setup_widget(void)
{
    egui_timer_init();
    egui_view_tool_tip_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 140, 118);
    egui_view_tool_tip_set_snapshots(EGUI_VIEW_OF(&test_widget), test_snapshots, 2);
    egui_view_tool_tip_set_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_tool_tip_set_meta_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&test_widget), on_tool_tip_click);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&test_widget), 1);
#endif
    reset_click_count();
}

static void setup_preview_widget(void)
{
    egui_timer_init();
    egui_view_tool_tip_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 104, 82);
    egui_view_tool_tip_set_snapshots(EGUI_VIEW_OF(&preview_widget), preview_snapshots, 1);
    egui_view_tool_tip_set_current_snapshot(EGUI_VIEW_OF(&preview_widget), 0);
    egui_view_tool_tip_set_font(EGUI_VIEW_OF(&preview_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tool_tip_set_meta_font(EGUI_VIEW_OF(&preview_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tool_tip_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_tool_tip_set_open(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_widget), on_tool_tip_click);
    egui_view_tool_tip_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&preview_widget), 0);
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

static void layout_widget(void)
{
    layout_view(EGUI_VIEW_OF(&test_widget), 12, 18, 140, 118);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_widget), 16, 24, 104, 82);
}

static void attach_view(egui_view_t *view)
{
    egui_view_dispatch_attach_to_window(view);
}

static void detach_view(egui_view_t *view)
{
    egui_view_dispatch_detach_from_window(view);
}

static int dispatch_touch_event_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
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

static void capture_preview_snapshot(tool_tip_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_widget)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_widget)->background;
    snapshot->snapshots = preview_widget.snapshots;
    snapshot->font = preview_widget.font;
    snapshot->meta_font = preview_widget.meta_font;
    snapshot->on_click_listener = EGUI_VIEW_OF(&preview_widget)->on_click_listener;
    snapshot->api = EGUI_VIEW_OF(&preview_widget)->api;
    snapshot->surface_color = preview_widget.surface_color;
    snapshot->border_color = preview_widget.border_color;
    snapshot->text_color = preview_widget.text_color;
    snapshot->muted_text_color = preview_widget.muted_text_color;
    snapshot->accent_color = preview_widget.accent_color;
    snapshot->warning_color = preview_widget.warning_color;
    snapshot->neutral_color = preview_widget.neutral_color;
    snapshot->shadow_color = preview_widget.shadow_color;
    snapshot->target_fill_color = preview_widget.target_fill_color;
    snapshot->target_border_color = preview_widget.target_border_color;
    snapshot->show_delay_ms = preview_widget.show_delay_ms;
    snapshot->snapshot_count = preview_widget.snapshot_count;
    snapshot->current_snapshot = preview_widget.current_snapshot;
    snapshot->current_part = preview_widget.current_part;
    snapshot->compact_mode = preview_widget.compact_mode;
    snapshot->read_only_mode = preview_widget.read_only_mode;
    snapshot->open = preview_widget.open;
    snapshot->timer_started = preview_widget.timer_started;
    snapshot->timer_running = (uint8_t)egui_timer_check_timer_start(&preview_widget.show_timer);
    snapshot->pending_show = preview_widget.pending_show;
    snapshot->touch_active = preview_widget.touch_active;
    snapshot->key_active = preview_widget.key_active;
    snapshot->toggle_on_release = preview_widget.toggle_on_release;
    snapshot->alpha = EGUI_VIEW_OF(&preview_widget)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_widget));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_widget)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_widget)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_widget)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_widget)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_widget)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_widget)->padding.bottom;
}

static void assert_preview_state_unchanged(const tool_tip_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_widget)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_widget)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_widget.snapshots == snapshot->snapshots);
    EGUI_TEST_ASSERT_TRUE(preview_widget.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_widget.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_widget)->on_click_listener == snapshot->on_click_listener);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_widget)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_widget.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_widget.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_widget.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->neutral_color.full, preview_widget.neutral_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->shadow_color.full, preview_widget.shadow_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->target_fill_color.full, preview_widget.target_fill_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->target_border_color.full, preview_widget.target_border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->show_delay_ms, preview_widget.show_delay_ms);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->snapshot_count, preview_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_snapshot, preview_widget.current_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_part, preview_widget.current_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->open, preview_widget.open);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->timer_started, preview_widget.timer_started);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->timer_running, egui_timer_check_timer_start(&preview_widget.show_timer));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pending_show, preview_widget.pending_show);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->touch_active, preview_widget.touch_active);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->key_active, preview_widget.key_active);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->toggle_on_release, preview_widget.toggle_on_release);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_widget)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_widget)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_widget)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_widget)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_widget)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_widget)->padding.bottom);
}

static void get_target_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    EGUI_TEST_ASSERT_TRUE(egui_view_tool_tip_get_part_region(view, EGUI_VIEW_TOOL_TIP_PART_TARGET, &region));
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
}

static void assert_timer_stopped(egui_view_tool_tip_t *widget)
{
    EGUI_TEST_ASSERT_FALSE(widget->timer_started);
    EGUI_TEST_ASSERT_FALSE(egui_timer_check_timer_start(&widget->show_timer));
}

static void assert_interaction_cleared(egui_view_tool_tip_t *widget)
{
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(widget->touch_active);
    EGUI_TEST_ASSERT_FALSE(widget->key_active);
    EGUI_TEST_ASSERT_FALSE(widget->pending_show);
    EGUI_TEST_ASSERT_FALSE(widget->toggle_on_release);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOOL_TIP_PART_NONE, widget->current_part);
    assert_timer_stopped(widget);
}

static void seed_pending_state(egui_view_tool_tip_t *widget)
{
    widget->pending_show = 1;
    widget->touch_active = 1;
    widget->key_active = 0;
    widget->toggle_on_release = 1;
    widget->current_part = EGUI_VIEW_TOOL_TIP_PART_TARGET;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), 1);
    egui_timer_start_timer(&widget->show_timer, 100, 0);
    widget->timer_started = 1;
    EGUI_TEST_ASSERT_TRUE(egui_timer_check_timer_start(&widget->show_timer));
}

static void click_target_to_begin_delay(egui_view_tool_tip_t *widget, egui_dim_t *center_x, egui_dim_t *center_y)
{
    get_target_center(EGUI_VIEW_OF(widget), center_x, center_y);
    EGUI_TEST_ASSERT_TRUE(dispatch_touch_event_to_view(EGUI_VIEW_OF(widget), EGUI_MOTION_EVENT_ACTION_DOWN, *center_x, *center_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(dispatch_touch_event_to_view(EGUI_VIEW_OF(widget), EGUI_MOTION_EVENT_ACTION_UP, *center_x, *center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(widget->pending_show);
    EGUI_TEST_ASSERT_TRUE(widget->timer_started);
    EGUI_TEST_ASSERT_TRUE(egui_timer_check_timer_start(&widget->show_timer));
}

static void test_tool_tip_init_uses_default_state(void)
{
    egui_timer_init();
    egui_view_tool_tip_init(EGUI_VIEW_OF(&test_widget));

    EGUI_TEST_ASSERT_EQUAL_INT(420, egui_view_tool_tip_get_show_delay(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tool_tip_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tool_tip_get_compact_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tool_tip_get_read_only_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(egui_view_get_clickable(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(egui_view_get_focusable(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOOL_TIP_PART_NONE, test_widget.current_part);
    assert_timer_stopped(&test_widget);
}

static void test_tool_tip_setters_clear_pending_state(void)
{
    setup_widget();
    seed_pending_state(&test_widget);
    egui_view_tool_tip_set_show_delay(EGUI_VIEW_OF(&test_widget), 10);
    assert_interaction_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(120, egui_view_tool_tip_get_show_delay(EGUI_VIEW_OF(&test_widget)));

    seed_pending_state(&test_widget);
    egui_view_tool_tip_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 1);
    assert_interaction_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tool_tip_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));

    seed_pending_state(&test_widget);
    egui_view_tool_tip_set_compact_mode(EGUI_VIEW_OF(&test_widget), 2);
    assert_interaction_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tool_tip_get_compact_mode(EGUI_VIEW_OF(&test_widget)));

    seed_pending_state(&test_widget);
    egui_view_tool_tip_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 3);
    assert_interaction_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tool_tip_get_read_only_mode(EGUI_VIEW_OF(&test_widget)));

    egui_view_tool_tip_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);
    seed_pending_state(&test_widget);
    egui_view_tool_tip_set_open(EGUI_VIEW_OF(&test_widget), 1);
    assert_interaction_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tool_tip_get_open(EGUI_VIEW_OF(&test_widget)));

    seed_pending_state(&test_widget);
    egui_view_tool_tip_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_interaction_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pending_state(&test_widget);
    egui_view_tool_tip_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_interaction_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pending_state(&test_widget);
    egui_view_tool_tip_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x111213), EGUI_COLOR_HEX(0x212223), EGUI_COLOR_HEX(0x313233),
                                   EGUI_COLOR_HEX(0x414243), EGUI_COLOR_HEX(0x515253), EGUI_COLOR_HEX(0x616263), EGUI_COLOR_HEX(0x717273),
                                   EGUI_COLOR_HEX(0x818283), EGUI_COLOR_HEX(0x919293), EGUI_COLOR_HEX(0xA1A2A3));
    assert_interaction_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x313233).full, test_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x818283).full, test_widget.shadow_color.full);
}

static void test_tool_tip_touch_click_arms_delay_and_second_click_closes(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_widget();
    layout_widget();
    attach_view(EGUI_VIEW_OF(&test_widget));
    click_target_to_begin_delay(&test_widget, &center_x, &center_y);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tool_tip_get_open(EGUI_VIEW_OF(&test_widget)));

    egui_view_tool_tip_tick(&test_widget.show_timer);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tool_tip_get_open(EGUI_VIEW_OF(&test_widget)));
    assert_timer_stopped(&test_widget);

    EGUI_TEST_ASSERT_TRUE(dispatch_touch_event_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_widget.toggle_on_release);
    EGUI_TEST_ASSERT_TRUE(dispatch_touch_event_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tool_tip_get_open(EGUI_VIEW_OF(&test_widget)));
    assert_interaction_cleared(&test_widget);
}

static void test_tool_tip_touch_move_outside_cancels_delay(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;
    egui_dim_t outside_x;

    setup_widget();
    layout_widget();
    attach_view(EGUI_VIEW_OF(&test_widget));
    get_target_center(EGUI_VIEW_OF(&test_widget), &center_x, &center_y);
    outside_x = EGUI_VIEW_OF(&test_widget)->region_screen.location.x + EGUI_VIEW_OF(&test_widget)->region_screen.size.width + 12;

    EGUI_TEST_ASSERT_TRUE(dispatch_touch_event_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(dispatch_touch_event_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(dispatch_touch_event_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, outside_x, center_y));

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tool_tip_get_open(EGUI_VIEW_OF(&test_widget)));
    assert_interaction_cleared(&test_widget);
}

static void test_tool_tip_key_delay_and_escape_close(void)
{
    setup_widget();
    layout_widget();
    attach_view(EGUI_VIEW_OF(&test_widget));

    EGUI_TEST_ASSERT_TRUE(dispatch_key_event_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_widget.key_active);
    EGUI_TEST_ASSERT_TRUE(dispatch_key_event_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(test_widget.key_active);
    EGUI_TEST_ASSERT_TRUE(test_widget.pending_show);
    EGUI_TEST_ASSERT_TRUE(test_widget.timer_started);

    egui_view_tool_tip_tick(&test_widget.show_timer);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tool_tip_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(dispatch_key_event_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tool_tip_get_open(EGUI_VIEW_OF(&test_widget)));
    assert_interaction_cleared(&test_widget);
}

static void test_tool_tip_unhandled_key_clears_pressed_state(void)
{
    setup_widget();
    attach_view(EGUI_VIEW_OF(&test_widget));

    EGUI_TEST_ASSERT_TRUE(dispatch_key_event_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_widget.key_active);
    EGUI_TEST_ASSERT_FALSE(dispatch_key_event_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_TAB));
    assert_interaction_cleared(&test_widget);
}

static void test_tool_tip_disabled_and_read_only_guard_prevent_open(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_widget();
    layout_widget();
    attach_view(EGUI_VIEW_OF(&test_widget));
    get_target_center(EGUI_VIEW_OF(&test_widget), &center_x, &center_y);

    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_TRUE(dispatch_touch_event_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(dispatch_touch_event_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tool_tip_get_open(EGUI_VIEW_OF(&test_widget)));
    assert_interaction_cleared(&test_widget);

    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 1);
    egui_view_tool_tip_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(dispatch_key_event_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tool_tip_get_open(EGUI_VIEW_OF(&test_widget)));
    assert_interaction_cleared(&test_widget);
}

static void test_tool_tip_static_preview_consumes_input_and_keeps_state(void)
{
    tool_tip_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_widget();
    layout_preview_widget();
    attach_view(EGUI_VIEW_OF(&preview_widget));
    get_target_center(EGUI_VIEW_OF(&preview_widget), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    seed_pending_state(&preview_widget);
    preview_widget.open = initial_snapshot.open;
    EGUI_TEST_ASSERT_TRUE(dispatch_touch_event_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(dispatch_touch_event_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    seed_pending_state(&preview_widget);
    preview_widget.open = initial_snapshot.open;
    EGUI_TEST_ASSERT_TRUE(dispatch_key_event_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(dispatch_key_event_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

static void test_tool_tip_attach_and_detach_restore_pending_timer(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_widget();
    layout_widget();
    attach_view(EGUI_VIEW_OF(&test_widget));
    click_target_to_begin_delay(&test_widget, &center_x, &center_y);
    detach_view(EGUI_VIEW_OF(&test_widget));

    EGUI_TEST_ASSERT_TRUE(test_widget.pending_show);
    assert_timer_stopped(&test_widget);

    attach_view(EGUI_VIEW_OF(&test_widget));
    EGUI_TEST_ASSERT_TRUE(test_widget.timer_started);
    EGUI_TEST_ASSERT_TRUE(egui_timer_check_timer_start(&test_widget.show_timer));
    egui_view_tool_tip_tick(&test_widget.show_timer);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tool_tip_get_open(EGUI_VIEW_OF(&test_widget)));
}

void test_tool_tip_run(void)
{
    EGUI_TEST_SUITE_BEGIN(tool_tip);
    EGUI_TEST_RUN(test_tool_tip_init_uses_default_state);
    EGUI_TEST_RUN(test_tool_tip_setters_clear_pending_state);
    EGUI_TEST_RUN(test_tool_tip_touch_click_arms_delay_and_second_click_closes);
    EGUI_TEST_RUN(test_tool_tip_touch_move_outside_cancels_delay);
    EGUI_TEST_RUN(test_tool_tip_key_delay_and_escape_close);
    EGUI_TEST_RUN(test_tool_tip_unhandled_key_clears_pressed_state);
    EGUI_TEST_RUN(test_tool_tip_disabled_and_read_only_guard_prevent_open);
    EGUI_TEST_RUN(test_tool_tip_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_RUN(test_tool_tip_attach_and_detach_restore_pending_timer);
    EGUI_TEST_SUITE_END();
}
