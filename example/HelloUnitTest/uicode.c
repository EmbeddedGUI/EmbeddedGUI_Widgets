#include "egui.h"
#include "uicode.h"

#include "test/test_annotated_scroll_bar.h"
#include "test/test_activity_ring.h"
#include "test/test_arc.h"
#include "test/test_animated_icon.h"
#include "test/test_auto_suggest_box.h"
#include "test/test_badge.h"
#include "test/test_badge_group.h"
#include "test/test_bitmap_icon.h"
#include "test/test_breadcrumb_bar.h"
#include "test/test_button.h"
#include "test/test_calendar_view.h"
#include "test/test_canvas.h"
#include "test/test_dock_panel.h"
#include "test/test_card_action.h"
#include "test/test_card_expander.h"
#include "test/test_card_control.h"
#include "test/test_card_panel.h"
#include "test/test_check_box.h"
#include "test/test_color_picker.h"
#include "test/test_combo_box.h"
#include "test/test_counter_badge.h"
#include "test/test_command_bar.h"
#include "test/test_command_bar_flyout.h"
#include "test/test_data_grid.h"
#include "test/test_data_list_panel.h"
#include "test/test_demo_scaffold.h"
#include "test/test_divider.h"
#include "test/test_date_picker.h"
#include "test/test_dialog_sheet.h"
#include "test/test_drop_down_button.h"
#include "test/test_expander.h"
#include "test/test_field.h"
#include "test/test_flip_view.h"
#include "test/test_flyout.h"
#include "test/test_font_icon.h"
#include "test/test_grid_splitter.h"
#include "test/test_grid.h"
#include "test/test_grid_view.h"
#include "test/test_hyperlink_button.h"
#include "test/test_image_icon.h"
#include "test/test_info_badge.h"
#include "test/test_info_label.h"
#include "test/test_master_detail.h"
#include "test/test_menu_bar.h"
#include "test/test_menu_flyout.h"
#include "test/test_message_bar.h"
#include "test/test_nav_panel.h"
#include "test/test_number_box.h"
#include "test/test_parallax_view.h"
#include "test/test_path_icon.h"
#include "test/test_password_box.h"
#include "test/test_radio_button.h"
#include "test/test_radio_buttons.h"
#include "test/test_persona_group.h"
#include "test/test_person_picture.h"
#include "test/test_presence_badge.h"
#include "test/test_pips_pager.h"
#include "test/test_pivot.h"
#include "test/test_progress_bar.h"
#include "test/test_rating_control.h"
#include "test/test_relative_panel.h"
#include "test/test_repeat_button.h"
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
#include "test/test_slider.h"
#include "test/test_spinner.h"
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
#include "test/test_thumb_rate.h"
#include "test/test_time_picker.h"
#include "test/test_title_bar.h"
#include "test/test_toast_stack.h"
#include "test/test_toggle_button.h"
#include "test/test_toggle_split_button.h"
#include "test/test_token_input.h"
#include "test/test_tree_view.h"
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

void uicode_init_ui(void)
{
}

void uicode_create_ui(void)
{
    uicode_init_ui();

    test_annotated_scroll_bar_run();
    test_activity_ring_run();
    test_arc_run();
    test_animated_icon_run();
    test_auto_suggest_box_run();
    test_badge_run();
    test_badge_group_run();
    test_bitmap_icon_run();
    test_breadcrumb_bar_run();
    test_button_run();
    test_calendar_view_run();
    test_canvas_run();
    test_dock_panel_run();
    test_card_action_run();
    test_card_expander_run();
    test_card_control_run();
    test_card_panel_run();
    test_check_box_run();
    test_color_picker_run();
    test_combo_box_run();
    test_counter_badge_run();
    test_command_bar_run();
    test_command_bar_flyout_run();
    test_data_grid_run();
    test_data_list_panel_run();
    test_demo_scaffold_run();
    test_divider_run();
    test_date_picker_run();
    test_dialog_sheet_run();
    test_drop_down_button_run();
    test_expander_run();
    test_field_run();
    test_flip_view_run();
    test_flyout_run();
    test_font_icon_run();
    test_grid_splitter_run();
    test_grid_run();
    test_hyperlink_button_run();
    test_image_icon_run();
    test_info_badge_run();
    test_info_label_run();
    test_master_detail_run();
    test_menu_bar_run();
    test_menu_flyout_run();
    test_message_bar_run();
    test_nav_panel_run();
    test_number_box_run();
    test_parallax_view_run();
    test_path_icon_run();
    test_password_box_run();
    test_radio_button_run();
    test_radio_buttons_run();
    test_persona_group_run();
    test_person_picture_run();
    test_presence_badge_run();
    test_pips_pager_run();
    test_pivot_run();
    test_progress_bar_run();
    test_rating_control_run();
    test_relative_panel_run();
    test_repeat_button_run();
    test_rich_edit_box_run();
    test_rich_text_block_run();
    test_scroll_bar_run();
    test_search_box_run();
    test_scroll_presenter_run();
    test_scroll_viewer_run();
    test_selector_bar_run();
    test_segmented_control_run();
    test_settings_card_run();
    test_settings_expander_run();
    test_settings_panel_run();
    test_shortcut_recorder_run();
    test_skeleton_run();
    test_slider_run();
    test_spinner_run();
    test_symbol_icon_run();
    test_split_button_run();
    test_split_view_run();
    test_stack_panel_run();
    test_swipe_control_run();
    test_switch_run();
    test_tag_run();
    test_tab_strip_run();
    test_tab_view_run();
    test_teaching_tip_run();
    test_tool_tip_run();
    test_text_box_run();
    test_text_block_run();
    test_thumb_rate_run();
    test_time_picker_run();
    test_title_bar_run();
    test_toast_stack_run();
    test_toggle_button_run();
    test_toggle_split_button_run();
    test_token_input_run();
    test_tree_view_run();
    test_uniform_grid_run();
    test_viewbox_run();
    test_virtualizing_stack_panel_run();
    test_virtualizing_wrap_panel_run();
    test_wrap_panel_run();
    test_items_repeater_run();
    test_grid_view_run();
}
