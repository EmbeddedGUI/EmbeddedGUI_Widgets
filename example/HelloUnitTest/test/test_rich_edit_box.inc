#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_rich_edit_box.h"

#include "../../HelloCustomWidgets/input/rich_edit_box/egui_view_rich_edit_box.h"
#include "../../HelloCustomWidgets/input/rich_edit_box/egui_view_rich_edit_box.c"

typedef struct rich_edit_box_preview_snapshot rich_edit_box_preview_snapshot_t;
struct rich_edit_box_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const egui_view_rich_edit_box_document_t *documents;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_rich_edit_box_action_listener_t on_action;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t editor_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t shadow_color;
    char editor_text[EGUI_CONFIG_TEXTINPUT_MAX_LENGTH + 1];
    uint8_t document_count;
    uint8_t current_document;
    uint8_t current_part;
    uint8_t current_preset;
    uint8_t applied_preset;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_part;
    uint8_t text_len;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_rich_edit_box_t rich_edit_box_test_widget;
static egui_view_rich_edit_box_t rich_edit_box_preview_widget;
static egui_view_api_t rich_edit_box_preview_api;
static uint8_t rich_edit_box_action_count;
static uint8_t rich_edit_box_action_document;
static uint8_t rich_edit_box_action_preset;
static uint8_t rich_edit_box_action_style;
static uint8_t rich_edit_box_action_text_length;

static const egui_view_rich_edit_box_preset_t rich_edit_box_main_presets[] = {
        {"Body", "Paragraph", EGUI_VIEW_RICH_EDIT_BOX_STYLE_BODY},
        {"Callout", "Highlight", EGUI_VIEW_RICH_EDIT_BOX_STYLE_CALLOUT},
        {"Checklist", "Tasks", EGUI_VIEW_RICH_EDIT_BOX_STYLE_CHECKLIST},
};

static const egui_view_rich_edit_box_document_t rich_edit_box_documents[] = {
        {"OPS", "Briefing", "Summary", "Footer", "Draft", rich_edit_box_main_presets, 3, 1},
        {"QA", "Blank", "Summary", "Footer", "", rich_edit_box_main_presets, 3, 0},
};

static const egui_view_rich_edit_box_preset_t rich_edit_box_overflow_presets[] = {
        {"A", "A", EGUI_VIEW_RICH_EDIT_BOX_STYLE_BODY},
        {"B", "B", EGUI_VIEW_RICH_EDIT_BOX_STYLE_CALLOUT},
        {"C", "C", EGUI_VIEW_RICH_EDIT_BOX_STYLE_CHECKLIST},
        {"D", "D", EGUI_VIEW_RICH_EDIT_BOX_STYLE_CALLOUT},
};

static const egui_view_rich_edit_box_document_t rich_edit_box_overflow_documents[] = {
        {"A", "A", "A", "A", "Alpha", rich_edit_box_overflow_presets, 4, 2},
        {"B", "B", "B", "B", "Bravo", rich_edit_box_main_presets, 3, 1},
        {"C", "C", "C", "C", "Charlie", rich_edit_box_main_presets, 3, 1},
        {"D", "D", "D", "D", "Delta", rich_edit_box_main_presets, 3, 1},
        {"E", "E", "E", "E", "Echo", rich_edit_box_main_presets, 3, 1},
        {"F", "F", "F", "F", "Foxtrot", rich_edit_box_main_presets, 3, 1},
        {"G", "G", "G", "G", "Golf", rich_edit_box_main_presets, 3, 1},
};

static const egui_view_rich_edit_box_document_t rich_edit_box_preview_document = {
        "UI", "Preview", "", "", "Snap", rich_edit_box_main_presets, 3, 0};

static void rich_edit_box_on_action(egui_view_t *self, uint8_t document_index, uint8_t preset_index, uint8_t style, uint8_t text_length)
{
    EGUI_UNUSED(self);
    rich_edit_box_action_count++;
    rich_edit_box_action_document = document_index;
    rich_edit_box_action_preset = preset_index;
    rich_edit_box_action_style = style;
    rich_edit_box_action_text_length = text_length;
}

