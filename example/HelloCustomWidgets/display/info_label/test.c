#include "egui.h"
#include "egui_view_info_label.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define INFO_LABEL_ROOT_WIDTH        224
#define INFO_LABEL_ROOT_HEIGHT       228
#define INFO_LABEL_PRIMARY_WIDTH     196
#define INFO_LABEL_PRIMARY_HEIGHT    96
#define INFO_LABEL_PREVIEW_WIDTH     84
#define INFO_LABEL_PREVIEW_HEIGHT    54
#define INFO_LABEL_PREVIEW_PANEL_W   104
#define INFO_LABEL_PREVIEW_PANEL_H   88
#define INFO_LABEL_BOTTOM_ROW_WIDTH  216
#define INFO_LABEL_BOTTOM_ROW_HEIGHT 88
#define INFO_LABEL_RECORD_WAIT       90
#define INFO_LABEL_RECORD_FRAME_WAIT 170
#define INFO_LABEL_RECORD_FINAL_WAIT 520

typedef struct
{
    const char *label;
    const char *title;
    const char *body;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t bubble_surface_color;
    egui_color_t shadow_color;
} info_label_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static hcw_info_label_t primary_info_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_panel;
static egui_view_label_t compact_heading_label;
static hcw_info_label_t compact_info_label;
static egui_view_linearlayout_t read_only_panel;
static egui_view_label_t read_only_heading_label;
static hcw_info_label_t read_only_info_label;
static egui_view_api_t compact_info_label_api;
static egui_view_api_t read_only_info_label_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, EGUI_COLOR_HEX(0xFBFCFD), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

static const char *title_text = "InfoLabel";

static const info_label_snapshot_t primary_snapshots[] = {
        {
                "Project policy",
                "Versioning",
                "Keep release notes aligned with the approved branch.",
                EGUI_COLOR_HEX(0xFFFFFF),
                EGUI_COLOR_HEX(0xD4DDE5),
                EGUI_COLOR_HEX(0x1B2835),
                EGUI_COLOR_HEX(0x697A8B),
                EGUI_COLOR_HEX(0x0F6CBD),
                EGUI_COLOR_HEX(0xF9FBFD),
                EGUI_COLOR_HEX(0xDCE4EA),
        },
        {
                "Export guidance",
                "Sensitive content",
                "Mask personal data before sharing outside the tenant.",
                EGUI_COLOR_HEX(0xFFFFFF),
                EGUI_COLOR_HEX(0xE1D3C3),
                EGUI_COLOR_HEX(0x3A2A1A),
                EGUI_COLOR_HEX(0x8A6F55),
                EGUI_COLOR_HEX(0xA15C00),
                EGUI_COLOR_HEX(0xFFF8F1),
                EGUI_COLOR_HEX(0xE8DDD0),
        },
        {
                "Reading help",
                "Reference note",
                "Use the compact preview when the layout has limited width.",
                EGUI_COLOR_HEX(0xFBFCFD),
                EGUI_COLOR_HEX(0xD7DEE6),
                EGUI_COLOR_HEX(0x334252),
                EGUI_COLOR_HEX(0x778594),
                EGUI_COLOR_HEX(0x6F8194),
                EGUI_COLOR_HEX(0xF6F8FA),
                EGUI_COLOR_HEX(0xD9E1E8),
        },
};

static const info_label_snapshot_t compact_snapshot = {
        "Compact help",
        "Inline note",
        "Compact mode keeps the bubble short.",
        EGUI_COLOR_HEX(0xFFFFFF),
        EGUI_COLOR_HEX(0xD6DEE6),
        EGUI_COLOR_HEX(0x22303C),
        EGUI_COLOR_HEX(0x73808C),
        EGUI_COLOR_HEX(0x0C7C73),
        EGUI_COLOR_HEX(0xF7FBFA),
        EGUI_COLOR_HEX(0xD7E7E4),
};

static const info_label_snapshot_t read_only_snapshot = {
        "Audit note",
        "Read only",
        "Static preview keeps input disabled.",
        EGUI_COLOR_HEX(0xFBFCFD),
        EGUI_COLOR_HEX(0xD9E1E8),
        EGUI_COLOR_HEX(0x566675),
        EGUI_COLOR_HEX(0x8895A1),
        EGUI_COLOR_HEX(0xA8B6C2),
        EGUI_COLOR_HEX(0xF7F9FB),
        EGUI_COLOR_HEX(0xE3E9EE),
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

static void apply_snapshot(egui_view_t *view, const info_label_snapshot_t *snapshot, uint8_t is_open)
{
    hcw_info_label_set_text(view, snapshot->label);
    hcw_info_label_set_info_title(view, snapshot->title);
    hcw_info_label_set_info_body(view, snapshot->body);
    hcw_info_label_set_palette(view, snapshot->surface_color, snapshot->border_color, snapshot->text_color, snapshot->muted_text_color, snapshot->accent_color,
                               snapshot->bubble_surface_color, snapshot->shadow_color);
    hcw_info_label_set_open(view, is_open);
}

static void apply_primary_snapshot(uint8_t index, uint8_t is_open)
{
    apply_snapshot(EGUI_VIEW_OF(&primary_info_label), &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)], is_open);
}

