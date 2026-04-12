#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_animated_icon.h"

#include "../../HelloCustomWidgets/display/animated_icon/egui_view_animated_icon.h"
#include "../../HelloCustomWidgets/display/animated_icon/egui_view_animated_icon.c"

static egui_view_animated_icon_t test_animated_icon;
static egui_view_animated_icon_t preview_animated_icon;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void setup_animated_icon(void)
{
    egui_view_animated_icon_init(EGUI_VIEW_OF(&test_animated_icon));
    egui_view_set_size(EGUI_VIEW_OF(&test_animated_icon), 40, 40);
    g_click_count = 0;
}

static void setup_preview_animated_icon(void)
{
    egui_view_animated_icon_init(EGUI_VIEW_OF(&preview_animated_icon));
    egui_view_set_size(EGUI_VIEW_OF(&preview_animated_icon), 24, 24);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_animated_icon), on_preview_click);
    egui_view_animated_icon_set_source(EGUI_VIEW_OF(&preview_animated_icon), egui_view_animated_icon_get_chevron_down_small_source());
    egui_view_animated_icon_set_state(EGUI_VIEW_OF(&preview_animated_icon), "Pressed");
    egui_view_animated_icon_apply_subtle_style(EGUI_VIEW_OF(&preview_animated_icon));
    egui_view_animated_icon_override_static_preview_api(EGUI_VIEW_OF(&preview_animated_icon), &preview_api);
    g_click_count = 0;
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

static int send_touch(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int send_key(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->on_key_event(view, &event);
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void test_animated_icon_init_uses_back_source_normal_state_and_palette(void)
{
    setup_animated_icon();

    EGUI_TEST_ASSERT_TRUE(egui_view_animated_icon_get_source(EGUI_VIEW_OF(&test_animated_icon)) == egui_view_animated_icon_get_back_source());
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_animated_icon_get_state(EGUI_VIEW_OF(&test_animated_icon)), "Normal") == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_animated_icon_get_fallback_glyph(EGUI_VIEW_OF(&test_animated_icon)), EGUI_ICON_MS_ARROW_BACK) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_animated_icon.icon_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_animated_icon.current_progress);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_animated_icon_get_animation_enabled(EGUI_VIEW_OF(&test_animated_icon)));
    EGUI_TEST_ASSERT_TRUE(egui_view_animated_icon_resolve_font(&test_animated_icon, 16) == EGUI_FONT_ICON_MS_16);
    EGUI_TEST_ASSERT_TRUE(egui_view_animated_icon_resolve_font(&test_animated_icon, 22) == EGUI_FONT_ICON_MS_20);
    EGUI_TEST_ASSERT_TRUE(egui_view_animated_icon_resolve_font(&test_animated_icon, 28) == EGUI_FONT_ICON_MS_24);
}

