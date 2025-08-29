const GRID_ROWS = 4;
const GRID_COLS = 5;
const GROWTH_TIME = 5; // seconds to grow
const DAY_LENGTH = 80; // seconds for a full day-night cycle (1 minutes)

const farmGrid = document.getElementById('farm-grid');
const dayNightIndicator = document.getElementById('day-night-indicator');
const plantBtn = document.getElementById('plant-btn');
const harvestBtn = document.getElementById('harvest-btn');

let selectedTile = null;
let dayCount = 1;
let timeOfDay = 0; // 0 to 1, 0 = morning, 0.5 = night, 1 = next morning
let tiles = [];
let coins = 50;
const COIN_PLANT_COST = 5;
const COIN_HARVEST_GAIN = 10;
const coinAmount = document.getElementById('coin-amount');
let lastUpdateTime = Date.now();

const CROPS = {
    carrot: {
        name: 'Carrot',
        stages: [
            { emoji: '🌱', label: 'Seed' },
            { emoji: '🥕', label: 'Sprout' },
            { emoji: '🥕', label: 'Small' },
            { emoji: '🥕', label: 'Mature' }
        ],
        growthDays: 5,
        plantCost: 5,
        harvestGain: 40
    },
    corn: {
        name: 'Corn',
        stages: [
            { emoji: '🌱', label: 'Seed' },
            { emoji: '🌽', label: 'Sprout' },
            { emoji: '🌽', label: 'Small' },
            { emoji: '🌽', label: 'Mature' }
        ],
        growthDays: 3,
        plantCost: 8,
        harvestGain: 20
    },
    tomato: {
        name: 'Tomato',
        stages: [
            { emoji: '🌱', label: 'Seed' },
            { emoji: '🍅', label: 'Sprout' },
            { emoji: '🍅', label: 'Small' },
            { emoji: '🍅', label: 'Mature' }
        ],
        growthDays: 7,
        plantCost: 12,
        harvestGain: 70
    }
};
let selectedCrop = 'carrot';

// Market and Inventory Data
const MARKET_ITEMS = [
    { type: 'crop', id: 'carrot', name: 'Carrot', emoji: '🥕', price: 20, description: 'A basic crop. Plant it to grow and harvest for coins.' },
    { type: 'crop', id: 'corn', name: 'Corn', emoji: '🌽', price: 40, description: 'A fast-growing crop. Plant it to grow and harvest for coins.' },
    { type: 'crop', id: 'tomato', name: 'Tomato', emoji: '🍅', price: 70, description: 'A valuable crop. Plant it to grow and harvest for coins.' },
    { type: 'item', id: 'water', name: 'Water', emoji: '💧', price: 80, description: 'Makes a crop grow one day faster when used.' },
    { type: 'item', id: 'fertilizer', name: 'Fertilizer', emoji: '🧪', price: 90, description: 'Gives a 40% chance to get a bonus seed when harvesting a fertilized crop.' },
    { type: 'item', id: 'tool', name: 'Harvest Tool', emoji: '🪓', price: 600, description: 'Allows you to harvest multiple matured crops at once by dragging over them.' }
];
let inventory = {
    carrot: 0, // unlocked by default
    corn: 0,
    tomato: 0,
    water: 0,
    fertilizer: 0,
    tool: 0
};

// Dynamic price ranges
const PRICE_RANGES = {
    carrot: {
        buy: [20, 30, 40],
        sell: [30, 50] // min, max
    },
    corn: {
        buy: [30, 40, 50, 60, 79],
        sell: [40, 50]
    },
    tomato: {
        buy: [50, 60, 70, 80, 90],
        sell: [90, 120]
    }
};

// Add dynamic price ranges for items
const ITEM_PRICE_RANGES = {
    water: [40, 50, 60, 90],
    fertilizer: [70, 80, 90, 100, 120],
    tool: [600, 700, 1000, 1200]
};

// Store today's prices
let todaysPrices = {
    carrot: { buy: 20, sell: 40 },
    corn: { buy: 40, sell: 45 },
    tomato: { buy: 70, sell: 100 },
    water: { buy: 40 },
    fertilizer: { buy: 70 },
    tool: { buy: 500 }
};

function randomFromArray(arr) {
    return arr[Math.floor(Math.random() * arr.length)];
}
function randomIntRange(min, max) {
    return Math.floor(Math.random() * (max - min + 1)) + min;
}

function setTodaysPrices() {
    // Set buy prices for crops
    todaysPrices.carrot.buy = randomFromArray(PRICE_RANGES.carrot.buy);
    todaysPrices.corn.buy = randomFromArray(PRICE_RANGES.corn.buy);
    todaysPrices.tomato.buy = randomFromArray(PRICE_RANGES.tomato.buy);
    // Set sell prices for crops
    todaysPrices.carrot.sell = randomIntRange(PRICE_RANGES.carrot.sell[0], PRICE_RANGES.carrot.sell[1]);
    todaysPrices.corn.sell = randomIntRange(PRICE_RANGES.corn.sell[0], PRICE_RANGES.corn.sell[1]);
    todaysPrices.tomato.sell = randomIntRange(PRICE_RANGES.tomato.sell[0], PRICE_RANGES.tomato.sell[1]);
    // Set buy prices for items
    todaysPrices.water = { buy: randomFromArray(ITEM_PRICE_RANGES.water) };
    todaysPrices.fertilizer = { buy: randomFromArray(ITEM_PRICE_RANGES.fertilizer) };
    todaysPrices.tool = { buy: randomFromArray(ITEM_PRICE_RANGES.tool) };
}

// Generalized price indicator for any item
function getItemPriceIndicator(itemId, price, rangeArr) {
    const min = Math.min(...rangeArr);
    const max = Math.max(...rangeArr);
    if (price === min) return '<span class="price-indicator" style="color:#43a047">▼</span>';
    if (price === max) return '<span class="price-indicator" style="color:#e53935">▲</span>';
    return '';
}

function updateWaterButtonState() {
    // TODO: Implement water button state logic if needed
}

// Helper for price indicator
function getPriceIndicator(type, crop, value) {
    const buyArr = PRICE_RANGES[crop].buy;
    const sellArr = PRICE_RANGES[crop].sell;
    let min, max;
    if (type === 'buy') {
        min = Math.min(...buyArr);
        max = Math.max(...buyArr);
    } else {
        min = sellArr[0];
        max = sellArr[1];
    }
    if (value === min) return { icon: '<span class="price-indicator" style="color:#43a047">▼</span>', color: '#43a047' };
    if (value === max) return { icon: '<span class="price-indicator" style="color:#e53935">▲</span>', color: '#e53935' };
    return { icon: '', color: '' };
}

// Update MARKET_ITEMS with today's prices
function updateMarketPrices() {
    MARKET_ITEMS.forEach(item => {
        if (item.type === 'crop') {
            item.price = todaysPrices[item.id].buy;
            item.harvestValue = todaysPrices[item.id].sell;
        } else if (item.type === 'item') {
            item.price = todaysPrices[item.id]?.buy || item.price;
        }
    });
}

// Call this at the start of each day
function newDayMarketUpdate() {
    setTodaysPrices();
    updateMarketPrices();
    renderMarket();
    renderInventory();
}

function createGrid() {
    farmGrid.innerHTML = '';
    tiles = [];
    for (let r = 0; r < GRID_ROWS; r++) {
        for (let c = 0; c < GRID_COLS; c++) {
            const tile = document.createElement('div');
            tile.className = 'farm-tile';
            tile.dataset.row = r;
            tile.dataset.col = c;
            tile.addEventListener('click', () => selectTile(tile, r, c));
            farmGrid.appendChild(tile);
            tiles.push({
                el: tile,
                state: 'empty',
                plantedAt: null,
                watered: false, // Watered status for today
                daysWatered: 0 // How many times watered since planted
            });
        }
    }
    setTimeout(attachCropTooltipEvents, 0);
}

function selectTile(tile, r, c) {
    tiles.forEach(t => t.el.classList.remove('selected'));
    tile.classList.add('selected');
    selectedTile = tiles[r * GRID_COLS + c];
    updateWaterButtonState();
}

let hasWon = false;

function updateCoins(amount) {
    coins += amount;
    coinAmount.textContent = coins;
    coinAmount.classList.add('animated');
    setTimeout(() => coinAmount.classList.remove('animated'), 200);
    console.log('[DEBUG] Coins after update:', coins, 'Amount added:', amount);
    if (Number(coins) >= 1000000 && !hasWon) {
        showWinScreen();
    }
}

function showWarning(msg) {
    let area = document.getElementById('notification-area');
    let warn = document.getElementById('warn-msg');
    if (!warn) {
        warn = document.createElement('div');
        warn.id = 'warn-msg';
        area.appendChild(warn);
    }
    warn.textContent = msg;
    warn.style.opacity = 1;
    setTimeout(() => { warn.style.opacity = 0; }, 1200);
}

