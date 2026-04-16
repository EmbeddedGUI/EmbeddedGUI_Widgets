#include "egui.h"
#include "egui_view_radio_button.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define RADIO_BUTTON_ROOT_WIDTH             224
#define RADIO_BUTTON_ROOT_HEIGHT            214
#define RADIO_BUTTON_PRIMARY_COLUMN_WIDTH   196
#define RADIO_BUTTON_PRIMARY_COLUMN_HEIGHT  98
#define RADIO_BUTTON_PRIMARY_ITEM_WIDTH     196
#define RADIO_BUTTON_PRIMARY_ITEM_HEIGHT    30
#define RADIO_BUTTON_PREVIEW_WIDTH          104
#define RADIO_BUTTON_PREVIEW_COLUMN_HEIGHT  52
#define RADIO_BUTTON_PREVIEW_ITEM_WIDTH     104
#define RADIO_BUTTON_PREVIEW_ITEM_HEIGHT    24
#define RADIO_BUTTON_BOTTOM_ROW_WIDTH       216
#define RADIO_BUTTON_BOTTOM_ROW_HEIGHT      52
#define RADIO_BUTTON_RECORD_WAIT            90
#define RADIO_BUTTON_RECORD_FRAME_WAIT      170
#define RADIO_BUTTON_RECORD_FINAL_WAIT      280
#define RADIO_BUTTON_DEFAULT_SNAPSHOT       0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct primary_radio_snapshot primary_radio_snapshot_t;
struct primary_radio_snapshot
{
    const char *texts[3];
    uint8_t selected_index;
};

