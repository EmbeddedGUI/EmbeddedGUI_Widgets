#include <string.h>

#include "egui.h"
#include "egui_view_search_box.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SEARCH_BOX_ROOT_WIDTH        224
#define SEARCH_BOX_ROOT_HEIGHT       136
#define SEARCH_BOX_PRIMARY_WIDTH     196
#define SEARCH_BOX_PRIMARY_HEIGHT    40
#define SEARCH_BOX_PREVIEW_WIDTH     104
#define SEARCH_BOX_PREVIEW_HEIGHT    32
#define SEARCH_BOX_BOTTOM_ROW_WIDTH  216
#define SEARCH_BOX_BOTTOM_ROW_HEIGHT 32
#define SEARCH_BOX_RECORD_WAIT       90
#define SEARCH_BOX_RECORD_FRAME_WAIT 170
#define SEARCH_BOX_RECORD_FINAL_WAIT 280
#define SEARCH_BOX_DEFAULT_SNAPSHOT  0
#define SEARCH_BOX_KEYBOARD_HEIGHT   128
#define SEARCH_BOX_KEYBOARD_Y        ((EGUI_CONFIG_SCEEN_HEIGHT > SEARCH_BOX_KEYBOARD_HEIGHT) ? (EGUI_CONFIG_SCEEN_HEIGHT - SEARCH_BOX_KEYBOARD_HEIGHT) : 0)
#define SEARCH_BOX_KEYBOARD_HIDDEN_Y (EGUI_CONFIG_SCEEN_HEIGHT + SEARCH_BOX_KEYBOARD_HEIGHT)

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct search_box_snapshot search_box_snapshot_t;
struct search_box_snapshot
{
    const char *placeholder;
    const char *text;
    uint8_t max_length;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_search_box_t box_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_search_box_t box_compact;
static egui_view_search_box_t box_read_only;
static egui_view_api_t box_compact_api;
static egui_view_api_t box_read_only_api;
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static egui_view_keyboard_t box_keyboard;
static egui_view_api_t box_primary_focus_api;
#endif
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Search Box";

static const search_box_snapshot_t primary_snapshots[] = {
        {"Search templates", "Roadmap", 24},
        {"Search templates", "Asset audit", 24},
        {"Search templates", "", 24},
};

static const search_box_snapshot_t compact_snapshot = {"Recent", "Assets", 16};
static const search_box_snapshot_t read_only_snapshot = {"Pinned", "Archive", 16};

static void layout_page(void);
static void relocate_view(egui_view_t *view, egui_dim_t x, egui_dim_t y);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void clear_primary_focus(egui_view_t *self)
{
    EGUI_UNUSED(self);
    egui_view_clear_focus(EGUI_VIEW_OF(&box_primary));
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void position_keyboard_visible(void)
{
    relocate_view(EGUI_VIEW_OF(&box_keyboard), 0, SEARCH_BOX_KEYBOARD_Y);
}

static void position_keyboard_hidden(void)
{
    relocate_view(EGUI_VIEW_OF(&box_keyboard), 0, SEARCH_BOX_KEYBOARD_HIDDEN_Y);
}

static void on_primary_submit(egui_view_t *self, const char *text)
{
    EGUI_UNUSED(text);
    egui_view_clear_focus(self);
}
#endif

static void apply_snapshot(egui_view_t *view, const search_box_snapshot_t *snapshot)
{
    egui_view_search_box_set_placeholder(view, snapshot->placeholder);
    egui_view_search_box_set_text(view, snapshot->text);
    egui_view_search_box_set_max_length(view, snapshot->max_length);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot(EGUI_VIEW_OF(&box_primary), &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT]);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(EGUI_VIEW_OF(&box_primary));
#endif
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(SEARCH_BOX_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    apply_snapshot(EGUI_VIEW_OF(&box_compact), &compact_snapshot);
    apply_snapshot(EGUI_VIEW_OF(&box_read_only), &read_only_snapshot);

    if (ui_ready)
    {
        layout_page();
    }
}

static void layout_local_views(void)
{
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
}

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

static egui_dim_t get_stable_root_y(void)
{
    egui_dim_t root_y = (EGUI_CONFIG_SCEEN_HEIGHT - EGUI_VIEW_OF(&root_layout)->region.size.height) / 2;

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (EGUI_VIEW_OF(&box_keyboard)->region.size.height > 0)
    {
        egui_dim_t max_root_bottom = EGUI_CONFIG_SCEEN_HEIGHT - EGUI_VIEW_OF(&box_keyboard)->region.size.height - 12;

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
    egui_dim_t root_x = (EGUI_CONFIG_SCEEN_WIDTH - EGUI_VIEW_OF(&root_layout)->region.size.width) / 2;

    layout_local_views();
    if (root_x < 0)
    {
        root_x = 0;
    }
    relocate_view(EGUI_VIEW_OF(&root_layout), root_x, get_stable_root_y());
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void on_primary_focus_changed(egui_view_t *self, int is_focused)
{
    egui_view_textinput_t *textinput = &((egui_view_search_box_t *)self)->textinput;

    if (is_focused)
    {
        textinput->cursor_visible = 1;
        egui_view_start_timer(self, &textinput->cursor_timer, EGUI_CONFIG_TEXTINPUT_CURSOR_BLINK_MS, 0);
        position_keyboard_visible();
        egui_view_keyboard_show(EGUI_VIEW_OF(&box_keyboard), self);
    }
    else
    {
        textinput->cursor_visible = 0;
        egui_view_stop_timer(self, &textinput->cursor_timer);
        egui_view_keyboard_hide(EGUI_VIEW_OF(&box_keyboard));
        position_keyboard_hidden();
    }
    if (ui_ready)
    {
        layout_page();
        egui_core_update_region_dirty_all(uicode_get_core());
    }
    egui_view_invalidate(self);
}
#endif

#if EGUI_CONFIG_RECORDING_TEST
static void request_page_snapshot(void)
{
    layout_page();
    egui_core_update_region_dirty_all(uicode_get_core());
    recording_request_snapshot();
}

static uint8_t set_primary_field_click_action(egui_sim_action_t *p_action, int interval_ms)
{
    egui_view_t *view = EGUI_VIEW_OF(&box_primary);

    if (p_action == NULL || view->region_screen.size.width <= 0 || view->region_screen.size.height <= 0)
    {
        return 0;
    }

    p_action->type = EGUI_SIM_ACTION_CLICK;
    p_action->x1 = view->region_screen.location.x + view->region_screen.size.width / 2;
    p_action->y1 = view->region_screen.location.y + view->region_screen.size.height / 2;
    p_action->interval_ms = interval_ms;
    return 1;
}

static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&box_primary));
#endif
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&box_primary), &event);

    event.type = EGUI_KEY_EVENT_ACTION_UP;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&box_primary), &event);
    if (ui_ready)
    {
        layout_page();
    }
}
#endif

