#include "egui.h"
#include "egui_view_shortcut_recorder.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SHORTCUT_ROOT_WIDTH     224
#define SHORTCUT_ROOT_HEIGHT    264
#define SHORTCUT_PRIMARY_WIDTH  194
#define SHORTCUT_PRIMARY_HEIGHT 138
#define SHORTCUT_PREVIEW_WIDTH  106
#define SHORTCUT_PREVIEW_HEIGHT 82
#define SHORTCUT_BOTTOM_WIDTH   218
#define SHORTCUT_BOTTOM_HEIGHT  82
#define SHORTCUT_RECORD_WAIT    120
#define SHORTCUT_RECORD_FRAME   150

typedef struct shortcut_scene shortcut_scene_t;
struct shortcut_scene
{
    uint8_t has_binding;
    uint8_t key_code;
    uint8_t is_shift;
    uint8_t is_ctrl;
    uint8_t listening;
    uint8_t current_part;
    uint8_t current_preset;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_shortcut_recorder_t recorder_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_shortcut_recorder_t recorder_compact;
static egui_view_shortcut_recorder_t recorder_read_only;
static egui_view_api_t recorder_compact_api;
static egui_view_api_t recorder_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Shortcut Recorder";

static const egui_view_shortcut_recorder_preset_t primary_presets[] = {
        {"Search files", "Workspace", EGUI_KEY_CODE_F, 1, 1},
        {"Command bar", "Quick command", EGUI_KEY_CODE_P, 1, 1},
        {"Pin focus", "One tap", EGUI_KEY_CODE_1, 0, 1},
};

static const egui_view_shortcut_recorder_preset_t compact_presets[] = {
        {"Review", "Preset", EGUI_KEY_CODE_P, 1, 1},
        {"Queue", "Preset", EGUI_KEY_CODE_1, 0, 1},
};

static const shortcut_scene_t primary_scenes[] = {
        {1, EGUI_KEY_CODE_K, 0, 1, 0, EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0},
        {1, EGUI_KEY_CODE_K, 0, 1, 1, EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0},
        {1, EGUI_KEY_CODE_P, 1, 1, 0, EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 1},
        {1, EGUI_KEY_CODE_1, 0, 1, 0, EGUI_VIEW_SHORTCUT_RECORDER_PART_PRESET, 2},
        {0, EGUI_KEY_CODE_NONE, 0, 0, 0, EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 2},
};

static const shortcut_scene_t compact_scenes[] = {
        {1, EGUI_KEY_CODE_P, 1, 1, 0, EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0},
        {1, EGUI_KEY_CODE_1, 0, 1, 0, EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 1},
};

static void dismiss_primary_shortcut_recorder(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(EGUI_VIEW_OF(&recorder_primary));
#endif
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        dismiss_primary_shortcut_recorder();
    }
    return 1;
}

static void apply_scene_to_recorder(egui_view_shortcut_recorder_t *recorder, const shortcut_scene_t *scene)
{
    if (scene->has_binding)
    {
        egui_view_shortcut_recorder_set_binding(EGUI_VIEW_OF(recorder), scene->key_code, scene->is_shift, scene->is_ctrl);
    }
    else
    {
        egui_view_shortcut_recorder_clear_binding(EGUI_VIEW_OF(recorder));
    }
    egui_view_shortcut_recorder_set_current_preset(EGUI_VIEW_OF(recorder), scene->current_preset);
    egui_view_shortcut_recorder_set_current_part(EGUI_VIEW_OF(recorder), scene->current_part);
    egui_view_shortcut_recorder_set_listening(EGUI_VIEW_OF(recorder), scene->listening);
}

static void apply_primary_scene(uint8_t index)
{
    apply_scene_to_recorder(&recorder_primary, &primary_scenes[index % EGUI_ARRAY_SIZE(primary_scenes)]);
}

static void apply_compact_scene(uint8_t index)
{
    apply_scene_to_recorder(&recorder_compact, &compact_scenes[index % EGUI_ARRAY_SIZE(compact_scenes)]);
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code, uint8_t is_shift, uint8_t is_ctrl)
{
    egui_key_event_t event = {0};

    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    event.is_shift = is_shift;
    event.is_ctrl = is_ctrl;
    EGUI_VIEW_OF(&recorder_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&recorder_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&recorder_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&recorder_primary), &event);
}

