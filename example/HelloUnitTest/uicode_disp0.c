#include <string.h>

#include "egui.h"
#include "uicode_disp0.h"

#include "test/test_accordion.h"
#include "test/test_access_text.h"
#include "test/test_annotated_scroll_bar.h"
#include "test/test_activity_ring.h"
#include "test/test_arc.h"
#include "test/test_animated_icon.h"
#include "test/test_auto_suggest_box.h"
#include "test/test_badge.h"
#include "test/test_badge_group.h"
#include "test/test_bitmap_icon.h"
#include "test/test_border.h"
#include "test/test_breadcrumb_bar.h"
#include "test/test_bullet_decorator.h"
#include "test/test_button.h"
#include "test/test_calendar_date_picker.h"
#include "test/test_calendar_view.h"
#include "test/test_canvas.h"
#include "test/test_dock_panel.h"
#include "test/test_drawer.h"
#include "test/test_list.h"
#include "test/test_card_action.h"
#include "test/test_card_expander.h"
#include "test/test_card_control.h"
#include "test/test_card_panel.h"
#include "test/test_check_box.h"
#include "test/test_color_picker.h"
#include "test/test_combo_box.h"
#include "test/test_compound_button.h"
#include "test/test_content_control.h"
#include "test/test_content_presenter.h"
#include "test/test_headered_content_control.h"
#include "test/test_headered_items_control.h"
#include "test/test_counter_badge.h"
#include "test/test_command_bar.h"
#include "test/test_command_bar_flyout.h"
#include "test/test_data_grid.h"
#include "test/test_data_list_panel.h"
#include "test/test_demo_scaffold.h"
#include "test/test_divider.h"
#include "test/test_ellipse.h"
#include "test/test_date_picker.h"
#include "test/test_dialog_sheet.h"
#include "test/test_drop_down_button.h"
#include "test/test_expander.h"
#include "test/test_field.h"
#include "test/test_flip_view.h"
#include "test/test_flyout.h"
#include "test/test_font_icon.h"
#include "test/test_glyphs.h"
#include "test/test_grid_splitter.h"
#include "test/test_grid.h"
#include "test/test_grid_view.h"
#include "test/test_group_box.h"
#include "test/test_hyperlink_button.h"
#include "test/test_image_control.h"
#include "test/test_image_icon.h"
#include "test/test_info_bar.h"
#include "test/test_info_badge.h"
#include "test/test_info_label.h"
#include "test/test_items_control.h"
#include "test/test_label_control.h"
#include "test/test_line.h"
#include "test/test_master_detail.h"
#include "test/test_menu_bar.h"
#include "test/test_menu_button.h"
#include "test/test_menu_flyout.h"
#include "test/test_message_bar.h"
#include "test/test_nav_panel.h"
#include "test/test_number_box.h"
#include "test/test_parallax_view.h"
#include "test/test_path.h"
#include "test/test_path_icon.h"
#include "test/test_password_box.h"
#include "test/test_radio_button.h"
#include "test/test_radio_buttons.h"
#include "test/test_persona_group.h"
#include "test/test_persona.h"
#include "test/test_person_picture.h"
#include "test/test_polygon.h"
#include "test/test_polyline.h"
#include "test/test_presence_badge.h"
#include "test/test_pips_pager.h"
#include "test/test_pivot.h"
#include "test/test_progress_bar.h"
#include "test/test_rating_control.h"
#include "test/test_relative_panel.h"
#include "test/test_repeat_button.h"
#include "test/test_rectangle.h"
#include "test/test_resize_grip.h"
#include "test/test_rich_edit_box.h"
#include "test/test_rich_text_block.h"
#include "test/test_scroll_bar.h"
#include "test/test_search_box.h"
#include "test/test_scroll_presenter.h"
#include "test/test_scroll_viewer.h"
#include "test/test_selector_bar.h"
#include "test/test_segmented_control.h"
#include "test/test_settings_card.h"
#include "test/test_settings_expander.h"
#include "test/test_settings_panel.h"
#include "test/test_shortcut_recorder.h"
#include "test/test_skeleton.h"
#include "test/test_snackbar.h"
#include "test/test_slider.h"
#include "test/test_spin_button.h"
#include "test/test_spinner.h"
#include "test/test_status_bar.h"
#include "test/test_symbol_icon.h"
#include "test/test_split_button.h"
#include "test/test_split_view.h"
#include "test/test_stack_panel.h"
#include "test/test_swipe_control.h"
#include "test/test_switch.h"
#include "test/test_tag.h"
#include "test/test_tab_strip.h"
#include "test/test_tab_view.h"
#include "test/test_teaching_tip.h"
#include "test/test_tool_tip.h"
#include "test/test_text_box.h"
#include "test/test_text_block.h"
#include "test/test_tick_bar.h"
#include "test/test_thumb_rate.h"
#include "test/test_time_picker.h"
#include "test/test_toolbar.h"
#include "test/test_title_bar.h"
#include "test/test_toast_stack.h"
#include "test/test_toggle_button.h"
#include "test/test_toggle_split_button.h"
#include "test/test_token_input.h"
#include "test/test_tree_view.h"
#include "test/test_two_pane_view.h"
#include "test/test_uniform_grid.h"
#include "test/test_viewbox.h"
#include "test/test_virtualizing_stack_panel.h"
#include "test/test_virtualizing_wrap_panel.h"
#include "test/test_wrap_panel.h"
#include "test/test_uniform_grid.inc"
#include "test/test_viewbox.inc"
#include "test/test_virtualizing_stack_panel.inc"
#include "test/test_virtualizing_wrap_panel.inc"
#include "test/test_wrap_panel.inc"
#include "test/test_items_repeater.h"
#include "test/test_items_repeater.inc"
#include "test/test_grid_view.inc"
#include "test/test_relative_panel.inc"
#include "test/test_rich_edit_box.inc"
#include "test/test_scroll_presenter.inc"
#include "test/test_scroll_viewer.inc"

