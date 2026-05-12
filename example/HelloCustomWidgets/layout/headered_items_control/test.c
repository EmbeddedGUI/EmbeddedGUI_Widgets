#include "egui.h"
#include "egui_view_headered_items_control.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"
#include "../../hcw_text_center.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define HEADERED_ITEMS_CONTROL_ROOT_WIDTH        238
#define HEADERED_ITEMS_CONTROL_ROOT_HEIGHT       232
#define HEADERED_ITEMS_CONTROL_PRIMARY_WIDTH     176
#define HEADERED_ITEMS_CONTROL_PRIMARY_HEIGHT    108
#define HEADERED_ITEMS_CONTROL_ITEM_COUNT        4
#define HEADERED_ITEMS_CONTROL_PREVIEW_ROW_WIDTH 202
#define HEADERED_ITEMS_CONTROL_PREVIEW_WIDTH     92
#define HEADERED_ITEMS_CONTROL_PREVIEW_HEIGHT    68
#define HEADERED_ITEMS_CONTROL_RECORD_WAIT       90
#define HEADERED_ITEMS_CONTROL_RECORD_FRAME_WAIT 170
#define HEADERED_ITEMS_CONTROL_RECORD_FINAL_WAIT 280
#define HEADERED_ITEMS_CONTROL_DEFAULT_SNAPSHOT  0
#define HEADERED_ITEMS_CONTROL_TEXT_PAD_X        6
#define HEADERED_ITEMS_CONTROL_TEXT_PAD_Y        2

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct headered_items_control_snapshot headered_items_control_snapshot_t;
struct headered_items_control_snapshot
{
    const char *header;
    const char *items[HEADERED_ITEMS_CONTROL_ITEM_COUNT];
    const char *caption;
    egui_color_t text_color;
    uint8_t style;
    egui_dim_t item_width;
    egui_dim_t item_height;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_headered_items_control_t primary_control;
static egui_view_label_t primary_header;
static egui_view_label_t primary_items[HEADERED_ITEMS_CONTROL_ITEM_COUNT];
static egui_view_label_t caption_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_headered_items_control_t wrap_preview;
static egui_view_label_t wrap_header;
static egui_view_label_t wrap_items[3];
static egui_view_headered_items_control_t read_only_preview;
static egui_view_label_t read_only_header;
static egui_view_label_t read_only_items[3];
static egui_view_api_t content_label_api;
static egui_view_api_t wrap_preview_api;
static egui_view_api_t read_only_preview_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, HCW_COLOR_PAGE_BG, EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "HeaderedItems";

static const headered_items_control_snapshot_t primary_snapshots[] = {
        {
                "Planning group",
                {"Inbox", "Design", "Review", "Done"},
                "Header / vertical items",
                HCW_COLOR_PRIMARY,
                0,
                124,
                14,
        },
        {
                "Sprint tags",
                {"UI", "SDK", "QA", "Docs"},
                "Header / horizontal strip",
                HCW_COLOR_PRIMARY,
                1,
                30,
                16,
        },
        {
                "Labels",
                {"Alpha", "Beta", "Gamma", "Delta"},
                "Header / wrap chips",
                HCW_COLOR_SUCCESS,
                2,
                112,
                16,
        },
        {
                "Read only",
                {"Static", "Muted", "Locked", "Items"},
                "Header / muted list",
                HCW_COLOR_TEXT_SOFT,
                3,
                122,
                14,
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
    if (region.size.width > HEADERED_ITEMS_CONTROL_TEXT_PAD_X * 2)
    {
        region.location.x += HEADERED_ITEMS_CONTROL_TEXT_PAD_X;
        region.size.width -= HEADERED_ITEMS_CONTROL_TEXT_PAD_X * 2;
    }
    if (region.size.height > HEADERED_ITEMS_CONTROL_TEXT_PAD_Y * 2)
    {
        region.location.y += HEADERED_ITEMS_CONTROL_TEXT_PAD_Y;
        region.size.height -= HEADERED_ITEMS_CONTROL_TEXT_PAD_Y * 2;
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

static void apply_headered_items_control_style(egui_view_t *view, uint8_t style)
{
    switch (style)
    {
    case 1:
        egui_view_headered_items_control_apply_strip_style(view);
        break;
    case 2:
        egui_view_headered_items_control_apply_wrap_style(view);
        break;
    case 3:
        egui_view_headered_items_control_apply_read_only_style(view);
        break;
    default:
        egui_view_headered_items_control_apply_standard_style(view);
        break;
    }
}

static void apply_primary_snapshot(uint8_t index)
{
    uint8_t i;
    const headered_items_control_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    apply_headered_items_control_style(EGUI_VIEW_OF(&primary_control), snapshot->style);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_header), snapshot->header);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_header), snapshot->text_color, EGUI_ALPHA_100);
    for (i = 0; i < HEADERED_ITEMS_CONTROL_ITEM_COUNT; i++)
    {
        egui_view_set_size(EGUI_VIEW_OF(&primary_items[i]), snapshot->item_width, snapshot->item_height);
        egui_view_label_set_text(EGUI_VIEW_OF(&primary_items[i]), snapshot->items[i]);
        egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_items[i]), snapshot->text_color, EGUI_ALPHA_100);
    }
    egui_view_label_set_text(EGUI_VIEW_OF(&caption_label), snapshot->caption);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&caption_label), snapshot->text_color, EGUI_ALPHA_100);
    egui_view_headered_items_control_layout_childs(EGUI_VIEW_OF(&primary_control));
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(HEADERED_ITEMS_CONTROL_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_headered_items_control_apply_wrap_style(EGUI_VIEW_OF(&wrap_preview));
    egui_view_label_set_text(EGUI_VIEW_OF(&wrap_header), "Wrap");
    egui_view_label_set_font_color(EGUI_VIEW_OF(&wrap_header), HCW_COLOR_SUCCESS, EGUI_ALPHA_100);
    egui_view_label_set_text(EGUI_VIEW_OF(&wrap_items[0]), "One");
    egui_view_label_set_text(EGUI_VIEW_OF(&wrap_items[1]), "Two");
    egui_view_label_set_text(EGUI_VIEW_OF(&wrap_items[2]), "Host");
    egui_view_headered_items_control_layout_childs(EGUI_VIEW_OF(&wrap_preview));

    egui_view_headered_items_control_apply_read_only_style(EGUI_VIEW_OF(&read_only_preview));
    egui_view_label_set_text(EGUI_VIEW_OF(&read_only_header), "Read only");
    egui_view_label_set_font_color(EGUI_VIEW_OF(&read_only_header), HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_100);
    egui_view_label_set_text(EGUI_VIEW_OF(&read_only_items[0]), "Static");
    egui_view_label_set_text(EGUI_VIEW_OF(&read_only_items[1]), "List");
    egui_view_label_set_text(EGUI_VIEW_OF(&read_only_items[2]), "Host");
    egui_view_headered_items_control_layout_childs(EGUI_VIEW_OF(&read_only_preview));

    if (ui_ready)
    {
        layout_page();
    }
}