static void test_animated_icon_setters_and_state_resolution_clear_pressed_state(void)
{
    setup_animated_icon();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_animated_icon), 1);
    egui_view_animated_icon_apply_subtle_style(EGUI_VIEW_OF(&test_animated_icon));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_animated_icon)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x6F7C8A).full, test_animated_icon.icon_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_animated_icon), 1);
    egui_view_animated_icon_set_source(EGUI_VIEW_OF(&test_animated_icon), egui_view_animated_icon_get_chevron_down_small_source());
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_animated_icon)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_animated_icon_get_source(EGUI_VIEW_OF(&test_animated_icon)) == egui_view_animated_icon_get_chevron_down_small_source());
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_animated_icon_get_fallback_glyph(EGUI_VIEW_OF(&test_animated_icon)), EGUI_ICON_MS_EXPAND_MORE) == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_animated_icon), 1);
    egui_view_animated_icon_set_state(EGUI_VIEW_OF(&test_animated_icon), "PointerOver");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_animated_icon)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_animated_icon_get_state(EGUI_VIEW_OF(&test_animated_icon)), "PointerOver") == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANIMATED_ICON_PROGRESS_POINTER_2, test_animated_icon.current_progress);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_animated_icon), 1);
    egui_view_animated_icon_set_fallback_glyph(EGUI_VIEW_OF(&test_animated_icon), EGUI_ICON_MS_SETTINGS);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_animated_icon)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_animated_icon_get_fallback_glyph(EGUI_VIEW_OF(&test_animated_icon)), EGUI_ICON_MS_SETTINGS) == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_animated_icon), 1);
    egui_view_animated_icon_set_source(EGUI_VIEW_OF(&test_animated_icon), egui_view_animated_icon_get_back_source());
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_animated_icon)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_animated_icon_get_fallback_glyph(EGUI_VIEW_OF(&test_animated_icon)), EGUI_ICON_MS_SETTINGS) == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_animated_icon), 1);
    egui_view_animated_icon_set_fallback_glyph(EGUI_VIEW_OF(&test_animated_icon), NULL);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_animated_icon)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_animated_icon_get_fallback_glyph(EGUI_VIEW_OF(&test_animated_icon)), EGUI_ICON_MS_ARROW_BACK) == 0);

    egui_view_animated_icon_set_icon_font(EGUI_VIEW_OF(&test_animated_icon), EGUI_FONT_ICON_MS_16);
    EGUI_TEST_ASSERT_TRUE(egui_view_animated_icon_resolve_font(&test_animated_icon, 28) == EGUI_FONT_ICON_MS_16);
    egui_view_animated_icon_set_icon_font(EGUI_VIEW_OF(&test_animated_icon), NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_animated_icon_resolve_font(&test_animated_icon, 28) == EGUI_FONT_ICON_MS_24);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_animated_icon), 1);
    egui_view_animated_icon_set_state(EGUI_VIEW_OF(&test_animated_icon), "UnknownState");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_animated_icon)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_animated_icon_get_state(EGUI_VIEW_OF(&test_animated_icon)), "Normal") == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_animated_icon.current_progress);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_animated_icon), 1);
    egui_view_animated_icon_set_animation_enabled(EGUI_VIEW_OF(&test_animated_icon), 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_animated_icon)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_animated_icon_get_animation_enabled(EGUI_VIEW_OF(&test_animated_icon)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_animated_icon), 1);
    egui_view_animated_icon_set_source(EGUI_VIEW_OF(&test_animated_icon), NULL);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_animated_icon)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_animated_icon_get_source(EGUI_VIEW_OF(&test_animated_icon)) == NULL);
}

static void test_animated_icon_attached_transition_starts_timer_and_disable_hard_cuts(void)
{
    setup_animated_icon();

    egui_view_dispatch_attach_to_window(EGUI_VIEW_OF(&test_animated_icon));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_animated_icon)->is_attached_to_window);

    egui_view_animated_icon_set_state(EGUI_VIEW_OF(&test_animated_icon), "NormalToPressed");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_animated_icon_get_state(EGUI_VIEW_OF(&test_animated_icon)), "Pressed") == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_animated_icon.from_progress);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANIMATED_ICON_PROGRESS_MAX, test_animated_icon.to_progress);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_animated_icon.timer_started);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_animated_icon.current_progress);

    egui_view_animated_icon_tick(&test_animated_icon.anim_timer);
    EGUI_TEST_ASSERT_TRUE(test_animated_icon.current_progress > 0);

    egui_view_animated_icon_set_animation_enabled(EGUI_VIEW_OF(&test_animated_icon), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_animated_icon.timer_started);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANIMATED_ICON_PROGRESS_MAX, test_animated_icon.current_progress);

    egui_view_animated_icon_set_animation_enabled(EGUI_VIEW_OF(&test_animated_icon), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_animated_icon_get_animation_enabled(EGUI_VIEW_OF(&test_animated_icon)));

    egui_view_dispatch_detach_from_window(EGUI_VIEW_OF(&test_animated_icon));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_animated_icon)->is_attached_to_window);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_animated_icon.timer_started);
}

static void test_animated_icon_static_preview_consumes_input(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_animated_icon();
    layout_view(EGUI_VIEW_OF(&preview_animated_icon), 12, 18, 24, 24);
    get_view_center(EGUI_VIEW_OF(&preview_animated_icon), &center_x, &center_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_animated_icon), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_animated_icon), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_animated_icon), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_animated_icon)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_animated_icon), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_animated_icon), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_animated_icon), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_animated_icon)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_animated_icon_run(void)
{
    EGUI_TEST_SUITE_BEGIN(animated_icon);
    EGUI_TEST_RUN(test_animated_icon_init_uses_back_source_normal_state_and_palette);
    EGUI_TEST_RUN(test_animated_icon_setters_and_state_resolution_clear_pressed_state);
    EGUI_TEST_RUN(test_animated_icon_attached_transition_starts_timer_and_disable_hard_cuts);
    EGUI_TEST_RUN(test_animated_icon_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
