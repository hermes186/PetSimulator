/**
 * Pet Simulator - 桌面端网页游戏
 * 严格API对接版：所有数据操作通过后端API
 */

// API配置
const API_BASE = 'http://localhost:8080/api';

// 带超时的fetch（防止代理/网络问题导致请求挂死）
async function fetchWithTimeout(url, options = {}, timeout = 5000) {
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), timeout);
    try {
        const response = await fetch(url, { ...options, signal: controller.signal });
        return response;
    } finally {
        clearTimeout(timer);
    }
}

// 全局状态
let pets = [];
let currentPetId = null;
let itemToUse = null;
let currentFilter = 'all';
let pendingItem = null;
let isApiConnected = false;

// 类型转换函数：将前端小写类型转换为服务器首字母大写格式
function capitalizeType(type) {
    const typeMap = {
        'fire': 'Fire',
        'water': 'Water',
        'mech': 'Mech',
        'grass': 'Grass',
        'electric': 'Electric',
        'rock': 'Rock',
        'wind': 'Wind',
        'ice': 'Ice'
    };
    return typeMap[type] || type;
}

// 类型转换函数：将服务器首字母大写类型转换为前端小写格式
function decapitalizeType(type) {
    const typeMap = {
        'Fire': 'fire',
        'Water': 'water',
        'Mech': 'mech',
        'Grass': 'grass',
        'Electric': 'electric',
        'Rock': 'rock',
        'Wind': 'wind',
        'Ice': 'ice'
    };
    return typeMap[type] || type.toLowerCase();
}

// 主要属性相克关系（循环）: 水克火, 火克草, 草克电, 电克水
const TYPE_ADVANTAGE = {
    'fire': 'grass',   // 火克草
    'grass': 'electric', // 草克电
    'electric': 'water', // 电克水
    'water': 'fire'     // 水克火
};

// 检查属性是否克制
function doesTypeBeat(attackerType, defenderType) {
    return TYPE_ADVANTAGE[attackerType] === defenderType;
}

// 获取属性克制关系描述
function getTypeRelation(attackerType, defenderType) {
    if (doesTypeBeat(attackerType, defenderType)) {
        return 'advantage';
    } else if (doesTypeBeat(defenderType, attackerType)) {
        return 'disadvantage';
    } else {
        return 'neutral';
    }
}

// 根据属性获取克制关系颜色
function getTypeRelationColor(relation) {
    switch (relation) {
        case 'advantage': return 'var(--success)';
        case 'disadvantage': return 'var(--danger)';
        default: return 'var(--text-muted)';
    }
}

// 本地游戏状态（后端不存储游戏机制数据）
const gameState = {
    coins: 1000,
    gems: 50,
    level: 1,
    exp: 0,
    expToNext: 100
};

// 当前选中的怪物信息
let currentMonster = null;

// 用于从宠物详情发起战斗时保存宠物ID
let battleFromPetDetail = null;

// ============================================
// 初始化
// ============================================
document.addEventListener('DOMContentLoaded', () => {
    console.log('[Pet Simulator] 初始化中...');
    initApp();
});

async function initApp() {
    try {
        updateUI();
        await checkApiConnection();
        await loadPetsFromApi();
        bindEvents();
        
        // 初始化商城和背包
        renderShop();
        renderInventory();
        
        switchPanel('pets');
        console.log('[Pet Simulator] 初始化完成');
        showToast(isApiConnected ? '✅ 已连接到服务器' : '🔌 离线模式', isApiConnected ? 'success' : 'info');
    } catch (error) {
        console.error('[Pet Simulator] 初始化失败:', error);
        showToast('初始化失败，请刷新页面', 'error');
    }
}

// ============================================
// API连接管理
// ============================================
async function checkApiConnection() {
    const statusDot = document.querySelector('#serverStatus .status-indicator');
    const statusText = document.querySelector('#serverStatus .status-text');

    if (!statusDot || !statusText) return;

    try {
        const response = await fetchWithTimeout(`${API_BASE}/pets`);
        if (response.ok) {
            isApiConnected = true;
            statusDot.classList.add('online');
            statusText.textContent = '已连接';
            return true;
        }
        throw new Error('API响应异常');
    } catch (error) {
        console.warn('[Pet Simulator] 服务器连接失败:', error);
        isApiConnected = false;
        statusDot.classList.remove('online');
        statusText.textContent = '离线模式';
        return false;
    }
}

// ============================================
// API数据加载（真实API → 映射前端字段）
// ============================================
async function loadPetsFromApi() {
    const petsGrid = document.getElementById('petsGrid');
    if (!petsGrid) return;

    petsGrid.innerHTML = '<div class="loading">加载中</div>';

    try {
        // 1. 从API获取宠物列表
        const response = await fetchWithTimeout(`${API_BASE}/pets`);

        if (response.ok) {
            const apiPets = await response.json();
            // 服务器直接返回数组 [{name, type, hp, attack, level...}, ...]
            // 不是 {pets: [...]} 格式
            if (Array.isArray(apiPets) && apiPets.length > 0) {
                // 2. 映射API数据为前端完整格式，使用数组索引作为id
                pets = apiPets.map((apiPet, index) => mapApiPetToFull(apiPet, index));
            } else if (apiPets.pets && Array.isArray(apiPets.pets)) {
                // 兼容对象包装格式
                pets = apiPets.pets.map((apiPet, index) => mapApiPetToFull(apiPet, index));
            } else {
                pets = [];
            }
        } else {
            throw new Error('API返回错误');
        }
    } catch (error) {
        console.warn('[Pet Simulator] API不可用，加载离线数据:', error);
        isApiConnected = false;
        pets = getOfflinePets();
    }

    updatePetCount();
    renderPets();
}

/**
 * 将API返回的宠物数据映射为前端完整格式
 * C++服务器JSON: {name, type, hp, maxHp, attack, level, exp, expToNextLevel, alive}
 * 前端期望: {id, name, type, level, health, attack, defense, speed, experience}
 */
