#include "egui.h"
#include "egui_view_path.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define PATH_ROOT_WIDTH        238
#define PATH_ROOT_HEIGHT       180
#define PATH_PRIMARY_WIDTH     144
#define PATH_PRIMARY_HEIGHT    64
#define PATH_PREVIEW_WIDTH     72
#define PATH_PREVIEW_HEIGHT    34
#define PATH_PREVIEW_ROW_WIDTH 164
#define PATH_RECORD_WAIT       90
#define PATH_RECORD_FRAME_WAIT 170
#define PATH_RECORD_FINAL_WAIT 280
#define PATH_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct path_snapshot path_snapshot_t;
struct path_snapshot
{
    const char *caption;
    egui_color_t caption_color;
    uint8_t style;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_path_t primary_control;
static egui_view_label_t caption_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_path_t line_preview;
static egui_view_path_t muted_preview;
static egui_view_api_t line_preview_api;
static egui_view_api_t muted_preview_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, HCW_COLOR_PAGE_BG, EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Path";

static const path_snapshot_t primary_snapshots[] = {
        {"Standard / geometry", HCW_COLOR_PRIMARY, 0},
        {"Accent / curve", HCW_COLOR_PRIMARY, 1},
        {"Line data", HCW_COLOR_PRIMARY, 2},
        {"Muted silhouette", HCW_COLOR_TEXT_MUTED, 3},
};

static void layout_page(void);

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font,
                            egui_color_t color, uint8_t align_type)
{
    egui_view_label_init(EGUI_VIEW_OF(label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align_type);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void apply_path_line_style(egui_view_t *view)
{
    egui_view_path_set_palette(view, HCW_COLOR_PRIMARY_TINT, HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY_SOFT);
    egui_view_path_set_stroke_width(view, 1);
    egui_view_path_set_data(view, egui_view_path_get_line_data());
}

static void apply_path_muted_style(egui_view_t *view)
{
    egui_view_path_set_palette(view, HCW_COLOR_BORDER, HCW_COLOR_TEXT_SOFT, HCW_COLOR_BORDER_STRONG);
    egui_view_path_set_stroke_width(view, 1);
    egui_view_path_set_data(view, egui_view_path_get_bookmark_data());
}

static void apply_path_style(egui_view_t *view, uint8_t style)
{
    switch (style)
    {
    case 1:
        egui_view_path_apply_accent_style(view);
        break;
    case 2:
        apply_path_line_style(view);
        break;
    case 3:
        apply_path_muted_style(view);
        break;
    default:
        egui_view_path_apply_standard_style(view);
        break;
    }
}

static void apply_primary_snapshot(uint8_t index)
{
    const path_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    apply_path_style(EGUI_VIEW_OF(&primary_control), snapshot->style);
    egui_view_label_set_text(EGUI_VIEW_OF(&caption_label), snapshot->caption);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&caption_label), snapshot->caption_color, EGUI_ALPHA_100);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(PATH_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    apply_path_line_style(EGUI_VIEW_OF(&line_preview));
    apply_path_muted_style(EGUI_VIEW_OF(&muted_preview));

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
    egui_core_layout_childs_user_root_view(uicode_get_core(), EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
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

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), PATH_ROOT_WIDTH, PATH_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, PATH_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4,
                    HCW_COLOR_TEXT, EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_path_init(EGUI_VIEW_OF(&primary_control));
    egui_view_set_size(EGUI_VIEW_OF(&primary_control), PATH_PRIMARY_WIDTH, PATH_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_control), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_control));

    init_text_label(&caption_label, PATH_ROOT_WIDTH, 12, "Standard / geometry", (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    HCW_COLOR_PRIMARY, EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&caption_label), 0, 0, 0, 14);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&caption_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), PATH_PREVIEW_ROW_WIDTH, PATH_PREVIEW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_path_init(EGUI_VIEW_OF(&line_preview));
    egui_view_set_size(EGUI_VIEW_OF(&line_preview), PATH_PREVIEW_WIDTH, PATH_PREVIEW_HEIGHT);
    egui_view_path_override_static_preview_api(EGUI_VIEW_OF(&line_preview), &line_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&line_preview), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&line_preview));

    egui_view_path_init(EGUI_VIEW_OF(&muted_preview));
    egui_view_set_size(EGUI_VIEW_OF(&muted_preview), PATH_PREVIEW_WIDTH, PATH_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&muted_preview), 16, 0, 0, 0);
    egui_view_path_override_static_preview_api(EGUI_VIEW_OF(&muted_preview), &muted_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&muted_preview), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&muted_preview));

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
        EGUI_SIM_SET_WAIT(p_action, PATH_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, PATH_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PATH_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, PATH_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PATH_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, PATH_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PATH_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PATH_RECORD_FINAL_WAIT);
        return true;
    default:
        break;
    }
    return false;
}
#endif
