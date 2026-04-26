#include "egui.h"
#include "egui_view_compound_button.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define COMPOUND_BUTTON_ROOT_WIDTH        224
#define COMPOUND_BUTTON_ROOT_HEIGHT       168
#define COMPOUND_BUTTON_PRIMARY_WIDTH     188
#define COMPOUND_BUTTON_PRIMARY_HEIGHT    58
#define COMPOUND_BUTTON_PREVIEW_WIDTH     104
#define COMPOUND_BUTTON_PREVIEW_HEIGHT    58
#define COMPOUND_BUTTON_BOTTOM_ROW_WIDTH  216
#define COMPOUND_BUTTON_BOTTOM_ROW_HEIGHT 58
#define COMPOUND_BUTTON_RECORD_WAIT       90
#define COMPOUND_BUTTON_RECORD_FRAME_WAIT 170
#define COMPOUND_BUTTON_RECORD_FINAL_WAIT 280

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct compound_button_snapshot compound_button_snapshot_t;
struct compound_button_snapshot
{
    const char *title;
    const char *subtitle;
    const char *icon;
    uint8_t style;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_compound_button_t button_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_compound_button_t button_compact;
static egui_view_compound_button_t button_read_only;
static egui_view_api_t button_compact_api;
static egui_view_api_t button_read_only_api;
static uint8_t current_primary_snapshot;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "CompoundButton";
static void layout_page(void);

static const compound_button_snapshot_t primary_snapshots[] = {
        {"Create workspace", "Start a shared review space.", EGUI_ICON_MS_ARROW_FORWARD, EGUI_VIEW_COMPOUND_BUTTON_STYLE_PRIMARY},
        {"Sync changes", "Refresh policy and access data.", EGUI_ICON_MS_SYNC, EGUI_VIEW_COMPOUND_BUTTON_STYLE_DEFAULT},
        {"Approve access", "Confirm reviewers before publish.", EGUI_ICON_MS_DONE, EGUI_VIEW_COMPOUND_BUTTON_STYLE_DEFAULT},
        {"Publish rollout", "Send the release to pilots.", EGUI_ICON_MS_UPLOAD, EGUI_VIEW_COMPOUND_BUTTON_STYLE_SUBTLE},
};

static const compound_button_snapshot_t compact_snapshot = {"Compact", "Short helper text.", EGUI_ICON_MS_ARROW_FORWARD,
                                                            EGUI_VIEW_COMPOUND_BUTTON_STYLE_SUBTLE};
static const compound_button_snapshot_t read_only_snapshot = {"Read only", "Preview remains locked.", EGUI_ICON_MS_DONE,
                                                              EGUI_VIEW_COMPOUND_BUTTON_STYLE_DEFAULT};

static void apply_snapshot_to_button(egui_view_compound_button_t *button, const compound_button_snapshot_t *snapshot)
{
    egui_view_compound_button_set_style(EGUI_VIEW_OF(button), snapshot->style);
    egui_view_compound_button_set_content(EGUI_VIEW_OF(button), snapshot->title, snapshot->subtitle, snapshot->icon);
}

static void apply_primary_snapshot(uint8_t index)
{
    current_primary_snapshot = index % PRIMARY_SNAPSHOT_COUNT;
    apply_snapshot_to_button(&button_primary, &primary_snapshots[current_primary_snapshot]);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(0);
}

static void apply_preview_states(void)
{
    apply_snapshot_to_button(&button_compact, &compact_snapshot);
    egui_view_compound_button_set_compact_mode(EGUI_VIEW_OF(&button_compact), 1);
    apply_snapshot_to_button(&button_read_only, &read_only_snapshot);
    egui_view_compound_button_set_read_only_mode(EGUI_VIEW_OF(&button_read_only), 1);
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
    egui_core_layout_childs_user_root_view(uicode_get_core(), EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

static void focus_primary_button(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&button_primary));
#endif
}

static void on_primary_action(egui_view_t *self)
{
    EGUI_UNUSED(self);
    apply_primary_snapshot((uint8_t)((current_primary_snapshot + 1U) % PRIMARY_SNAPSHOT_COUNT));
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

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), COMPOUND_BUTTON_ROOT_WIDTH, COMPOUND_BUTTON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), COMPOUND_BUTTON_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_compound_button_init(EGUI_VIEW_OF(&button_primary));
    egui_view_set_size(EGUI_VIEW_OF(&button_primary), COMPOUND_BUTTON_PRIMARY_WIDTH, COMPOUND_BUTTON_PRIMARY_HEIGHT);
    egui_view_compound_button_set_fonts(EGUI_VIEW_OF(&button_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4,
                                        (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_FONT_ICON_MS_20);
    egui_view_set_margin(EGUI_VIEW_OF(&button_primary), 0, 0, 0, 10);
    egui_view_compound_button_set_on_action_listener(EGUI_VIEW_OF(&button_primary), on_primary_action);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&button_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), COMPOUND_BUTTON_BOTTOM_ROW_WIDTH, COMPOUND_BUTTON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_compound_button_init(EGUI_VIEW_OF(&button_compact));
    egui_view_set_size(EGUI_VIEW_OF(&button_compact), COMPOUND_BUTTON_PREVIEW_WIDTH, COMPOUND_BUTTON_PREVIEW_HEIGHT);
    egui_view_compound_button_set_fonts(EGUI_VIEW_OF(&button_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4,
                                        (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_FONT_ICON_MS_16);
    egui_view_compound_button_override_static_preview_api(EGUI_VIEW_OF(&button_compact), &button_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&button_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&button_compact));

    egui_view_compound_button_init(EGUI_VIEW_OF(&button_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&button_read_only), COMPOUND_BUTTON_PREVIEW_WIDTH, COMPOUND_BUTTON_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&button_read_only), 8, 0, 0, 0);
    egui_view_compound_button_set_fonts(EGUI_VIEW_OF(&button_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4,
                                        (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_FONT_ICON_MS_16);
    egui_view_compound_button_override_static_preview_api(EGUI_VIEW_OF(&button_read_only), &button_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&button_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&button_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
    focus_primary_button();
}

#if EGUI_CONFIG_RECORDING_TEST
static void dispatch_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    focus_primary_button();
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&button_primary), &event);

    event.type = EGUI_KEY_EVENT_ACTION_UP;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&button_primary), &event);
    if (ui_ready)
    {
        layout_page();
    }
}

