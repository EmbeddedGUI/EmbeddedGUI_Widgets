# Notification Stack

## 定位

`notification_stack` 是历史反馈类 showcase 控件，用来展示多条通知卡片、焦点卡片和紧凑态通知栈。它保留了较强的展示感，但没有被纳入当前 Fluent 主线 reference 集。

## 当前示例内容

- 主卡在 `Ops A / Ops B` 两组快照之间切换，示例通知包括 `Build delay`、`Cache drift`、`Deploy gate`、`Alert storm` 等。
- `Compact` 预览提供 `Ping / Auth / Mail` 和 `Web / DB / Ops` 两组缩略通知。
- `Locked` 预览保留只读样式，检验通知栈在禁用态下的层级与标签清晰度。
- 点击主卡或紧凑区块，会同步切换状态文本和聚焦卡片。

## 现状判断

- 轨道：`showcase`
- 当前仓库主线更倾向 `message_bar`、`toast_stack` 这类更标准化的反馈模式。
- 该控件仍适合作为多卡堆叠和状态层级的视觉实验样例。

## 使用建议

- 适合运维告警墙、设备异常摘要、时间线式提醒面板的原型阶段。
- 不建议直接作为统一反馈控件继续扩展。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=feedback/notification_stack PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub feedback/notification_stack --timeout 10 --keep-screenshots
```
