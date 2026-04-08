# Candlestick Chart

## 定位

`candlestick_chart` 是一组偏金融行情可视化的历史 demo，主卡展示完整蜡烛图，底部保留 `Compact` 与 `Locked` 两个缩略预览。它不对应 `Fluent 2 / WPF UI` 当前主线里的标准组件，也不再属于本仓库统一风格的延续方向。

## 当前示例内容

- 主卡在 `Core A / Core B` 两组数据之间切换，包含 8 个时间片标签。
- `Compact` 预览在两组 6 列数据之间轮换，并切换是否显示标签。
- `Locked` 预览保留只读外观，用来验证弱化状态下的可读性。
- 运行时点击主卡或紧凑预览，会更新状态文本和配色。

## 现状判断

- 轨道：`deprecated`
- 站点默认构建不会发布该控件；只有显式使用 `--include-deprecated` 时才会重新进入 `demos.json`。
- 如果后续真的需要主线图表，应重新定义面向 Fluent 风格的 reference 方案，而不是继续沿用这套深色展示稿。

## 使用建议

- 仅适合回看旧方案、验证自定义绘制能力或保留历史截图。
- 不建议继续追加交互、皮肤或框架级抽象。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=chart/candlestick_chart PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub chart/candlestick_chart --timeout 10 --keep-screenshots
```
