# text_box 自定义控件设计说明
## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`TextBox`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主区保留标准单行文本输入、光标与键盘提交语义；底部 `compact / read only` 统一收口为静态 preview 对照
- 当前移除内容：页面级 guide、preview 清焦桥接、录制阶段真实键盘轨道、preview 文本同步、showcase 式场景化装饰

## 1. 为什么需要这个控件
`text_box` 用于承载最基础的单行文本输入，例如显示名称、标签、备注标题和轻量配置字段。主线仓库需要一版与 `Fluent 2 / WPF UI TextBox` 语义对齐的 `TextBox` reference，用来补齐最小文本输入能力。

## 2. 为什么现有控件不够用
- `password_box` 面向密文字段，保留遮罩与 reveal 语义，不适合作为普通文本输入。
- `number_box` 面向离散数值输入，不承担自由文本编辑。
- `search_box` 带固定搜索图标和 clear affordance，语义比普通文本框更具体。
- SDK 虽然已有基础 `textinput`，但当前仓库仍需要完整的 `input/text_box` 包装页、单测和 web 验收闭环。

## 3. 当前页面结构
- 标题：`Text Box`
- 主区：一个标准 `text_box`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Queued`
- 右侧 preview：`read only`，固定显示 `Managed`

目录：
- `example/HelloCustomWidgets/input/text_box/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，不再在录制阶段发送真实键盘事件，也不再依赖 preview 帮主区清焦：

1. 默认态
   placeholder：`Display name`
   文本：`Node 01`
2. 快照 2
   placeholder：`Display name`
   文本：`Node 02`
3. 快照 3
   placeholder：`Display name`
   文本：空

底部 preview 在整条轨道中始终固定：
1. `compact`
   placeholder：`Compact`
   文本：`Queued`
2. `read only`
   placeholder：`Read only`
   文本：`Managed`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 128`
- 主控件：`196 x 40`
- 底部 preview 行：`216 x 32`
- 单个 preview：`104 x 32`
- 页面结构：标题 -> 主 `text_box` -> 底部 `compact / read only`
- 风格约束：浅灰 page panel、白色主表面、低噪音边框、轻量焦点描边和稳定的文本层级，不回退到 showcase 式说明卡片

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Node 01` | `Queued` | `Managed` |
| 快照 2 | `Node 02` | 保持不变 | 保持不变 |
| 快照 3 | 空文本，仅显示 placeholder | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到默认 `Node 01` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Node 02`
4. 抓取第二组主区快照
5. 切到空文本态
6. 抓取第三组主区快照
7. 回到默认 `Node 01`
8. 抓取最终稳定帧

说明：
- 录制阶段不再发送 `Backspace`、数字键或 `Enter`
- 录制阶段不再点击 `compact` preview
- 底部 preview 统一通过 `hcw_text_box_override_static_preview_api()` 吞掉 `touch / key` 且不改状态
- preview 只负责静态 reference 对照，不再承担清焦或页面状态桥接职责
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `TEXT_BOX_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_text_box.c` 当前覆盖两部分：

1. 主控件交互与状态守卫
   覆盖样式 helper、键盘编辑与提交、导航键、setter / max_length，以及只读态输入拦截。
2. 静态 preview 不变性断言
   通过 `text_box_preview_snapshot_t`、`capture_preview_snapshot()`、`assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`on_submit`、`font`、`placeholder`、`text_color`、`text_alpha`、`placeholder_color`、`placeholder_alpha`、`cursor_color`、`text`、`text_len`、`cursor_pos`、`max_length`、`cursor_visible`、`align_type`、`scroll_offset_x`、`alpha`、`enable`、`is_focused`、`padding`

同时要求：
- `is_pressed == false`
- `submit_count == 0`
- `submitted_text == ""`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/text_box PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/text_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/text_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_text_box
```

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=input/text_box PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 和 `X:\output\main.exe`，总计 `845 / 845`，其中 `text_box` suite `6 / 6`
- `sync_widget_catalog.py`：已通过，本轮无额外目录变化
- `touch release semantics`：已通过，结果 `custom_audited=28 custom_skipped_allowlist=5`
- `docs encoding`：已通过，结果 `134 files`
- `widget catalog check`：已通过，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：已通过 `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/text_box --track reference --timeout 10 --keep-screenshots`，输出 `9 frames captured -> D:\workspace\gitee\EmbeddedGUI_Widgets\runtime_check_output\HelloCustomWidgets_input_text_box\default`
- input 分类 compile/runtime 回归：已通过
  compile `33 / 33`
  runtime `33 / 33`
- wasm 构建：已通过 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/text_box`，输出 `web/demos/HelloCustomWidgets_input_text_box`
- web smoke：已通过 `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_text_box`，结果 `PASS status=Running canvas=480x480 ratio=0.0977 colors=103`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_input_text_box/default`

复核结果：
- 总帧数：`9`
- 主区 RGB 差分边界：`(45, 196) - (118, 206)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁切后，主区唯一状态数：`3`
- 按 `y >= 258` 裁切底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

结论：
- 主区变化严格收敛在 `text_box` 文本区，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区保持 `3` 组唯一状态：`[0,1,6,7,8]` 对应默认 `Node 01`，`[2,3]` 对应 `Node 02`，`[4,5]` 对应空文本态；最终稳定帧已显式回到默认 `Node 01`。
- 按 `y >= 258` 裁切底部 preview 区域后保持单哈希，确认 `compact / read only` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前只覆盖单行 `TextBox` reference，不扩展多行编辑器或搜索框派生能力。
- 当前不接入系统 IME、虚拟键盘联动或表单级验证。
- 底部 preview 仅作为静态 reference，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `password_box`：这里展示明文文本输入，不承担 reveal / hide。
- 相比 `number_box`：这里不做数值范围、步进和后缀约束。
- 相比 `search_box`：这里不包含搜索图标、clear affordance 和搜索入口语义。

## 14. EGUI 适配说明
- 继续复用 SDK `textinput` 的文本编辑、光标与提交逻辑，不修改 SDK。
- Fluent 风格外观与静态 preview 语义都收口在 custom 层。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制无污染，再评估是否扩展更多文本输入派生控件。