function mapApiPetToFull(apiData, index) {
    // C++类型是首字母大写，前端是小写
    const petType = (apiData.type || 'fire').toLowerCase();
    const defaultStats = getDefaultStats(petType);
    
    // 生成稳定id（基于索引）
    const petId = apiData.id !== undefined ? apiData.id : (index + 1);
    
    return {
        id: petId,
        name: apiData.name || '未知宠物',
        type: petType, // 转换为前端小写格式
        level: apiData.level || 1,
        // C++服务器返回hp/attack，前端用health/attack/defense/speed
        health: apiData.hp || apiData.maxHp || defaultStats.health,
        attack: apiData.attack || defaultStats.attack,
        defense: defaultStats.defense, // C++服务器无此字段，用默认值
        speed: defaultStats.speed,     // C++服务器无此字段，用默认值
        experience: apiData.exp || 0,
        maxHp: apiData.maxHp || apiData.hp || defaultStats.health,
        expToNextLevel: apiData.expToNextLevel || 100,
        alive: apiData.alive !== undefined ? apiData.alive : true
    };
}

/**
 * 根据类型生成默认属性值
 */
function getDefaultStats(type) {
    const baseByType = {
        'fire': { health: 80, attack: 75, defense: 40, speed: 60, description: '高攻击，低防御' },
        'water': { health: 120, attack: 50, defense: 80, speed: 50, description: '高HP，低攻击' },
        'grass': { health: 100, attack: 60, defense: 60, speed: 55, description: '平衡型，恢复能力' },
        'electric': { health: 85, attack: 80, defense: 45, speed: 75, description: '高速度，暴击率高' },
        'rock': { health: 130, attack: 55, defense: 90, speed: 35, description: '高防御，低速度' },
        'wind': { health: 90, attack: 65, defense: 50, speed: 85, description: '高速度，闪避率高' },
        'ice': { health: 95, attack: 70, defense: 55, speed: 65, description: '控制能力，减速效果' },
        'mech': { health: 120, attack: 60, defense: 80, speed: 40, description: '均衡型，机械特性' }
    };
    const base = baseByType[type] || baseByType.fire;
    return {
        health: base.health + Math.floor(Math.random() * 20),
        attack: base.attack + Math.floor(Math.random() * 15),
        defense: base.defense + Math.floor(Math.random() * 15),
        speed: base.speed + Math.floor(Math.random() * 10),
        description: base.description
    };
}

// 离线数据（初始为空，不从本地加载示例宠物）
function getOfflinePets() {
    return [];
}

// ============================================
// 商城系统
// ============================================
const shopItems = [
    {
        id: 'potion_hp',
        name: '生命药水',
        icon: '🧪',
        description: '恢复宠物50点生命值',
        price: 100,
        currency: 'coins',
        type: 'consumable'
    },
    {
        id: 'potion_atk',
        name: '攻击药水',
        icon: '⚔️',
        description: '临时提升宠物攻击力20点（战斗内）',
        price: 150,
        currency: 'coins',
        type: 'consumable'
    },
    {
        id: 'exp_boost',
        name: '经验加倍卡',
        icon: '⭐',
        description: '下一次战斗获得双倍经验',
        price: 30,
        currency: 'gems',
        type: 'consumable'
    },
    {
        id: 'pet_food',
        name: '宠物粮食',
        icon: '🍖',
        description: '提升宠物亲密度，增加5点全属性',
        price: 200,
        currency: 'coins',
        type: 'consumable'
    },
    {
        id: 'summon_scroll',
        name: '召唤卷轴',
        icon: '📜',
        description: '免除200金币直接召唤一次',
        price: 50,
        currency: 'gems',
        type: 'special'
    },
    {
        id: 'revive_stone',
        name: '复活石',
        icon: '💎',
        description: '复活在战斗中失败的宠物',
        price: 100,
        currency: 'gems',
        type: 'special'
    }
];

// 玩家背包数据
let inventory = [];

// 渲染商城
function renderShop() {
    const shopGrid = document.getElementById('shopGrid');
    if (!shopGrid) return;

    shopGrid.innerHTML = shopItems.map(item => `
        <div class="shop-card">
            <div class="shop-item-icon">${item.icon}</div>
            <div class="shop-item-info">
                <h3 class="shop-item-name">${item.name}</h3>
                <p class="shop-item-desc">${item.description}</p>
                <div class="shop-item-footer">
                    <div class="shop-item-price">
                        <span>${item.currency === 'coins' ? '🪙' : '💎'}</span>
                        <span>${item.price}</span>
                    </div>
                    <button class="shop-buy-btn" onclick="buyItem('${item.id}')">
                        购买
                    </button>
                </div>
            </div>
        </div>
    `).join('');
}

// 购买物品
function buyItem(itemId) {
    const item = shopItems.find(i => i.id === itemId);
    if (!item) {
        showToast('物品不存在', 'error');
        return;
    }

    // 检查货币是否足够
    if (item.currency === 'coins') {
        if (gameState.coins < item.price) {
            showToast('金币不足！', 'error');
            return;
        }
        gameState.coins -= item.price;
    } else if (item.currency === 'gems') {
        if (gameState.gems < item.price) {
            showToast('宝石不足！', 'error');
            return;
        }
        gameState.gems -= item.price;
    }

    // 添加到背包
    const existingItem = inventory.find(i => i.id === itemId);
    if (existingItem) {
        existingItem.quantity += 1;
    } else {
        inventory.push({
            ...item,
            quantity: 1
        });
    }

    // 更新UI
    updateUI();
    renderInventory();
    
    showToast(`成功购买 ${item.name}！`, 'success');
}

