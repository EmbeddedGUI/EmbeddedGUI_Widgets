#include "egui.h"
#include "egui_view_field.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define FIELD_ROOT_WIDTH        224
#define FIELD_ROOT_HEIGHT       248
#define FIELD_PRIMARY_WIDTH     196
#define FIELD_PRIMARY_HEIGHT    126
#define FIELD_PREVIEW_WIDTH      84
#define FIELD_PREVIEW_HEIGHT     66
#define FIELD_PREVIEW_PANEL_W   104
#define FIELD_PREVIEW_PANEL_H    96
#define FIELD_BOTTOM_ROW_WIDTH  216
#define FIELD_BOTTOM_ROW_HEIGHT  96
#define FIELD_RECORD_WAIT        90
#define FIELD_RECORD_FRAME_WAIT 170
#define FIELD_RECORD_FINAL_WAIT 520

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
static egui_view_linearlayout_t compact_panel;
static egui_view_label_t compact_heading_label;
static hcw_field_t compact_field;
static egui_view_linearlayout_t read_only_panel;
static egui_view_label_t read_only_heading_label;
static hcw_field_t read_only_field;
static egui_view_api_t compact_field_api;
static egui_view_api_t read_only_field_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, EGUI_COLOR_HEX(0xFBFCFD), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

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
                "Needed to refresh the reference feed.",
                "Required before handoff.",
                "Why required",
                "Field keeps label, helper and validation aligned with a low-noise info entry.",
                1,
                HCW_FIELD_VALIDATION_WARNING,
                0,
        },
        {
                "Retention window",
                "14 days",
                "",
                "Matches current export policy.",
                "Review before publishing.",
                "Policy note",
                "Warning state stays compact while keeping the field shell stable.",
                0,
                HCW_FIELD_VALIDATION_WARNING,
                0,
        },
        {
                "Owner alias",
                "",
                "team@example.com",
                "Used by audit routing.",
                "Enter a valid alias before saving.",
                "Alias format",
                "Use the shared team mailbox instead of a personal address.",
                1,
                HCW_FIELD_VALIDATION_ERROR,
                0,
        },
        {
                "Approval mail",
                "ops@studio.dev",
                "",
                "Pending final sign-off.",
                "Address format looks good.",
                "Validation",
                "Success keeps the message subtle while the field remains lightweight.",
                0,
                HCW_FIELD_VALIDATION_SUCCESS,
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
        "Locked by policy.",
        "",
        "",
        0,
        HCW_FIELD_VALIDATION_SUCCESS,
        0,
};

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font, egui_color_t color,
                            uint8_t align_type)
{
    egui_view_label_init(EGUI_VIEW_OF(label));
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align_type);
    egui_view_label_set_font(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void init_panel(egui_view_linearlayout_t *panel, egui_dim_t width, egui_dim_t height, egui_background_t *background, uint8_t align_type)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(panel));
    egui_view_set_size(EGUI_VIEW_OF(panel), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(panel), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(panel), align_type);
    egui_view_set_background(EGUI_VIEW_OF(panel), background);
    egui_view_set_padding(EGUI_VIEW_OF(panel), 8, 8, 8, 8);
}

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
    apply_snapshot(EGUI_VIEW_OF(&primary_field), &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)]);
}

static void apply_compact_state(void)
{
    hcw_field_apply_compact_style(EGUI_VIEW_OF(&compact_field));
    apply_snapshot(EGUI_VIEW_OF(&compact_field), &compact_snapshot);
}

static void apply_read_only_state(void)
{
    hcw_field_apply_compact_style(EGUI_VIEW_OF(&read_only_field));
    apply_snapshot(EGUI_VIEW_OF(&read_only_field), &read_only_snapshot);
    hcw_field_set_read_only_mode(EGUI_VIEW_OF(&read_only_field), 1);
}

#if EGUI_CONFIG_RECORDING_TEST
static void set_click_field_info_button(egui_sim_action_t *p_action, egui_view_t *view, int interval_ms)
{
    egui_region_t region;

    if (!hcw_field_get_part_region(view, HCW_FIELD_PART_INFO_BUTTON, &region))
    {
        EGUI_SIM_SET_WAIT(p_action, interval_ms);
        return;
    }

    p_action->type = EGUI_SIM_ACTION_CLICK;
    p_action->x1 = region.location.x + region.size.width / 2;
    p_action->y1 = region.location.y + region.size.height / 2;
    p_action->x2 = 0;
    p_action->y2 = 0;
    p_action->steps = 0;
    p_action->interval_ms = interval_ms;
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), FIELD_ROOT_WIDTH, FIELD_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, FIELD_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    hcw_field_init(EGUI_VIEW_OF(&primary_field));
    egui_view_set_size(EGUI_VIEW_OF(&primary_field), FIELD_PRIMARY_WIDTH, FIELD_PRIMARY_HEIGHT);
    hcw_field_set_font(EGUI_VIEW_OF(&primary_field), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_field_set_meta_font(EGUI_VIEW_OF(&primary_field), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_field_set_icon_font(EGUI_VIEW_OF(&primary_field), EGUI_FONT_ICON_MS_16);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_field), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_field));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), FIELD_BOTTOM_ROW_WIDTH, FIELD_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&compact_panel, FIELD_PREVIEW_PANEL_W, FIELD_PREVIEW_PANEL_H, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_panel));

    init_text_label(&compact_heading_label, 84, 12, "Compact", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_heading_label));

    hcw_field_init(EGUI_VIEW_OF(&compact_field));
    egui_view_set_size(EGUI_VIEW_OF(&compact_field), FIELD_PREVIEW_WIDTH, FIELD_PREVIEW_HEIGHT);
    hcw_field_set_font(EGUI_VIEW_OF(&compact_field), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_field_set_meta_font(EGUI_VIEW_OF(&compact_field), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_field_set_icon_font(EGUI_VIEW_OF(&compact_field), EGUI_FONT_ICON_MS_16);
    hcw_field_override_static_preview_api(EGUI_VIEW_OF(&compact_field), &compact_field_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_field), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_field));

    init_panel(&read_only_panel, FIELD_PREVIEW_PANEL_W, FIELD_PREVIEW_PANEL_H, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_panel));

    init_text_label(&read_only_heading_label, 84, 12, "Read only", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_panel), EGUI_VIEW_OF(&read_only_heading_label));

    hcw_field_init(EGUI_VIEW_OF(&read_only_field));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_field), FIELD_PREVIEW_WIDTH, FIELD_PREVIEW_HEIGHT);
    hcw_field_set_font(EGUI_VIEW_OF(&read_only_field), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_field_set_meta_font(EGUI_VIEW_OF(&read_only_field), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_field_set_icon_font(EGUI_VIEW_OF(&read_only_field), EGUI_FONT_ICON_MS_16);
    hcw_field_override_static_preview_api(EGUI_VIEW_OF(&read_only_field), &read_only_field_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&read_only_field), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_panel), EGUI_VIEW_OF(&read_only_field));

    apply_primary_snapshot(0);
    apply_compact_state();
    apply_read_only_state();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&read_only_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
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
            apply_primary_snapshot(0);
            apply_compact_state();
            apply_read_only_state();
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_WAIT);
        return true;
    case 3:
        set_click_field_info_button(p_action, EGUI_VIEW_OF(&primary_field), FIELD_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_FRAME_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            apply_primary_snapshot(4);
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FIELD_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
