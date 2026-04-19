# rating_control 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`RatingControl`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主控件保留 touch 评分、拖拽跨星更新、`Clear` 清空和键盘步进评分
- 当前移除内容：页面级 guide、说明面板、preview 轮换、preview 清焦桥接、showcase / HMI 装饰、半星评分和额外 hover 动画

## 1. 为什么需要这个控件
`rating_control` 用于表达 1 到 5 档的离散评分，适合服务评价、速度反馈、安装体验和满意度选择。相比 `radio_button`、`slider`、`number_box` 这类通用输入，星级评分更贴近用户对“满意度档位”的认知。

## 2. 为什么现有控件不够用
- `radio_button` 适合互斥选项，不表达连续评分档位。
- `slider` 更偏连续调节，不适合评分语义和星级反馈。
- `number_box` 偏数值录入，不提供评分控件的图形化认知和快捷交互。
- 当前主线仍需要一版与 `Fluent 2 / WPF UI RatingControl` 语义对齐的 `RatingControl` reference。

## 3. 当前页面结构
- 标题：`Rating Control`
- 主区：1 个可真实交互的 `rating_control`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `3 / 5`
- 右侧 preview：`read only`，固定显示 `4 / 5`

目录：
- `example/HelloCustomWidgets/input/rating_control/`

## 4. 主区 reference 快照
主控件在录制轨道里只程序化切换 3 组 reference 状态，不再录制真实点击和 preview 轮换：

1. 默认态
   `Service quality`
   `Low / High`，值为 `4`
2. 快照 2
   `Delivery speed`
   `Slow / Fast`，值为 `2`
3. 快照 3
   `Setup experience`
   `Hard / Easy`，值为 `5`

底部 preview 在整条轨道中始终固定：
1. `compact`
   使用 `No rating / Poor / Fair / Good / Great / Excellent` 文案，值为 `3`
2. `read only`
   使用同一组文案，值为 `4`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 174`
- 主控件：`196 x 92`
- 底部 preview 行：`216 x 42`
- 单个 preview：`104 x 42`
- 页面结构：标题 -> 主 `rating_control` -> 底部 `compact / read only`
- 风格约束：浅灰 `page panel`、白色评分卡、低噪音边框和暖金色评分 accent，不回退到旧 demo 的说明型 chrome

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Service quality`，值 `4` | `3 / 5` | `4 / 5` |
| 快照 2 | `Delivery speed`，值 `2` | 保持不变 | 保持不变 |
| 快照 3 | `Setup experience`，值 `5` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 保持 `Setup experience`，值 `5` | 保持不变 | 保持不变 |
| 点击星级提交 | 是 | 否 | 否 |
| 拖拽跨星后按最新星位提交 | 是 | 否 | 否 |
| `Clear` 清空 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Delivery speed`
4. 抓取第二组主区快照
5. 切到 `Setup experience`
6. 抓取第三组主区快照
7. 保持最终状态不变并等待稳定
8. 抓取最终稳定帧

说明：
- 主区仍然保留真实 touch 和 keyboard 行为，供运行时手动交互。
- runtime 录制阶段不再执行点击第 5 星、点 `Clear`、`End` 键步进或 compact preview 轮换。
- 底部 preview 统一通过 `egui_view_rating_control_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `RATING_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，最终稳定帧继续走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_rating_control.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 `value / current_part / clear / item_count / value_labels / compact / read_only / disabled / touch / key`，以及拖拽跨星后按最新星位提交的评分控件例外语义。
2. 静态 preview 不变性断言
   通过 `rating_control_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`font`、`meta_font`、`on_changed`、`title`、`low_label`、`high_label`、`value_labels`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`shadow_color`、`label_count`、`item_count`、`current_value`、`current_part`、`compact_mode`、`read_only_mode`、`clear_enabled`、`pressed_part`、`alpha`

同时要求：
- `g_changed_count == 0`
- `g_changed_value == 0xFF`
- `g_changed_part == EGUI_VIEW_RATING_CONTROL_PART_NONE`
- `is_pressed == false`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/rating_control PORT=pc

# 在 X:\ 短路径下执行
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/rating_control --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/rating_control
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_rating_control
```

## 10. 验收重点
- 主区与底部双 preview 必须完整可见，不能黑屏、白屏或被裁切。
- 主区录制只允许出现 `Service quality`、`Delivery speed`、`Setup experience` 3 组可识别状态。
- 主区真实交互仍需保留点击评分、`Clear` 清空、键盘步进和 `DOWN(2) -> MOVE(5) -> UP(5)` 的拖拽提交语义。
- 底部双 preview 必须在全部 runtime 帧里保持静态一致。
- 静态 preview 收到输入后，不能改写 `font / meta_font / on_changed / title / low_label / high_label / value_labels / surface_color / border_color / text_color / muted_text_color / accent_color / shadow_color / label_count / item_count / current_value / current_part / compact_mode / read_only_mode / clear_enabled / pressed_part / alpha / region_screen`。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_rating_control/default`
- 本轮复核结果：
  - 共捕获 `8` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5,6,7]`
  - 主区 RGB 差分边界收敛到 `(54, 146) - (312, 213)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 214` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `radio_button`：这里表达评分档位，不是互斥选项。
- 相比 `slider`：这里是离散评分，不是连续调节。
- 相比 `segmented_control`：这里表达满意度，不是页面切换。
- 相比 `number_box`：这里表达评价等级，不是数值录入。

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `Service quality`
  - `Delivery speed`
  - `Setup experience`
  - `compact`
  - `read only`
- 删减的装饰或桥接：
  - preview 轮换
  - preview 清焦桥接
  - showcase / HMI 装饰
  - 半星评分与额外 hover 动画

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/rating_control PORT=pc`
- `HelloUnitTest`：`PASS`
  - 在 `X:\` 短路径下执行 `make clean APP=HelloUnitTest PORT=pc_test`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 总计 `845 / 845`，其中 `rating_control` suite `37 / 37`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/rating_control --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_rating_control/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/rating_control`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_rating_control`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1327 colors=147`
- 截图复核结论：
  - 共捕获 `8` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1] / [2,3] / [4,5,6,7]`
  - 主区 RGB 差分边界为 `(54, 146) - (312, 213)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 214` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认 `Service quality`、`Delivery speed` 与 `Setup experience` 三组 reference 快照，最终稳定帧保持 `Setup experience`，底部 `compact / read only` preview 全程静态
