#include "egui.h"
#include "egui_view_rich_text_block.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define RICH_TEXT_BLOCK_ROOT_WIDTH        224
#define RICH_TEXT_BLOCK_ROOT_HEIGHT       272
#define RICH_TEXT_BLOCK_PANEL_WIDTH       196
#define RICH_TEXT_BLOCK_PANEL_HEIGHT      142
#define RICH_TEXT_BLOCK_PRIMARY_WIDTH     176
#define RICH_TEXT_BLOCK_PRIMARY_HEIGHT    86
#define RICH_TEXT_BLOCK_PREVIEW_WIDTH     104
#define RICH_TEXT_BLOCK_PREVIEW_HEIGHT    92
#define RICH_TEXT_BLOCK_PREVIEW_BODY_W    84
#define RICH_TEXT_BLOCK_PREVIEW_BODY_H    42
#define RICH_TEXT_BLOCK_BOTTOM_ROW_WIDTH  216
#define RICH_TEXT_BLOCK_BOTTOM_ROW_HEIGHT 92
#define RICH_TEXT_BLOCK_RECORD_WAIT       90
#define RICH_TEXT_BLOCK_RECORD_FRAME_WAIT 170

typedef struct
{
    const char *heading;
    egui_view_rich_text_block_paragraph_t paragraphs[3];
    const char *summary;
    egui_color_t summary_color;
    const char *note;
} rich_text_block_snapshot_t;

typedef struct
{
    egui_view_rich_text_block_paragraph_t paragraphs[2];
    const char *caption;
} rich_text_block_preview_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t primary_heading_label;
static egui_view_rich_text_block_t primary_widget;
static egui_view_label_t primary_summary_label;
static egui_view_label_t primary_note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_panel;
static egui_view_label_t compact_heading_label;
static egui_view_rich_text_block_t compact_widget;
static egui_view_label_t compact_body_label;
static egui_view_linearlayout_t read_only_panel;
static egui_view_label_t read_only_heading_label;
static egui_view_rich_text_block_t read_only_widget;
static egui_view_label_t read_only_body_label;
static egui_view_api_t compact_widget_api;
static egui_view_api_t read_only_widget_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, EGUI_COLOR_HEX(0xFBFCFD), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

static const char *title_text = "RichTextBlock";