static void get_part_center(egui_view_t *view, uint8_t part, uint8_t preset_index, int *x, int *y)
{
    egui_region_t region = {0};

    if (!egui_view_shortcut_recorder_get_part_region(view, part, preset_index, &region))
    {
        *x = view->region_screen.location.x + view->region_screen.size.width / 2;
        *y = view->region_screen.location.y + view->region_screen.size.height / 2;
        return;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SHORTCUT_ROOT_WIDTH, SHORTCUT_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SHORTCUT_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x22313F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_shortcut_recorder_init(EGUI_VIEW_OF(&recorder_primary));
    egui_view_set_size(EGUI_VIEW_OF(&recorder_primary), SHORTCUT_PRIMARY_WIDTH, SHORTCUT_PRIMARY_HEIGHT);
    egui_view_shortcut_recorder_set_font(EGUI_VIEW_OF(&recorder_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_shortcut_recorder_set_meta_font(EGUI_VIEW_OF(&recorder_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_shortcut_recorder_set_header(EGUI_VIEW_OF(&recorder_primary), "Quick launch", "Capture a shortcut", "Ready to capture");
    egui_view_shortcut_recorder_set_presets(EGUI_VIEW_OF(&recorder_primary), primary_presets, EGUI_ARRAY_SIZE(primary_presets));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&recorder_primary), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&recorder_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&recorder_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SHORTCUT_BOTTOM_WIDTH, SHORTCUT_BOTTOM_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_shortcut_recorder_init(EGUI_VIEW_OF(&recorder_compact));
    egui_view_set_size(EGUI_VIEW_OF(&recorder_compact), SHORTCUT_PREVIEW_WIDTH, SHORTCUT_PREVIEW_HEIGHT);
    egui_view_shortcut_recorder_set_font(EGUI_VIEW_OF(&recorder_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_shortcut_recorder_set_meta_font(EGUI_VIEW_OF(&recorder_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_shortcut_recorder_set_header(EGUI_VIEW_OF(&recorder_compact), "Compact", "Preset preview", "Peek");
    egui_view_shortcut_recorder_set_presets(EGUI_VIEW_OF(&recorder_compact), compact_presets, EGUI_ARRAY_SIZE(compact_presets));
    egui_view_shortcut_recorder_set_compact_mode(EGUI_VIEW_OF(&recorder_compact), 1);
    egui_view_shortcut_recorder_set_palette(EGUI_VIEW_OF(&recorder_compact), EGUI_COLOR_HEX(0xFCFFFE), EGUI_COLOR_HEX(0xCBE4DE), EGUI_COLOR_HEX(0x12463F),
                                            EGUI_COLOR_HEX(0x5B7D77), EGUI_COLOR_HEX(0x0F766E), EGUI_COLOR_HEX(0xD97706), EGUI_COLOR_HEX(0x0F766E),
                                            EGUI_COLOR_HEX(0xBE5168));
    egui_view_shortcut_recorder_override_static_preview_api(EGUI_VIEW_OF(&recorder_compact), &recorder_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    recorder_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&recorder_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&recorder_compact));

    egui_view_shortcut_recorder_init(EGUI_VIEW_OF(&recorder_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&recorder_read_only), SHORTCUT_PREVIEW_WIDTH, SHORTCUT_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&recorder_read_only), 6, 0, 0, 0);
    egui_view_shortcut_recorder_set_font(EGUI_VIEW_OF(&recorder_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_shortcut_recorder_set_meta_font(EGUI_VIEW_OF(&recorder_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_shortcut_recorder_set_header(EGUI_VIEW_OF(&recorder_read_only), "Read only", "Locked binding", "Locked");
    egui_view_shortcut_recorder_set_compact_mode(EGUI_VIEW_OF(&recorder_read_only), 1);
    egui_view_shortcut_recorder_set_read_only_mode(EGUI_VIEW_OF(&recorder_read_only), 1);
    egui_view_shortcut_recorder_set_binding(EGUI_VIEW_OF(&recorder_read_only), EGUI_KEY_CODE_S, 0, 1);
    egui_view_shortcut_recorder_set_palette(EGUI_VIEW_OF(&recorder_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD7DFE6), EGUI_COLOR_HEX(0x54616D),
                                            EGUI_COLOR_HEX(0x8A97A3), EGUI_COLOR_HEX(0x93A3B4), EGUI_COLOR_HEX(0xD97706), EGUI_COLOR_HEX(0x93A3B4),
                                            EGUI_COLOR_HEX(0xBE5168));
    egui_view_shortcut_recorder_override_static_preview_api(EGUI_VIEW_OF(&recorder_read_only), &recorder_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    recorder_read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&recorder_read_only), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&recorder_read_only));

    apply_primary_scene(0);
    apply_compact_scene(0);

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&recorder_primary));
#endif
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
}

#if EGUI_CONFIG_RECORDING_TEST
bool egui_port_get_recording_action(int action_index, egui_sim_action_t *p_action)
{
    static int last_action = -1;
    int first_call = action_index != last_action;
    int x = 0;
    int y = 0;

    last_action = action_index;

    switch (action_index)
    {
    case 0:
        if (first_call)
        {
            apply_primary_scene(0);
            apply_compact_scene(0);
        }
        EGUI_SIM_SET_WAIT(p_action, SHORTCUT_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SHORTCUT_RECORD_FRAME);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_ENTER, 0, 0);
        }
        EGUI_SIM_SET_WAIT(p_action, SHORTCUT_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SHORTCUT_RECORD_FRAME);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_P, 1, 1);
        }
        EGUI_SIM_SET_WAIT(p_action, SHORTCUT_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SHORTCUT_RECORD_FRAME);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_TAB, 0, 0);
            apply_primary_key(EGUI_KEY_CODE_END, 0, 0);
            apply_primary_key(EGUI_KEY_CODE_ENTER, 0, 0);
        }
        EGUI_SIM_SET_WAIT(p_action, SHORTCUT_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SHORTCUT_RECORD_FRAME);
        return true;
    case 8:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_TAB, 0, 0);
            apply_primary_key(EGUI_KEY_CODE_ENTER, 0, 0);
            apply_compact_scene(1);
        }
        EGUI_SIM_SET_WAIT(p_action, SHORTCUT_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SHORTCUT_RECORD_FRAME);
        return true;
    case 10:
        get_part_center(EGUI_VIEW_OF(&recorder_compact), EGUI_VIEW_SHORTCUT_RECORDER_PART_FIELD, 0, &x, &y);
        p_action->type = EGUI_SIM_ACTION_CLICK;
        p_action->x1 = x;
        p_action->y1 = y;
        p_action->interval_ms = 240;
        return true;
    case 11:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
