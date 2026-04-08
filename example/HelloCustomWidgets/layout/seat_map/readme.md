# Seat Map

## 定位

`seat_map` 是座位块布局的 showcase 控件，适合剧场、会议室、车厢或活动区块原型。它保留了完整区块图、紧凑预览和锁定态，但目前仍停留在领域展示层。

## 当前示例内容

- 主卡在 `Block A / Block B` 两组座位状态之间切换，并同步更新焦点行与焦点座位。
- `Compact` 预览使用 `3 x 4` 的缩略块图，便于测试小尺寸布局。
- `Locked` 预览验证只读状态下座位占用、焦点位置和边框是否仍清晰。
- 状态文本会在 `Block A row / Block B row / Compact A seats / Compact B seats` 之间轮换。

## 现状判断

- 轨道：`showcase`
- 该控件属于明显的行业布局组件，不对应 Fluent 通用参考组件。
- 当前保留它，主要是为了保留网格布局、焦点定位和状态映射的样例。

## 使用建议

- 适合票务、会议区块、设备机位或座位排布原型。
- 如果要推进主线，应先抽离领域语义，再评估是否需要保留统一 reference 版本。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=layout/seat_map PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub layout/seat_map --timeout 10 --keep-screenshots
```
