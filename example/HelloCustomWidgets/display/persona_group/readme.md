# persona_group 设计说明
## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 平台语义参考：`WinUI AvatarGroup`
- 补充对照实现：`ModernWpf`
- 对应组件：`AvatarGroup`
- 当前保留语义：`standard`、`compact`、`read only`、`accent`、`success`、`warning`、`neutral`、`live`、`busy`、`away`、`idle`
- 当前保留交互：主区保留程序化 snapshot 切换，底部 `compact / read only` 统一收口为静态 preview
- 当前移除内容：旧 `preview` 轮换、preview 点击桥接清主区焦点、额外收尾帧、页面级 `guide / status` 文案、旧双列预览包裹层
- EGUI 适配说明：继续复用当前目录下的 `egui_view_persona_group` custom view，在不修改 `sdk/EmbeddedGUI` 的前提下，把 `reference` 页面统一收口到主区三态 snapshots 加底部双静态 preview

## 1. 为什么需要这个控件
`persona_group` 用来表达一组协作成员及其当前关注对象。它适合出现在审阅链、责任人概览、交接班摘要和归档面板中，用一张轻量卡片同时传达成员身份、presence 和当前焦点成员。

## 2. 为什么现有控件不够用
- `persona` 只覆盖单人信息，不表达成员组关系和重叠头像结构。
- `person_picture` 只表达头像本身，不承担标题、摘要和焦点成员语义。
- `badge_group` 表达的是状态标签集合，不承载成员身份和 presence。
- 当前 `reference` 主线仍需要一版与 `Fluent / WPF UI AvatarGroup` 语义对齐的轻量成员组控件。

## 3. 当前页面结构
- 标题：`Persona Group`
- 主区：一个标准 `persona_group`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`
- 右侧 preview：`read only`

目录：
- `example/HelloCustomWidgets/display/persona_group/`

## 4. 主区 reference snapshots
主区录制轨道只保留 `3` 组 reference snapshots：

1. 默认态
   - eyebrow：`DESIGN`
   - title：`Design review`
   - summary：`Design team`
   - 焦点成员：`LM Lena Design`
2. 快照 2
   - eyebrow：`OPS`
   - title：`Ops handoff`
   - summary：`Shift lead`
   - 焦点成员：`IV Ivy PM`
3. 快照 3
   - eyebrow：`ARCHIVE`
   - title：`Archive sweep`
   - summary：`Restore desk`
   - 焦点成员：`MB Mina Archive`

底部 preview 在整条录制轨道中始终固定：

1. `compact`
   - title：`Compact`
   - summary：`Short roster`
   - 成员：`LM Lena Lead`、`AR Arun Ops`、`MY Maya QA`
2. `read only`
   - title：`Read only`
   - summary：`Muted roster`
   - 成员：`MB Mina Archive owner`、`KO Kora Retention QA`、`YU Yuri Restore desk`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 230`
- 标题：`224 x 18`
- 主 `persona_group`：`196 x 114`
- 底部 preview 行：`216 x 76`
- 单个 preview：`104 x 76`
- 页面结构：标题 -> 主 `persona_group` -> 底部 `compact / read only`
- 风格约束：浅色 page panel、低噪音边框、保留头像重叠与 presence dot，但不回退到旧 showcase 风格的高对比说明页

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `DESIGN / Design review / Design team` | `Compact / Short roster` | `Read only / Muted roster` |
| 快照 2 | `OPS / Ops handoff / Shift lead` | 保持不变 | 保持不变 |
| 快照 3 | `ARCHIVE / Archive sweep / Restore desk` | 保持不变 | 保持不变 |
| 录制最终稳定帧 | `ARCHIVE / Archive sweep / Restore desk` | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认 snapshot 和底部 preview 固定状态
2. 抓取首帧
3. 切到 `OPS / Ops handoff / Shift lead`
4. 抓取第二组主区快照
5. 切到 `ARCHIVE / Archive sweep / Restore desk`
6. 抓取第三组主区快照
7. 继续等待并抓取最终稳定帧

