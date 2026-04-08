# Fader Scenes

## 定位

`fader_bank` 是音频/舞台场景相关的历史 showcase 输入控件，用来表现多通道推子、焦点通道和场景切换。它具有明显的专业设备语义，不适合作为通用 Fluent 输入控件主线。

## 当前示例内容

- 主卡在 `Scene A / Scene B` 两组快照间切换，示例通道包括 `DR / BS / VX / FX / MB`。
- `Compact` 预览提供 `A / B / C / D` 四通道压缩视图，用于小尺寸验证。
- `Locked` 预览固定在只读态，观察禁用推子和焦点通道的弱化表现。
- 点击主卡或紧凑卡片时，状态文本会在 `Scene A / Scene B / Compact A / Compact B` 之间更新。

## 现状判断

- 轨道：`showcase`
- 该控件更偏行业控制台场景，不属于 `Fluent 2 / WPF UI` 通用输入语义。
- 当前保留它，主要是为了保留多通道垂直推子和焦点高亮的实现样例。

## 使用建议

- 适合音频调音、灯光控制、媒体混音这类专业场景原型。
- 如果项目主线只需要普通数值输入，应优先使用更标准的 reference 控件。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=input/fader_bank PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/fader_bank --timeout 10 --keep-screenshots
```