// 渲染背包
function renderInventory() {
    const inventoryGrid = document.getElementById('inventoryGrid');
    if (!inventoryGrid) return;

    if (inventory.length === 0) {
        inventoryGrid.innerHTML = `
            <div class="empty-state">
                <div class="empty-icon">🎒</div>
                <h3>背包空空如也</h3>
                <p>去商店购买道具吧！</p>
                <button class="cta-button" onclick="switchPanel('shop')">去商城</button>
            </div>
        `;
        return;
    }

    inventoryGrid.innerHTML = inventory.map(item => `
        <div class="inventory-card">
            <div class="inventory-item-icon">${item.icon}</div>
            <div class="inventory-item-info">
                <h3 class="inventory-item-name">${item.name}</h3>
                <p class="inventory-item-desc">${item.description}</p>
                <div class="inventory-item-quantity">数量: ${item.quantity}</div>
            </div>
            <button class="use-btn" onclick="useItem('${item.id}')">使用</button>
        </div>
    `).join('');
}

// 使用物品
function useItem(itemId) {
    const itemIndex = inventory.findIndex(i => i.id === itemId);
    if (itemIndex === -1) {
        showToast('物品不存在', 'error');
        return;
    }

    const item = inventory[itemIndex];

    // 根据物品类型执行不同效果
    switch (item.id) {
        case 'potion_hp':
            if (currentPetId) {
                const pet = pets.find(p => p.id === currentPetId);
                if (pet) {
                    pet.health += 50;
                    showToast(`${pet.name} 的生命值恢复了50点！`, 'success');
                }
            } else {
                // 无选中宠物时显示选择界面
                pendingItem = itemIndex;
                showPetSelectionForItem();
            }
            break;
        case 'exp_boost':
            // 标记下次战斗双倍经验
            gameState.expBoost = true;
            showToast('已激活经验加倍！下次战斗生效', 'success');
            break;
        case 'pet_food':
            if (currentPetId) {
                const pet = pets.find(p => p.id === currentPetId);
                if (pet) {
                    pet.health += 5;
                    pet.attack += 5;
                    pet.defense += 5;
                    pet.speed += 5;
                    showToast(`${pet.name} 的全属性提升了5点！`, 'success');
                }
            } else {
                // 无选中宠物时显示选择界面
                pendingItem = itemIndex;
                showPetSelectionForItem();
            }
            break;
        case 'summon_scroll':
            // 免费召唤一次
            const types = ['fire', 'water', 'mech'];
            const randomType = types[Math.floor(Math.random() * types.length)];
            summonPetFree(randomType);
            break;
        default:
            showToast(`使用了 ${item.name}`, 'info');
    }

    // 注意：potion_hp和pet_food的数量减少在applyPendingItem中处理
    // 其他物品在这里处理
    if (item.id !== 'potion_hp' && item.id !== 'pet_food') {
        item.quantity -= 1;
        if (item.quantity <= 0) {
            inventory.splice(itemIndex, 1);
        }
    }

    // 更新显示
    renderInventory();
    renderPets();
}

// 免费召唤（使用召唤卷轴）
async function summonPetFree(type) {
    const petNames = {
        fire: ['火焰精灵', '烈焰兽', '火凤凰', '炎龙'],
        water: ['海洋精灵', '海浪兽', '水凤凰', '冰龙'],
        mech: ['机械精灵', '钢铁兽', '机器人', '机械龙']
    };
    const names = petNames[type] || ['神秘宠物'];
    const randomName = names[Math.floor(Math.random() * names.length)];
    const defaultStats = getDefaultStats(type);

    try {
        const response = await fetchWithTimeout(`${API_BASE}/pets`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                name: randomName,
                type: capitalizeType(type),  // 转换为首字母大写格式
                level: 1,
                health: defaultStats.health,
                attack: defaultStats.attack,
                defense: defaultStats.defense,
                speed: defaultStats.speed
            })
        });

        if (!response.ok) throw new Error('API创建失败');
        await loadPetsFromApi();
    } catch (error) {
        const newPet = {
            id: Date.now(),
            name: randomName,
            type: type,
            level: 1,
            ...defaultStats,
            experience: 0
        };
        pets.push(newPet);
        updatePetCount();
        renderPets();
    }

    showToast(`使用召唤卷轴获得了 ${randomName}！`, 'success');
}

function updatePetCount() {
    const el = document.getElementById('petCount');
    if (el) {
        el.textContent = pets.length;
        el.style.display = pets.length > 0 ? 'block' : 'none';
    }
    // 更新类型统计
    ['fire', 'water', 'mech'].forEach(type => {
        const countEl = document.getElementById(`${type}Count`);
        if (countEl) countEl.textContent = pets.filter(p => p.type === type).length;
    });
}

// ============================================
// 渲染宠物列表
// ============================================
function renderPets() {
    const petsGrid = document.getElementById('petsGrid');
    if (!petsGrid) return;

    const filtered = currentFilter === 'all'
        ? pets
        : pets.filter(p => p.type === currentFilter);

    if (filtered.length === 0) {
        petsGrid.innerHTML = `
            <div class="empty-state">
                <div class="empty-icon">🎯</div>
                <h3>${currentFilter === 'all' ? '还没有宠物' : '该类型还没有宠物'}</h3>
                <p>${currentFilter === 'all' ? '点击左侧"召唤宠物"获取你的第一个伙伴！' : '尝试召唤其他类型的宠物吧！'}</p>
                ${currentFilter === 'all' ? '<button class="cta-button" onclick="switchPanel(\'summon\')">✨ 去召唤宠物</button>' : ''}
            </div>
        `;
        return;
    }

    petsGrid.innerHTML = filtered.map(pet => `
        <div class="pet-card" onclick="showPetDetail(${pet.id})">
            <div class="pet-emoji">${getPetEmoji(pet.type)}</div>
            <h3 class="pet-name">${pet.name}</h3>
            <span class="pet-type ${pet.type}">${getTypeName(pet.type)}</span>
            <div class="pet-stats">
                <div class="pet-stat">
                    <span class="stat-label">等级</span>
                    <span class="stat-number">${pet.level}</span>
                </div>
                <div class="pet-stat">
                    <span class="stat-label">生命</span>
                    <span class="stat-number">${pet.health}</span>
                </div>
                <div class="pet-stat">
                    <span class="stat-label">攻击</span>
                    <span class="stat-number">${pet.attack}</span>
                </div>
            </div>
            <div class="pet-exp-bar">
                <div class="pet-exp-fill" style="width: ${calcExpPercent(pet.experience, pet.level)}%"></div>
            </div>
        </div>
    `).join('');
}

