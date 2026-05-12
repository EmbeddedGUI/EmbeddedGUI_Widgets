#include "egui.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"
#include "showcase_generated.h"

#if EGUI_CONFIG_FUNCTION_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define HCW_SHOWCASE_HEADER_HEIGHT 56
#define HCW_SHOWCASE_NAV_HEIGHT    42
#define HCW_SHOWCASE_PAGE_TOP      (HCW_SHOWCASE_HEADER_HEIGHT + HCW_SHOWCASE_NAV_HEIGHT)
#define HCW_SHOWCASE_PAGE_HEIGHT   (EGUI_CONFIG_SCREEN_HEIGHT - HCW_SHOWCASE_PAGE_TOP)
#define HCW_SHOWCASE_COLS          1
#define HCW_SHOWCASE_CARD_WIDTH    226
#define HCW_SHOWCASE_ROW_GAP       14
#define HCW_SHOWCASE_SIDE_PADDING  10
#define HCW_SHOWCASE_NAV_GAP           6
#define HCW_SHOWCASE_BASIC_CHILD_COUNT 2

static egui_core_t *s_core;
static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_label_t subtitle_label;
static egui_view_linearlayout_t nav_row;
static egui_view_button_t nav_buttons[5];
static egui_view_scroll_t category_scrolls[5];
static egui_view_linearlayout_t category_contents[5];
static egui_view_t demo_capture_host;
static egui_view_t *s_captured_demo;
static uint8_t s_capture_enabled;
static uint8_t s_active_category;

EGUI_BACKGROUND_COLOR_PARAM_INIT_SOLID(bg_page_param, HCW_COLOR_PAGE_BG, EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT(bg_page_params, &bg_page_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page, &bg_page_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_nav_normal_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_nav_normal_params, &bg_nav_normal_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_nav_normal, &bg_nav_normal_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_nav_active_param, HCW_COLOR_PRIMARY, EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_nav_active_params, &bg_nav_active_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_nav_active, &bg_nav_active_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(bg_missing_param, EGUI_COLOR_HEX(0xFFF7ED), EGUI_ALPHA_100, 12, 1, EGUI_COLOR_HEX(0xFDBA74),
                                                        EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT(bg_missing_params, &bg_missing_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_missing, &bg_missing_params);

void hcw_showcase_capture_user_root_view(egui_view_t *view)
{
    if (!s_capture_enabled || view == NULL)
    {
        return;
    }

    if (s_captured_demo == NULL)
    {
        s_captured_demo = view;
    }
}

static void reset_view_spacing(egui_view_t *view)
{
    if (view == NULL)
    {
        return;
    }

    view->margin.left = 0;
    view->margin.right = 0;
    view->margin.top = 0;
    view->margin.bottom = 0;
}

static uint8_t is_group_view(egui_view_t *view)
{
    return view != NULL && view->api != NULL && view->api->draw == egui_view_group_draw;
}

static egui_dim_t measure_visible_vertical_extent(egui_view_t *view)
{
    egui_dnode_t *node;
    egui_dim_t height;

    if (!is_group_view(view))
    {
        return 0;
    }

    height = view->padding.top + view->padding.bottom;
    EGUI_DLIST_FOR_EACH_NODE(&EGUI_CAST_TO(egui_view_group_t, view)->childs, node)
    {
        egui_view_t *child = EGUI_DLIST_ENTRY(node, egui_view_t, node);

        if (child->is_gone)
        {
            continue;
        }

        height += child->margin.top + child->region.size.height + child->margin.bottom;
    }

    return height;
}

static void keep_basic_demo_state(egui_view_t *view)
{
    egui_dnode_t *node;
    uint8_t visible_child_count = 0;
    uint8_t hidden_child_count = 0;

    if (!is_group_view(view))
    {
        return;
    }

    EGUI_DLIST_FOR_EACH_NODE(&EGUI_CAST_TO(egui_view_group_t, view)->childs, node)
    {
        egui_view_t *child = EGUI_DLIST_ENTRY(node, egui_view_t, node);

        if (child->is_gone)
        {
            continue;
        }

        if (visible_child_count >= HCW_SHOWCASE_BASIC_CHILD_COUNT)
        {
            egui_view_set_gone(child, 1);
            ++hidden_child_count;
        }
        ++visible_child_count;
    }

    if (hidden_child_count > 0)
    {
        egui_dim_t height = measure_visible_vertical_extent(view);

        if (height > 0)
        {
            egui_view_set_size(view, view->region.size.width, height);
        }
        egui_view_group_layout_childs(view, 0, 0, 0, EGUI_ALIGN_HCENTER);
    }
}

static egui_view_t *create_demo_view(const hcw_showcase_demo_entry_t *entry)
{
    if (entry == NULL || entry->init == NULL)
    {
        return NULL;
    }

    s_captured_demo = NULL;
    s_capture_enabled = 1;
    entry->init();
    s_capture_enabled = 0;

    if (s_captured_demo != NULL)
    {
        keep_basic_demo_state(s_captured_demo);
        reset_view_spacing(s_captured_demo);
    }
    return s_captured_demo;
}

static void init_missing_demo_card(egui_view_label_t *label, const char *title)
{
    egui_view_label_init(EGUI_VIEW_OF(label), s_core);
    egui_view_set_size(EGUI_VIEW_OF(label), HCW_SHOWCASE_CARD_WIDTH, 70);
    egui_view_label_set_text(EGUI_VIEW_OF(label), title);
    egui_view_label_set_font(EGUI_VIEW_OF(label), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), EGUI_COLOR_HEX(0x9A3412), EGUI_ALPHA_100);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), EGUI_ALIGN_CENTER);
    egui_view_set_background(EGUI_VIEW_OF(label), EGUI_BG_OF(&bg_missing));
}

