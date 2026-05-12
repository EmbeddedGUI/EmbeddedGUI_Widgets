#include "egui.h"
#include "egui_view_group_box.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"
#include "../../hcw_text_center.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define GROUP_BOX_ROOT_WIDTH        238
#define GROUP_BOX_ROOT_HEIGHT       232
#define GROUP_BOX_PRIMARY_WIDTH     176
#define GROUP_BOX_PRIMARY_HEIGHT    104
#define GROUP_BOX_HEADER_WIDTH      112
#define GROUP_BOX_HEADER_HEIGHT     16
#define GROUP_BOX_CONTENT_WIDTH     116
#define GROUP_BOX_CONTENT_HEIGHT    24
#define GROUP_BOX_PREVIEW_ROW_WIDTH 202
#define GROUP_BOX_PREVIEW_WIDTH     92
#define GROUP_BOX_PREVIEW_HEIGHT    54
#define GROUP_BOX_RECORD_WAIT       90
#define GROUP_BOX_RECORD_FRAME_WAIT 170
#define GROUP_BOX_RECORD_FINAL_WAIT 280
#define GROUP_BOX_DEFAULT_SNAPSHOT  0
#define GROUP_BOX_TEXT_PAD_X        5
#define GROUP_BOX_TEXT_PAD_Y        2

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct group_box_snapshot group_box_snapshot_t;
struct group_box_snapshot
{
    const char *header_text;
    const char *content_text;
    const char *caption;
    egui_color_t text_color;
    uint8_t style;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_group_box_t primary_control;
static egui_view_label_t primary_header;
static egui_view_label_t primary_content;
static egui_view_label_t caption_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_group_box_t compact_preview;
static egui_view_label_t compact_header;
static egui_view_label_t compact_content;
static egui_view_group_box_t read_only_preview;
static egui_view_label_t read_only_header;
static egui_view_label_t read_only_content;
static egui_view_api_t content_label_api;
static egui_view_api_t compact_preview_api;
static egui_view_api_t read_only_preview_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, HCW_COLOR_PAGE_BG, EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "GroupBox";

static const group_box_snapshot_t primary_snapshots[] = {
        {
                "Settings group",
                "Content region",
                "Standard / centered content",
                HCW_COLOR_PRIMARY,
                0,
        },
        {
                "Policy group",
                "Leading content",
                "Accent / leading content",
                HCW_COLOR_PRIMARY,
                1,
        },
        {
                "Compact",
                "Top left",
                "Compact / framed group",
                HCW_COLOR_SUCCESS,
                2,
        },
        {
                "Read only",
                "Muted",
                "Read only / muted group",
                HCW_COLOR_TEXT_SOFT,
                3,
        },
};

static void layout_page(void);

static void content_label_on_draw(egui_view_t *self)
{
    egui_view_label_t *label = EGUI_CAST_TO(egui_view_label_t, self);
    egui_region_t region;

    if (label->font == NULL || label->text == NULL)
    {
        egui_view_label_on_draw(self);
        return;
    }

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }
    if (region.size.width > GROUP_BOX_TEXT_PAD_X * 2)
    {
        region.location.x += GROUP_BOX_TEXT_PAD_X;
        region.size.width -= GROUP_BOX_TEXT_PAD_X * 2;
    }
    if (region.size.height > GROUP_BOX_TEXT_PAD_Y * 2)
    {
        region.location.y += GROUP_BOX_TEXT_PAD_Y;
        region.size.height -= GROUP_BOX_TEXT_PAD_Y * 2;
    }
    region.location.y += hcw_text_center_get_delta(label->font, label->text, &region, label->align_type);
    egui_canvas_draw_text_in_rect_with_line_space(egui_view_get_canvas(self), label->font, label->text, &region, label->align_type, label->line_space,
                                                  label->color, label->alpha);
}

static void apply_content_label_api(egui_view_t *label)
{
    egui_view_copy_api(label, &content_label_api);
    content_label_api.on_draw = content_label_on_draw;
}

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font,
                            egui_color_t color, uint8_t align_type)
{
    egui_view_label_init(EGUI_VIEW_OF(label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align_type);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
    apply_content_label_api(EGUI_VIEW_OF(label));
}

static void apply_group_box_style(egui_view_t *view, uint8_t style)
{
    switch (style)
    {
    case 1:
        egui_view_group_box_apply_accent_style(view);
        break;
    case 2:
        egui_view_group_box_apply_compact_style(view);
        break;
    case 3:
        egui_view_group_box_apply_read_only_style(view);
        break;
    default:
        egui_view_group_box_apply_standard_style(view);
        break;
    }
}

static void apply_primary_snapshot(uint8_t index)
{
    const group_box_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    apply_group_box_style(EGUI_VIEW_OF(&primary_control), snapshot->style);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_header), snapshot->header_text);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_header), snapshot->text_color, EGUI_ALPHA_100);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_content), snapshot->content_text);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_content), snapshot->text_color, EGUI_ALPHA_100);
    egui_view_label_set_text(EGUI_VIEW_OF(&caption_label), snapshot->caption);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&caption_label), snapshot->text_color, EGUI_ALPHA_100);
    egui_view_group_box_layout_childs(EGUI_VIEW_OF(&primary_control));
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(GROUP_BOX_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_group_box_apply_compact_style(EGUI_VIEW_OF(&compact_preview));
    egui_view_label_set_text(EGUI_VIEW_OF(&compact_header), "Compact");
    egui_view_label_set_text(EGUI_VIEW_OF(&compact_content), "Top left");
    egui_view_label_set_font_color(EGUI_VIEW_OF(&compact_header), HCW_COLOR_SUCCESS, EGUI_ALPHA_100);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&compact_content), HCW_COLOR_SUCCESS, EGUI_ALPHA_100);
    egui_view_group_box_layout_childs(EGUI_VIEW_OF(&compact_preview));

    egui_view_group_box_apply_read_only_style(EGUI_VIEW_OF(&read_only_preview));
    egui_view_label_set_text(EGUI_VIEW_OF(&read_only_header), "Read only");
    egui_view_label_set_text(EGUI_VIEW_OF(&read_only_content), "Muted");
    egui_view_label_set_font_color(EGUI_VIEW_OF(&read_only_header), HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_100);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&read_only_content), HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_100);
    egui_view_group_box_layout_childs(EGUI_VIEW_OF(&read_only_preview));

    if (ui_ready)
    {
        layout_page();
    }
}

