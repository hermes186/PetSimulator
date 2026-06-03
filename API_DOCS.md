# 🎮 Pet Simulator API 文档

**版本**：1.0-Release  
**更新时间**：2026-06-03  
**基础 URL**：`http://localhost:8080/api`

---

## 📋 目录

1. [基本信息](#基本信息)
2. [数据格式](#数据格式)
3. [宠物管理](#宠物管理)
4. [背包管理](#背包管理)
5. [战斗系统](#战斗系统)
6. [游戏状态](#游戏状态)
7. [错误处理](#错误处理)

---

## 基本信息

### 请求方式
- **协议**：HTTP/HTTPS
- **主机**：localhost
- **端口**：8080
- **内容类型**：`application/json`

### 响应格式

所有 API 响应都遵循以下格式：

**成功响应**：
```json
{
  "success": true,
  "message": "操作成功",
  "data": { ... }
}
```

**错误响应**：
```json
{
  "success": false,
  "error": "错误信息"
}
```

---

## 数据格式

### 宠物对象

```json
{
  "name": "FirePet",
  "type": "Fire",           // "Fire" | "Water" | "Mech"
  "hp": 90,
  "maxHp": 90,
  "attack": 18,
  "level": 1,
  "exp": 0,
  "expToNextLevel": 100,
  "alive": true
}
```

### 背包物品对象

```json
{
  "id": "potion_hp",
  "name": "生命药水",
  "icon": "🧪",
  "description": "恢复宠物50点生命值",
  "price": 100,
  "currency": "coins",      // "coins" | "gems"
  "type": "consumable",     // "consumable" | "special"
  "quantity": 1             // 仅在背包中显示
}
```

---

## 宠物管理

### 获取所有宠物

**请求**：
```
GET /api/pets
```

**响应**：
```json
[
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
  },
  ...
]
```

**HTTP 状态码**：200 OK

---

### 获取单个宠物

**请求**：
```
GET /api/pets/{id}
GET /api/pet/{id}
```

**参数**：
- `id` (integer, 必需)：宠物 ID（1-based）

**响应**：
```json
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
```

**HTTP 状态码**：200 OK，404 Not Found

---

### 创建新宠物

**请求**：
```
POST /api/pets
POST /api/pet
Content-Type: application/json

{
  "type": "Fire",
  "name": "火焰精灵"
}
```

**参数**：
- `type` (string, 必需)：宠物类型，取值 "Fire"、"Water" 或 "Mech"
- `name` (string, 必需)：宠物名称

**响应**：
```json
{
  "success": true,
  "pet": {
    "name": "火焰精灵",
    "type": "Fire",
    "hp": 90,
    "maxHp": 90,
    "attack": 18,
    "level": 1,
    "exp": 0,
    "expToNextLevel": 100,
    "alive": true
  }
}
```

**HTTP 状态码**：201 Created，400 Bad Request

**限制**：
- 最多 5 只宠物
- 类型必须有效
- 名称不能为空

---

### 删除宠物

**请求**：
```
DELETE /api/pets/{id}
DELETE /api/pet/{id}
```

**参数**：
- `id` (integer, 必需)：宠物 ID（1-based）

**响应**：
```json
{
  "success": true,
  "message": "Pet deleted"
}
```

**HTTP 状态码**：200 OK，404 Not Found

---

### 训练宠物

**请求**：
```
POST /api/pets/{id}/train
POST /api/pet/{id}/train
```

**参数**：
- `id` (integer, 必需)：宠物 ID（1-based）

**响应**：
```json
{
  "success": true,
  "pet": {
    "name": "FirePet",
    "type": "Fire",
    "hp": 90,
    "maxHp": 90,
    "attack": 20,           // 增加 2 点攻击力
    "level": 1,
    "exp": 30,              // 获得经验
    "expToNextLevel": 100,
    "alive": true
  }
}
```

**HTTP 状态码**：200 OK，404 Not Found

---

### 治疗宠物

**请求**：
```
POST /api/pets/{id}/heal
POST /api/pet/{id}/heal
Content-Type: application/json

{
  "itemIndex": 0
}
```

**参数**：
- `id` (integer, 必需)：宠物 ID（1-based）
- `itemIndex` (integer, 必需)：背包物品索引

**响应**：
```json
{
  "success": true,
  "pet": {
    "name": "FirePet",
    "type": "Fire",
    "hp": 100,              // 生命值恢复
    "maxHp": 100,
    "attack": 18,
    "level": 1,
    "exp": 0,
    "expToNextLevel": 100,
    "alive": true
  }
}
```

**HTTP 状态码**：200 OK，400 Bad Request，404 Not Found

---

### 强化宠物

**请求**：
```
POST /api/pets/{id}/enhance
POST /api/pet/{id}/enhance
Content-Type: application/json

{
  "itemIndex": 0
}
```

**参数**：
- `id` (integer, 必需)：宠物 ID（1-based）
- `itemIndex` (integer, 必需)：背包物品索引

**响应**：
```json
{
  "success": true,
  "pet": {
    "name": "FirePet",
    "type": "Fire",
    "hp": 90,
    "maxHp": 90,
    "attack": 18,
    "level": 2,             // 升级
    "exp": 0,
    "expToNextLevel": 150,
    "alive": true
  }
}
```

**HTTP 状态码**：200 OK，400 Bad Request，404 Not Found

---

## 背包管理

### 获取背包内容

**请求**：
```
GET /api/backpack
```

**响应**：
```json
{
  "success": true,
  "items": [
    {
      "id": "potion_hp",
      "name": "生命药水",
      "icon": "🧪",
      "description": "恢复宠物50点生命值",
      "price": 100,
      "currency": "coins",
      "type": "consumable",
      "quantity": 1
    }
  ]
}
```

**HTTP 状态码**：200 OK

---

### 添加物品到背包

**请求**：
```
POST /api/backpack
Content-Type: application/json

{
  "name": "生命药水",
  "type": "Potion",
  "value": 50
}
```

**参数**：
- `name` (string, 必需)：物品名称
- `type` (string, 必需)：物品类型
- `value` (integer, 必需)：物品价值

**响应**：
```json
{
  "success": true,
  "message": "Item added to backpack"
}
```

**HTTP 状态码**：200 OK，400 Bad Request

---

### 移除物品

**请求**：
```
DELETE /api/backpack/{id}
```

**参数**：
- `id` (integer, 必需)：物品索引（0-based）

**响应**：
```json
{
  "success": true,
  "message": "Item removed"
}
```

**HTTP 状态码**：200 OK，404 Not Found

---

### 排序背包

**请求**：
```
POST /api/backpack/sort
```

**响应**：
```json
{
  "success": true,
  "message": "Backpack sorted"
}
```

**HTTP 状态码**：200 OK

---

## 战斗系统

### 开始战斗

**请求**：
```
POST /api/battle
Content-Type: application/json

{
  "attacker": 0,
  "defender": 1
}
```

**参数**：
- `attacker` (integer, 必需)：攻击方宠物索引（0-based）
- `defender` (integer, 必需)：防守方宠物索引（0-based）

**响应**：
```json
{
  "success": true,
  "battle": {
    "attacker": {
      "name": "FirePet",
      "damage": 25,
      "finalHp": 65
    },
    "defender": {
      "name": "WaterPet",
      "damage": 15,
      "finalHp": 120
    },
    "winner": "FirePet",
    "log": [
      "FirePet 发动攻击！造成 25 点伤害",
      "WaterPet 发动攻击！造成 15 点伤害",
      "FirePet 战胜了 WaterPet"
    ]
  }
}
```

**HTTP 状态码**：200 OK，400 Bad Request

---

### 对比宠物

**请求**：
```
POST /api/compare
Content-Type: application/json

{
  "petA": 0,
  "petB": 1
}
```

**参数**：
- `petA` (integer, 必需)：宠物 A 索引（0-based）
- `petB` (integer, 必需)：宠物 B 索引（0-based）

**响应**：
```json
{
  "success": true,
  "comparison": {
    "petA": {
      "name": "FirePet",
      "type": "Fire",
      "stats": { "hp": 90, "attack": 18, "level": 1 }
    },
    "petB": {
      "name": "WaterPet",
      "type": "Water",
      "stats": { "hp": 135, "attack": 12, "level": 1 }
    },
    "winner": "petB"
  }
}
```

**HTTP 状态码**：200 OK，400 Bad Request

---

## 游戏状态

### 获取完整游戏状态

**请求**：
```
GET /api/state
```

**响应**：
```json
{
  "success": true,
  "state": {
    "pets": [ ... ],
    "backpack": [ ... ],
    "player": {
      "level": 1,
      "exp": 0,
      "coins": 1000,
      "gems": 50
    }
  }
}
```

**HTTP 状态码**：200 OK

---

### 保存游戏

**请求**：
```
POST /api/save
```

**响应**：
```json
{
  "success": true,
  "message": "Game saved"
}
```

**HTTP 状态码**：200 OK，500 Internal Server Error

---

## 错误处理

### 错误响应码

| 状态码 | 含义 | 示例 |
|--------|------|------|
| 200 | 请求成功 | 宠物已获取 |
| 201 | 创建成功 | 宠物已创建 |
| 400 | 请求参数错误 | 缺少必需字段 |
| 404 | 资源不存在 / 安全拦截 | 宠物 ID 不存在 / 路径遍历检测拦截 |
| 500 | 服务器内部错误 | 保存失败 / 未捕获系统错误 |

### 错误响应示例

```json
{
  "success": false,
  "error": "Pet not found"
}
```

---

## 使用示例

### 使用 curl 获取宠物列表

```bash
curl -X GET http://localhost:8080/api/pets
```

### 使用 curl 创建宠物

```bash
curl -X POST http://localhost:8080/api/pets \
  -H "Content-Type: application/json" \
  -d '{"type":"Fire","name":"火焰精灵"}'
```

### 使用 curl 删除宠物

```bash
curl -X DELETE http://localhost:8080/api/pets/1
```

### 使用 curl 开始战斗

```bash
curl -X POST http://localhost:8080/api/battle \
  -H "Content-Type: application/json" \
  -d '{"attacker":0,"defender":1}'
```

---

## 路由兼容性

所有 API 端点都支持**单数和复数形式**的路由，便于不同前端的适配：

- `GET /api/pets` = `GET /api/pets`（宠物列表）
- `GET /api/pets/1` = `GET /api/pet/1`（单只宠物）
- `DELETE /api/pets/1` = `DELETE /api/pet/1`（删除宠物）
- `POST /api/pets` = `POST /api/pet`（创建宠物）

---

## 性能指标

- **平均响应时间**：< 100ms
- **最大连接数**：100
- **请求大小限制**：1MB
- **响应大小限制**：10MB

---

## 版本历史

### v1.0-Release (2026-06-03)
- 安全加固版本发布
- 解决路径遍历 (Path Traversal) 漏洞，拦截含 `..` 访问请求
- 修复空请求头引发的 DoS 拒绝服务崩溃，提升服务稳定性
- 优化 TCP 套接字分段包缓冲接收算法，彻底攻克 POST 请求偶发性 400 失败

### v1.0-Beta (2026-05-13)
- 初始版本，完成全部核心 API 构建与文档编写

---

## 联系方式

如有问题或反馈，请联系开发团队。

---

**最后更新**：2026-06-03  
**下一个版本**：v1.1（计划功能增强）