static egui_dim_t get_category_row_width(uint16_t first_index, uint16_t demo_count, const hcw_showcase_demo_entry_t *entries, egui_view_t **views)
{
    uint8_t col;
    egui_dim_t width = 0;

    for (col = 0; col < HCW_SHOWCASE_COLS; ++col)
    {
        uint16_t index = first_index + col;
        egui_view_t *view;

        if (index >= demo_count)
        {
            break;
        }

        view = views[index];
        if (view == NULL)
        {
            EGUI_UNUSED(entries);
            width += HCW_SHOWCASE_CARD_WIDTH;
        }
        else
        {
            width += view->region.size.width;
        }

        if (col + 1 < HCW_SHOWCASE_COLS && index + 1 < demo_count)
        {
            width += HCW_SHOWCASE_ROW_GAP;
        }
    }

    return width;
}

static egui_dim_t get_category_row_height(uint16_t first_index, uint16_t demo_count, egui_view_t **views)
{
    uint8_t col;
    egui_dim_t height = 0;

    for (col = 0; col < HCW_SHOWCASE_COLS; ++col)
    {
        uint16_t index = first_index + col;
        egui_view_t *view;

        if (index >= demo_count)
        {
            break;
        }

        view = views[index];
        if (view == NULL)
        {
            height = EGUI_MAX(height, 70);
        }
        else
        {
            height = EGUI_MAX(height, view->region.size.height);
        }
    }

    return height;
}

static void layout_category_content(uint8_t category_index, egui_view_t **views)
{
    const hcw_showcase_category_t *category = &hcw_showcase_categories[category_index];
    egui_view_t *content = EGUI_VIEW_OF(&category_contents[category_index]);
    uint16_t index = 0;
    egui_dim_t y = HCW_SHOWCASE_SIDE_PADDING;
    egui_dim_t max_width = 0;

    while (index < category->demo_count)
    {
        egui_dim_t row_width = get_category_row_width(index, category->demo_count, category->demos, views);
        egui_dim_t row_height = get_category_row_height(index, category->demo_count, views);
        egui_dim_t x = (EGUI_CONFIG_SCREEN_WIDTH - row_width) / 2;
        uint8_t col;

        if (x < HCW_SHOWCASE_SIDE_PADDING)
        {
            x = HCW_SHOWCASE_SIDE_PADDING;
        }

        for (col = 0; col < HCW_SHOWCASE_COLS && index < category->demo_count; ++col, ++index)
        {
            egui_view_t *view = views[index];
            egui_dim_t child_y = y;

            if (view == NULL)
            {
                x += HCW_SHOWCASE_CARD_WIDTH + HCW_SHOWCASE_ROW_GAP;
                continue;
            }

            if (row_height > view->region.size.height)
            {
                child_y += (row_height - view->region.size.height) / 2;
            }
            egui_view_set_position(view, x, child_y);
            x += view->region.size.width + HCW_SHOWCASE_ROW_GAP;
        }

        max_width = EGUI_MAX(max_width, row_width);
        y += row_height + HCW_SHOWCASE_ROW_GAP;
    }

    egui_view_set_size(content, EGUI_MAX((egui_dim_t)EGUI_CONFIG_SCREEN_WIDTH, (egui_dim_t)(max_width + HCW_SHOWCASE_SIDE_PADDING * 2)), y);
}

