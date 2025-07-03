// Game variables
let canvas, ctx, player, platforms, collectibles, obstacles;
let gameLoop, isGameRunning = false;
let score = 0;
const TOTAL_COLLECTIBLES = 8;

let particles = [];

// Player properties
const PLAYER = {
    x: 50,
    y: 200,
    width: 24,
    height: 24,
    speed: 6,         // Slightly increased speed
    jumpForce: -13,   // Stronger jump
    gravity: 0.6,     // Adjusted gravity
    velocityX: 0,
    velocityY: 0,
    isJumping: false,
    color: '#ff4d4d',
    trailPoints: []   // For motion trail
};

// Initialize game
function init() {
    resetGame();
    canvas = document.getElementById('gameCanvas');
    ctx = canvas.getContext('2d');

    // Hide message during initialization
    document.getElementById('message').style.display = 'none';

    canvas.width = 800;
    canvas.height = 500;
    
    // Reset game state
    player = { ...PLAYER };
    
    // More challenging platform layout
    platforms = [
        { x: 0, y: canvas.height - 40, width: canvas.width, height: 40 },      // Ground
        { x: 150, y: canvas.height - 120, width: 80, height: 15 },            // First jump (narrower)
        { x: 350, y: canvas.height - 190, width: 100, height: 15 },           // Middle platform
        { x: 550, y: canvas.height - 240, width: 90, height: 15 },            // Higher platform
        { x: 400, y: canvas.height - 320, width: 70, height: 15 },            // Upper middle (narrower)
        { x: 200, y: canvas.height - 280, width: 80, height: 15 },            // Upper left
        { x: 650, y: canvas.height - 350, width: 90, height: 15 },            // Highest right
        { x: 280, y: canvas.height - 400, width: 60, height: 15 }             // New challenging top platform
    ];
    
    // More challenging collectible placement
    collectibles = [
        { x: 180, y: canvas.height - 160, collected: false },     // Above first platform
        { x: 380, y: canvas.height - 230, collected: false },     // Middle challenge
        { x: 580, y: canvas.height - 280, collected: false },     // Higher challenge
        { x: 420, y: canvas.height - 360, collected: false },     // Upper middle reward
        { x: 220, y: canvas.height - 320, collected: false },     // Upper left challenge
        { x: 680, y: canvas.height - 390, collected: false },     // Highest right reward
        { x: 300, y: canvas.height - 440, collected: false },     // Top challenge
        { x: 750, y: canvas.height - 200, collected: false }      // Tricky side jump
    ];
    
    // More strategic obstacle placement
    obstacles = [
        { x: 280, y: canvas.height - 160, width: 20, height: 20 },    // Between first platforms
        { x: 480, y: canvas.height - 260, width: 20, height: 20 },    // Mid-height challenge
        { x: 320, y: canvas.height - 340, width: 20, height: 20 },    // Upper area guard
        { x: 600, y: canvas.height - 120, width: 20, height: 20 },    // Lower right challenge
        { x: 500, y: canvas.height - 380, width: 20, height: 20 }     // New high challenge
    ];
    
    score = 0;
    updateScore();
}

function skipToPattern() {
    isGameRunning = false;
    clearInterval(gameLoop);
    gameWon(); // Reuse the existing gameWon function
}

// Start game
function startGame() {
    if (!isGameRunning) {
        init();
        document.getElementById('startScreen').style.display = 'none';
        document.getElementById('message').style.display = 'none';
        isGameRunning = true;
        gameLoop = setInterval(update, 1000/60);
        
        // Add event listeners for controls
        window.addEventListener('keydown', handleKeyDown);
        window.addEventListener('keyup', handleKeyUp);
    }
}


// Handle keyboard input
function handleKeyDown(e) {
    switch(e.key) {
        case 'ArrowLeft':
        case 'a':
            player.velocityX = -player.speed;
            break;
        case 'ArrowRight':
        case 'd':
            player.velocityX = player.speed;
            break;
        case 'ArrowUp':
        case 'w':
        case ' ':
            if (!player.isJumping) {
                player.velocityY = player.jumpForce;
                player.isJumping = true;
            }
            break;
    }
}

