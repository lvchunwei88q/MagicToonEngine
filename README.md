# MagicToonEngine

使用C++ 编写的一个简单的卡通渲染验证引擎

## 🔧 构建指南

### 环境要求

| 组件 | 版本要求 | 下载链接 |
|------|----------|----------|
| **Visual Studio** | 2022 或 2026 | [下载](https://visualstudio.microsoft.com/) |
| **CMake** | 3.20 或更高 | [下载](https://cmake.org/download/) |
| **Git** | 最新版本 | [下载](https://git-scm.com/) |
| **Git LFS** | 最新版本 | [下载](https://git-lfs.github.com/) |

> ⚠️ **重要**：Git LFS 是必需的，否则无法获取 ThirdParty 文件夹中的大文件。

### 获取源码

稍后下载LFS:
```bash
# 克隆仓库（推荐使用快速克隆，跳过 LFS 自动下载）
git clone --config lfs.skipSmudge=true https://github.com/lvchunwei88q/MagicToonEngine.git
cd MagicToonEngine

# 运行 setup 脚本下载 LFS 大文件
setup.bat
```
直接下载LFS:
```bash
git clone https://github.com/lvchunwei88q/MagicToonEngine.git
```
### 构建
在构建之前，请确保已安装 Visual Studio 和 CMake，并且已正确设置环境变量。
```bash
cd MagicToonEngine
# 运行 Build 脚本
Build.bat
```
选择Debug或Release版本进行构建，
之后选择使用Ninja或MSBuild生成构建文件，最后Build脚本会自动构建整个项目。

### 生成项目文件
```bash
cd MagicToonEngine
# 运行 GenerateVSProject 脚本
GenerateVSProject.bat
```
之后请在out/VS中打开MagicToonEngine.sln文件，即可查看和编辑项目。