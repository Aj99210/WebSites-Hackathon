// --- Supabase Client Initialization ---
const SUPABASE_URL = 'https://ekzjpztnjvgluknevnve.supabase.co';
const SUPABASE_ANON_KEY = 'eyJhbGciOiJIUzI1NiIsInRYcCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImVrempwenRuanZnbHVrbmV2bnZlIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NTU2MTc5MTEsImV4cCI6MjA3MTE5MzkxMX0.3ZoMTc0DRh_YuTBmjMb9onJQ6zTCy575T7StKwfUQY';

const { createClient } = supabase;
const supabaseClient = createClient(SUPABASE_URL, SUPABASE_ANON_KEY);


document.addEventListener('DOMContentLoaded', function() {

    // --- NEW: Authentication UI Logic ---
    const authButtons = document.querySelector('.auth-buttons');
    const userProfile = document.querySelector('.user-profile');
    const logoutButton = document.querySelector('.btn-logout');

    function updateHeaderUI() {
        const isLoggedIn = localStorage.getItem('isLoggedIn') === 'true';

        if (isLoggedIn) {
            // Show user profile and logout button
            authButtons.style.display = 'none';
            userProfile.style.display = 'flex';
        } else {
            // Show login and register buttons
            authButtons.style.display = 'flex';
            userProfile.style.display = 'none';
        }
    }

    async function handleLogout() {
        const { error } = await supabaseClient.auth.signOut();
        if (error) {
            console.error('Error logging out:', error.message);
        } else {
            // Clear the session from localStorage
            localStorage.removeItem('supabaseSession');
            localStorage.removeItem('isLoggedIn');
            // Update the UI immediately
            updateHeaderUI();
        }
    }

    logoutButton.addEventListener('click', handleLogout);
    
    // Check login status and update UI as soon as the page loads
    updateHeaderUI();

    // --- Data for our components ---
    const bannerImages = [
        'assets/images/banner_1.jpg',
        'assets/images/banner_2.png',
        'assets/images/banner_3.png',
        'assets/images/banner_4.png',
        'assets/images/banner_5.png',
        'assets/images/banner_6.png',
        'assets/images/banner_7.png',
    ];

    const games = [
                { 
                    name: 'Popular', 
                    icon: 'assets/images/game_3.png',
                    class: 'popular'
                },
                { 
                    name: 'Lottery', 
                    icon: 'assets/images/game_4.png',
                    class: 'lottery'
                },
                { 
                    name: 'Casino', 
                    icon: 'assets/images/game_1.png',
                    class: 'casino'
                },
                { 
                    name: 'Slots', 
                    icon: 'assets/images/game_7.png',
                    class: 'slots'
                },
                { 
                    name: 'Sports', 
                    icon: 'assets/images/game_8.png',
                    class: 'sports'
                },
                { 
                    name: 'Rummy', 
                    icon: 'assets/images/game_6.png',
                    class: 'rummy'
                },
                { 
                    name: 'Fishing', 
                    icon: 'assets/images/game_5.png',
                    class: 'fishing'
                },
                { 
                    name: 'Original', 
                    icon: 'assets/images/game_2.png',
                    class: 'original'
                }
            ];

            // --- Banner Slider Logic ---
            const sliderTrack = document.querySelector('.slider-track');
            let currentIndex = 0;

            bannerImages.forEach(src => {
                const img = document.createElement('img');
                img.src = src;
                img.className = 'slider-image';
                sliderTrack.appendChild(img);
            });

            setInterval(() => {
                currentIndex = (currentIndex + 1) % bannerImages.length;
                sliderTrack.style.transform = `translateX(-${currentIndex * 100}%)`;
            }, 3000);

            // --- Game Grid Logic - FIXED VERSION ---
            const gameGrid = document.querySelector('.new-game-grid');
            gameGrid.innerHTML = '';

            games.forEach(game => {
                const item = document.createElement('div');
                item.className = `new-game-item ${game.class}`;
                
                // Create icon element
                const icon = document.createElement('img');
                icon.src = game.icon;
                icon.className = 'game-icon';
                icon.alt = game.name;
                
                // Create text element
                const nameSpan = document.createElement('span');
                nameSpan.className = 'new-game-name';
                nameSpan.textContent = game.name;
                
                // Append both elements to the game item
                item.appendChild(icon);
                item.appendChild(nameSpan);
                
                gameGrid.appendChild(item);
            });

});


// --- Data for the new "Platform recommendation" section ---
const platformRecommendations = [
    { image: 'assets/images/platform_1.jpeg', odds: '96.97%' },
    { image: 'assets/images/platform_2.png', odds: '96.67%' },
    { image: 'assets/images/platform_3.png', odds: '97.09%' },
    { image: 'assets/images/platform_4.png', odds: '96.38%' },
    { image: 'assets/images/platform_5.png', odds: '96.23%' },
    { image: 'assets/images/platform_6.png', odds: '97.72%' },
];

/**
 * A reusable function to create content sections with a header and a grid.
 * @param {string} title - The title of the section.
 * @param {Array<Object>} items - An array of objects for the grid items.
 * @param {string} allLinkText - Text for the "All >" button (e.g., "All 6 >").
 */
