# Anywhere Translate

基于 Qt6 的 Windows 划词翻译工具。

## 功能

- **Ctrl+C+C 快捷翻译**：选中文字后，复制（`Ctrl+C`）再快速按一次 `C`，翻译气泡即刻浮现在光标附近
- **剪贴板监听模式**（可选）：开启后，每次复制自动翻译，结果显示在主窗口
- 支持三种翻译引擎：**LibreTranslate**、**AI（OpenAI 兼容接口）**、**腾讯云机器翻译**
- 托盘常驻后台，关闭窗口不退出程序

## 依赖

| 工具 | 版本要求 |
|------|----------|
| Windows | 10 / 11 |
| Qt6 | 6.x（含 Widgets、Network 模块） |
| CMake | 3.16+ |
| MinGW-w64 | 随 Qt 安装包附带即可 |

> Qt 官方安装包：https://www.qt.io/download-qt-installer

## 构建

```bash
cmake -S . -B build -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH="<你的Qt路径>/mingw_64" ^
    -DCMAKE_C_COMPILER="<你的MinGW路径>/gcc.exe" ^
    -DCMAKE_CXX_COMPILER="<你的MinGW路径>/g++.exe"

cmake --build build --parallel
```

构建产物：`build\AnywhereTranslate.exe`

> 也可以直接用 Qt Creator 打开 `CMakeLists.txt` 构建。

> 或者使用批处理脚本，不过需要你提前设置路径。
```
set QT_DIR=<你的Qt路径>
set MINGW_DIR=<你的MinGW路径>
set BUILD_DIR=build
```

## 使用方法

### Ctrl+C+C 翻译（推荐）

1. 在任意应用里划词并复制（`Ctrl+C`）
2. 600ms 内再按一次 `C`（可按住 Ctrl 不放，即 Ctrl+C+C）
3. 翻译气泡出现在光标附近，6 秒后自动关闭；点击气泡立即关闭

### 剪贴板监听模式

主界面或托盘菜单中勾选"监听剪贴板"，之后每次复制都会自动翻译，结果更新到主窗口（默认关闭）。

### 配置翻译引擎

点击"设置"按钮填入对应接口信息：

#### LibreTranslate
- 接口地址（默认 `https://libretranslate.com/translate`）
- API Key（公共实例可留空）

#### AI 翻译（OpenAI 兼容）
- 接口地址（默认 `https://api.openai.com/v1/chat/completions`）
- API Key
- 模型（默认 `gpt-4o-mini`）

#### 腾讯云机器翻译
1. 在腾讯云控制台开通机器翻译（TMT）服务
2. 创建 SecretId / SecretKey
3. 在设置中填入 SecretId、SecretKey、Region（如 `ap-guangzhou`）、ProjectId（默认项目填 `0`）

> 签名方式：TC3-HMAC-SHA256 · 接口：TextTranslate · 版本：2018-03-21

## 语言代码

| 含义 | 代码 |
|------|------|
| 自动检测 | `auto` |
| 中文 | `zh` |
| 英文 | `en` |
| 日文 | `ja` |
| 韩文 | `ko` |
| 法文 | `fr` |
| 德文 | `de` |

## 安全说明

所有 API 密钥通过 `QSettings` 保存在 Windows 注册表（`HKCU\Software\AnywhereTranslate`），不写入任何文件，不会被意外提交到代码仓库。