function getPetEmoji(type) {
    const emojiMap = {
        'fire': '🔥',
        'water': '💧',
        'grass': '🌿',
        'electric': '⚡',
        'rock': '🪨',
        'wind': '💨',
        'ice': '❄️',
        'mech': '⚙️'
    };
    return emojiMap[type] || '🐾';
}

function getTypeName(type) {
    const nameMap = {
        'fire': '🔥 火焰',
        'water': '💧 水系',
        'grass': '🌿 草系',
        'electric': '⚡ 雷系',
        'rock': '🪨 岩石',
        'wind': '💨 风系',
        'ice': '❄️ 冰系',
        'mech': '⚙️ 机械'
    };
    return nameMap[type] || '未知';
}

function calcExpPercent(exp, level) {
    return Math.min(100, (exp / (level * 100)) * 100);
}

// ============================================
// 面板切换
// ============================================
function switchPanel(panelName) {
    document.querySelectorAll('.nav-item').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.panel === panelName);
    });
    document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
    const target = document.getElementById(`${panelName}Panel`);
    if (target) {
        target.classList.add('active');
        // 触发重绘实现动画
        target.style.animation = 'none';
        requestAnimationFrame(() => { target.style.animation = ''; });
    }

    // 竞技场面板：显示怪物选择或直接进入战斗准备
    if (panelName === 'battle') {
        // 如果是从宠物详情发起的战斗
        if (battleFromPetDetail) {
            const pet = pets.find(p => p.id === battleFromPetDetail);
            if (pet) {
                // 生成一个随机怪物进行战斗
                // 由于我们还没有后端API，这里使用临时方法
                const monsterTypes = ['fire', 'water', 'grass', 'electric', 'rock', 'wind', 'ice', 'mech'];
                const randomType = monsterTypes[Math.floor(Math.random() * monsterTypes.length)];
                const randomLevel = Math.floor(Math.random() * 5) + 1;

                currentMonster = {
                    id: Date.now(), // 临时ID
                    type: randomType,
                    level: randomLevel,
                    difficulty: randomLevel,
                    name: `野生${getTypeName(randomType)}怪`,
                    attack: getDefaultStats(randomType).attack,
                    hp: getDefaultStats(randomType).health
                };

                battleFromPetDetail = null; // 重置
                showBattlePrepForMonster(pet);
                return;
            }
        }

        // 重置当前怪物和宠物选择
        currentMonster = null;
        currentPetId = null;
        // 显示怪物选择界面
        showMonsterSelect();
    }
}

// ============================================
// 事件绑定
// ============================================
function bindEvents() {
    document.querySelectorAll('.filter-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.filter-btn').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            currentFilter = btn.dataset.filter;
            renderPets();
        });
    });
}

// ============================================
// 显示宠物详情（使用本地已加载的数据）
// ============================================
async function showPetDetail(petId) {
    // 不再从API获取，直接使用本地数据（已在loadPetsFromApi中加载）
    const pet = pets.find(p => p.id === petId);
    if (!pet) {
        showToast('宠物数据未找到', 'error');
        return;
    }
    
    currentPetId = petId;
    const modal = document.getElementById('petModal');
    const modalBody = document.getElementById('modalBody');
    if (!modal || !modalBody) return;

    const typeName = getTypeName(pet.type);
    modalBody.innerHTML = `
        <div class="modal-pet-emoji">${getPetEmoji(pet.type)}</div>
        <h2 class="modal-pet-name">${pet.name}</h2>
        <div class="modal-stats-grid">
            <div class="modal-stat">
                <span class="modal-stat-label">类型</span>
                <span class="modal-stat-value">${typeName}</span>
            </div>
            <div class="modal-stat">
                <span class="modal-stat-label">等级</span>
                <span class="modal-stat-value">${pet.level}</span>
            </div>
            <div class="modal-stat">
                <span class="modal-stat-label">生命值</span>
                <span class="modal-stat-value">${pet.health}</span>
            </div>
            <div class="modal-stat">
                <span class="modal-stat-label">攻击力</span>
                <span class="modal-stat-value">${pet.attack}</span>
            </div>
            <div class="modal-stat">
                <span class="modal-stat-label">防御力</span>
                <span class="modal-stat-value">${pet.defense}</span>
            </div>
            <div class="modal-stat">
                <span class="modal-stat-label">速度</span>
                <span class="modal-stat-value">${pet.speed}</span>
            </div>
        </div>
        <div class="modal-actions">
            <button class="modal-btn primary" onclick="battleWithPet(${pet.id})">⚔️ 战斗</button>
            <button class="modal-btn secondary" onclick="deletePet(${pet.id})">🗑️ 放生</button>
        </div>
    `;
    modal.classList.add('active');
}

