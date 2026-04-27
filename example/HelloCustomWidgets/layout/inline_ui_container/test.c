#include "egui.h"
#include "egui_view_inline_ui_container.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define INLINE_UI_ROOT_WIDTH        238
#define INLINE_UI_ROOT_HEIGHT       180
#define INLINE_UI_PRIMARY_WIDTH     184
#define INLINE_UI_PRIMARY_HEIGHT    42
#define INLINE_UI_CHILD_WIDTH       58
#define INLINE_UI_CHILD_HEIGHT      18
#define INLINE_UI_PREVIEW_ROW_WIDTH 196
#define INLINE_UI_PREVIEW_WIDTH     92
#define INLINE_UI_PREVIEW_HEIGHT    30
#define INLINE_UI_PREVIEW_CHILD_W   48
#define INLINE_UI_PREVIEW_CHILD_H   16
#define INLINE_UI_RECORD_WAIT       90
#define INLINE_UI_RECORD_FRAME_WAIT 170
#define INLINE_UI_RECORD_FINAL_WAIT 280
#define INLINE_UI_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct inline_ui_snapshot inline_ui_snapshot_t;
struct inline_ui_snapshot
{
    const char *prefix_text;
    const char *child_text;
    const char *suffix_text;
    const char *caption;
    egui_color_t caption_color;
    uint8_t style;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_inline_ui_container_t primary_control;
static egui_view_label_t primary_child;
static egui_view_label_t caption_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_inline_ui_container_t compact_preview;
static egui_view_label_t compact_child;
static egui_view_inline_ui_container_t read_only_preview;
static egui_view_label_t read_only_child;
static egui_view_api_t compact_preview_api;
static egui_view_api_t read_only_preview_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "InlineUIContainer";

static const inline_ui_snapshot_t primary_snapshots[] = {
        {"Before", "Action", "after", "Inline child host", EGUI_COLOR_HEX(0x0F6CBD), 0},
        {"Status", "Ready", "now", "Baseline accent", EGUI_COLOR_HEX(0x0F6CBD), 1},
        {"Fit", "Tag", "inline", "Compact inline", EGUI_COLOR_HEX(0x0C7C73), 2},
        {"Audit", "Locked", "only", "Read only inline", EGUI_COLOR_HEX(0x65717E), 3},
};

static void layout_page(void);

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font,
                            egui_color_t color)
{
    egui_view_label_init(EGUI_VIEW_OF(label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void apply_inline_ui_style(egui_view_t *view, uint8_t style)
{
    switch (style)
    {
    case 1:
        egui_view_inline_ui_container_apply_accent_style(view);
        break;
    case 2:
        egui_view_inline_ui_container_apply_compact_style(view);
        break;
    case 3:
        egui_view_inline_ui_container_apply_read_only_style(view);
        break;
    default:
        egui_view_inline_ui_container_apply_standard_style(view);
        break;
    }
}

static void apply_primary_snapshot(uint8_t index)
{
    const inline_ui_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    apply_inline_ui_style(EGUI_VIEW_OF(&primary_control), snapshot->style);
    egui_view_inline_ui_container_set_text(EGUI_VIEW_OF(&primary_control), snapshot->prefix_text, snapshot->suffix_text);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_child), snapshot->child_text);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_child), snapshot->caption_color, EGUI_ALPHA_100);
    egui_view_label_set_text(EGUI_VIEW_OF(&caption_label), snapshot->caption);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&caption_label), snapshot->caption_color, EGUI_ALPHA_100);
    egui_view_inline_ui_container_layout_child(EGUI_VIEW_OF(&primary_control));
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(INLINE_UI_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_inline_ui_container_apply_compact_style(EGUI_VIEW_OF(&compact_preview));
    egui_view_inline_ui_container_set_text(EGUI_VIEW_OF(&compact_preview), "Fit", "line");
    egui_view_label_set_text(EGUI_VIEW_OF(&compact_child), "Tag");
    egui_view_label_set_font_color(EGUI_VIEW_OF(&compact_child), EGUI_COLOR_HEX(0x0C7C73), EGUI_ALPHA_100);
    egui_view_inline_ui_container_layout_child(EGUI_VIEW_OF(&compact_preview));

    egui_view_inline_ui_container_apply_read_only_style(EGUI_VIEW_OF(&read_only_preview));
    egui_view_inline_ui_container_set_text(EGUI_VIEW_OF(&read_only_preview), "RO", "line");
    egui_view_label_set_text(EGUI_VIEW_OF(&read_only_child), "Lock");
    egui_view_label_set_font_color(EGUI_VIEW_OF(&read_only_child), EGUI_COLOR_HEX(0x65717E), EGUI_ALPHA_100);
    egui_view_inline_ui_container_layout_child(EGUI_VIEW_OF(&read_only_preview));

    if (ui_ready)
    {
        layout_page();
    }
}

