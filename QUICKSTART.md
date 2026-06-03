# 🚀 Pet Simulator 快速入门指南

**版本**：1.0-Release  
**适用于**：Windows 11（支持 原生 CMD 命令行 与 WSL2 Bash 环境）

---

## 📋 目录

1. [系统需求](#系统需求)
2. [安装步骤](#安装步骤)
3. [启动游戏](#启动游戏)
4. [游戏操作](#游戏操作)
5. [常见问题](#常见问题)
6. [项目结构](#项目结构)

---

## 系统需求

### 软件要求

- **操作系统**：Windows 11 或更高版本
- **WSL2**：已安装并启用
- **编译器**：MinGW g++ (≥ 10.0)
- **浏览器**：Chrome、Edge 或 Firefox

### 硬件要求

- **处理器**：Intel i5 或同等级
- **内存**：≥ 4GB RAM
- **存储**：≥ 100MB 可用空间

---

## 安装步骤

### 第一步：下载项目（克隆仓库）

```bash
# 克隆仓库到你的本地电脑中
git clone https://github.com/hermes186/PetSimulator.git
cd PetSimulator
```

### 第二步：验证环境

```bash
# 检查 g++ 是否可用
g++ --version

# 检查是否有需要的库
ls src/
```

### 第三步：编译后端

```bash
# 执行编译脚本
./compile.bat

# 或者手动编译
g++ -std=c++11 -I./src -o PetSimulatorServer.exe src/main_server.cpp src/BattleResultCalculator.cpp -lws2_32
```

**成功标志**：
```
✅ PetSimulatorServer.exe 已生成（大小约 800KB）
```

### 第四步：验证前端文件

```bash
# 检查前端文件是否存在
ls web/

# 应该看到以下文件：
# ├── index.html
# ├── app.js
# └── style.css
```

---

## 启动游戏

### 方法 1：使用启动脚本（推荐）

```bash
# 在项目根目录执行
./start_server.bat
```

**输出示例**：
```
🎮 Pet Simulator Server
📍 Listen on: 0.0.0.0:8080
✅ Server started successfully
```

### 方法 2：手动启动

```bash
# 在 WSL2 bash 中执行
cd c:/Users/Lenovo/WorkBuddy/2026-05-07-task-1/PetSimulator
./PetSimulatorServer.exe
```

### 第二步：打开浏览器

```
地址栏输入：http://localhost:8080/web/index.html

或者访问：http://localhost:8080/

系统会自动重定向到前端页面
```

### 验证启动成功

✅ 看到以下界面说明启动成功：
- 顶部显示"🎮 Pet Simulator"标题
- 左侧显示导航菜单
- 宠物列表显示初始 4 只宠物
- 右上角显示玩家资源（金币、钻石）

---

## 游戏操作

### 🏠 我的宠物

**功能**：查看和管理你的宠物

**操作**：
1. 点击左侧"🏠 我的宠物"菜单
2. 查看宠物列表和统计数据
3. 使用筛选按钮过滤宠物类型：
   - "全部" - 显示所有宠物
   - "🔥" - 只显示火焰系
   - "💧" - 只显示水系
   - "⚙️" - 只显示机械系
4. 点击宠物卡片查看详情
5. 在详情窗口中可以选择"⚔️ 战斗"或"🗑️ 放生"

---

### ✨ 召唤宠物

**功能**：获取新宠物

**操作**：
1. 点击左侧"✨ 召唤宠物"菜单
2. 选择召唤阵类型：
   - 🔥 火焰召唤阵（200 金币）
   - 💧 水系召唤阵（200 金币）
   - ⚙️ 机械召唤阵（200 金币）
3. 系统随机生成宠物名字和属性
4. 金币自动扣除，宠物加入队伍

**限制**：最多 5 只宠物

---

### ⚔️ 竞技场

**功能**：进行宠物战斗

**操作**：
1. 点击左侧"⚔️ 竞技场"菜单
2. 从列表中选择一只宠物作为"我方"
3. 点击"⚔️ 开始战斗"按钮
4. 系统自动进行战斗计算
5. 查看战斗日志和结果
6. 赢取经验值和奖励

**战斗奖励**：
- 胜利：获得经验值和金币
- 失败：宠物受伤，需要使用药水治疗

---

### 🎒 背包

**功能**：管理道具和装备

**操作**：
1. 点击左侧"🎒 背包"菜单
2. 查看拥有的所有道具
3. 道具可以用来：
   - 治疗宠物（生命药水）
   - 强化宠物（增强石）
   - 其他效果（根据道具类型）
4. 点击"使用"按钮使用道具

---

### 🛒 商城

**功能**：购买道具和装备

**可购买物品**：
| 物品 | 价格 | 货币 | 效果 |
|------|------|------|------|
| 🧪 生命药水 | 100 | 金币 | 恢复 50 生命值 |
| ⚔️ 攻击药水 | 150 | 金币 | 临时 +20 攻击力 |
| ⭐ 经验加倍卡 | 30 | 钻石 | 下次战斗双倍经验 |
| 🍖 宠物粮食 | 200 | 金币 | +5 全属性 |
| 📜 召唤卷轴 | 50 | 钻石 | 免费召唤一次 |
| 💎 复活石 | 100 | 钻石 | 复活失败宠物 |

**操作**：
1. 点击左侧"🛒 商城"菜单
2. 浏览商品
3. 点击"购买"按钮
4. 自动扣除相应货币
5. 道具自动加入背包

---

## 常见问题

### Q1：无法访问 http://localhost:8080

**原因**：后端服务器未启动或端口被占用

**解决**：
```bash
# 检查是否有进程占用 8080 端口
netstat -ano | grep 8080

# 如果有占用，杀死进程
taskkill /PID <PID> /F

# 重新启动服务器
./start_server.bat
```

### Q2：编译失败，错误为"未找到头文件"

**原因**：编译器无法找到 ./src 目录中的头文件

**解决**：
```bash
# 检查文件是否存在
ls src/

# 确保在正确的目录
pwd

# 尝试手动指定路径
g++ -std=c++11 -I/full/path/to/src -o PetSimulatorServer.exe src/main_server.cpp -lws2_32
```

### Q3：删除宠物后无法删除其他宠物

**原因**：可能是宠物 ID 映射问题（已在第四阶段修复）

**解决**：
```bash
# 重新编译后端
./compile.bat

# 重启服务器
./start_server.bat

# 刷新浏览器
```

### Q4：页面显示"离线模式"

**原因**：正常现象，说明 `/api/status` 端点未实现

**解决**：无需处理，所有功能都可正常使用

### Q5：创建宠物后数据消失

**原因**：服务器未正确保存数据

**解决**：
```bash
# 检查 save_data.json 是否存在
ls save_data.json

# 如果不存在，手动创建
touch save_data.json

# 重启服务器
```

---

## 项目结构

```
PetSimulator/
├── src/                          # C++ 后端源代码
│   ├── main_server.cpp          # 服务器入口
│   ├── HttpServer.h             # HTTP 服务器实现
│   ├── ApiController.h          # API 路由处理
│   ├── GameState.h              # 游戏状态管理
│   ├── Pet.h                    # 宠物基类
│   ├── FirePet.h, WaterPet.h, MechPet.h  # 宠物子类
│   ├── BattleSystem.h           # 战斗系统
│   ├── Backpack.h               # 背包系统
│   ├── Exceptions.h             # 异常定义
│   └── Json.h                   # JSON 工具类
├── web/                          # 前端代码
│   ├── index.html               # 页面结构
│   ├── app.js                   # 游戏逻辑
│   └── style.css                # 页面样式
├── compile.bat                  # 编译脚本
├── start_server.bat             # 启动脚本
├── PetSimulatorServer.exe       # 编译后的可执行文件
├── save_data.json               # 游戏存档
├── API_DOCS.md                  # API 文档
├── QUICKSTART.md                # 本文件
└── README.md                    # 项目说明
```

---

## 性能建议

### 优化性能

1. **清理旧编译产物**：
```bash
# 删除旧的可执行文件
rm -f PetSimulatorServer.exe

# 重新编译
./compile.bat
```

2. **优化网络连接**：
- 使用有线连接而非 WiFi
- 确保本地网络延迟 < 50ms

3. **优化浏览器**：
- 关闭不必要的浏览器标签页
- 禁用浏览器扩展
- 使用最新版本的浏览器

---

## 快速命令参考

| 命令 | 功能 |
|------|------|
| `./compile.bat` | 编译后端 |
| `./start_server.bat` | 启动服务器 |
| `netstat -ano \| grep 8080` | 检查端口占用 |
| `ps aux \| grep PetSimulator` | 查看进程 |
| `kill <PID>` | 杀死进程 |
| `curl http://localhost:8080/api/pets` | 测试 API |

---

## 下一步

### 学习资源

1. **API 文档**：查看 [API_DOCS.md](API_DOCS.md)
2. **项目结构**：查看下面的"项目结构说明"部分
3. **开发指南**：联系开发团队获取

### 修改游戏

1. **改变宠物属性**：编辑 `src/Pet.h`
2. **调整游戏平衡**：编辑 `src/GameState.h`
3. **修改 UI 样式**：编辑 `web/style.css`
4. **添加新功能**：编辑 `web/app.js` 和对应的后端代码

---

## 支持

### 获取帮助

- 📧 邮件：support@example.com
- 💬 讨论：GitHub Issues
- 📖 文档：项目 wiki

### 报告问题

请提交以下信息：
1. 问题描述
2. 操作系统版本
3. 浏览器类型和版本
4. 错误信息或截图
5. 复现步骤

---

**版本**：1.0-Release  
**最后更新**：2026-06-03  
**维护者**：Claude