function handleKeyUp(e) {
    switch(e.key) {
        case 'ArrowLeft':
        case 'ArrowRight':
        case 'a':
        case 'd':
            player.velocityX = 0;
            break;
    }
}

// Update game state
function update() {
    // Apply gravity
    player.velocityY += player.gravity;
    
    // Update player position
    player.x += player.velocityX;
    player.y += player.velocityY;
    
    // Add trail effect
    player.trailPoints.unshift({ x: player.x, y: player.y });
    if (player.trailPoints.length > 6) {
        player.trailPoints.pop();
    }
    
    // Check platform collisions with improved feel
    player.isJumping = true;
    platforms.forEach(platform => {
        if (checkCollision(player, platform)) {
            if (player.velocityY > 0 && player.y < platform.y + platform.height / 2) {
                player.y = platform.y - player.height;
                player.velocityY = 0;
                player.isJumping = false;
            }
        }
    });
    
    // Improved boundary checking
    if (player.x < 0) {
        player.x = 0;
        player.velocityX *= -0.5; // Bounce effect
    }
    if (player.x + player.width > canvas.width) {
        player.x = canvas.width - player.width;
        player.velocityX *= -0.5; // Bounce effect
    }
    
    // Check collectibles with particle effect
    collectibles.forEach((collectible, index) => {
        if (!collectible.collected && checkCollision(
            player,
            { x: collectible.x, y: collectible.y, width: 20, height: 20 }
        )) {
            collectible.collected = true;
            score++;
            updateScore();
            createCollectParticles(collectible.x, collectible.y);
            
            if (score === TOTAL_COLLECTIBLES) {
                gameWon();
            }
        }
    });
    
    // Check obstacles with improved collision
    obstacles.forEach(obstacle => {
        if (checkCollision(player, obstacle)) {
            createDeathParticles(player.x, player.y);
            gameOver();
        }
    });
    
    draw();
}

function draw() {
    // Clear canvas
    ctx.fillStyle = '#fff0f0';
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    
    // Draw platforms with gradient
    platforms.forEach(platform => {
        const gradient = ctx.createLinearGradient(
            platform.x, platform.y,
            platform.x, platform.y + platform.height
        );
        gradient.addColorStop(0, '#ff8e8e');
        gradient.addColorStop(1, '#ff6b6b');
        ctx.fillStyle = gradient;
        ctx.fillRect(platform.x, platform.y, platform.width, platform.height);
    });
    
    // Draw trail effect
    player.trailPoints.forEach((point, index) => {
        const alpha = (5 - index) / 5;
        ctx.fillStyle = `rgba(255, 77, 77, ${alpha * 0.3})`;
        ctx.beginPath();
        ctx.arc(
            point.x + player.width / 2,
            point.y + player.height / 2,
            player.width / 2 * (1 - index / 5),
            0,
            Math.PI * 2
        );
        ctx.fill();
    });
    
    // Draw player with shadow
    ctx.shadowColor = 'rgba(0, 0, 0, 0.2)';
    ctx.shadowBlur = 5;
    ctx.fillStyle = player.color;
    ctx.fillRect(player.x, player.y, player.width, player.height);
    ctx.shadowBlur = 0;
    
    // Draw collectibles with glow
    collectibles.forEach(collectible => {
        if (!collectible.collected) {
            ctx.shadowColor = '#ff6b6b';
            ctx.shadowBlur = 15;
            ctx.fillStyle = '#ff6b6b';
            ctx.beginPath();
            ctx.arc(collectible.x + 10, collectible.y + 10, 8, 0, Math.PI * 2);
            ctx.fill();
            ctx.shadowBlur = 0;
        }
    });
    
    // Draw obstacles with warning effect
    obstacles.forEach(obstacle => {
        ctx.fillStyle = '#000000';
        ctx.fillRect(obstacle.x, obstacle.y, obstacle.width, obstacle.height);
        
        // Warning glow
        const time = Date.now() / 500;
        const glowIntensity = (Math.sin(time) + 1) / 2;
        ctx.shadowColor = `rgba(255, 0, 0, ${glowIntensity * 0.5})`;
        ctx.shadowBlur = 10 * glowIntensity;
        ctx.fillRect(obstacle.x, obstacle.y, obstacle.width, obstacle.height);
        ctx.shadowBlur = 0;
    });
}

