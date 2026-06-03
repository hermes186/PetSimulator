# 📁 PetSimulator 项目结构说明

**版本**：1.0-Release  
**更新时间**：2026-06-03

---

## 目录结构

```
PetSimulator/
├── src/                          # 后端 C++ 源代码（核心逻辑）
│   ├── main_server.cpp          # 服务器入口点
│   ├── HttpServer.h             # HTTP 服务器实现
│   ├── ApiController.h          # REST API 路由处理
│   ├── GameState.h              # 游戏状态和数据管理
│   ├── Pet.h                    # 宠物基类（抽象类）
│   ├── FirePet.h                # 火焰系宠物
│   ├── WaterPet.h               # 水系宠物
│   ├── MechPet.h                # 机械系宠物
│   ├── BattleSystem.h           # 战斗系统逻辑
│   ├── Backpack.h               # 背包系统
│   ├── Item.h                   # 物品基类
│   ├── Exceptions.h             # 自定义异常
│   └── Json.h                   # JSON 处理工具
│
├── web/                          # 前端 HTML/CSS/JavaScript（用户界面）
│   ├── index.html               # 页面主体结构（HTML5）
│   ├── app.js                   # 游戏逻辑和 UI 交互
│   └── style.css                # 页面样式和主题
│
├── compile.bat                  # 批处理脚本：编译后端
├── start_server.bat             # 批处理脚本：启动服务器
├── PetSimulatorServer.exe       # 编译后的可执行文件
├── save_data.json               # 游戏存档（JSON 格式）
│
├── 📄 README.md                 # 项目总体说明
├── 📄 API_DOCS.md               # REST API 文档
├── 📄 QUICKSTART.md             # 快速入门指南
├── 📄 PROJECT_STRUCTURE.md      # 本文件
│
└── 文档/                         # 测试报告和文档
    ├── 第三阶段审计报告.md       # API 格式一致性审计
    ├── 第四阶段功能测试报告.md   # 功能测试结果
    ├── 第四阶段测试总结.md       # 第四阶段总结
    ├── 项目进度报告.md          # 项目进度跟踪
    └── 整理计划.md              # 分阶段工作计划
```

---

## 各目录说明

### src/ 后端源代码

后端采用 C++ 编写，使用现代 C++11 特性。

#### 核心文件

| 文件 | 说明 | 关键类/函数 |
|------|------|-----------|
| `main_server.cpp` | 服务器启动入口 | `main()` |
| `HttpServer.h` | HTTP 服务器框架 | `HttpServer`, `HttpRequest`, `HttpResponse` |
| `ApiController.h` | REST API 路由处理 | `ApiController`, 路由注册 |
| `GameState.h` | 游戏状态和数据持久化 | `GameState`, 宠物管理、保存/加载 |
| `Pet.h` | 宠物基类（多态） | `Pet` (虚函数) |
| `FirePet.h` | 火焰系宠物类 | `FirePet` |
| `WaterPet.h` | 水系宠物类 | `WaterPet` |
| `MechPet.h` | 机械系宠物类 | `MechPet` |
| `BattleSystem.h` | 战斗计算引擎 | `BattleSystem`, 伤害计算、暴击等 |
| `Backpack.h` | 背包和物品管理 | `Backpack`, `Item` |
| `Exceptions.h` | 自定义异常 | `GameException`, `InvalidPetException` |
| `Json.h` | JSON 序列化工具 | `json::quote()`, JSON 辅助函数 |

#### 架构设计

```
HttpServer（HTTP 框架）
    ↓
ApiController（路由处理）
    ↓
GameState（业务逻辑）
    ├── Pets（宠物管理）
    │   └── Pet（基类）
    │       ├── FirePet
    │       ├── WaterPet
    │       └── MechPet
    ├── BattleSystem（战斗系统）
    └── Backpack（背包系统）
        └── Item（物品）
```

---

### web/ 前端代码

前端采用原生 HTML5、CSS3 和 JavaScript（无框架）。

#### 核心文件

| 文件 | 说明 | 主要内容 |
|------|------|---------|
| `index.html` | 页面结构 | DOM 元素、导航菜单、5 个面板 |
| `app.js` | 游戏逻辑 | 事件处理、API 调用、状态管理 |
| `style.css` | 页面样式 | 深色主题、响应式布局、动画 |

