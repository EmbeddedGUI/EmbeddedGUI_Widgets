#include "egui.h"
#include "egui_view_persona_group.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define PERSONA_GROUP_ROOT_WIDTH        224
#define PERSONA_GROUP_ROOT_HEIGHT       230
#define PERSONA_GROUP_PRIMARY_WIDTH     196
#define PERSONA_GROUP_PRIMARY_HEIGHT    114
#define PERSONA_GROUP_PREVIEW_WIDTH     104
#define PERSONA_GROUP_PREVIEW_HEIGHT    76
#define PERSONA_GROUP_BOTTOM_ROW_WIDTH  216
#define PERSONA_GROUP_BOTTOM_ROW_HEIGHT 76
#define PERSONA_GROUP_RECORD_WAIT       90
#define PERSONA_GROUP_RECORD_FRAME_WAIT 170
#define PERSONA_GROUP_RECORD_FINAL_WAIT 280
#define PERSONA_GROUP_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_persona_group_t group_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_persona_group_t group_compact;
static egui_view_persona_group_t group_read_only;
static egui_view_api_t group_compact_api;
static egui_view_api_t group_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Persona Group";

static const egui_view_persona_group_item_t primary_items_0[] = {
        {"LM", "Lena", "Design", EGUI_VIEW_PERSONA_GROUP_TONE_ACCENT, EGUI_VIEW_PERSONA_GROUP_PRESENCE_LIVE, 1},
        {"AR", "Arun", "Ops", EGUI_VIEW_PERSONA_GROUP_TONE_SUCCESS, EGUI_VIEW_PERSONA_GROUP_PRESENCE_BUSY, 0},
        {"MY", "Maya", "QA", EGUI_VIEW_PERSONA_GROUP_TONE_WARNING, EGUI_VIEW_PERSONA_GROUP_PRESENCE_LIVE, 0},
        {"JN", "Jin", "Content", EGUI_VIEW_PERSONA_GROUP_TONE_NEUTRAL, EGUI_VIEW_PERSONA_GROUP_PRESENCE_AWAY, 0},
};

static const egui_view_persona_group_item_t primary_items_1[] = {
        {"SO", "Sora", "Lead", EGUI_VIEW_PERSONA_GROUP_TONE_SUCCESS, EGUI_VIEW_PERSONA_GROUP_PRESENCE_LIVE, 0},
        {"IV", "Ivy", "PM", EGUI_VIEW_PERSONA_GROUP_TONE_ACCENT, EGUI_VIEW_PERSONA_GROUP_PRESENCE_BUSY, 1},
        {"TE", "Teo", "QA", EGUI_VIEW_PERSONA_GROUP_TONE_WARNING, EGUI_VIEW_PERSONA_GROUP_PRESENCE_AWAY, 0},
        {"AL", "Ali", "Docs", EGUI_VIEW_PERSONA_GROUP_TONE_NEUTRAL, EGUI_VIEW_PERSONA_GROUP_PRESENCE_IDLE, 0},
};

static const egui_view_persona_group_item_t primary_items_2[] = {
        {"MB", "Mina", "Archive", EGUI_VIEW_PERSONA_GROUP_TONE_NEUTRAL, EGUI_VIEW_PERSONA_GROUP_PRESENCE_IDLE, 1},
        {"KO", "Kora", "QA", EGUI_VIEW_PERSONA_GROUP_TONE_WARNING, EGUI_VIEW_PERSONA_GROUP_PRESENCE_AWAY, 0},
        {"YU", "Yuri", "Restore", EGUI_VIEW_PERSONA_GROUP_TONE_ACCENT, EGUI_VIEW_PERSONA_GROUP_PRESENCE_LIVE, 0},
        {"RA", "Rae", "Records", EGUI_VIEW_PERSONA_GROUP_TONE_SUCCESS, EGUI_VIEW_PERSONA_GROUP_PRESENCE_BUSY, 0},
};

static const egui_view_persona_group_snapshot_t primary_snapshots[] = {
        {"DESIGN", "Design review", "Design team", primary_items_0, 4, 0, 2},
        {"OPS", "Ops handoff", "Shift lead", primary_items_1, 4, 1, 1},
        {"ARCHIVE", "Archive sweep", "Restore desk", primary_items_2, 4, 0, 1},
};

static const egui_view_persona_group_item_t compact_items[] = {
        {"LM", "Lena", "Lead", EGUI_VIEW_PERSONA_GROUP_TONE_ACCENT, EGUI_VIEW_PERSONA_GROUP_PRESENCE_LIVE, 1},
        {"AR", "Arun", "Ops", EGUI_VIEW_PERSONA_GROUP_TONE_SUCCESS, EGUI_VIEW_PERSONA_GROUP_PRESENCE_BUSY, 0},
        {"MY", "Maya", "QA", EGUI_VIEW_PERSONA_GROUP_TONE_WARNING, EGUI_VIEW_PERSONA_GROUP_PRESENCE_LIVE, 0},
};

static const egui_view_persona_group_snapshot_t compact_snapshots[] = {
        {"", "Compact", "Short roster", compact_items, 3, 0, 1},
};