static void layout_local_views(void)
{
    egui_view_headered_items_control_layout_childs(EGUI_VIEW_OF(&primary_control));
    egui_view_headered_items_control_layout_childs(EGUI_VIEW_OF(&wrap_preview));
    egui_view_headered_items_control_layout_childs(EGUI_VIEW_OF(&read_only_preview));
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
    uint8_t i;

    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), HEADERED_ITEMS_CONTROL_ROOT_WIDTH, HEADERED_ITEMS_CONTROL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), HEADERED_ITEMS_CONTROL_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), HCW_COLOR_TEXT, EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_headered_items_control_init(EGUI_VIEW_OF(&primary_control));
    egui_view_set_size(EGUI_VIEW_OF(&primary_control), HEADERED_ITEMS_CONTROL_PRIMARY_WIDTH, HEADERED_ITEMS_CONTROL_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_control), 0, 0, 0, 8);
    init_text_label(&primary_header, 126, 16, primary_snapshots[0].header, (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    HCW_COLOR_PRIMARY, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_headered_items_control_set_header(EGUI_VIEW_OF(&primary_control), EGUI_VIEW_OF(&primary_header));
    for (i = 0; i < HEADERED_ITEMS_CONTROL_ITEM_COUNT; i++)
    {
        init_text_label(&primary_items[i], 124, 14, primary_snapshots[0].items[i], (const egui_font_t *)&egui_res_font_montserrat_8_4,
                        HCW_COLOR_PRIMARY, EGUI_ALIGN_CENTER);
        egui_view_headered_items_control_add_item(EGUI_VIEW_OF(&primary_control), EGUI_VIEW_OF(&primary_items[i]));
    }
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_control));

    egui_view_label_init(EGUI_VIEW_OF(&caption_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&caption_label), HEADERED_ITEMS_CONTROL_ROOT_WIDTH, 12);
    egui_view_label_set_text(EGUI_VIEW_OF(&caption_label), "Header / vertical items");
    egui_view_label_set_align_type(EGUI_VIEW_OF(&caption_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&caption_label), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&caption_label), HCW_COLOR_PRIMARY, EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&caption_label), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&caption_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), HEADERED_ITEMS_CONTROL_PREVIEW_ROW_WIDTH, HEADERED_ITEMS_CONTROL_PREVIEW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_headered_items_control_init(EGUI_VIEW_OF(&wrap_preview));
    egui_view_set_size(EGUI_VIEW_OF(&wrap_preview), HEADERED_ITEMS_CONTROL_PREVIEW_WIDTH, HEADERED_ITEMS_CONTROL_PREVIEW_HEIGHT);
    init_text_label(&wrap_header, 58, 12, "Wrap", (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_SUCCESS,
                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_headered_items_control_set_header(EGUI_VIEW_OF(&wrap_preview), EGUI_VIEW_OF(&wrap_header));
    for (i = 0; i < 3; i++)
    {
        init_text_label(&wrap_items[i], 36, 12, "One", (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_SUCCESS,
                        EGUI_ALIGN_CENTER);
        egui_view_headered_items_control_add_item(EGUI_VIEW_OF(&wrap_preview), EGUI_VIEW_OF(&wrap_items[i]));
    }
    egui_view_headered_items_control_override_static_preview_api(EGUI_VIEW_OF(&wrap_preview), &wrap_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&wrap_preview), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&wrap_preview));

    egui_view_headered_items_control_init(EGUI_VIEW_OF(&read_only_preview));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_preview), HEADERED_ITEMS_CONTROL_PREVIEW_WIDTH, HEADERED_ITEMS_CONTROL_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_preview), 12, 0, 0, 0);
    init_text_label(&read_only_header, 64, 12, "Read only", (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_TEXT_SOFT,
                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_headered_items_control_set_header(EGUI_VIEW_OF(&read_only_preview), EGUI_VIEW_OF(&read_only_header));
    for (i = 0; i < 3; i++)
    {
        init_text_label(&read_only_items[i], 58, 11, "Static", (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_TEXT_SOFT,
                        EGUI_ALIGN_CENTER);
        egui_view_headered_items_control_add_item(EGUI_VIEW_OF(&read_only_preview), EGUI_VIEW_OF(&read_only_items[i]));
    }
    egui_view_headered_items_control_override_static_preview_api(EGUI_VIEW_OF(&read_only_preview), &read_only_preview_api);
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
        EGUI_SIM_SET_WAIT(p_action, HEADERED_ITEMS_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_ITEMS_CONTROL_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_ITEMS_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_ITEMS_CONTROL_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_ITEMS_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_ITEMS_CONTROL_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_ITEMS_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_default_state();
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_ITEMS_CONTROL_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, HEADERED_ITEMS_CONTROL_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