static void dispatch_primary_touch_click(void)
{
    egui_motion_event_t event = {0};

    event.location.x = EGUI_VIEW_OF(&button_primary)->region_screen.location.x + EGUI_VIEW_OF(&button_primary)->region_screen.size.width / 2;
    event.location.y = EGUI_VIEW_OF(&button_primary)->region_screen.location.y + EGUI_VIEW_OF(&button_primary)->region_screen.size.height / 2;

    event.type = EGUI_MOTION_EVENT_ACTION_DOWN;
    egui_view_dispatch_touch_event(EGUI_VIEW_OF(&button_primary), &event);

    event.type = EGUI_MOTION_EVENT_ACTION_UP;
    egui_view_dispatch_touch_event(EGUI_VIEW_OF(&button_primary), &event);
    if (ui_ready)
    {
        layout_page();
    }
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
            apply_primary_default_state();
            apply_preview_states();
            focus_primary_button();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMPOUND_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            dispatch_primary_touch_click();
        }
        EGUI_SIM_SET_WAIT(p_action, COMPOUND_BUTTON_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMPOUND_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            dispatch_primary_key(EGUI_KEY_CODE_SPACE);
        }
        EGUI_SIM_SET_WAIT(p_action, COMPOUND_BUTTON_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMPOUND_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            dispatch_primary_key(EGUI_KEY_CODE_ENTER);
        }
        EGUI_SIM_SET_WAIT(p_action, COMPOUND_BUTTON_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMPOUND_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, COMPOUND_BUTTON_RECORD_FINAL_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMPOUND_BUTTON_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
