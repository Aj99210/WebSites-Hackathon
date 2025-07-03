const messages = [
    "Are you sure?",
    "Really sure??",
    "Pookie please...",
    "Just think about it!",
    "Ik you're middle child! C'mon!",
    "C'mon! You have a High IQ",
    "Ik I fked up last time.. Im sryy T_T",
    "If you say no, I will be really sad...",
    "I even left those Dalle for you :)",
    "I will be very very very sad...",
    "Ok fine, I will stop asking...",
    "Just kidding, say yes please! ❤️"
];

let messageIndex = 0;
const MAX_BUTTON_SIZE = 60;

function handleNoClick() {
    const noButton = document.querySelector('.no-button');
    const yesButton = document.querySelector('.yes-button');
    const container = document.querySelector('.container');
    
    // Add vibration effect
    container.classList.add('vibrate');
    
    // Remove vibration after 500ms
    setTimeout(() => {
        container.classList.remove('vibrate');
    }, 500);
    
    // Update the no button text
    noButton.textContent = messages[messageIndex];
    messageIndex = (messageIndex + 1) % messages.length;
    
    // Increase yes button size
    const currentSize = parseFloat(window.getComputedStyle(yesButton).fontSize);
    const newSize = Math.min(currentSize * 1.5, MAX_BUTTON_SIZE);
    yesButton.style.fontSize = `${newSize}px`;
    
    // Make container scrollable and hide corners
    container.style.maxHeight = '80vh';
    container.style.overflowY = 'auto';
    container.classList.add('scrollable');
    
    // Scroll yes button into view
    yesButton.scrollIntoView({ behavior: 'smooth', block: 'center' });
}

function showDialog(message) {
    const wrapper = document.createElement('div');
    wrapper.style.position = 'fixed';
    wrapper.style.top = '50%';
    wrapper.style.left = '52%';
    wrapper.style.transform = 'translate(-50%, -50%) perspective(1000px)';
    wrapper.style.zIndex = '1000';
    
    const dialog = document.createElement('div');
    dialog.style.background = 'rgba(255, 255, 255, 0.97)';
    dialog.style.padding = '35px 45px';
    dialog.style.borderRadius = '25px';
    dialog.style.boxShadow = `
        0 10px 30px rgba(211, 47, 47, 0.2),
        0 0 20px rgba(211, 47, 47, 0.1),
        inset 0 0 20px rgba(255, 255, 255, 0.5)
    `;
    dialog.style.opacity = '0';
    dialog.style.transform = 'rotateX(-20deg) scale(0.8)';
    dialog.style.transition = 'all 0.6s cubic-bezier(0.34, 1.56, 0.64, 1)';
    dialog.style.color = '#d32f2f';
    dialog.style.fontSize = '1.8em';
    dialog.style.fontFamily = "'Poppins', sans-serif";
    dialog.style.textAlign = 'center';
    dialog.style.maxWidth = '80%';
    dialog.style.border = '2px solid rgba(211, 47, 47, 0.15)';
    dialog.style.backdropFilter = 'blur(10px)';
    dialog.style.position = 'relative';
    dialog.style.overflow = 'hidden';
    dialog.style.background = `
        linear-gradient(135deg, 
            rgba(255, 255, 255, 0.97) 0%,
            rgba(255, 240, 240, 0.97) 100%)
    `;
    
    // Add decorative corner hearts
    const corners = ['top-left', 'top-right', 'bottom-left', 'bottom-right'];
    corners.forEach(corner => {
        const heart = document.createElement('div');
        heart.textContent = '♥';
        heart.style.position = 'absolute';
        heart.style.fontSize = '1.2em';
        heart.style.color = 'rgba(211, 47, 47, 0.2)';
        heart.style.transition = 'all 0.3s ease';
        
        if (corner.includes('top')) heart.style.top = '12px';
        if (corner.includes('bottom')) heart.style.bottom = '12px';
        if (corner.includes('left')) heart.style.left = '18px';
        if (corner.includes('right')) heart.style.right = '18px';
        
        dialog.appendChild(heart);
    });

    const textContent = document.createElement('div');
    textContent.textContent = message;
    textContent.style.marginBottom = '15px';
    textContent.style.fontWeight = '500';
    textContent.style.letterSpacing = '0.5px';
    textContent.style.textShadow = '0 1px 2px rgba(211, 47, 47, 0.1)';
    
    const emoji = document.createElement('div');
    emoji.innerHTML = '(｡♥‿♥｡)<br>💝';
    emoji.style.fontSize = '1.2em';
    emoji.style.marginTop = '15px';
    emoji.style.opacity = '0';
    emoji.style.transform = 'translateY(10px)';
    emoji.style.transition = 'all 0.5s ease';
    emoji.style.lineHeight = '1.5';
    
    dialog.appendChild(textContent);
    dialog.appendChild(emoji);
    wrapper.appendChild(dialog);
    document.body.appendChild(wrapper);

    // Particle system
    const createParticle = (mouseX = null, mouseY = null, isMouseParticle = false) => {
        const particle = document.createElement('div');
        const particles = isMouseParticle ? ['✧', '♡', '✿'] : ['♥', '✧', '♡'];
        const colors = [
            'rgba(211, 47, 47, 0.35)',
            'rgba(255, 138, 128, 0.35)',
            'rgba(255, 82, 82, 0.35)'
        ];
        
        particle.textContent = particles[Math.floor(Math.random() * particles.length)];
        particle.style.position = 'absolute';
        particle.style.color = colors[Math.floor(Math.random() * colors.length)];
        particle.style.fontSize = (Math.random() * 0.8 + 0.5) + 'em';
        particle.style.pointerEvents = 'none';
        particle.style.userSelect = 'none';
        particle.style.zIndex = '-1';
        particle.style.textShadow = '0 0 3px rgba(255, 255, 255, 0.5)';

        if (isMouseParticle) {
            particle.style.left = mouseX + 'px';
            particle.style.top = mouseY + 'px';
        } else {
            particle.style.left = Math.random() * 100 + '%';
            particle.style.top = '100%';
        }

        dialog.appendChild(particle);

        const duration = 1500 + Math.random() * 1000;
        const xDistance = (Math.random() - 0.5) * 100;
        const yDistance = isMouseParticle ? -100 : -(dialog.offsetHeight + 20);
        
        particle.animate([
            {
                transform: 'translate(0, 0) rotate(0deg)',
                opacity: 0.9
            },
            {
                transform: `translate(${xDistance}px, ${yDistance}px) rotate(${Math.random() * 360}deg)`,
                opacity: 0
            }
        ], {
            duration: duration,
            easing: 'cubic-bezier(0.4, 0, 0.2, 1)'
        });

        setTimeout(() => particle.remove(), duration);
    };

    // Autonomous particles
    const particleInterval = setInterval(() => {
        if (document.body.contains(dialog)) {
            createParticle();
        } else {
            clearInterval(particleInterval);
        }
    }, 200);

    // Initial particles burst
    for (let i = 0; i < 12; i++) {
        setTimeout(createParticle, i * 100);
    }

    // Mouse interaction
    let lastMouseMove = 0;
    dialog.addEventListener('mousemove', (e) => {
        const now = Date.now();
        if (now - lastMouseMove > 50) { // Limit particle creation rate
            lastMouseMove = now;
            const rect = dialog.getBoundingClientRect();
            const x = e.clientX - rect.left;
            const y = e.clientY - rect.top;
            createParticle(x, y, true);
            
            // Update corner hearts
            const hearts = dialog.querySelectorAll('div');
            hearts.forEach(heart => {
                if (heart.textContent === '♥') {
                    const dx = (x - rect.width / 2) / rect.width;
                    const dy = (y - rect.height / 2) / rect.height;
                    heart.style.transform = `translate(${dx * 10}px, ${dy * 10}px)`;
                }
            });
        }
    });

    dialog.addEventListener('mouseleave', () => {
        const hearts = dialog.querySelectorAll('div');
        hearts.forEach(heart => {
            if (heart.textContent === '♥') {
                heart.style.transform = 'translate(0, 0)';
            }
        });
    });

    // Entrance animation
    requestAnimationFrame(() => {
        dialog.style.opacity = '1';
        dialog.style.transform = 'rotateX(0) scale(1)';
        
        setTimeout(() => {
            emoji.style.opacity = '1';
            emoji.style.transform = 'translateY(0)';
        }, 300);
    });

    return wrapper;
}