function showWinScreen() {
    if (hasWon || document.getElementById('win-overlay')) return;
    hasWon = true;
    // Stop the game loop
    if (window.cancelAnimationFrame && window._gameLoopId) {
        cancelAnimationFrame(window._gameLoopId);
    }
    // Overlay code as before...
    const overlay = document.createElement('div');
    overlay.id = 'win-overlay';
    overlay.style.position = 'fixed';
    overlay.style.top = '0';
    overlay.style.left = '0';
    overlay.style.width = '100vw';
    overlay.style.height = '100vh';
    overlay.style.background = 'rgba(255,255,255,0.55)';
    overlay.style.zIndex = '30000';
    overlay.style.display = 'flex';
    overlay.style.alignItems = 'center';
    overlay.style.justifyContent = 'center';
    overlay.style.backdropFilter = 'blur(8px)';
    overlay.style.transition = 'opacity 0.5s';
    overlay.style.pointerEvents = 'auto';
    // Card
    const card = document.createElement('div');
    card.style.background = 'linear-gradient(135deg, #fbe7ff 0%, #e3f0ff 100%)';
    card.style.borderRadius = '32px';
    card.style.boxShadow = '0 16px 48px rgba(120, 120, 180, 0.18), 0 4px 16px rgba(120, 120, 180, 0.10)';
    card.style.padding = '54px 44px 44px 44px';
    card.style.display = 'flex';
    card.style.flexDirection = 'column';
    card.style.alignItems = 'center';
    card.style.gap = '22px';
    card.style.maxWidth = '92vw';
    card.style.minWidth = '260px';
    card.style.position = 'relative';
    card.style.zIndex = '30001';
    card.style.pointerEvents = 'auto';
    // Cute emoji
    const emoji = document.createElement('div');
    emoji.textContent = '🏆✨';
    emoji.style.fontSize = '4.5rem';
    emoji.style.marginBottom = '8px';
    emoji.style.filter = 'drop-shadow(0 2px 12px #ffd54f)';
    // Title
    const title = document.createElement('div');
    title.textContent = 'You Did The Impossible!';
    title.style.fontSize = '2.3rem';
    title.style.fontWeight = '900';
    title.style.color = '#7e57c2';
    title.style.letterSpacing = '0.5px';
    title.style.marginBottom = '2px';
    // Message
    const msg = document.createElement('div');
    msg.style.fontSize = '1.18rem';
    msg.style.color = '#4a4a4a';
    msg.style.textAlign = 'center';
    msg.style.maxWidth = '420px';
    msg.innerHTML = `There are <b>very few</b> people who have ever achieved this goal.<br><br>This was a <b>social experiment</b> to test the indomitable human spirit.<br><br><b>You are definitely worth that title!</b><br><br><span style="font-size:1.25rem;color:#e65100;font-weight:700;">You have earned the title:</span><br><span style="font-size:1.5rem;color:#51754e;font-weight:900;">"The Eternal"</span><br><br>You can screenshot this and post on Instagram, tag and follow <b>@ajay_99210</b> and get a chance to meet him!`;
    // Sparkles
    const sparkle = document.createElement('span');
    sparkle.innerHTML = '✨';
    sparkle.style.position = 'absolute';
    sparkle.style.top = '18px';
    sparkle.style.right = '28px';
    sparkle.style.fontSize = '2.1rem';
    sparkle.style.opacity = '0.7';
    // Restart button
    const restartBtn = document.createElement('button');
    restartBtn.textContent = 'Restart Game';
    restartBtn.style.background = 'linear-gradient(90deg, #81c784 60%, #aed581 100%)';
    restartBtn.style.color = '#fff';
    restartBtn.style.border = 'none';
    restartBtn.style.borderRadius = '14px';
    restartBtn.style.padding = '14px 44px';
    restartBtn.style.fontSize = '1.18rem';
    restartBtn.style.fontWeight = 'bold';
    restartBtn.style.cursor = 'pointer';
    restartBtn.style.boxShadow = '0 2px 8px rgba(120,120,180,0.10)';
    restartBtn.style.transition = 'background 0.2s, transform 0.1s';
    restartBtn.style.marginTop = '18px';
    restartBtn.addEventListener('click', () => { window.location.reload(); });
    // Add to card
    card.appendChild(sparkle);
    card.appendChild(emoji);
    card.appendChild(title);
    card.appendChild(msg);
    card.appendChild(restartBtn);
    overlay.appendChild(card);
    document.body.appendChild(overlay);
    setTimeout(() => { overlay.style.opacity = '1'; }, 30);
    // Prevent interaction with background
    overlay.addEventListener('click', (e) => { e.stopPropagation(); });
    card.addEventListener('click', (e) => { e.stopPropagation(); });
}

// Crop selection logic
const cropBtns = document.querySelectorAll('.crop-btn');
cropBtns.forEach(btn => {
    btn.addEventListener('click', () => {
        cropBtns.forEach(b => b.classList.remove('selected'));
        btn.classList.add('selected');
        selectedCrop = btn.dataset.crop;
    });
});

// Helper: Add sparkle effect to an element
function addSparkle(el) {
    const sparkle = document.createElement('span');
    sparkle.className = 'sparkle';
    sparkle.innerHTML = '✨';
    el.appendChild(sparkle);
    setTimeout(() => sparkle.remove(), 700);
}

// Helper: Add coin sparkle to coin counter
function coinSparkle() {
    const counter = document.getElementById('coin-counter');
    const sparkle = document.createElement('span');
    sparkle.className = 'coin-sparkle';
    sparkle.innerHTML = '✨';
    counter.appendChild(sparkle);
    setTimeout(() => sparkle.remove(), 700);
}

// Helper: Animate crop jump out
function animateHarvest(el) {
    el.style.transition = 'transform 0.5s cubic-bezier(.68,-0.55,.27,1.55), opacity 0.5s';
    el.style.transform = 'translateY(-40px) scale(1.2)';
    el.style.opacity = '0';
    setTimeout(() => { el && el.remove(); }, 500);
}

// Helper: Squish button
function squishButton(btn) {
    btn.classList.add('squish');
    setTimeout(() => btn.classList.remove('squish'), 180);
}

// Sun/Moon SVG logic
function updateSunMoon(phase) {
    let sun = document.getElementById('sun-svg');
    let moon = document.getElementById('moon-svg');
    if (!sun) {
        sun = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
        sun.setAttribute('id', 'sun-svg');
        sun.setAttribute('class', 'sun-svg');
        sun.setAttribute('viewBox', '0 0 38 38');
        sun.innerHTML = '<circle cx="19" cy="19" r="13" fill="#ffe082"><animate attributeName="r" values="13;15;13" dur="4s" repeatCount="indefinite"/></circle>';
        document.getElementById('game-card').appendChild(sun);
    }
    if (!moon) {
        moon = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
        moon.setAttribute('id', 'moon-svg');
        moon.setAttribute('class', 'moon-svg');
        moon.setAttribute('viewBox', '0 0 38 38');
        moon.innerHTML = '<ellipse cx="22" cy="19" rx="10" ry="13" fill="#b2cfff"/><ellipse cx="26" cy="19" rx="7" ry="10" fill="#e3f0ff"/>';
        document.getElementById('game-card').appendChild(moon);
    }
    if (phase === 'Night') {
        sun.style.opacity = 0;
        moon.style.opacity = 1;
    } else {
        sun.style.opacity = 1;
        moon.style.opacity = 0;
    }
}

// --- Crop Tooltip Logic ---
let cropTooltip = null;
let tooltipTimeout = null;

function showCropTooltip(tile, event) {
    // Cancel any pending hide/show
    if (tooltipTimeout) {
        clearTimeout(tooltipTimeout);
        tooltipTimeout = null;
    }
    if (!tile || !tile.crop) return;
    const crop = CROPS[tile.crop];
    if (!crop) return;
    // Remove any existing tooltip
    hideCropTooltip(true);
    cropTooltip = document.createElement('div');
    cropTooltip.className = 'crop-tooltip';
    // Calculate days passed: 1 per day, +1 for each day watered
    const totalStages = crop.stages.length;
    const stageIdx = tile.stage ?? 0;
    const stageObj = crop.stages[stageIdx];
    const plantedDay = tile.plantedDay || dayCount;
    let daysPassed = (dayCount - plantedDay) + tile.daysWatered;
    let daysLeft = Math.max(0, crop.growthDays - daysPassed);
    const isWatered = tile.watered;
    const isFertilized = tile.fertilized || false;
    const sellValue = todaysPrices[tile.crop]?.sell || crop.harvestGain;
    const sellInd = getPriceIndicator('sell', tile.crop, sellValue);
    cropTooltip.innerHTML = `
      <div class=\"tooltip-title\">${crop.stages[totalStages-1].emoji} <span>${crop.name}</span></div>
      <div class=\"tooltip-row\"><span class=\"tooltip-label\">Stage:</span> <span class=\"tooltip-value\">${stageObj.emoji} ${stageObj.label}</span></div>
      <div class=\"tooltip-row\"><span class=\"tooltip-label\">Days Left:</span> <span class=\"tooltip-value\">${daysLeft}</span></div>
      <div class=\"tooltip-row\"><span class=\"tooltip-label\">Watered:</span> <span class=\"tooltip-value\">${isWatered ? '💧 Yes' : 'No'}</span></div>
      <div class=\"tooltip-row\"><span class=\"tooltip-label\">Fertilized:</span> <span class=\"tooltip-value\">${isFertilized ? '🧪 Yes' : 'No'}</span></div>
      <div class=\"tooltip-row\"><span class=\"tooltip-label\">Planted Day:</span> <span class=\"tooltip-value\">${tile.plantedDay ?? '-'}</span></div>
      <div class=\"tooltip-row\"><span class=\"tooltip-label\">Harvest Value:</span> <span class=\"tooltip-value\">🪙 ${sellValue}${sellInd.icon}</span></div>
    `;
    document.body.appendChild(cropTooltip);
    cropTooltip.classList.add('visible');
    positionCropTooltip(event);
}