static void rich_edit_box_reset_action_state(void)
{
    rich_edit_box_action_count = 0;
    rich_edit_box_action_document = 0xFF;
    rich_edit_box_action_preset = 0xFF;
    rich_edit_box_action_style = 0xFF;
    rich_edit_box_action_text_length = 0xFF;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void rich_edit_box_setup_widget(const egui_view_rich_edit_box_document_t *documents, uint8_t document_count)
{
    egui_view_rich_edit_box_init(EGUI_VIEW_OF(&rich_edit_box_test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&rich_edit_box_test_widget), 196, 146);
    egui_view_rich_edit_box_set_documents(EGUI_VIEW_OF(&rich_edit_box_test_widget), documents, document_count);
    egui_view_rich_edit_box_set_font(EGUI_VIEW_OF(&rich_edit_box_test_widget), NULL);
    egui_view_rich_edit_box_set_meta_font(EGUI_VIEW_OF(&rich_edit_box_test_widget), NULL);
    egui_view_rich_edit_box_set_on_action_listener(EGUI_VIEW_OF(&rich_edit_box_test_widget), rich_edit_box_on_action);
    rich_edit_box_reset_action_state();
}

static void rich_edit_box_setup_preview_widget(void)
{
    egui_view_rich_edit_box_init(EGUI_VIEW_OF(&rich_edit_box_preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&rich_edit_box_preview_widget), 104, 82);
    egui_view_rich_edit_box_set_documents(EGUI_VIEW_OF(&rich_edit_box_preview_widget), &rich_edit_box_preview_document, 1);
    egui_view_rich_edit_box_set_font(EGUI_VIEW_OF(&rich_edit_box_preview_widget), NULL);
    egui_view_rich_edit_box_set_meta_font(EGUI_VIEW_OF(&rich_edit_box_preview_widget), NULL);
    egui_view_rich_edit_box_set_compact_mode(EGUI_VIEW_OF(&rich_edit_box_preview_widget), 1);
    egui_view_rich_edit_box_override_static_preview_api(EGUI_VIEW_OF(&rich_edit_box_preview_widget), &rich_edit_box_preview_api);
    rich_edit_box_reset_action_state();
}

static void rich_edit_box_layout_view(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static void rich_edit_box_layout_widget(void)
{
    rich_edit_box_layout_view(EGUI_VIEW_OF(&rich_edit_box_test_widget), 10, 20, 196, 146);
}

static void rich_edit_box_layout_preview_widget(void)
{
    rich_edit_box_layout_view(EGUI_VIEW_OF(&rich_edit_box_preview_widget), 12, 18, 104, 82);
}

static int rich_edit_box_send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->dispatch_touch_event(view, &event);
}

static int rich_edit_box_send_key_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int rich_edit_box_send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return rich_edit_box_send_touch_to_view(EGUI_VIEW_OF(&rich_edit_box_test_widget), type, x, y);
}

static int rich_edit_box_send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return rich_edit_box_send_touch_to_view(EGUI_VIEW_OF(&rich_edit_box_preview_widget), type, x, y);
}

static int rich_edit_box_send_key_action(uint8_t type, uint8_t key_code)
{
    return rich_edit_box_send_key_to_view(EGUI_VIEW_OF(&rich_edit_box_test_widget), type, key_code);
}

static int rich_edit_box_send_preview_key_action(uint8_t type, uint8_t key_code)
{
    return rich_edit_box_send_key_to_view(EGUI_VIEW_OF(&rich_edit_box_preview_widget), type, key_code);
}