function createCollectParticles(x, y) {
    for (let i = 0; i < 10; i++) {
        const particle = {
            x: x + 10,
            y: y + 10,
            vx: (Math.random() - 0.5) * 5,
            vy: (Math.random() - 0.5) * 5,
            life: 1
        };
        particles.push(particle);
    }
}

// Death particles
function createDeathParticles(x, y) {
    for (let i = 0; i < 20; i++) {
        const particle = {
            x: x + player.width / 2,
            y: y + player.height / 2,
            vx: (Math.random() - 0.5) * 10,
            vy: (Math.random() - 0.5) * 10,
            life: 1
        };
        particles.push(particle);
    }
}

// Update particles in the update function
function updateParticles() {
    for (let i = particles.length - 1; i >= 0; i--) {
        const particle = particles[i];
        particle.x += particle.vx;
        particle.y += particle.vy;
        particle.life -= 0.02;
        
        if (particle.life <= 0) {
            particles.splice(i, 1);
        }
    }
}

// Draw particles in the draw function
function drawParticles() {
    particles.forEach(particle => {
        ctx.fillStyle = `rgba(255, 77, 77, ${particle.life})`;
        ctx.beginPath();
        ctx.arc(particle.x, particle.y, 3, 0, Math.PI * 2);
        ctx.fill();
    });
}

// Collision detection
function checkCollision(rect1, rect2) {
    return rect1.x < rect2.x + rect2.width &&
           rect1.x + rect1.width > rect2.x &&
           rect1.y < rect2.y + rect2.height &&
           rect1.y + rect1.height > rect2.y;
}

// Update score display
function updateScore() {
    document.getElementById('score').textContent = `⭐ ${score}/${TOTAL_COLLECTIBLES}`;
}

// Game over
// Game over
function gameOver() {
    isGameRunning = false;
    clearInterval(gameLoop);
    
    const message = document.getElementById('message');
    message.style.display = 'block'; // Only show message now
    message.textContent = "Oh no! Try again!";
    
    setTimeout(() => {
        message.style.display = 'none';
        document.getElementById('startScreen').style.display = 'flex';
    }, 2000);
}

// Pattern lock functionality
let pattern = [];
const correctPattern = [7, 4, 1, 5, 9, 6, 3];
let isPatternComplete = false;
let failedAttempts = 0;

