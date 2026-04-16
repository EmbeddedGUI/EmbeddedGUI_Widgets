#include "egui.h"
#include "egui_view_field.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define FIELD_ROOT_WIDTH        224
#define FIELD_ROOT_HEIGHT       220
#define FIELD_PRIMARY_WIDTH     196
#define FIELD_PRIMARY_HEIGHT    126
#define FIELD_PREVIEW_WIDTH     104
#define FIELD_PREVIEW_HEIGHT     54
#define FIELD_BOTTOM_ROW_WIDTH  216
#define FIELD_BOTTOM_ROW_HEIGHT  54
#define FIELD_RECORD_WAIT        90
#define FIELD_RECORD_FRAME_WAIT 170
#define FIELD_RECORD_FINAL_WAIT 320
#define FIELD_DEFAULT_SNAPSHOT    0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct field_snapshot field_snapshot_t;
struct field_snapshot
{
    const char *label;
    const char *field_text;
    const char *placeholder;
    const char *helper_text;
    const char *validation_text;
    const char *info_title;
    const char *info_body;
    uint8_t required;
    uint8_t validation_state;
    uint8_t open;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static hcw_field_t primary_field;
static egui_view_linearlayout_t bottom_row;
static hcw_field_t compact_field;
static hcw_field_t read_only_field;
static egui_view_api_t compact_field_api;
static egui_view_api_t read_only_field_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Field";

static const field_snapshot_t primary_snapshots[] = {
        {
                "Notification email",
                "",
                "name@company.com",
                "Used for build alerts only.",
                "",
                "",
                "",
                0,
                HCW_FIELD_VALIDATION_NONE,
                0,
        },
        {
                "API token",
                "staging-reader",
                "",
                "",
                "Required before handoff.",
                "Why required",
                "",
                1,
                HCW_FIELD_VALIDATION_WARNING,
                1,
        },
        {
                "Owner alias",
                "",
                "team@example.com",
                "Used by audit routing.",
                "Enter a valid alias before saving.",
                "",
                "",
                1,
                HCW_FIELD_VALIDATION_ERROR,
                0,
        },
};

static const field_snapshot_t compact_snapshot = {
        "Alias",
        "core-api",
        "",
        "",
        "",
        "",
        "",
        0,
        HCW_FIELD_VALIDATION_NONE,
        0,
};

static const field_snapshot_t read_only_snapshot = {
        "Region",
        "North Europe",
        "",
        "",
        "",
        "",
        "",
        0,
        HCW_FIELD_VALIDATION_NONE,
        0,
};

static void layout_page(void);

static void apply_snapshot(egui_view_t *view, const field_snapshot_t *snapshot)
{
    hcw_field_set_label(view, snapshot->label);
    hcw_field_set_field_text(view, snapshot->field_text);
    hcw_field_set_placeholder(view, snapshot->placeholder);
    hcw_field_set_helper_text(view, snapshot->helper_text);
    hcw_field_set_validation_text(view, snapshot->validation_text);
    hcw_field_set_validation_state(view, snapshot->validation_state);
    hcw_field_set_info_title(view, snapshot->info_title);
    hcw_field_set_info_body(view, snapshot->info_body);
    hcw_field_set_required(view, snapshot->required);
    hcw_field_set_open(view, snapshot->open);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot(EGUI_VIEW_OF(&primary_field), &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT]);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(FIELD_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    hcw_field_apply_compact_style(EGUI_VIEW_OF(&compact_field));
    apply_snapshot(EGUI_VIEW_OF(&compact_field), &compact_snapshot);

    hcw_field_apply_compact_style(EGUI_VIEW_OF(&read_only_field));
    apply_snapshot(EGUI_VIEW_OF(&read_only_field), &read_only_snapshot);
    hcw_field_set_read_only_mode(EGUI_VIEW_OF(&read_only_field), 1);

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
    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), FIELD_ROOT_WIDTH, FIELD_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), FIELD_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    hcw_field_init(EGUI_VIEW_OF(&primary_field));
    egui_view_set_size(EGUI_VIEW_OF(&primary_field), FIELD_PRIMARY_WIDTH, FIELD_PRIMARY_HEIGHT);
    hcw_field_set_font(EGUI_VIEW_OF(&primary_field), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_field_set_meta_font(EGUI_VIEW_OF(&primary_field), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_field_set_icon_font(EGUI_VIEW_OF(&primary_field), EGUI_FONT_ICON_MS_16);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_field), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_field));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), FIELD_BOTTOM_ROW_WIDTH, FIELD_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    hcw_field_init(EGUI_VIEW_OF(&compact_field));
    egui_view_set_size(EGUI_VIEW_OF(&compact_field), FIELD_PREVIEW_WIDTH, FIELD_PREVIEW_HEIGHT);
    hcw_field_set_font(EGUI_VIEW_OF(&compact_field), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_field_set_meta_font(EGUI_VIEW_OF(&compact_field), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_field_set_icon_font(EGUI_VIEW_OF(&compact_field), EGUI_FONT_ICON_MS_16);
    hcw_field_override_static_preview_api(EGUI_VIEW_OF(&compact_field), &compact_field_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_field), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_field));

    hcw_field_init(EGUI_VIEW_OF(&read_only_field));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_field), FIELD_PREVIEW_WIDTH, FIELD_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_field), 8, 0, 0, 0);
    hcw_field_set_font(EGUI_VIEW_OF(&read_only_field), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_field_set_meta_font(EGUI_VIEW_OF(&read_only_field), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_field_set_icon_font(EGUI_VIEW_OF(&read_only_field), EGUI_FONT_ICON_MS_16);
    hcw_field_override_static_preview_api(EGUI_VIEW_OF(&read_only_field), &read_only_field_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&read_only_field), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_field));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
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
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_FRAME_WAIT);
        return true;
    case 5:
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
