# pips_pager 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI PipsPager`
- 补充对照控件：`flip_view`、`tab_strip`
- 对应组件名：`PipsPager`
- 当前保留状态：`standard`、`current page`、`previous / next`、`compact`、`read only`
- 当前删除内容：compact preview 第二轨道、preview 点击清 focus 的桥接动作，以及与离散分页本体无关的额外交互录制
- EGUI 适配说明：继续复用仓库内 `pips_pager` 基础实现，本轮只收口 `reference` 页面结构、静态 preview 语义与录制轨道，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`pips_pager` 用来表达离散页码切换、前后翻页和当前位置反馈。它适合 onboarding、轻量轮播和短流程向导这类“页数有限，但位置必须一眼可见”的场景。

## 2. 为什么现有控件不够用
- `tab_strip` 更偏 section 切换，不强调页码位置。
- `flip_view` 以内容卡片翻页为主，不以 pips rail 本体为中心。
- `scroll_bar` 是连续范围值，不适合离散页码。
- 当前仓库仍需要一版贴近 Fluent / WPF UI `PipsPager` 语义的 reference 示例。

## 3. 目标场景与示例概览
- 主控件展示标准 `PipsPager`，录制轨道覆盖 `Onboarding`、`Right`、`End`、`Gallery` 四组状态。
- 底部左侧展示 `compact` 静态 preview，对照小尺寸 rail 的渲染结果。
- 底部右侧展示 `read only` 静态 preview，对照冻结交互后的弱化状态。
- 页面结构固定为：标题 -> 主 `pips_pager` -> `compact / read only` 双 preview。
- 两个 preview 统一通过 `egui_view_pips_pager_override_static_preview_api()` 吞掉 `touch / key`，不再切换页码，也不再承担清主控件 focus 的桥接职责。

目标目录：`example/HelloCustomWidgets/navigation/pips_pager/`

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 198`
- 主控件尺寸：`196 x 92`
- 底部对照行尺寸：`216 x 58`
- `compact` preview：`104 x 58`
- `read only` preview：`104 x 58`

视觉约束：
- 使用浅灰 `page panel`、白色主卡片和低噪音浅边框。
- previous / next 按钮、当前页 pill 和 inactive dots 需要可辨识，但整体不能回到高噪音 showcase 风格。
- `compact` 保留短 rail 语义，不退化成简单文本摘要。
- `read only` 除了弱化 palette，还要作为真正静态 preview，不再承接录制桥接逻辑。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 198` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Pips Pager` | 页面标题 |
| `pager_primary` | `egui_view_pips_pager_t` | `196 x 92` | `Onboarding / 2 of 7` | 主 `PipsPager` |
| `pager_compact` | `egui_view_pips_pager_t` | `104 x 58` | `Compact / static` | 紧凑静态对照 |
| `pager_read_only` | `egui_view_pips_pager_t` | `104 x 58` | `Read only / static` | 只读静态对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Onboarding` | 默认 snapshot，验证标准分页层级 |
| 主控件 | `Right` | 键盘推进一页 |
| 主控件 | `End` | 验证尾页边界 |
| 主控件 | `Gallery` | 程序化切换主 snapshot，验证长页数和短窗口 |
| `compact` | `Compact / 3 of 6` | 紧凑静态 preview |
| `read only` | `Read only / 4 of 7` | 只读静态 preview，禁用交互并弱化 palette |

## 7. 交互语义要求
- 主控件继续保留真实 touch / key 闭环，并遵守 same-target release。
- `PIP` 还要求 `pressed_index` 与释放目标一致，不能在移出后误提交。
- `set_font()`、`set_meta_font()`、`set_title()`、`set_helper()`、`set_palette()`、`set_page_metrics()`、`set_current_index()`、`set_current_part()`、`set_compact_mode()`、`set_read_only_mode()` 都会先清理残留 `pressed_part / pressed_index / is_pressed`。
- `compact_mode`、`read_only_mode`、`!enable` 收到新的 `touch / key` 输入时，会先清掉残留 pressed，再拒绝提交。
- 静态 preview 必须吞掉 `touch / key`，不能修改 `current_index / current_part`，也不能触发 `on_changed`。
- 本轮不修改 SDK，只在 demo、README 和单测层面移除 preview 桥接录制逻辑。

## 8. 本轮收口内容
- 继续维护 `example/HelloCustomWidgets/navigation/pips_pager/test.c`
- 调整底部 `compact` preview 为单一静态轨道，删除 compact preview 第二组状态
- 删除 preview 点击清主控件 focus 的桥接逻辑和对应录制动作
- 录制轨道只保留主控件四组状态变化与最终稳定帧
- 单测同步补齐“静态 preview 输入抑制后仍保持固定页码和 part”的覆盖

## 9. `egui_port_get_recording_action()` 录制动作设计
1. 还原主控件到 `Onboarding`，并同步两个静态 preview 状态。
2. 请求默认截图。
3. 对主控件发送 `Right`，验证向后翻页。
4. 请求第二张截图。
5. 对主控件发送 `End`，验证尾页状态。
6. 请求第三张截图。
7. 程序化切换主控件到 `Gallery`。
8. 请求第四张截图。
9. 恢复默认 `Onboarding` 并再次请求最终稳定帧，确认页面收尾状态一致。

## 10. 编译、交互、runtime 与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/pips_pager PORT=pc
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/pips_pager --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/pips_pager
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_pips_pager
```

验收重点：
- 主控件四张关键截图必须能清晰区分 `Onboarding`、`Right`、`End`、`Gallery`。
- `compact / read only` preview 必须在所有 runtime 帧中保持静态，不得因点击或轨道切换产生额外变化。
- 页面不能出现黑白屏、裁切、pressed 残留或底部 preview 脏态。
- preview 不响应触摸或键盘输入，也不改变 `current_index / current_part`。

## 11. 已知限制
- 当前继续使用固定 snapshot 数据，不接真实业务内容。
- 当前不实现复杂分页动画或外部内容联动。
- 当前优先保证 `reference` 页面、单测和发布链路闭环，不扩展额外场景化包装。
