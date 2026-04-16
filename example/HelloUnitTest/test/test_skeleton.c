#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_skeleton.h"

#include "../../HelloCustomWidgets/feedback/skeleton/egui_view_skeleton.h"
#include "../../HelloCustomWidgets/feedback/skeleton/egui_view_skeleton.c"

typedef struct skeleton_preview_snapshot skeleton_preview_snapshot_t;
struct skeleton_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const egui_view_skeleton_snapshot_t *snapshots;
    const egui_font_t *font;
    egui_view_on_click_listener_t on_click_listener;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t block_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t emphasis_block;
    uint8_t show_footer;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t animation_mode;
    uint8_t anim_phase;
    uint8_t timer_started;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_skeleton_t test_skeleton;
static egui_view_skeleton_t preview_skeleton;
static egui_view_api_t preview_api;
static int click_count;

static const egui_view_skeleton_block_t g_blocks_a[] = {
        {0, 0, 24, 8, 4},
        {0, 12, 56, 7, 3},
        {0, 24, 72, 18, 7},
        {0, 48, 48, 6, 3},
};

static const egui_view_skeleton_block_t g_blocks_b[] = {
        {0, 0, 18, 18, 9},
        {24, 2, 72, 7, 3},
        {24, 14, 48, 6, 3},
        {0, 32, 92, 14, 5},
        {0, 52, 64, 6, 3},
};

static const egui_view_skeleton_snapshot_t g_snapshots[] = {
        {"Article", "Loading article", g_blocks_a, 4, 2},
        {"Feed", "Loading feed", g_blocks_b, 5, 3},
};

static const egui_view_skeleton_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", g_blocks_a, 4, 0},
        {"B", "B", g_blocks_b, 5, 1},
        {"C", "C", g_blocks_a, 4, 2},
        {"D", "D", g_blocks_b, 5, 3},
        {"E", "E", g_blocks_a, 4, 0},
};

static const egui_view_skeleton_snapshot_t g_preview_snapshots[] = {
        {"Compact row", NULL, g_blocks_a, 4, 2},
};

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void on_skeleton_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    click_count++;
}

static void reset_click_count(void)
{
    click_count = 0;
}

static void setup_skeleton(void)
{
    egui_timer_init();
    egui_view_skeleton_init(EGUI_VIEW_OF(&test_skeleton));
    egui_view_set_size(EGUI_VIEW_OF(&test_skeleton), 196, 96);
    egui_view_skeleton_set_snapshots(EGUI_VIEW_OF(&test_skeleton), g_snapshots, 2);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&test_skeleton), on_skeleton_click);
    reset_click_count();
}

static void setup_preview_skeleton(void)
{
    egui_timer_init();
    egui_view_skeleton_init(EGUI_VIEW_OF(&preview_skeleton));
    egui_view_set_size(EGUI_VIEW_OF(&preview_skeleton), 104, 60);
    egui_view_skeleton_set_snapshots(EGUI_VIEW_OF(&preview_skeleton), g_preview_snapshots, 1);
    egui_view_skeleton_set_current_snapshot(EGUI_VIEW_OF(&preview_skeleton), 0);
    egui_view_skeleton_set_font(EGUI_VIEW_OF(&preview_skeleton), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_skeleton_set_show_footer(EGUI_VIEW_OF(&preview_skeleton), 0);
    egui_view_skeleton_set_compact_mode(EGUI_VIEW_OF(&preview_skeleton), 1);
    egui_view_skeleton_set_animation_mode(EGUI_VIEW_OF(&preview_skeleton), EGUI_VIEW_SKELETON_ANIM_NONE);
    egui_view_skeleton_set_emphasis_block(EGUI_VIEW_OF(&preview_skeleton), 2);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_skeleton), on_skeleton_click);
    egui_view_skeleton_override_static_preview_api(EGUI_VIEW_OF(&preview_skeleton), &preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&preview_skeleton), 0);
#endif
    reset_click_count();
}

static void attach_view(egui_view_t *view)
{
    egui_view_dispatch_attach_to_window(view);
}