static int rich_edit_box_send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= rich_edit_box_send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= rich_edit_box_send_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int rich_edit_box_send_preview_key(uint8_t key_code)
{
    int handled = 0;

    handled |= rich_edit_box_send_preview_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= rich_edit_box_send_preview_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static void rich_edit_box_capture_preview_snapshot(rich_edit_box_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&rich_edit_box_preview_widget)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&rich_edit_box_preview_widget)->background;
    snapshot->documents = rich_edit_box_preview_widget.documents;
    snapshot->font = rich_edit_box_preview_widget.font;
    snapshot->meta_font = rich_edit_box_preview_widget.meta_font;
    snapshot->on_action = rich_edit_box_preview_widget.on_action;
    snapshot->surface_color = rich_edit_box_preview_widget.surface_color;
    snapshot->border_color = rich_edit_box_preview_widget.border_color;
    snapshot->editor_color = rich_edit_box_preview_widget.editor_color;
    snapshot->text_color = rich_edit_box_preview_widget.text_color;
    snapshot->muted_text_color = rich_edit_box_preview_widget.muted_text_color;
    snapshot->accent_color = rich_edit_box_preview_widget.accent_color;
    snapshot->shadow_color = rich_edit_box_preview_widget.shadow_color;
    strcpy(snapshot->editor_text, rich_edit_box_preview_widget.editor_text);
    snapshot->document_count = rich_edit_box_preview_widget.document_count;
    snapshot->current_document = rich_edit_box_preview_widget.current_document;
    snapshot->current_part = rich_edit_box_preview_widget.current_part;
    snapshot->current_preset = rich_edit_box_preview_widget.current_preset;
    snapshot->applied_preset = rich_edit_box_preview_widget.applied_preset;
    snapshot->compact_mode = rich_edit_box_preview_widget.compact_mode;
    snapshot->read_only_mode = rich_edit_box_preview_widget.read_only_mode;
    snapshot->pressed_part = rich_edit_box_preview_widget.pressed_part;
    snapshot->text_len = rich_edit_box_preview_widget.text_len;
    snapshot->alpha = EGUI_VIEW_OF(&rich_edit_box_preview_widget)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&rich_edit_box_preview_widget));
    snapshot->is_focused = EGUI_VIEW_OF(&rich_edit_box_preview_widget)->is_focused;
    snapshot->padding_left = EGUI_VIEW_OF(&rich_edit_box_preview_widget)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&rich_edit_box_preview_widget)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&rich_edit_box_preview_widget)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&rich_edit_box_preview_widget)->padding.bottom;
}

static void rich_edit_box_assert_preview_state_unchanged(const rich_edit_box_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&rich_edit_box_preview_widget)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&rich_edit_box_preview_widget)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_preview_widget.documents == snapshot->documents);
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_preview_widget.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_preview_widget.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_preview_widget.on_action == snapshot->on_action);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, rich_edit_box_preview_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, rich_edit_box_preview_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->editor_color.full, rich_edit_box_preview_widget.editor_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, rich_edit_box_preview_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, rich_edit_box_preview_widget.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, rich_edit_box_preview_widget.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->shadow_color.full, rich_edit_box_preview_widget.shadow_color.full);
    EGUI_TEST_ASSERT_TRUE(strcmp(snapshot->editor_text, rich_edit_box_preview_widget.editor_text) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->document_count, rich_edit_box_preview_widget.document_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_document, rich_edit_box_preview_widget.current_document);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_part, rich_edit_box_preview_widget.current_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_preset, rich_edit_box_preview_widget.current_preset);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->applied_preset, rich_edit_box_preview_widget.applied_preset);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, rich_edit_box_preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, rich_edit_box_preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_part, rich_edit_box_preview_widget.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_len, rich_edit_box_preview_widget.text_len);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&rich_edit_box_preview_widget)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&rich_edit_box_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&rich_edit_box_preview_widget)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&rich_edit_box_preview_widget)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&rich_edit_box_preview_widget)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&rich_edit_box_preview_widget)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&rich_edit_box_preview_widget)->padding.bottom);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&rich_edit_box_preview_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, rich_edit_box_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, rich_edit_box_action_document);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, rich_edit_box_action_preset);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, rich_edit_box_action_style);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, rich_edit_box_action_text_length);
}

