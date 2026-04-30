#include <string.h>

#include "egui.h"
#include "egui_view_auto_suggest_box.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define AUTO_SUGGEST_BOX_ROOT_WIDTH        224
#define AUTO_SUGGEST_BOX_ROOT_HEIGHT       206
#define AUTO_SUGGEST_BOX_PRIMARY_WIDTH     196
#define AUTO_SUGGEST_BOX_PRIMARY_HEIGHT    34
#define AUTO_SUGGEST_BOX_PREVIEW_WIDTH     104
#define AUTO_SUGGEST_BOX_PREVIEW_HEIGHT    28
#define AUTO_SUGGEST_BOX_BOTTOM_ROW_WIDTH  216
#define AUTO_SUGGEST_BOX_BOTTOM_ROW_HEIGHT 28
#define AUTO_SUGGEST_BOX_RECORD_WAIT       90
#define AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT 170
#define AUTO_SUGGEST_BOX_RECORD_FINAL_WAIT 280
#define AUTO_SUGGEST_BOX_KEYBOARD_HEIGHT   128
#define AUTO_SUGGEST_BOX_KEYBOARD_Y        ((EGUI_CONFIG_SCREEN_HEIGHT > AUTO_SUGGEST_BOX_KEYBOARD_HEIGHT) ? (EGUI_CONFIG_SCREEN_HEIGHT - AUTO_SUGGEST_BOX_KEYBOARD_HEIGHT) : 0)
#define AUTO_SUGGEST_BOX_KEYBOARD_HIDDEN_Y (EGUI_CONFIG_SCREEN_HEIGHT + AUTO_SUGGEST_BOX_KEYBOARD_HEIGHT)