function handleYesClick() {
    const container = document.querySelector('.container');
    const heading = document.querySelector('.heading');
    const buttons = document.querySelector('.buttons');
    const gifContainer = document.querySelector('.gif_container');
    
    // Hide buttons and gif with fade
    buttons.style.transition = 'opacity 0.5s ease';
    buttons.style.opacity = '0';
    gifContainer.style.transition = 'opacity 0.5s ease';
    gifContainer.style.opacity = '0.3';
    
    // Reset container scroll and styling
    container.style.maxHeight = '';
    container.style.overflowY = '';
    container.classList.remove('scrollable');
    
    // Initial heading setup
    heading.textContent = "Will you be my life project!";
    heading.style.transition = "all 0.8s cubic-bezier(0.34, 1.56, 0.64, 1)";
    
    // Ensure container doesn't clip the heading animation
    container.style.overflow = 'visible';
    
    // First animation phase
    heading.style.transform = "scale(1.3) translateY(-10px)";
    heading.style.textShadow = "0 0 15px rgba(211, 47, 47, 0.6)";
    heading.style.color = "#ff1744";
    
    // Ensure heading is visible
    window.scrollTo({
        top: 0,
        behavior: 'smooth'
    });
    heading.scrollIntoView({ behavior: 'smooth', block: 'center' });
    heading.focus();
    
    // Second animation phase
    setTimeout(() => {
        heading.style.transform = "scale(1.15) translateY(0)";
        heading.style.textShadow = "0 0 20px rgba(211, 47, 47, 0.4), 0 0 30px rgba(211, 47, 47, 0.2)";
    }, 800);
    
    // Final animation phase
    setTimeout(() => {
        heading.style.transform = "scale(1)";
        heading.style.textShadow = "0 0 10px rgba(211, 47, 47, 0.3)";
        heading.style.color = "#d32f2f";
    }, 1600);
    
    // Show dialog after heading animation
    setTimeout(() => {
        const dialog = showDialog("Heheh You got tricked! Now you're my life project! :D");
        
        // Wait for dialog to be shown before redirecting
        setTimeout(() => {
            window.location.href = "yes_page.html";
        }, 8500);
    }, 2500);
}


// Add these functions to your script.js file

// Window control functions
function minimizeWindow() {
  if (window.electronAPI) {
    window.electronAPI.minimize();
  }
}

function maximizeWindow() {
  if (window.electronAPI) {
    window.electronAPI.maximize();
  }
}

function closeWindow() {
  if (window.electronAPI) {
    window.electronAPI.close();
  }
}