static const rich_text_block_snapshot_t primary_snapshots[] = {
        {
                "Lead paragraph",
                {
                        {"Release notes stay readable\nwhen headline and detail share one block.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_EMPHASIS},
                        {"A RichTextBlock keeps the opening sentence,\nsupporting paragraph, and metadata together.",
                         EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
                        {"Caption text remains quieter for follow-up context and timestamps.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
                },
                "Lead + body + caption",
                EGUI_COLOR_HEX(0x0F6CBD),
                "Paragraph styling stays inside one display control.",
        },
        {
                "Accent callout",
                {
                        {"Policy updates can keep a short setup line\nbefore the highlighted action block.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
                        {"Approve the summary\nbefore publishing.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_ACCENT},
                        {"The accent paragraph keeps urgency visible without changing layout.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
                },
                "Accent paragraph",
                EGUI_COLOR_HEX(0x0F7B45),
                "Callouts can live inline with the rest of the copy.",
        },
        {
                "Compact editorial",
                {
                        {"Editorial copy can still open\nwith a stronger first sentence.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_EMPHASIS},
                        {"Follow-up text wraps like a normal paragraph,\nwhile the footer line drops to caption tone.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
                        {"Secondary copy stays visually lighter.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
                },
                "Mixed paragraph styles",
                EGUI_COLOR_HEX(0x7A4DF3),
                "The control stays display-only and reference-friendly.",
        },
};

static const rich_text_block_preview_snapshot_t compact_snapshot = {
        {
                {"Short lead copy\nfor compact panes.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
                {"Compact preview", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
        },
        "Compact",
};

static const rich_text_block_preview_snapshot_t read_only_snapshot = {
        {
                {"Approve before\npublishing.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_ACCENT},
                {"Read-only preview", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
        },
        "Read only",
};

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

static void init_panel(egui_view_linearlayout_t *panel, egui_dim_t width, egui_dim_t height, egui_background_t *background, uint8_t align)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(panel));
    egui_view_set_size(EGUI_VIEW_OF(panel), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(panel), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(panel), align);
    egui_view_set_background(EGUI_VIEW_OF(panel), background);
    egui_view_set_padding(EGUI_VIEW_OF(panel), 8, 8, 8, 8);
}

static void apply_primary_snapshot(uint8_t index)
{
    const rich_text_block_snapshot_t *snapshot = &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)];

    egui_view_rich_text_block_set_paragraphs(EGUI_VIEW_OF(&primary_widget), snapshot->paragraphs, EGUI_ARRAY_SIZE(snapshot->paragraphs));
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_heading_label), snapshot->heading);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_summary_label), snapshot->summary);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_summary_label), snapshot->summary_color, EGUI_ALPHA_100);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_note_label), snapshot->note);
}

static void apply_preview_states(void)
{
    egui_view_rich_text_block_set_paragraphs(EGUI_VIEW_OF(&compact_widget), compact_snapshot.paragraphs, EGUI_ARRAY_SIZE(compact_snapshot.paragraphs));
    egui_view_rich_text_block_set_compact_mode(EGUI_VIEW_OF(&compact_widget), 1);
    egui_view_label_set_text(EGUI_VIEW_OF(&compact_body_label), compact_snapshot.caption);

    egui_view_rich_text_block_set_paragraphs(EGUI_VIEW_OF(&read_only_widget), read_only_snapshot.paragraphs, EGUI_ARRAY_SIZE(read_only_snapshot.paragraphs));
    egui_view_rich_text_block_set_compact_mode(EGUI_VIEW_OF(&read_only_widget), 1);
    egui_view_rich_text_block_set_read_only_mode(EGUI_VIEW_OF(&read_only_widget), 1);
    egui_view_rich_text_block_set_palette(EGUI_VIEW_OF(&read_only_widget), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD8DFE6), EGUI_COLOR_HEX(0x425160),
                                          EGUI_COLOR_HEX(0x738191), EGUI_COLOR_HEX(0x8AA2B5));
    egui_view_label_set_text(EGUI_VIEW_OF(&read_only_body_label), read_only_snapshot.caption);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), RICH_TEXT_BLOCK_ROOT_WIDTH, RICH_TEXT_BLOCK_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, RICH_TEXT_BLOCK_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_panel(&primary_panel, RICH_TEXT_BLOCK_PANEL_WIDTH, RICH_TEXT_BLOCK_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&primary_heading_label, 176, 12, "Lead paragraph", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x51606F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_heading_label));

    egui_view_rich_text_block_init(EGUI_VIEW_OF(&primary_widget));
    egui_view_set_size(EGUI_VIEW_OF(&primary_widget), RICH_TEXT_BLOCK_PRIMARY_WIDTH, RICH_TEXT_BLOCK_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_widget), 0, 0, 0, 6);
    egui_view_rich_text_block_set_font(EGUI_VIEW_OF(&primary_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_rich_text_block_set_emphasis_font(EGUI_VIEW_OF(&primary_widget), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_rich_text_block_set_caption_font(EGUI_VIEW_OF(&primary_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_widget));

    init_text_label(&primary_summary_label, 176, 12, "Lead + body + caption", (const egui_font_t *)&egui_res_font_montserrat_10_4, EGUI_COLOR_HEX(0x0F6CBD),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_summary_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_summary_label));

    init_text_label(&primary_note_label, 176, 12, "Paragraph styling stays inside one display control.",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), RICH_TEXT_BLOCK_BOTTOM_ROW_WIDTH, RICH_TEXT_BLOCK_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&compact_panel, RICH_TEXT_BLOCK_PREVIEW_WIDTH, RICH_TEXT_BLOCK_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_panel));

    init_text_label(&compact_heading_label, 84, 12, "Compact", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_heading_label));

    egui_view_rich_text_block_init(EGUI_VIEW_OF(&compact_widget));
    egui_view_set_size(EGUI_VIEW_OF(&compact_widget), RICH_TEXT_BLOCK_PREVIEW_BODY_W, RICH_TEXT_BLOCK_PREVIEW_BODY_H);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_widget), 0, 0, 0, 4);
    egui_view_rich_text_block_set_font(EGUI_VIEW_OF(&compact_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_text_block_set_emphasis_font(EGUI_VIEW_OF(&compact_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_rich_text_block_set_caption_font(EGUI_VIEW_OF(&compact_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_text_block_override_static_preview_api(EGUI_VIEW_OF(&compact_widget), &compact_widget_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_widget));

    init_text_label(&compact_body_label, 84, 10, "Compact", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_body_label));

    init_panel(&read_only_panel, RICH_TEXT_BLOCK_PREVIEW_WIDTH, RICH_TEXT_BLOCK_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_panel));

    init_text_label(&read_only_heading_label, 84, 12, "Read only", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_panel), EGUI_VIEW_OF(&read_only_heading_label));

    egui_view_rich_text_block_init(EGUI_VIEW_OF(&read_only_widget));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_widget), RICH_TEXT_BLOCK_PREVIEW_BODY_W, RICH_TEXT_BLOCK_PREVIEW_BODY_H);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_widget), 0, 0, 0, 4);
    egui_view_rich_text_block_set_font(EGUI_VIEW_OF(&read_only_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_text_block_set_emphasis_font(EGUI_VIEW_OF(&read_only_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_rich_text_block_set_caption_font(EGUI_VIEW_OF(&read_only_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_text_block_override_static_preview_api(EGUI_VIEW_OF(&read_only_widget), &read_only_widget_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_panel), EGUI_VIEW_OF(&read_only_widget));

    init_text_label(&read_only_body_label, 84, 10, "Read only", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89),
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_panel), EGUI_VIEW_OF(&read_only_body_label));

    apply_primary_snapshot(0);
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_panel));
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
            apply_primary_snapshot(0);
            apply_preview_states();
            recording_request_snapshot();
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
            recording_request_snapshot();
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
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_TEXT_BLOCK_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(0);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
