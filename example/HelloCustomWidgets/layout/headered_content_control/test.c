#include "egui.h"
#include "egui_view_headered_content_control.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define HEADERED_CONTENT_CONTROL_ROOT_WIDTH        238
#define HEADERED_CONTENT_CONTROL_ROOT_HEIGHT       232
#define HEADERED_CONTENT_CONTROL_PRIMARY_WIDTH     176
#define HEADERED_CONTENT_CONTROL_PRIMARY_HEIGHT    104
#define HEADERED_CONTENT_CONTROL_HEADER_WIDTH      126
#define HEADERED_CONTENT_CONTROL_HEADER_HEIGHT     16
#define HEADERED_CONTENT_CONTROL_CONTENT_WIDTH     116
#define HEADERED_CONTENT_CONTROL_CONTENT_HEIGHT    24
#define HEADERED_CONTENT_CONTROL_PREVIEW_ROW_WIDTH 202
#define HEADERED_CONTENT_CONTROL_PREVIEW_WIDTH     92
#define HEADERED_CONTENT_CONTROL_PREVIEW_HEIGHT    54
#define HEADERED_CONTENT_CONTROL_PREVIEW_HEADER_W  70
#define HEADERED_CONTENT_CONTROL_PREVIEW_HEADER_H  12
#define HEADERED_CONTENT_CONTROL_PREVIEW_LABEL_W   70
#define HEADERED_CONTENT_CONTROL_PREVIEW_LABEL_H   16
#define HEADERED_CONTENT_CONTROL_RECORD_WAIT       90
#define HEADERED_CONTENT_CONTROL_RECORD_FRAME_WAIT 170
#define HEADERED_CONTENT_CONTROL_RECORD_FINAL_WAIT 280
#define HEADERED_CONTENT_CONTROL_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct headered_content_control_snapshot headered_content_control_snapshot_t;
struct headered_content_control_snapshot
{
    const char *header_text;
    const char *content_text;
    const char *caption;
    egui_color_t caption_color;
    uint8_t style;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_headered_content_control_t primary_control;
static egui_view_label_t primary_header;
static egui_view_label_t primary_content;
static egui_view_label_t caption_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_headered_content_control_t compact_preview;
static egui_view_label_t compact_header;
static egui_view_label_t compact_content;
static egui_view_headered_content_control_t read_only_preview;
static egui_view_label_t read_only_header;
static egui_view_label_t read_only_content;
static egui_view_api_t compact_preview_api;
static egui_view_api_t read_only_preview_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "HeaderedContent";

static const headered_content_control_snapshot_t primary_snapshots[] = {
        {
                "Section header",
                "Center content",
                "Header / centered content",
                EGUI_COLOR_HEX(0x0F6CBD),
                0,
        },
        {
                "Primary header",
                "Leading content",
                "Accent / leading content",
                EGUI_COLOR_HEX(0x0F6CBD),
                1,
        },
        {
                "Compact",
                "Top left",
                "Compact / stacked",
                EGUI_COLOR_HEX(0x0F7B45),
                2,
        },
        {
                "Read only",
                "Muted",
                "Read only / muted header",
                EGUI_COLOR_HEX(0x65717E),
                3,
        },
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

static void apply_headered_content_control_style(egui_view_t *view, uint8_t style)
{
    switch (style)
    {
    case 1:
        egui_view_headered_content_control_apply_accent_style(view);
        break;
    case 2:
        egui_view_headered_content_control_apply_compact_style(view);
        break;
    case 3:
        egui_view_headered_content_control_apply_read_only_style(view);
        break;
    default:
        egui_view_headered_content_control_apply_standard_style(view);
        break;
    }
}

static void apply_primary_snapshot(uint8_t index)
{
    const headered_content_control_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    apply_headered_content_control_style(EGUI_VIEW_OF(&primary_control), snapshot->style);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_header), snapshot->header_text);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_header), snapshot->caption_color, EGUI_ALPHA_100);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_content), snapshot->content_text);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_content), snapshot->caption_color, EGUI_ALPHA_100);
    egui_view_label_set_text(EGUI_VIEW_OF(&caption_label), snapshot->caption);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&caption_label), snapshot->caption_color, EGUI_ALPHA_100);
    egui_view_headered_content_control_layout_childs(EGUI_VIEW_OF(&primary_control));
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(HEADERED_CONTENT_CONTROL_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_headered_content_control_apply_compact_style(EGUI_VIEW_OF(&compact_preview));
    egui_view_label_set_text(EGUI_VIEW_OF(&compact_header), "Compact");
    egui_view_label_set_text(EGUI_VIEW_OF(&compact_content), "Top left");
    egui_view_label_set_font_color(EGUI_VIEW_OF(&compact_header), EGUI_COLOR_HEX(0x0F7B45), EGUI_ALPHA_100);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&compact_content), EGUI_COLOR_HEX(0x0F7B45), EGUI_ALPHA_100);
    egui_view_headered_content_control_layout_childs(EGUI_VIEW_OF(&compact_preview));

    egui_view_headered_content_control_apply_read_only_style(EGUI_VIEW_OF(&read_only_preview));
    egui_view_label_set_text(EGUI_VIEW_OF(&read_only_header), "Read only");
    egui_view_label_set_text(EGUI_VIEW_OF(&read_only_content), "Muted");
    egui_view_label_set_font_color(EGUI_VIEW_OF(&read_only_header), EGUI_COLOR_HEX(0x65717E), EGUI_ALPHA_100);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&read_only_content), EGUI_COLOR_HEX(0x65717E), EGUI_ALPHA_100);
    egui_view_headered_content_control_layout_childs(EGUI_VIEW_OF(&read_only_preview));

    if (ui_ready)
    {
        layout_page();
    }
}