static egui_core_t *s_core;
static const char *s_test_filter;

static int uicode_is_filter_separator(char ch)
{
    return ch == ',' || ch == ';' || ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

static int uicode_should_run_suite(const char *suite_name)
{
    const char *cursor = s_test_filter;

    if (cursor == NULL || cursor[0] == '\0')
    {
        return 1;
    }

    while (*cursor != '\0')
    {
        const char *token_start;
        const char *token_end;

        while (*cursor != '\0' && uicode_is_filter_separator(*cursor))
        {
            cursor++;
        }
        if (*cursor == '\0')
        {
            break;
        }

        token_start = cursor;
        while (*cursor != '\0' && !uicode_is_filter_separator(*cursor))
        {
            cursor++;
        }
        token_end = cursor;

        if ((size_t)(token_end - token_start) == strlen(suite_name) && strncmp(token_start, suite_name, (size_t)(token_end - token_start)) == 0)
        {
            return 1;
        }
    }

    return 0;
}

void uicode_set_test_filter(const char *filter)
{
    s_test_filter = filter;
}

#define RUN_TEST_SUITE(_name)             \
    do                                    \
    {                                     \
        if (uicode_should_run_suite(#_name)) \
        {                                 \
            test_##_name##_run();         \
        }                                 \
    } while (0)

static void uicode_disp0_init_ui(egui_core_t *core)
{
    EGUI_UNUSED(core);
}

void uicode_disp0_init(egui_core_t *core)
{
    s_core = core;
    uicode_disp0_init_ui(core);
    RUN_TEST_SUITE(accordion);
    RUN_TEST_SUITE(access_text);
    RUN_TEST_SUITE(annotated_scroll_bar);
    RUN_TEST_SUITE(activity_ring);
    RUN_TEST_SUITE(arc);
    RUN_TEST_SUITE(animated_icon);
    RUN_TEST_SUITE(auto_suggest_box);
    RUN_TEST_SUITE(badge);
    RUN_TEST_SUITE(badge_group);
    RUN_TEST_SUITE(bitmap_icon);
    RUN_TEST_SUITE(border);
    RUN_TEST_SUITE(breadcrumb_bar);
    RUN_TEST_SUITE(bullet_decorator);
    RUN_TEST_SUITE(button);
    RUN_TEST_SUITE(calendar_date_picker);
    RUN_TEST_SUITE(calendar_view);
    RUN_TEST_SUITE(canvas);
    RUN_TEST_SUITE(dock_panel);
    RUN_TEST_SUITE(drawer);
    RUN_TEST_SUITE(list);
    RUN_TEST_SUITE(card_action);
    RUN_TEST_SUITE(card_expander);
    RUN_TEST_SUITE(card_control);
    RUN_TEST_SUITE(card_panel);
    RUN_TEST_SUITE(check_box);
    RUN_TEST_SUITE(color_picker);
    RUN_TEST_SUITE(combo_box);
    RUN_TEST_SUITE(compound_button);
    RUN_TEST_SUITE(content_control);
    RUN_TEST_SUITE(content_presenter);
    RUN_TEST_SUITE(headered_content_control);
    RUN_TEST_SUITE(headered_items_control);
    RUN_TEST_SUITE(counter_badge);
    RUN_TEST_SUITE(command_bar);
    RUN_TEST_SUITE(command_bar_flyout);
    RUN_TEST_SUITE(data_grid);
    RUN_TEST_SUITE(data_list_panel);
    RUN_TEST_SUITE(demo_scaffold);
    RUN_TEST_SUITE(divider);
    RUN_TEST_SUITE(ellipse);
    RUN_TEST_SUITE(date_picker);
    RUN_TEST_SUITE(dialog_sheet);
    RUN_TEST_SUITE(drop_down_button);
    RUN_TEST_SUITE(expander);
    RUN_TEST_SUITE(field);
    RUN_TEST_SUITE(flip_view);
    RUN_TEST_SUITE(flyout);
    RUN_TEST_SUITE(font_icon);
    RUN_TEST_SUITE(glyphs);
    RUN_TEST_SUITE(grid_splitter);
    RUN_TEST_SUITE(grid);
    RUN_TEST_SUITE(group_box);
    RUN_TEST_SUITE(hyperlink_button);
    RUN_TEST_SUITE(image_control);
    RUN_TEST_SUITE(image_icon);
    RUN_TEST_SUITE(info_bar);
    RUN_TEST_SUITE(info_badge);
    RUN_TEST_SUITE(info_label);
    RUN_TEST_SUITE(items_control);
    RUN_TEST_SUITE(label_control);
    RUN_TEST_SUITE(line);
    RUN_TEST_SUITE(master_detail);
    RUN_TEST_SUITE(menu_bar);
    RUN_TEST_SUITE(menu_button);
    RUN_TEST_SUITE(menu_flyout);
    RUN_TEST_SUITE(message_bar);
    RUN_TEST_SUITE(nav_panel);
    RUN_TEST_SUITE(number_box);
    RUN_TEST_SUITE(parallax_view);
    RUN_TEST_SUITE(path);
    RUN_TEST_SUITE(path_icon);
    RUN_TEST_SUITE(password_box);
    RUN_TEST_SUITE(radio_button);
    RUN_TEST_SUITE(radio_buttons);
    RUN_TEST_SUITE(persona_group);
    RUN_TEST_SUITE(persona);
    RUN_TEST_SUITE(person_picture);
    RUN_TEST_SUITE(polygon);
    RUN_TEST_SUITE(polyline);
    RUN_TEST_SUITE(presence_badge);
    RUN_TEST_SUITE(pips_pager);
    RUN_TEST_SUITE(pivot);
    RUN_TEST_SUITE(progress_bar);
    RUN_TEST_SUITE(rating_control);
    RUN_TEST_SUITE(relative_panel);
    RUN_TEST_SUITE(repeat_button);
    RUN_TEST_SUITE(rectangle);
    RUN_TEST_SUITE(resize_grip);
    RUN_TEST_SUITE(rich_edit_box);
    RUN_TEST_SUITE(rich_text_block);
    RUN_TEST_SUITE(scroll_bar);
    RUN_TEST_SUITE(search_box);
    RUN_TEST_SUITE(scroll_presenter);
    RUN_TEST_SUITE(scroll_viewer);
    RUN_TEST_SUITE(selector_bar);
    RUN_TEST_SUITE(segmented_control);
    RUN_TEST_SUITE(settings_card);
    RUN_TEST_SUITE(settings_expander);
    RUN_TEST_SUITE(settings_panel);
    RUN_TEST_SUITE(shortcut_recorder);
    RUN_TEST_SUITE(skeleton);
    RUN_TEST_SUITE(snackbar);
    RUN_TEST_SUITE(slider);
    RUN_TEST_SUITE(spin_button);
    RUN_TEST_SUITE(spinner);
    RUN_TEST_SUITE(status_bar);
    RUN_TEST_SUITE(symbol_icon);
    RUN_TEST_SUITE(split_button);
    RUN_TEST_SUITE(split_view);
    RUN_TEST_SUITE(stack_panel);
    RUN_TEST_SUITE(swipe_control);
    RUN_TEST_SUITE(switch);
    RUN_TEST_SUITE(tag);
    RUN_TEST_SUITE(tab_strip);
    RUN_TEST_SUITE(tab_view);
    RUN_TEST_SUITE(teaching_tip);
    RUN_TEST_SUITE(tool_tip);
    RUN_TEST_SUITE(text_box);
    RUN_TEST_SUITE(text_block);
    RUN_TEST_SUITE(tick_bar);
    RUN_TEST_SUITE(thumb_rate);
    RUN_TEST_SUITE(time_picker);
    RUN_TEST_SUITE(toolbar);
    RUN_TEST_SUITE(title_bar);
    RUN_TEST_SUITE(toast_stack);
    RUN_TEST_SUITE(toggle_button);
    RUN_TEST_SUITE(toggle_split_button);
    RUN_TEST_SUITE(token_input);
    RUN_TEST_SUITE(tree_view);
    RUN_TEST_SUITE(two_pane_view);
    RUN_TEST_SUITE(uniform_grid);
    RUN_TEST_SUITE(viewbox);
    RUN_TEST_SUITE(virtualizing_stack_panel);
    RUN_TEST_SUITE(virtualizing_wrap_panel);
    RUN_TEST_SUITE(wrap_panel);
    RUN_TEST_SUITE(items_repeater);
    RUN_TEST_SUITE(grid_view);
}

egui_core_t *uicode_get_core(void)
{
    return s_core;
}
