#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_toggle_button.h"

#include "../../HelloCustomWidgets/input/toggle_button/egui_view_toggle_button.h"
#include "../../HelloCustomWidgets/input/toggle_button/egui_view_toggle_button.c"

typedef struct
{
    egui_region_t region_screen;
    egui_view_on_toggled_listener_t on_toggled;
    uint8_t is_toggled;
    const char *icon;
    const char *text;
    const egui_font_t *font;
    const egui_font_t *icon_font;
    egui_color_t text_color;
    egui_color_t on_color;
    egui_color_t off_color;
    egui_dim_t corner_radius;
    egui_dim_t icon_text_gap;
    egui_alpha_t alpha;
} toggle_button_preview_snapshot_t;

static egui_view_toggle_button_t test_button;
static egui_view_toggle_button_t preview_button;
static egui_view_api_t preview_api;
static int toggled_count;
static uint8_t last_toggled;

static void on_toggled(egui_view_t *self, uint8_t is_toggled)
{
    EGUI_UNUSED(self);
    toggled_count++;
    last_toggled = is_toggled;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_button(uint8_t is_toggled)
{
    static egui_view_api_t test_button_interaction_api;

    egui_view_toggle_button_init(EGUI_VIEW_OF(&test_button));
    egui_view_set_size(EGUI_VIEW_OF(&test_button), 116, 44);
    egui_view_toggle_button_set_text(EGUI_VIEW_OF(&test_button), "Alerts");
    egui_view_toggle_button_set_icon(EGUI_VIEW_OF(&test_button), EGUI_ICON_MS_NOTIFICATIONS);
    egui_view_toggle_button_set_icon_font(EGUI_VIEW_OF(&test_button), EGUI_FONT_ICON_MS_16);
    hcw_toggle_button_apply_standard_style(EGUI_VIEW_OF(&test_button));
    egui_view_toggle_button_set_on_toggled_listener(EGUI_VIEW_OF(&test_button), on_toggled);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&test_button), 1);
#endif
    hcw_toggle_button_override_interaction_api(EGUI_VIEW_OF(&test_button), &test_button_interaction_api);
    hcw_toggle_button_set_toggled(EGUI_VIEW_OF(&test_button), is_toggled);
    toggled_count = 0;
    last_toggled = is_toggled;
}

static void setup_preview_button(void)
{
    egui_view_toggle_button_init(EGUI_VIEW_OF(&preview_button));
    egui_view_set_size(EGUI_VIEW_OF(&preview_button), 104, 44);
    egui_view_toggle_button_set_text(EGUI_VIEW_OF(&preview_button), "Visible");
    egui_view_toggle_button_set_icon(EGUI_VIEW_OF(&preview_button), EGUI_ICON_MS_VISIBILITY);
    egui_view_toggle_button_set_font(EGUI_VIEW_OF(&preview_button), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    egui_view_toggle_button_set_icon_font(EGUI_VIEW_OF(&preview_button), EGUI_FONT_ICON_MS_16);
    hcw_toggle_button_apply_compact_style(EGUI_VIEW_OF(&preview_button));
    egui_view_toggle_button_set_on_toggled_listener(EGUI_VIEW_OF(&preview_button), on_toggled);
    hcw_toggle_button_set_toggled(EGUI_VIEW_OF(&preview_button), 1);
    hcw_toggle_button_override_static_preview_api(EGUI_VIEW_OF(&preview_button), &preview_api);
    toggled_count = 0;
    last_toggled = 1;
}

static void capture_preview_snapshot(toggle_button_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_button)->region_screen;
    snapshot->on_toggled = preview_button.on_toggled;
    snapshot->is_toggled = preview_button.is_toggled;
    snapshot->icon = preview_button.icon;
    snapshot->text = preview_button.text;
    snapshot->font = preview_button.font;
    snapshot->icon_font = preview_button.icon_font;
    snapshot->text_color = preview_button.text_color;
    snapshot->on_color = preview_button.on_color;
    snapshot->off_color = preview_button.off_color;
    snapshot->corner_radius = preview_button.corner_radius;
    snapshot->icon_text_gap = preview_button.icon_text_gap;
    snapshot->alpha = EGUI_VIEW_OF(&preview_button)->alpha;
}

static void assert_preview_state_unchanged(const toggle_button_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_button)->region_screen);
    EGUI_TEST_ASSERT_TRUE(preview_button.on_toggled == snapshot->on_toggled);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_toggled, preview_button.is_toggled);
    EGUI_TEST_ASSERT_TRUE(preview_button.icon == snapshot->icon);
    EGUI_TEST_ASSERT_TRUE(preview_button.text == snapshot->text);
    EGUI_TEST_ASSERT_TRUE(preview_button.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_button.icon_font == snapshot->icon_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_button.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->on_color.full, preview_button.on_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->off_color.full, preview_button.off_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->corner_radius, preview_button.corner_radius);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->icon_text_gap, preview_button.icon_text_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_button)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(0, toggled_count);
    EGUI_TEST_ASSERT_FALSE(egui_view_get_pressed(EGUI_VIEW_OF(&preview_button)));
}