说明：
- 录制阶段不再轮换 `compact` preview。
- 不再通过 preview 点击桥接去清主区焦点。
- 不再保留额外 preview 点击后的收尾帧。
- `request_page_snapshot()` 会统一做 `layout + invalidate + recording_request_snapshot()`，保证 3 组主区快照和最终稳定帧口径一致。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_persona_group.c` 当前覆盖 `7` 个用例：

1. `set_snapshots` 钳制与 `pressed` 清理
2. snapshot 与 setter 更新后的 `pressed` 清理
3. metrics、命中测试与 helper
4. `same-target release`、`ACTION_CANCEL` 与 touch 行为
5. `read only / disabled` guard 清理残留 `pressed`
6. 键盘导航与边界守卫
7. 静态 preview `consumes input and keeps state`

静态 preview 用例通过 `persona_group_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验以下字段：

- `region_screen`
- `background`
- `snapshots`
- `font`
- `meta_font`
- `on_focus_changed`
- `api`
- `surface_color`
- `border_color`
- `section_color`
- `text_color`
- `muted_text_color`
- `accent_color`
- `success_color`
- `warning_color`
- `neutral_color`
- `snapshot_count`
- `current_snapshot`
- `current_index`
- `compact_mode`
- `read_only_mode`
- `pressed_index`
- `alpha`
- `enable`
- `is_focused`
- `is_pressed`
- `padding`

补充说明：
- preview 事件分发已统一走 `dispatch_touch_event()` 与 `dispatch_key_event()`。
- 静态 preview 只负责吞掉输入并恢复到原始静态状态，不触发 `focus changed`，也不改 `current_snapshot / current_index`。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=display/persona_group PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/persona_group --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/persona_group
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_persona_group
```

## 10. 当前结果
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=display/persona_group PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 和 `X:\output\main.exe`，总计 `845 / 845`，其中 `persona_group` suite `7 / 7`
- `sync_widget_catalog.py`：已通过，本轮无额外目录变化
- `touch release semantics`：已通过，结果 `custom_audited=21 custom_skipped_allowlist=0`
- `docs encoding`：已通过，结果 `134 files`
- `widget catalog check`：已通过，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：已通过 `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/persona_group --track reference --timeout 10 --keep-screenshots`，输出 `8` 帧截图
- display 分类 compile/runtime 回归：已通过 `python scripts/code_compile_check.py --custom-widgets --category display --bits64` 和 `python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64`，分类内 `21` 个控件全部通过
- wasm 构建：已通过 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/persona_group`，输出 `web/demos/HelloCustomWidgets_display_persona_group`
- web smoke：已通过 `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_persona_group`，结果 `PASS status=Running canvas=480x480 ratio=0.1754 colors=400`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_display_persona_group/default`

复核结果：
- 总帧数：`8`
- 主区 RGB 差分边界：`(198, 103) - (282, 258)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区差分边界裁剪后，主区唯一状态数：`3`
- 按 `y >= 258` 裁剪底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

## 12. 已知限制
- 当前仍使用固定 `snapshot + item` 数据，不接真实头像资源和外部团队数据模型。
- 当前不扩展 hover、focus ring、成员详情弹层和真实 overflow 展开面板。
- `+n` overflow 仍是静态摘要气泡，不承接更多成员列表弹出层。
- 底部 `compact / read only` preview 只承担静态 reference 对照，不承载额外交互职责。

## 13. 与现有控件的边界
- 相比 `persona`：这里表达的是成员组关系，不是单人卡片。
- 相比 `person_picture`：这里保留标题、摘要和焦点成员，不只是头像占位。
- 相比 `badge_group`：这里表达的是成员身份和 presence，不是状态标签集合。
- 相比旧 showcase 页面：这里回到统一的 `reference` 页面结构，不再保留叙事式说明壳层。

## 14. EGUI 适配说明
- 继续复用当前目录下的 `egui_view_persona_group` custom view，不修改 SDK。
- 主区保留 `DESIGN`、`OPS`、`ARCHIVE` 三组 reference snapshots。
- 底部 preview 通过 `egui_view_persona_group_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 `3` 组 reference snapshots、底部 preview 全程静态，以及真实 touch / key 语义由单测闭环而不是由 runtime 录制承担。