static void init_category_page(uint8_t category_index)
{
    const hcw_showcase_category_t *category = &hcw_showcase_categories[category_index];
    egui_view_t *content = EGUI_VIEW_OF(&category_contents[category_index]);
    static egui_view_label_t missing_labels[141];
    static uint16_t missing_label_count;
    egui_view_t *views[64] = {NULL};
    uint16_t i;

    egui_view_scroll_init(EGUI_VIEW_OF(&category_scrolls[category_index]), s_core);
    egui_view_set_position(EGUI_VIEW_OF(&category_scrolls[category_index]), 0, HCW_SHOWCASE_PAGE_TOP);
    egui_view_scroll_set_size(EGUI_VIEW_OF(&category_scrolls[category_index]), EGUI_CONFIG_SCREEN_WIDTH, HCW_SHOWCASE_PAGE_HEIGHT);
    egui_view_scroll_set_scrollbar_enabled(EGUI_VIEW_OF(&category_scrolls[category_index]), 1);
    egui_view_set_background(EGUI_VIEW_OF(&category_scrolls[category_index]), EGUI_BG_OF(&bg_page));

    egui_view_linearlayout_init(content, s_core);
    egui_view_linearlayout_set_auto_width(content, 0);
    egui_view_linearlayout_set_auto_height(content, 0);
    egui_view_linearlayout_set_orientation(content, 0);

    for (i = 0; i < category->demo_count && i < EGUI_ARRAY_SIZE(views); ++i)
    {
        egui_view_t *view = create_demo_view(&category->demos[i]);
        if (view == NULL && missing_label_count < EGUI_ARRAY_SIZE(missing_labels))
        {
            init_missing_demo_card(&missing_labels[missing_label_count], category->demos[i].title);
            view = EGUI_VIEW_OF(&missing_labels[missing_label_count]);
            ++missing_label_count;
        }

        views[i] = view;
        if (view != NULL)
        {
            egui_view_group_add_child(content, view);
        }
    }

    layout_category_content(category_index, views);
    egui_view_scroll_add_child(EGUI_VIEW_OF(&category_scrolls[category_index]), content);
    egui_view_scroll_layout_childs(EGUI_VIEW_OF(&category_scrolls[category_index]));
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&category_scrolls[category_index]));
}

static void update_nav_visuals(void)
{
    uint8_t i;

    for (i = 0; i < hcw_showcase_category_count; ++i)
    {
        const uint8_t is_active = i == s_active_category;
        egui_view_t *button = EGUI_VIEW_OF(&nav_buttons[i]);

        egui_view_set_background(button, EGUI_BG_OF(is_active ? &bg_nav_active : &bg_nav_normal));
        egui_view_label_set_font_color(button, is_active ? EGUI_COLOR_WHITE : EGUI_COLOR_HEX(0x334155), EGUI_ALPHA_100);
        egui_view_set_gone(EGUI_VIEW_OF(&category_scrolls[i]), !is_active);
    }
}

static void select_category(uint8_t category_index)
{
    if (category_index >= hcw_showcase_category_count)
    {
        return;
    }

    s_active_category = category_index;
    update_nav_visuals();
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
}

static void on_nav_input(egui_view_t *self)
{
    EGUI_UNUSED(self);
    select_category(0);
}

static void on_nav_layout(egui_view_t *self)
{
    EGUI_UNUSED(self);
    select_category(1);
}

static void on_nav_navigation(egui_view_t *self)
{
    EGUI_UNUSED(self);
    select_category(2);
}

static void on_nav_display(egui_view_t *self)
{
    EGUI_UNUSED(self);
    select_category(3);
}

static void on_nav_feedback(egui_view_t *self)
{
    EGUI_UNUSED(self);
    select_category(4);
}

