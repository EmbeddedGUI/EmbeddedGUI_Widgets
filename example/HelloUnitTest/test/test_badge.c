#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_badge.h"

#include "../../HelloCustomWidgets/display/badge/egui_view_badge.h"
#include "../../HelloCustomWidgets/display/badge/egui_view_badge.c"
#include "../../../sdk/EmbeddedGUI/src/resource/egui_icon_material_symbols.h"

static egui_view_badge_t test_badge_widget;
static egui_view_badge_t preview_badge_widget;
static egui_view_api_t preview_api;

static void setup_badge(void)
{
    egui_view_badge_init(EGUI_VIEW_OF(&test_badge_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_badge_widget), 128, 28);
    egui_view_badge_apply_filled_style(EGUI_VIEW_OF(&test_badge_widget));
    egui_view_badge_set_text(EGUI_VIEW_OF(&test_badge_widget), "Verified");
    egui_view_badge_set_icon(EGUI_VIEW_OF(&test_badge_widget), EGUI_ICON_MS_DONE);
    egui_view_badge_set_font(EGUI_VIEW_OF(&test_badge_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_badge_set_icon_font(EGUI_VIEW_OF(&test_badge_widget), EGUI_FONT_ICON_MS_16);
}

static void setup_preview_badge(void)
{
    egui_view_badge_init(EGUI_VIEW_OF(&preview_badge_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_badge_widget), 96, 24);
    egui_view_badge_apply_outline_style(EGUI_VIEW_OF(&preview_badge_widget));
    egui_view_badge_set_text(EGUI_VIEW_OF(&preview_badge_widget), "Beta");
    egui_view_badge_set_icon(EGUI_VIEW_OF(&preview_badge_widget), EGUI_ICON_MS_INFO);
    egui_view_badge_set_font(EGUI_VIEW_OF(&preview_badge_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_badge_set_icon_font(EGUI_VIEW_OF(&preview_badge_widget), EGUI_FONT_ICON_MS_16);
    egui_view_badge_override_static_preview_api(EGUI_VIEW_OF(&preview_badge_widget), &preview_api);
}

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

static int send_touch(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int send_preview_key_action(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->on_key_event(view, &event);
}

static void test_badge_style_helpers_update_modes_and_palette(void)
{
    egui_view_badge_t *local;

    setup_badge();
    local = &test_badge_widget;

    EGUI_TEST_ASSERT_FALSE(local->compact_mode);
    EGUI_TEST_ASSERT_FALSE(local->read_only_mode);
    EGUI_TEST_ASSERT_FALSE(local->outline_mode);
    EGUI_TEST_ASSERT_FALSE(local->subtle_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, local->surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_WHITE.full, local->text_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_apply_outline_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->outline_mode);
    EGUI_TEST_ASSERT_FALSE(local->subtle_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xFFFFFF).full, local->surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, local->accent_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_apply_subtle_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(local->outline_mode);
    EGUI_TEST_ASSERT_TRUE(local->subtle_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xF3F7FB).full, local->surface_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_apply_read_only_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->compact_mode);
    EGUI_TEST_ASSERT_TRUE(local->read_only_mode);
    EGUI_TEST_ASSERT_TRUE(local->subtle_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xF5F7FA).full, local->surface_color.full);
}

static void test_badge_setters_clear_pressed_state_and_update_content(void)
{
    egui_view_badge_t *local;

    setup_badge();
    local = &test_badge_widget;

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_text(EGUI_VIEW_OF(local), "Needs review");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(local->text, "Needs review"));

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_icon(EGUI_VIEW_OF(local), EGUI_ICON_MS_WARNING);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(local->icon, EGUI_ICON_MS_WARNING));

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_font(EGUI_VIEW_OF(local), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->font == (const egui_font_t *)&egui_res_font_montserrat_12_4);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_icon_font(EGUI_VIEW_OF(local), EGUI_FONT_ICON_MS_20);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->icon_font == EGUI_FONT_ICON_MS_20);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_palette(EGUI_VIEW_OF(local), EGUI_COLOR_HEX(0xF8F9FA), EGUI_COLOR_HEX(0xCCD3DA), EGUI_COLOR_HEX(0x22303F),
                                EGUI_COLOR_HEX(0x13579B));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x13579B).full, local->accent_color.full);
}

static void test_badge_icon_region_visibility_tracks_icon_and_compact_mode(void)
{
    egui_region_t icon_region;
    egui_dim_t standard_width;
    egui_dim_t compact_width;

    setup_badge();
    layout_badge(EGUI_VIEW_OF(&test_badge_widget), 12, 18, 128, 28);

    EGUI_TEST_ASSERT_TRUE(egui_view_badge_get_icon_region(EGUI_VIEW_OF(&test_badge_widget), &icon_region));
    standard_width = icon_region.size.width;

    egui_view_badge_set_compact_mode(EGUI_VIEW_OF(&test_badge_widget), 1);
    layout_badge(EGUI_VIEW_OF(&test_badge_widget), 12, 18, 96, 24);
    EGUI_TEST_ASSERT_TRUE(egui_view_badge_get_icon_region(EGUI_VIEW_OF(&test_badge_widget), &icon_region));
    compact_width = icon_region.size.width;
    EGUI_TEST_ASSERT_TRUE(compact_width < standard_width);

    egui_view_badge_set_icon(EGUI_VIEW_OF(&test_badge_widget), NULL);
    EGUI_TEST_ASSERT_FALSE(egui_view_badge_get_icon_region(EGUI_VIEW_OF(&test_badge_widget), &icon_region));
}

static void test_badge_mode_setters_clear_pressed_state(void)
{
    egui_view_badge_t *local;

    setup_badge();
    local = &test_badge_widget;

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_compact_mode(EGUI_VIEW_OF(local), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->compact_mode);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_read_only_mode(EGUI_VIEW_OF(local), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->read_only_mode);
}

static void test_badge_static_preview_consumes_input_and_clears_pressed_state(void)
{
    setup_preview_badge();
    layout_badge(EGUI_VIEW_OF(&preview_badge_widget), 12, 18, 96, 24);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_badge_widget), EGUI_MOTION_EVENT_ACTION_DOWN, 20, 24));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_badge_widget), EGUI_MOTION_EVENT_ACTION_UP, 20, 24));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_badge_widget)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(EGUI_VIEW_OF(&preview_badge_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(EGUI_VIEW_OF(&preview_badge_widget), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_badge_widget)->is_pressed);
}

void test_badge_run(void)
{
    EGUI_TEST_SUITE_BEGIN(badge);
    EGUI_TEST_RUN(test_badge_style_helpers_update_modes_and_palette);
    EGUI_TEST_RUN(test_badge_setters_clear_pressed_state_and_update_content);
    EGUI_TEST_RUN(test_badge_icon_region_visibility_tracks_icon_and_compact_mode);
    EGUI_TEST_RUN(test_badge_mode_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_badge_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