function hideCropTooltip(immediate = false) {
    if (tooltipTimeout) {
        clearTimeout(tooltipTimeout);
        tooltipTimeout = null;
    }
    if (cropTooltip) {
        cropTooltip.classList.remove('visible');
        if (immediate) {
            cropTooltip.remove();
            cropTooltip = null;
        } else {
            setTimeout(() => { cropTooltip && cropTooltip.remove(); cropTooltip = null; }, 180);
        }
    }
}

function positionCropTooltip(event) {
    if (!cropTooltip) return;
    const padding = 12;
    const rect = cropTooltip.getBoundingClientRect();
    let x = event.clientX + 18;
    let y = event.clientY + 12;
    // Prevent overflow right/bottom
    if (x + rect.width + padding > window.innerWidth) x = window.innerWidth - rect.width - padding;
    if (y + rect.height + padding > window.innerHeight) y = window.innerHeight - rect.height - padding;
    // Prevent overflow left/top
    if (x < padding) x = padding;
    if (y < padding) y = padding;
    cropTooltip.style.left = x + 'px';
    cropTooltip.style.top = y + 'px';
}

function attachCropTooltipEvents() {
    tiles.forEach(tile => {
        tile.el.onmouseenter = null;
        tile.el.onmousemove = null;
        tile.el.onmouseleave = null;
        if (tile.state === 'planted' || tile.state === 'grown') {
            tile.el.onmouseenter = (e) => {
                showCropTooltip(tile, e);
            };
            tile.el.onmousemove = (e) => {
                if (cropTooltip) positionCropTooltip(e);
            };
            tile.el.onmouseleave = () => {
                hideCropTooltip();
            };
        }
    });
}

// --- Market Tooltip Logic ---
let marketTooltip = null;
let marketTooltipTimeout = null;

function showMarketTooltip(item, event) {
    // Cancel any pending hide/show
    if (marketTooltipTimeout) {
        clearTimeout(marketTooltipTimeout);
        marketTooltipTimeout = null;
    }
    if (!item || !item.description) return;
    hideMarketTooltip(true);
    if (!marketTooltip) {
        marketTooltip = document.createElement('div');
        marketTooltip.className = 'market-tooltip';
        document.body.appendChild(marketTooltip);
    }
    marketTooltip.innerHTML = `
        <div class=\"tooltip-title\">${item.emoji} <span>${item.name}</span></div>
        <div class=\"tooltip-desc\">${item.description}</div>
    `;
    marketTooltip.classList.add('visible');
    positionMarketTooltip(event);
}

function hideMarketTooltip(immediate = false) {
    if (marketTooltipTimeout) {
        clearTimeout(marketTooltipTimeout);
        marketTooltipTimeout = null;
    }
    if (marketTooltip) {
        marketTooltip.classList.remove('visible');
        if (immediate) {
            marketTooltip.remove();
            marketTooltip = null;
        } else {
            setTimeout(() => { if (marketTooltip) marketTooltip.remove(); marketTooltip = null; }, 180);
        }
    }
}

function positionMarketTooltip(event) {
    if (!marketTooltip) return;
    const padding = 12;
    const rect = marketTooltip.getBoundingClientRect();
    let x = event.clientX + 18;
    let y = event.clientY + 12;
    // Prevent overflow right/bottom
    if (x + rect.width + padding > window.innerWidth) x = window.innerWidth - rect.width - padding;
    if (y + rect.height + padding > window.innerHeight) y = window.innerHeight - rect.height - padding;
    // Prevent overflow left/top
    if (x < padding) x = padding;
    if (y < padding) y = padding;
    marketTooltip.style.left = x + 'px';
    marketTooltip.style.top = y + 'px';
}

function attachMarketTooltipEvents() {
    const marketDiv = document.getElementById('market-items');
    marketDiv.querySelectorAll('.market-item').forEach((el, idx) => {
        const item = MARKET_ITEMS[idx];
        el.onmouseenter = (e) => {
            showMarketTooltip(item, e);
        };
        el.onmousemove = (e) => {
            if (marketTooltip) positionMarketTooltip(e);
        };
        el.onmouseleave = () => {
            hideMarketTooltip();
        };
    });
}

function renderMarket() {
    const marketDiv = document.getElementById('market-items');
    marketDiv.innerHTML = '';
    MARKET_ITEMS.forEach(item => {
        let price = item.price;
        let priceIndicator = '';
        let priceColor = '';
        if (item.type === 'crop') {
            const ind = getPriceIndicator('buy', item.id, price);
            priceIndicator = ind.icon;
            priceColor = ind.color;
        } else if (item.type === 'item') {
            // Use generalized indicator for items
            let rangeArr = ITEM_PRICE_RANGES[item.id];
            if (rangeArr) priceIndicator = getItemPriceIndicator(item.id, price, rangeArr);
        }
        marketDiv.innerHTML += `
            <div class="market-item">
                <span class="item-emoji">${item.emoji}</span>
                <span class="item-name">${item.name}</span>
                <span class="item-price">🪙 ${price}${priceIndicator}</span>
                <button class="buy-btn" data-id="${item.id}" data-type="${item.type}">Buy</button>
            </div>
        `;
    });
    // Add buy logic
    marketDiv.querySelectorAll('.buy-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            const id = btn.getAttribute('data-id');
            const type = btn.getAttribute('data-type');
            const item = MARKET_ITEMS.find(i => i.id === id);
            if (coins < item.price) {
                showWarning('Not enough coins!');
                return;
            }
            updateCoins(-item.price);
            if (type === 'crop') {
                inventory[id] = (inventory[id] || 0) + 1;
                showWarning(`Bought 1 ${item.emoji} ${item.name} seed!`);
            } else {
                inventory[id] = (inventory[id] || 0) + 1;
                showWarning(`Bought ${item.emoji} ${item.name}!`);
            }
            renderInventory();
        });
    });
    // Attach tooltips
    attachMarketTooltipEvents();
}

function renderInventory() {
    const invList = document.getElementById('inventory-list');
    invList.innerHTML = '';
    Object.entries(inventory).forEach(([id, count]) => {
        if (count > 0) {
            const item = MARKET_ITEMS.find(i => i.id === id);
            let li = document.createElement('li');
            li.innerHTML = `<span class="item-emoji">${item ? item.emoji : ''}</span> <span>${item ? item.name : id}</span> <b>x${count}</b>`;
            invList.appendChild(li);
        }
    });
    updateCropButtonsState();
    enableInventoryDrag();
}

// Only allow planting unlocked crops
function plantCrop() {
    if (!selectedTile || selectedTile.state !== 'empty') return;
    const crop = CROPS[selectedCrop];
    if (!inventory[selectedCrop] || inventory[selectedCrop] < 1) {
        showWarning('Buy seeds for this crop in the market first!');
        return;
    }
    inventory[selectedCrop] -= 1;
    renderInventory();
    updateCropButtonsState();
    selectedTile.state = 'planted';
    selectedTile.crop = selectedCrop;
    selectedTile.stage = 0;
    selectedTile.stageStart = Date.now();
    selectedTile.plantedDay = dayCount;
    selectedTile.el.classList.add('planted');
    selectedTile.el.innerHTML = `<span class="crop">${crop.stages[0].emoji}</span>`;
    addSparkle(selectedTile.el);
    selectedTile.el.querySelector('.crop').animate([
        { transform: 'scale(0.7)' },
        { transform: 'scale(1.2)' },
        { transform: 'scale(1)' }
    ], { duration: 350, easing: 'ease-out' });
}

function harvestCrop() {
    if (!selectedTile || selectedTile.state !== 'grown') return;
    const crop = CROPS[selectedTile.crop];
    // Use today's sell price
    const sellValue = todaysPrices[selectedTile.crop]?.sell || crop.harvestGain;
    updateCoins(sellValue);
    coinSparkle();
    // 40% chance to get a seed if fertilized
    if (selectedTile.fertilized) {
        if (Math.random() < 0.4) {
            inventory[selectedTile.crop] = (inventory[selectedTile.crop] || 0) + 1;
            showWarning(`Bonus! You got 1 ${crop.stages[3].emoji} ${crop.name} seed from fertilizer!`);
            renderInventory();
        }
    }
    const cropEl = selectedTile.el.querySelector('.crop');
    if (cropEl) animateHarvest(cropEl);
    selectedTile.state = 'empty';
    selectedTile.crop = null;
    selectedTile.stage = null;
    selectedTile.stageStart = null;
    selectedTile.plantedDay = null;
    selectedTile.watered = false;
    selectedTile.daysWatered = 0;
    selectedTile.fertilized = false;
    // Remove badges
    let fertBadge = selectedTile.el.querySelector('.fertilizer-badge');
    if (fertBadge) fertBadge.remove();
    selectedTile.el.classList.remove('planted', 'grown', 'glow');
    setTimeout(() => { selectedTile.el.innerHTML = ''; }, 500);
    checkGameOver();
}