static void detach_view(egui_view_t *view)
{
    egui_view_dispatch_detach_from_window(view);
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

static void layout_skeleton(void)
{
    layout_view(EGUI_VIEW_OF(&test_skeleton), 10, 20, 196, 96);
}

static void layout_preview_skeleton(void)
{
    layout_view(EGUI_VIEW_OF(&preview_skeleton), 12, 18, 104, 60);
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
    return send_touch_to_view(EGUI_VIEW_OF(&test_skeleton), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_skeleton), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_skeleton), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_skeleton), key_code);
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void capture_preview_snapshot(skeleton_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_skeleton)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_skeleton)->background;
    snapshot->snapshots = preview_skeleton.snapshots;
    snapshot->font = preview_skeleton.font;
    snapshot->on_click_listener = EGUI_VIEW_OF(&preview_skeleton)->on_click_listener;
    snapshot->api = EGUI_VIEW_OF(&preview_skeleton)->api;
    snapshot->surface_color = preview_skeleton.surface_color;
    snapshot->border_color = preview_skeleton.border_color;
    snapshot->block_color = preview_skeleton.block_color;
    snapshot->text_color = preview_skeleton.text_color;
    snapshot->muted_text_color = preview_skeleton.muted_text_color;
    snapshot->accent_color = preview_skeleton.accent_color;
    snapshot->snapshot_count = preview_skeleton.snapshot_count;
    snapshot->current_snapshot = preview_skeleton.current_snapshot;
    snapshot->emphasis_block = preview_skeleton.emphasis_block;
    snapshot->show_footer = preview_skeleton.show_footer;
    snapshot->compact_mode = preview_skeleton.compact_mode;
    snapshot->read_only_mode = preview_skeleton.read_only_mode;
    snapshot->animation_mode = preview_skeleton.animation_mode;
    snapshot->anim_phase = preview_skeleton.anim_phase;
    snapshot->timer_started = preview_skeleton.timer_started;
    snapshot->alpha = EGUI_VIEW_OF(&preview_skeleton)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_skeleton));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_skeleton)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_skeleton)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_skeleton)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_skeleton)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_skeleton)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_skeleton)->padding.bottom;
}

static void assert_preview_state_unchanged(const skeleton_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_skeleton)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_skeleton)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_skeleton.snapshots == snapshot->snapshots);
    EGUI_TEST_ASSERT_TRUE(preview_skeleton.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_skeleton)->on_click_listener == snapshot->on_click_listener);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_skeleton)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_skeleton.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_skeleton.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->block_color.full, preview_skeleton.block_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_skeleton.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_skeleton.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_skeleton.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->snapshot_count, preview_skeleton.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_snapshot, preview_skeleton.current_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->emphasis_block, preview_skeleton.emphasis_block);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->show_footer, preview_skeleton.show_footer);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_skeleton.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_skeleton.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->animation_mode, preview_skeleton.animation_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->anim_phase, preview_skeleton.anim_phase);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->timer_started, preview_skeleton.timer_started);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_skeleton)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_skeleton)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_skeleton)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_skeleton)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_skeleton)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_skeleton)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_skeleton)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_skeleton)->padding.bottom);
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

static void stop_timer_if_started(egui_view_skeleton_t *skeleton)
{
    if (skeleton->timer_started)
    {
        egui_view_dispatch_detach_from_window(EGUI_VIEW_OF(skeleton));
    }
}

