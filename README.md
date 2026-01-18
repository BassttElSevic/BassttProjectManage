# 👁️ 任务管理器 - Task Manager

一个功能丰富、界面现代化的Windows桌面任务管理应用程序，采用原生C语言和Win32 API开发。

## ✨ 特色功能

### 📋 任务管理核心功能
- **📅 每日任务管理** - 基于日历的每日待办事项管理
- **📆 中期任务规划** - 中期项目和目标管理
- **🎯 长期目标追踪** - 长远规划和目标设定
- **⭐ 优先级标记** - 重要任务标识
- **🔥 紧急状态提醒** - 紧急任务高亮显示
- **⚠️ 逾期任务提醒** - 自动检测并显示过期未完成任务

### 🎨 现代化UI设计
- **💎 Material Design风格** - 清新的蓝白配色方案
- **🎪 圆角卡片布局** - 左右分栏的卡片式设计
- **✨ 平滑动画效果** - 按钮点击动画和悬停效果
- **🖼️ 高清图标支持** - 256x256自定义眼睛图标
- **🎯 高DPI适配** - 支持高分辨率显示器
- **🌈 交替行颜色** - ListView美化和自定义选中状态

### 📊 智能交互
- **📅 可视化日历** - 点击日期查看对应任务
- **📝 实时任务编辑** - 即时添加和删除任务
- **💾 自动数据持久化** - 任务数据自动保存到本地文件
- **🔍 分类视图** - 今日待办和长期规划分离显示

## 🚀 技术特点

- **🔧 纯C语言开发** - 使用C11标准，性能优异
- **🏗️ Win32原生API** - 直接调用Windows API，响应迅速
- **🎯 现代化字体** - 微软雅黑字体，清晰易读
- **💻 CMake构建系统** - 跨平台构建支持
- **📦 资源文件嵌入** - 图标和版本信息直接编译到exe中
- **🎪 主题样式支持** - 使用Windows Explorer样式

## 📸 界面预览

应用程序采用双栏布局设计：
- **左侧面板**：日历选择器 + 任务输入区域
- **右侧面板**：今日待办 + 长期规划任务列表

### 🎨 配色方案
- 主背景：淡灰蓝 (#F5F7FB)
- 卡片背景：纯白 (#FFFFFF) 
- 主色调：明亮蓝 (#3B82F6)
- 悬停色：深蓝 (#2563EB)
- 危险色：红色 (#EF4444)

## 🛠️ 构建和运行

### 系统要求
- Windows 10/11 或更高版本
- Visual Studio 2022 或 MinGW-w64
- CMake 3.20 或更高版本

### 编译步骤

1. **克隆项目**
   ```bash
   git clone https://github.com/BassttElSevic/BassttProjectManage.git
   cd BassttProjectManage
   ```

2. **使用CMake构建**
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

3. **或使用已配置的构建目录**
   ```bash
   cmake --build cmake-build-debug --config Debug
   ```

### 运行程序
```bash
cd cmake-build-debug
./untitled.exe
```

## 📁 项目结构

```
├── main.c              # 主程序源代码
├── resource.rc         # Windows资源文件
├── CMakeLists.txt      # CMake构建配置
├── EYE_256x256.ico     # 高清应用图标
├── EYE_128x128.ico     # 中等尺寸图标
├── EYE_48x48.ico       # 小尺寸图标
└── README.md           # 项目说明文档
```

## 💾 数据存储

- **存储位置**：程序运行目录下的 `tasks.dat` 文件
- **存储格式**：二进制文件，包含任务数量和任务详情
- **自动保存**：添加或删除任务时自动保存
- **数据结构**：
  ```c
  typedef struct {
      TCHAR description[256];  // 任务描述
      SYSTEMTIME date;         // 任务日期
      bool is_important;       // 是否重要
      bool is_urgent;          // 是否紧急
      TaskType type;          // 任务类型
  } Task;
  ```

## 🎯 使用说明

### 添加任务
1. 在左侧日历中选择日期
2. 输入任务描述
3. 选择任务类型（每日/中期/长期）
4. 标记优先级（重要/紧急）
5. 点击"+ 添加任务"按钮

### 管理任务
- **查看任务**：点击日历日期查看对应的每日任务
- **删除任务**：选中任务后点击"✕ 删除"按钮
- **逾期提醒**：过期未完成的任务会自动在今日视图显示

### 任务分类
- **📅 每日任务**：显示在"今日待办"区域，支持日期筛选
- **📆 中期任务**：显示在"规划与目标"区域
- **🎯 长期目标**：显示在"规划与目标"区域

## 🎨 界面特性

### 动画效果
- **按钮点击动画**：点击时的缩放反馈效果
- **悬停状态变化**：鼠标悬停时的颜色渐变
- **平滑过渡**：所有UI状态变化都有平滑动画

### 视觉优化
- **圆角设计**：所有按钮和卡片都采用圆角设计
- **阴影效果**：卡片具有轻微阴影提升层次感
- **高清图标**：256x256分辨率图标，支持高DPI显示
- **现代字体**：全程使用微软雅黑字体

## 🔧 技术实现

### 核心技术栈
- **UI框架**：Win32 API + Common Controls
- **图形绘制**：GDI/GDI+ 自定义绘制
- **主题支持**：UxTheme API
- **动画系统**：Windows定时器驱动
- **字体渲染**：ClearType抗锯齿

### 关键特性实现
- **自定义按钮绘制**：使用窗口子类化技术
- **ListView美化**：自定义绘制实现现代化外观
- **资源嵌入**：图标直接编译到可执行文件中
- **内存管理**：严格的资源释放和内存管理

## 📝 更新日志

### v1.0.0 (2026-01-18)
- ✨ 初始版本发布
- 🎨 现代化UI设计实现
- 📅 完整的任务管理功能
- 🎪 动画效果和交互优化
- 🖼️ 高清图标集成
- 💾 数据持久化功能

## 👨‍💻 开发者

**BassttElSevic**
- GitHub: [@BassttElSevic](https://github.com/BassttElSevic)
- 项目仓库: [BassttProjectManage](https://github.com/BassttElSevic/BassttProjectManage)

## 📄 许可证

Copyright © 2026 BassttElSevic. All rights reserved.

## 🤝 贡献指南

欢迎提交Issue和Pull Request！

1. Fork 这个项目
2. 创建你的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交你的修改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开一个 Pull Request

## 🔮 未来计划

- [ ] 🌙 深色主题支持
- [ ] 📱 多语言界面支持
- [ ] ☁️ 云同步功能
- [ ] 📊 统计图表功能
- [ ] 🔔 系统通知提醒
- [ ] 📋 任务模板功能
- [ ] 🏷️ 任务标签系统
- [ ] 📤 导入/导出功能

---

**用现代化的设计，打造高效的任务管理体验！** ✨
