#include "egui.h"
#include "egui_view_divider.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define DIVIDER_ROOT_WIDTH        224
#define DIVIDER_ROOT_HEIGHT       132
#define DIVIDER_PRIMARY_WIDTH     176
#define DIVIDER_PRIMARY_HEIGHT    2
#define DIVIDER_PREVIEW_WIDTH     36
#define DIVIDER_PREVIEW_HEIGHT    2
#define DIVIDER_BOTTOM_ROW_WIDTH  80
#define DIVIDER_BOTTOM_ROW_HEIGHT 2
#define DIVIDER_RECORD_WAIT       90
#define DIVIDER_RECORD_FRAME_WAIT 170
#define DIVIDER_RECORD_FINAL_WAIT 280
#define DIVIDER_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct divider_snapshot divider_snapshot_t;
struct divider_snapshot
{
    uint8_t variant;
    const char *status_label;
    egui_color_t status_label_color;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_divider_t primary_divider;
static egui_view_label_t primary_status_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_divider_t subtle_divider;
static egui_view_divider_t accent_divider;
static egui_view_api_t subtle_divider_api;
static egui_view_api_t accent_divider_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Separator";

static const divider_snapshot_t primary_snapshots[] = {
        {
                0,
                "Standard / neutral",
                EGUI_COLOR_HEX(0x51606F),
        },
        {
                1,
                "Accent / emphasis",
                EGUI_COLOR_HEX(0x0F6CBD),
        },
        {
                2,
                "Subtle / muted",
                EGUI_COLOR_HEX(0x6F7C8A),
        },
};

static void layout_page(void);

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
    const divider_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    switch (snapshot->variant)
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

    egui_view_label_set_text(EGUI_VIEW_OF(&primary_status_label), snapshot->status_label);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_status_label), snapshot->status_label_color, EGUI_ALPHA_100);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(DIVIDER_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    hcw_divider_apply_subtle_style(EGUI_VIEW_OF(&subtle_divider));
    hcw_divider_apply_accent_style(EGUI_VIEW_OF(&accent_divider));

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
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), DIVIDER_ROOT_WIDTH, DIVIDER_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, DIVIDER_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 18);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_divider_init(EGUI_VIEW_OF(&primary_divider));
    egui_view_set_size(EGUI_VIEW_OF(&primary_divider), DIVIDER_PRIMARY_WIDTH, DIVIDER_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_divider), 0, 0, 0, 10);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&primary_divider), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_divider));

    init_text_label(&primary_status_label, DIVIDER_ROOT_WIDTH, 12, "Standard / neutral", (const egui_font_t *)&egui_res_font_montserrat_10_4,
                    EGUI_COLOR_HEX(0x51606F), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_status_label), 0, 0, 0, 16);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_status_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), DIVIDER_BOTTOM_ROW_WIDTH, DIVIDER_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_divider_init(EGUI_VIEW_OF(&subtle_divider));
    egui_view_set_size(EGUI_VIEW_OF(&subtle_divider), DIVIDER_PREVIEW_WIDTH, DIVIDER_PREVIEW_HEIGHT);
    hcw_divider_override_static_preview_api(EGUI_VIEW_OF(&subtle_divider), &subtle_divider_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&subtle_divider), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&subtle_divider));

    egui_view_divider_init(EGUI_VIEW_OF(&accent_divider));
    egui_view_set_size(EGUI_VIEW_OF(&accent_divider), DIVIDER_PREVIEW_WIDTH, DIVIDER_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&accent_divider), 8, 0, 0, 0);
    hcw_divider_override_static_preview_api(EGUI_VIEW_OF(&accent_divider), &accent_divider_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&accent_divider), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&accent_divider));

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
        EGUI_SIM_SET_WAIT(p_action, DIVIDER_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, DIVIDER_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DIVIDER_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, DIVIDER_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DIVIDER_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DIVIDER_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