static uint8_t rich_edit_box_get_part_center(egui_view_t *view, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_rich_edit_box_get_part_region(view, part, &region))
    {
        return 0;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void rich_edit_box_get_view_outside_point(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x - 4;
    *y = view->region_screen.location.y - 4;
}

static void rich_edit_box_seed_pressed_state(egui_view_rich_edit_box_t *widget, uint8_t part, uint8_t visual_pressed)
{
    widget->pressed_part = part;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void rich_edit_box_assert_pressed_cleared(egui_view_rich_edit_box_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_EDIT_BOX_PART_NONE, widget->pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void test_rich_edit_box_set_documents_clamp_and_defaults(void)
{
    rich_edit_box_setup_widget(rich_edit_box_overflow_documents, EGUI_ARRAY_SIZE(rich_edit_box_overflow_documents));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_EDIT_BOX_MAX_DOCUMENTS, rich_edit_box_test_widget.document_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_edit_box_get_current_document(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_rich_edit_box_get_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_rich_edit_box_get_applied_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_rich_edit_box_get_text_length(EGUI_VIEW_OF(&rich_edit_box_test_widget)));

    rich_edit_box_seed_pressed_state(&rich_edit_box_test_widget, EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_1, 1);
    rich_edit_box_test_widget.current_document = 5;
    egui_view_rich_edit_box_set_documents(EGUI_VIEW_OF(&rich_edit_box_test_widget), rich_edit_box_documents, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, rich_edit_box_test_widget.document_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_edit_box_get_current_document(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rich_edit_box_get_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rich_edit_box_get_applied_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_rich_edit_box_get_text_length(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    rich_edit_box_assert_pressed_cleared(&rich_edit_box_test_widget);

    egui_view_rich_edit_box_set_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rich_edit_box_get_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));

    egui_view_rich_edit_box_set_documents(EGUI_VIEW_OF(&rich_edit_box_test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, rich_edit_box_test_widget.document_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_edit_box_get_current_document(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_edit_box_get_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_edit_box_get_applied_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_edit_box_get_text_length(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
}

static void test_rich_edit_box_setters_clear_pressed_and_update_state(void)
{
    rich_edit_box_setup_widget(rich_edit_box_documents, EGUI_ARRAY_SIZE(rich_edit_box_documents));

    rich_edit_box_seed_pressed_state(&rich_edit_box_test_widget, EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_1, 1);
    egui_view_rich_edit_box_set_font(EGUI_VIEW_OF(&rich_edit_box_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    rich_edit_box_assert_pressed_cleared(&rich_edit_box_test_widget);

    rich_edit_box_seed_pressed_state(&rich_edit_box_test_widget, EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_1, 1);
    egui_view_rich_edit_box_set_meta_font(EGUI_VIEW_OF(&rich_edit_box_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    rich_edit_box_assert_pressed_cleared(&rich_edit_box_test_widget);

    rich_edit_box_seed_pressed_state(&rich_edit_box_test_widget, EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_1, 1);
    egui_view_rich_edit_box_set_compact_mode(EGUI_VIEW_OF(&rich_edit_box_test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, rich_edit_box_test_widget.compact_mode);
    rich_edit_box_assert_pressed_cleared(&rich_edit_box_test_widget);
    egui_view_rich_edit_box_set_compact_mode(EGUI_VIEW_OF(&rich_edit_box_test_widget), 0);

    rich_edit_box_seed_pressed_state(&rich_edit_box_test_widget, EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_1, 1);
    egui_view_rich_edit_box_set_read_only_mode(EGUI_VIEW_OF(&rich_edit_box_test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, rich_edit_box_test_widget.read_only_mode);
    rich_edit_box_assert_pressed_cleared(&rich_edit_box_test_widget);
    egui_view_rich_edit_box_set_read_only_mode(EGUI_VIEW_OF(&rich_edit_box_test_widget), 0);

    rich_edit_box_seed_pressed_state(&rich_edit_box_test_widget, EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_2, 1);
    egui_view_rich_edit_box_set_palette(EGUI_VIEW_OF(&rich_edit_box_test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122),
                                        EGUI_COLOR_HEX(0x303132), EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162),
                                        EGUI_COLOR_HEX(0x707172));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, rich_edit_box_test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, rich_edit_box_test_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, rich_edit_box_test_widget.editor_color.full);
    rich_edit_box_assert_pressed_cleared(&rich_edit_box_test_widget);

    egui_view_rich_edit_box_set_current_document(EGUI_VIEW_OF(&rich_edit_box_test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rich_edit_box_get_current_document(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_edit_box_get_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_edit_box_get_text_length(EGUI_VIEW_OF(&rich_edit_box_test_widget)));

    rich_edit_box_seed_pressed_state(&rich_edit_box_test_widget, EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_0, 1);
    egui_view_rich_edit_box_set_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_rich_edit_box_get_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_2, rich_edit_box_test_widget.current_part);
    rich_edit_box_assert_pressed_cleared(&rich_edit_box_test_widget);
}

static void test_rich_edit_box_regions_apply_and_listener(void)
{
    egui_dim_t x;
    egui_dim_t y;

    rich_edit_box_setup_widget(rich_edit_box_documents, EGUI_ARRAY_SIZE(rich_edit_box_documents));
    rich_edit_box_layout_widget();

    EGUI_TEST_ASSERT_TRUE(rich_edit_box_get_part_center(EGUI_VIEW_OF(&rich_edit_box_test_widget), EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR, &x, &y));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_get_part_center(EGUI_VIEW_OF(&rich_edit_box_test_widget), EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_2, &x, &y));
    egui_view_rich_edit_box_set_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget), 2);
    EGUI_TEST_ASSERT_TRUE(egui_view_rich_edit_box_apply_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, rich_edit_box_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, rich_edit_box_action_document);
    EGUI_TEST_ASSERT_EQUAL_INT(2, rich_edit_box_action_preset);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_EDIT_BOX_STYLE_CHECKLIST, rich_edit_box_action_style);
    EGUI_TEST_ASSERT_EQUAL_INT(5, rich_edit_box_action_text_length);

    rich_edit_box_reset_action_state();
    egui_view_rich_edit_box_set_on_action_listener(EGUI_VIEW_OF(&rich_edit_box_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_rich_edit_box_apply_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, rich_edit_box_action_count);
}

static void test_rich_edit_box_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t preset_x;
    egui_dim_t preset_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    rich_edit_box_setup_widget(rich_edit_box_documents, EGUI_ARRAY_SIZE(rich_edit_box_documents));
    rich_edit_box_layout_widget();
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_get_part_center(EGUI_VIEW_OF(&rich_edit_box_test_widget), EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_0, &preset_x, &preset_y));
    rich_edit_box_get_view_outside_point(EGUI_VIEW_OF(&rich_edit_box_test_widget), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, preset_x, preset_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_0, rich_edit_box_test_widget.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&rich_edit_box_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&rich_edit_box_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, rich_edit_box_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rich_edit_box_get_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    rich_edit_box_assert_pressed_cleared(&rich_edit_box_test_widget);

    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, preset_x, preset_y));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, preset_x, preset_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&rich_edit_box_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_touch(EGUI_MOTION_EVENT_ACTION_UP, preset_x, preset_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, rich_edit_box_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_edit_box_get_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_edit_box_get_applied_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    rich_edit_box_assert_pressed_cleared(&rich_edit_box_test_widget);

    rich_edit_box_reset_action_state();
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, preset_x, preset_y));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, preset_x, preset_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, rich_edit_box_action_count);
    rich_edit_box_assert_pressed_cleared(&rich_edit_box_test_widget);
}

static void test_rich_edit_box_key_navigation_apply_and_editing(void)
{
    rich_edit_box_setup_widget(rich_edit_box_documents, EGUI_ARRAY_SIZE(rich_edit_box_documents));
    rich_edit_box_layout_widget();

    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_edit_box_get_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rich_edit_box_get_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_rich_edit_box_get_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rich_edit_box_get_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_rich_edit_box_get_current_preset(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_2, rich_edit_box_test_widget.current_part);

    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_2, rich_edit_box_test_widget.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&rich_edit_box_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, rich_edit_box_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, rich_edit_box_action_preset);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_EDIT_BOX_STYLE_CHECKLIST, rich_edit_box_action_style);
    rich_edit_box_assert_pressed_cleared(&rich_edit_box_test_widget);

    egui_view_rich_edit_box_set_current_document(EGUI_VIEW_OF(&rich_edit_box_test_widget), 1);
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key(EGUI_KEY_CODE_H));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key(EGUI_KEY_CODE_I));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key(EGUI_KEY_CODE_Q));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key(EGUI_KEY_CODE_Z));
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_key(EGUI_KEY_CODE_BACKSPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_rich_edit_box_get_text_length(EGUI_VIEW_OF(&rich_edit_box_test_widget)));
    EGUI_TEST_ASSERT_TRUE(strcmp("hi q\n", egui_view_rich_edit_box_get_text(EGUI_VIEW_OF(&rich_edit_box_test_widget))) == 0);
}

