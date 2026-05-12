#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_compound_button.h"

#include "../../HelloCustomWidgets/input/compound_button/egui_view_compound_button.h"
#include "../../HelloCustomWidgets/input/compound_button/egui_view_compound_button.c"

typedef struct compound_button_preview_snapshot compound_button_preview_snapshot_t;
struct compound_button_preview_snapshot
{
    egui_region_t region_screen;
    const char *title;
    const char *subtitle;
    const char *icon;
    const egui_font_t *title_font;
    const egui_font_t *subtitle_font;
    const egui_font_t *icon_font;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t pressed_color;
    egui_color_t border_color;
    egui_color_t focus_color;
    egui_color_t title_color;
    egui_color_t subtitle_color;
    egui_color_t icon_color;
    uint8_t style;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_target;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
};

static egui_view_compound_button_t test_widget;
static egui_view_compound_button_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_action_count;

static void on_action(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_action_count++;
}

static void reset_action_state(void)
{
    g_action_count = 0;
}

static void setup_widget(void)
{
    egui_view_compound_button_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 180, 58);
    egui_view_compound_button_set_content(EGUI_VIEW_OF(&test_widget), "Create workspace", "Start a shared review space.", EGUI_ICON_MS_ARROW_FORWARD);
    egui_view_compound_button_set_fonts(EGUI_VIEW_OF(&test_widget), NULL, NULL, NULL);
    egui_view_compound_button_set_on_action_listener(EGUI_VIEW_OF(&test_widget), on_action);
    reset_action_state();
}

static void setup_preview_widget(void)
{
    egui_view_compound_button_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 104, 58);
    egui_view_compound_button_set_content(EGUI_VIEW_OF(&preview_widget), "Compact", "Short helper text.", EGUI_ICON_MS_ARROW_FORWARD);
    egui_view_compound_button_set_fonts(EGUI_VIEW_OF(&preview_widget), NULL, NULL, NULL);
    egui_view_compound_button_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_compound_button_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
    reset_action_state();
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
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 20, 180, 58);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 104, 58);
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

static int send_key_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_widget), type, x, y);
}

static int send_key_action(uint8_t type, uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_widget), type, key_code);
}

static int send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static uint8_t get_button_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_compound_button_get_button_region(view, &region))
    {
        return 0;
    }
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void get_view_outside_point(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x - 5;
    *y = view->region_screen.location.y - 5;
}

static void seed_pressed_state(egui_view_compound_button_t *widget, uint8_t visual_pressed)
{
    widget->pressed_target = EGUI_VIEW_COMPOUND_BUTTON_TARGET_BODY;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void assert_pressed_cleared(egui_view_compound_button_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMPOUND_BUTTON_TARGET_NONE, widget->pressed_target);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void capture_preview_snapshot(compound_button_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_widget)->region_screen;
    snapshot->title = preview_widget.title;
    snapshot->subtitle = preview_widget.subtitle;
    snapshot->icon = preview_widget.icon;
    snapshot->title_font = preview_widget.title_font;
    snapshot->subtitle_font = preview_widget.subtitle_font;
    snapshot->icon_font = preview_widget.icon_font;
    snapshot->api = EGUI_VIEW_OF(&preview_widget)->api;
    snapshot->surface_color = preview_widget.surface_color;
    snapshot->pressed_color = preview_widget.pressed_color;
    snapshot->border_color = preview_widget.border_color;
    snapshot->focus_color = preview_widget.focus_color;
    snapshot->title_color = preview_widget.title_color;
    snapshot->subtitle_color = preview_widget.subtitle_color;
    snapshot->icon_color = preview_widget.icon_color;
    snapshot->style = preview_widget.style;
    snapshot->compact_mode = preview_widget.compact_mode;
    snapshot->read_only_mode = preview_widget.read_only_mode;
    snapshot->pressed_target = preview_widget.pressed_target;
    snapshot->alpha = EGUI_VIEW_OF(&preview_widget)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_widget));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_widget)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_widget)->is_focused;
}

