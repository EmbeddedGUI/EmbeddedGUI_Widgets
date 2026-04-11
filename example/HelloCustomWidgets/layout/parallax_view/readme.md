# parallax_view 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 ParallaxView`
- 对应组件名：`ParallaxView`
- 本次保留状态：`standard`、`compact`、`read only`、`offset stepped`、`hero shift`、`active row`
- 本次删除效果：页面级 `guide`、状态回显、外部 preview 标签、旧双列包裹壳、真实图片背景、复杂滚动动画和过重的 hero / row / footer chrome
- EGUI 适配说明：保留 offset 驱动 hero 慢速位移、active row 和 footer 摘要语义，仅在 `HelloCustomWidgets` 内维护 `reference widget` 版本；底部 preview 统一通过 `egui_view_parallax_view_override_static_preview_api()` 固定为静态 reference

## 1. 为什么需要这个控件？
`parallax_view` 用来表达“前景内容滚动时，背景 hero 区域以更慢速度位移”的标准景深语义。它适合 onboarding、内容导览、长页面摘要和 dashboard hero 等场景，重点不是列表本身，而是 offset 与背景层位移的关联。

## 2. 为什么现有控件不够用？
- `split_view`、`master_detail` 强调双栏结构，不表达单卡内的景深滚动。
- `flip_view` 强调分页切换，不表达连续 offset。
- `scroll_bar` 只表达滚动位置，不承担 hero depth 的视觉反馈。
- `card_panel` 更偏静态摘要卡，不处理前景 rows 与背景 hero 的联动。

## 3. 目标场景与示例概览
- 主控件展示标准 `parallax_view`，录制轨道覆盖四组锚点状态：`Hero Banner / Pinned Deck / Quiet Layer / System Cards`。
- 底部左侧展示 `compact` 静态对照，验证低密度 hero 与 rows 的压缩表达。
- 底部右侧展示 `read only` 静态对照，验证景深弱化和输入抑制后的被动态。
- 页面结构统一收口为：标题 -> 主 `parallax_view` -> `compact / read only`。
- 两个 preview 都通过 `egui_view_parallax_view_override_static_preview_api()` 吞掉 `touch / key`，点击 preview 时只清主控件 `parallax_primary` 的 focus，不再承接页面桥接逻辑。

目标目录：`example/HelloCustomWidgets/layout/parallax_view/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 304`
- 主控件：`194 x 136`
- 底部对照容器：`218 x 82`
- `compact` 预览：`106 x 82`
- `read only` 预览：`106 x 82`
- 视觉原则：
  - 使用浅色 Fluent 卡片，不回退到 showcase 式的重装饰 hero。
  - hero 区域保留低对比三层 background strips，通过 offset 和 vertical shift 表达景深。
  - 前景 rows 以低噪音列表卡呈现，active row 只保留轻量 tone bar 和弱化边框强调。
  - footer summary 保持可读，但不能压过 hero 标题和 rows。
  - `read only` 除了视觉弱化，还必须抑制 touch / key 输入。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 304` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Parallax View` | 页面标题 |
| `parallax_primary` | `egui_view_parallax_view_t` | `194 x 136` | `offset=0` | 主 parallax 卡 |
| `parallax_compact` | `egui_view_parallax_view_t` | `106 x 82` | `compact` | 紧凑对照 |
| `parallax_read_only` | `egui_view_parallax_view_t` | `106 x 82` | `compact + read only` | 只读对照 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `Hero Banner` | `Depth Strip` | `Review Shelf` |
| 切换 1 | `Pinned Deck` | 保持 | 保持 |
| 切换 2 | `Quiet Layer` | 保持 | 保持 |
| 切换 3 | `System Cards` | 保持 | 保持 |
| 紧凑切换 | 保持 | `Depth Strip -> Quiet Stack` | 保持 |
| 只读弱化 | 不适用 | 不适用 | hero、rows、footer tone 降低，同时抑制输入 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 对照，并给主控件请求 focus。
2. 请求第一帧截图。
3. 程序化切到 `Pinned Deck`。
4. 请求第二帧截图。
5. 程序化切到 `Quiet Layer`。
6. 请求第三帧截图。
7. 程序化切到 `Quiet Stack` 的 `compact` 对照。
8. 请求第四帧截图。
9. 程序化切到主控件尾态 `System Cards`。
10. 请求第五帧截图。
11. 再次给主控件请求 focus。
12. 点击 `compact` preview，只验证 focus 收尾。
13. 请求最终截图并保留收尾等待。

## 8. 编译、交互、runtime、截图验收标准
```bash
make all APP=HelloCustomWidgets APP_SUB=layout/parallax_view PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category layout
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/parallax_view --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 `compact / read only` 对照必须完整可见，不能被裁切。
- hero layers 的位移变化必须能从关键帧中稳定辨认出来，且不能压住 rows 文本。
- active row、progress pill 和 footer summary 在主态与切换态下都要保持可辨识。
- `read only` 需要同时满足视觉弱化和输入抑制，单测必须覆盖 pressed 清理与 touch / key 忽略。
- 底部 preview 必须统一通过 `egui_view_parallax_view_override_static_preview_api()` 吞掉 `touch / key`，且不能改变 `offset / active row`。
- 点击底部 preview 时只允许清主控件 `parallax_primary` 的 focus，最终收尾帧不能出现焦点残留、黑白屏或异常重排。
- 页面中不再出现旧列容器壳、guide、状态回显或额外 preview 标签。

## 9. 已知限制与后续方向
- 当前版本使用固定 `row + anchor_offset` 数据，不接真实滚动容器。
- 当前只做 stepped offset 的示例级 parallax，不做连续惯性动画。
- 当前 hero 层是抽象 strips，不引入真实图片或视频资源。
- 是否下沉到 `src/widget/` 作为通用控件，后续单独评估。

## 10. 与现有控件的边界
- 相比 `split_view` / `master_detail`：这里是单卡内部的景深滚动，不是双栏布局。
- 相比 `flip_view`：这里是连续 offset 语义，不是分页翻页。
- 相比 `scroll_bar`：这里重点是 hero depth 反馈，不是标准滚动条输入。
- 相比 `card_panel`：这里强调 offset 驱动的层位移，而不是静态摘要卡。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 官方语义参考：`WinUI 3 ParallaxView`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`ParallaxView`
- 本次保留核心状态：
  - `standard`
  - `compact`
  - `read only`
  - `offset stepped`
  - `hero shift`
  - `active row`

## 13. 相比参考原型删掉了哪些效果或装饰？
- 删除页面级 `guide`、状态回显、preview 标签和旧双列包裹壳。
- 删除真实图片 / 视频背景、复杂桌面级滚动动画和材质阴影。
- 删除跨容器联动和多源滚动绑定。
- 删除完整 hover、focus ring 等桌面交互细节。
- 删除重装饰 showcase 风格，只保留 reference 级景深语义。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `row + anchor_offset` 数据，优先保证 `480 x 480` 下的审阅稳定性。
- 通过 hero strips 位移模拟 depth，不引入额外资源依赖。
- `compact` 与 `read only` 直接复用同一控件模式，并通过 `egui_view_parallax_view_override_static_preview_api()` 固定为静态对照，减少页面级桥接逻辑。
- `read only` 不仅弱化视觉，也抑制输入，避免语义和交互脱节。