static void test_rich_edit_box_read_only_and_disabled_guards(void)
{
    egui_dim_t preset_x;
    egui_dim_t preset_y;

    rich_edit_box_setup_widget(rich_edit_box_documents, EGUI_ARRAY_SIZE(rich_edit_box_documents));
    rich_edit_box_layout_widget();
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_get_part_center(EGUI_VIEW_OF(&rich_edit_box_test_widget), EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_1, &preset_x, &preset_y));

    rich_edit_box_seed_pressed_state(&rich_edit_box_test_widget, EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_1, 1);
    egui_view_rich_edit_box_set_read_only_mode(EGUI_VIEW_OF(&rich_edit_box_test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(rich_edit_box_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, preset_x, preset_y));
    EGUI_TEST_ASSERT_FALSE(rich_edit_box_send_key(EGUI_KEY_CODE_ENTER));
    rich_edit_box_assert_pressed_cleared(&rich_edit_box_test_widget);
    egui_view_rich_edit_box_set_read_only_mode(EGUI_VIEW_OF(&rich_edit_box_test_widget), 0);

    rich_edit_box_seed_pressed_state(&rich_edit_box_test_widget, EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_1, 1);
    egui_view_set_enable(EGUI_VIEW_OF(&rich_edit_box_test_widget), 0);
    EGUI_TEST_ASSERT_FALSE(rich_edit_box_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, preset_x, preset_y));
    EGUI_TEST_ASSERT_FALSE(rich_edit_box_send_key(EGUI_KEY_CODE_ENTER));
    rich_edit_box_assert_pressed_cleared(&rich_edit_box_test_widget);
    egui_view_set_enable(EGUI_VIEW_OF(&rich_edit_box_test_widget), 1);
}