static egui_view_on_click_listener_t get_nav_listener(uint8_t index)
{
    static const egui_view_on_click_listener_t listeners[] = {
            on_nav_input, on_nav_layout, on_nav_navigation, on_nav_display, on_nav_feedback,
    };

    return listeners[index];
}

static void init_header(void)
{
    static char subtitle_text[64];

    egui_view_label_init(EGUI_VIEW_OF(&title_label), s_core);
    egui_view_set_position(EGUI_VIEW_OF(&title_label), 0, 8);
    egui_view_set_size(EGUI_VIEW_OF(&title_label), EGUI_CONFIG_SCREEN_WIDTH, 24);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), "HelloCustomWidgets Showcase");
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_16_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x0F172A), EGUI_ALPHA_100);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    snprintf(subtitle_text, sizeof(subtitle_text), "%u widgets / %u categories", (unsigned)hcw_showcase_demo_count, (unsigned)hcw_showcase_category_count);
    egui_view_label_init(EGUI_VIEW_OF(&subtitle_label), s_core);
    egui_view_set_position(EGUI_VIEW_OF(&subtitle_label), 0, 34);
    egui_view_set_size(EGUI_VIEW_OF(&subtitle_label), EGUI_CONFIG_SCREEN_WIDTH, 14);
    egui_view_label_set_text(EGUI_VIEW_OF(&subtitle_label), subtitle_text);
    egui_view_label_set_font(EGUI_VIEW_OF(&subtitle_label), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&subtitle_label), EGUI_COLOR_HEX(0x64748B), EGUI_ALPHA_100);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&subtitle_label), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&subtitle_label));
}

static void init_nav(void)
{
    uint8_t i;
    egui_dim_t nav_width =
            (EGUI_CONFIG_SCREEN_WIDTH - HCW_SHOWCASE_SIDE_PADDING * 2 - HCW_SHOWCASE_NAV_GAP * (hcw_showcase_category_count - 1)) / hcw_showcase_category_count;
    egui_dim_t x = HCW_SHOWCASE_SIDE_PADDING;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&nav_row), s_core);
    egui_view_set_position(EGUI_VIEW_OF(&nav_row), 0, HCW_SHOWCASE_HEADER_HEIGHT);
    egui_view_set_size(EGUI_VIEW_OF(&nav_row), EGUI_CONFIG_SCREEN_WIDTH, HCW_SHOWCASE_NAV_HEIGHT);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&nav_row));

    for (i = 0; i < hcw_showcase_category_count; ++i)
    {
        egui_view_t *button = EGUI_VIEW_OF(&nav_buttons[i]);
        egui_view_button_init(button, s_core);
        egui_view_set_position(button, x, 4);
        egui_view_set_size(button, nav_width, 30);
        egui_view_label_set_text(button, hcw_showcase_categories[i].title);
        egui_view_label_set_font(button, (const egui_font_t *)&egui_res_font_montserrat_10_4);
        egui_view_label_set_align_type(button, EGUI_ALIGN_CENTER);
        egui_view_set_on_click_listener(button, get_nav_listener(i));
        egui_view_group_add_child(EGUI_VIEW_OF(&nav_row), button);
        x += nav_width + HCW_SHOWCASE_NAV_GAP;
    }
}

void test_init_ui(void)
{
    uint8_t category_index;

    s_core = uicode_get_core();
    s_active_category = 0;

    egui_view_init(&demo_capture_host, s_core);

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), s_core);
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), EGUI_CONFIG_SCREEN_WIDTH, EGUI_CONFIG_SCREEN_HEIGHT);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page));

    init_header();
    init_nav();

    for (category_index = 0; category_index < hcw_showcase_category_count; ++category_index)
    {
        init_category_page(category_index);
    }

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    update_nav_visuals();
}

#if EGUI_CONFIG_FUNCTION_RECORDING_TEST
bool egui_port_get_recording_action(int action_index, egui_sim_action_t *p_action)
{
    static int last_action = -1;
    int first_call = action_index != last_action;

    last_action = action_index;

    if (first_call && action_index >= 0 && action_index < hcw_showcase_category_count)
    {
        select_category((uint8_t)action_index);
        recording_request_snapshot();
    }

    switch (action_index)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        EGUI_SIM_SET_WAIT(p_action, 220);
        return true;
    default:
        return false;
    }
}
#endif
