#include <string.h>

#include "egui.h"
#include "egui_view_annotated_scroll_bar.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define ANNOTATED_SCROLL_BAR_ROOT_WIDTH        224
#define ANNOTATED_SCROLL_BAR_ROOT_HEIGHT       260
#define ANNOTATED_SCROLL_BAR_PRIMARY_WIDTH     196
#define ANNOTATED_SCROLL_BAR_PRIMARY_HEIGHT    150
#define ANNOTATED_SCROLL_BAR_PREVIEW_WIDTH     104
#define ANNOTATED_SCROLL_BAR_PREVIEW_HEIGHT    68
#define ANNOTATED_SCROLL_BAR_BOTTOM_ROW_WIDTH  216
#define ANNOTATED_SCROLL_BAR_BOTTOM_ROW_HEIGHT 68
#define ANNOTATED_SCROLL_BAR_RECORD_WAIT       110
#define ANNOTATED_SCROLL_BAR_RECORD_FRAME_WAIT 150

typedef struct annotated_scroll_bar_snapshot annotated_scroll_bar_snapshot_t;
struct annotated_scroll_bar_snapshot
{
    const char *title;
    const char *helper;
    const egui_view_annotated_scroll_bar_marker_t *markers;
    uint8_t marker_count;
    egui_dim_t content_length;
    egui_dim_t viewport_length;
    egui_dim_t offset;
    egui_dim_t small_change;
    egui_dim_t large_change;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_annotated_scroll_bar_t annotated_scroll_bar_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_annotated_scroll_bar_t annotated_scroll_bar_compact;
static egui_view_linearlayout_t locked_column;
static egui_view_annotated_scroll_bar_t annotated_scroll_bar_locked;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Annotated Scroll Bar";
static uint8_t primary_snapshot_index = 0;
static uint8_t compact_snapshot_index = 0;

static const egui_view_annotated_scroll_bar_marker_t gallery_markers[] = {
        {"2019", "Travel imports", 0, EGUI_COLOR_HEX(0x0F6CBD)},    {"2020", "Remote shoots", 146, EGUI_COLOR_HEX(0x0F6CBD)},
        {"2021", "Archive cleanup", 312, EGUI_COLOR_HEX(0x0F6CBD)}, {"2022", "Color picks", 486, EGUI_COLOR_HEX(0x0F6CBD)},
        {"2023", "Review batch", 674, EGUI_COLOR_HEX(0x0F6CBD)},    {"2024", "Client selects", 860, EGUI_COLOR_HEX(0x0F6CBD)},
};

static const egui_view_annotated_scroll_bar_marker_t release_markers[] = {
        {"Alpha", "Feature freeze", 0, EGUI_COLOR_HEX(0x9D5D00)}, {"Beta", "Docs pass", 124, EGUI_COLOR_HEX(0x9D5D00)},
        {"RC1", "Perf sweep", 278, EGUI_COLOR_HEX(0x9D5D00)},     {"RC2", "Bug triage", 412, EGUI_COLOR_HEX(0x9D5D00)},
        {"GA", "Ship review", 588, EGUI_COLOR_HEX(0x9D5D00)},     {"Patch", "Audit notes", 760, EGUI_COLOR_HEX(0x9D5D00)},
};

static const egui_view_annotated_scroll_bar_marker_t incident_markers[] = {
        {"Ingress", "Wake edge", 0, EGUI_COLOR_HEX(0x0F766E)},  {"Auth", "Token burst", 144, EGUI_COLOR_HEX(0x0F766E)},
        {"Queue", "Retry wall", 284, EGUI_COLOR_HEX(0x0F766E)}, {"Core", "Shard fail", 452, EGUI_COLOR_HEX(0x0F766E)},
        {"Audit", "Replay export", 620, EGUI_COLOR_HEX(0x0F766E)},
};

static const egui_view_annotated_scroll_bar_marker_t compact_gallery_markers[] = {
        {"Mix", "Intro", 0, EGUI_COLOR_HEX(0x0F6CBD)},
        {"Edit", "Board", 104, EGUI_COLOR_HEX(0x0F6CBD)},
        {"Focus", "Focus", 222, EGUI_COLOR_HEX(0x0F6CBD)},
        {"Wrap", "Wrap", 360, EGUI_COLOR_HEX(0x0F6CBD)},
};

static const egui_view_annotated_scroll_bar_marker_t compact_docs_markers[] = {
        {"Plan", "Start", 0, EGUI_COLOR_HEX(0x5E6A75)},
        {"Draft", "Draft", 126, EGUI_COLOR_HEX(0x5E6A75)},
        {"Proof", "Proof", 248, EGUI_COLOR_HEX(0x5E6A75)},
        {"Ship", "Close", 384, EGUI_COLOR_HEX(0x5E6A75)},
};

static const egui_view_annotated_scroll_bar_marker_t locked_markers[] = {
        {"Plan", "Locked rail", 0, EGUI_COLOR_HEX(0xA7B4C1)},
        {"Check", "No input", 118, EGUI_COLOR_HEX(0xA7B4C1)},
        {"Ship", "Read only", 246, EGUI_COLOR_HEX(0xA7B4C1)},
        {"Archive", "Audit trail", 384, EGUI_COLOR_HEX(0xA7B4C1)},
};

static const annotated_scroll_bar_snapshot_t primary_snapshots[] = {
        {"Gallery rail", "Jump by year, keep the line fixed", gallery_markers, (uint8_t)(sizeof(gallery_markers) / sizeof(gallery_markers[0])), 1160, 300, 312, 24,
         136},
        {"Release rail", "Group long notes by milestone", release_markers, (uint8_t)(sizeof(release_markers) / sizeof(release_markers[0])), 980, 220, 278, 20,
         120},
        {"Incident rail", "Jump sections without a thumb", incident_markers, (uint8_t)(sizeof(incident_markers) / sizeof(incident_markers[0])), 880, 260, 284, 18,
         96},
};

static const annotated_scroll_bar_snapshot_t compact_snapshots[] = {
        {NULL, NULL, compact_gallery_markers, (uint8_t)(sizeof(compact_gallery_markers) / sizeof(compact_gallery_markers[0])), 540, 180, 222, 18, 96},
        {NULL, NULL, compact_docs_markers, (uint8_t)(sizeof(compact_docs_markers) / sizeof(compact_docs_markers[0])), 560, 176, 126, 18, 108},
};

static const annotated_scroll_bar_snapshot_t locked_snapshot = {
        NULL, NULL, locked_markers, (uint8_t)(sizeof(locked_markers) / sizeof(locked_markers[0])), 600, 216, 246, 18, 110};

static void apply_snapshot(egui_view_t *view, const annotated_scroll_bar_snapshot_t *snapshot)
{
    egui_view_annotated_scroll_bar_set_title(view, snapshot->title);
    egui_view_annotated_scroll_bar_set_helper(view, snapshot->helper);
    egui_view_annotated_scroll_bar_set_markers(view, snapshot->markers, snapshot->marker_count);
    egui_view_annotated_scroll_bar_set_content_metrics(view, snapshot->content_length, snapshot->viewport_length);
    egui_view_annotated_scroll_bar_set_step_size(view, snapshot->small_change, snapshot->large_change);
    egui_view_annotated_scroll_bar_set_offset(view, snapshot->offset);
    egui_view_annotated_scroll_bar_set_current_part(view, EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL);
}

static void apply_primary_snapshot(uint8_t index)
{
    const annotated_scroll_bar_snapshot_t *snapshot = &primary_snapshots[index % (sizeof(primary_snapshots) / sizeof(primary_snapshots[0]))];

    primary_snapshot_index = index % (sizeof(primary_snapshots) / sizeof(primary_snapshots[0]));
    apply_snapshot(EGUI_VIEW_OF(&annotated_scroll_bar_primary), snapshot);
}

static void apply_compact_snapshot(uint8_t index)
{
    const annotated_scroll_bar_snapshot_t *snapshot = &compact_snapshots[index % (sizeof(compact_snapshots) / sizeof(compact_snapshots[0]))];

    compact_snapshot_index = index % (sizeof(compact_snapshots) / sizeof(compact_snapshots[0]));
    apply_snapshot(EGUI_VIEW_OF(&annotated_scroll_bar_compact), snapshot);
}

static int consume_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(event);
    return 1;
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), ANNOTATED_SCROLL_BAR_ROOT_WIDTH, ANNOTATED_SCROLL_BAR_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), ANNOTATED_SCROLL_BAR_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_annotated_scroll_bar_init(EGUI_VIEW_OF(&annotated_scroll_bar_primary));
    egui_view_set_size(EGUI_VIEW_OF(&annotated_scroll_bar_primary), ANNOTATED_SCROLL_BAR_PRIMARY_WIDTH, ANNOTATED_SCROLL_BAR_PRIMARY_HEIGHT);
    egui_view_annotated_scroll_bar_set_font(EGUI_VIEW_OF(&annotated_scroll_bar_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_annotated_scroll_bar_set_meta_font(EGUI_VIEW_OF(&annotated_scroll_bar_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_annotated_scroll_bar_set_palette(EGUI_VIEW_OF(&annotated_scroll_bar_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4),
                                               EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xAAB6C3));
    egui_view_set_margin(EGUI_VIEW_OF(&annotated_scroll_bar_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&annotated_scroll_bar_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), ANNOTATED_SCROLL_BAR_BOTTOM_ROW_WIDTH, ANNOTATED_SCROLL_BAR_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), ANNOTATED_SCROLL_BAR_PREVIEW_WIDTH, ANNOTATED_SCROLL_BAR_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_annotated_scroll_bar_init(EGUI_VIEW_OF(&annotated_scroll_bar_compact));
    egui_view_set_size(EGUI_VIEW_OF(&annotated_scroll_bar_compact), ANNOTATED_SCROLL_BAR_PREVIEW_WIDTH, ANNOTATED_SCROLL_BAR_PREVIEW_HEIGHT);
    egui_view_annotated_scroll_bar_set_font(EGUI_VIEW_OF(&annotated_scroll_bar_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_annotated_scroll_bar_set_meta_font(EGUI_VIEW_OF(&annotated_scroll_bar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_annotated_scroll_bar_set_compact_mode(EGUI_VIEW_OF(&annotated_scroll_bar_compact), 1);
    egui_view_annotated_scroll_bar_set_palette(EGUI_VIEW_OF(&annotated_scroll_bar_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4),
                                               EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xAAB6C3));
    static egui_view_api_t annotated_scroll_bar_compact_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&annotated_scroll_bar_compact), &annotated_scroll_bar_compact_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&annotated_scroll_bar_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&annotated_scroll_bar_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&locked_column));
    egui_view_set_size(EGUI_VIEW_OF(&locked_column), ANNOTATED_SCROLL_BAR_PREVIEW_WIDTH, ANNOTATED_SCROLL_BAR_BOTTOM_ROW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&locked_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&locked_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&locked_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&locked_column));

    egui_view_annotated_scroll_bar_init(EGUI_VIEW_OF(&annotated_scroll_bar_locked));
    egui_view_set_size(EGUI_VIEW_OF(&annotated_scroll_bar_locked), ANNOTATED_SCROLL_BAR_PREVIEW_WIDTH, ANNOTATED_SCROLL_BAR_PREVIEW_HEIGHT);
    egui_view_annotated_scroll_bar_set_font(EGUI_VIEW_OF(&annotated_scroll_bar_locked), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_annotated_scroll_bar_set_meta_font(EGUI_VIEW_OF(&annotated_scroll_bar_locked), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_annotated_scroll_bar_set_compact_mode(EGUI_VIEW_OF(&annotated_scroll_bar_locked), 1);
    egui_view_annotated_scroll_bar_set_read_only_mode(EGUI_VIEW_OF(&annotated_scroll_bar_locked), 1);
    egui_view_annotated_scroll_bar_set_palette(EGUI_VIEW_OF(&annotated_scroll_bar_locked), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4),
                                               EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xAAB6C3));
    static egui_view_api_t annotated_scroll_bar_locked_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&annotated_scroll_bar_locked), &annotated_scroll_bar_locked_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&annotated_scroll_bar_locked), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&locked_column), EGUI_VIEW_OF(&annotated_scroll_bar_locked));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    apply_snapshot(EGUI_VIEW_OF(&annotated_scroll_bar_locked), &locked_snapshot);

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&locked_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&annotated_scroll_bar_primary)->api->on_key_event(EGUI_VIEW_OF(&annotated_scroll_bar_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&annotated_scroll_bar_primary)->api->on_key_event(EGUI_VIEW_OF(&annotated_scroll_bar_primary), &event);
}

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
            apply_compact_snapshot(0);
        }
        EGUI_SIM_SET_WAIT(p_action, ANNOTATED_SCROLL_BAR_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ANNOTATED_SCROLL_BAR_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_DOWN);
        }
        EGUI_SIM_SET_WAIT(p_action, ANNOTATED_SCROLL_BAR_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ANNOTATED_SCROLL_BAR_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_PLUS);
        }
        EGUI_SIM_SET_WAIT(p_action, ANNOTATED_SCROLL_BAR_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ANNOTATED_SCROLL_BAR_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_END);
        }
        EGUI_SIM_SET_WAIT(p_action, ANNOTATED_SCROLL_BAR_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ANNOTATED_SCROLL_BAR_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_primary_snapshot((uint8_t)(primary_snapshot_index + 1));
        }
        EGUI_SIM_SET_WAIT(p_action, ANNOTATED_SCROLL_BAR_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ANNOTATED_SCROLL_BAR_RECORD_FRAME_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            apply_compact_snapshot((uint8_t)(compact_snapshot_index + 1));
        }
        EGUI_SIM_SET_WAIT(p_action, ANNOTATED_SCROLL_BAR_RECORD_WAIT);
        return true;
    case 11:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ANNOTATED_SCROLL_BAR_RECORD_FRAME_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
