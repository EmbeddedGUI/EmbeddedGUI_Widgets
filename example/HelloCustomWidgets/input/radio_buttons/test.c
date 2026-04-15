#include "egui.h"
#include "egui_view_radio_buttons.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define RADIO_BUTTONS_ROOT_WIDTH        224
#define RADIO_BUTTONS_ROOT_HEIGHT       196
#define RADIO_BUTTONS_PRIMARY_WIDTH     196
#define RADIO_BUTTONS_PRIMARY_HEIGHT    90
#define RADIO_BUTTONS_PREVIEW_WIDTH     104
#define RADIO_BUTTONS_PREVIEW_HEIGHT    60
#define RADIO_BUTTONS_BOTTOM_ROW_WIDTH  216
#define RADIO_BUTTONS_BOTTOM_ROW_HEIGHT 60
#define RADIO_BUTTONS_RECORD_WAIT       90
#define RADIO_BUTTONS_RECORD_FRAME_WAIT 170
#define RADIO_BUTTONS_RECORD_FINAL_WAIT 280
#define RADIO_BUTTONS_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct primary_radio_buttons_snapshot primary_radio_buttons_snapshot_t;
struct primary_radio_buttons_snapshot
{
    const char *items[3];
    uint8_t selected_index;
};

typedef struct preview_radio_buttons_snapshot preview_radio_buttons_snapshot_t;
struct preview_radio_buttons_snapshot
{
    const char *items[2];
    uint8_t selected_index;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_radio_buttons_t primary_widget;
static egui_view_linearlayout_t bottom_row;
static egui_view_radio_buttons_t compact_widget;
static egui_view_radio_buttons_t read_only_widget;
static egui_view_api_t compact_widget_api;
static egui_view_api_t read_only_widget_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "RadioButtons";

static const primary_radio_buttons_snapshot_t primary_snapshots[] = {
        {{"Email", "Push", "SMS"}, 0},
        {{"Daily", "Weekly", "Monthly"}, 1},
        {{"Auto", "Light", "Dark"}, 2},
};

static const preview_radio_buttons_snapshot_t compact_snapshot = {
        {"Auto", "Manual"}, 0,
};

static const preview_radio_buttons_snapshot_t read_only_snapshot = {
        {"Desktop", "Tablet"}, 1,
};

static void layout_page(void);
static void focus_primary_widget(void);

static void apply_snapshot_to_widget(egui_view_radio_buttons_t *widget, const char *const *items, uint8_t item_count, uint8_t selected_index)
{
    egui_view_radio_buttons_set_items(EGUI_VIEW_OF(widget), items, item_count);
    egui_view_radio_buttons_set_current_index(EGUI_VIEW_OF(widget), selected_index);
}

static void apply_primary_snapshot(uint8_t index)
{
    const primary_radio_buttons_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    apply_snapshot_to_widget(&primary_widget, snapshot->items, EGUI_ARRAY_SIZE(snapshot->items), snapshot->selected_index);
    if (ui_ready)
    {
        layout_page();
        focus_primary_widget();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(RADIO_BUTTONS_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    apply_snapshot_to_widget(&compact_widget, compact_snapshot.items, EGUI_ARRAY_SIZE(compact_snapshot.items), compact_snapshot.selected_index);
    egui_view_radio_buttons_set_compact_mode(EGUI_VIEW_OF(&compact_widget), 1);

    apply_snapshot_to_widget(&read_only_widget, read_only_snapshot.items, EGUI_ARRAY_SIZE(read_only_snapshot.items), read_only_snapshot.selected_index);
    egui_view_radio_buttons_set_compact_mode(EGUI_VIEW_OF(&read_only_widget), 1);
    egui_view_radio_buttons_set_read_only_mode(EGUI_VIEW_OF(&read_only_widget), 1);

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

static void layout_page(void)
{
    layout_local_views();
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

static void focus_primary_widget(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&primary_widget));
#endif
}

#if EGUI_CONFIG_RECORDING_TEST
static void request_page_snapshot(void)
{
    layout_page();
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), RADIO_BUTTONS_ROOT_WIDTH, RADIO_BUTTONS_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), RADIO_BUTTONS_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_radio_buttons_init(EGUI_VIEW_OF(&primary_widget));
    egui_view_set_size(EGUI_VIEW_OF(&primary_widget), RADIO_BUTTONS_PRIMARY_WIDTH, RADIO_BUTTONS_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_widget), 0, 0, 0, 12);
    egui_view_radio_buttons_set_font(EGUI_VIEW_OF(&primary_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_widget));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), RADIO_BUTTONS_BOTTOM_ROW_WIDTH, RADIO_BUTTONS_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_radio_buttons_init(EGUI_VIEW_OF(&compact_widget));
    egui_view_set_size(EGUI_VIEW_OF(&compact_widget), RADIO_BUTTONS_PREVIEW_WIDTH, RADIO_BUTTONS_PREVIEW_HEIGHT);
    egui_view_radio_buttons_set_font(EGUI_VIEW_OF(&compact_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_radio_buttons_override_static_preview_api(EGUI_VIEW_OF(&compact_widget), &compact_widget_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_widget), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_widget));

    egui_view_radio_buttons_init(EGUI_VIEW_OF(&read_only_widget));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_widget), RADIO_BUTTONS_PREVIEW_WIDTH, RADIO_BUTTONS_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_widget), 8, 0, 0, 0);
    egui_view_radio_buttons_set_font(EGUI_VIEW_OF(&read_only_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_radio_buttons_override_static_preview_api(EGUI_VIEW_OF(&read_only_widget), &read_only_widget_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&read_only_widget), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_widget));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    layout_page();
    focus_primary_widget();
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
            focus_primary_widget();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTONS_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTONS_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTONS_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTONS_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTONS_RECORD_FRAME_WAIT);
        return true;
    case 5:
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTONS_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTONS_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