void test_init_ui(void)
{
    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SEARCH_BOX_ROOT_WIDTH, SEARCH_BOX_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&root_layout), clear_primary_focus);
#endif

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SEARCH_BOX_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_search_box_init(EGUI_VIEW_OF(&box_primary));
    egui_view_set_size(EGUI_VIEW_OF(&box_primary), SEARCH_BOX_PRIMARY_WIDTH, SEARCH_BOX_PRIMARY_HEIGHT);
    egui_view_search_box_set_font(EGUI_VIEW_OF(&box_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_search_box_apply_standard_style(EGUI_VIEW_OF(&box_primary));
    egui_view_set_margin(EGUI_VIEW_OF(&box_primary), 0, 0, 0, 8);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&box_primary), true);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_textinput_set_on_submit(EGUI_VIEW_OF(&box_primary), on_primary_submit);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&box_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SEARCH_BOX_BOTTOM_ROW_WIDTH, SEARCH_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_search_box_init(EGUI_VIEW_OF(&box_compact));
    egui_view_set_size(EGUI_VIEW_OF(&box_compact), SEARCH_BOX_PREVIEW_WIDTH, SEARCH_BOX_PREVIEW_HEIGHT);
    egui_view_search_box_set_font(EGUI_VIEW_OF(&box_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_search_box_apply_compact_style(EGUI_VIEW_OF(&box_compact));
    egui_view_search_box_override_static_preview_api(EGUI_VIEW_OF(&box_compact), &box_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&box_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&box_compact));

    egui_view_search_box_init(EGUI_VIEW_OF(&box_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&box_read_only), SEARCH_BOX_PREVIEW_WIDTH, SEARCH_BOX_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&box_read_only), 4, 0, 0, 0);
    egui_view_search_box_set_font(EGUI_VIEW_OF(&box_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_search_box_apply_read_only_style(EGUI_VIEW_OF(&box_read_only));
    egui_view_search_box_override_static_preview_api(EGUI_VIEW_OF(&box_read_only), &box_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&box_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&box_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_keyboard_init(EGUI_VIEW_OF(&box_keyboard), uicode_get_core());
#if EGUI_CONFIG_FUNCTION_SUPPORT_LAYER
    egui_view_set_layer(EGUI_VIEW_OF(&box_keyboard), EGUI_VIEW_LAYER_TOP);
#endif
    egui_view_set_position(EGUI_VIEW_OF(&box_keyboard), 0, SEARCH_BOX_KEYBOARD_HIDDEN_Y);
    egui_view_set_size(EGUI_VIEW_OF(&box_keyboard), EGUI_CONFIG_SCEEN_WIDTH, SEARCH_BOX_KEYBOARD_HEIGHT);
    position_keyboard_hidden();
    egui_view_keyboard_set_font(EGUI_VIEW_OF(&box_keyboard), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    egui_view_keyboard_set_icon_font(EGUI_VIEW_OF(&box_keyboard), EGUI_FONT_ICON_MS_20);
    egui_view_keyboard_set_special_key_icons(EGUI_VIEW_OF(&box_keyboard), EGUI_ICON_MS_KEYBOARD_ARROW_UP, EGUI_ICON_MS_BACKSPACE, EGUI_ICON_MS_DONE);
    egui_core_add_user_root_view(EGUI_VIEW_OF(&box_keyboard));
    egui_view_override_api_on_focus_changed(EGUI_VIEW_OF(&box_primary), &box_primary_focus_api, on_primary_focus_changed);
#endif
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
}

#if EGUI_CONFIG_RECORDING_TEST
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
        EGUI_SIM_SET_WAIT(p_action, SEARCH_BOX_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (!set_primary_field_click_action(p_action, SEARCH_BOX_RECORD_WAIT))
        {
            EGUI_SIM_SET_WAIT(p_action, SEARCH_BOX_RECORD_WAIT);
        }
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SEARCH_BOX_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_BACKSPACE);
        }
        EGUI_SIM_SET_WAIT(p_action, SEARCH_BOX_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SEARCH_BOX_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_ENTER);
        }
        EGUI_SIM_SET_WAIT(p_action, SEARCH_BOX_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SEARCH_BOX_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
