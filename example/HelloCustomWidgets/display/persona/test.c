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
#define PERSONA_BOTTOM_ROW_WIDTH  216
#define PERSONA_BOTTOM_ROW_HEIGHT 78
#define PERSONA_RECORD_WAIT       90
#define PERSONA_RECORD_FRAME_WAIT 170
#define PERSONA_RECORD_FINAL_WAIT 280
#define PERSONA_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct persona_snapshot persona_snapshot_t;
struct persona_snapshot
{
    const char *display_name;
    const char *secondary_text;
    const char *tertiary_text;
    const char *quaternary_text;
    const char *initials;
    uint8_t tone;
    uint8_t status;
    const char *status_label;
    egui_color_t status_label_color;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_persona_t primary_persona;
static egui_view_label_t primary_status_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_persona_t compact_persona;
static egui_view_persona_t read_only_persona;
static egui_view_api_t compact_persona_api;
static egui_view_api_t read_only_persona_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Persona";

static const persona_snapshot_t primary_snapshots[] = {
        {
                "Lena Marsh",
                "Principal designer",
                "Focuses on spacing audit",
                "Available for review",
                NULL,
                EGUI_VIEW_PERSONA_TONE_ACCENT,
                EGUI_VIEW_PERSONA_STATUS_AVAILABLE,
                "Lena Marsh / available",
                EGUI_COLOR_HEX(0x0F6CBD),
        },
        {
                "Aria Rowan",
                "Release manager",
                "Holding export approvals",
                "Busy until 16:00",
                "AR",
                EGUI_VIEW_PERSONA_TONE_SUCCESS,
                EGUI_VIEW_PERSONA_STATUS_BUSY,
                "Aria Rowan / busy",
                EGUI_COLOR_HEX(0x9D5D00),
        },
        {
                "Mina Brooks",
                "Archive owner",
                "Handles retention queue",
                "Offline after sync",
                "MB",
                EGUI_VIEW_PERSONA_TONE_NEUTRAL,
                EGUI_VIEW_PERSONA_STATUS_OFFLINE,
                "Mina Brooks / offline",
                EGUI_COLOR_HEX(0x6B7A89),
        },
};

static void layout_page(void);

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font,
                            egui_color_t color, uint8_t align)
{
    egui_view_label_init(EGUI_VIEW_OF(label));
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align);
    egui_view_label_set_font(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

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
    const persona_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    apply_snapshot(&primary_persona, snapshot);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_status_label), snapshot->status_label);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_status_label), snapshot->status_label_color, EGUI_ALPHA_100);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(PERSONA_DEFAULT_SNAPSHOT);
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
            NULL,
            EGUI_COLOR_HEX(0x000000),
    };
    static const persona_snapshot_t read_only_snapshot = {
            "Jin Park",
            "Compliance archive",
            "Read-only audit trail",
            NULL,
            "JP",
            EGUI_VIEW_PERSONA_TONE_NEUTRAL,
            EGUI_VIEW_PERSONA_STATUS_DO_NOT_DISTURB,
            NULL,
            EGUI_COLOR_HEX(0x000000),
    };

    apply_snapshot(&compact_persona, &compact_snapshot);
    egui_view_persona_set_compact_mode(EGUI_VIEW_OF(&compact_persona), 1);

    apply_snapshot(&read_only_persona, &read_only_snapshot);
    egui_view_persona_set_compact_mode(EGUI_VIEW_OF(&read_only_persona), 1);
    egui_view_persona_set_read_only_mode(EGUI_VIEW_OF(&read_only_persona), 1);
    egui_view_persona_set_palette(EGUI_VIEW_OF(&read_only_persona), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD8DFE6), EGUI_COLOR_HEX(0xEEF2F6),
                                  EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x99A6B2), EGUI_COLOR_HEX(0xA7B4C1), EGUI_COLOR_HEX(0xB2C4BA),
                                  EGUI_COLOR_HEX(0xC4B8A4), EGUI_COLOR_HEX(0xB4BDC8));

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
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), PERSONA_ROOT_WIDTH, PERSONA_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, PERSONA_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_persona_init(EGUI_VIEW_OF(&primary_persona));
    egui_view_set_size(EGUI_VIEW_OF(&primary_persona), PERSONA_PRIMARY_WIDTH, PERSONA_PRIMARY_HEIGHT);
    egui_view_persona_set_font(EGUI_VIEW_OF(&primary_persona), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_persona_set_meta_font(EGUI_VIEW_OF(&primary_persona), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_persona), 0, 0, 0, 6);
    #if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&primary_persona), 0);
    #endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_persona));

    init_text_label(&primary_status_label, PERSONA_ROOT_WIDTH, 12, "Lena Marsh / available", (const egui_font_t *)&egui_res_font_montserrat_10_4,
                    EGUI_COLOR_HEX(0x0F6CBD), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_status_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_status_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), PERSONA_BOTTOM_ROW_WIDTH, PERSONA_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_persona_init(EGUI_VIEW_OF(&compact_persona));
    egui_view_set_size(EGUI_VIEW_OF(&compact_persona), 104, 78);
    egui_view_persona_set_font(EGUI_VIEW_OF(&compact_persona), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_persona_set_meta_font(EGUI_VIEW_OF(&compact_persona), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_persona_override_static_preview_api(EGUI_VIEW_OF(&compact_persona), &compact_persona_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_persona), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_persona));

    egui_view_persona_init(EGUI_VIEW_OF(&read_only_persona));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_persona), 104, 78);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_persona), 8, 0, 0, 0);
    egui_view_persona_set_font(EGUI_VIEW_OF(&read_only_persona), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_persona_set_meta_font(EGUI_VIEW_OF(&read_only_persona), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_persona_override_static_preview_api(EGUI_VIEW_OF(&read_only_persona), &read_only_persona_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&read_only_persona), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_persona));

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
        EGUI_SIM_SET_WAIT(p_action, PERSONA_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