static void test_skeleton_set_snapshots_clamps_and_clears_pressed_state(void)
{
    setup_skeleton();

    test_skeleton.current_snapshot = EGUI_VIEW_SKELETON_MAX_SNAPSHOTS;
    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_snapshots(EGUI_VIEW_OF(&test_skeleton), g_overflow_snapshots, 5);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SKELETON_MAX_SNAPSHOTS, test_skeleton.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_skeleton_get_current_snapshot(EGUI_VIEW_OF(&test_skeleton)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    test_skeleton.current_snapshot = 1;
    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_snapshots(EGUI_VIEW_OF(&test_skeleton), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_skeleton.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_skeleton_get_current_snapshot(EGUI_VIEW_OF(&test_skeleton)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));
}

static void test_skeleton_snapshot_setters_and_timer_state_clear_pressed(void)
{
    egui_color_t surface = EGUI_COLOR_HEX(0x101112);
    egui_color_t border = EGUI_COLOR_HEX(0x202122);
    egui_color_t block = EGUI_COLOR_HEX(0x303132);
    egui_color_t text = EGUI_COLOR_HEX(0x404142);
    egui_color_t muted = EGUI_COLOR_HEX(0x505152);
    egui_color_t accent = EGUI_COLOR_HEX(0x606162);

    setup_skeleton();

    egui_view_skeleton_set_current_snapshot(EGUI_VIEW_OF(&test_skeleton), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_skeleton_get_current_snapshot(EGUI_VIEW_OF(&test_skeleton)));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_current_snapshot(EGUI_VIEW_OF(&test_skeleton), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_skeleton_get_current_snapshot(EGUI_VIEW_OF(&test_skeleton)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_current_snapshot(EGUI_VIEW_OF(&test_skeleton), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_skeleton_get_current_snapshot(EGUI_VIEW_OF(&test_skeleton)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_emphasis_block(EGUI_VIEW_OF(&test_skeleton), 6);
    EGUI_TEST_ASSERT_EQUAL_INT(6, test_skeleton.emphasis_block);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_font(EGUI_VIEW_OF(&test_skeleton), NULL);
    EGUI_TEST_ASSERT_TRUE(test_skeleton.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_show_footer(EGUI_VIEW_OF(&test_skeleton), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_skeleton.show_footer);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_palette(EGUI_VIEW_OF(&test_skeleton), surface, border, block, text, muted, accent);
    EGUI_TEST_ASSERT_EQUAL_INT(surface.full, test_skeleton.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(border.full, test_skeleton.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(block.full, test_skeleton.block_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(text.full, test_skeleton.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(muted.full, test_skeleton.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(accent.full, test_skeleton.accent_color.full);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_compact_mode(EGUI_VIEW_OF(&test_skeleton), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_skeleton.compact_mode);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_animation_mode(EGUI_VIEW_OF(&test_skeleton), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SKELETON_ANIM_PULSE, test_skeleton.animation_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_skeleton.timer_started);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_read_only_mode(EGUI_VIEW_OF(&test_skeleton), 4);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_skeleton.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_skeleton.timer_started);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_animation_mode(EGUI_VIEW_OF(&test_skeleton), EGUI_VIEW_SKELETON_ANIM_WAVE);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SKELETON_ANIM_WAVE, test_skeleton.animation_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_skeleton.timer_started);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_read_only_mode(EGUI_VIEW_OF(&test_skeleton), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_skeleton.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_skeleton.timer_started);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    egui_view_skeleton_set_animation_mode(EGUI_VIEW_OF(&test_skeleton), EGUI_VIEW_SKELETON_ANIM_NONE);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SKELETON_ANIM_NONE, test_skeleton.animation_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_skeleton.timer_started);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    stop_timer_if_started(&test_skeleton);
}

static void test_skeleton_attach_detach_and_helper_functions(void)
{
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);

    setup_skeleton();

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SKELETON_MAX_SNAPSHOTS, egui_view_skeleton_clamp_snapshot_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SKELETON_MAX_BLOCKS, egui_view_skeleton_clamp_block_count(20));
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_skeleton_get_pulse_mix(0));
    EGUI_TEST_ASSERT_EQUAL_INT(42, egui_view_skeleton_get_pulse_mix(6));
    EGUI_TEST_ASSERT_EQUAL_INT(27, egui_view_skeleton_get_pulse_mix(9));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 62).full, egui_view_skeleton_mix_disabled(sample).full);

    egui_view_skeleton_set_animation_mode(EGUI_VIEW_OF(&test_skeleton), EGUI_VIEW_SKELETON_ANIM_WAVE);
    egui_view_dispatch_attach_to_window(EGUI_VIEW_OF(&test_skeleton));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_skeleton.timer_started);

    test_skeleton.anim_phase = 23;
    egui_view_skeleton_tick(&test_skeleton.anim_timer);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_skeleton.anim_phase);

    egui_view_dispatch_detach_from_window(EGUI_VIEW_OF(&test_skeleton));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_skeleton.timer_started);
}