// Game won
function gameWon() {
    isGameRunning = false;
    clearInterval(gameLoop);
    
    const gameContainer = document.getElementById('gameContainer');
    gameContainer.style.display = 'none';
    
    const patternContainer = document.createElement('div');
    patternContainer.id = 'patternPuzzleContainer';
    patternContainer.style.cssText = `
        position: fixed;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
        width: 900px;
        height: 520px;
        background: linear-gradient(145deg, rgba(255, 255, 255, 0.98), rgba(255, 240, 240, 0.98));
        border-radius: 30px;
        box-shadow: 0 10px 40px rgba(255, 77, 77, 0.2);
        padding: 30px;
        z-index: 1000;
        overflow: visible;
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-items: center;
        border: 1px solid rgba(255, 77, 77, 0.1);
        user-select: none;
    `;
    
    patternContainer.innerHTML = `
        <div style="
            text-align: center;
            padding: 20px;
            width: 100%;
            max-width: 600px;
            position: relative;
            user-select: none;
        ">
            <div style="
                position: absolute;
                top: 0;
                left: 0;
                right: 0;
                bottom: 0;
                background: radial-gradient(circle at 30% 20%, rgba(255, 182, 193, 0.1) 0%, transparent 60%),
                            radial-gradient(circle at 70% 80%, rgba(255, 192, 203, 0.1) 0%, transparent 60%);
                pointer-events: none;
                user-select: none;
            "></div>
            
            <h3 style="
                font-family: 'Dancing Script', cursive;
                font-size: 2.2em;
                color: #ff4d4d;
                margin-bottom: 15px;
                text-shadow: 2px 2px 4px rgba(255, 77, 77, 0.1);
                position: relative;
                user-select: none;
            ">One Last Challenge! ⭐</h3>
            
            <p style="
                font-size: 1.1em;
                color: #ff6b6b;
                margin-bottom: 25px;
                font-weight: 300;
                letter-spacing: 0.5px;
                user-select: none;
            ">Draw the pattern to unlock your special gift!</p>
            
            <div id="patternGrid" style="
                display: grid;
                grid-template-columns: repeat(3, 1fr);
                gap: 22px;
                margin: 30px auto;
                width: fit-content;
                position: relative;
                padding: 15px;
                background: rgba(255, 255, 255, 0.5);
                border-radius: 20px;
                box-shadow: inset 0 0 20px rgba(255, 77, 77, 0.05);
                user-select: none;
            ">
                ${Array(9).fill(0).map((_, i) => `
                    <div class="pattern-dot" data-index="${i + 1}" style="
                        width: 45px;
                        height: 45px;
                        position: relative;
                        user-select: none;
                    "></div>
                `).join('')}
            </div>
            
            <div class="hint-container" style="
                margin-top: 30px;
                display: flex;
                align-items: center;
                justify-content: center;
                gap: 15px;
                user-select: none;
            ">
                <button onclick="showHint()" style="
                    background: none;
                    border: 2px solid #ff4d4d;
                    color: #ff4d4d;
                    padding: 10px 20px;
                    border-radius: 25px;
                    cursor: pointer;
                    transition: all 0.3s ease;
                    font-size: 1em;
                    font-weight: 500;
                    letter-spacing: 0.5px;
                    user-select: none;
                ">Need a hint?</button>
                <div id="patternMessage" style="
                    min-height: 24px;
                    color: #ff4d4d;
                    font-size: 1em;
                    user-select: none;
                "></div>
            </div>
            
            <div id="successModal" style="
                display: none;
                margin-top: 25px;
                padding: 20px;
                background: rgba(255, 255, 255, 0.95);
                border-radius: 15px;
                box-shadow: 0 5px 20px rgba(255, 77, 77, 0.1);
                transform: scale(0.95);
                opacity: 0;
                transition: all 0.5s ease;
                user-select: none;
            ">
                <h3 style="
                    color: #ff4d4d;
                    font-size: 1.4em;
                    margin-bottom: 10px;
                    font-weight: 600;
                    user-select: none;
                ">Congratulations my StuntWoman! 🎉</h3>
                <p style="
                    color: #ff6b6b;
                    font-size: 1.2em;
                    font-weight: 300;
                    user-select: none;
                ">You've won 40 CS hours!<br>Idk why did I even do this! But I still love you so much :D</p>
            </div>
        </div>
    `;
    
    document.body.appendChild(patternContainer);
    
    const existingLines = document.getElementsByClassName('pattern-line');
    Array.from(existingLines).forEach(line => line.remove());
    
    initializePatternLock();
}

// Add a reset function to clean up when starting a new game
function resetGame() {
    const patternContainer = document.getElementById('patternPuzzleContainer');
    const hintDialog = document.getElementById('autoHintDialog');
    
    if (patternContainer) {
        patternContainer.remove();
    }
    if (hintDialog) {
        hintDialog.remove();
    }
    
    const gameContainer = document.getElementById('gameContainer');
    gameContainer.style.display = 'block';
    failedAttempts = 0;
}

