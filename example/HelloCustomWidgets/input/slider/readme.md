# slider 自定义控件设计说明
## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`Slider`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主区保留真实 `touch drag` 与 `Left / Right / Up / Down / Home / End / +/-` 键盘步进；底部 `compact / read only` 统一收口为静态 preview
- 当前移除内容：页面级 guide、状态说明文案、录制阶段真实键盘轨道、preview 值轮换与额外收尾帧

## 1. 为什么需要这个控件
`slider` 用来表达“在连续数值范围中拖动选择当前值”的标准输入语义，适合音量、亮度、透明度、缩放比例、灵敏度和阈值调节。仓库虽然已经有 `number_box`、`rating_control` 和 `scroll_bar`，但还缺少一颗与 `Fluent 2 / WPF UI Slider` 语义对齐的 reference。

## 2. 为什么现有控件不够用
- `number_box` 更偏精确离散步进输入，不是连续拖动。
- `rating_control` 是离散等级评分，不适合一般数值范围。
- `scroll_bar` 表达 viewport 位置，不表达抽象数值。
- SDK 自带 `slider` 更偏基础控件验证，仓库仍需要完整的 `input/slider` 页面、单测和 web 验收闭环。

## 3. 当前页面结构
- 标题：`Slider`
- 主区：一个标准 `slider`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `28%`
- 右侧 preview：`read only`，固定显示 `64%`

目录：
- `example/HelloCustomWidgets/input/slider/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，不再在录制阶段发送真实键盘事件：

1. 默认态
   value：`18%`
2. 快照 2
   value：`52%`
3. 快照 3
   value：`86%`

底部 preview 在整条轨道中始终固定：
1. `compact`
   value：`28%`
2. `read only`
   value：`64%`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 128`
- 主控件：`196 x 38`
- 底部 preview 行：`216 x 28`
- 单个 preview：`104 x 28`
- 页面结构：标题 -> 主 `slider` -> 底部 `compact / read only`
- 风格约束：浅色 page panel、低噪音轨道、白色 thumb、轻量 focus ring 设计语言，不回退到 showcase 式说明页

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `18%` | `28%` | `64%` |
| 快照 2 | `52%` | 保持不变 | 保持不变 |
| 快照 3 | `86%` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到默认 `18%` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `52%`
4. 抓取第二组主区快照
5. 切到 `86%`
6. 抓取第三组主区快照
7. 回到默认 `18%`
8. 抓取最终稳定帧

说明：
- 录制阶段不再真实发送 `Right / Plus / End`
- 页面层不再切换 `compact` preview 到第二组值
- 底部 preview 统一通过 `hcw_slider_override_static_preview_api()` 吞掉 `touch / key` 且不改状态
- preview 只负责静态 reference 对照，不再承担页面桥接职责
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `SLIDER_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_slider.c` 当前覆盖两部分：

1. 主控件交互与状态守卫
   覆盖样式 helper、`set_value()` clamp 与清理拖动态、键盘步进、touch drag、touch cancel、`!enable` guard
2. 静态 preview 不变性断言
   通过 `slider_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`on_value_changed`、`value`、`is_dragging`、`track_color`、`active_color`、`thumb_color`、`alpha`、`enable`、`is_focused`、`is_pressed`、`padding`

同时要求：
- `changed_count == 0`
- `changed_value == 0xFF`

验收命令：
```bash
make all APP=HelloCustomWidgets APP_SUB=input/slider PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/slider --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/slider
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_slider
```

## 10. 验收重点
- 主控件和底部 preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `18% / 52% / 86%` 三组 reference 快照必须能从截图中稳定区分。
- 主区继续保留标准 `touch drag` 和键盘步进语义，但录制轨道不再引入额外交互噪音。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_slider/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(110, 185) - (384, 213)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 214` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `number_box`：这里表达连续数值调节，不是离散步进输入。
- 相比 `rating_control`：这里不是离散等级评分。
- 相比 `scroll_bar`：这里的 thumb 尺寸固定，目标是选择值，而不是表达 viewport 比例。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `18%`
  - `52%`
  - `86%`
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - 录制阶段真实 `Right / Plus / End` 键盘轨道
  - 页面层 `compact` preview 值轮换
  - 让 preview 承担页面桥接职责的旧链路
  - 垂直方向、刻度、tooltip 和 range slider 扩展

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/slider PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 本轮按本地 unit 日志复核总计 `845 / 845`，其中 `slider` suite `7 / 7`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/slider --track reference --timeout 10 --keep-screenshots`
  - `9 frames captured -> runtime_check_output/HelloCustomWidgets_input_slider/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/slider`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_slider`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.0977 colors=84`
- 截图复核结论：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界为 `(110, 185) - (384, 213)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 214` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `18%`、`52%` 与 `86%` 三组 reference 快照，最终稳定帧已显式回到默认 `18%`，底部 `compact / read only` preview 全程静态
