# ContentPresenter

`ContentPresenter` 是 WPF / WinUI 控件模板体系中的内容呈现槽，用来把一个 content 或 child 呈现在模板指定位置。它比 `ContentControl` 更轻，不表达完整控件外观，也不承担业务卡片、边框容器或多 child 布局职责。

## 1. 为什么需要这个控件

复杂控件在模板内部经常需要一个稳定的内容出口：外层控件负责状态、边框和交互，`ContentPresenter` 只负责把传入内容呈现到模板槽位。保留它可以让后续 layout / input 控件复用一致的内容呈现语义。

## 2. 为什么现有控件不够用

`ContentControl` 是拥有内容的控件宿主；`Border` 是可见边界容器；`Grid`、`StackPanel`、`Canvas` 是布局容器。它们都不能直接表达“模板里显示 content 的轻量 presenter”。

## 3. 目标场景与示例概览

本示例保留四组主区状态：

- `Template / center content`
- `Presenter / top left`
- `Compact / top left`
- `Read only / muted slot`

底部保留 `compact` 与 `read only` 两个静态 preview，验证 preview 消费输入但不改变状态。

## 4. 视觉与布局规格

Presenter 使用浅色 slot surface、低对比 guide 线和底部细 accent marker。默认尺寸为 `168 x 94`，主区 child 使用 `120 x 24` 文本内容。compact 模式收紧 padding 与圆角，read only 模式降低 guide、accent 和文字对比。

## 5. 控件清单与状态矩阵

- `egui_view_content_presenter_t`
- 单一 `child`
- `surface_color / guide_color / accent_color`
- `corner_radius / guide_width`
- `content_align_type`
- `compact_mode`
- `read_only_mode`

## 6. 录制动作设计

录制轨道只切换主区快照：默认 template slot、top-left presenter、compact、read only，并最终回到默认 template slot。底部 preview 全程静态，用于观察 compact / read only reference 对照。

## 7. 编译 / runtime / 截图验收标准

- `make all APP=HelloCustomWidgets APP_SUB=layout/content_presenter PORT=pc`
- `make all APP=HelloUnitTest PORT=pc_test`
- `output\main.exe content_presenter`
- `python scripts/checks/check_touch_release_semantics.py --scope custom --category layout`
- `python scripts/checks/check_docs_encoding.py`
- `python scripts/checks/check_widget_catalog.py`
- `python scripts/sync_widget_catalog.py --check`
- `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/content_presenter --track reference --timeout 10 --keep-screenshots`
- `python scripts/code_compile_check.py --custom-widgets --category layout --bits64`
- `python scripts/code_runtime_check.py --app HelloCustomWidgets --category layout --track reference --bits64`
- `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub layout/content_presenter`
- `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_layout_content_presenter`

截图必须确认主区四态可见、底部 preview 静态、没有黑屏 / 白屏 / 主体缺失 / 文本重叠 / 裁切。

## 8. 参考设计体系与开源母本

参考 WPF / WinUI `ContentPresenter` 的核心语义：呈现 `Content`、支持内容对齐、作为控件模板里的轻量内容槽。视觉语言收口到 Fluent 2 的浅色 surface、低噪声 guide 和克制 accent。

## 9. catalog 元数据

- `reference_system`: `Fluent 2 / WPF UI`
- `reference_library`: `WPF / WinUI`
- `reference_component`: `ContentPresenter`

## 10. 保留与删除

保留单 content 呈现、内容对齐、padding、compact、read only 和静态 preview。删除模板选择、数据绑定、动画、访问键解析、复杂内容模板和多 child panel 行为。

## 11. EGUI 适配简化点

当前实现只在 custom widget 层提供轻量 `egui_view_group_t` 派生版本，不下沉到 SDK。内容模板简化为一个 child 指针；对齐通过 `content_align_type` 映射到现有 `EGUI_ALIGN_*`；静态 preview 通过覆盖 API 消费 touch / key 并保持状态。
