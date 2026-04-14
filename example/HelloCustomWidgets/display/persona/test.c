#include "egui.h"
#include "egui_view_persona.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define PERSONA_ROOT_WIDTH        224
#define PERSONA_ROOT_HEIGHT       238
#define PERSONA_PRIMARY_WIDTH     196
#define PERSONA_PRIMARY_HEIGHT    104
#define PERSONA_PREVIEW_WIDTH     104
#define PERSONA_PREVIEW_HEIGHT    78
#define PERSONA_BOTTOM_ROW_WIDTH  216
#define PERSONA_BOTTOM_ROW_HEIGHT 78
#define PERSONA_RECORD_WAIT       90
#define PERSONA_RECORD_FRAME_WAIT 170

typedef struct
{
    const char *display_name;
    const char *secondary_text;
    const char *tertiary_text;
    const char *quaternary_text;
    const char *initials;
    uint8_t tone;
    uint8_t status;
} persona_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_persona_t persona_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_persona_t persona_compact;
static egui_view_persona_t persona_read_only;
static egui_view_api_t persona_compact_api;
static egui_view_api_t persona_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Persona";

static const persona_snapshot_t primary_snapshots[] = {
        {"Lena Marsh", "Principal designer", "Focuses on spacing audit", "Available for review", NULL, EGUI_VIEW_PERSONA_TONE_ACCENT,
         EGUI_VIEW_PERSONA_STATUS_AVAILABLE},
        {"Aria Rowan", "Release manager", "Holding export approvals", "Busy until 16:00", "AR", EGUI_VIEW_PERSONA_TONE_SUCCESS,
         EGUI_VIEW_PERSONA_STATUS_BUSY},
        {"Mina Brooks", "Archive owner", "Handles retention queue", "Offline after sync", "MB", EGUI_VIEW_PERSONA_TONE_NEUTRAL,
         EGUI_VIEW_PERSONA_STATUS_OFFLINE},
};

static void apply_snapshot(egui_view_persona_t *persona, const persona_snapshot_t *snapshot)
{
    egui_view_persona_set_display_name(EGUI_VIEW_OF(persona), snapshot->display_name);
    egui_view_persona_set_secondary_text(EGUI_VIEW_OF(persona), snapshot->secondary_text);
    egui_view_persona_set_tertiary_text(EGUI_VIEW_OF(persona), snapshot->tertiary_text);
    egui_view_persona_set_quaternary_text(EGUI_VIEW_OF(persona), snapshot->quaternary_text);
    egui_view_persona_set_initials(EGUI_VIEW_OF(persona), snapshot->initials);
    egui_view_persona_set_tone(EGUI_VIEW_OF(persona), snapshot->tone);
    egui_view_persona_set_status(EGUI_VIEW_OF(persona), snapshot->status);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot(&persona_primary, &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)]);
}

static void apply_preview_states(void)
{
    static const persona_snapshot_t compact_snapshot = {
            "Maya Yu",
            "Research lead",
            NULL,
            NULL,
            NULL,
            EGUI_VIEW_PERSONA_TONE_WARNING,
            EGUI_VIEW_PERSONA_STATUS_AWAY,
    };
    static const persona_snapshot_t read_only_snapshot = {
            "Jin Park",
            "Compliance archive",
            "Read-only audit trail",
            NULL,
            "JP",
            EGUI_VIEW_PERSONA_TONE_NEUTRAL,
            EGUI_VIEW_PERSONA_STATUS_DO_NOT_DISTURB,
    };

    apply_snapshot(&persona_compact, &compact_snapshot);
    egui_view_persona_set_compact_mode(EGUI_VIEW_OF(&persona_compact), 1);

    apply_snapshot(&persona_read_only, &read_only_snapshot);
    egui_view_persona_set_compact_mode(EGUI_VIEW_OF(&persona_read_only), 1);
    egui_view_persona_set_read_only_mode(EGUI_VIEW_OF(&persona_read_only), 1);
    egui_view_persona_set_palette(EGUI_VIEW_OF(&persona_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD8DFE6), EGUI_COLOR_HEX(0xEEF2F6),
                                  EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x99A6B2), EGUI_COLOR_HEX(0xA7B4C1), EGUI_COLOR_HEX(0xB2C4BA),
                                  EGUI_COLOR_HEX(0xC4B8A4), EGUI_COLOR_HEX(0xB4BDC8));
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), PERSONA_ROOT_WIDTH, PERSONA_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), PERSONA_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_persona_init(EGUI_VIEW_OF(&persona_primary));
    egui_view_set_size(EGUI_VIEW_OF(&persona_primary), PERSONA_PRIMARY_WIDTH, PERSONA_PRIMARY_HEIGHT);
    egui_view_persona_set_font(EGUI_VIEW_OF(&persona_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_persona_set_meta_font(EGUI_VIEW_OF(&persona_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_set_margin(EGUI_VIEW_OF(&persona_primary), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&persona_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), PERSONA_BOTTOM_ROW_WIDTH, PERSONA_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_persona_init(EGUI_VIEW_OF(&persona_compact));
    egui_view_set_size(EGUI_VIEW_OF(&persona_compact), PERSONA_PREVIEW_WIDTH, PERSONA_PREVIEW_HEIGHT);
    egui_view_persona_set_font(EGUI_VIEW_OF(&persona_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_persona_set_meta_font(EGUI_VIEW_OF(&persona_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_persona_override_static_preview_api(EGUI_VIEW_OF(&persona_compact), &persona_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&persona_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&persona_compact));

    egui_view_persona_init(EGUI_VIEW_OF(&persona_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&persona_read_only), PERSONA_PREVIEW_WIDTH, PERSONA_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&persona_read_only), 8, 0, 0, 0);
    egui_view_persona_set_font(EGUI_VIEW_OF(&persona_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_persona_set_meta_font(EGUI_VIEW_OF(&persona_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_persona_override_static_preview_api(EGUI_VIEW_OF(&persona_read_only), &persona_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&persona_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&persona_read_only));

    apply_primary_snapshot(0);
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

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
        EGUI_SIM_SET_WAIT(p_action, PERSONA_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_RECORD_FRAME_WAIT);
        return true;
    case 6:
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