// ============================================
// 召唤宠物 → 通过API创建
// ============================================
async function summonPet(type) {
    if (gameState.coins < 200) {
        showToast('金币不足！需要200金币', 'error');
        return;
    }

    const summonResult = document.getElementById('summonResult');
    if (summonResult) summonResult.innerHTML = '<div class="loading">召唤中</div>';

    // 生成本地数据（先决定名字和属性，再提交到API）
    const petNames = {
        fire: ['火焰精灵', '烈焰兽', '火凤凰', '炎龙'],
        water: ['海洋精灵', '海浪兽', '水凤凰', '冰龙'],
        mech: ['机械精灵', '钢铁兽', '机器人', '机械龙']
    };
    const names = petNames[type] || ['神秘宠物'];
    const randomName = names[Math.floor(Math.random() * names.length)];
    const defaultStats = getDefaultStats(type);

    let summonSuccess = false;

    // 先提交到API，再用API返回的id创建完整数据
    try {
        const response = await fetchWithTimeout(`${API_BASE}/pets`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                name: randomName,
                type: capitalizeType(type),  // 转换为首字母大写格式
                level: 1,
                health: defaultStats.health,
                attack: defaultStats.attack,
                defense: defaultStats.defense,
                speed: defaultStats.speed
            })
        });

        if (!response.ok) throw new Error('API创建失败');
        
        const result = await response.json();
        console.log('[召唤宠物] API返回:', result);
        
        // 成功：从API重新加载宠物列表（获取真实id）
        await loadPetsFromApi();
        summonSuccess = true;
    } catch (error) {
        console.warn('API创建失败，使用本地模式:', error);
        // 离线模式：本地创建
        const newPet = {
            id: Date.now(), // 用时间戳做临时id
            name: randomName,
            type: type,
            level: 1,
            ...defaultStats,
            experience: 0
        };
        pets.push(newPet);
        updatePetCount();
        renderPets();
        summonSuccess = true;
    }

    // 只有成功召唤后才扣除金币
    if (summonSuccess) {
        gameState.coins -= 200;
        updateUI();

        // 显示结果
        if (summonResult) {
            summonResult.innerHTML = `
                <div class="summon-success">
                    <div class="summon-emoji">${getPetEmoji(type)}</div>
                    <h3>召唤成功！</h3>
                    <p>你获得了 <strong>${randomName}</strong></p>
                    <div class="summon-stats">
                        <span>❤️ ${defaultStats.health}</span>
                        <span>⚔️ ${defaultStats.attack}</span>
                        <span>🛡️ ${defaultStats.defense}</span>
                    </div>
                    <button class="cta-button" onclick="switchPanel('pets')">查看宠物</button>
                </div>
            `;
        }

        showToast(`成功召唤 ${randomName}！`, 'success');
    }
}

// ============================================
// 删除宠物（API不支持DELETE时走本地）
// ============================================
function deletePet(petId) {
    if (!confirm('确定要放生这个宠物吗？')) return;

    // 尝试调用API删除
    fetchWithTimeout(`${API_BASE}/pet/${petId}`, { method: 'DELETE' }, 3000)
        .catch(() => {}); // API可能不支持DELETE，忽略

    // 无论API是否成功，都从本地移除
    pets = pets.filter(p => p.id !== petId);
    updatePetCount();
    renderPets();
    closeModal('petModal');
    showToast('宠物已放生', 'success');
}

// ============================================
// 战斗系统（纯客户端游戏逻辑）
// ============================================

// 显示竞技场宠物选择界面
function showBattlePetSelect() {
    const battleArea = document.getElementById('battleArea');
    if (!battleArea) return;
    
    if (pets.length === 0) {
        battleArea.innerHTML = `
            <div class="battle-empty">
                <div class="battle-icon">🏟️</div>
                <h3>还没有宠物</h3>
                <p>先去召唤一些宠物吧！</p>
                <button class="cta-button" onclick="switchPanel('summon')">去召唤宠物</button>
            </div>
        `;
        return;
    }
    
    battleArea.innerHTML = `
        <div class="battle-pet-select">
            <h3>选择你的宠物</h3>
            <div class="battle-pet-list">
                ${pets.map(pet => `
                    <div class="battle-pet-card" onclick="selectBattlePet(${pet.id})">
                        <div class="battle-pet-emoji">${getPetEmoji(pet.type)}</div>
                        <div class="battle-pet-info">
                            <strong>${pet.name}</strong>
                            <span class="pet-type ${pet.type}">${getTypeName(pet.type)}</span>
                            <div class="battle-pet-stats">
                                <span>LV. ${pet.level}</span>
                                <span>⚔️ ${pet.attack}</span>
                                <span>❤️ ${pet.health}</span>
                            </div>
                        </div>
                    </div>
                `).join('')}
            </div>
        </div>
    `;
}

// 选择战斗宠物
function selectBattlePet(petId) {
    currentPetId = petId;
    const pet = pets.find(p => p.id === petId);
    if (pet) {
        showBattlePrep(pet);
    }
}

// 选择宠物对战怪物
function selectBattlePetForMonster(petId) {
    currentPetId = petId;
    const pet = pets.find(p => p.id === petId);
    if (pet) {
        showBattlePrepForMonster(pet);
    }
}

// 显示宠物选择界面（用于道具使用）
function showPetSelectionForItem() {
    if (!pets.length) {
        showToast('请先召唤宠物', 'error');
        return;
    }

    // 创建模态框（如果不存在）
    let modal = document.getElementById('petSelectionModal');
    if (!modal) {
        modal = document.createElement('div');
        modal.id = 'petSelectionModal';
        modal.className = 'modal-overlay';
        modal.innerHTML = `
            <div class="modal-content">
                <h3>请选择要使用道具的宠物</h3>
                <div id="petSelectionList" class="pet-selection-list"></div>
                <button class="modal-close-btn" onclick="closePetSelectionModal()">取消</button>
            </div>
        `;
        document.body.appendChild(modal);
    }

    const list = document.getElementById('petSelectionList');
    list.innerHTML = pets.map(pet => `
        <div class="pet-selection-card" onclick="selectPetForItem(${pet.id})">
            <div class="pet-emoji">${getPetEmoji(pet.type)}</div>
            <div class="pet-info">
                <strong>${pet.name}</strong>
                <span class="pet-type ${pet.type}">${getTypeName(pet.type)}</span>
                <div class="pet-stats">
                    <span>LV. ${pet.level}</span>
                    <span>⚔️ ${pet.attack}</span>
                    <span>❤️ ${pet.health}</span>
                </div>
            </div>
        </div>
    `).join('');

    modal.style.display = 'block';
}

