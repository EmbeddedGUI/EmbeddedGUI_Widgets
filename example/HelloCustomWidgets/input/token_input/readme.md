# token_input 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：常见 `TokenInput / Tag editor / recipient field`
- 对应组件：`TokenInput`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主控件支持 token 输入、提交、删除和键盘焦点循环
- 当前移除内容：页面级 guide、状态文案、preview 轮换、preview 清焦桥接、showcase/HMI 装饰、runtime 真实输入与删除录制

## 1. 为什么需要这个控件
`token_input` 用来表达“在同一个输入槽里连续提交多个离散值”的表单语义，适合收件人、标签、过滤条件和设备分组编辑。相比普通 `text_box` 或展示型 `chips`，它同时承担输入位、已提交 token、删除 affordance 和焦点导航。

## 2. 当前页面结构
- 标题：`Token Input`
- 主区：一个可真实交互的 `token_input`
- 底部：两个静态 preview
- 左侧 preview：`compact`，固定展示 `UI / QA / OPS / SYS / NET`
- 右侧 preview：`read only`，固定展示 `Audit / Ops / QA / Net / Sys`

当前目录：`example/HelloCustomWidgets/input/token_input/`

## 3. 主区参考快照
主控件在录制轨道里只程序化切换三组 reference 状态，不再录制真实输入、提交和 remove：

1. `Add person`
   token：`Alice / Ops / QA`
2. `Add tag`
   token：`Design / Demo / VIP / Build`
3. `Add source`
   token：`Audit / Mail / Sync`

底部 preview 始终固定：

1. `compact`
   placeholder 为 `Add`，token 为 `UI / QA / OPS / SYS / NET`
2. `read only`
   token 为 `Audit / Ops / QA / Net / Sys`，并开启 `read only`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局基准尺寸：`224 x 180`
- 主控件尺寸：`196 x 92`
- 底部行容器尺寸：`216 x 48`
- 单个 preview 尺寸：`104 x 48`
- 页面风格：浅灰 page panel、白色 surface、低噪音边框、低饱和 token 胶囊

## 5. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认 token 列表 | 是 | 是 | 是 |
| draft 输入与提交 | 是 | 否 | 否 |
| remove token | 是 | 否 | 否 |
| `Left / Right / Home / End / Tab` 焦点循环 | 是 | 否 | 否 |
| overflow 摘要 | 是 | 是 | 是 |
| 静态 reference 对照 | 否 | 是 | 是 |

## 6. 录制动作设计
录制轨道已经收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Add tag`
4. 抓取第二组主区快照
5. 切到 `Add source`
6. 抓取第三组主区快照
7. 等待并抓取最终稳定帧

说明：
- 主区仍然保留真实 touch 和 keyboard 行为，供运行时手动交互
- runtime 录制阶段不再执行真实输入 `NET`、`Enter` 提交、remove 点击或 preview 轮换
- 底部 preview 只做静态 reference，对外统一吞掉 touch 和 key

## 7. 单元测试口径
`example/HelloUnitTest/test/test_token_input.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 draft 输入、`Enter / Comma / Space` 提交、`Backspace / Delete` 删除、remove 触摸、同目标 release 语义、setter pressed 清理、`ACTION_CANCEL`、`read only / disabled` guard、overflow 下隐藏输入位后的 draft 保留与焦点恢复
2. 静态 preview 不变性断言
   通过 `token_input_preview_snapshot_t`、`capture_preview_snapshot()`、`assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`font`、`meta_font`、`on_changed`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`shadow_color`、`placeholder`、`draft_text`、`tokens`、`token_count`、`draft_len`、`current_part`、`pressed_part`、`pressed_remove`、`compact_mode`、`read_only_mode`、`restore_input_focus`、`alpha`

同时要求：
- `g_changed_count == 0`
- `g_changed_token_count == 0xFF`
- `g_changed_part == EGUI_VIEW_TOKEN_INPUT_PART_NONE`
- `is_pressed == false`

## 8. 验收命令
```bash
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

make all APP=HelloCustomWidgets APP_SUB=input/token_input PORT=pc

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/token_input --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/token_input
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_token_input
```

## 9. 本轮结果
- `HelloUnitTest`：`845 / 845` 通过，`token_input` suite `44 / 44`
- `touch release semantics`：PASS
- `docs encoding`：PASS
- `widget catalog`：PASS
- widget 级 runtime：PASS
- input 分类 compile/runtime 回归：PASS
- wasm 构建：PASS
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1373 colors=104`

## 10. Runtime 复核结论
复核目录：`runtime_check_output/HelloCustomWidgets_input_token_input/default`

- 总帧数：`8`
- 主区 RGB 差分边界：`(65, 140) - (424, 159)`
- 遮罩主区变化边界后，边界外唯一哈希数：`1`
- 按主区变化边界裁切后，主区唯一状态数：`3`
- 按 `y >= 282` 裁切底部 preview 区域后，preview 区唯一哈希数：`1`

结论：
- 变化只发生在主区 token 行内部，符合 `Add person`、`Add tag`、`Add source` 三态轨道
- 主区变化边界外全程静态，没有黑屏、白屏、错位或 preview 污染
- 底部 `compact / read only` preview 在整条录制轨道中保持静态一致

## 11. 已知限制
- 当前不做拖拽重排、批量粘贴解析和 IME 候选联动
- 当前仍以固定 snapshot 和固定录制脚本为主，不接动态建议源
- overflow 摘要只做静态压缩显示，不展开完整 token 面板

## 12. 与现有控件的边界
- 相比 `text_box`：这里是多 token 编辑器，不是单值文本框
- 相比 `chips`：这里保留输入位与编辑闭环，不是纯展示 chip 列表
- 相比 `auto_suggest_box`：这里强调“已提交 token + 当前 draft”的混合状态，而不是建议下拉

## 13. EGUI 适配说明
- 继续复用仓库内现有 `token_input` 基础实现，优先把示例页收口到统一的 reference 页面与静态 preview 工作流
- 通过 `egui_view_token_input_override_static_preview_api()` 让底部 preview 不再承担第二套交互逻辑
- 当前优先保证 release 语义、pressed 清理、隐藏输入恢复和 runtime 渲染稳定，再评估是否继续上升为框架层公共控件