static const egui_view_persona_group_item_t read_only_items[] = {
        {"MB", "Mina", "Archive owner", EGUI_VIEW_PERSONA_GROUP_TONE_NEUTRAL, EGUI_VIEW_PERSONA_GROUP_PRESENCE_IDLE, 1},
        {"KO", "Kora", "Retention QA", EGUI_VIEW_PERSONA_GROUP_TONE_WARNING, EGUI_VIEW_PERSONA_GROUP_PRESENCE_AWAY, 0},
        {"YU", "Yuri", "Restore desk", EGUI_VIEW_PERSONA_GROUP_TONE_ACCENT, EGUI_VIEW_PERSONA_GROUP_PRESENCE_LIVE, 0},
};

static const egui_view_persona_group_snapshot_t read_only_snapshots[] = {
        {"", "Read only", "Muted roster", read_only_items, 3, 0, 1},
};

static void layout_page(void);

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_persona_group_set_current_snapshot(EGUI_VIEW_OF(&group_primary), index % PRIMARY_SNAPSHOT_COUNT);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(PERSONA_GROUP_DEFAULT_SNAPSHOT);
}

static void apply_compact_state(void)
{
    egui_view_persona_group_set_current_snapshot(EGUI_VIEW_OF(&group_compact), 0);
    egui_view_persona_group_set_compact_mode(EGUI_VIEW_OF(&group_compact), 1);
    egui_view_persona_group_set_read_only_mode(EGUI_VIEW_OF(&group_compact), 0);
}

static void apply_read_only_state(void)
{
    egui_view_persona_group_set_current_snapshot(EGUI_VIEW_OF(&group_read_only), 0);
    egui_view_persona_group_set_compact_mode(EGUI_VIEW_OF(&group_read_only), 1);
    egui_view_persona_group_set_read_only_mode(EGUI_VIEW_OF(&group_read_only), 1);
}

static void apply_preview_states(void)
{
    apply_compact_state();
    apply_read_only_state();

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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), PERSONA_GROUP_ROOT_WIDTH, PERSONA_GROUP_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), PERSONA_GROUP_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_persona_group_init(EGUI_VIEW_OF(&group_primary));
    egui_view_set_size(EGUI_VIEW_OF(&group_primary), PERSONA_GROUP_PRIMARY_WIDTH, PERSONA_GROUP_PRIMARY_HEIGHT);
    egui_view_persona_group_set_snapshots(EGUI_VIEW_OF(&group_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_persona_group_set_font(EGUI_VIEW_OF(&group_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_persona_group_set_meta_font(EGUI_VIEW_OF(&group_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_persona_group_set_palette(EGUI_VIEW_OF(&group_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0xEEF3F7),
                                        EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                        EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x7A8796));
    egui_view_set_margin(EGUI_VIEW_OF(&group_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&group_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), PERSONA_GROUP_BOTTOM_ROW_WIDTH, PERSONA_GROUP_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_persona_group_init(EGUI_VIEW_OF(&group_compact));
    egui_view_set_size(EGUI_VIEW_OF(&group_compact), PERSONA_GROUP_PREVIEW_WIDTH, PERSONA_GROUP_PREVIEW_HEIGHT);
    egui_view_persona_group_set_snapshots(EGUI_VIEW_OF(&group_compact), compact_snapshots, EGUI_ARRAY_SIZE(compact_snapshots));
    egui_view_persona_group_set_compact_mode(EGUI_VIEW_OF(&group_compact), 1);
    egui_view_persona_group_set_font(EGUI_VIEW_OF(&group_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_persona_group_set_meta_font(EGUI_VIEW_OF(&group_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_persona_group_set_palette(EGUI_VIEW_OF(&group_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0xEEF3F7),
                                        EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                        EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x7A8796));
    egui_view_persona_group_override_static_preview_api(EGUI_VIEW_OF(&group_compact), &group_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&group_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&group_compact));

    egui_view_persona_group_init(EGUI_VIEW_OF(&group_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&group_read_only), PERSONA_GROUP_PREVIEW_WIDTH, PERSONA_GROUP_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&group_read_only), 8, 0, 0, 0);
    egui_view_persona_group_set_snapshots(EGUI_VIEW_OF(&group_read_only), read_only_snapshots, EGUI_ARRAY_SIZE(read_only_snapshots));
    egui_view_persona_group_set_compact_mode(EGUI_VIEW_OF(&group_read_only), 1);
    egui_view_persona_group_set_read_only_mode(EGUI_VIEW_OF(&group_read_only), 1);
    egui_view_persona_group_set_font(EGUI_VIEW_OF(&group_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_persona_group_set_meta_font(EGUI_VIEW_OF(&group_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_persona_group_set_palette(EGUI_VIEW_OF(&group_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD8DFE6), EGUI_COLOR_HEX(0xEEF2F6),
                                        EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x99A6B2), EGUI_COLOR_HEX(0xA7B4C1), EGUI_COLOR_HEX(0xB2C4BA),
                                        EGUI_COLOR_HEX(0xC4B8A4), EGUI_COLOR_HEX(0xB4BDC8));
    egui_view_persona_group_override_static_preview_api(EGUI_VIEW_OF(&group_read_only), &group_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&group_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&group_read_only));

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
        EGUI_SIM_SET_WAIT(p_action, PERSONA_GROUP_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_GROUP_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_GROUP_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_GROUP_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_GROUP_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PERSONA_GROUP_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
