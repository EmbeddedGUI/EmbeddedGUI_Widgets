# Pivot 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 平台语义参考：`WinUI 3 Pivot`
- 次级补充参考：`WPF UI`、`ModernWpf`
- 对应组件名：`Pivot`
- 当前保留状态：`standard`、`compact`、`read only`、`same-target release`、键盘切换、静态 preview
- 当前移除内容：旧 `compact_panel / read_only_panel` 包装、标题说明文案、preview 轮换、录制里的 preview 切换桥接、手势翻页动画、场景化页面 chrome

## 1. 为什么需要这个控件
`pivot` 用来表达“顶部 header 负责切换分区，主体区域一次只展示一个内容页”的导航语义，适合总览、活动、历史这类平级 section 的轻量切换。它比 `tab_view` 更轻，不承担页签管理；又比单纯的选择条更完整，因为它需要把当前 body 内容一起收口。

## 2. 为什么现有控件不够用
- `tab_strip` 只覆盖 header 切换，不承载单页 body 区域。
- `selector_bar` 更偏向选择入口，缺少 `header + body` 一体化结构。
- `flip_view` 强调顺序翻页，不表达顶部 header 导航语义。
- `tab_view` 语义更重，偏桌面页签容器，不适合轻量 section 切换。

## 3. 当前页面结构
- 标题：`Pivot`
- 主区：一个可真实交互的 `primary_pivot`
- 底部：两个并排的静态 preview
- 左侧 preview：`compact`，固定显示 `Home`
- 右侧 preview：`read only`，固定显示 `Audit`

当前目录：`example/HelloCustomWidgets/navigation/pivot/`

## 4. 主区 reference 快照
主控件录制轨道已经收口为 3 组 reference 快照：

1. `Overview`
   `Core view / Project overview / Goals, owner and next step.`
2. `Activity`
   `Daily feed / Recent activity / Ship notes and open reviews.`
3. `History`
   `Past work / Change history / Milestones and archived updates.`

底部 preview 在整条轨道中保持固定：

1. `compact`
   `Home / Quick / Pinned work`
   `compact_mode=1`
   `current_index=0`
2. `read only`
   `Audit / Static / Read only`
   `compact_mode=1`
   `read_only_mode=1`
   `current_index=1`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局尺寸：`224 x 206`
- 主控件尺寸：`196 x 108`
- 底部行容器尺寸：`216 x 72`
- 单个 preview 尺寸：`104 x 72`
- 页面结构：标题 -> 主 `pivot` -> 底部 `compact / read only`
- 页面风格：浅灰 page panel、白色主 surface、低噪音边框、轻量 active fill 与 underline

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | 是 | 是 | 是 |
| `Overview / Activity / History` 三态切换 | 是 | 否 | 否 |
| `DOWN(A) -> MOVE(B) -> UP(B)` 不提交 | 是 | 否 | 否 |
| `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交 | 是 | 否 | 否 |
| `Left / Right / Up / Down / Home / End / Tab` 键盘切换 | 是 | 否 | 否 |
| `Enter / Space` consume 但不切换 | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且保持状态不变 | 否 | 是 | 是 |
| `read_only_mode / !enable` 先清 pressed 再拒绝输入 | 是 | 否 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为 static preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Activity`
4. 抓取第二组主区快照
5. 切到 `History`
6. 抓取第三组主区快照
7. 继续等待并抓取最终稳定帧