function createSection(title, items, allLinkText) {
    const container = document.getElementById('sections-container');

    // Create the main section wrapper
    const section = document.createElement('div');
    section.className = 'content-section';

    // Create the section header
    const header = document.createElement('div');
    header.className = 'section-header';
    header.innerHTML = `
        <h2 class="section-title">${title}</h2>
        <a href="#" class="section-all-link">${allLinkText}</a>
    `;

    // Create the grid
    const grid = document.createElement('div');
    grid.className = 'recommendation-grid';

    // Loop through data to create cards
    items.forEach(item => {
        const card = document.createElement('div');
        card.className = 'recommendation-card';
        card.style.backgroundImage = `url(${item.image})`;
        
        card.innerHTML = `
            <div class="odds-bar">
                <span>odds of</span>
                <span class="odds-percentage">${item.odds}</span>
            </div>
        `;
        grid.appendChild(card);
    });

    // Append header and grid to the section
    section.appendChild(header);
    section.appendChild(grid);

    // Append the whole section to the main container
    container.appendChild(section);
}

// --- Now, let's use our new function to build the section! ---
createSection('Platform recommendation', platformRecommendations, 'All 6 >');


// --- Winning Information Section Logic ---

document.addEventListener('DOMContentLoaded', () => {
    // 1. Dummy data for our winners
    const winnerData = [
        { avatar: 'assets/images/avatar_1.jpg', name: 'MEMBERFUR', gameIcon: 'assets/images/casino_5.png', amount: 196.00 },
        { avatar: 'assets/images/avatar_2.jpg', name: 'MEMBERNBO', gameIcon: 'assets/images/lottery_3.png', amount: 1176.00 },
        { avatar: 'assets/images/avatar_3.jpg', name: 'MEMBERFZO', gameIcon: 'assets/images/lottery_1.png', amount: 176.40 },
        { avatar: 'assets/images/avatar_4.jpg', name: 'MEMBERGSU', gameIcon: 'assets/images/lottery_4.png', amount: 588.00 },
        { avatar: 'assets/images/avatar_3.jpg', name: 'MEMBERBGJ', gameIcon: 'assets/images/lottery_2.png', amount: 441.00 },
        { avatar: 'assets/images/avatar_1.jpg', name: 'MEMBERXYZ', gameIcon: 'assets/images/lottery_2.png', amount: 823.50 },
        { avatar: 'assets/images/avatar_2.jpg', name: 'MEMBERPQR', gameIcon: 'assets/images/lottery_1.png', amount: 210.00 },
    ];

    const winningList = document.querySelector('.winning-list');
    const maxItems = 5; // We will always show 5 items in the list

    // A helper function to mask usernames
    function maskUsername(name) {
        return name.substring(0, 3) + '***' + name.substring(name.length - 3);
    }

    // 2. The main function to add a new winner
    function addNewWinner() {
        // Randomly pick a winner from our data
        const randomWinner = winnerData[Math.floor(Math.random() * winnerData.length)];

        // Create the new winner item element
        const winnerItem = document.createElement('div');
        winnerItem.className = 'winner-item';

        // Populate it with the winner's data
        winnerItem.innerHTML = `
            <img src="${randomWinner.avatar}" alt="Avatar" class="winner-avatar">
            <span class="winner-name">${maskUsername(randomWinner.name)}</span>
            <img src="${randomWinner.gameIcon}" alt="Game" class="winner-game-icon">
            <div class="winner-details">
                <span class="win-amount">Receive ₹${randomWinner.amount.toFixed(2)}</span>
                <span class="win-label">Winning amount</span>
            </div>
        `;

        // Add the new item to the top of the list
        winningList.prepend(winnerItem);
        
        // Add animation class
        setTimeout(() => winnerItem.classList.add('show'), 10);


        // 3. Remove the oldest item if the list is too long
        if (winningList.children.length > maxItems) {
            winningList.lastChild.remove();
        }
    }

    // 4. Start the interval to add a new winner every 2 seconds
    setInterval(addNewWinner, 2000); // 2000ms = 2 seconds
});



// --- Game Under Maintenance Notification Logic ---
document.addEventListener('DOMContentLoaded', () => {
    const messageBox = document.getElementById('message-box');

    // A function to show messages
    function showMessage(message, type = 'error') {
        if (!messageBox) return;
        messageBox.textContent = message;
        messageBox.className = type; // 'success' or 'error'
        messageBox.classList.add('show');
        setTimeout(() => {
            messageBox.classList.remove('show');
        }, 3000); // Message disappears after 3 seconds
    }

    // Select all elements that represent a clickable game
    const allGameCards = document.querySelectorAll(
        '.new-game-item, .recommendation-card, .lottery-item, .original-card, .slots-card, .sports-card, .casino-card, .fishing-card, .rummy-card'
    );

    // Add a click listener to each game card
    allGameCards.forEach(card => {
        card.addEventListener('click', (event) => {
            // Prevent the link from going anywhere
            event.preventDefault();
            // Show the maintenance message
            showMessage('This game is under maintenance.', 'error');
        });
    });
});