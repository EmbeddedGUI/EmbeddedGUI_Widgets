# Heatmap Chart

## 定位

`heatmap_chart` 是历史展示型图表 demo，用来演示矩阵热力图、紧凑卡片和锁定态的组合效果。它同样不属于 `Fluent 2 / WPF UI` 主线参考体系，当前只保留作回归和资料用途。

## 当前示例内容

- 主卡展示 `4 x 4` 的热力矩阵，并在 `Core A / Core B` 两组值之间切换。
- `Compact` 预览会在两张小热图之间轮换，同时切换数值可见性和强调色。
- `Locked` 预览固定为只读缩略图，用来观察弱化态的边框、文字和热点分布。
- 页面引导语是 `Tap cards to cycle`，运行时点击主卡或紧凑卡即可切换快照。

## 现状判断

- 轨道：`deprecated`
- 默认网页构建不再发布该控件。
- 如果主线需要热力图，应先补齐统一的浅色 reference 视觉，而不是继续沿用这套深色展示模板。

## 使用建议

- 仅在需要历史图表案例、验证矩阵布局或回放旧截图时使用。
- 不建议继续扩展为通用业务控件。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=chart/heatmap_chart PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub chart/heatmap_chart --timeout 10 --keep-screenshots
```
