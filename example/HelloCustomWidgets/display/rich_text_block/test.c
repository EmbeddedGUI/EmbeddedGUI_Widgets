#include "egui.h"
#include "egui_view_rich_text_block.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define RICH_TEXT_BLOCK_ROOT_WIDTH        224
#define RICH_TEXT_BLOCK_ROOT_HEIGHT       204
#define RICH_TEXT_BLOCK_PRIMARY_WIDTH     176
#define RICH_TEXT_BLOCK_PRIMARY_HEIGHT    96
#define RICH_TEXT_BLOCK_PREVIEW_WIDTH     84
#define RICH_TEXT_BLOCK_PREVIEW_HEIGHT    42
#define RICH_TEXT_BLOCK_BOTTOM_ROW_WIDTH  176
#define RICH_TEXT_BLOCK_BOTTOM_ROW_HEIGHT 42
#define RICH_TEXT_BLOCK_RECORD_WAIT       90
#define RICH_TEXT_BLOCK_RECORD_FRAME_WAIT 170
#define RICH_TEXT_BLOCK_RECORD_FINAL_WAIT 280
#define RICH_TEXT_BLOCK_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct rich_text_block_snapshot rich_text_block_snapshot_t;
struct rich_text_block_snapshot
{
    const egui_view_rich_text_block_paragraph_t *paragraphs;
    uint8_t paragraph_count;
    const char *status_label;
    egui_color_t status_label_color;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_rich_text_block_t primary_widget;
static egui_view_label_t primary_status_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_rich_text_block_t compact_widget;
static egui_view_rich_text_block_t read_only_widget;
static egui_view_api_t compact_widget_api;
static egui_view_api_t read_only_widget_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "RichTextBlock";

static const egui_view_rich_text_block_paragraph_t snapshot_release_note[] = {
        {"Release notes keep the opening line stronger for scan-first reading.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_EMPHASIS},
        {"Supporting copy stays inside the same block so summary and context do not split into extra labels.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
        {"Updated 10 minutes ago", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
};

static const egui_view_rich_text_block_paragraph_t snapshot_policy_callout[] = {
        {"Policy updates can open with a short setup line before the action block.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
        {"Approve before publishing.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_ACCENT},
        {"Owner review required", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
};

static const egui_view_rich_text_block_paragraph_t snapshot_editorial_brief[] = {
        {"Editorial copy can still start with a stronger sentence.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_EMPHASIS},
        {"Body text wraps like a normal paragraph while remaining display-only and reference-friendly.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
        {"Secondary copy stays visually lighter.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
};

static const egui_view_rich_text_block_paragraph_t compact_preview_snapshot[] = {
        {"Short brief\nfor narrow slots.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
        {"Compact", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
};

static const egui_view_rich_text_block_paragraph_t read_only_preview_snapshot[] = {
        {"Owner review\nrequired.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
        {"Read only", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
};

static const rich_text_block_snapshot_t primary_snapshots[] = {
        {
                snapshot_release_note,
                (uint8_t)EGUI_ARRAY_SIZE(snapshot_release_note),
                "Release note / emphasis + body + caption",
                EGUI_COLOR_HEX(0x0F6CBD),
        },
        {
                snapshot_policy_callout,
                (uint8_t)EGUI_ARRAY_SIZE(snapshot_policy_callout),
                "Policy callout / body + accent + caption",
                EGUI_COLOR_HEX(0x0F7B45),
        },
        {
                snapshot_editorial_brief,
                (uint8_t)EGUI_ARRAY_SIZE(snapshot_editorial_brief),
                "Editorial brief / emphasis + body + caption",
                EGUI_COLOR_HEX(0x8F4C11),
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
    const rich_text_block_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    egui_view_rich_text_block_set_paragraphs(EGUI_VIEW_OF(&primary_widget), snapshot->paragraphs, snapshot->paragraph_count);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_status_label), snapshot->status_label);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_status_label), snapshot->status_label_color, EGUI_ALPHA_100);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(RICH_TEXT_BLOCK_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_rich_text_block_set_paragraphs(EGUI_VIEW_OF(&compact_widget), compact_preview_snapshot, (uint8_t)EGUI_ARRAY_SIZE(compact_preview_snapshot));
    egui_view_rich_text_block_set_compact_mode(EGUI_VIEW_OF(&compact_widget), 1);

    egui_view_rich_text_block_set_paragraphs(EGUI_VIEW_OF(&read_only_widget), read_only_preview_snapshot, (uint8_t)EGUI_ARRAY_SIZE(read_only_preview_snapshot));
    egui_view_rich_text_block_set_compact_mode(EGUI_VIEW_OF(&read_only_widget), 1);
    egui_view_rich_text_block_set_read_only_mode(EGUI_VIEW_OF(&read_only_widget), 1);
    egui_view_rich_text_block_set_palette(EGUI_VIEW_OF(&read_only_widget), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD8DFE6), EGUI_COLOR_HEX(0x425160),
                                          EGUI_COLOR_HEX(0x738191), EGUI_COLOR_HEX(0x8AA2B5));
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
    ui_ready = 0;
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), RICH_TEXT_BLOCK_ROOT_WIDTH, RICH_TEXT_BLOCK_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, RICH_TEXT_BLOCK_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_rich_text_block_init(EGUI_VIEW_OF(&primary_widget));
    egui_view_set_size(EGUI_VIEW_OF(&primary_widget), RICH_TEXT_BLOCK_PRIMARY_WIDTH, RICH_TEXT_BLOCK_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_widget), 0, 0, 0, 10);
    egui_view_rich_text_block_set_font(EGUI_VIEW_OF(&primary_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_rich_text_block_set_emphasis_font(EGUI_VIEW_OF(&primary_widget), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_rich_text_block_set_caption_font(EGUI_VIEW_OF(&primary_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_widget));

    init_text_label(&primary_status_label, RICH_TEXT_BLOCK_ROOT_WIDTH, 12, "Release note / emphasis + body + caption",
                    (const egui_font_t *)&egui_res_font_montserrat_10_4, EGUI_COLOR_HEX(0x0F6CBD), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_status_label), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_status_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), RICH_TEXT_BLOCK_BOTTOM_ROW_WIDTH, RICH_TEXT_BLOCK_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_rich_text_block_init(EGUI_VIEW_OF(&compact_widget));
    egui_view_set_size(EGUI_VIEW_OF(&compact_widget), RICH_TEXT_BLOCK_PREVIEW_WIDTH, RICH_TEXT_BLOCK_PREVIEW_HEIGHT);
    egui_view_rich_text_block_set_font(EGUI_VIEW_OF(&compact_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_text_block_set_emphasis_font(EGUI_VIEW_OF(&compact_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_rich_text_block_set_caption_font(EGUI_VIEW_OF(&compact_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_text_block_override_static_preview_api(EGUI_VIEW_OF(&compact_widget), &compact_widget_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_widget));

    egui_view_rich_text_block_init(EGUI_VIEW_OF(&read_only_widget));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_widget), RICH_TEXT_BLOCK_PREVIEW_WIDTH, RICH_TEXT_BLOCK_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_widget), 8, 0, 0, 0);
    egui_view_rich_text_block_set_font(EGUI_VIEW_OF(&read_only_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_text_block_set_emphasis_font(EGUI_VIEW_OF(&read_only_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_rich_text_block_set_caption_font(EGUI_VIEW_OF(&read_only_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_text_block_override_static_preview_api(EGUI_VIEW_OF(&read_only_widget), &read_only_widget_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_widget));

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
        EGUI_SIM_SET_WAIT(p_action, RICH_TEXT_BLOCK_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_TEXT_BLOCK_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_TEXT_BLOCK_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_TEXT_BLOCK_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_TEXT_BLOCK_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_TEXT_BLOCK_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_TEXT_BLOCK_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
