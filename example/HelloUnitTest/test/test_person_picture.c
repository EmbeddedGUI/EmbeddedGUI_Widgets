#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_person_picture.h"

#include "../../HelloCustomWidgets/display/person_picture/egui_view_person_picture.h"
#include "../../HelloCustomWidgets/display/person_picture/egui_view_person_picture.c"

typedef struct person_picture_preview_snapshot person_picture_preview_snapshot_t;
struct person_picture_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const char *display_name;
    const char *initials;
    const char *fallback_glyph;
    const egui_image_t *image;
    const egui_font_t *font;
    const egui_font_t *icon_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t foreground_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t neutral_color;
    egui_color_t muted_color;
    egui_view_on_click_listener_t on_click_listener;
    const egui_view_api_t *api;
    egui_alpha_t alpha;
    uint8_t tone;
    uint8_t presence;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_person_picture_t test_picture;
static egui_view_person_picture_t preview_picture;
static egui_view_api_t preview_api;
static const egui_image_t g_fake_image = {0};
static uint8_t g_click_count;

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void assert_optional_string_equal(const char *expected, const char *actual)
{
    if (expected == NULL || actual == NULL)
    {
        EGUI_TEST_ASSERT_TRUE(expected == actual);
        return;
    }
    EGUI_TEST_ASSERT_TRUE(strcmp(expected, actual) == 0);
}

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
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

static void setup_person_picture(void)
{
    egui_view_person_picture_init(EGUI_VIEW_OF(&test_picture));
    egui_view_set_size(EGUI_VIEW_OF(&test_picture), 56, 56);
    g_click_count = 0;
}

static void setup_preview_picture(void)
{
    egui_view_person_picture_init(EGUI_VIEW_OF(&preview_picture));
    egui_view_set_size(EGUI_VIEW_OF(&preview_picture), 38, 38);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_picture), on_preview_click);
    egui_view_person_picture_set_display_name(EGUI_VIEW_OF(&preview_picture), "Maya Yu");
    egui_view_person_picture_set_presence(EGUI_VIEW_OF(&preview_picture), EGUI_VIEW_PERSON_PICTURE_PRESENCE_AWAY);
    egui_view_person_picture_set_tone(EGUI_VIEW_OF(&preview_picture), EGUI_VIEW_PERSON_PICTURE_TONE_WARNING);
    egui_view_person_picture_set_compact_mode(EGUI_VIEW_OF(&preview_picture), 1);
    egui_view_person_picture_override_static_preview_api(EGUI_VIEW_OF(&preview_picture), &preview_api);
    g_click_count = 0;
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

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->on_key_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void capture_preview_snapshot(person_picture_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_picture)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_picture)->background;
    snapshot->display_name = preview_picture.display_name;
    snapshot->initials = preview_picture.initials;
    snapshot->fallback_glyph = preview_picture.fallback_glyph;
    snapshot->image = preview_picture.image;
    snapshot->font = preview_picture.font;
    snapshot->icon_font = preview_picture.icon_font;
    snapshot->surface_color = preview_picture.surface_color;
    snapshot->border_color = preview_picture.border_color;
    snapshot->foreground_color = preview_picture.foreground_color;
    snapshot->accent_color = preview_picture.accent_color;
    snapshot->success_color = preview_picture.success_color;
    snapshot->warning_color = preview_picture.warning_color;
    snapshot->neutral_color = preview_picture.neutral_color;
    snapshot->muted_color = preview_picture.muted_color;
    snapshot->on_click_listener = EGUI_VIEW_OF(&preview_picture)->on_click_listener;
    snapshot->api = EGUI_VIEW_OF(&preview_picture)->api;
    snapshot->alpha = EGUI_VIEW_OF(&preview_picture)->alpha;
    snapshot->tone = preview_picture.tone;
    snapshot->presence = preview_picture.presence;
    snapshot->compact_mode = preview_picture.compact_mode;
    snapshot->read_only_mode = preview_picture.read_only_mode;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_picture));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_picture)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_picture)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_picture)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_picture)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_picture)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_picture)->padding.bottom;
}

static void assert_preview_state_unchanged(const person_picture_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_picture)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_picture)->background == snapshot->background);
    assert_optional_string_equal(snapshot->display_name, preview_picture.display_name);
    assert_optional_string_equal(snapshot->initials, preview_picture.initials);
    assert_optional_string_equal(snapshot->fallback_glyph, preview_picture.fallback_glyph);
    EGUI_TEST_ASSERT_TRUE(preview_picture.image == snapshot->image);
    EGUI_TEST_ASSERT_TRUE(preview_picture.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_picture.icon_font == snapshot->icon_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_picture.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_picture.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->foreground_color.full, preview_picture.foreground_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_picture.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->success_color.full, preview_picture.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_picture.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->neutral_color.full, preview_picture.neutral_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_color.full, preview_picture.muted_color.full);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_picture)->on_click_listener == snapshot->on_click_listener);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_picture)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_picture)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->tone, preview_picture.tone);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->presence, preview_picture.presence);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_picture.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_picture.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_picture)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_picture)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_picture)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_picture)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_picture)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_picture)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_picture)->padding.bottom);
}