static void layout_button(void)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = 116;
    region.size.height = 44;
    egui_view_layout(EGUI_VIEW_OF(&test_button), &region);
    egui_region_copy(&EGUI_VIEW_OF(&test_button)->region_screen, &region);
}

static void layout_preview_button(void)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = 104;
    region.size.height = 44;
    egui_view_layout(EGUI_VIEW_OF(&preview_button), &region);
    egui_region_copy(&EGUI_VIEW_OF(&preview_button)->region_screen, &region);
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

static int send_touch_at(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_button), type, x, y);
}

static int send_touch(uint8_t type)
{
    return send_touch_at(type, 40, 36);
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

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_button), key_code);
}

static int send_preview_touch(uint8_t type)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_button), type, 40, 36);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_button), key_code);
}

static void test_toggle_button_touch_toggles_state(void)
{
    setup_button(0);
    layout_button();
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP));
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_button_is_toggled(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, toggled_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_toggled);
    EGUI_TEST_ASSERT_FALSE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
}

static void test_toggle_button_enter_and_space_toggle_state(void)
{
    setup_button(0);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_button_is_toggled(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, toggled_count);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(egui_view_toggle_button_is_toggled(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, toggled_count);
}

static void test_toggle_button_disabled_ignores_input(void)
{
    setup_button(1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_button), 0);
    layout_button();
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_button_is_toggled(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, toggled_count);
}

static void test_toggle_button_setters_and_style_helpers_clear_pressed_state(void)
{
    setup_button(1);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), 1);
    hcw_toggle_button_set_toggled(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_button_is_toggled(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, toggled_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), 1);
    hcw_toggle_button_apply_compact_style(EGUI_VIEW_OF(&test_button));
    EGUI_TEST_ASSERT_FALSE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), 1);
    hcw_toggle_button_apply_read_only_style(EGUI_VIEW_OF(&test_button));
    EGUI_TEST_ASSERT_FALSE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
}

static void test_toggle_button_style_helpers_update_palette(void)
{
    setup_button(0);
    EGUI_TEST_ASSERT_EQUAL_INT(10, test_button.corner_radius);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x2563EB).full, test_button.on_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xEAF1FB).full, test_button.off_color.full);

    hcw_toggle_button_apply_compact_style(EGUI_VIEW_OF(&test_button));
    EGUI_TEST_ASSERT_EQUAL_INT(7, test_button.corner_radius);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0C7C73).full, test_button.on_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xDBEAE5).full, test_button.off_color.full);

    hcw_toggle_button_apply_read_only_style(EGUI_VIEW_OF(&test_button));
    EGUI_TEST_ASSERT_EQUAL_INT(7, test_button.corner_radius);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xAFB8C3).full, test_button.on_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xF3F6F8).full, test_button.off_color.full);
}

static void test_toggle_button_touch_cancel_and_disabled_clear_pressed_state(void)
{
    setup_button(0);
    layout_button();

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_TRUE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL));
    EGUI_TEST_ASSERT_FALSE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_TRUE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
    egui_view_set_enable(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP));
    EGUI_TEST_ASSERT_FALSE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_FALSE(egui_view_toggle_button_is_toggled(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, toggled_count);
}

static void test_toggle_button_same_target_release_requires_return_to_origin(void)
{
    egui_dim_t inside_x = 40;
    egui_dim_t inside_y = 36;
    egui_dim_t outside_x = 4;
    egui_dim_t outside_y = 4;

    setup_button(0);
    layout_button();

    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_FALSE(send_touch_at(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(egui_view_toggle_button_is_toggled(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, toggled_count);

    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_button_is_toggled(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, toggled_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_toggled);
}

static void test_toggle_button_unhandled_key_does_not_toggle(void)
{
    setup_button(0);
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_FALSE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_FALSE(egui_view_toggle_button_is_toggled(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, toggled_count);
}

static void test_toggle_button_disabled_key_event_clears_pressed_state(void)
{
    setup_button(0);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(egui_view_get_pressed(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_FALSE(egui_view_toggle_button_is_toggled(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, toggled_count);
}

static void test_toggle_button_static_preview_consumes_input_and_keeps_state(void)
{
    toggle_button_preview_snapshot_t initial_snapshot;

    setup_preview_button();
    layout_preview_button();
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_button), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN));
    assert_preview_state_unchanged(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_button), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_toggle_button_run(void)
{
    EGUI_TEST_SUITE_BEGIN(toggle_button);
    EGUI_TEST_RUN(test_toggle_button_touch_toggles_state);
    EGUI_TEST_RUN(test_toggle_button_enter_and_space_toggle_state);
    EGUI_TEST_RUN(test_toggle_button_disabled_ignores_input);
    EGUI_TEST_RUN(test_toggle_button_setters_and_style_helpers_clear_pressed_state);
    EGUI_TEST_RUN(test_toggle_button_style_helpers_update_palette);
    EGUI_TEST_RUN(test_toggle_button_touch_cancel_and_disabled_clear_pressed_state);
    EGUI_TEST_RUN(test_toggle_button_same_target_release_requires_return_to_origin);
    EGUI_TEST_RUN(test_toggle_button_unhandled_key_does_not_toggle);
    EGUI_TEST_RUN(test_toggle_button_disabled_key_event_clears_pressed_state);
    EGUI_TEST_RUN(test_toggle_button_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