function updateCrops() {
    tiles.forEach(tile => {
        // Remove old badges and classes
        let waterBadge = tile.el.querySelector('.water-badge');
        if (waterBadge) waterBadge.remove();
        tile.el.classList.remove('watered-tile');
        let fertBadge = tile.el.querySelector('.fertilizer-badge');
        if (fertBadge) fertBadge.remove();
        // Show water badge if watered today
        if (tile.state === 'planted' && tile.watered) {
            let badge = document.createElement('span');
            badge.className = 'water-badge';
            badge.innerHTML = '💧';
            badge.style.position = 'absolute';
            badge.style.right = '2px';
            badge.style.top = '2px';
            badge.style.fontSize = '1.3rem';
            badge.style.pointerEvents = 'none';
            badge.style.filter = 'drop-shadow(0 0 6px #81c784)';
            badge.style.animation = 'water-badge-pop 0.7s cubic-bezier(.68,-0.55,.27,1.55)';
            tile.el.appendChild(badge);
            tile.el.classList.add('watered-tile');
        }
        if (tile.state === 'planted' && tile.stageStart && tile.crop) {
            const crop = CROPS[tile.crop];
            const plantedDay = tile.plantedDay || dayCount;
            // Calculate days passed: 1 per day, +1 for each day watered
            let daysPassed = (dayCount - plantedDay) + tile.daysWatered;
            if (!tile.plantedDay) tile.plantedDay = dayCount;
            const totalStages = crop.stages.length;
            const daysPerStage = crop.growthDays / (totalStages - 1);
            let newStage = Math.min(Math.floor(daysPassed / daysPerStage), totalStages - 1);
            if (newStage !== tile.stage) {
                tile.stage = newStage;
                tile.el.innerHTML = `<span class="crop">${crop.stages[tile.stage].emoji}</span>`;
                tile.el.classList.remove('grown', 'glow');
                tile.el.classList.add('planted');
                addSparkle(tile.el);
                tile.el.querySelector('.crop').animate([
                    { transform: 'scale(1.2)' },
                    { transform: 'scale(0.9)' },
                    { transform: 'scale(1)' }
                ], { duration: 300, easing: 'ease-out' });
            }
            if (tile.stage === totalStages - 1) {
                tile.state = 'grown';
                tile.el.classList.remove('planted');
                tile.el.classList.add('grown', 'glow');
            }
        }
        // Show fertilizer badge if fertilized
        if ((tile.state === 'planted' || tile.state === 'grown') && tile.fertilized) {
            let badge = document.createElement('span');
            badge.className = 'fertilizer-badge';
            badge.innerHTML = '🧪';
            badge.style.position = 'absolute';
            badge.style.left = '2px';
            badge.style.top = '2px';
            badge.style.fontSize = '1.3rem';
            badge.style.pointerEvents = 'none';
            badge.style.filter = 'drop-shadow(0 0 8px #b39ddb)';
            badge.style.background = 'rgba(255,255,255,0.7)';
            badge.style.borderRadius = '50%';
            badge.style.padding = '1px 3px 0 3px';
            badge.style.boxShadow = '0 0 8px #b39ddb55';
            tile.el.appendChild(badge);
        }
    });
    setTimeout(attachCropTooltipEvents, 0);
}

// Add global tint overlay to the body if not present
function ensureGlobalTint() {
    let tint = document.getElementById('global-tint');
    if (!tint) {
        tint = document.createElement('div');
        tint.id = 'global-tint';
        tint.style.position = 'fixed';
        tint.style.top = '0';
        tint.style.left = '0';
        tint.style.width = '100vw';
        tint.style.height = '100vh';
        tint.style.pointerEvents = 'none';
        tint.style.zIndex = '10000';
        tint.style.transition = 'background 0.8s cubic-bezier(.4,0,.2,1), opacity 0.8s cubic-bezier(.4,0,.2,1)';
        document.body.appendChild(tint);
    }
    return tint;
}

function updateDayNightCycle() {
    const now = Date.now();
    const delta = (now - lastUpdateTime) / 1000; // seconds
    lastUpdateTime = now;
    timeOfDay += delta / DAY_LENGTH;
    if (timeOfDay >= 1) {
        timeOfDay = 0;
        dayCount++;
        startNewDay();
        newDayMarketUpdate();
    }
    // Calculate current phase and clock
    let phase = '';
    let icon = '';
    let phaseStart = 0;
    let phaseEnd = 1;
    let phases = [
        { name: 'Morning', icon: '🌅', start: 0.0, end: 0.2 },
        { name: 'Noon', icon: '🌞', start: 0.2, end: 0.4 },
        { name: 'Afternoon', icon: '☀️', start: 0.4, end: 0.6 },
        { name: 'Evening', icon: '🌇', start: 0.6, end: 0.8 },
        { name: 'Night', icon: '🌙', start: 0.8, end: 1.0 }
    ];
    for (let p of phases) {
        if (timeOfDay >= p.start && timeOfDay < p.end) {
            phase = p.name;
            icon = p.icon;
            phaseStart = p.start;
            phaseEnd = p.end;
            break;
        }
    }
    // Calculate seconds in current phase
    const phaseLength = (phaseEnd - phaseStart) * DAY_LENGTH;
    const secondsInPhase = Math.floor((timeOfDay - phaseStart) * DAY_LENGTH);
    const totalSeconds = Math.floor(timeOfDay * DAY_LENGTH);
    // Format clock as mm:ss
    function formatClock(secs) {
        const m = Math.floor(secs / 60).toString().padStart(2, '0');
        const s = (secs % 60).toString().padStart(2, '0');
        return `${m}:${s}`;
    }
    dayNightIndicator.innerHTML = `<span class="icon">${icon}</span> Day ${dayCount} - ${phase} <span class="clock">${formatClock(totalSeconds)}</span>`;
    // Overlay for day-night effect
    let overlay = document.getElementById('day-night-overlay');
    if (!overlay) {
        overlay = document.createElement('div');
        overlay.id = 'day-night-overlay';
        farmGrid.appendChild(overlay);
    }
    // More nuanced overlay for each phase
    let overlayStyles = {
        'Morning': { bg: '#fffde7', opacity: 0.10 },
        'Noon':    { bg: '#fffde7', opacity: 0.03 },
        'Afternoon': { bg: '#ffe082', opacity: 0.08 },
        'Evening': { bg: '#fbc02d', opacity: 0.18 },
        'Night':   { bg: '#263238', opacity: 0.45 }
    };
    let style = overlayStyles[phase] || { bg: '#fffde7', opacity: 0.1 };
    overlay.style.background = style.bg;
    overlay.style.opacity = style.opacity;
    updateSunMoon(phase);
    // Global tint for the whole screen
    const tint = ensureGlobalTint();
    const tintStyles = {
        'Morning':   { bg: 'rgba(255, 245, 220, 0.10)' },
        'Noon':      { bg: 'rgba(255, 255, 255, 0.01)' },
        'Afternoon': { bg: 'rgba(255, 200, 100, 0.10)' },
        'Evening':   { bg: 'rgba(255, 170, 80, 0.18)' },
        'Night':     { bg: 'rgba(30, 40, 80, 0.38)' }
    };
    tint.style.background = (tintStyles[phase] || tintStyles['Morning']).bg;
    tint.style.opacity = '1';
    checkGameOver();
}

// Add squish to buttons
['plant-btn', 'harvest-btn'].forEach(id => {
    const btn = document.getElementById(id);
    btn.addEventListener('click', () => squishButton(btn));
});

plantBtn.addEventListener('click', plantCrop);
harvestBtn.addEventListener('click', harvestCrop);

// Patch the game loop to allow stopping
let _gameLoopId = null;
function gameLoop() {
    if (hasWon) return;
    updateCrops();
    updateDayNightCycle();
    window._gameLoopId = requestAnimationFrame(gameLoop);
}

// On load, set initial coins and render market and inventory
window.addEventListener('DOMContentLoaded', () => {
    coinAmount.textContent = coins;
    renderMarket();
    renderInventory();
    updateCropButtonsState();
    if (coins >= 1000000 && !hasWon) {
        showWinScreen();
    }
});

createGrid();
gameLoop();

// Add this function to update crop button states
function updateCropButtonsState() {
    cropBtns.forEach(btn => {
        const cropId = btn.dataset.crop;
        if (!inventory[cropId] || inventory[cropId] < 1) {
            btn.disabled = true;
            btn.classList.add('disabled-crop');
        } else {
            btn.disabled = false;
            btn.classList.remove('disabled-crop');
        }
    });
}

// Add End Day button in the controls
const controlsDiv = document.getElementById('controls');
const endDayBtn = document.createElement('button');
endDayBtn.id = 'end-day-btn';
endDayBtn.textContent = 'End Day';
controlsDiv.appendChild(endDayBtn);