static void test_rich_edit_box_static_preview_consumes_input_and_keeps_state(void)
{
    rich_edit_box_preview_snapshot_t initial_snapshot;
    egui_dim_t preset_x;
    egui_dim_t preset_y;

    rich_edit_box_setup_preview_widget();
    rich_edit_box_layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(
            rich_edit_box_get_part_center(EGUI_VIEW_OF(&rich_edit_box_preview_widget), EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_0, &preset_x, &preset_y));
    rich_edit_box_capture_preview_snapshot(&initial_snapshot);

    rich_edit_box_seed_pressed_state(&rich_edit_box_preview_widget, EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_0, 1);
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, preset_x, preset_y));
    rich_edit_box_assert_preview_state_unchanged(&initial_snapshot);

    rich_edit_box_seed_pressed_state(&rich_edit_box_preview_widget, EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_0, 1);
    EGUI_TEST_ASSERT_TRUE(rich_edit_box_send_preview_key(EGUI_KEY_CODE_ENTER));
    rich_edit_box_assert_preview_state_unchanged(&initial_snapshot);
}

void test_rich_edit_box_run(void)
{
    EGUI_TEST_SUITE_BEGIN(rich_edit_box);
    EGUI_TEST_RUN(test_rich_edit_box_set_documents_clamp_and_defaults);
    EGUI_TEST_RUN(test_rich_edit_box_setters_clear_pressed_and_update_state);
    EGUI_TEST_RUN(test_rich_edit_box_regions_apply_and_listener);
    EGUI_TEST_RUN(test_rich_edit_box_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_rich_edit_box_key_navigation_apply_and_editing);
    EGUI_TEST_RUN(test_rich_edit_box_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_rich_edit_box_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
