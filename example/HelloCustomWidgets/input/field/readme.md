# field 自定义控件设计说明
## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`Fluent UI React`
- 对应组件：`Field`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主区保留 `info button / bubble` 的 same-target release、`Enter / Space` 开关与 `Esc` 关闭语义；底部 `compact / read only` 统一收口为静态 preview
- 当前移除内容：页面级 guide、底部 panel 包装与 heading、录制阶段真实点击 `info button`、preview 清焦桥接与额外收尾轨道

## 1. 为什么需要这个控件
`field` 用来把 `label`、必填标记、辅助说明、校验信息和轻量 info 入口稳定地组织在同一个字段上下文里。仓库已经有 `text_box`、`info_label` 和 `message_bar`，但还缺少一颗与 `Fluent 2 / Fluent UI React Field` 语义对齐的字段壳层 reference。

## 2. 为什么现有控件不够用
- `text_box` 只覆盖输入框本体，不承载字段级 `label / helper / validation / info`
- `info_label` 更像独立说明入口，不承担字段壳层布局
- `message_bar` 面向整条反馈条，不适合贴在字段底部做低噪音校验提示
- SDK 基础控件可以复用绘制与交互能力，但仓库仍需要完整的 `input/field` 页面、单测和 web 验收闭环

## 3. 当前页面结构
- 标题：`Field`
- 主区：一个标准 `field`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `Alias / core-api`
- 右侧 preview：`read only`，固定显示 `Region / North Europe`

目录：
- `example/HelloCustomWidgets/input/field/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，不再在录制阶段发送真实点击事件：

1. 快照 1
   label：`Notification email`
   text：空
   placeholder：`name@company.com`
   helper：`Used for build alerts only.`
2. 快照 2
   label：`API token`
   text：`staging-reader`
   required：开启
   validation：`Required before handoff.`
   info bubble：程序化设为展开，标题为 `Why required`
3. 快照 3
   label：`Owner alias`
   text：空
   placeholder：`team@example.com`
   helper：`Used by audit routing.`
   validation：`Enter a valid alias before saving.`

底部 preview 在整条轨道中始终固定：
1. `compact`
   label：`Alias`
   text：`core-api`
2. `read only`
   label：`Region`
   text：`North Europe`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 220`
- 主控件：`196 x 126`
- 底部 preview 行：`216 x 54`
- 单个 preview：`104 x 54`
- 页面结构：标题 -> 主 `field` -> 底部 `compact / read only`
- 风格约束：浅色 page panel、白色主表面、低噪音边框、字段级辅助文字与校验提示，不回退到 showcase 式说明面板

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `Notification email` + helper | `Alias / core-api` | `Region / North Europe` |
| 快照 2 | `API token` + required + warning + open bubble | 保持不变 | 保持不变 |
| 快照 3 | `Owner alias` + error | 保持不变 | 保持不变 |
| 录制最终稳定帧 | 回到默认 `Notification email` + helper | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `API token`
4. 抓取第二组主区快照
5. 切到 `Owner alias`
6. 抓取第三组主区快照
7. 回到默认 `Notification email`
8. 抓取最终稳定帧

说明：
- 录制阶段不再真实点击 `info button`
- warning + bubble 状态直接通过 `hcw_field_set_open()` 程序化设定
- 底部 preview 统一通过 `hcw_field_override_static_preview_api()` 吞掉 `touch / key` 且不改状态
- preview 只负责静态 reference 对照，不再承担页面桥接职责
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 `3` 组主区快照和最终稳定帧口径一致。

当前 `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：保留既有 `FIELD_DEFAULT_SNAPSHOT` 与 `apply_primary_default_state()`，初始化阶段在 root view 挂载前后各重放一次默认态与 preview，`case 0` 和最终稳定帧前的默认态恢复统一走显式布局路径。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_field.c` 当前覆盖两部分：

1. 主控件交互与状态守卫
   覆盖样式 helper、setter 清理 `pressed`、same-target release / cancel、`Enter / Space / Esc`、`read only / !enable` guard
2. 静态 preview 不变性断言
   通过 `field_preview_snapshot_t`、`capture_preview_snapshot()` 与 `assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`background`、`label`、`field_text`、`placeholder`、`helper_text`、`validation_text`、`info_title`、`info_body`、`font`、`meta_font`、`icon_font`、`on_open_changed`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`success_color`、`warning_color`、`error_color`、`bubble_surface_color`、`shadow_color`、`required`、`compact_mode`、`read_only_mode`、`open`、`validation_state`、`pressed_part`、`alpha`、`enable`、`is_focused`、`padding`

同时要求：
- `is_pressed == false`
- `g_open_count == 0`
- `g_open_state == 0xFF`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/field PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/field --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/field
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_field
```

## 10. 当前验收结果（2026-04-18）
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=input/field PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 和 `X:\output\main.exe`，总计 `845 / 845`，其中 `field` suite `5 / 5`
- `sync_widget_catalog.py`：已通过，本轮无额外目录变化
- `touch release semantics`：已通过，结果 `custom_audited=28 custom_skipped_allowlist=5`
- `docs encoding`：已通过，结果 `134 files`
- `widget catalog check`：已通过，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：已通过 `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/field --track reference --timeout 10 --keep-screenshots`，输出 `9 frames captured -> D:\workspace\gitee\EmbeddedGUI_Widgets\runtime_check_output\HelloCustomWidgets_input_field\default`
- input 分类 compile/runtime 回归：已通过
  compile `33 / 33`
  runtime `33 / 33`
- wasm 构建：已通过 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/field`，输出 `web/demos/HelloCustomWidgets_input_field`
- web smoke：已通过 `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_field`，结果 `PASS status=Running canvas=480x480 ratio=0.1678 colors=80`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_input_field/default`

复核结果：
- 总帧数：`9`
- 主区 RGB 差分边界：`(46, 108) - (435, 222)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区裁剪后，主区唯一状态数：`3`
- 按 `y >= 222` 裁切底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

结论：
- 主区变化严格收敛在 `field` 主体，主区外页面 chrome 在整条轨道中保持静态。
- `9` 帧里主区保持 `3` 组唯一状态：`[0,1,6,7,8]` 对应默认 `Notification email`，`[2,3]` 对应 `API token` + warning + open bubble，`[4,5]` 对应 `Owner alias` + error；最终稳定帧已显式回到默认态。
- 按 `y >= 222` 裁切底部 preview 区域后保持单哈希，确认 `compact / read only` preview 在整条录制轨道中始终静态一致。

## 12. 已知限制
- 当前 `field box` 仍是静态 field shell，不承载任意 child slot
- 当前 bubble 为控件内 anchored bubble，不引入系统级 popup
- 底部 preview 仅作静态 reference，对外不承担额外交互职责

## 13. 与现有控件的边界
- 相比 `text_box`：这里聚焦字段壳层，不承担真实文本编辑
- 相比 `info_label`：这里把说明入口放回字段上下文，而不是独立标签
- 相比 `message_bar`：这里只保留字段级低噪音校验提示，不做整条反馈条

## 14. EGUI 适配说明
- 继续使用当前目录下的 `egui_view_field` custom view，不修改 SDK
- `label / helper / validation / info bubble / static preview` 语义全部收口在 custom 层
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制无多余交互噪音，再评估是否需要扩展到更通用的字段容器