function initializePatternLock() {
    const grid = document.getElementById('patternGrid');
    const dots = grid.getElementsByClassName('pattern-dot');
    let isDrawing = false;
    let currentLine = null;
    
    function updateLine(e) {
        if (!isDrawing || !currentLine) return;
        
        const gridRect = grid.getBoundingClientRect();
        const x = e.clientX - gridRect.left;
        const y = e.clientY - gridRect.top;
        
        const angle = Math.atan2(y - currentLine.y1, x - currentLine.x1);
        const length = Math.sqrt(Math.pow(x - currentLine.x1, 2) + Math.pow(y - currentLine.y1, 2));
        
        currentLine.style.width = `${length}px`;
        currentLine.style.transform = `rotate(${angle}rad)`;
        currentLine.style.position = 'absolute';
        currentLine.style.zIndex = '1';
    }
    
    Array.from(dots).forEach(dot => {
        dot.addEventListener('mousedown', (e) => {
            isDrawing = true;
            pattern = [parseInt(dot.dataset.index)];
            dot.classList.add('active');
            
            const dotRect = dot.getBoundingClientRect();
            const gridRect = grid.getBoundingClientRect();
            currentLine = document.createElement('div');
            currentLine.className = 'pattern-line';
            currentLine.style.left = `${dotRect.left - gridRect.left + dot.offsetWidth / 2}px`;
            currentLine.style.top = `${dotRect.top - gridRect.top + dot.offsetHeight / 2}px`;
            currentLine.x1 = dotRect.left - gridRect.left + dot.offsetWidth / 2;
            currentLine.y1 = dotRect.top - gridRect.top + dot.offsetHeight / 2;
            grid.appendChild(currentLine);
        });
        
        dot.addEventListener('mouseenter', (e) => {
            if (!isDrawing) return;
            
            const index = parseInt(dot.dataset.index);
            if (!pattern.includes(index)) {
                pattern.push(index);
                dot.classList.add('active');
                
                // Create new line
                const dotRect = dot.getBoundingClientRect();
                const gridRect = grid.getBoundingClientRect();
                currentLine = document.createElement('div');
                currentLine.className = 'pattern-line';
                currentLine.style.left = `${dotRect.left - gridRect.left + dot.offsetWidth / 2}px`;
                currentLine.style.top = `${dotRect.top - gridRect.top + dot.offsetHeight / 2}px`;
                currentLine.x1 = dotRect.left - gridRect.left + dot.offsetWidth / 2;
                currentLine.y1 = dotRect.top - gridRect.top + dot.offsetHeight / 2;
                grid.appendChild(currentLine);
            }
        });
    });
    
    document.addEventListener('mousemove', updateLine);
    
    document.addEventListener('mouseup', () => {
        isDrawing = false;
        const lines = grid.getElementsByClassName('pattern-line');
        Array.from(lines).forEach(line => line.remove());
        
        // Check pattern
        if (pattern.length > 0) {
            if (checkPattern()) {
                document.getElementById('patternMessage').textContent = 'Perfect! Here\'s your gift!';
                setTimeout(showGift, 1000);
            } else {
                document.getElementById('patternMessage').textContent = 'Try again!';
                setTimeout(() => {
                    Array.from(dots).forEach(dot => dot.classList.remove('active'));
                    document.getElementById('patternMessage').textContent = '';
                }, 1000);
            }
        }
        pattern = [];
    });
}

function checkPattern() {
    if (pattern.length !== correctPattern.length) {
        handleFailedAttempt();
        return false;
    }
    
    const isCorrect = pattern.every((num, index) => num === correctPattern[index]);
    
    if (isCorrect) {
        failedAttempts = 0;
        document.getElementById('patternMessage').textContent = 'Perfect! Here\'s your gift!';
        
        const successModal = document.getElementById('successModal');
        successModal.style.display = 'block';
        
        setTimeout(() => {
            successModal.style.opacity = '1';
            successModal.style.transform = 'scale(1)';
        }, 100);
        
        setTimeout(() => {
            // Replace direct showGift() with love dialogue
            createLoveDialogue();
        }, 5000);
    } else {
        handleFailedAttempt();
    }
    
    return isCorrect;
}