说明：
- 录制阶段不再切换 `compact` preview，也不再补录恢复默认态的额外收尾帧。
- 底部 preview 统一通过 `hcw_pivot_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照。
- 主区真实 touch 和 keyboard 语义仍然保留在控件实现与单测里闭环。

## 8. 单元测试口径
`example/HelloUnitTest/test/test_pivot.c` 当前覆盖四部分：

1. 样式 helper 与 setter 状态清理
   覆盖 `apply_compact_style()`、`apply_read_only_style()`、`set_font()`、`set_meta_font()`、`set_palette()`、`set_items()`、`set_current_index()` 对残留 `pressed` 的清理。
2. touch same-target release 与 cancel
   覆盖 `DOWN(A) -> MOVE(B) -> UP(B)` 不提交，以及回到 `A` 后才提交。
3. 键盘导航与 guard
   覆盖 `Right / End / Home / Tab / Enter / Space`，以及 `read_only_mode / !enable` 下的拒绝输入与 pressed 清理。
4. 静态 preview 不变性断言
   通过 `pivot_preview_snapshot_t`、`capture_preview_snapshot()` 和 `assert_preview_state_unchanged()` 固定校验：
   `region_screen / background / items / font / meta_font / on_changed / api / surface_color / border_color / text_color / muted_text_color / accent_color / card_surface_color / item_count / current_index / compact_mode / read_only_mode / pressed_index / alpha / enable / is_focused / is_pressed / padding`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/pivot PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/pivot --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/pivot
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_pivot
```

## 10. 当前结果
- `HelloCustomWidgets` 单控件编译：已通过 `make all APP=HelloCustomWidgets APP_SUB=navigation/pivot PORT=pc`
- `HelloUnitTest`：已在 `X:\` 短路径通过 `make clean APP=HelloUnitTest PORT=pc_test`、`make all APP=HelloUnitTest PORT=pc_test` 和 `X:\output\main.exe`，总计 `845 / 845`，其中 `pivot` suite `4 / 4`
- `sync_widget_catalog.py`：PASS，重新同步 `widget_catalog.json` 与 `web/catalog-policy.json`
- `touch release semantics`：PASS，结果 `custom_audited=12 custom_skipped_allowlist=1`
- `docs encoding`：PASS，结果 `134 files`
- `widget catalog check`：PASS，结果 `106 widgets: reference=106, showcase=0, deprecated=0`
- 单控件 runtime：PASS，输出 `8` 帧截图
- navigation 分类 compile/runtime 回归：PASS，分类内 `13` 个控件全部通过
- wasm 构建：PASS，输出 `web/demos/HelloCustomWidgets_navigation_pivot`
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1581 colors=171`

## 11. Runtime 复核结论
复核目录：`runtime_check_output/HelloCustomWidgets_navigation_pivot/default`

- 总帧数：`8`
- 主区 RGB 差分边界：`(54, 121) - (425, 264)`
- 遮罩主区变化边界后，主区外唯一哈希数：`1`
- 按主区边界裁切后，主区唯一状态数：`3`
- 按 `y >= 265` 裁切底部 preview 区域后，preview 区唯一哈希数：`1`

结论：
- 变化只发生在主区 `Overview / Activity / History` 三态轨道里。
- 主区外页面 chrome 全程静态，没有黑屏、白屏、错位或 preview 污染。
- 底部 `compact / read only` preview 在整条录制轨道中保持静态一致。

## 12. 已知限制
- 当前只覆盖固定 item 集合，不引入数据源、溢出折叠或滚动联动。
- header 宽度仍采用轻量估算，不接入真实文本测量。
- body 只展示摘要卡，不扩展为复杂分页容器。
- 底部 preview 只承担 reference 对照，不承担交互职责。

## 13. 与现有控件的边界
- 相比 `tab_strip`：这里补齐 body 区域，不只是 header 切换条。
- 相比 `selector_bar`：这里表达页内 section 切换，而不只是选择入口。
- 相比 `flip_view`：这里保留顶部 header 导航，不强调翻页手势。
- 相比 `tab_view`：这里不绑定页签壳层，也不处理 `close / add`。

## 14. EGUI 适配说明
- 继续在 custom 层维护轻量 `hcw_pivot`，不修改 `sdk/EmbeddedGUI`。
- 主区、`compact`、`read only` 复用同一套控件实现，只通过 palette 和 mode 切换表现。
- setter、guard 和 static preview 都统一走 pressed-state 清理逻辑。
- `HCW_PIVOT_MAX_ITEMS` 当前固定为 `6`，超出部分会被截断。