static void test_skeleton_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_skeleton();
    layout_skeleton();
    get_view_center(EGUI_VIEW_OF(&test_skeleton), &inside_x, &inside_y);
    get_view_outside_point(EGUI_VIEW_OF(&test_skeleton), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_skeleton)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_skeleton)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_skeleton)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_skeleton)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    stop_timer_if_started(&test_skeleton);
}

static void test_skeleton_keyboard_click_listener(void)
{
    setup_skeleton();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);

    stop_timer_if_started(&test_skeleton);
}

static void test_skeleton_compact_mode_clears_pressed_and_keeps_click_behavior(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_skeleton();
    layout_skeleton();
    get_view_center(EGUI_VIEW_OF(&test_skeleton), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_skeleton)->is_pressed);

    egui_view_skeleton_set_compact_mode(EGUI_VIEW_OF(&test_skeleton), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_skeleton.compact_mode);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);

    stop_timer_if_started(&test_skeleton);
}

static void test_skeleton_read_only_and_disabled_guards_clear_pressed_and_timer(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_skeleton();
    layout_skeleton();
    get_view_center(EGUI_VIEW_OF(&test_skeleton), &x, &y);

    egui_view_skeleton_set_animation_mode(EGUI_VIEW_OF(&test_skeleton), EGUI_VIEW_SKELETON_ANIM_PULSE);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_skeleton.timer_started);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_skeleton)->is_pressed);

    egui_view_skeleton_set_read_only_mode(EGUI_VIEW_OF(&test_skeleton), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_skeleton.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_skeleton.timer_started);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    egui_view_skeleton_set_read_only_mode(EGUI_VIEW_OF(&test_skeleton), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_skeleton.timer_started);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);

    setup_skeleton();
    layout_skeleton();
    get_view_center(EGUI_VIEW_OF(&test_skeleton), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_skeleton)->is_pressed);

    egui_view_set_enable(EGUI_VIEW_OF(&test_skeleton), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));

    seed_pressed_state(EGUI_VIEW_OF(&test_skeleton), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_skeleton));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    stop_timer_if_started(&test_skeleton);
}

static void test_skeleton_static_preview_consumes_input_and_keeps_state(void)
{
    skeleton_preview_snapshot_t initial_snapshot;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_skeleton();
    attach_view(EGUI_VIEW_OF(&preview_skeleton));
    layout_preview_skeleton();
    get_view_center(EGUI_VIEW_OF(&preview_skeleton), &x, &y);
    capture_preview_snapshot(&initial_snapshot);

    seed_pressed_state(EGUI_VIEW_OF(&preview_skeleton), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    seed_pressed_state(EGUI_VIEW_OF(&preview_skeleton), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    detach_view(EGUI_VIEW_OF(&preview_skeleton));
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_skeleton.timer_started);
}

void test_skeleton_run(void)
{
    EGUI_TEST_SUITE_BEGIN(skeleton);
    EGUI_TEST_RUN(test_skeleton_set_snapshots_clamps_and_clears_pressed_state);
    EGUI_TEST_RUN(test_skeleton_snapshot_setters_and_timer_state_clear_pressed);
    EGUI_TEST_RUN(test_skeleton_attach_detach_and_helper_functions);
    EGUI_TEST_RUN(test_skeleton_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_skeleton_keyboard_click_listener);
    EGUI_TEST_RUN(test_skeleton_compact_mode_clears_pressed_and_keeps_click_behavior);
    EGUI_TEST_RUN(test_skeleton_read_only_and_disabled_guards_clear_pressed_and_timer);
    EGUI_TEST_RUN(test_skeleton_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
