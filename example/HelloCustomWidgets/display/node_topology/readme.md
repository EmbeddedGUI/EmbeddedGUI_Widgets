# Node Topology

## 定位

`node_topology` 是网络/设备关系展示 demo，主卡和底部两张缩略图分别表现完整拓扑、紧凑拓扑和锁定拓扑。它属于历史 showcase 轨道，用来保留复杂节点关系的可视化样例。

## 当前示例内容

- 主卡在 `Cluster A` 与 `Cluster B` 两组拓扑快照之间切换，并高亮当前焦点节点。
- `Compact` 预览显示 4 个节点的缩略拓扑，适合观察小尺寸布局和路线强调。
- `Locked` 预览保留禁用态，用来确认节点与连线在弱化后仍然可辨认。
- 状态文本会在 `Cluster A route / Cluster B route / Compact A map / Compact B map` 之间变化。

## 现状判断

- 轨道：`showcase`
- 该控件没有直接对应的 Fluent 2 / WPF UI 标准组件，更多是领域型展示控件。
- 保留价值主要在于复杂关系图、布局压缩和绘制能力验证，而不是通用业务主线。

## 使用建议

- 适合展示设备拓扑、链路概览、资源依赖等领域页面原型。
- 如需进入主线，应该先定义统一的 reference 语义和浅色视觉，再考虑是否保留。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=display/node_topology PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/node_topology --timeout 10 --keep-screenshots
```