typedef struct preview_radio_snapshot preview_radio_snapshot_t;
struct preview_radio_snapshot
{
    const char *texts[2];
    uint8_t selected_index;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_column;
static egui_view_radio_button_t primary_buttons[3];
static egui_view_radio_group_t primary_group;
static egui_view_api_t primary_button_api[3];
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_radio_button_t compact_buttons[2];
static egui_view_radio_group_t compact_group;
static egui_view_api_t compact_button_api[2];
static egui_view_linearlayout_t read_only_column;
static egui_view_radio_button_t read_only_buttons[2];
static egui_view_radio_group_t read_only_group;
static egui_view_api_t read_only_button_api[2];
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Radio Button";

static const primary_radio_snapshot_t primary_snapshots[] = {
        {{"Home", "Alerts", "Privacy"}, 0},
        {{"Email", "Push", "SMS"}, 1},
        {{"Daily", "Weekly", "Monthly"}, 2},
};

static const preview_radio_snapshot_t compact_snapshot = {
        {"Auto", "Manual"}, 0,
};

static const preview_radio_snapshot_t read_only_snapshot = {
        {"Desktop", "Tablet"}, 1,
};

static void layout_page(void);
static void focus_primary_button(uint8_t index);

static void apply_snapshot_to_group(egui_view_radio_button_t *buttons, uint8_t count, const char *const *texts, uint8_t selected_index)
{
    uint8_t i;

    for (i = 0; i < count; ++i)
    {
        hcw_radio_button_set_text(EGUI_VIEW_OF(&buttons[i]), texts[i]);
        hcw_radio_button_set_checked(EGUI_VIEW_OF(&buttons[i]), 0);
    }

    if (selected_index < count)
    {
        hcw_radio_button_set_checked(EGUI_VIEW_OF(&buttons[selected_index]), 1);
    }
}

static void apply_primary_snapshot(uint8_t index)
{
    const primary_radio_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    apply_snapshot_to_group(primary_buttons, EGUI_ARRAY_SIZE(primary_buttons), snapshot->texts, snapshot->selected_index);
    if (ui_ready)
    {
        focus_primary_button(snapshot->selected_index);
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(RADIO_BUTTON_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    apply_snapshot_to_group(compact_buttons, EGUI_ARRAY_SIZE(compact_buttons), compact_snapshot.texts, compact_snapshot.selected_index);
    apply_snapshot_to_group(read_only_buttons, EGUI_ARRAY_SIZE(read_only_buttons), read_only_snapshot.texts, read_only_snapshot.selected_index);
    if (ui_ready)
    {
        layout_page();
    }
}

static void init_radio_button(egui_view_radio_button_t *button, egui_view_api_t *api, egui_dim_t width, egui_dim_t height, const egui_font_t *font,
                              void (*apply_style)(egui_view_t *), uint8_t is_focusable, uint8_t is_static_preview)
{
    egui_view_radio_button_init(EGUI_VIEW_OF(button));
    egui_view_set_size(EGUI_VIEW_OF(button), width, height);
    hcw_radio_button_set_font(EGUI_VIEW_OF(button), font);
    apply_style(EGUI_VIEW_OF(button));
    if (is_static_preview)
    {
        hcw_radio_button_override_static_preview_api(EGUI_VIEW_OF(button), api);
    }
    else
    {
        hcw_radio_button_override_interaction_api(EGUI_VIEW_OF(button), api);
    }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(button), is_focusable);
#else
    EGUI_UNUSED(is_focusable);
#endif
}

static void layout_local_views(void)
{
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&read_only_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
}

static void layout_page(void)
{
    layout_local_views();
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

static void focus_primary_button(uint8_t index)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (index < EGUI_ARRAY_SIZE(primary_buttons))
    {
        egui_view_request_focus(EGUI_VIEW_OF(&primary_buttons[index]));
    }
#else
    EGUI_UNUSED(index);
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
    uint8_t i;

    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), RADIO_BUTTON_ROOT_WIDTH, RADIO_BUTTON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), RADIO_BUTTON_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&primary_column));
    egui_view_set_size(EGUI_VIEW_OF(&primary_column), RADIO_BUTTON_PRIMARY_COLUMN_WIDTH, RADIO_BUTTON_PRIMARY_COLUMN_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&primary_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&primary_column), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_column), 0, 0, 0, 12);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_column));

    egui_view_radio_group_init(&primary_group);
    for (i = 0; i < EGUI_ARRAY_SIZE(primary_buttons); ++i)
    {
        init_radio_button(&primary_buttons[i], &primary_button_api[i], RADIO_BUTTON_PRIMARY_ITEM_WIDTH, RADIO_BUTTON_PRIMARY_ITEM_HEIGHT,
                          (const egui_font_t *)&egui_res_font_montserrat_10_4, hcw_radio_button_apply_standard_style, 1, 0);
        if (i + 1 < EGUI_ARRAY_SIZE(primary_buttons))
        {
            egui_view_set_margin(EGUI_VIEW_OF(&primary_buttons[i]), 0, 0, 0, 4);
        }
        egui_view_radio_group_add(&primary_group, EGUI_VIEW_OF(&primary_buttons[i]));
        egui_view_group_add_child(EGUI_VIEW_OF(&primary_column), EGUI_VIEW_OF(&primary_buttons[i]));
    }

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), RADIO_BUTTON_BOTTOM_ROW_WIDTH, RADIO_BUTTON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), RADIO_BUTTON_PREVIEW_WIDTH, RADIO_BUTTON_PREVIEW_COLUMN_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_radio_group_init(&compact_group);
    for (i = 0; i < EGUI_ARRAY_SIZE(compact_buttons); ++i)
    {
        init_radio_button(&compact_buttons[i], &compact_button_api[i], RADIO_BUTTON_PREVIEW_ITEM_WIDTH, RADIO_BUTTON_PREVIEW_ITEM_HEIGHT,
                          (const egui_font_t *)&egui_res_font_montserrat_10_4, hcw_radio_button_apply_compact_style, 0, 1);
        if (i + 1 < EGUI_ARRAY_SIZE(compact_buttons))
        {
            egui_view_set_margin(EGUI_VIEW_OF(&compact_buttons[i]), 0, 0, 0, 4);
        }
        egui_view_radio_group_add(&compact_group, EGUI_VIEW_OF(&compact_buttons[i]));
        egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&compact_buttons[i]));
    }

    egui_view_linearlayout_init(EGUI_VIEW_OF(&read_only_column));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_column), RADIO_BUTTON_PREVIEW_WIDTH, RADIO_BUTTON_PREVIEW_COLUMN_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&read_only_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&read_only_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_column));

    egui_view_radio_group_init(&read_only_group);
    for (i = 0; i < EGUI_ARRAY_SIZE(read_only_buttons); ++i)
    {
        init_radio_button(&read_only_buttons[i], &read_only_button_api[i], RADIO_BUTTON_PREVIEW_ITEM_WIDTH, RADIO_BUTTON_PREVIEW_ITEM_HEIGHT,
                          (const egui_font_t *)&egui_res_font_montserrat_10_4, hcw_radio_button_apply_read_only_style, 0, 1);
        if (i + 1 < EGUI_ARRAY_SIZE(read_only_buttons))
        {
            egui_view_set_margin(EGUI_VIEW_OF(&read_only_buttons[i]), 0, 0, 0, 4);
        }
        egui_view_radio_group_add(&read_only_group, EGUI_VIEW_OF(&read_only_buttons[i]));
        egui_view_group_add_child(EGUI_VIEW_OF(&read_only_column), EGUI_VIEW_OF(&read_only_buttons[i]));
    }

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    layout_page();
    focus_primary_button(primary_snapshots[RADIO_BUTTON_DEFAULT_SNAPSHOT].selected_index);
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
            focus_primary_button(primary_snapshots[RADIO_BUTTON_DEFAULT_SNAPSHOT].selected_index);
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTON_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTON_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 5:
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTON_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RADIO_BUTTON_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