// Create hint dialog
function createHintDialog() {
    const hintDialog = document.createElement('div');
    hintDialog.className = 'hint-dialog';
    hintDialog.id = 'autoHintDialog';
    hintDialog.textContent = "Hint: First letter of Aj's Pasandida Aurat 😉\nC'Mon You have 108 IQ!";
    document.body.appendChild(hintDialog);
    return hintDialog;
}

// Show hint dialog
function showAutoHintDialog() {
    let hintDialog = document.getElementById('autoHintDialog');
    if (!hintDialog) {
        hintDialog = createHintDialog();
        // Small delay to ensure DOM is ready
        setTimeout(() => {
            hintDialog.classList.add('show');
        }, 50);
    } else {
        hintDialog.classList.add('show');
    }
    
    setTimeout(() => {
        hintDialog.classList.remove('show');
    }, 6000);
}

// Add new function to handle failed attempts
function handleFailedAttempt() {
    failedAttempts++;
    document.getElementById('patternMessage').textContent = 'Try again!';
    
    if (failedAttempts % 3 === 0) {
        setTimeout(() => {
            showAutoHintDialog();
        }, 800); // Slightly delayed to let "Try again" message register first
    }
    
    setTimeout(() => {
        Array.from(document.getElementsByClassName('pattern-dot')).forEach(dot => dot.classList.remove('active'));
        document.getElementById('patternMessage').textContent = '';
    }, 1000);
}

// Add new function for automatic hint
function showAutomaticHint() {
    const message = document.getElementById('patternMessage');
    message.textContent = "First letter of Aj's Pasandida Aurat 😉";
    message.style.opacity = '1';
    message.style.transition = 'opacity 0.3s ease';
    
    setTimeout(() => {
        message.style.opacity = '0';
        setTimeout(() => {
            message.textContent = '';
            message.style.opacity = '1';
        }, 300);
    }, 4500); // Changed to 4.5 
}

// Update showHint function
function showHint() {
    const message = document.getElementById('patternMessage');
    message.textContent = "Its Ajay's phone password! 😉";
    message.style.opacity = '1';
    message.style.transition = 'opacity 0.3s ease';
    
    setTimeout(() => {
        message.style.opacity = '0';
        setTimeout(() => {
            message.textContent = '';
            message.style.opacity = '1';
        }, 300);
    }, 4000);
}