static void apply_compact_state(void)
{
    hcw_info_label_apply_compact_style(EGUI_VIEW_OF(&compact_info_label));
    apply_snapshot(EGUI_VIEW_OF(&compact_info_label), &compact_snapshot, 1);
}

static void apply_read_only_state(void)
{
    hcw_info_label_apply_compact_style(EGUI_VIEW_OF(&read_only_info_label));
    apply_snapshot(EGUI_VIEW_OF(&read_only_info_label), &read_only_snapshot, 1);
    hcw_info_label_set_read_only_mode(EGUI_VIEW_OF(&read_only_info_label), 1);
}

#if EGUI_CONFIG_RECORDING_TEST
static void set_click_info_label_icon(egui_sim_action_t *p_action, egui_view_t *view, int interval_ms)
{
    egui_region_t region;

    if (!hcw_info_label_get_part_region(view, HCW_INFO_LABEL_PART_ICON, &region))
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), INFO_LABEL_ROOT_WIDTH, INFO_LABEL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, INFO_LABEL_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    hcw_info_label_init(EGUI_VIEW_OF(&primary_info_label));
    egui_view_set_size(EGUI_VIEW_OF(&primary_info_label), INFO_LABEL_PRIMARY_WIDTH, INFO_LABEL_PRIMARY_HEIGHT);
    hcw_info_label_set_font(EGUI_VIEW_OF(&primary_info_label), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_info_label_set_meta_font(EGUI_VIEW_OF(&primary_info_label), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_info_label_set_icon_font(EGUI_VIEW_OF(&primary_info_label), EGUI_FONT_ICON_MS_16);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_info_label), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_info_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), INFO_LABEL_BOTTOM_ROW_WIDTH, INFO_LABEL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&compact_panel, INFO_LABEL_PREVIEW_PANEL_W, INFO_LABEL_PREVIEW_PANEL_H, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_panel));

    init_text_label(&compact_heading_label, 84, 12, "Compact", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_heading_label));

    hcw_info_label_init(EGUI_VIEW_OF(&compact_info_label));
    egui_view_set_size(EGUI_VIEW_OF(&compact_info_label), INFO_LABEL_PREVIEW_WIDTH, INFO_LABEL_PREVIEW_HEIGHT);
    hcw_info_label_set_font(EGUI_VIEW_OF(&compact_info_label), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_info_label_set_meta_font(EGUI_VIEW_OF(&compact_info_label), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_info_label_set_icon_font(EGUI_VIEW_OF(&compact_info_label), EGUI_FONT_ICON_MS_16);
    hcw_info_label_override_static_preview_api(EGUI_VIEW_OF(&compact_info_label), &compact_info_label_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_info_label), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_info_label));

    init_panel(&read_only_panel, INFO_LABEL_PREVIEW_PANEL_W, INFO_LABEL_PREVIEW_PANEL_H, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_panel));

    init_text_label(&read_only_heading_label, 84, 12, "Read only", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_panel), EGUI_VIEW_OF(&read_only_heading_label));

    hcw_info_label_init(EGUI_VIEW_OF(&read_only_info_label));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_info_label), INFO_LABEL_PREVIEW_WIDTH, INFO_LABEL_PREVIEW_HEIGHT);
    hcw_info_label_set_font(EGUI_VIEW_OF(&read_only_info_label), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_info_label_set_meta_font(EGUI_VIEW_OF(&read_only_info_label), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_info_label_set_icon_font(EGUI_VIEW_OF(&read_only_info_label), EGUI_FONT_ICON_MS_16);
    hcw_info_label_override_static_preview_api(EGUI_VIEW_OF(&read_only_info_label), &read_only_info_label_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&read_only_info_label), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_panel), EGUI_VIEW_OF(&read_only_info_label));

    apply_primary_snapshot(0, 0);
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
            apply_primary_snapshot(0, 0);
            apply_compact_state();
            apply_read_only_state();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_LABEL_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_LABEL_RECORD_FRAME_WAIT);
        return true;
    case 2:
        set_click_info_label_icon(p_action, EGUI_VIEW_OF(&primary_info_label), INFO_LABEL_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_LABEL_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(1, 1);
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_LABEL_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_LABEL_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(2, 1);
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_LABEL_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_LABEL_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
