# Swatch Scenes

## 定位

`swatch_picker` 是颜色样本选择的历史 showcase 控件，主卡强调色板预览，底部保留紧凑态和锁定态。它适合做视觉演示，但目前没有进入统一的主线 reference 集。

## 当前示例内容

- 主卡在 `Palette A / Palette B` 两组色板之间切换，示例命名包括 `Sky / Mint / Rose / Sand / Ink / Lime` 等。
- `Compact` 预览用 `A-F` 和 `G-L` 两组简写样本测试小尺寸布局。
- `Locked` 预览保持禁用态，验证弱化后的色块边界和焦点项是否仍可辨认。
- 点击主卡或紧凑卡，会同步更新焦点样本和状态文本。

## 现状判断

- 轨道：`showcase`
- 仓库主线已经有 `input/color_picker` 作为更标准的 reference 方案。
- 当前保留它，主要是为了保留色板式选择器和缩略布局的实现样例。

## 使用建议

- 适合做主题色预设、品牌色样本或快速 palette 选择的概念验证。
- 若是正式产品输入，应优先考虑 `input/color_picker` 这类主线 reference 控件。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=input/swatch_picker PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/swatch_picker --timeout 10 --keep-screenshots
```