// 选择宠物并应用道具
function selectPetForItem(petId) {
    currentPetId = petId;
    closePetSelectionModal();
    applyPendingItem();
}

// 关闭宠物选择模态框
function closePetSelectionModal() {
    const modal = document.getElementById('petSelectionModal');
    if (modal) modal.style.display = 'none';
}

// 应用待处理的道具
function applyPendingItem() {
    if (pendingItem === null) return;
    const itemIndex = pendingItem;
    const item = inventory[itemIndex];
    // 根据物品类型执行不同效果
    switch (item.id) {
        case 'potion_hp':
            if (currentPetId) {
                const pet = pets.find(p => p.id === currentPetId);
                if (pet) {
                    pet.health += 50;
                    showToast(`${pet.name} 的生命值恢复了50点！`, 'success');
                }
            }
            break;
        case 'pet_food':
            if (currentPetId) {
                const pet = pets.find(p => p.id === currentPetId);
                if (pet) {
                    pet.health += 5;
                    pet.attack += 5;
                    pet.defense += 5;
                    pet.speed += 5;
                    showToast(`${pet.name} 的全属性提升了5点！`, 'success');
                }
            }
            break;
    }
    // 减少物品数量
    item.quantity -= 1;
    if (item.quantity <= 0) {
        inventory.splice(itemIndex, 1);
    }
    // 更新显示
    renderInventory();
    renderPets();
    // 重置
    pendingItem = null;
}
    function showBattlePrep(pet) {
    const battleArea = document.getElementById('battleArea');
    if (!battleArea) return;

    battleArea.innerHTML = `
        <div class="battle-prep">
            <h3>⚔️ 战斗准备</h3>
            <div class="battle-pet-info">
                <span class="battle-pet-emoji">${getPetEmoji(pet.type)}</span>
                <div class="battle-pet-details">
                    <strong>${pet.name}</strong>
                    <p>LV. ${pet.level} | 攻击力 ${pet.attack}</p>
                </div>
            </div>
            <button class="cta-button" onclick="startBattle(${pet.id})">⚔️ 开始战斗</button>
            <div class="battle-log" id="battleLog"></div>
        </div>
    `;
}

function battleWithPet(petId) {
    const pet = pets.find(p => p.id === petId);
    if (!pet) return;

    // 保存从宠物详情发起的战斗宠物ID
    battleFromPetDetail = petId;
    closeModal('petModal');
    switchPanel('battle');
}

async function startBattle(petId) {
    const pet = pets.find(p => p.id === petId);
    const battleLog = document.getElementById('battleLog');
    if (!pet || !battleLog) return;

    const battleBtn = document.querySelector('.battle-prep .cta-button');
    if (battleBtn) {
        battleBtn.disabled = true;
        battleBtn.textContent = '战斗进行中...';
    }

    battleLog.innerHTML = '<p>⚔️ 战斗开始！</p>';
    await delay(1000);
    battleLog.innerHTML += `<p>${pet.name} 发动攻击！造成 ${pet.attack} 点伤害</p>`;
    await delay(1000);
    const critDmg = Math.floor(Math.random() * 30) + 10;
    battleLog.innerHTML += `<p>🎯 暴击！额外造成 ${critDmg} 点伤害</p>`;
    await delay(1000);

    const expGained = 30 + Math.floor(Math.random() * 20);
    pet.experience += expGained;

    const expForNext = pet.level * 100;
    if (pet.experience >= expForNext) {
        pet.level += 1;
        pet.experience -= expForNext;
        pet.health += 10;
        pet.attack += 5;
        pet.defense += 5;
        pet.speed += 3;
        battleLog.innerHTML += `<p>🎉 升级！${pet.name} 达到 LV. ${pet.level}！</p>`;
    }

    battleLog.innerHTML += `<p>🎉 战斗胜利！获得 ${expGained} 经验值</p>`;

    if (battleBtn) {
        battleBtn.disabled = false;
        battleBtn.textContent = '⚔️ 再战一次';
    }

    renderPets();
    showToast('战斗胜利！', 'success');

    // 战斗有收益时同步到API（可选）
    if (isApiConnected) {
        try {
            await fetchWithTimeout(`${API_BASE}/pets`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    name: pet.name,
                    type: pet.type,
                    level: pet.level
                })
            });
        } catch (e) {
            // 静默失败，本地数据保留
        }
    }
}

// ============================================
// 模态框控制
// ============================================
function closeModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) modal.classList.remove('active');
    currentPetId = null;
}

document.addEventListener('click', (e) => {
    if (e.target.classList.contains('modal-overlay')) {
        e.target.classList.remove('active');
    }
});

// ============================================
// UI更新
// ============================================
function updateUI() {
    const setText = (id, text) => {
        const el = document.getElementById(id);
        if (el) el.textContent = text;
    };
    setText('coinCount', gameState.coins.toLocaleString());
    setText('gemCount', gameState.gems);
    setText('playerLevel', gameState.level);

    const expFill = document.getElementById('expFill');
    if (expFill) {
        const pct = Math.min(100, (gameState.exp / gameState.expToNext) * 100);
        expFill.style.width = `${pct}%`;
    }
}

// ============================================
// Toast通知
// ============================================
function showToast(message, type = 'info') {
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.innerHTML = `
        <span class="toast-icon">${type === 'success' ? '✅' : type === 'error' ? '❌' : 'ℹ️'}</span>
        <span class="toast-message">${message}</span>
    `;
    document.body.appendChild(toast);
    setTimeout(() => {
        toast.style.animation = 'toastSlideOut 0.3s ease forwards';
        setTimeout(() => toast.remove(), 300);
    }, 3500);
}

function delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

// 显示怪物选择界面
function showMonsterSelect() {
    const battleArea = document.getElementById('battleArea');
    if (!battleArea) return;

    // 从API获取随机怪物
    fetchWithTimeout(`${API_BASE}/monsters/random?difficulty=3`)
        .then(res => res.json())
        .then(data => {
            const monster = data.monster;
            battleArea.innerHTML = `
                <div class="battle-monster-select">
                    <h3>选择你的对手</h3>
                    <div class="battle-subtitle">点击怪物卡片开始战斗</div>
                    <div class="battle-monster-list">
                        <div class="battle-monster-card type-${monster.type}" onclick="selectMonster(${monster.id}, '${monster.type}', ${monster.level}, ${monster.difficulty}, '${monster.name}', ${monster.attack}, ${monster.hp})">
                            <div class="battle-monster-emoji">${getMonsterEmoji(monster.type)}</div>
                            <div class="battle-monster-info">
                                <strong>${monster.name}</strong>
                                <span class="monster-type ${monster.type}">${getTypeName(monster.type)}</span>
                                <div class="battle-monster-stats">
                                    <span>LV. ${monster.level}</span>
                                    <span>⚔️ ${monster.attack}</span>
                                    <span>❤️ ${monster.hp}</span>
                                </div>
                                <div class="monster-difficulty-badge">
                                    <span>⚔️</span>难度: ${monster.difficulty}
                                </div>
                                <div class="monster-desc">${getMonsterDescription(monster.type)}</div>
                            </div>
                        </div>
                    </div>
                </div>
            `;
        })
        .catch(err => {
            console.error('获取怪物失败:', err);
            battleArea.innerHTML = `
                <div class="battle-empty">
                    <div class="battle-icon">🏟️</div>
                    <h3>无法获取怪物数据</h3>
                    <p>请检查服务器连接</p>
                </div>
            `;
        });
}

// 选择怪物
function selectMonster(monsterId, type, level, difficulty, name, attack, hp) {
    // 存储怪物信息到全局变量
    currentMonster = {
        id: monsterId,
        type: type,
        level: level,
        difficulty: difficulty,
        name: name,
        attack: attack,
        hp: hp
    };

    // 显示宠物选择界面
    showPetSelectForMonster();
}

// 显示为怪物选择的宠物选择界面
function showPetSelectForMonster() {
    const battleArea = document.getElementById('battleArea');
    if (!battleArea) return;

    if (pets.length === 0) {
        battleArea.innerHTML = `
            <div class="battle-empty">
                <div class="battle-icon">🏟️</div>
                <h3>还没有宠物</h3>
                <p>先去召唤一些宠物吧！</p>
                <button class="cta-button" onclick="switchPanel('summon')">去召唤宠物</button>
            </div>
        `;
        return;
    }

    battleArea.innerHTML = `
        <div class="battle-pet-select">
            <h3>选择你的宠物</h3>
            <p class="battle-subtitle">当前对手: ${getMonsterEmoji(currentMonster.type)}${currentMonster.level}级${getTypeName(currentMonster.type)} (难度: ${currentMonster.difficulty})</p>
            <div class="battle-pet-list">
                ${pets.map(pet => `
                    <div class="battle-pet-card ${pet.id === currentPetId ? 'selected' : ''}"
                         onclick="selectBattlePetForMonster(${pet.id})">
                        <div class="battle-pet-emoji">${getPetEmoji(pet.type)}</div>
                        <div class="battle-pet-info">
                            <strong>${pet.name}</strong>
                            <span class="pet-type ${pet.type}">${getTypeName(pet.type)}</span>
                            <div class="battle-pet-stats">
                                <span>LV. ${pet.level}</span>
                                <span>⚔️ ${pet.attack}</span>
                                <span>❤️ ${pet.health}</span>
                            </div>
                            <div class="battle-type-advantage ${doesTypeBeat(pet.type, currentMonster.type) ? 'advantage' : ''}" style="${doesTypeBeat(pet.type, currentMonster.type) ? 'color: var(--success)' : ''}">
                                ${doesTypeBeat(pet.type, currentMonster.type) ? '✅ 属性克制' : ''}
                            </div>
                        </div>
                    </div>
                `).join('')}
            </div>
        </div>
    `;
}

// 显示怪物战斗准备界面
function showBattlePrepForMonster(pet) {
    const battleArea = document.getElementById('battleArea');
    if (!battleArea) return;

    battleArea.innerHTML = `
        <div class="battle-prep">
            <h3>⚔️ 战斗准备</h3>
            <div class="battle-monster-preview">
                <div class="battle-combatant">
                    <div class="battle-combatant-emoji">${getPetEmoji(pet.type)}</div>
                    <div class="battle-combatant-name">${pet.name}</div>
                    <div class="battle-combatant-stats">
                        等级: ${pet.level} | 攻击: ${pet.attack} | 生命: ${pet.health}
                    </div>
                    <div class="monster-type ${pet.type}">${getTypeName(pet.type)}</div>
                </div>
                <div class="battle-vs">VS</div>
                <div class="battle-combatant">
                    <div class="battle-combatant-emoji">${getMonsterEmoji(currentMonster.type)}</div>
                    <div class="battle-combatant-name">${currentMonster.name || '野生'+getTypeName(currentMonster.type)+'怪'}</div>
                    <div class="battle-combatant-stats">
                        等级: ${currentMonster.level} | 攻击: ${currentMonster.attack} | 生命: ${currentMonster.hp}
                    </div>
                    <div class="monster-type ${currentMonster.type}">${getTypeName(currentMonster.type)}</div>
                    <div class="monster-difficulty-badge">
                        <span>⚔️</span>难度: ${currentMonster.difficulty}
                    </div>
                </div>
            </div>
            <button class="cta-button" onclick="startMonsterBattle(${pet.id})">⚔️ 开始战斗</button>
            <div class="battle-log" id="battleLog"></div>
        </div>
    `;
}

