#include "egui.h"
#include "egui_view_badge.h"
#include "uicode.h"
#include "demo_scaffold.h"

#include "../../../../sdk/EmbeddedGUI/src/resource/egui_icon_material_symbols.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define BADGE_ROOT_WIDTH           224
#define BADGE_ROOT_HEIGHT          190
#define BADGE_PRIMARY_PANEL_WIDTH  196
#define BADGE_PRIMARY_PANEL_HEIGHT 84
#define BADGE_PRIMARY_WIDTH        126
#define BADGE_PRIMARY_HEIGHT       28
#define BADGE_PREVIEW_PANEL_WIDTH  104
#define BADGE_PREVIEW_PANEL_HEIGHT 62
#define BADGE_PREVIEW_WIDTH        88
#define BADGE_PREVIEW_HEIGHT       24
#define BADGE_BOTTOM_ROW_WIDTH     216
#define BADGE_BOTTOM_ROW_HEIGHT    62
#define BADGE_RECORD_WAIT          90
#define BADGE_RECORD_FRAME_WAIT    170
#define BADGE_RECORD_FINAL_WAIT    520

typedef enum
{
    BADGE_STYLE_FILLED = 0,
    BADGE_STYLE_OUTLINE,
    BADGE_STYLE_SUBTLE,
} badge_style_t;

typedef struct
{
    const char *text;
    const char *icon;
    const char *note;
    badge_style_t style;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t accent_color;
} badge_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t primary_heading_label;
static egui_view_badge_t primary_badge;
static egui_view_label_t primary_note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_panel;
static egui_view_label_t compact_heading_label;
static egui_view_badge_t compact_badge;
static egui_view_label_t compact_note_label;
static egui_view_linearlayout_t read_only_panel;
static egui_view_label_t read_only_heading_label;
static egui_view_badge_t read_only_badge;
static egui_view_label_t read_only_note_label;
static egui_view_api_t compact_api;
static egui_view_api_t read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, EGUI_COLOR_HEX(0xFBFCFD), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

static const char *title_text = "Badge";

static const badge_snapshot_t primary_snapshots[] = {
        {
                "Verified",
                EGUI_ICON_MS_DONE,
                "Filled badge keeps the strongest emphasis.",
                BADGE_STYLE_FILLED,
                EGUI_COLOR_HEX(0x0F7B45),
                EGUI_COLOR_HEX(0x0F7B45),
                EGUI_COLOR_WHITE,
                EGUI_COLOR_WHITE,
        },
        {
                "Preview",
                EGUI_ICON_MS_INFO,
                "Outline badge reads as metadata, not action.",
                BADGE_STYLE_OUTLINE,
                EGUI_COLOR_HEX(0xFFFFFF),
                EGUI_COLOR_HEX(0xB7CBE7),
                EGUI_COLOR_HEX(0x0F548C),
                EGUI_COLOR_HEX(0x0F6CBD),
        },
        {
                "Needs review",
                EGUI_ICON_MS_WARNING,
                "Subtle badge stays low-noise beside content.",
                BADGE_STYLE_SUBTLE,
                EGUI_COLOR_HEX(0xFFF6EE),
                EGUI_COLOR_HEX(0xF2D4B2),
                EGUI_COLOR_HEX(0x8F4C11),
                EGUI_COLOR_HEX(0xB95A00),
        },
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
    egui_view_set_padding(EGUI_VIEW_OF(panel), 10, 10, 10, 10);
}

static void apply_primary_snapshot(uint8_t index)
{
    const badge_snapshot_t *snapshot = &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)];

    switch (snapshot->style)
    {
    case BADGE_STYLE_FILLED:
        egui_view_badge_apply_filled_style(EGUI_VIEW_OF(&primary_badge));
        break;
    case BADGE_STYLE_OUTLINE:
        egui_view_badge_apply_outline_style(EGUI_VIEW_OF(&primary_badge));
        break;
    default:
        egui_view_badge_apply_subtle_style(EGUI_VIEW_OF(&primary_badge));
        break;
    }

    egui_view_badge_set_text(EGUI_VIEW_OF(&primary_badge), snapshot->text);
    egui_view_badge_set_icon(EGUI_VIEW_OF(&primary_badge), snapshot->icon);
    egui_view_badge_set_palette(EGUI_VIEW_OF(&primary_badge), snapshot->surface_color, snapshot->border_color, snapshot->text_color, snapshot->accent_color);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_note_label), snapshot->note);
}

