# Tag Cloud

## 定位

`tag_cloud` 是历史 showcase 轨道里的词云展示 demo，用来验证词条权重、焦点轮换和紧凑态布局。它不属于当前 Fluent 主线参考控件，但仍有一定的展示和绘制验证价值。

## 当前示例内容

- 主卡在 `Cloud A / Cloud B` 两组词条之间切换，示例词包括 `dashboard / motion / clarity` 与 `preview / canvas / runtime`。
- `Compact` 预览提供两套更短的关键词集合，用来测试小尺寸权重分布。
- `Locked` 预览保持只读外观，验证禁用态下词云层级是否仍然清楚。
- 页面提示为 `Tap clouds to rotate focus`，点击主卡或紧凑卡即可轮换快照。

## 现状判断

- 轨道：`showcase`
- 该控件更适合专题展示、数据可视化概念页或品牌词汇墙，不适合作为常规业务主线组件。
- 目前保留它，主要是为了保留自定义布局与绘制样例。

## 使用建议

- 适合做词频预览、标签热度展示或概念验证页面。
- 如果要正式进入主线，应先明确是否真的需要词云这种高装饰密度表达。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=display/tag_cloud PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/tag_cloud --timeout 10 --keep-screenshots
```
