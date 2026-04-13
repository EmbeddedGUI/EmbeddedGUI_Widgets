# Pivot 设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 平台语义参考：`WinUI 3 Pivot`
- 次级补充参考：`WPF UI`、`ModernWpf`
- 对应组件名：`Pivot`
- 本次保留状态：`standard`、`compact`、`read only`、`same-target release`、键盘切换、静态 preview
- 本次删除效果：手势翻页动画、惰性加载、自定义 header 模板、嵌套滚动联动、场景化页面 chrome
- EGUI 适配说明：在 custom 层实现轻量 `hcw_pivot`，自绘 header 与 body，复用现有 `tab_strip / flip_view` 的交互经验，不修改 SDK

## 1. 为什么需要这个控件

`Pivot` 用来表达“顶部 header 负责切换分区，主体区域一次只展示一个内容页”的导航语义，适合总览、活动、历史、设置这类平级 section 的轻量切换。它比 `tab_view` 更轻，不承担 close / add / 文档页签管理；又比纯 header 条更完整，因为它需要把当前页内容一起收口。

## 2. 为什么现有控件不够用

- `tab_strip` 只覆盖 header 切换，不承载单页 body 区域。
- `flip_view` 强调顺序翻页，不表达顶部 header 导航语义。
- `tab_view` 语义更重，偏桌面页签容器，不适合轻量 section 切换。
- `selector_bar` 只负责选择入口，本次仍缺少 `Pivot` 的“header + body”一体化结构。

## 3. 目标场景与示例概览

- 主控件展示一个标准 `Pivot`，录制轨道依次切换：
  - `Overview`
  - `Activity`
  - `History`
- 底部左侧保留 `Compact` static preview，对照窄宽度下的 header 与 body 收口。
- 底部右侧保留 `Read only` static preview，对照冻结态的低噪音表现。
- 页面结构统一为：标题 -> 主 `Pivot` -> `Compact / Read only` 双 preview。

目标目录：`example/HelloCustomWidgets/navigation/pivot/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根容器尺寸：`224 x 206`
- 主控件尺寸：`196 x 108`
- 底部对照行尺寸：`216 x 72`
- 单个 preview 面板尺寸：`104 x 72`
- 单个 preview 控件尺寸：`84 x 46`
- 风格约束：
  - 使用浅灰 page panel、白色主 surface、低噪音边框。
  - header 保留轻量 active fill + underline，不回退到重型 tab shell。
  - body 只展示单页摘要卡片，不把 demo 扩展成场景化故事板。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `primary_pivot` | `hcw_pivot_t` | `196 x 108` | `Overview` | 主 `Pivot` |
| `compact_pivot` | `hcw_pivot_t` | `84 x 46` | `Home` | 底部 compact static preview |
| `read_only_pivot` | `hcw_pivot_t` | `84 x 46` | `Audit` | 底部 read only static preview |
| `primary_items` | `const hcw_pivot_item_t[3]` | - | `Overview / Activity / History` | 主控件数据轨道 |
| `compact_items` | `const hcw_pivot_item_t[2]` | - | `Home / Queue` | compact 对照 |
| `read_only_items` | `const hcw_pivot_item_t[2]` | - | `Usage / Audit` | read only 对照 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | 状态 | 说明 |
| --- | --- | --- |
| 主控件 | `Overview` | 默认状态，验证标准 `Pivot` 视觉 |
| 主控件 | `Activity` | 程序化切换第二项 |
| 主控件 | `History` | 程序化切换第三项 |
| `compact` | `Home` | 默认 compact 对照 |
| `compact` | `Queue` | 紧凑宽度下的第二项状态 |
| `read only` | `Audit` | 默认冻结态对照 |

- 主控件保留真实 `touch / key` 切换闭环：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- 键盘覆盖：
  - `Left / Right / Up / Down / Home / End / Tab` 切换索引
  - `Enter / Space` 只 consume，不触发切换
- `read_only_mode`、`disabled` 与静态 preview 都会清掉残留 pressed 状态。
- 底部 preview 通过 `hcw_pivot_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只清 pressed，不改变 `current_index`
  - 不触发 `on_changed`

## 7. 公开 API