static void test_person_picture_init_uses_defaults(void)
{
    setup_person_picture();

    EGUI_TEST_ASSERT_NULL(egui_view_person_picture_get_display_name(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_NULL(egui_view_person_picture_get_initials(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_person_picture_get_fallback_glyph(EGUI_VIEW_OF(&test_picture)), EGUI_ICON_MS_PERSON) == 0);
    EGUI_TEST_ASSERT_NULL(egui_view_person_picture_get_image(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSON_PICTURE_TONE_NEUTRAL, egui_view_person_picture_get_tone(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSON_PICTURE_PRESENCE_NONE, egui_view_person_picture_get_presence(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_person_picture_get_compact_mode(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_person_picture_get_read_only_mode(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_TRUE(test_picture.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_default_icon_font(20) == EGUI_FONT_ICON_MS_16);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_default_icon_font(32) == EGUI_FONT_ICON_MS_20);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_default_icon_font(52) == EGUI_FONT_ICON_MS_24);
}

static void test_person_picture_setters_and_resolvers_clear_pressed_state(void)
{
    egui_color_t surface = EGUI_COLOR_HEX(0x101112);
    egui_color_t border = EGUI_COLOR_HEX(0x202122);
    egui_color_t foreground = EGUI_COLOR_HEX(0x303132);
    egui_color_t accent = EGUI_COLOR_HEX(0x404142);
    egui_color_t success = EGUI_COLOR_HEX(0x505152);
    egui_color_t warning = EGUI_COLOR_HEX(0x606162);
    egui_color_t neutral = EGUI_COLOR_HEX(0x707172);
    egui_color_t muted = EGUI_COLOR_HEX(0x808182);
    char resolved[3];

    setup_person_picture();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_display_name(EGUI_VIEW_OF(&test_picture), "Lena Marsh");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_person_picture_get_display_name(EGUI_VIEW_OF(&test_picture)), "Lena Marsh") == 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_picture)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_initials(EGUI_VIEW_OF(&test_picture), "ar");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_person_picture_get_initials(EGUI_VIEW_OF(&test_picture)), "ar") == 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_picture)->is_pressed);

    egui_view_person_picture_resolve_initials("ar", NULL, resolved);
    EGUI_TEST_ASSERT_TRUE(strcmp(resolved, "AR") == 0);
    egui_view_person_picture_resolve_initials(NULL, "Lena Marsh", resolved);
    EGUI_TEST_ASSERT_TRUE(strcmp(resolved, "LM") == 0);
    egui_view_person_picture_resolve_initials(NULL, "solo", resolved);
    EGUI_TEST_ASSERT_TRUE(strcmp(resolved, "SO") == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_fallback_glyph(EGUI_VIEW_OF(&test_picture), EGUI_ICON_MS_SETTINGS);
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_person_picture_get_fallback_glyph(EGUI_VIEW_OF(&test_picture)), EGUI_ICON_MS_SETTINGS) == 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_picture)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_fallback_glyph(EGUI_VIEW_OF(&test_picture), NULL);
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_person_picture_get_fallback_glyph(EGUI_VIEW_OF(&test_picture)), EGUI_ICON_MS_PERSON) == 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_picture)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_image(EGUI_VIEW_OF(&test_picture), &g_fake_image);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_get_image(EGUI_VIEW_OF(&test_picture)) == &g_fake_image);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_picture)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_tone(EGUI_VIEW_OF(&test_picture), EGUI_VIEW_PERSON_PICTURE_TONE_SUCCESS);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSON_PICTURE_TONE_SUCCESS, egui_view_person_picture_get_tone(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_picture)->is_pressed);

    egui_view_person_picture_set_tone(EGUI_VIEW_OF(&test_picture), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSON_PICTURE_TONE_NEUTRAL, egui_view_person_picture_get_tone(EGUI_VIEW_OF(&test_picture)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_presence(EGUI_VIEW_OF(&test_picture), EGUI_VIEW_PERSON_PICTURE_PRESENCE_BUSY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSON_PICTURE_PRESENCE_BUSY, egui_view_person_picture_get_presence(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_picture)->is_pressed);

    egui_view_person_picture_set_presence(EGUI_VIEW_OF(&test_picture), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSON_PICTURE_PRESENCE_NONE, egui_view_person_picture_get_presence(EGUI_VIEW_OF(&test_picture)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_font(EGUI_VIEW_OF(&test_picture), NULL);
    EGUI_TEST_ASSERT_TRUE(test_picture.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_picture)->is_pressed);

    egui_view_person_picture_set_icon_font(EGUI_VIEW_OF(&test_picture), EGUI_FONT_ICON_MS_16);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_resolve_icon_font(&test_picture, 56) == EGUI_FONT_ICON_MS_16);
    egui_view_person_picture_set_icon_font(EGUI_VIEW_OF(&test_picture), NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_resolve_icon_font(&test_picture, 56) == EGUI_FONT_ICON_MS_24);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_palette(EGUI_VIEW_OF(&test_picture), surface, border, foreground, accent, success, warning, neutral, muted);
    EGUI_TEST_ASSERT_EQUAL_INT(surface.full, test_picture.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(border.full, test_picture.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(foreground.full, test_picture.foreground_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(accent.full, test_picture.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(success.full, test_picture.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(warning.full, test_picture.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(neutral.full, test_picture.neutral_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(muted.full, test_picture.muted_color.full);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_picture)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_compact_mode(EGUI_VIEW_OF(&test_picture), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_person_picture_get_compact_mode(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_picture)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_read_only_mode(EGUI_VIEW_OF(&test_picture), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_person_picture_get_read_only_mode(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_picture)->is_pressed);
}

static void test_person_picture_helpers_compute_regions_and_colors(void)
{
    egui_region_t avatar_region;
    egui_region_t presence_region;
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);

    setup_person_picture();
    layout_view(EGUI_VIEW_OF(&test_picture), 10, 20, 56, 56);
    egui_view_person_picture_get_avatar_region(&test_picture, EGUI_VIEW_OF(&test_picture), &avatar_region);

    EGUI_TEST_ASSERT_TRUE(avatar_region.size.width > 0);
    EGUI_TEST_ASSERT_EQUAL_INT(54, avatar_region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(1, avatar_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(1, avatar_region.location.y);
    EGUI_TEST_ASSERT_FALSE(egui_view_person_picture_should_draw_presence(&test_picture));
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.accent_color.full,
                               egui_view_person_picture_tone_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_TONE_ACCENT).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.success_color.full,
                               egui_view_person_picture_tone_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_TONE_SUCCESS).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.warning_color.full,
                               egui_view_person_picture_tone_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_TONE_WARNING).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.neutral_color.full,
                               egui_view_person_picture_tone_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_TONE_NEUTRAL).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.success_color.full,
                               egui_view_person_picture_presence_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_PRESENCE_LIVE).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.warning_color.full,
                               egui_view_person_picture_presence_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_PRESENCE_BUSY).full);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(test_picture.warning_color, test_picture.accent_color, 24).full,
                               egui_view_person_picture_presence_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_PRESENCE_AWAY).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.neutral_color.full,
                               egui_view_person_picture_presence_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_PRESENCE_OFFLINE).full);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 66).full, egui_view_person_picture_mix_disabled(sample).full);

    egui_view_person_picture_set_presence(EGUI_VIEW_OF(&test_picture), EGUI_VIEW_PERSON_PICTURE_PRESENCE_BUSY);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_should_draw_presence(&test_picture));
    egui_view_person_picture_get_presence_region(&test_picture, &avatar_region, &presence_region);
    EGUI_TEST_ASSERT_TRUE(presence_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(presence_region.location.x > avatar_region.location.x);

    egui_view_person_picture_set_compact_mode(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_set_size(EGUI_VIEW_OF(&test_picture), 36, 36);
    layout_view(EGUI_VIEW_OF(&test_picture), 4, 6, 36, 36);
    egui_view_person_picture_get_avatar_region(&test_picture, EGUI_VIEW_OF(&test_picture), &avatar_region);
    EGUI_TEST_ASSERT_EQUAL_INT(36, avatar_region.size.width);
    egui_view_person_picture_get_presence_region(&test_picture, &avatar_region, &presence_region);
    EGUI_TEST_ASSERT_TRUE(presence_region.size.width <= 12);
}

static void test_person_picture_static_preview_consumes_input_and_keeps_state(void)
{
    person_picture_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_picture();
    layout_view(EGUI_VIEW_OF(&preview_picture), 12, 18, 38, 38);
    get_view_center(EGUI_VIEW_OF(&preview_picture), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_picture), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_picture), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_picture), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_picture), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_person_picture_run(void)
{
    EGUI_TEST_SUITE_BEGIN(person_picture);
    EGUI_TEST_RUN(test_person_picture_init_uses_defaults);
    EGUI_TEST_RUN(test_person_picture_setters_and_resolvers_clear_pressed_state);
    EGUI_TEST_RUN(test_person_picture_helpers_compute_regions_and_colors);
    EGUI_TEST_RUN(test_person_picture_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