#### 页面结构（5 个主要面板）

```
Header（顶部栏）
├── Logo 和标题
├── 玩家信息（等级、经验）
└── 资源显示（金币、钻石）

Sidebar（左侧导航）
├── 🏠 我的宠物
├── ✨ 召唤宠物
├── ⚔️ 竞技场
├── 🎒 背包
└── 🛒 商城

Main Content（主内容区 - 5 个面板）
├── Panel 1: 我的宠物
│   ├── 宠物统计
│   ├── 筛选按钮
│   └── 宠物卡片网格
├── Panel 2: 召唤宠物
│   ├── 召唤阵选择
│   └── 召唤结果显示
├── Panel 3: 竞技场
│   ├── 宠物选择
│   └── 战斗日志
├── Panel 4: 背包
│   └── 物品列表
└── Panel 5: 商城
    └── 商品列表

Modal（弹窗）
├── 宠物详情弹窗
├── 战斗结果弹窗
└── 购买确认弹窗
```

#### JavaScript 功能模块

| 功能 | 函数 | 说明 |
|------|------|------|
| API 连接 | `checkApiConnection()` | 检查后端连接状态 |
| 数据加载 | `loadPetsFromApi()` | 从后端加载宠物数据 |
| 宠物管理 | `deletePet()`, `createPet()` | 删除/创建宠物 |
| 战斗系统 | `battleWithPet()` | 开始战斗 |
| 背包系统 | `buyItem()`, `useItem()` | 购买/使用物品 |
| UI 更新 | `renderPets()`, `updateUI()` | 渲染 UI 组件 |
| 状态管理 | 全局变量 `pets`, `gameState` | 游戏状态 |

---

## 数据流

### 创建宠物流程

```
前端界面
  ↓
用户点击"召唤宠物"
  ↓
POST /api/pets
  ↓
ApiController::create_pet()
  ↓
GameState::create_pet()
  ↓
创建 Pet 对象
  ↓
JSON 序列化
  ↓
返回 JSON 响应
  ↓
前端更新 UI
  ↓
显示新宠物
```

### 删除宠物流程

```
前端界面
  ↓
用户点击"放生"
  ↓
DELETE /api/pet/{id}
  ↓
ApiController::delete_pet()
  ↓
GameState::delete_pet()
  ↓
从宠物数组删除
  ↓
auto_save()
  ↓
返回成功响应
  ↓
前端更新 UI
  ↓
列表刷新
```

---

## 数据存储

### JSON 格式

**save_data.json 结构**：
```json
{
  "pets": [
    {
      "name": "FirePet",
      "type": "Fire",
      "hp": 90,
      "maxHp": 90,
      "attack": 18,
      "level": 1,
      "exp": 0,
      "expToNextLevel": 100,
      "alive": true
    }
  ],
  "backpack": [
    {
      "id": "potion_hp",
      "name": "生命药水",
      "quantity": 1
    }
  ],
  "gameState": {
    "playerLevel": 1,
    "coins": 1000,
    "gems": 50
  }
}
```

---

## 类设计

### Pet 类（宠物基类）

```cpp
class Pet {
private:
    std::string m_name;
    int m_hp;
    int m_maxHp;
    int m_attack;
    int m_level;
    int m_exp;
    int m_expToNextLevel;
    bool m_alive;

public:
    virtual std::string to_json() const;      // JSON 序列化
    virtual std::string get_type() const = 0; // 虚函数：获取类型
    // ... 其他方法
};
```

### 宠物子类

```cpp
class FirePet : public Pet {
public:
    std::string get_type() const override { return "Fire"; }
};

class WaterPet : public Pet {
public:
    std::string get_type() const override { return "Water"; }
};

class MechPet : public Pet {
public:
    std::string get_type() const override { return "Mech"; }
};
```

---

## 文件大小统计

| 文件 | 大小 | 说明 |
|------|------|------|
| `PetSimulatorServer.exe` | ~800KB | 编译后的可执行文件 |
| `src/` | ~50KB | 所有 C++ 源文件 |
| `web/` | ~200KB | HTML、CSS、JS 文件 |
| `save_data.json` | ~1KB | 游戏存档 |
| **总计** | ~1MB | 整个项目 |

---

## 编译和链接

### 编译命令

