#include "egui.h"
#include "egui_view_hyperlink_button.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define HYPERLINK_ROOT_WIDTH        224
#define HYPERLINK_ROOT_HEIGHT       170
#define HYPERLINK_PANEL_WIDTH       196
#define HYPERLINK_PANEL_HEIGHT      92
#define HYPERLINK_PRIMARY_WIDTH     164
#define HYPERLINK_PRIMARY_HEIGHT    24
#define HYPERLINK_PREVIEW_WIDTH     96
#define HYPERLINK_PREVIEW_HEIGHT    24
#define HYPERLINK_BOTTOM_ROW_WIDTH  200
#define HYPERLINK_BOTTOM_ROW_HEIGHT 24
#define HYPERLINK_RECORD_WAIT       90
#define HYPERLINK_RECORD_FRAME_WAIT 170
#define HYPERLINK_RECORD_FINAL_WAIT 280
#define HYPERLINK_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct hyperlink_snapshot hyperlink_snapshot_t;
struct hyperlink_snapshot
{
    const char *heading;
    const char *link_text;
    const char *note;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t panel_heading_label;
static egui_view_button_t primary_link_button;
static egui_view_api_t primary_link_api;
static egui_view_label_t note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_button_t inline_preview_button;
static egui_view_api_t inline_preview_api;
static egui_view_button_t disabled_preview_button;
static egui_view_api_t disabled_preview_api;
static uint8_t current_primary_snapshot;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

static const char *title_text = "HyperlinkButton";

static const hyperlink_snapshot_t primary_snapshots[] = {
        {
                "Project updates",
                "Open release notes",
                "Primary action keeps the lighter link affordance.",
        },
        {
                "Policy changes",
                "Review change summary",
                "Touch and keyboard still reuse the button submit path.",
        },
        {
                "Deployment checklist",
                "Browse final checklist",
                "Reference shell stays compact without extra chrome.",
        },
};

static void layout_page(void);
static void focus_primary_widget(void);

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
    const hyperlink_snapshot_t *snapshot;

    current_primary_snapshot = index % PRIMARY_SNAPSHOT_COUNT;
    snapshot = &primary_snapshots[current_primary_snapshot];
    egui_view_label_set_text(EGUI_VIEW_OF(&panel_heading_label), snapshot->heading);
    hcw_hyperlink_button_set_text(EGUI_VIEW_OF(&primary_link_button), snapshot->link_text);
    egui_view_label_set_text(EGUI_VIEW_OF(&note_label), snapshot->note);

    if (ui_ready)
    {
        layout_page();
        focus_primary_widget();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(HYPERLINK_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    hcw_hyperlink_button_set_text(EGUI_VIEW_OF(&inline_preview_button), "Inline article");

    hcw_hyperlink_button_set_text(EGUI_VIEW_OF(&disabled_preview_button), "Archived link");
    egui_view_set_enable(EGUI_VIEW_OF(&disabled_preview_button), 0);

    if (ui_ready)
    {
        layout_page();
    }
}

static void layout_local_views(void)
{
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_panel));
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
    egui_view_request_focus(EGUI_VIEW_OF(&primary_link_button));
#endif
}

static void on_primary_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    apply_primary_snapshot((uint8_t)((current_primary_snapshot + 1U) % PRIMARY_SNAPSHOT_COUNT));
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), HYPERLINK_ROOT_WIDTH, HYPERLINK_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, HYPERLINK_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&primary_panel));
    egui_view_set_size(EGUI_VIEW_OF(&primary_panel), HYPERLINK_PANEL_WIDTH, HYPERLINK_PANEL_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&primary_panel), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&primary_panel), EGUI_ALIGN_LEFT);
    egui_view_set_background(EGUI_VIEW_OF(&primary_panel), EGUI_BG_OF(&bg_surface_panel));
    egui_view_set_padding(EGUI_VIEW_OF(&primary_panel), 12, 12, 12, 10);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&panel_heading_label, 172, 12, "Project updates", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x51606F),
                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&panel_heading_label), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&panel_heading_label));

    egui_view_button_init(EGUI_VIEW_OF(&primary_link_button));
    egui_view_set_size(EGUI_VIEW_OF(&primary_link_button), HYPERLINK_PRIMARY_WIDTH, HYPERLINK_PRIMARY_HEIGHT);
    hcw_hyperlink_button_apply_standard_style(EGUI_VIEW_OF(&primary_link_button));
    hcw_hyperlink_button_set_font(EGUI_VIEW_OF(&primary_link_button), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    hcw_hyperlink_button_override_interaction_api(EGUI_VIEW_OF(&primary_link_button), &primary_link_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&primary_link_button), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&primary_link_button), 0, 0, 0, 10);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&primary_link_button), on_primary_click);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_link_button));

    init_text_label(&note_label, 172, 20, "Primary action keeps the lighter link affordance.", (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), HYPERLINK_BOTTOM_ROW_WIDTH, HYPERLINK_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_button_init(EGUI_VIEW_OF(&inline_preview_button));
    egui_view_set_size(EGUI_VIEW_OF(&inline_preview_button), HYPERLINK_PREVIEW_WIDTH, HYPERLINK_PREVIEW_HEIGHT);
    hcw_hyperlink_button_apply_inline_style(EGUI_VIEW_OF(&inline_preview_button));
    hcw_hyperlink_button_set_font(EGUI_VIEW_OF(&inline_preview_button), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_hyperlink_button_override_static_preview_api(EGUI_VIEW_OF(&inline_preview_button), &inline_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&inline_preview_button), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&inline_preview_button));

    egui_view_button_init(EGUI_VIEW_OF(&disabled_preview_button));
    egui_view_set_size(EGUI_VIEW_OF(&disabled_preview_button), HYPERLINK_PREVIEW_WIDTH, HYPERLINK_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&disabled_preview_button), 8, 0, 0, 0);
    hcw_hyperlink_button_apply_disabled_style(EGUI_VIEW_OF(&disabled_preview_button));
    hcw_hyperlink_button_set_font(EGUI_VIEW_OF(&disabled_preview_button), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_hyperlink_button_override_static_preview_api(EGUI_VIEW_OF(&disabled_preview_button), &disabled_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&disabled_preview_button), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&disabled_preview_button));

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
            focus_primary_widget();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, HYPERLINK_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, HYPERLINK_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, HYPERLINK_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, HYPERLINK_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, HYPERLINK_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
        }
        EGUI_SIM_SET_WAIT(p_action, HYPERLINK_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, HYPERLINK_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