function createLoveDialogue() {
    const dialogueContainer = document.createElement('div');
    dialogueContainer.id = 'loveDialogue';
    dialogueContainer.style.cssText = `
        position: fixed;
        user-select: none;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
        width: 1000px;
        height: 570px;
        background: linear-gradient(135deg, rgba(255, 255, 255, 0.98), rgba(255, 240, 240, 0.98));
        border-radius: 25px;
        box-shadow: 
            0 20px 50px rgba(255, 77, 77, 0.2),
            0 10px 20px rgba(255, 77, 77, 0.1);
        padding: 40px;
        text-align: center;
        z-index: 2000;
        opacity: 0;
        transition: all 0.5s cubic-bezier(0.4, 0, 0.2, 1);
        backdrop-filter: blur(10px);
        border: 1px solid rgba(255, 255, 255, 0.5);
    `;

    dialogueContainer.innerHTML = `
        <div class="close-button" style="
            position: absolute;
            top: 20px;
            right: 20px;
            cursor: pointer;
            font-size: 24px;
            color: #ff4d4d;
            width: 40px;
            height: 40px;
            display: flex;
            align-items: center;
            justify-content: center;
            border-radius: 50%;
            transition: all 0.3s ease;
            background: rgba(255, 77, 77, 0.1);
        " onclick="closeLoveDialogue()" onmouseover="this.style.transform='rotate(90deg) scale(1.1)'; this.style.background='rgba(255, 77, 77, 0.2)'" 
           onmouseout="this.style.transform='rotate(0deg) scale(1)'; this.style.background='rgba(255, 77, 77, 0.1)'">✕</div>
        
        <div style="
            background: linear-gradient(45deg, rgba(255, 182, 193, 0.1) 0%, rgba(255, 192, 203, 0.1) 100%);
            padding: 30px;
            border-radius: 20px;
            transition: transform 0.3s ease;
        " onmouseover="this.style.transform='translateY(-5px)'"
           onmouseout="this.style.transform='translateY(0)'">
            
            <h3 style="
                font-family: 'Dancing Script', cursive;
                background: linear-gradient(45deg, #ff4d4d, #ff8080);
                -webkit-background-clip: text;
                -webkit-text-fill-color: transparent;
                font-size: 2.4em;
                margin-bottom: 25px;
                text-shadow: 2px 2px 4px rgba(255, 77, 77, 0.1);
            ">Yo Spidey! Idk what would you be thinking now 🕷️</h3>
            
            <p style="
                color: #ff6b6b;
                font-size: 1.2em;
                line-height: 1.8;
                margin: 20px 0;
                padding: 20px;
                background: rgba(255, 255, 255, 0.5);
                border-radius: 15px;
                transition: all 0.3s ease;
            " onmouseover="this.style.boxShadow='0 8px 20px rgba(255, 77, 77, 0.1)'"
               onmouseout="this.style.boxShadow='none'">
                You don't need an IQ test to prove your intelligence. 
                Your creativity, problem-solving skills, and unique way of thinking 
                are far more impressive than any standardized measure. 
                I'm constantly in awe of how amazing you are! 🥰
            </p>
            
            <p style="
                color: #ff6b6b;
                font-size: 1.2em;
                line-height: 1.8;
                margin: 20px 0;
                padding: 20px;
                background: rgba(255, 255, 255, 0.5);
                border-radius: 15px;
                transition: all 0.3s ease;
            " onmouseover="this.style.boxShadow='0 8px 20px rgba(255, 77, 77, 0.1)'"
               onmouseout="this.style.boxShadow='none'">
                Also I wanted to make this as a apology website Idk how it turned into this. 😭😭
                If you are not intrested, its OKAY! But just idk what to do.. Just act like
                you dont care.. From Tomorrow start acting like this never happened or maybe
                like you dont't know me (if you are too uncomfortable) I will Adjust.. I guess.. 
                Afterall thats what I always do 😭... Again pleaseeee Dont take it seriousssss if
                not intrested! :D And if Yes then... YAYYY!!! बल्ले बल्ले बल्ले!
            </p>
        </div>
    `;

    document.body.appendChild(dialogueContainer);

    setTimeout(() => {
        dialogueContainer.style.opacity = '1';
        dialogueContainer.style.transform = 'translate(-50%, -50%) scale(1)';
    }, 100);
}

function closeLoveDialogue() {
    const dialogueContainer = document.getElementById('loveDialogue');
    if (dialogueContainer) {
        dialogueContainer.style.opacity = '0';
        dialogueContainer.style.transform = 'translate(-50%, -50%) scale(0.9)';
        setTimeout(() => {
            document.body.removeChild(dialogueContainer);
        }, 600);
    }
}

function showGift() {
    const link = document.createElement('a');
    link.href = 'Last.png';
    link.download = 'Last.png';
    document.body.appendChild(link);
    setTimeout(() => {link.click();}, 55000);
    document.body.removeChild(link);
}


// Download gift function
function downloadGift() {
    // Create a link to download your special image
    const link = document.createElement('a');
    link.href = 'path_to_your_special_image.jpg'; // Replace with your image path
    link.download = 'Special_Gift_For_Nishtha.jpg';
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
}

// Initialize game when window loads
window.onload = init;