// --- Raccoon Event Logic ---
function raccoonEvent(onDone) {
    // 30% chance
    if (Math.random() > 0.3) { onDone(); return; }
    // Find all planted crops (not empty, not null, not grown-only)
    const plantedTiles = tiles.filter(tile => tile.state === 'planted' && tile.crop);
    if (plantedTiles.length === 0) { onDone(); return; }
    // Pick random number (1 to all)
    const numVictims = Math.max(1, Math.floor(Math.random() * plantedTiles.length) + 1);
    // Shuffle and pick
    const shuffled = plantedTiles.slice().sort(() => Math.random() - 0.5);
    const victims = shuffled.slice(0, numVictims);
    let finished = 0;
    victims.forEach((tile, idx) => {
        // Create raccoon emoji
        const raccoon = document.createElement('span');
        raccoon.className = 'raccoon-emoji';
        raccoon.innerHTML = '🦝';
        raccoon.style.position = 'absolute';
        raccoon.style.left = '-60px';
        raccoon.style.top = '-8px';
        raccoon.style.fontSize = '2.9rem'; // Larger raccoon
        raccoon.style.pointerEvents = 'none';
        raccoon.style.zIndex = '20';
        raccoon.style.filter = 'drop-shadow(0 4px 16px #888)';
        tile.el.appendChild(raccoon);
        // Animate raccoon in: bounce in, then playful wiggle, then pause, then jump and fade out
        gsap.fromTo(raccoon, 
            { x: -60, scale: 0.7, opacity: 0.7, rotate: -10 },
            { x: 18, scale: 1.18, opacity: 1, rotate: 0, duration: 0.48, ease: 'back.out(1.7)',
                onComplete: () => {
                    // Playful wiggle
                    gsap.to(raccoon, { rotate: 18, duration: 0.13, yoyo: true, repeat: 1, ease: 'power1.inOut',
                        onComplete: () => {
                            gsap.to(raccoon, { rotate: -14, duration: 0.13, yoyo: true, repeat: 1, ease: 'power1.inOut',
                                onComplete: () => {
                                    // Pause, then crop disappears
                                    setTimeout(() => {
                                        const cropEl = tile.el.querySelector('.crop');
                                        if (cropEl) {
                                            gsap.to(cropEl, { scale: 0, opacity: 0, duration: 0.32, ease: 'back.in', onComplete: () => {
                                                cropEl.remove();
                                            }});
                                        }
                                        // Raccoon jumps and fades out
                                        gsap.to(raccoon, { x: 60, y: -30, scale: 1.25, opacity: 0, rotate: 30, duration: 0.55, ease: 'power2.in', delay: 0.18, onComplete: () => {
                                            raccoon.remove();
                                            // Remove crop from tile
                                            tile.state = 'empty';
                                            tile.crop = null;
                                            tile.stage = null;
                                            tile.stageStart = null;
                                            tile.plantedDay = null;
                                            tile.watered = false;
                                            tile.daysWatered = 0;
                                            tile.fertilized = false;
                                            tile.el.classList.remove('planted', 'grown', 'glow');
                                            setTimeout(() => { tile.el.innerHTML = ''; }, 100);
                                            finished++;
                                            if (finished === victims.length) {
                                                showWarning(`A raccoon stole ${victims.length} crop${victims.length > 1 ? 's' : ''}!`);
                                                setTimeout(onDone, 700);
                                            }
                                        }});
                                    }, 180);
                                }
                            });
                        }
                    });
                }
            }
        );
    });
}

// Replace the existing End Day button event listener with this enhanced version
endDayBtn.addEventListener('click', () => {
    // Squish effect
    squishButton(endDayBtn);
    endDayBtn.disabled = true;
    // Raccoon event logic (wait for it to finish if triggered)
    raccoonEvent(() => {
        // Create or get animation overlay
        let anim = document.getElementById('day-pass-anim');
        if (!anim) {
            anim = document.createElement('div');
            anim.id = 'day-pass-anim';
            anim.style.position = 'fixed';
            anim.style.top = '0';
            anim.style.left = '0';
            anim.style.width = '100vw';
            anim.style.height = '100vh';
            anim.style.zIndex = '10001';
            anim.style.pointerEvents = 'none';
            anim.style.display = 'flex';
            anim.style.alignItems = 'center';
            anim.style.justifyContent = 'center';
            anim.style.fontSize = '5rem';
            document.body.appendChild(anim);
        }
        // Create icon container for smooth transitions
        const iconContainer = document.createElement('div');
        iconContainer.style.position = 'relative';
        iconContainer.style.width = '120px';
        iconContainer.style.height = '120px';
        iconContainer.style.display = 'flex';
        iconContainer.style.alignItems = 'center';
        iconContainer.style.justifyContent = 'center';
        
        // Reset animation state
        anim.style.transition = 'none';
        anim.style.background = 'rgba(0,0,0,0)';
        anim.style.opacity = '0';
        anim.innerHTML = '';
        anim.appendChild(iconContainer);
        
        // Smooth animation sequence
        const phases = [
            {
                name: 'morning',
                icon: '🌅',
                duration: 1100,
                background: 'linear-gradient(135deg, rgba(255, 245, 220, 0.3), rgba(255, 225, 180, 0.25))',
                shadow: '0 0 30px rgba(255, 200, 100, 0.4)'
            },
            {
                name: 'afternoon', 
                icon: '☀️',
                duration: 1100,
                background: 'radial-gradient(circle at 30% 30%, rgba(255, 255, 255, 0.4), rgba(255, 235, 150, 0.3))',
                shadow: '0 0 40px rgba(255, 255, 0, 0.6)'
            },
            {
                name: 'evening',
                icon: '🌇', 
                duration: 1100,
                background: 'linear-gradient(45deg, rgba(255, 150, 80, 0.4), rgba(255, 100, 50, 0.3))',
                shadow: '0 0 35px rgba(255, 100, 0, 0.5)'
            },
            {
                name: 'night',
                icon: '🌙',
                duration: 1100,
                background: 'radial-gradient(ellipse at top, rgba(30, 40, 80, 0.5), rgba(0, 0, 30, 0.4))',
                shadow: '0 0 50px rgba(173, 216, 230, 0.7)'
            }
        ];
        
        let currentPhase = 0;
        
        // Start the animation
        setTimeout(() => {
            anim.style.transition = 'opacity 0.8s cubic-bezier(0.25, 0.46, 0.45, 0.94)';
            anim.style.opacity = '1';
            
            function transitionToPhase(index) {
                if (index >= phases.length) {
                    // End animation - fade to morning
                    anim.style.transition = 'all 1s cubic-bezier(0.25, 0.46, 0.45, 0.94)';
                    anim.style.background = 'rgba(255, 245, 220, 0.2)';
                    
                    setTimeout(() => {
                        anim.style.transition = 'opacity 0.8s cubic-bezier(0.4, 0, 0.2, 1)';
                        anim.style.opacity = '0';
                        
                        setTimeout(() => {
                            endDayBtn.disabled = false;
                        }, 800);
                    }, 400);
                    return;
                }
                
                const phase = phases[index];
                
                // Smooth background transition
                anim.style.transition = 'background 1.2s cubic-bezier(0.25, 0.46, 0.45, 0.94)';
                anim.style.background = phase.background;
                
                // Create new icon with entrance animation
                const newIcon = document.createElement('div');
                newIcon.innerHTML = phase.icon;
                newIcon.style.position = 'absolute';
                newIcon.style.textShadow = phase.shadow;
                newIcon.style.opacity = '0';
                newIcon.style.transform = 'scale(0.7) translateY(20px)';
                newIcon.style.transition = 'all 0.8s cubic-bezier(0.34, 1.56, 0.64, 1)';
                newIcon.classList.add(`phase-${phase.name}`);
                
                // Remove old icon with exit animation
                const oldIcon = iconContainer.querySelector('div');
                if (oldIcon) {
                    oldIcon.style.transition = 'all 0.6s cubic-bezier(0.55, 0.085, 0.68, 0.53)';
                    oldIcon.style.opacity = '0';
                    oldIcon.style.transform = 'scale(0.5) translateY(-30px)';
                    setTimeout(() => oldIcon.remove(), 600);
                }
                
                iconContainer.appendChild(newIcon);
                
                // Trigger entrance animation
                setTimeout(() => {
                    newIcon.style.opacity = '1';
                    newIcon.style.transform = 'scale(1) translateY(0)';
                }, 100);
                
                // Add phase-specific continuous animation
                setTimeout(() => {
                    newIcon.style.animation = `phase-${phase.name}-loop 2s ease-in-out infinite`;
                }, 800);
                
                // Move to next phase
                setTimeout(() => {
                    transitionToPhase(index + 1);
                }, phase.duration);
            }
            
            transitionToPhase(0);
        }, 100);
        
        // Actually end the day
        dayCount++;
        timeOfDay = 0;
        startNewDay();
        newDayMarketUpdate();
        updateCrops();
        updateDayNightCycle();
    });
});