| API | 作用 |
| --- | --- |
| `hcw_pivot_init()` | 初始化控件并挂接 `hcw_pivot` API |
| `hcw_pivot_apply_standard_style()` | 应用标准样式 |
| `hcw_pivot_apply_compact_style()` | 应用紧凑样式 |
| `hcw_pivot_apply_read_only_style()` | 应用只读样式 |
| `hcw_pivot_set_items()` | 设置 `header / eyebrow / title / body / meta / tone` 轨道 |
| `hcw_pivot_set_current_index()` | 切换当前页并在必要时回调 `on_changed` |
| `hcw_pivot_get_current_index()` | 读取当前索引 |
| `hcw_pivot_set_on_changed_listener()` | 注册切换回调 |
| `hcw_pivot_set_font()` | 设置 header / title 字体 |
| `hcw_pivot_set_meta_font()` | 设置 eyebrow / meta / body 字体 |
| `hcw_pivot_set_palette()` | 设置 surface、border、text、muted、accent、card surface 调色板 |
| `hcw_pivot_set_compact_mode()` | 切换紧凑模式 |
| `hcw_pivot_set_read_only_mode()` | 切换只读模式 |
| `hcw_pivot_get_header_region()` | 导出 header hit region，供单测验证 |
| `hcw_pivot_override_static_preview_api()` | 把 preview 改成吞输入的静态对照态 |

## 8. `egui_port_get_recording_action()` 录制动作设计

1. 重置主控件、compact preview 与 read only preview 到默认状态
2. 输出默认截图
3. 切换主控件到 `Activity`
4. 输出第二张截图
5. 切换主控件到 `History`
6. 输出第三张截图
7. 切换 `compact` 到 `Queue`
8. 输出最终稳定帧

说明：

- `read_only` preview 在整个录制链路中保持静态冻结态，用来持续对照低噪音只读外观。
- 录制不依赖手势翻页或动画，保证 PC runtime 与 wasm 截图稳定。

## 9. 编译、单测、touch、runtime 与 web 验收路径

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/pivot PORT=pc

make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py

python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/pivot --track reference --timeout 10 --keep-screenshots

python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/pivot
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_pivot
```

## 10. 验收重点

- 主 `Pivot` 与底部两个 preview 必须完整可见，不能裁切或发虚。
- header 选中态必须稳定可辨识，但不能回到高噪音 tab shell。
- body 内容卡片必须跟随当前索引切换，不允许出现空白页或残留页。
- same-target release 语义必须通过，只有回到原 header 才提交。
- `read_only` 与静态 preview 必须吞掉输入并清掉残留 pressed。

## 11. 已知限制与后续方向

- 当前只覆盖少量固定 item，不引入数据源、滚动和溢出折叠。
- 当前 header 宽度仍使用轻量估算，不接入真实文本测量。
- 当前 body 只做摘要卡，不引入复杂内容模板和分页容器。
- 当前先作为 `HelloCustomWidgets` reference widget 维护，是否下沉 SDK 层后续再评估。

## 12. 与现有控件的边界

- 相比 `tab_strip`：这里补齐 body 区域，不只是 header 切换条。
- 相比 `selector_bar`：这里表达单页内容切换，而不只是选择入口。
- 相比 `flip_view`：这里保留顶部 header 导航，不强调翻页手势。
- 相比 `tab_view`：这里不绑定页签壳层，也不处理 close / add。

## 13. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 平台语义参考：`WinUI 3 Pivot`
- 次级补充参考：`WPF UI`、`ModernWpf`

## 14. 对应组件名与本次保留的核心状态

- 对应组件名：`Pivot`
- 本次保留核心状态：
  - `standard`
  - `compact`
  - `read only`
  - `same-target release`
  - `keyboard navigation`
  - `static preview`

## 15. 相比参考原型删除的效果或装饰

- 删除手势翻页动画与惰性加载
- 删除自定义 header 模板、复杂容器嵌套和滚动联动
- 删除额外页面 chrome、guide 文案和场景化叙事层
- 删除与 `Pivot` 主线语义无关的重交互页签管理能力

## 16. EGUI 适配时的简化点与约束

- 自绘 header 与 body，不额外引入重型容器。
- `compact` 与 `read only` 直接复用同一控件实现，不增加额外页面壳层。
- setter 与 guard 都通过统一的 pressed-state 清理逻辑收口。
- `HCW_PIVOT_MAX_ITEMS` 当前固定为 `6`，超出部分会被截断。