static void assert_preview_state_unchanged(const compound_button_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_widget)->region_screen);
    EGUI_TEST_ASSERT_TRUE(snapshot->title == preview_widget.title);
    EGUI_TEST_ASSERT_TRUE(snapshot->subtitle == preview_widget.subtitle);
    EGUI_TEST_ASSERT_TRUE(snapshot->icon == preview_widget.icon);
    EGUI_TEST_ASSERT_TRUE(snapshot->title_font == preview_widget.title_font);
    EGUI_TEST_ASSERT_TRUE(snapshot->subtitle_font == preview_widget.subtitle_font);
    EGUI_TEST_ASSERT_TRUE(snapshot->icon_font == preview_widget.icon_font);
    EGUI_TEST_ASSERT_TRUE(snapshot->api == EGUI_VIEW_OF(&preview_widget)->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_color.full, preview_widget.pressed_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->focus_color.full, preview_widget.focus_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->title_color.full, preview_widget.title_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->subtitle_color.full, preview_widget.subtitle_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->icon_color.full, preview_widget.icon_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->style, preview_widget.style);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_target, preview_widget.pressed_target);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_widget)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_widget)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_compound_button_internal_helpers_cover_text_fitting(void)
{
    char label[28];

    setup_widget();

    EGUI_TEST_ASSERT_FALSE(egui_view_compound_button_has_text(NULL));
    EGUI_TEST_ASSERT_TRUE(egui_view_compound_button_has_text("Ready"));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_compound_button_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_compound_button_text_len("Review"));
    EGUI_TEST_ASSERT_TRUE(egui_view_compound_button_is_space_char(' '));
    EGUI_TEST_ASSERT_FALSE(egui_view_compound_button_is_space_char('x'));
    EGUI_TEST_ASSERT_TRUE(egui_view_compound_button_is_break_after_char('-'));
    EGUI_TEST_ASSERT_TRUE(egui_view_compound_button_is_break_after_char('/'));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_compound_button_find_elide_boundary("Open latest release notes.", 7));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_compound_button_find_elide_boundary("scan-first review", 5));
    egui_view_compound_button_copy_elided(label, sizeof(label), "Publish", 6);
    EGUI_TEST_ASSERT_TRUE(strcmp("Pub...", label) == 0);
    egui_view_compound_button_copy_elided(label, sizeof(label), "Mode", 3);
    EGUI_TEST_ASSERT_TRUE(strcmp("...", label) == 0);
}

static void test_compound_button_setters_clear_pressed_and_update_state(void)
{
    setup_widget();

    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PANEL.full, test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_SURFACE_PRESS.full, test_widget.pressed_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_BORDER_STRONG.full, test_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_DARK.full, test_widget.focus_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_STRONG.full, test_widget.title_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_SOFT.full, test_widget.subtitle_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_DARK.full, test_widget.icon_color.full);

    seed_pressed_state(&test_widget, 1);
    egui_view_compound_button_set_title(EGUI_VIEW_OF(&test_widget), "Sync changes");
    EGUI_TEST_ASSERT_TRUE(strcmp("Sync changes", test_widget.title) == 0);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1);
    egui_view_compound_button_set_subtitle(EGUI_VIEW_OF(&test_widget), "Refresh current policy.");
    EGUI_TEST_ASSERT_TRUE(strcmp("Refresh current policy.", test_widget.subtitle) == 0);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1);
    egui_view_compound_button_set_icon(EGUI_VIEW_OF(&test_widget), EGUI_ICON_MS_SYNC);
    EGUI_TEST_ASSERT_TRUE(strcmp(test_widget.icon, EGUI_ICON_MS_SYNC) == 0);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1);
    egui_view_compound_button_set_content(EGUI_VIEW_OF(&test_widget), "Approve access", "Confirm reviewers.", EGUI_ICON_MS_DONE);
    EGUI_TEST_ASSERT_TRUE(strcmp("Approve access", test_widget.title) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("Confirm reviewers.", test_widget.subtitle) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp(test_widget.icon, EGUI_ICON_MS_DONE) == 0);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1);
    egui_view_compound_button_set_fonts(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4,
                                        (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_FONT_ICON_MS_16);
    EGUI_TEST_ASSERT_TRUE(test_widget.title_font == (const egui_font_t *)&egui_res_font_montserrat_10_4);
    EGUI_TEST_ASSERT_TRUE(test_widget.subtitle_font == (const egui_font_t *)&egui_res_font_montserrat_8_4);
    EGUI_TEST_ASSERT_TRUE(test_widget.icon_font == EGUI_FONT_ICON_MS_16);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1);
    egui_view_compound_button_set_style(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_COMPOUND_BUTTON_STYLE_PRIMARY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMPOUND_BUTTON_STYLE_PRIMARY, test_widget.style);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_DARK.full, test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_DARK.full, test_widget.pressed_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_DARK.full, test_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_LIGHT.full, test_widget.focus_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_WHITE.full, test_widget.title_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_SURFACE.full, test_widget.subtitle_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_WHITE.full, test_widget.icon_color.full);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1);
    egui_view_compound_button_set_compact_mode(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.compact_mode);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1);
    egui_view_compound_button_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.read_only_mode);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1);
    egui_view_compound_button_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122),
                                          EGUI_COLOR_HEX(0x303132), EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152),
                                          EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_widget.pressed_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_widget.focus_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_widget.title_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, test_widget.subtitle_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x707172).full, test_widget.icon_color.full);
    assert_pressed_cleared(&test_widget);
}