static void layout_local_views(void)
{
    egui_view_inline_ui_container_layout_child(EGUI_VIEW_OF(&primary_control));
    egui_view_inline_ui_container_layout_child(EGUI_VIEW_OF(&compact_preview));
    egui_view_inline_ui_container_layout_child(EGUI_VIEW_OF(&read_only_preview));
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), INLINE_UI_ROOT_WIDTH, INLINE_UI_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, INLINE_UI_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4,
                    EGUI_COLOR_HEX(0x21303F));
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_inline_ui_container_init(EGUI_VIEW_OF(&primary_control));
    egui_view_set_size(EGUI_VIEW_OF(&primary_control), INLINE_UI_PRIMARY_WIDTH, INLINE_UI_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_control), 0, 0, 0, 10);
    egui_view_inline_ui_container_set_font(EGUI_VIEW_OF(&primary_control), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    init_text_label(&primary_child, INLINE_UI_CHILD_WIDTH, INLINE_UI_CHILD_HEIGHT, "Action", (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_inline_ui_container_set_child(EGUI_VIEW_OF(&primary_control), EGUI_VIEW_OF(&primary_child));
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_control));

    init_text_label(&caption_label, INLINE_UI_ROOT_WIDTH, 12, "Inline child host", (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_set_margin(EGUI_VIEW_OF(&caption_label), 0, 0, 0, 16);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&caption_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), INLINE_UI_PREVIEW_ROW_WIDTH, INLINE_UI_PREVIEW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_inline_ui_container_init(EGUI_VIEW_OF(&compact_preview));
    egui_view_set_size(EGUI_VIEW_OF(&compact_preview), INLINE_UI_PREVIEW_WIDTH, INLINE_UI_PREVIEW_HEIGHT);
    egui_view_inline_ui_container_set_font(EGUI_VIEW_OF(&compact_preview), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    init_text_label(&compact_child, INLINE_UI_PREVIEW_CHILD_W, INLINE_UI_PREVIEW_CHILD_H, "Tag",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x0C7C73));
    egui_view_inline_ui_container_set_child(EGUI_VIEW_OF(&compact_preview), EGUI_VIEW_OF(&compact_child));
    egui_view_inline_ui_container_override_static_preview_api(EGUI_VIEW_OF(&compact_preview), &compact_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_preview), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_preview));

    egui_view_inline_ui_container_init(EGUI_VIEW_OF(&read_only_preview));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_preview), INLINE_UI_PREVIEW_WIDTH, INLINE_UI_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_preview), 12, 0, 0, 0);
    egui_view_inline_ui_container_set_font(EGUI_VIEW_OF(&read_only_preview), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    init_text_label(&read_only_child, INLINE_UI_PREVIEW_CHILD_W, INLINE_UI_PREVIEW_CHILD_H, "Lock",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x65717E));
    egui_view_inline_ui_container_set_child(EGUI_VIEW_OF(&read_only_preview), EGUI_VIEW_OF(&read_only_child));
    egui_view_inline_ui_container_override_static_preview_api(EGUI_VIEW_OF(&read_only_preview), &read_only_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&read_only_preview), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_preview));

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
        EGUI_SIM_SET_WAIT(p_action, INLINE_UI_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, INLINE_UI_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INLINE_UI_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, INLINE_UI_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INLINE_UI_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, INLINE_UI_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INLINE_UI_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INLINE_UI_RECORD_FINAL_WAIT);
        return true;
    default:
        break;
    }
    return false;
}
#endif
