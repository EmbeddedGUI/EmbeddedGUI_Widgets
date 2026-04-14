#include "egui.h"
#include "egui_view_counter_badge.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define COUNTER_BADGE_ROOT_WIDTH        224
#define COUNTER_BADGE_ROOT_HEIGHT       206
#define COUNTER_BADGE_PANEL_WIDTH       196
#define COUNTER_BADGE_PANEL_HEIGHT      96
#define COUNTER_BADGE_PRIMARY_WIDTH     42
#define COUNTER_BADGE_PRIMARY_HEIGHT    24
#define COUNTER_BADGE_PREVIEW_WIDTH     104
#define COUNTER_BADGE_PREVIEW_HEIGHT    68
#define COUNTER_BADGE_BOTTOM_ROW_WIDTH  216
#define COUNTER_BADGE_BOTTOM_ROW_HEIGHT 68
#define COUNTER_BADGE_RECORD_WAIT       90
#define COUNTER_BADGE_RECORD_FRAME_WAIT 170
#define COUNTER_BADGE_RECORD_FINAL_WAIT 520

typedef struct
{
    const char *heading;
    const char *summary;
    const char *note;
    uint16_t count;
    uint8_t max_display;
    uint8_t dot_mode;
    egui_color_t badge_color;
    egui_color_t summary_color;
} counter_badge_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t primary_heading_label;
static egui_view_counter_badge_t primary_badge;
static egui_view_label_t primary_summary_label;
static egui_view_label_t primary_note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_panel;
static egui_view_label_t compact_heading_label;
static egui_view_counter_badge_t compact_badge;
static egui_view_label_t compact_note_label;
static egui_view_linearlayout_t read_only_panel;
static egui_view_label_t read_only_heading_label;
static egui_view_counter_badge_t read_only_badge;
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

static const char *title_text = "CounterBadge";

static const counter_badge_snapshot_t primary_snapshots[] = {
        {
                "Inbox queue",
                "7 unread threads",
                "Single counts stay compact and attached.",
                7,
                99,
                0,
                EGUI_COLOR_HEX(0xC42B1C),
                EGUI_COLOR_HEX(0xC42B1C),
        },
        {
                "Escalation queue",
                "99+ pending reviews",
                "Overflow clamps without stretching the host shell.",
                128,
                99,
                0,
                EGUI_COLOR_HEX(0xC42B1C),
                EGUI_COLOR_HEX(0xC42B1C),
        },
        {
                "Quiet watch",
                "Background attention",
                "Dot mode collapses into a lower-noise signal.",
                1,
                99,
                1,
                EGUI_COLOR_HEX(0x0F6CBD),
                EGUI_COLOR_HEX(0x0F6CBD),
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
    const counter_badge_snapshot_t *snapshot = &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)];

    egui_view_label_set_text(EGUI_VIEW_OF(&primary_heading_label), snapshot->heading);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_summary_label), snapshot->summary);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_summary_label), snapshot->summary_color, EGUI_ALPHA_100);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_note_label), snapshot->note);

    egui_view_counter_badge_set_count(EGUI_VIEW_OF(&primary_badge), snapshot->count);
    egui_view_counter_badge_set_max_display(EGUI_VIEW_OF(&primary_badge), snapshot->max_display);
    egui_view_counter_badge_set_dot_mode(EGUI_VIEW_OF(&primary_badge), snapshot->dot_mode);
    egui_view_counter_badge_set_palette(EGUI_VIEW_OF(&primary_badge), snapshot->badge_color, EGUI_COLOR_WHITE, EGUI_COLOR_WHITE);
}

static void apply_preview_states(void)
{
    egui_view_counter_badge_set_count(EGUI_VIEW_OF(&compact_badge), 4);
    egui_view_counter_badge_set_compact_mode(EGUI_VIEW_OF(&compact_badge), 1);

    egui_view_counter_badge_set_count(EGUI_VIEW_OF(&read_only_badge), 12);
    egui_view_counter_badge_set_compact_mode(EGUI_VIEW_OF(&read_only_badge), 1);
    egui_view_counter_badge_set_read_only_mode(EGUI_VIEW_OF(&read_only_badge), 1);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), COUNTER_BADGE_ROOT_WIDTH, COUNTER_BADGE_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, COUNTER_BADGE_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_panel(&primary_panel, COUNTER_BADGE_PANEL_WIDTH, COUNTER_BADGE_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&primary_heading_label, 176, 12, "Inbox queue", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x51606F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_heading_label));

    egui_view_counter_badge_init(EGUI_VIEW_OF(&primary_badge));
    egui_view_set_size(EGUI_VIEW_OF(&primary_badge), COUNTER_BADGE_PRIMARY_WIDTH, COUNTER_BADGE_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_badge), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_badge));

    init_text_label(&primary_summary_label, 176, 12, "7 unread threads", (const egui_font_t *)&egui_res_font_montserrat_10_4, EGUI_COLOR_HEX(0xC42B1C),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_summary_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_summary_label));

    init_text_label(&primary_note_label, 176, 12, "Single counts stay compact and attached.", (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), COUNTER_BADGE_BOTTOM_ROW_WIDTH, COUNTER_BADGE_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&compact_panel, COUNTER_BADGE_PREVIEW_WIDTH, COUNTER_BADGE_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_panel));

    init_text_label(&compact_heading_label, 84, 12, "Compact", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_heading_label));

    egui_view_counter_badge_init(EGUI_VIEW_OF(&compact_badge));
    egui_view_set_size(EGUI_VIEW_OF(&compact_badge), 18, 16);
    egui_view_counter_badge_override_static_preview_api(EGUI_VIEW_OF(&compact_badge), &compact_api);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_badge), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_badge));

    init_text_label(&compact_note_label, 84, 12, "Count 4", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x51606F),
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_note_label));

    init_panel(&read_only_panel, COUNTER_BADGE_PREVIEW_WIDTH, COUNTER_BADGE_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_panel));

    init_text_label(&read_only_heading_label, 84, 12, "Read only", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_panel), EGUI_VIEW_OF(&read_only_heading_label));

    egui_view_counter_badge_init(EGUI_VIEW_OF(&read_only_badge));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_badge), 22, 16);
    egui_view_counter_badge_override_static_preview_api(EGUI_VIEW_OF(&read_only_badge), &read_only_api);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_badge), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_panel), EGUI_VIEW_OF(&read_only_badge));

    init_text_label(&read_only_note_label, 84, 12, "Muted 12", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89),
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_panel), EGUI_VIEW_OF(&read_only_note_label));

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
        }
        EGUI_SIM_SET_WAIT(p_action, COUNTER_BADGE_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COUNTER_BADGE_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, COUNTER_BADGE_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COUNTER_BADGE_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, COUNTER_BADGE_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COUNTER_BADGE_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(0);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COUNTER_BADGE_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