static void layout_local_views(void)
{
    egui_view_group_box_layout_childs(EGUI_VIEW_OF(&primary_control));
    egui_view_group_box_layout_childs(EGUI_VIEW_OF(&compact_preview));
    egui_view_group_box_layout_childs(EGUI_VIEW_OF(&read_only_preview));
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), GROUP_BOX_ROOT_WIDTH, GROUP_BOX_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), GROUP_BOX_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), HCW_COLOR_TEXT, EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_group_box_init(EGUI_VIEW_OF(&primary_control));
    egui_view_set_size(EGUI_VIEW_OF(&primary_control), GROUP_BOX_PRIMARY_WIDTH, GROUP_BOX_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_control), 0, 0, 0, 8);
    init_text_label(&primary_header, GROUP_BOX_HEADER_WIDTH, GROUP_BOX_HEADER_HEIGHT, "Settings group",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_PRIMARY, EGUI_ALIGN_CENTER);
    init_text_label(&primary_content, GROUP_BOX_CONTENT_WIDTH, GROUP_BOX_CONTENT_HEIGHT, "Content region",
                    (const egui_font_t *)&egui_res_font_montserrat_10_4, HCW_COLOR_PRIMARY, EGUI_ALIGN_CENTER);
    egui_view_group_box_set_header(EGUI_VIEW_OF(&primary_control), EGUI_VIEW_OF(&primary_header));
    egui_view_group_box_set_content(EGUI_VIEW_OF(&primary_control), EGUI_VIEW_OF(&primary_content));
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_control));

    egui_view_label_init(EGUI_VIEW_OF(&caption_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&caption_label), GROUP_BOX_ROOT_WIDTH, 12);
    egui_view_label_set_text(EGUI_VIEW_OF(&caption_label), "Standard / centered content");
    egui_view_label_set_align_type(EGUI_VIEW_OF(&caption_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&caption_label), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&caption_label), HCW_COLOR_PRIMARY, EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&caption_label), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&caption_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), GROUP_BOX_PREVIEW_ROW_WIDTH, GROUP_BOX_PREVIEW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_group_box_init(EGUI_VIEW_OF(&compact_preview));
    egui_view_set_size(EGUI_VIEW_OF(&compact_preview), GROUP_BOX_PREVIEW_WIDTH, GROUP_BOX_PREVIEW_HEIGHT);
    init_text_label(&compact_header, 62, 12, "Compact", (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_SUCCESS, EGUI_ALIGN_CENTER);
    init_text_label(&compact_content, 68, 16, "Top left", (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_SUCCESS,
                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_group_box_set_header(EGUI_VIEW_OF(&compact_preview), EGUI_VIEW_OF(&compact_header));
    egui_view_group_box_set_content(EGUI_VIEW_OF(&compact_preview), EGUI_VIEW_OF(&compact_content));
    egui_view_group_box_override_static_preview_api(EGUI_VIEW_OF(&compact_preview), &compact_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_preview), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_preview));

    egui_view_group_box_init(EGUI_VIEW_OF(&read_only_preview));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_preview), GROUP_BOX_PREVIEW_WIDTH, GROUP_BOX_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_preview), 12, 0, 0, 0);
    init_text_label(&read_only_header, 68, 12, "Read only", (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_TEXT_SOFT, EGUI_ALIGN_CENTER);
    init_text_label(&read_only_content, 68, 16, "Muted", (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_TEXT_SOFT,
                    EGUI_ALIGN_CENTER);
    egui_view_group_box_set_header(EGUI_VIEW_OF(&read_only_preview), EGUI_VIEW_OF(&read_only_header));
    egui_view_group_box_set_content(EGUI_VIEW_OF(&read_only_preview), EGUI_VIEW_OF(&read_only_content));
    egui_view_group_box_override_static_preview_api(EGUI_VIEW_OF(&read_only_preview), &read_only_preview_api);
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
        EGUI_SIM_SET_WAIT(p_action, GROUP_BOX_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, GROUP_BOX_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GROUP_BOX_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, GROUP_BOX_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GROUP_BOX_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, GROUP_BOX_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GROUP_BOX_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_default_state();
        }
        EGUI_SIM_SET_WAIT(p_action, GROUP_BOX_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GROUP_BOX_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
