# Toggle Split Button

## 参考来源

- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件语义：`ToggleSplitButton`

## 定位

`toggle_split_button` 是当前主线 reference 输入控件，用来表达“带保持状态的主动作”与“附加菜单动作”同时存在的场景。它比普通 `toggle_button` 多了分裂菜单入口，也比 `split_button` 多了稳定的 checked 语义。

## 当前示例内容

- 主卡展示标准态，包含标题、辅助说明、主按钮区和菜单区。
- `Compact` 预览压缩了布局高度，只保留核心结构与选中状态。
- `Read only` 预览验证只读态下的边框、`ON/OFF` 文案和 chevron 是否仍然清楚。
- 录制动作会切换 checked 状态、左右焦点部分以及多个快照。

## 现状判断

- 轨道：`reference`
- 该控件是带二段式操作的主线基线，后续凡是同时需要开关状态和附加菜单的输入场景，都应优先向它靠齐。
- 设计上应继续保持 Fluent 2 / WPF UI 的浅色、克制和语义清晰，不回退到重装饰风格。

## 使用建议

- 适合模式切换、发布开关、工具启停这类需要“主动作 + 更多选项”的页面。
- 不适合拿来替代完整 `menu_flyout` 或复杂多级命令面板。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=input/toggle_split_button PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/toggle_split_button --timeout 10 --keep-screenshots
```
