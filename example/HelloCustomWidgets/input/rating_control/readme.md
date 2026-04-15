# rating_control 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`RatingControl`
- 当前保留形态：`standard`、`compact`、`read only`
- 当前保留交互：主控件支持 touch 评分、拖拽跨星更新、`Clear` 清空、键盘步进评分
- 当前移除内容：页面级 guide、说明面板、preview 轮换、preview 清焦桥接、showcase/HMI 装饰、半星评分、额外 hover 动画

## 1. 为什么需要这个控件
`rating_control` 用于表达 1 到 5 档的离散评分，适合服务评价、速度反馈、安装体验和满意度选择。相比 `radio_button`、`slider`、`number_box` 这类通用输入，星级评分更贴近用户对“满意度档位”的认知。

## 2. 当前页面结构
- 标题：`Rating Control`
- 主区：一个可真实交互的 `rating_control`
- 底部：两个静态 preview
- 左侧 preview：`compact`，固定展示 `3 / 5`
- 右侧 preview：`read only`，固定展示 `4 / 5`

当前目录：`example/HelloCustomWidgets/input/rating_control/`

## 3. 主区参考快照
主控件在录制轨道里只程序化切换三组 reference 状态，不再录制真实点击和 preview 轮换：

1. `Service quality`
   `Low / High`，值为 `4`
2. `Delivery speed`
   `Slow / Fast`，值为 `2`
3. `Setup experience`
   `Hard / Easy`，值为 `5`

底部 preview 始终固定：

1. `compact`
   使用 `No rating / Poor / Fair / Good / Great / Excellent` 文案，值为 `3`
2. `read only`
   使用同一组文案，值为 `4`，并开启 `read only`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局基准尺寸：`224 x 174`
- 主控件尺寸：`196 x 92`
- 底部行容器尺寸：`216 x 42`
- 单个 preview 尺寸：`104 x 42`
- 页面风格：浅灰页面底板、白色评分卡、低噪音边框、暖金色评分 accent

## 5. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | 是 | 是 | 是 |
| 点击星级提交 | 是 | 否 | 否 |
| 拖拽跨星后按最新星位提交 | 是 | 否 | 否 |
| `Clear` 清空 | 是 | 否 | 否 |
| 键盘 `Left/Right/Up/Down/Home/End/Tab/Enter/Space/Esc` | 是 | 否 | 否 |
| 静态 reference 对照 | 否 | 是 | 是 |

## 6. 录制动作设计
录制轨道已经收口为静态 preview 工作流：

1. 应用默认主区快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Delivery speed`
4. 抓取第二组主区快照
5. 切到 `Setup experience`
6. 抓取第三组主区快照
7. 等待并抓取最终稳定帧

说明：
- 主区仍然保留真实 touch 和 keyboard 行为，供运行时手动交互
- runtime 录制阶段不再执行点击第 5 星、点 `Clear`、`End` 键步进或 compact preview 轮换
- 底部 preview 只做静态 reference，对外统一吞掉 touch 和 key

## 7. 单元测试口径
`example/HelloUnitTest/test/test_rating_control.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 `value / current_part / clear / item_count / value_labels / compact / read_only / disabled / touch / key`
2. 静态 preview 不变性断言
   通过 `rating_control_preview_snapshot_t`、`capture_preview_snapshot()`、`assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`font`、`meta_font`、`on_changed`、`title`、`low_label`、`high_label`、`value_labels`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`shadow_color`、`label_count`、`item_count`、`current_value`、`current_part`、`compact_mode`、`read_only_mode`、`clear_enabled`、`pressed_part`、`alpha`

同时要求：
- `g_changed_count == 0`
- `g_changed_value == 0xFF`
- `g_changed_part == EGUI_VIEW_RATING_CONTROL_PART_NONE`
- `is_pressed == false`

## 8. 验收命令
```bash
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

make all APP=HelloCustomWidgets APP_SUB=input/rating_control PORT=pc

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

本轮结果：
- `HelloUnitTest`：`845 / 845` 通过，`rating_control` suite `37 / 37`
- `touch release semantics`：PASS
- `docs encoding`：PASS
- `widget catalog`：PASS
- widget 级 runtime：PASS
- input 分类 compile/runtime 回归：PASS
- wasm 构建：PASS
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1327 colors=147`

## 9. Runtime 复核结论
复核目录：`runtime_check_output/HelloCustomWidgets_input_rating_control/default`

- 总帧数：`8`
- 主区 RGB 差分边界：`(54, 146) - (312, 213)`
- 遮罩主区变化边界后，边界外唯一哈希数：`1`
- 按主区边界裁切后，主区唯一状态数：`3`
- 按 `y >= 288` 裁切底部 preview 区域后，preview 区唯一哈希数：`1`

结论：
- 变化只发生在主区评分卡内部，符合 `Service quality`、`Delivery speed`、`Setup experience` 三态轨道
- 主区外区域全程静态，没有黑屏、白屏、错位或 preview 污染
- 底部 `compact / read only` preview 在整条录制轨道中保持静态一致

## 10. 已知限制
- 当前只支持整星评分，不支持半星或浮点评分
- `Clear` 是页内轻量入口，不做二次确认
- 底部 preview 只承担 reference 对照，不承担交互职责

## 11. 与现有控件的边界
- 相比 `radio_button`：这里表达评分档位，不是互斥选项
- 相比 `slider`：这里是离散评分，不是连续调节
- 相比 `segmented_control`：这里表达满意度，不是页面切换
- 相比 `number_box`：这里表达评价等级，不是数值录入