static void layout_local_views(void)
{
    egui_view_headered_content_control_layout_childs(EGUI_VIEW_OF(&primary_control));
    egui_view_headered_content_control_layout_childs(EGUI_VIEW_OF(&compact_preview));
    egui_view_headered_content_control_layout_childs(EGUI_VIEW_OF(&read_only_preview));
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), HEADERED_CONTENT_CONTROL_ROOT_WIDTH, HEADERED_CONTENT_CONTROL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), HEADERED_CONTENT_CONTROL_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_headered_content_control_init(EGUI_VIEW_OF(&primary_control));
    egui_view_set_size(EGUI_VIEW_OF(&primary_control), HEADERED_CONTENT_CONTROL_PRIMARY_WIDTH, HEADERED_CONTENT_CONTROL_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_control), 0, 0, 0, 8);
    init_text_label(&primary_header, HEADERED_CONTENT_CONTROL_HEADER_WIDTH, HEADERED_CONTENT_CONTROL_HEADER_HEIGHT, "Section header",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x0F6CBD), EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    init_text_label(&primary_content, HEADERED_CONTENT_CONTROL_CONTENT_WIDTH, HEADERED_CONTENT_CONTROL_CONTENT_HEIGHT, "Center content",
                    (const egui_font_t *)&egui_res_font_montserrat_10_4, EGUI_COLOR_HEX(0x0F6CBD), EGUI_ALIGN_CENTER);
    egui_view_headered_content_control_set_header(EGUI_VIEW_OF(&primary_control), EGUI_VIEW_OF(&primary_header));
    egui_view_headered_content_control_set_content(EGUI_VIEW_OF(&primary_control), EGUI_VIEW_OF(&primary_content));
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_control));

    egui_view_label_init(EGUI_VIEW_OF(&caption_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&caption_label), HEADERED_CONTENT_CONTROL_ROOT_WIDTH, 12);
    egui_view_label_set_text(EGUI_VIEW_OF(&caption_label), "Header / centered content");
    egui_view_label_set_align_type(EGUI_VIEW_OF(&caption_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&caption_label), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&caption_label), EGUI_COLOR_HEX(0x0F6CBD), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&caption_label), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&caption_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), HEADERED_CONTENT_CONTROL_PREVIEW_ROW_WIDTH, HEADERED_CONTENT_CONTROL_PREVIEW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_headered_content_control_init(EGUI_VIEW_OF(&compact_preview));
    egui_view_set_size(EGUI_VIEW_OF(&compact_preview), HEADERED_CONTENT_CONTROL_PREVIEW_WIDTH, HEADERED_CONTENT_CONTROL_PREVIEW_HEIGHT);
    init_text_label(&compact_header, HEADERED_CONTENT_CONTROL_PREVIEW_HEADER_W, HEADERED_CONTENT_CONTROL_PREVIEW_HEADER_H, "Compact",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x0F7B45), EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    init_text_label(&compact_content, HEADERED_CONTENT_CONTROL_PREVIEW_LABEL_W, HEADERED_CONTENT_CONTROL_PREVIEW_LABEL_H, "Top left",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x0F7B45), EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_headered_content_control_set_header(EGUI_VIEW_OF(&compact_preview), EGUI_VIEW_OF(&compact_header));
    egui_view_headered_content_control_set_content(EGUI_VIEW_OF(&compact_preview), EGUI_VIEW_OF(&compact_content));
    egui_view_headered_content_control_override_static_preview_api(EGUI_VIEW_OF(&compact_preview), &compact_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_preview), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_preview));

    egui_view_headered_content_control_init(EGUI_VIEW_OF(&read_only_preview));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_preview), HEADERED_CONTENT_CONTROL_PREVIEW_WIDTH, HEADERED_CONTENT_CONTROL_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_preview), 12, 0, 0, 0);
    init_text_label(&read_only_header, HEADERED_CONTENT_CONTROL_PREVIEW_HEADER_W, HEADERED_CONTENT_CONTROL_PREVIEW_HEADER_H, "Read only",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x65717E), EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    init_text_label(&read_only_content, HEADERED_CONTENT_CONTROL_PREVIEW_LABEL_W, HEADERED_CONTENT_CONTROL_PREVIEW_LABEL_H, "Muted",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x65717E), EGUI_ALIGN_CENTER);
    egui_view_headered_content_control_set_header(EGUI_VIEW_OF(&read_only_preview), EGUI_VIEW_OF(&read_only_header));
    egui_view_headered_content_control_set_content(EGUI_VIEW_OF(&read_only_preview), EGUI_VIEW_OF(&read_only_content));
    egui_view_headered_content_control_override_static_preview_api(EGUI_VIEW_OF(&read_only_preview), &read_only_preview_api);
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
        EGUI_SIM_SET_WAIT(p_action, HEADERED_CONTENT_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_CONTENT_CONTROL_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_CONTENT_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_CONTENT_CONTROL_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_CONTENT_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_CONTENT_CONTROL_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_CONTENT_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_default_state();
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_CONTENT_CONTROL_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_CONTENT_CONTROL_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
