#include "egui.h"
#include "egui_view_divider.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define DIVIDER_ROOT_WIDTH        224
#define DIVIDER_ROOT_HEIGHT       208
#define DIVIDER_PANEL_WIDTH       196
#define DIVIDER_PANEL_HEIGHT      84
#define DIVIDER_PREVIEW_WIDTH     104
#define DIVIDER_PREVIEW_HEIGHT    72
#define DIVIDER_BOTTOM_ROW_WIDTH  216
#define DIVIDER_BOTTOM_ROW_HEIGHT 72
#define DIVIDER_RECORD_WAIT       90
#define DIVIDER_RECORD_FRAME_WAIT 170

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t primary_heading_label;
static egui_view_divider_t primary_divider;
static egui_view_label_t primary_body_label;
static egui_view_label_t primary_note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t subtle_panel;
static egui_view_label_t subtle_heading_label;
static egui_view_divider_t subtle_divider;
static egui_view_label_t subtle_body_label;
static egui_view_linearlayout_t accent_panel;
static egui_view_label_t accent_heading_label;
static egui_view_divider_t accent_divider;
static egui_view_label_t accent_body_label;
static egui_view_api_t subtle_divider_api;
static egui_view_api_t accent_divider_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, EGUI_COLOR_HEX(0xFBFCFD), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

static const char *title_text = "Separator";
static const char *primary_heading_texts[] = {
        "Overview section",
        "Active section",
        "Muted section",
};
static const char *primary_note_texts[] = {
        "Neutral tone stays quiet.",
        "Accent tone leads detail.",
        "Subtle tone softens density.",
};

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font, egui_color_t color)
{
    egui_view_label_init(EGUI_VIEW_OF(label));
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void init_panel(egui_view_linearlayout_t *panel, egui_dim_t width, egui_dim_t height, egui_background_t *background)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(panel));
    egui_view_set_size(EGUI_VIEW_OF(panel), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(panel), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(panel), EGUI_ALIGN_LEFT);
    egui_view_set_background(EGUI_VIEW_OF(panel), background);
    egui_view_set_padding(EGUI_VIEW_OF(panel), 10, 10, 10, 10);
}

static void apply_primary_variant(uint8_t variant)
{
    uint8_t safe_variant = (uint8_t)(variant % EGUI_ARRAY_SIZE(primary_heading_texts));

    egui_view_label_set_text(EGUI_VIEW_OF(&primary_heading_label), primary_heading_texts[safe_variant]);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_note_label), primary_note_texts[safe_variant]);

    switch (safe_variant)
    {
    case 1:
        hcw_divider_apply_accent_style(EGUI_VIEW_OF(&primary_divider));
        break;
    case 2:
        hcw_divider_apply_subtle_style(EGUI_VIEW_OF(&primary_divider));
        break;
    default:
        hcw_divider_apply_standard_style(EGUI_VIEW_OF(&primary_divider));
        break;
    }
}

static void apply_preview_variants(void)
{
    hcw_divider_apply_subtle_style(EGUI_VIEW_OF(&subtle_divider));
    hcw_divider_apply_accent_style(EGUI_VIEW_OF(&accent_divider));
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), DIVIDER_ROOT_WIDTH, DIVIDER_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), DIVIDER_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_panel(&primary_panel, DIVIDER_PANEL_WIDTH, DIVIDER_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel));
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&primary_heading_label, 176, 14, primary_heading_texts[0], (const egui_font_t *)&egui_res_font_montserrat_10_4, EGUI_COLOR_HEX(0x1A2734));
    egui_view_set_margin(EGUI_VIEW_OF(&primary_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_heading_label));

    egui_view_divider_init(EGUI_VIEW_OF(&primary_divider));
    egui_view_set_size(EGUI_VIEW_OF(&primary_divider), 176, 2);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_divider), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_divider));

    init_text_label(&primary_body_label, 176, 10, "Split related content.", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x5F6E7D));
    egui_view_set_margin(EGUI_VIEW_OF(&primary_body_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_body_label));

    init_text_label(&primary_note_label, 176, 10, primary_note_texts[0], (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x5F6E7D));
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), DIVIDER_BOTTOM_ROW_WIDTH, DIVIDER_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&subtle_panel, DIVIDER_PREVIEW_WIDTH, DIVIDER_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_muted_panel));
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&subtle_panel));

    init_text_label(&subtle_heading_label, 84, 12, "Subtle", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241));
    egui_view_set_margin(EGUI_VIEW_OF(&subtle_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&subtle_panel), EGUI_VIEW_OF(&subtle_heading_label));

    egui_view_divider_init(EGUI_VIEW_OF(&subtle_divider));
    egui_view_set_size(EGUI_VIEW_OF(&subtle_divider), 84, 1);
    egui_view_set_margin(EGUI_VIEW_OF(&subtle_divider), 0, 0, 0, 8);
    hcw_divider_override_static_preview_api(EGUI_VIEW_OF(&subtle_divider), &subtle_divider_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&subtle_divider), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&subtle_panel), EGUI_VIEW_OF(&subtle_divider));

    init_text_label(&subtle_body_label, 84, 10, "Secondary split", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89));
    egui_view_group_add_child(EGUI_VIEW_OF(&subtle_panel), EGUI_VIEW_OF(&subtle_body_label));

    init_panel(&accent_panel, DIVIDER_PREVIEW_WIDTH, DIVIDER_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_surface_panel));
    egui_view_set_margin(EGUI_VIEW_OF(&accent_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&accent_panel));

    init_text_label(&accent_heading_label, 84, 12, "Accent", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x1A2734));
    egui_view_set_margin(EGUI_VIEW_OF(&accent_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&accent_panel), EGUI_VIEW_OF(&accent_heading_label));

    egui_view_divider_init(EGUI_VIEW_OF(&accent_divider));
    egui_view_set_size(EGUI_VIEW_OF(&accent_divider), 84, 2);
    egui_view_set_margin(EGUI_VIEW_OF(&accent_divider), 0, 0, 0, 7);
    hcw_divider_override_static_preview_api(EGUI_VIEW_OF(&accent_divider), &accent_divider_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&accent_divider), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&accent_panel), EGUI_VIEW_OF(&accent_divider));

    init_text_label(&accent_body_label, 84, 10, "Active split", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_group_add_child(EGUI_VIEW_OF(&accent_panel), EGUI_VIEW_OF(&accent_body_label));

    apply_primary_variant(0);
    apply_preview_variants();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&subtle_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&accent_panel));
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
            apply_primary_variant(0);
            apply_preview_variants();
        }
        EGUI_SIM_SET_WAIT(p_action, DIVIDER_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DIVIDER_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_variant(1);
        }
        EGUI_SIM_SET_WAIT(p_action, DIVIDER_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DIVIDER_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_variant(2);
        }
        EGUI_SIM_SET_WAIT(p_action, DIVIDER_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DIVIDER_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_variant(0);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
