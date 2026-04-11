# settings_panel 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`
- 对应组件名：`SettingCard` / `SettingsGroup`
- 本次保留状态：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`
- 本次删除效果：页面级 `guide`、状态回显、外部 preview 标签、旧双列包裹壳、过重的 section row 对比、过重的 value badge 和 footer meta chrome
- EGUI 适配说明：保留设置卡分组、focus row 驱动 tone、尾部 `value / switch / chevron` 语义和 read-only 对照，仅在 `HelloCustomWidgets` 内维护 `reference widget` 版本；`snapshot / compact / read only / disabled` 切换共享同一套 `pressed` 清理语义，底部 preview 统一通过 `egui_view_settings_panel_override_static_preview_api()` 固定为静态 reference

## 1. 为什么需要这个控件？
`settings_panel` 用来表达一组风格统一的设置卡行，适合设置页、偏好面板和系统信息页。它强调分组卡片、focus row、尾部值和开关语义，而不是普通列表或摘要卡片。

## 2. 为什么现有控件不够用？
- `card_panel` 更偏摘要信息卡，不是标准设置面板。
- `data_list_panel` 只有列表语义，没有 Fluent 设置卡的层级和尾部控件节奏。
- `nav_panel` 偏导航入口，不适合承载 `value / switch / chevron` 这类设置行尾部状态。
- `number_box`、`toggle_button` 等输入控件只解决单点交互，不解决整组设置卡的编排。

## 3. 目标场景与示例概览
- 主控件展示标准 `settings_panel`，录制轨道覆盖 `accent / success / warning / neutral` 四组 snapshot。
- 底部左侧展示 `compact` 静态对照，验证小尺寸下 rows、value badge 和 switch 的密度。
- 底部右侧展示 `read only` 静态对照，验证视觉弱化和输入抑制后的被动态。
- 页面结构统一收口为：标题 -> 主 `settings_panel` -> `compact / read only`。
- 两个 preview 都通过 `egui_view_settings_panel_override_static_preview_api()` 吞掉 `touch / key`，点击 preview 时只清主控件 `panel_primary` 的 focus，不再承接页面桥接逻辑。

目标目录：`example/HelloCustomWidgets/layout/settings_panel/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 258`
- 主控件：`196 x 132`
- 底部对照容器：`216 x 84`
- `compact` 预览：`104 x 84`
- `read only` 预览：`104 x 84`
- 视觉原则：
  - 使用浅灰 page panel + 低噪音白底卡片，统一到 Fluent / WPF UI 的浅色基线。
  - focus tone 只在 eyebrow、section rows、footer meta 和尾部控件里保留低强度提示。
  - section rows 必须保持稳定节奏，不能压住 footer，也不能让尾部 `value / switch / chevron` 抢过标题层级。
  - `read only` 预览除了 tone 弱化，还必须抑制输入，避免语义和交互脱节。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 258` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Settings Panel` | 页面标题 |
| `panel_primary` | `egui_view_settings_panel_t` | `196 x 132` | `accent` | 主设置面板 |
| `panel_compact` | `egui_view_settings_panel_t` | `104 x 84` | `accent compact` | 紧凑对照 |
| `panel_read_only` | `egui_view_settings_panel_t` | `104 x 84` | `neutral read only` | 只读对照 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `accent value row` | `accent compact` | `neutral read only` |
| 切换 1 | `success switch row` | 保持 | 保持 |
| 切换 2 | `warning update row` | 保持 | 保持 |
| 切换 3 | `neutral privacy row` | 保持 | 保持 |
| 紧凑切换 | 保持 | `accent -> warning` | 保持 |
| 只读弱化 | 不适用 | 不适用 | tone 降低，同时抑制 touch / key 输入 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 对照，并给主控件请求 focus。
2. 请求第一帧截图。
3. 程序化切到 `success` 主 snapshot。
4. 请求第二帧截图。
5. 程序化切到 `warning` 主 snapshot。
6. 请求第三帧截图。
7. 程序化切到 `neutral` 主 snapshot。
8. 请求第四帧截图。
9. 程序化切到第二组 `compact` snapshot。
10. 请求第五帧截图。
11. 再次给主控件请求 focus。
12. 点击 `compact` preview，只验证 focus 收尾。
13. 请求最终截图并保留收尾等待。

## 8. 编译、交互、runtime、截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/settings_panel PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/settings_panel --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 `compact / read only` 对照必须完整可见，不能被裁切。
- section rows、value badge、switch 和 chevron 在主态与切换态下都要保持可辨识。
- `read only` 需要同时满足视觉弱化和输入抑制，单测必须覆盖 pressed 清理与 touch / key 忽略。
- `snapshot / compact / read only / disabled` 切换后不能残留 `pressed` 高亮或下压位移渲染。
- `read only / disabled` 不仅要忽略后续 touch / key 输入，还要在收到新输入时清理残留 `pressed` 渲染。
- 底部 preview 必须统一通过 `egui_view_settings_panel_override_static_preview_api()` 吞掉 `touch / key`，且不能改变 `snapshot`。
- 点击底部 preview 时只允许清主控件 `panel_primary` 的 focus，最终收尾帧不能出现焦点残留、黑白屏或异常重排。
- runtime 关键帧里，snapshot 切换后的 footer meta、focus row 和 section block 层级必须稳定。
- 页面中不再出现旧列容器壳、guide、状态回显或额外 preview 标签。

## 9. 已知限制与后续方向
- 当前版本仍是固定尺寸 reference 实现，不覆盖超长标题和超过 4 行 settings rows。
- 当前不做真实图标、hover、focus ring 和复杂桌面设置页联动。
- 当前 switch 是示例级绘制语义，不接真实业务状态同步。
- 是否下沉到 `src/widget/` 作为通用控件，后续单独评估。

## 10. 与现有控件的边界
- 相比 `card_panel`：这里不是摘要卡，而是设置行分组。
- 相比 `data_list_panel`：这里不是通用列表，重点在 setting card 节奏和尾部控件语义。
- 相比 `nav_panel`：这里不是导航入口，不强调 rail 或 selected destination。
- 相比单个输入控件：这里关注的是整组设置面板的层级和阅读稳定性。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充对照实现：`ModernWpf`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`SettingCard` / `SettingsGroup`
- 本次保留核心状态：
  - `standard`
  - `compact`
  - `read only`
  - `accent`
  - `success`
  - `warning`
  - `neutral`

## 13. 相比参考原型删掉了哪些效果或装饰？
- 删除页面级 `guide`、状态回显、preview 标签和旧双列包裹壳。
- 删除过重的 section row 对比、value badge chrome 和 footer meta chrome。
- 删除真实图标、Acrylic、长列表滚动和完整桌面设置页联动。
- 删除 hover、focus ring 和复杂展开动效。
- 删除示例外的页面桥接逻辑，只保留单组 `settings_panel` 的核心语义。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot + item` 数据，优先保证 `480 x 480` 下的审阅稳定性。
- `compact` 与 `read only` 直接复用同一控件模式，并通过 `egui_view_settings_panel_override_static_preview_api()` 固定为静态对照，减少页面级桥接逻辑。
- `read only` 不仅弱化视觉，也抑制输入，避免语义和交互脱节。
- 当前先作为 `HelloCustomWidgets` 的 `reference widget` 维护，后续再评估是否下沉框架层。