typedef struct auto_suggest_snapshot auto_suggest_snapshot_t;
struct auto_suggest_snapshot
{
    const char **suggestions;
    uint8_t suggestion_count;
    const char *query;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_auto_suggest_box_t control_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_auto_suggest_box_t control_compact;
static egui_view_linearlayout_t read_only_column;
static egui_view_auto_suggest_box_t control_read_only;
static egui_view_api_t control_compact_api;
static egui_view_api_t control_read_only_api;
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static egui_view_keyboard_t control_keyboard;
static egui_view_api_t control_primary_focus_api;
static uint8_t preserve_root_position_on_focus_loss;
#endif
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "AutoSuggest Box";

static const char *people_suggestions[] = {"Alice Chen", "Alicia Gomez", "Allen Park", "Amelia Stone"};
#if EGUI_CONFIG_RECORDING_TEST
static const char *command_suggestions[] = {"Deploy API", "Deploy Docs", "Deploy Worker"};
#endif
static const char *preview_suggestions[] = {"Recent", "Reminder"};

static const auto_suggest_snapshot_t primary_people = {
        .suggestions = people_suggestions,
        .suggestion_count = EGUI_ARRAY_SIZE(people_suggestions),
        .query = "Ali",
};

#if EGUI_CONFIG_RECORDING_TEST
static const auto_suggest_snapshot_t primary_commands = {
        .suggestions = command_suggestions,
        .suggestion_count = EGUI_ARRAY_SIZE(command_suggestions),
        .query = "Dep",
};
#endif

static void layout_page(void);
static void sync_page_layout(void);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void clear_primary_focus(egui_view_t *self)
{
    EGUI_UNUSED(self);
    preserve_root_position_on_focus_loss = 0;
    egui_view_clear_focus(EGUI_VIEW_OF(&control_primary));
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void relocate_view(egui_view_t *view, egui_dim_t x, egui_dim_t y)
{
    egui_region_t region;

    if (view->region.location.x == x && view->region.location.y == y)
    {
        return;
    }

    region = view->region;
    region.location.x = x;
    region.location.y = y;
    egui_view_layout(view, &region);
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void position_keyboard_visible(void)
{
    relocate_view(EGUI_VIEW_OF(&control_keyboard), 0, AUTO_SUGGEST_BOX_KEYBOARD_Y);
}

static void position_keyboard_hidden(void)
{
    relocate_view(EGUI_VIEW_OF(&control_keyboard), 0, AUTO_SUGGEST_BOX_KEYBOARD_HIDDEN_Y);
}

static void on_primary_selected(egui_view_t *self, uint8_t index)
{
    EGUI_UNUSED(index);
    preserve_root_position_on_focus_loss = 1;
    egui_view_auto_suggest_box_collapse(self);
    egui_view_clear_focus(self);
}

static void on_primary_submit(egui_view_t *self, const char *text)
{
    EGUI_UNUSED(text);
    preserve_root_position_on_focus_loss = 1;
    egui_view_auto_suggest_box_collapse(self);
    egui_view_clear_focus(self);
}
#endif

static void apply_snapshot(egui_view_t *view, const auto_suggest_snapshot_t *snapshot)
{
    hcw_auto_suggest_box_set_suggestions(view, snapshot->suggestions, snapshot->suggestion_count);
    hcw_auto_suggest_box_set_query(view, snapshot->query);
    egui_view_auto_suggest_box_collapse(view);
}

static void apply_primary_default_state(void)
{
    apply_snapshot(EGUI_VIEW_OF(&control_primary), &primary_people);
    if (ui_ready)
    {
        sync_page_layout();
    }
}

static void apply_preview_states(void)
{
    hcw_auto_suggest_box_set_suggestions(EGUI_VIEW_OF(&control_compact), preview_suggestions, EGUI_ARRAY_SIZE(preview_suggestions));
    hcw_auto_suggest_box_set_current_index(EGUI_VIEW_OF(&control_compact), 0);
    hcw_auto_suggest_box_set_suggestions(EGUI_VIEW_OF(&control_read_only), preview_suggestions, EGUI_ARRAY_SIZE(preview_suggestions));
    hcw_auto_suggest_box_set_current_index(EGUI_VIEW_OF(&control_read_only), 0);
    if (ui_ready)
    {
        sync_page_layout();
    }
}

static void layout_root_children(void)
{
    egui_dim_t x;
    egui_dim_t y;

    x = (EGUI_VIEW_OF(&root_layout)->region.size.width - EGUI_VIEW_OF(&title_label)->region.size.width) / 2;
    if (x < 0)
    {
        x = 0;
    }
    y = EGUI_VIEW_OF(&title_label)->margin.top;
    egui_view_set_position(EGUI_VIEW_OF(&title_label), x, y);

    y += EGUI_VIEW_OF(&title_label)->region.size.height + EGUI_VIEW_OF(&title_label)->margin.bottom + EGUI_VIEW_OF(&control_primary)->margin.top;
    x = (EGUI_VIEW_OF(&root_layout)->region.size.width - EGUI_VIEW_OF(&control_primary)->region.size.width) / 2;
    if (x < 0)
    {
        x = 0;
    }
    egui_view_set_position(EGUI_VIEW_OF(&control_primary), x, y);

    y += EGUI_VIEW_OF(&control_primary)->region.size.height + EGUI_VIEW_OF(&control_primary)->margin.bottom + EGUI_VIEW_OF(&bottom_row)->margin.top;
    x = (EGUI_VIEW_OF(&root_layout)->region.size.width - EGUI_VIEW_OF(&bottom_row)->region.size.width) / 2;
    if (x < 0)
    {
        x = 0;
    }
    egui_view_set_position(EGUI_VIEW_OF(&bottom_row), x, y);
}

static void layout_local_views(void)
{
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&read_only_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    layout_root_children();
}

static egui_dim_t get_stable_root_y(void)
{
    egui_dim_t root_y = (EGUI_CONFIG_SCREEN_HEIGHT - EGUI_VIEW_OF(&root_layout)->region.size.height) / 2;

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (EGUI_VIEW_OF(&control_keyboard)->region.size.height > 0)
    {
        egui_dim_t max_root_bottom = EGUI_CONFIG_SCREEN_HEIGHT - EGUI_VIEW_OF(&control_keyboard)->region.size.height - 12;

        if (root_y + EGUI_VIEW_OF(&root_layout)->region.size.height > max_root_bottom)
        {
            root_y = max_root_bottom - EGUI_VIEW_OF(&root_layout)->region.size.height;
        }
    }
#endif
    if (root_y < 0)
    {
        root_y = 0;
    }
    return root_y;
}

static void layout_page(void)
{
    egui_dim_t root_x;
    egui_dim_t root_y;

    layout_local_views();
    root_x = (EGUI_CONFIG_SCREEN_WIDTH - EGUI_VIEW_OF(&root_layout)->region.size.width) / 2;
    root_y = get_stable_root_y();
    if (root_x < 0)
    {
        root_x = 0;
    }
    relocate_view(EGUI_VIEW_OF(&root_layout), root_x, root_y);
}

static void layout_page_preserve_root_position(void)
{
    egui_dim_t root_x = EGUI_VIEW_OF(&root_layout)->region.location.x;
    egui_dim_t root_y = EGUI_VIEW_OF(&root_layout)->region.location.y;

    layout_local_views();
    if (root_x < 0)
    {
        root_x = 0;
    }
    if (root_y < 0)
    {
        root_y = 0;
    }
    relocate_view(EGUI_VIEW_OF(&root_layout), root_x, root_y);
}

static void sync_page_layout(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (preserve_root_position_on_focus_loss)
    {
        layout_page_preserve_root_position();
        return;
    }
#endif
    layout_page();
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void on_primary_focus_changed(egui_view_t *self, int is_focused)
{
    egui_view_textinput_t *textinput = (egui_view_textinput_t *)self;

    if (is_focused)
    {
        textinput->cursor_visible = 1;
        egui_view_start_timer(self, &textinput->cursor_timer, EGUI_CONFIG_TEXTINPUT_CURSOR_BLINK_MS, 0);
        position_keyboard_visible();
        egui_view_keyboard_show(EGUI_VIEW_OF(&control_keyboard), self);
    }
    else
    {
        textinput->cursor_visible = 0;
        egui_view_stop_timer(self, &textinput->cursor_timer);
        egui_view_auto_suggest_box_collapse(self);
        egui_view_keyboard_hide(EGUI_VIEW_OF(&control_keyboard));
        position_keyboard_hidden();
    }
    if (ui_ready)
    {
        sync_page_layout();
        egui_core_update_region_dirty_all(uicode_get_core());
    }
    egui_view_invalidate(self);
}
#endif

#if EGUI_CONFIG_RECORDING_TEST
static void request_page_snapshot(void)
{
    sync_page_layout();
    egui_core_update_region_dirty_all(uicode_get_core());
    recording_request_snapshot();
}

static uint8_t set_primary_field_click_action(egui_sim_action_t *p_action, int interval_ms)
{
    egui_view_t *view = EGUI_VIEW_OF(&control_primary);

    if (p_action == NULL || view->region_screen.size.width <= 0 || control_primary.collapsed_height <= 0)
    {
        return 0;
    }

    p_action->type = EGUI_SIM_ACTION_CLICK;
    p_action->x1 = view->region_screen.location.x + view->region_screen.size.width / 2;
    p_action->y1 = view->region_screen.location.y + control_primary.collapsed_height / 2;
    p_action->interval_ms = interval_ms;
    return 1;
}
#endif

void test_init_ui(void)
{
    ui_ready = 0;
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    preserve_root_position_on_focus_loss = 0;
#endif

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), AUTO_SUGGEST_BOX_ROOT_WIDTH, AUTO_SUGGEST_BOX_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&root_layout), clear_primary_focus);
#endif

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), AUTO_SUGGEST_BOX_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_auto_suggest_box_init(EGUI_VIEW_OF(&control_primary), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&control_primary), AUTO_SUGGEST_BOX_PRIMARY_WIDTH, AUTO_SUGGEST_BOX_PRIMARY_HEIGHT);
    egui_view_auto_suggest_box_set_font(EGUI_VIEW_OF(&control_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_auto_suggest_box_apply_standard_style(EGUI_VIEW_OF(&control_primary));
    egui_view_auto_suggest_box_set_placeholder(EGUI_VIEW_OF(&control_primary), "Search people or commands");
    egui_view_set_margin(EGUI_VIEW_OF(&control_primary), 0, 0, 0, 8);
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_auto_suggest_box_set_on_selected_listener(EGUI_VIEW_OF(&control_primary), on_primary_selected);
    egui_view_textinput_set_on_submit(EGUI_VIEW_OF(&control_primary), on_primary_submit);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&control_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), AUTO_SUGGEST_BOX_BOTTOM_ROW_WIDTH, AUTO_SUGGEST_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), AUTO_SUGGEST_BOX_PREVIEW_WIDTH, AUTO_SUGGEST_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_auto_suggest_box_init(EGUI_VIEW_OF(&control_compact), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&control_compact), AUTO_SUGGEST_BOX_PREVIEW_WIDTH, AUTO_SUGGEST_BOX_PREVIEW_HEIGHT);
    egui_view_auto_suggest_box_set_font(EGUI_VIEW_OF(&control_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_auto_suggest_box_apply_compact_style(EGUI_VIEW_OF(&control_compact));
    hcw_auto_suggest_box_override_static_preview_api(EGUI_VIEW_OF(&control_compact), &control_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&control_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&read_only_column), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&read_only_column), AUTO_SUGGEST_BOX_PREVIEW_WIDTH, AUTO_SUGGEST_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&read_only_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&read_only_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_column));

    egui_view_auto_suggest_box_init(EGUI_VIEW_OF(&control_read_only), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&control_read_only), AUTO_SUGGEST_BOX_PREVIEW_WIDTH, AUTO_SUGGEST_BOX_PREVIEW_HEIGHT);
    egui_view_auto_suggest_box_set_font(EGUI_VIEW_OF(&control_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_auto_suggest_box_apply_read_only_style(EGUI_VIEW_OF(&control_read_only));
    hcw_auto_suggest_box_override_static_preview_api(EGUI_VIEW_OF(&control_read_only), &control_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_column), EGUI_VIEW_OF(&control_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_keyboard_init(EGUI_VIEW_OF(&control_keyboard), uicode_get_core());
#if EGUI_CONFIG_FUNCTION_SUPPORT_LAYER
    egui_view_set_layer(EGUI_VIEW_OF(&control_keyboard), EGUI_VIEW_LAYER_TOP);
#endif
    egui_view_set_position(EGUI_VIEW_OF(&control_keyboard), 0, AUTO_SUGGEST_BOX_KEYBOARD_HIDDEN_Y);
    egui_view_set_size(EGUI_VIEW_OF(&control_keyboard), EGUI_CONFIG_SCREEN_WIDTH, AUTO_SUGGEST_BOX_KEYBOARD_HEIGHT);
    position_keyboard_hidden();
    egui_view_keyboard_set_font(EGUI_VIEW_OF(&control_keyboard), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    egui_view_keyboard_set_icon_font(EGUI_VIEW_OF(&control_keyboard), EGUI_FONT_ICON_MS_20);
    egui_view_keyboard_set_special_key_icons(EGUI_VIEW_OF(&control_keyboard), EGUI_ICON_MS_KEYBOARD_ARROW_UP, EGUI_ICON_MS_BACKSPACE, EGUI_ICON_MS_DONE);
    egui_core_add_user_root_view(EGUI_VIEW_OF(&control_keyboard));
    egui_view_override_api_on_focus_changed(EGUI_VIEW_OF(&control_primary), &control_primary_focus_api, on_primary_focus_changed);
#endif
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&control_primary));
#endif
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&control_primary), &event);

    event.type = EGUI_KEY_EVENT_ACTION_UP;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&control_primary), &event);
    if (ui_ready)
    {
        sync_page_layout();
    }
}

bool egui_port_get_recording_action(int action_index, egui_sim_action_t *p_action)
{
    static int last_action = -1;
    int first_call = action_index != last_action;

    last_action = action_index;

    switch (action_index)
    {
    case 0:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_DOWN);
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_DOWN);
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_snapshot(EGUI_VIEW_OF(&control_primary), &primary_commands);
            apply_primary_key(EGUI_KEY_CODE_DOWN);
            apply_primary_key(EGUI_KEY_CODE_END);
            apply_primary_key(EGUI_KEY_CODE_ENTER);
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (!set_primary_field_click_action(p_action, AUTO_SUGGEST_BOX_RECORD_WAIT))
        {
            EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_WAIT);
        }
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_BACKSPACE);
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