```bash
g++ -std=c++11 -I./src -o PetSimulatorServer.exe src/main_server.cpp src/BattleResultCalculator.cpp -lws2_32
```

### 编译参数说明

| 参数 | 说明 |
|------|------|
| `-std=c++11` | 使用 C++11 标准 |
| `-I./src` | 头文件搜索路径 |
| `-o PetSimulatorServer.exe` | 输出文件名 |
| `-lws2_32` | 链接 Windows Socket 库 |

### 链接库

- **ws2_32** - Windows Socket API（网络通信）
- **stdlibc++** - C++ 标准库（自动链接）

---

## 依赖关系

### 后端依赖

```
main_server.cpp
    ├── HttpServer.h（HTTP 框架）
    ├── ApiController.h（API 路由）
    │   └── GameState.h（业务逻辑）
    │       ├── Pet.h（宠物基类）
    │       │   ├── FirePet.h
    │       │   ├── WaterPet.h
    │       │   └── MechPet.h
    │       ├── BattleSystem.h（战斗系统）
    │       └── Backpack.h（背包系统）
    ├── Exceptions.h（异常）
    └── Json.h（JSON 工具）
```

### 前端依赖

```
index.html
    ├── app.js（游戏逻辑）
    │   └── HTTP API (后端服务)
    └── style.css（样式）
```

---

## 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.0 | 2026-05-13 | 初始版本，所有功能完成 |

---

## 性能指标

### 后端性能

- **编译时间**：< 2 秒
- **启动时间**：< 1 秒
- **平均响应时间**：< 100ms
- **最大并发连接**：100

### 前端性能

- **页面加载时间**：< 2 秒
- **API 调用响应**：< 500ms
- **DOM 渲染**：< 1 秒
- **浏览器兼容性**：Chrome, Edge, Firefox

---

## 扩展指南

### 添加新的宠物类型

1. 在 `src/` 中创建新文件 `NewPet.h`
2. 继承 `Pet` 类
3. 实现 `get_type()` 方法
4. 在 `GameState::create_pet()` 中添加条件分支
5. 在 `ApiController::register_routes()` 中注册新路由

### 添加新的物品类型

1. 在 `src/Backpack.h` 中定义新物品
2. 在 `web/app.js` 中添加物品效果
3. 在商城菜单中注册新物品

### 添加新的 API 端点

1. 在 `ApiController.h` 中实现新方法
2. 在 `register_routes()` 中注册路由
3. 在 `web/app.js` 中调用新 API
4. 在 `API_DOCS.md` 中文档化

---

## 常见修改点

### 调整游戏参数

| 参数 | 位置 | 说明 |
|------|------|------|
| 初始宠物数 | `GameState.h` | 构造函数 |
| 最大宠物数 | `ApiController.h` | `create_pet()` |
| 召唤费用 | `web/app.js` | `summonPet()` |
| 战斗伤害 | `BattleSystem.h` | `calculate_damage()` |
| 物品价格 | `web/app.js` | `shopItems` 数组 |
| 初始资源 | `web/app.js` | `gameState` 对象 |

---

## 测试覆盖

### 已测试的功能

- ✅ 后端编译和启动
- ✅ 前端页面加载
- ✅ 宠物列表获取
- ✅ 宠物创建和删除
- ✅ 数据持久化
- ✅ 战斗系统
- ✅ 背包系统
- ✅ 商城购买
- ✅ 筛选功能

### 测试工具

- Playwright - 浏览器自动化测试
- curl - API 测试
- g++ - 编译验证

---

## 维护建议

### 定期任务

1. **备份存档**：定期备份 `save_data.json`
2. **更新文档**：功能更新时同步文档
3. **检查日志**：监控服务器错误日志
4. **性能监测**：定期检查响应时间

### 故障排查

1. 检查 `PetSimulatorServer.exe` 是否运行
2. 检查端口 8080 是否被占用
3. 检查 `save_data.json` 是否存在且有效
4. 查看浏览器控制台是否有错误

---

## 相关文档

- [API_DOCS.md](API_DOCS.md) - REST API 详细文档
- [QUICKSTART.md](QUICKSTART.md) - 快速入门指南
- [README.md](README.md) - 项目总体说明

---

**版本**：1.0-Release  
**最后更新**：2026-06-03  
**维护者**：Claude
