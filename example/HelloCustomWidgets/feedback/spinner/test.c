#include "egui.h"
#include "egui_view_spinner.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SPINNER_ROOT_WIDTH        224
#define SPINNER_ROOT_HEIGHT       152
#define SPINNER_PRIMARY_SIZE      44
#define SPINNER_STATUS_WIDTH      196
#define SPINNER_STATUS_HEIGHT     14
#define SPINNER_BOTTOM_ROW_WIDTH  88
#define SPINNER_BOTTOM_ROW_HEIGHT 40
#define SPINNER_PREVIEW_SIZE      40
#define SPINNER_RECORD_WAIT       90
#define SPINNER_RECORD_FRAME_WAIT 170
#define SPINNER_RECORD_FINAL_WAIT 520
#define SPINNER_DEFAULT_SNAPSHOT  0

typedef struct spinner_snapshot spinner_snapshot_t;
struct spinner_snapshot
{
    const char *status;
    egui_color_t color;
    egui_dim_t stroke_width;
    int16_t arc_length;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_spinner_t primary_spinner;
static egui_view_label_t primary_status_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_spinner_t compact_spinner;
static egui_view_spinner_t muted_spinner;
static egui_view_api_t compact_api;
static egui_view_api_t muted_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Spinner";
static void layout_page(void);

static const spinner_snapshot_t primary_snapshots[] = {
        {"Syncing files", EGUI_COLOR_HEX(0x0F6CBD), 4, 104},
        {"Publishing docs", EGUI_COLOR_HEX(0xB95A00), 4, 112},
        {"Refreshing cache", EGUI_COLOR_HEX(0x0C7C73), 3, 88},
};

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font, egui_color_t color,
                            uint8_t align)
{
    egui_view_label_init(EGUI_VIEW_OF(label));
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align);
    egui_view_label_set_font(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void apply_primary_snapshot(uint8_t index)
{
    const spinner_snapshot_t *snapshot = &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)];

    hcw_spinner_apply_standard_style(EGUI_VIEW_OF(&primary_spinner));
    hcw_spinner_set_palette(EGUI_VIEW_OF(&primary_spinner), snapshot->color);
    hcw_spinner_set_stroke_width(EGUI_VIEW_OF(&primary_spinner), snapshot->stroke_width);
    hcw_spinner_set_arc_length(EGUI_VIEW_OF(&primary_spinner), snapshot->arc_length);
    hcw_spinner_set_spinning(EGUI_VIEW_OF(&primary_spinner), 1);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_status_label), snapshot->status);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(SPINNER_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    hcw_spinner_apply_compact_style(EGUI_VIEW_OF(&compact_spinner));
    hcw_spinner_set_spinning(EGUI_VIEW_OF(&compact_spinner), 0);
    hcw_spinner_set_rotation_angle(EGUI_VIEW_OF(&compact_spinner), 300);

    hcw_spinner_apply_muted_style(EGUI_VIEW_OF(&muted_spinner));
    hcw_spinner_set_spinning(EGUI_VIEW_OF(&muted_spinner), 0);
    hcw_spinner_set_rotation_angle(EGUI_VIEW_OF(&muted_spinner), 24);
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SPINNER_ROOT_WIDTH, SPINNER_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, SPINNER_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 6, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_spinner_init(EGUI_VIEW_OF(&primary_spinner));
    egui_view_set_size(EGUI_VIEW_OF(&primary_spinner), SPINNER_PRIMARY_SIZE, SPINNER_PRIMARY_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_spinner), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_spinner));

    init_text_label(&primary_status_label, SPINNER_STATUS_WIDTH, SPINNER_STATUS_HEIGHT, "Syncing files",
                    (const egui_font_t *)&egui_res_font_montserrat_10_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_status_label), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_status_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SPINNER_BOTTOM_ROW_WIDTH, SPINNER_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_spinner_init(EGUI_VIEW_OF(&compact_spinner));
    egui_view_set_size(EGUI_VIEW_OF(&compact_spinner), SPINNER_PREVIEW_SIZE, SPINNER_PREVIEW_SIZE);
    hcw_spinner_override_static_preview_api(EGUI_VIEW_OF(&compact_spinner), &compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_spinner), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_spinner));

    egui_view_spinner_init(EGUI_VIEW_OF(&muted_spinner));
    egui_view_set_size(EGUI_VIEW_OF(&muted_spinner), SPINNER_PREVIEW_SIZE, SPINNER_PREVIEW_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&muted_spinner), 8, 0, 0, 0);
    hcw_spinner_override_static_preview_api(EGUI_VIEW_OF(&muted_spinner), &muted_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&muted_spinner), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&muted_spinner));

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
        EGUI_SIM_SET_WAIT(p_action, SPINNER_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, SPINNER_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPINNER_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, SPINNER_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPINNER_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
        }
        EGUI_SIM_SET_WAIT(p_action, SPINNER_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPINNER_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