// Add smooth CSS animations for the enhanced day passing animation
const style = document.createElement('style');
style.textContent = `
    /* Smooth continuous phase animations */
    @keyframes phase-morning-loop {
        0%, 100% { 
            transform: scale(1) translateY(0px);
            filter: brightness(1);
        }
        50% { 
            transform: scale(1.05) translateY(-2px);
            filter: brightness(1.1);
        }
    }
    
    @keyframes phase-afternoon-loop {
        0%, 100% { 
            transform: scale(1) rotate(0deg);
            filter: brightness(1) drop-shadow(0 0 5px rgba(255,255,0,0.3));
        }
        25% { 
            transform: scale(1.03) rotate(90deg);
            filter: brightness(1.15) drop-shadow(0 0 10px rgba(255,255,0,0.5));
        }
        50% { 
            transform: scale(1.06) rotate(180deg);
            filter: brightness(1.2) drop-shadow(0 0 15px rgba(255,255,0,0.7));
        }
        75% { 
            transform: scale(1.03) rotate(270deg);
            filter: brightness(1.15) drop-shadow(0 0 10px rgba(255,255,0,0.5));
        }
    }
    
    @keyframes phase-evening-loop {
        0%, 100% { 
            transform: scale(1) translateY(0px);
            filter: hue-rotate(0deg) brightness(1);
        }
        33% { 
            transform: scale(1.04) translateY(-3px);
            filter: hue-rotate(10deg) brightness(1.1);
        }
        66% { 
            transform: scale(1.02) translateY(-1px);
            filter: hue-rotate(-5deg) brightness(1.05);
        }
    }
    
    @keyframes phase-night-loop {
        0%, 100% { 
            transform: scale(1) translateY(0px);
            filter: brightness(1) drop-shadow(0 0 8px rgba(173,216,230,0.4));
        }
        50% { 
            transform: scale(1.08) translateY(-4px);
            filter: brightness(1.2) drop-shadow(0 0 20px rgba(173,216,230,0.8));
        }
    }
    
    #day-pass-anim {
        backdrop-filter: blur(1px);
        will-change: background, opacity;
    }
    
    #day-pass-anim div {
        will-change: transform, opacity, filter;
    }

    /* Minimal price indicator styles */
    .price-indicator {
        display: inline-block;
        font-size: 1em;
        vertical-align: middle;
        font-weight: bold;
        margin-left: 2px;
        margin-right: 0;
        line-height: 1;
        padding: 0;
        background: none;
        box-shadow: none;
    }
    .item-price {
        transition: color 0.3s;
        font-weight: 700;
        letter-spacing: 0.01em;
    }
`;
document.head.appendChild(style);

// Unified drag state for seed, water, fertilizer, and tool
let draggingType = null; // 'seed' | 'water' | 'fertilizer' | 'tool' | null
let dragIcon = null;
let dragSeedType = null;
let dragOverPlantTile = null;
let dragOverWaterTile = null;
let dragOverFertilizerTile = null;
let harvestedTilesThisDrag = new Set();

function enableInventoryDrag() {
    const invList = document.getElementById('inventory-list');
    invList.querySelectorAll('li').forEach(li => {
        // Check if this is a crop
        const crop = Object.keys(CROPS).find(c => li.textContent.includes(CROPS[c].stages[3].emoji) || li.textContent.toLowerCase().includes(CROPS[c].name.toLowerCase()));
        if (crop && inventory[crop] > 0) {
            li.style.cursor = 'grab';
            li.onmousedown = (e) => {
                if (draggingType) return;
                e.preventDefault();
                draggingType = 'seed';
                dragSeedType = crop;
                // Create drag icon
                dragIcon = document.createElement('div');
                dragIcon.className = 'drag-seed-icon';
                dragIcon.innerHTML = `${CROPS[crop].stages[3].emoji} <span style="font-size:1rem;">${CROPS[crop].name}</span>`;
                dragIcon.style.position = 'fixed';
                dragIcon.style.pointerEvents = 'none';
                dragIcon.style.fontSize = '2.2rem';
                dragIcon.style.zIndex = '10010';
                dragIcon.style.transition = 'transform 0.18s cubic-bezier(.4,0,.2,1), opacity 0.18s cubic-bezier(.4,0,.2,1)';
                dragIcon.style.background = 'rgba(255,255,255,0.85)';
                dragIcon.style.borderRadius = '16px';
                dragIcon.style.padding = '6px 16px 6px 12px';
                dragIcon.style.boxShadow = '0 4px 16px #aed581cc';
                document.body.appendChild(dragIcon);
                moveSeedDragIcon(e);
                document.body.style.userSelect = 'none';
            };
        } else if (li.textContent.includes('💧') && inventory.water > 0) {
            li.style.cursor = 'grab';
            li.onmousedown = (e) => {
                if (draggingType) return;
                e.preventDefault();
                draggingType = 'water';
                // Create drag icon
                dragIcon = document.createElement('div');
                dragIcon.className = 'drag-water-icon';
                dragIcon.innerHTML = '💧';
                dragIcon.style.position = 'fixed';
                dragIcon.style.pointerEvents = 'none';
                dragIcon.style.fontSize = '2.5rem';
                dragIcon.style.zIndex = '10010';
                dragIcon.style.transition = 'transform 0.18s cubic-bezier(.4,0,.2,1), opacity 0.18s cubic-bezier(.4,0,.2,1)';
                document.body.appendChild(dragIcon);
                moveWaterDragIcon(e);
                document.body.style.userSelect = 'none';
            };
        } else if (li.textContent.includes('🧪') && inventory.fertilizer > 0) {
            li.style.cursor = 'grab';
            li.onmousedown = (e) => {
                if (draggingType) return;
                e.preventDefault();
                draggingType = 'fertilizer';
                // Create drag icon
                dragIcon = document.createElement('div');
                dragIcon.className = 'drag-fertilizer-icon';
                dragIcon.innerHTML = '🧪';
                dragIcon.style.position = 'fixed';
                dragIcon.style.pointerEvents = 'none';
                dragIcon.style.fontSize = '2.5rem';
                dragIcon.style.zIndex = '10010';
                dragIcon.style.transition = 'transform 0.18s cubic-bezier(.4,0,.2,1), opacity 0.18s cubic-bezier(.4,0,.2,1)';
                dragIcon.style.background = 'rgba(255,255,255,0.85)';
                dragIcon.style.borderRadius = '16px';
                dragIcon.style.padding = '6px 16px 6px 12px';
                dragIcon.style.boxShadow = '0 4px 16px #b39ddbcc';
                document.body.appendChild(dragIcon);
                moveFertilizerDragIcon(e);
                document.body.style.userSelect = 'none';
            };
        } else if (li.textContent.includes('🪓') && inventory.tool > 0) {
            li.style.cursor = 'grab';
            li.onmousedown = (e) => {
                if (draggingType) return;
                e.preventDefault();
                draggingType = 'tool';
                harvestedTilesThisDrag = new Set();
                // Create drag icon
                dragIcon = document.createElement('div');
                dragIcon.className = 'drag-tool-icon';
                dragIcon.innerHTML = '🪓';
                dragIcon.style.position = 'fixed';
                dragIcon.style.pointerEvents = 'none';
                dragIcon.style.fontSize = '2.5rem';
                dragIcon.style.zIndex = '10010';
                dragIcon.style.transition = 'transform 0.18s cubic-bezier(.4,0,.2,1), opacity 0.18s cubic-bezier(.4,0,.2,1)';
                // Remove any background, border, padding, or box shadow
                dragIcon.style.background = 'none';
                dragIcon.style.borderRadius = '0';
                dragIcon.style.padding = '0';
                dragIcon.style.boxShadow = 'none';
                document.body.appendChild(dragIcon);
                moveToolDragIcon(e);
                document.body.style.userSelect = 'none';
            };
        } else {
            li.style.cursor = '';
            li.onmousedown = null;
        }
    });
}