static void apply_compact_state(void)
{
    egui_view_badge_apply_outline_style(EGUI_VIEW_OF(&compact_badge));
    egui_view_badge_set_text(EGUI_VIEW_OF(&compact_badge), "Beta");
    egui_view_badge_set_icon(EGUI_VIEW_OF(&compact_badge), EGUI_ICON_MS_INFO);
    egui_view_badge_set_palette(EGUI_VIEW_OF(&compact_badge), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xC6D5E8), EGUI_COLOR_HEX(0x124B78),
                                EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_badge_set_compact_mode(EGUI_VIEW_OF(&compact_badge), 1);
}

static void apply_read_only_state(void)
{
    egui_view_badge_apply_read_only_style(EGUI_VIEW_OF(&read_only_badge));
    egui_view_badge_set_text(EGUI_VIEW_OF(&read_only_badge), "Archived");
    egui_view_badge_set_icon(EGUI_VIEW_OF(&read_only_badge), NULL);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), BADGE_ROOT_WIDTH, BADGE_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, BADGE_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_panel(&primary_panel, BADGE_PRIMARY_PANEL_WIDTH, BADGE_PRIMARY_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&primary_heading_label, 176, 12, "Associated status", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x5E6D7C),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_heading_label));

    egui_view_badge_init(EGUI_VIEW_OF(&primary_badge));
    egui_view_set_size(EGUI_VIEW_OF(&primary_badge), BADGE_PRIMARY_WIDTH, BADGE_PRIMARY_HEIGHT);
    egui_view_badge_set_font(EGUI_VIEW_OF(&primary_badge), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_badge_set_icon_font(EGUI_VIEW_OF(&primary_badge), EGUI_FONT_ICON_MS_16);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_badge), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_badge));

    init_text_label(&primary_note_label, 176, 10, "Filled badge keeps the strongest emphasis.", (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), BADGE_BOTTOM_ROW_WIDTH, BADGE_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&compact_panel, BADGE_PREVIEW_PANEL_WIDTH, BADGE_PREVIEW_PANEL_HEIGHT, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_panel));

    init_text_label(&compact_heading_label, 84, 12, "Compact", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_heading_label));

    egui_view_badge_init(EGUI_VIEW_OF(&compact_badge));
    egui_view_set_size(EGUI_VIEW_OF(&compact_badge), BADGE_PREVIEW_WIDTH, BADGE_PREVIEW_HEIGHT);
    egui_view_badge_set_font(EGUI_VIEW_OF(&compact_badge), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_badge_set_icon_font(EGUI_VIEW_OF(&compact_badge), EGUI_FONT_ICON_MS_16);
    egui_view_badge_override_static_preview_api(EGUI_VIEW_OF(&compact_badge), &compact_api);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_badge), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_badge));

    init_text_label(&compact_note_label, 84, 12, "Static preview.", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89),
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_note_label));

    init_panel(&read_only_panel, BADGE_PREVIEW_PANEL_WIDTH, BADGE_PREVIEW_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_panel));

    init_text_label(&read_only_heading_label, 84, 12, "Read only", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_panel), EGUI_VIEW_OF(&read_only_heading_label));

    egui_view_badge_init(EGUI_VIEW_OF(&read_only_badge));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_badge), BADGE_PREVIEW_WIDTH, BADGE_PREVIEW_HEIGHT);
    egui_view_badge_set_font(EGUI_VIEW_OF(&read_only_badge), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_badge_override_static_preview_api(EGUI_VIEW_OF(&read_only_badge), &read_only_api);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_badge), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_panel), EGUI_VIEW_OF(&read_only_badge));

    init_text_label(&read_only_note_label, 84, 12, "Muted reference.", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89),
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_panel), EGUI_VIEW_OF(&read_only_note_label));

    apply_primary_snapshot(0);
    apply_compact_state();
    apply_read_only_state();

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
            apply_compact_state();
            apply_read_only_state();
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(0);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