// 开始怪物战斗（调用后端API）
async function startMonsterBattle(petId) {
    const pet = pets.find(p => p.id === petId);
    const battleLog = document.getElementById('battleLog');
    if (!pet || !battleLog) return;

    const battleBtn = document.querySelector('.battle-prep .cta-button');
    if (battleBtn) {
        battleBtn.disabled = true;
        battleBtn.textContent = '战斗进行中...';
    }

    battleLog.innerHTML = '<p>⚔️ 战斗开始！</p>';
    await delay(500);

    let isWin = false;

    try {
        const response = await fetchWithTimeout(`${API_BASE}/monsters/battle`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                petIndex: pet.id - 1,
                monsterName: currentMonster.name || `野生${getTypeName(currentMonster.type)}怪`,
                monsterType: currentMonster.type,
                monsterLevel: currentMonster.level
            })
        });

        const data = await response.json();

        if (!data.success) {
            battleLog.innerHTML += `<p style="color:red">❌ 战斗出错: ${data.error || '未知错误'}</p>`;
            showToast('战斗出错', 'error');
            return;
        }

        // 逐回合显示战斗日志
        const logEntries = data.battle.log || [];
        for (const entry of logEntries) {
            battleLog.innerHTML += `<p>${entry}</p>`;
            await delay(700);
        }

        // 判断胜负：如果winner是宠物名字，则宠物获胜
        isWin = data.battle.winner === pet.name;

        if (isWin) {
            battleLog.innerHTML += `<p style="color:var(--success);font-weight:bold">🎉 战斗胜利！</p>`;

            // 应用奖励
            const reward = data.reward || {};
            const expGained = reward.exp || 50;
            const coinsGained = reward.coins || 100;
            const diamondsGained = reward.diamonds || 0;

            pet.experience = (pet.experience || 0) + expGained;
            gameState.coins += coinsGained;
            gameState.gems += diamondsGained;

            // 检查升级
            const expForNext = pet.level * 100;
            if (pet.experience >= expForNext) {
                pet.level += 1;
                pet.experience -= expForNext;
                pet.health += 10;
                pet.attack += 5;
                pet.defense += 5;
                pet.speed += 3;
                battleLog.innerHTML += `<p>🎉 升级！${pet.name} 达到 LV. ${pet.level}！</p>`;
            }

            battleLog.innerHTML += `<p>💰 获得 ${expGained} 经验值, ${coinsGained} 金币, ${diamondsGained} 钻石！</p>`;
        } else {
            battleLog.innerHTML += `<p style="color:var(--danger);font-weight:bold">💀 战斗失败！宠物失去战斗能力</p>`;
            pet.health = 0;
        }

        // 从服务器响应更新宠物状态（HP等）
        if (data.pet) {
            const serverPet = data.pet;
            pet.health = serverPet.hp !== undefined ? serverPet.hp : pet.health;
            pet.attack = serverPet.attack !== undefined ? serverPet.attack : pet.attack;
        }

    } catch (err) {
        console.error('战斗API调用失败:', err);
        battleLog.innerHTML += `<p style="color:red">❌ 网络错误: ${err.message}</p>`;
        showToast('网络连接失败', 'error');
    }

    if (battleBtn) {
        battleBtn.disabled = false;
        battleBtn.textContent = '⚔️ 再战一次';
    }

    renderPets();
    updateUI();
    showToast(isWin ? '战斗胜利！' : '战斗失败', isWin ? 'success' : 'error');
}

// 获取怪物表情符号
function getMonsterEmoji(type) {
    const emojiMap = {
        'fire': '🔥',
        'water': '💧',
        'grass': '🌿',
        'electric': '⚡',
        'rock': '🪨',
        'wind': '💨',
        'ice': '❄️',
        'mech': '⚙️'
    };
    return emojiMap[type] || '❓';
}

// 获取怪物类型名称
function getTypeName(type) {
    const nameMap = {
        'fire': '🔥 火焰',
        'water': '💧 水系',
        'grass': '🌿 草系',
        'electric': '⚡ 雷系',
        'rock': '🪨 岩石',
        'wind': '💨 风系',
        'ice': '❄️ 冰系',
        'mech': '⚙️ 机械'
    };
    return nameMap[type] || '未知';
}

// 获取怪物描述
function getMonsterDescription(type) {
    const descMap = {
        'fire': '高攻击力，但防御较弱',
        'water': '高生命值，攻击较低',
        'grass': '平衡型，具有恢复能力',
        'electric': '高速度，暴击率较高',
        'rock': '高防御，速度极慢',
        'wind': '高速度，闪避率较高',
        'ice': '控制能力，可降低对手速度',
        'mech': '均衡型，机械特性'
    };
    return descMap[type] || '未知属性';
}

// 附加样式（动态生成的UI组件需要）
const style = document.createElement('style');
style.textContent = `
    @keyframes toastSlideOut {
        to { opacity: 0; transform: translateX(-50%) translateY(-20px); }
    }
    .summon-success { text-align: center; padding: var(--space-xl); }
    .summon-emoji { font-size: 4rem; margin-bottom: var(--space-lg); animation: petFloat 2s ease-in-out infinite; }
    .summon-success h3 { font-family: var(--font-display); font-size: 1.5rem; color: var(--gold); margin-bottom: var(--space-sm); }
    .summon-success p { color: var(--text-secondary); margin-bottom: var(--space-md); }
    .summon-stats { display: flex; justify-content: center; gap: var(--space-lg); margin-bottom: var(--space-xl); font-size: 0.95rem; color: var(--text-primary); }
`;
document.head.appendChild(style);

console.log('[Pet Simulator] JS加载完成');