function moveSeedDragIcon(e) {
    if (draggingType === 'seed' && dragIcon) {
        dragIcon.style.left = (e.clientX - 30) + 'px';
        dragIcon.style.top = (e.clientY - 30) + 'px';
        dragIcon.style.opacity = '0.97';
        dragIcon.style.transform = 'scale(1.08)';
    }
}
function moveWaterDragIcon(e) {
    if (draggingType === 'water' && dragIcon) {
        dragIcon.style.left = (e.clientX - 20) + 'px';
        dragIcon.style.top = (e.clientY - 20) + 'px';
        dragIcon.style.opacity = '0.96';
        dragIcon.style.transform = 'scale(1.12)';
    }
}
function moveFertilizerDragIcon(e) {
    if (draggingType === 'fertilizer' && dragIcon) {
        dragIcon.style.left = (e.clientX - 20) + 'px';
        dragIcon.style.top = (e.clientY - 20) + 'px';
        dragIcon.style.opacity = '0.96';
        dragIcon.style.transform = 'scale(1.12)';
    }
}
function moveToolDragIcon(e) {
    if (draggingType === 'tool' && dragIcon) {
        dragIcon.style.left = (e.clientX - 20) + 'px';
        dragIcon.style.top = (e.clientY - 20) + 'px';
        dragIcon.style.opacity = '0.96';
        dragIcon.style.transform = 'scale(1.12)';
    }
}
window.addEventListener('mousemove', (e) => {
    if (draggingType === 'seed') {
        moveSeedDragIcon(e);
        // Highlight tile under cursor
        let found = null;
        tiles.forEach(tile => {
            const tileRect = tile.el.getBoundingClientRect();
            tile.el.classList.remove('seed-drag-hover');
            if (
                e.clientX >= tileRect.left &&
                e.clientX <= tileRect.right &&
                e.clientY >= tileRect.top &&
                e.clientY <= tileRect.bottom
            ) {
                found = tile;
            }
        });
        if (
            found &&
            found.state === 'empty' &&
            inventory[dragSeedType] > 0
        ) {
            found.el.classList.add('seed-drag-hover');
            dragOverPlantTile = found;
        } else {
            dragOverPlantTile = null;
        }
    } else if (draggingType === 'water') {
        moveWaterDragIcon(e);
        // Highlight tile under cursor
        let found = null;
        tiles.forEach(tile => {
            const tileRect = tile.el.getBoundingClientRect();
            tile.el.classList.remove('water-drag-hover');
            if (
                e.clientX >= tileRect.left &&
                e.clientX <= tileRect.right &&
                e.clientY >= tileRect.top &&
                e.clientY <= tileRect.bottom
            ) {
                found = tile;
            }
        });
        if (
            found &&
            found.state === 'planted' &&
            !found.watered &&
            inventory.water > 0
        ) {
            found.el.classList.add('water-drag-hover');
            dragOverWaterTile = found;
        } else {
            dragOverWaterTile = null;
        }
    } else if (draggingType === 'fertilizer') {
        moveFertilizerDragIcon(e);
        // Highlight tile under cursor
        let found = null;
        tiles.forEach(tile => {
            const tileRect = tile.el.getBoundingClientRect();
            tile.el.classList.remove('fertilizer-drag-hover');
            if (
                e.clientX >= tileRect.left &&
                e.clientX <= tileRect.right &&
                e.clientY >= tileRect.top &&
                e.clientY <= tileRect.bottom
            ) {
                found = tile;
            }
        });
        if (
            found &&
            (found.state === 'planted' || found.state === 'grown') &&
            !found.fertilized &&
            inventory.fertilizer > 0
        ) {
            found.el.classList.add('fertilizer-drag-hover');
            dragOverFertilizerTile = found;
        } else {
            dragOverFertilizerTile = null;
        }
    } else if (draggingType === 'tool') {
        moveToolDragIcon(e);
        // Harvest matured crops under cursor, only once per drag
        let found = null;
        tiles.forEach(tile => {
            const tileRect = tile.el.getBoundingClientRect();
            tile.el.classList.remove('tool-drag-hover');
            if (
                e.clientX >= tileRect.left &&
                e.clientX <= tileRect.right &&
                e.clientY >= tileRect.top &&
                e.clientY <= tileRect.bottom
            ) {
                found = tile;
            }
        });
        if (
            found &&
            found.state === 'grown' &&
            !harvestedTilesThisDrag.has(found)
        ) {
            // Harvest this tile
            harvestTileWithTool(found);
            harvestedTilesThisDrag.add(found);
            found.el.classList.add('tool-drag-hover');
        }
    }
});
window.addEventListener('mouseup', (e) => {
    if (draggingType === 'seed') {
        if (dragIcon) dragIcon.remove();
        dragIcon = null;
        document.body.style.userSelect = '';
        // Plant the crop if valid
        if (
            dragOverPlantTile &&
            dragOverPlantTile.state === 'empty' &&
            inventory[dragSeedType] > 0
        ) {
            inventory[dragSeedType] -= 1;
            renderInventory();
            dragOverPlantTile.state = 'planted';
            dragOverPlantTile.crop = dragSeedType;
            dragOverPlantTile.stage = 0;
            dragOverPlantTile.stageStart = Date.now();
            dragOverPlantTile.plantedDay = dayCount;
            dragOverPlantTile.el.classList.add('planted');
            dragOverPlantTile.el.innerHTML = `<span class="crop">${CROPS[dragSeedType].stages[0].emoji}</span>`;
            // Planting pop animation
            let cropEl = dragOverPlantTile.el.querySelector('.crop');
            if (cropEl) {
                cropEl.animate([
                    { transform: 'scale(0.7)', opacity: 0.5 },
                    { transform: 'scale(1.2)', opacity: 1 },
                    { transform: 'scale(1)', opacity: 1 }
                ], { duration: 400, easing: 'cubic-bezier(.68,-0.55,.27,1.55)' });
                addSparkle(dragOverPlantTile.el);
            }
        } else if (dragOverPlantTile) {
            // Subtle shake if not allowed
            dragOverPlantTile.el.classList.add('shake-tile');
            setTimeout(() => dragOverPlantTile.el.classList.remove('shake-tile'), 400);
        }
        // Remove all highlights
        tiles.forEach(tile => tile.el.classList.remove('seed-drag-hover'));
        draggingType = null;
        dragSeedType = null;
        dragOverPlantTile = null;
    } else if (draggingType === 'water') {
        if (dragIcon) dragIcon.remove();
        dragIcon = null;
        document.body.style.userSelect = '';
        // Water the tile if valid
        if (
            dragOverWaterTile &&
            dragOverWaterTile.state === 'planted' &&
            !dragOverWaterTile.watered &&
            inventory.water > 0
        ) {
            inventory.water -= 1;
            dragOverWaterTile.watered = true;
            dragOverWaterTile.daysWatered += 1;
            renderInventory();
            // Persistent droplet indicator
            let badge = dragOverWaterTile.el.querySelector('.water-badge');
            if (!badge) {
                badge = document.createElement('span');
                badge.className = 'water-badge';
                badge.innerHTML = '💧';
                badge.style.position = 'absolute';
                badge.style.right = '2px';
                badge.style.top = '2px';
                badge.style.fontSize = '1.3rem';
                badge.style.pointerEvents = 'none';
                badge.style.filter = 'drop-shadow(0 0 8px #81c784)';
                badge.style.animation = 'water-badge-pop 0.7s cubic-bezier(.68,-0.55,.27,1.55)';
                dragOverWaterTile.el.appendChild(badge);
            }
            dragOverWaterTile.el.classList.add('watered-tile');
            // Enhanced splash animation
            let splash = document.createElement('span');
            splash.className = 'water-splash';
            splash.innerHTML = '💦';
            splash.style.position = 'absolute';
            splash.style.left = '50%';
            splash.style.top = '50%';
            splash.style.transform = 'translate(-50%, -50%) scale(0.7)';
            splash.style.fontSize = '2.5rem';
            splash.style.opacity = '0.85';
            splash.style.pointerEvents = 'none';
            splash.style.filter = 'drop-shadow(0 0 12px #81c784cc)';
            dragOverWaterTile.el.appendChild(splash);
            setTimeout(() => {
                splash.style.transition = 'all 0.8s cubic-bezier(.68,-0.55,.27,1.55)';
                splash.style.transform = 'translate(-50%, -90%) scale(1.25)';
                splash.style.opacity = '0';
            }, 10);
            setTimeout(() => splash.remove(), 900);
            // Ripple effect
            let ripple = document.createElement('span');
            ripple.className = 'water-ripple';
            ripple.style.position = 'absolute';
            ripple.style.left = '50%';
            ripple.style.top = '50%';
            ripple.style.transform = 'translate(-50%, -50%) scale(0.7)';
            ripple.style.width = '38px';
            ripple.style.height = '38px';
            ripple.style.borderRadius = '50%';
            ripple.style.background = 'rgba(129,199,132,0.18)';
            ripple.style.opacity = '0.7';
            ripple.style.pointerEvents = 'none';
            dragOverWaterTile.el.appendChild(ripple);
            setTimeout(() => {
                ripple.style.transition = 'all 0.7s cubic-bezier(.68,-0.55,.27,1.55)';
                ripple.style.transform = 'translate(-50%, -50%) scale(1.7)';
                ripple.style.opacity = '0';
            }, 10);
            setTimeout(() => ripple.remove(), 800);
        }
        // Remove all highlights
        tiles.forEach(tile => tile.el.classList.remove('water-drag-hover'));
        draggingType = null;
        dragOverWaterTile = null;
    } else if (draggingType === 'fertilizer') {
        if (dragIcon) dragIcon.remove();
        dragIcon = null;
        document.body.style.userSelect = '';
        // Fertilize the tile if valid
        if (
            dragOverFertilizerTile &&
            (dragOverFertilizerTile.state === 'planted' || dragOverFertilizerTile.state === 'grown') &&
            !dragOverFertilizerTile.fertilized &&
            inventory.fertilizer > 0
        ) {
            inventory.fertilizer -= 1;
            dragOverFertilizerTile.fertilized = true;
            renderInventory();
            // Add fertilizer badge
            let badge = dragOverFertilizerTile.el.querySelector('.fertilizer-badge');
            if (!badge) {
                badge = document.createElement('span');
                badge.className = 'fertilizer-badge';
                badge.innerHTML = '🧪';
                badge.style.position = 'absolute';
                badge.style.left = '2px';
                badge.style.top = '2px';
                badge.style.fontSize = '1.3rem';
                badge.style.pointerEvents = 'none';
                badge.style.filter = 'drop-shadow(0 0 8px #b39ddb)';
                badge.style.background = 'rgba(255,255,255,0.7)';
                badge.style.borderRadius = '50%';
                badge.style.padding = '1px 3px 0 3px';
                badge.style.boxShadow = '0 0 8px #b39ddb55';
                dragOverFertilizerTile.el.appendChild(badge);
            }
        } else if (dragOverFertilizerTile) {
            // Subtle shake if not allowed
            dragOverFertilizerTile.el.classList.add('shake-tile');
            setTimeout(() => dragOverFertilizerTile.el.classList.remove('shake-tile'), 400);
        }
        // Remove all highlights
        tiles.forEach(tile => tile.el.classList.remove('fertilizer-drag-hover'));
        draggingType = null;
        dragOverFertilizerTile = null;
    } else if (draggingType === 'tool') {
        if (dragIcon) dragIcon.remove();
        dragIcon = null;
        document.body.style.userSelect = '';
        tiles.forEach(tile => tile.el.classList.remove('tool-drag-hover'));
        draggingType = null;
        harvestedTilesThisDrag = new Set();
    }
});