static void test_compound_button_activate_listener(void)
{
    setup_widget();

    EGUI_TEST_ASSERT_TRUE(egui_view_compound_button_activate(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);

    egui_view_compound_button_set_on_action_listener(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_compound_button_activate(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
}

static void test_compound_button_touch_same_target_release_and_cancel(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_widget();
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_button_center(EGUI_VIEW_OF(&test_widget), &inside_x, &inside_y));
    get_view_outside_point(EGUI_VIEW_OF(&test_widget), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMPOUND_BUTTON_TARGET_BODY, test_widget.pressed_target);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMPOUND_BUTTON_TARGET_BODY, test_widget.pressed_target);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    assert_pressed_cleared(&test_widget);

    reset_action_state();
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_widget);
}

static void test_compound_button_keyboard_activate_and_unhandled_key(void)
{
    setup_widget();

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMPOUND_BUTTON_TARGET_BODY, test_widget.pressed_target);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_action_count);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_TAB));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_action_count);
}

static void test_compound_button_read_only_and_disabled_guards(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;

    setup_widget();
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_button_center(EGUI_VIEW_OF(&test_widget), &inside_x, &inside_y));

    egui_view_compound_button_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    seed_pressed_state(&test_widget, 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_compound_button_activate(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    egui_view_compound_button_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);
    seed_pressed_state(&test_widget, 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_compound_button_activate(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_SPACE));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_compound_button_static_preview_consumes_input_and_keeps_state(void)
{
    compound_button_preview_snapshot_t initial_snapshot;
    egui_dim_t inside_x;
    egui_dim_t inside_y;

    setup_preview_widget();
    layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(get_button_center(EGUI_VIEW_OF(&preview_widget), &inside_x, &inside_y));
    capture_preview_snapshot(&initial_snapshot);

    seed_pressed_state(&preview_widget, 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    assert_preview_state_unchanged(&initial_snapshot);

    seed_pressed_state(&preview_widget, 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_compound_button_run(void)
{
    EGUI_TEST_SUITE_BEGIN(compound_button);
    EGUI_TEST_RUN(test_compound_button_internal_helpers_cover_text_fitting);
    EGUI_TEST_RUN(test_compound_button_setters_clear_pressed_and_update_state);
    EGUI_TEST_RUN(test_compound_button_activate_listener);
    EGUI_TEST_RUN(test_compound_button_touch_same_target_release_and_cancel);
    EGUI_TEST_RUN(test_compound_button_keyboard_activate_and_unhandled_key);
    EGUI_TEST_RUN(test_compound_button_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_compound_button_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
