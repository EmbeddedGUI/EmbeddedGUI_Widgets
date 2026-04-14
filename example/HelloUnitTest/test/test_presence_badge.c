#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_presence_badge.h"

#include "../../HelloCustomWidgets/display/presence_badge/egui_view_presence_badge.h"
#include "../../HelloCustomWidgets/display/presence_badge/egui_view_presence_badge.c"

static egui_view_presence_badge_t test_badge_widget;
static egui_view_presence_badge_t preview_badge_widget;
static egui_view_api_t preview_api;

static void layout_badge(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static void setup_presence_badge(void)
{
    egui_view_presence_badge_init(EGUI_VIEW_OF(&test_badge_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_badge_widget), 24, 24);
}

static void setup_preview_badge(void)
{
    egui_view_presence_badge_init(EGUI_VIEW_OF(&preview_badge_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_badge_widget), 18, 18);
    egui_view_presence_badge_set_status(EGUI_VIEW_OF(&preview_badge_widget), EGUI_VIEW_PRESENCE_BADGE_STATUS_AWAY);
    egui_view_presence_badge_set_compact_mode(EGUI_VIEW_OF(&preview_badge_widget), 1);
    egui_view_presence_badge_override_static_preview_api(EGUI_VIEW_OF(&preview_badge_widget), &preview_api);
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

static int send_key(egui_view_t *view, uint8_t key_code)
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

static void test_presence_badge_init_uses_defaults(void)
{
    setup_presence_badge();

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PRESENCE_BADGE_STATUS_AVAILABLE, egui_view_presence_badge_get_status(EGUI_VIEW_OF(&test_badge_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_presence_badge_get_compact_mode(EGUI_VIEW_OF(&test_badge_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_presence_badge_get_read_only_mode(EGUI_VIEW_OF(&test_badge_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xFFFFFF).full, test_badge_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xD5DEE6).full, test_badge_widget.outline_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x107C41).full, test_badge_widget.available_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xC4314B).full, test_badge_widget.busy_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x7A8796).full, test_badge_widget.offline_color.full);
}

static void test_presence_badge_setters_clear_pressed_state_and_update_palette(void)
{
    egui_color_t surface = EGUI_COLOR_HEX(0x101112);
    egui_color_t outline = EGUI_COLOR_HEX(0x202122);
    egui_color_t available = EGUI_COLOR_HEX(0x303132);
    egui_color_t busy = EGUI_COLOR_HEX(0x404142);
    egui_color_t away = EGUI_COLOR_HEX(0x505152);
    egui_color_t dnd = EGUI_COLOR_HEX(0x606162);
    egui_color_t offline = EGUI_COLOR_HEX(0x707172);
    egui_color_t glyph = EGUI_COLOR_HEX(0x808182);
    egui_color_t muted = EGUI_COLOR_HEX(0x909192);

    setup_presence_badge();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_presence_badge_set_status(EGUI_VIEW_OF(&test_badge_widget), EGUI_VIEW_PRESENCE_BADGE_STATUS_DO_NOT_DISTURB);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PRESENCE_BADGE_STATUS_DO_NOT_DISTURB, egui_view_presence_badge_get_status(EGUI_VIEW_OF(&test_badge_widget)));

    egui_view_presence_badge_set_status(EGUI_VIEW_OF(&test_badge_widget), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PRESENCE_BADGE_STATUS_AVAILABLE, egui_view_presence_badge_get_status(EGUI_VIEW_OF(&test_badge_widget)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_presence_badge_set_palette(EGUI_VIEW_OF(&test_badge_widget), surface, outline, available, busy, away, dnd, offline, glyph, muted);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(surface.full, test_badge_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(outline.full, test_badge_widget.outline_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(available.full, test_badge_widget.available_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(busy.full, test_badge_widget.busy_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(away.full, test_badge_widget.away_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(dnd.full, test_badge_widget.do_not_disturb_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(offline.full, test_badge_widget.offline_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(glyph.full, test_badge_widget.glyph_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(muted.full, test_badge_widget.muted_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_presence_badge_set_compact_mode(EGUI_VIEW_OF(&test_badge_widget), 2);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_presence_badge_get_compact_mode(EGUI_VIEW_OF(&test_badge_widget)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_presence_badge_set_read_only_mode(EGUI_VIEW_OF(&test_badge_widget), 3);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_presence_badge_get_read_only_mode(EGUI_VIEW_OF(&test_badge_widget)));
}

static void test_presence_badge_helpers_compute_regions_and_status_colors(void)
{
    egui_region_t region;
    egui_dim_t standard_width;
    egui_dim_t compact_width;
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);

    setup_presence_badge();
    layout_badge(EGUI_VIEW_OF(&test_badge_widget), 10, 20, 24, 24);

    EGUI_TEST_ASSERT_TRUE(egui_view_presence_badge_get_indicator_region(EGUI_VIEW_OF(&test_badge_widget), &region));
    EGUI_TEST_ASSERT_EQUAL_INT(12, region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(22, region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(20, region.size.width);
    standard_width = region.size.width;

    EGUI_TEST_ASSERT_EQUAL_INT(test_badge_widget.available_color.full,
                               egui_view_presence_badge_resolve_status_color(&test_badge_widget, EGUI_VIEW_PRESENCE_BADGE_STATUS_AVAILABLE).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_badge_widget.busy_color.full,
                               egui_view_presence_badge_resolve_status_color(&test_badge_widget, EGUI_VIEW_PRESENCE_BADGE_STATUS_BUSY).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_badge_widget.away_color.full,
                               egui_view_presence_badge_resolve_status_color(&test_badge_widget, EGUI_VIEW_PRESENCE_BADGE_STATUS_AWAY).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_badge_widget.do_not_disturb_color.full,
                               egui_view_presence_badge_resolve_status_color(&test_badge_widget, EGUI_VIEW_PRESENCE_BADGE_STATUS_DO_NOT_DISTURB).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_badge_widget.offline_color.full,
                               egui_view_presence_badge_resolve_status_color(&test_badge_widget, EGUI_VIEW_PRESENCE_BADGE_STATUS_OFFLINE).full);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_HEX(0x83909D), 54).full, egui_view_presence_badge_mix_disabled(sample).full);

    egui_view_presence_badge_set_compact_mode(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_set_size(EGUI_VIEW_OF(&test_badge_widget), 18, 18);
    layout_badge(EGUI_VIEW_OF(&test_badge_widget), 4, 6, 18, 18);
    EGUI_TEST_ASSERT_TRUE(egui_view_presence_badge_get_indicator_region(EGUI_VIEW_OF(&test_badge_widget), &region));
    compact_width = region.size.width;
    EGUI_TEST_ASSERT_TRUE(compact_width < standard_width);
}

static void test_presence_badge_static_preview_consumes_input_and_clears_pressed_state(void)
{
    setup_preview_badge();
    layout_badge(EGUI_VIEW_OF(&preview_badge_widget), 12, 18, 18, 18);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_badge_widget), EGUI_MOTION_EVENT_ACTION_DOWN, 20, 24));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_badge_widget)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_badge_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_presence_badge_get_compact_mode(EGUI_VIEW_OF(&preview_badge_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PRESENCE_BADGE_STATUS_AWAY, egui_view_presence_badge_get_status(EGUI_VIEW_OF(&preview_badge_widget)));
}

void test_presence_badge_run(void)
{
    EGUI_TEST_SUITE_BEGIN(presence_badge);
    EGUI_TEST_RUN(test_presence_badge_init_uses_defaults);
    EGUI_TEST_RUN(test_presence_badge_setters_clear_pressed_state_and_update_palette);
    EGUI_TEST_RUN(test_presence_badge_helpers_compute_regions_and_status_colors);
    EGUI_TEST_RUN(test_presence_badge_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