function harvestTileWithTool(tile) {
    if (!tile || tile.state !== 'grown') return;
    const crop = CROPS[tile.crop];
    // Use today's sell price
    const sellValue = todaysPrices[tile.crop]?.sell || crop.harvestGain;
    updateCoins(sellValue);
    coinSparkle();
    // 40% chance to get a seed if fertilized
    if (tile.fertilized) {
        if (Math.random() < 0.4) {
            inventory[tile.crop] = (inventory[tile.crop] || 0) + 1;
            showWarning(`Bonus! You got 1 ${crop.stages[3].emoji} ${crop.name} seed from fertilizer!`);
            renderInventory();
        }
    }
    // Coin pop animation
    let coinPop = document.createElement('span');
    coinPop.className = 'coin-pop';
    coinPop.innerHTML = '🪙';
    coinPop.style.position = 'absolute';
    coinPop.style.left = '50%';
    coinPop.style.top = '50%';
    coinPop.style.transform = 'translate(-50%, -50%) scale(1)';
    coinPop.style.fontSize = '2rem';
    coinPop.style.opacity = '1';
    coinPop.style.pointerEvents = 'none';
    coinPop.style.filter = 'drop-shadow(0 0 12px #ffd54fcc)';
    tile.el.appendChild(coinPop);
    setTimeout(() => {
        coinPop.style.transition = 'all 0.7s cubic-bezier(.68,-0.55,.27,1.55)';
        coinPop.style.transform = 'translate(-50%, -90%) scale(1.4)';
        coinPop.style.opacity = '0';
    }, 10);
    setTimeout(() => coinPop.remove(), 800);
    // Animate crop out
    const cropEl = tile.el.querySelector('.crop');
    if (cropEl) animateHarvest(cropEl);
    tile.state = 'empty';
    tile.crop = null;
    tile.stage = null;
    tile.stageStart = null;
    tile.plantedDay = null;
    tile.watered = false;
    tile.daysWatered = 0;
    tile.fertilized = false;
    // Remove badges
    let fertBadge = tile.el.querySelector('.fertilizer-badge');
    if (fertBadge) fertBadge.remove();
    tile.el.classList.remove('planted', 'grown', 'glow');
    setTimeout(() => { tile.el.innerHTML = ''; }, 500);
    checkGameOver();
}

// Reset watered status and remove indicators at new day
function startNewDay() {
    tiles.forEach(tile => {
        if (tile.state === 'planted') {
            tile.watered = false; // Reset watered status for new day
            let badge = tile.el.querySelector('.water-badge');
            if (badge) badge.remove();
            tile.el.classList.remove('watered-tile');
        }
    });
}

// Utility: get a random price from a range array (for future items)
function getRandomPriceFromRange(rangeArr) {
    return rangeArr[Math.floor(Math.random() * rangeArr.length)];
}

// --- Game Over Logic ---
function checkGameOver() {
    // Find the minimum crop price in the market
    const minCropPrice = Math.min(
        ...MARKET_ITEMS.filter(i => i.type === 'crop').map(i => i.price)
    );
    // Check if any crops are planted or grown
    const anyCrops = tiles.some(tile => tile.state === 'planted' || tile.state === 'grown');
    // Also check inventory for seeds
    const anySeeds = Object.keys(CROPS).some(crop => inventory[crop] > 0);
    if (coins < minCropPrice && !anyCrops && !anySeeds) {
        showGameOverScreen();
        return true;
    }
    return false;
}

function showGameOverScreen() {
    // Prevent multiple overlays
    if (document.getElementById('gameover-overlay')) return;
    // Overlay (blur and dim background, but not a full replacement)
    const overlay = document.createElement('div');
    overlay.id = 'gameover-overlay';
    overlay.style.position = 'fixed';
    overlay.style.top = '0';
    overlay.style.left = '0';
    overlay.style.width = '100vw';
    overlay.style.height = '100vh';
    overlay.style.background = 'rgba(255,255,255,0.45)';
    overlay.style.zIndex = '20000';
    overlay.style.display = 'flex';
    overlay.style.alignItems = 'center';
    overlay.style.justifyContent = 'center';
    overlay.style.backdropFilter = 'blur(7px)';
    overlay.style.transition = 'opacity 0.5s';
    overlay.style.pointerEvents = 'auto';
    // Dialog card
    const card = document.createElement('div');
    card.style.background = 'linear-gradient(135deg, #fffbe7 0%, #e3f0ff 100%)';
    card.style.borderRadius = '28px';
    card.style.boxShadow = '0 12px 40px rgba(60, 60, 80, 0.18), 0 4px 16px rgba(120, 120, 180, 0.10)';
    card.style.padding = '48px 38px 38px 38px';
    card.style.display = 'flex';
    card.style.flexDirection = 'column';
    card.style.alignItems = 'center';
    card.style.gap = '18px';
    card.style.maxWidth = '90vw';
    card.style.minWidth = '260px';
    card.style.position = 'relative';
    card.style.zIndex = '20001';
    card.style.pointerEvents = 'auto';
    // Sad emoji
    const emoji = document.createElement('div');
    emoji.textContent = '😢';
    emoji.style.fontSize = '4.2rem';
    emoji.style.marginBottom = '8px';
    emoji.style.filter = 'drop-shadow(0 2px 8px #bdbdbd)';
    // Title
    const title = document.createElement('div');
    title.textContent = "Didn't Make It";
    title.style.fontSize = '2.1rem';
    title.style.fontWeight = '800';
    title.style.color = '#e65100';
    title.style.letterSpacing = '0.5px';
    title.style.marginBottom = '2px';
    // Stats
    const stats = document.createElement('div');
    stats.style.fontSize = '1.18rem';
    stats.style.color = '#4a4a4a';
    stats.style.textAlign = 'center';
    stats.innerHTML = `Days Survived: <b>${dayCount}</b><br>Money Raised: <b>🪙 ${coins}</b>`;
    // Restart button
    const restartBtn = document.createElement('button');
    restartBtn.textContent = 'Restart Game';
    restartBtn.style.background = 'linear-gradient(90deg, #81c784 60%, #aed581 100%)';
    restartBtn.style.color = '#fff';
    restartBtn.style.border = 'none';
    restartBtn.style.borderRadius = '12px';
    restartBtn.style.padding = '12px 38px';
    restartBtn.style.fontSize = '1.15rem';
    restartBtn.style.fontWeight = 'bold';
    restartBtn.style.cursor = 'pointer';
    restartBtn.style.boxShadow = '0 2px 8px rgba(120,120,180,0.10)';
    restartBtn.style.transition = 'background 0.2s, transform 0.1s';
    restartBtn.style.marginTop = '18px';
    restartBtn.addEventListener('click', () => { window.location.reload(); });
    // Add sparkle
    const sparkle = document.createElement('span');
    sparkle.innerHTML = '✨';
    sparkle.style.position = 'absolute';
    sparkle.style.top = '18px';
    sparkle.style.right = '28px';
    sparkle.style.fontSize = '2.1rem';
    sparkle.style.opacity = '0.7';
    card.appendChild(sparkle);
    card.appendChild(emoji);
    card.appendChild(title);
    card.appendChild(stats);
    card.appendChild(restartBtn);
    overlay.appendChild(card);
    document.body.appendChild(overlay);
    setTimeout(() => { overlay.style.opacity = '1'; }, 30);
    // Prevent interaction with background
    overlay.addEventListener('click', (e) => { e.stopPropagation(); });
    card.addEventListener('click', (e) => { e.stopPropagation(); });
}

// --- WIN SCREEN OBSERVER ---
(function() {
    let observer = new MutationObserver(() => {
        const val = parseInt(coinAmount.textContent.replace(/[^\d]/g, ''));
        if (val >= 1000000 && !hasWon) {
            showWinScreen();
        }
    });
    observer.observe(coinAmount, { childList: true, characterData: true, subtree: true });
})();

// --- WIN SCREEN INTERVAL POLLING ---
setInterval(() => {
    // Try both the JS variable and the DOM value, just in case
    let val = typeof coins === 'number' ? coins : parseInt(coinAmount.textContent.replace(/[^\d]/g, ''));
    if (val >= 1000000 && !hasWon) {
        showWinScreen();
    }
}, 200);