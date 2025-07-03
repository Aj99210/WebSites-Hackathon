// Custom Cursor
        const cursor = document.querySelector('.cursor');
        const cursorFollower = document.querySelector('.cursor-follower');

        document.addEventListener('mousemove', (e) => {
            cursor.style.left = e.clientX + 'px';
            cursor.style.top = e.clientY + 'px';
            
            cursorFollower.style.left = e.clientX - 20 + 'px';
            cursorFollower.style.top = e.clientY - 20 + 'px';
        });

        // Navbar scroll effect
        const navbar = document.getElementById('navbar');
        window.addEventListener('scroll', () => {
            if (window.scrollY > 100) {
                navbar.classList.add('scrolled');
            } else {
                navbar.classList.remove('scrolled');
            }
        });

        // Smooth scrolling for navigation links
        document.querySelectorAll('a[href^="#"]').forEach(anchor => {
            anchor.addEventListener('click', function (e) {
                e.preventDefault();
                const target = document.querySelector(this.getAttribute('href'));
                if (target) {
                    target.scrollIntoView({
                        behavior: 'smooth',
                        block: 'start'
                    });
                }
            });
        });

        // Animated counters for stats
        const observerOptions = {
            threshold: 0.5,
            rootMargin: '0px 0px -100px 0px'
        };

        const observer = new IntersectionObserver((entries) => {
            entries.forEach(entry => {
                if (entry.isIntersecting) {
                    const counter = entry.target;
                    const target = parseInt(counter.getAttribute('data-count'));
                    const increment = target / 100;
                    let current = 0;
                    
                    const updateCounter = () => {
                        if (current < target) {
                            current += increment;
                            counter.textContent = Math.floor(current);
                            requestAnimationFrame(updateCounter);
                        } else {
                            counter.textContent = target;
                        }
                    };
                    
                    updateCounter();
                    observer.unobserve(counter);
                }
            });
        }, observerOptions);

        document.querySelectorAll('.stat-number').forEach(counter => {
            observer.observe(counter);
        });

        // Parallax effect for hero 3D elements
        window.addEventListener('scroll', () => {
            const scrolled = window.pageYOffset;
            const rate = scrolled * -0.5;
            
            document.querySelectorAll('.hero-3d-element').forEach((element, index) => {
                const speed = 0.5 + (index * 0.2);
                element.style.transform = `translateY(${rate * speed}px) rotateX(${scrolled * 0.1}deg) rotateY(${scrolled * 0.1}deg)`;
            });
        });

        // Interactive card hover effects
        document.querySelectorAll('.interactive-card').forEach(card => {
            card.addEventListener('mouseenter', () => {
                card.style.transform = 'translateY(-15px) rotateX(5deg) scale(1.02)';
            });
            
            card.addEventListener('mouseleave', () => {
                card.style.transform = 'translateY(0) rotateX(0) scale(1)';
            });
        });

        // Fade in animation on scroll
        const fadeElements = document.querySelectorAll('.fade-in');
        const fadeObserver = new IntersectionObserver((entries) => {
            entries.forEach(entry => {
                if (entry.isIntersecting) {
                    entry.target.style.opacity = '1';
                    entry.target.style.transform = 'translateY(0)';
                }
            });
        }, { threshold: 0.1 });

        fadeElements.forEach(element => {
            fadeObserver.observe(element);
        });

        // Enhanced hover effects for buttons
        document.querySelectorAll('.cta-button, .craft-btn').forEach(button => {
            button.addEventListener('mouseenter', () => {
                button.style.transform = 'translateY(-5px) scale(1.05)';
            });
            
            button.addEventListener('mouseleave', () => {
                button.style.transform = 'translateY(0) scale(1)';
            });
        });

        // Dynamic background animation
        const shapes = document.querySelectorAll('.shape');
        let mouseX = 0;
        let mouseY = 0;

        document.addEventListener('mousemove', (e) => {
            mouseX = e.clientX;
            mouseY = e.clientY;
        });

        function animateShapes() {
            shapes.forEach((shape, index) => {
                const speed = 0.001 + (index * 0.0005);
                const x = (mouseX - window.innerWidth / 2) * speed;
                const y = (mouseY - window.innerHeight / 2) * speed;
                
                shape.style.transform = `translate(${x}px, ${y}px)`;
            });
            
            requestAnimationFrame(animateShapes);
        }

        animateShapes();

        // Recipe Generator Modal (placeholder for future integration)
        document.addEventListener('DOMContentLoaded', () => {
            const craftButtons = document.querySelectorAll('[href="#craft"]');
            
            craftButtons.forEach(button => {
                button.addEventListener('click', (e) => {
                    e.preventDefault();
                    
                    // Placeholder for recipe generator modal
                    const modal = document.createElement('div');
                    modal.style.cssText = `
                        position: fixed;
                        top: 0;
                        left: 0;
                        width: 100%;
                        height: 100%;
                        background: rgba(0, 0, 0, 0.8);
                        backdrop-filter: blur(10px);
                        z-index: 10000;
                        display: flex;
                        align-items: center;
                        justify-content: center;
                        opacity: 0;
                        transition: opacity 0.3s ease;
                    `;
                    
                    const modalContent = document.createElement('div');
                    modalContent.style.cssText = `
                        background: linear-gradient(135deg, #faf8f5, #f5f2ee);
                        padding: 3rem;
                        border-radius: 25px;
                        max-width: 600px;
                        width: 90%;
                        text-align: center;
                        box-shadow: 0 25px 50px rgba(139, 90, 60, 0.3);
                        transform: scale(0.9);
                        transition: transform 0.3s ease;
                    `;
                    
                    modalContent.innerHTML = `
                        <h2 style="font-size: 2.5rem; color: #8b5a3c; margin-bottom: 1.5rem; font-weight: 800;">
                            🍳 Recipe Generator
                        </h2>
                        <p style="font-size: 1.2rem; color: #666; margin-bottom: 2rem; line-height: 1.6;">
                            Enter any food name or ingredient, and our AI will craft the perfect recipe for you!
                        </p>
                        <div style="margin-bottom: 2rem;">
                            <input type="text" placeholder="Enter food name or ingredient..." style="
                                width: 100%;
                                padding: 1rem 1.5rem;
                                font-size: 1.1rem;
                                border: 2px solid #d4b896;
                                border-radius: 50px;
                                outline: none;
                                margin-bottom: 1.5rem;
                                transition: all 0.3s ease;
                            " id="recipeInput">
                            <div style="display: flex; gap: 1rem; justify-content: center; flex-wrap: wrap;">
                                <button style="
                                    background: linear-gradient(135deg, #8b5a3c, #6b4423);
                                    color: white;
                                    padding: 1rem 2rem;
                                    border: none;
                                    border-radius: 50px;
                                    font-size: 1.1rem;
                                    font-weight: 600;
                                    cursor: pointer;
                                    transition: all 0.3s ease;
                                " onclick="generateRecipe()">
                                    Generate Recipe ✨
                                </button>
                                <button style="
                                    background: rgba(139, 90, 60, 0.1);
                                    color: #8b5a3c;
                                    padding: 1rem 2rem;
                                    border: 2px solid #8b5a3c;
                                    border-radius: 50px;
                                    font-size: 1.1rem;
                                    font-weight: 600;
                                    cursor: pointer;
                                    transition: all 0.3s ease;
                                " onclick="closeModal()">
                                    Close
                                </button>
                            </div>
                        </div>
                        <div id="recipeResult" style="display: none; margin-top: 2rem; text-align: left;">
                            <!-- Recipe results will be displayed here -->
                        </div>
                    `;
                    
                    modal.appendChild(modalContent);
                    document.body.appendChild(modal);
                    
                    // Show modal with animation
                    setTimeout(() => {
                        modal.style.opacity = '1';
                        modalContent.style.transform = 'scale(1)';
                    }, 10);
                    
                    // Close modal function
                    window.closeModal = () => {
                        modal.style.opacity = '0';
                        modalContent.style.transform = 'scale(0.9)';
                        setTimeout(() => {
                            document.body.removeChild(modal);
                        }, 300);
                    };
                    // Close on outside click
                    modal.addEventListener('click', (e) => {
                        if (e.target === modal) {
                            closeModal();
                        }
                    });
                });
            });
        });

        // Add some interactive magic to the logo
        const logo = document.querySelector('.logo');
        logo.addEventListener('click', (e) => {
            e.preventDefault();
            
            // Create floating emojis
            const emojis = ['🍳', '🍕', '🍔', '🍝', '🥗', '🍰', '🍲', '🥘'];
            
            for (let i = 0; i < 8; i++) {
                const emoji = document.createElement('div');
                emoji.textContent = emojis[Math.floor(Math.random() * emojis.length)];
                emoji.style.cssText = `
                    position: fixed;
                    font-size: 2rem;
                    pointer-events: none;
                    z-index: 9999;
                    left: ${e.clientX}px;
                    top: ${e.clientY}px;
                    animation: floatAway 2s ease-out forwards;
                `;
                
                document.body.appendChild(emoji);
                
                // Random direction
                const angle = (Math.PI * 2 * i) / 8;
                const distance = 100 + Math.random() * 50;
                const endX = e.clientX + Math.cos(angle) * distance;
                const endY = e.clientY + Math.sin(angle) * distance;
                
                emoji.style.setProperty('--end-x', endX + 'px');
                emoji.style.setProperty('--end-y', endY + 'px');
                
                setTimeout(() => {
                    if (emoji.parentNode) {
                        emoji.parentNode.removeChild(emoji);
                    }
                }, 2000);
            }
        });

        // Add CSS for floating animation
        const style = document.createElement('style');
        style.textContent = `
            @keyframes floatAway {
                0% {
                    transform: translateY(0) scale(1) rotate(0deg);
                    opacity: 1;
                }
                100% {
                    transform: translateX(calc(var(--end-x) - 50vw)) translateY(calc(var(--end-y) - 50vh)) scale(0.3) rotate(360deg);
                    opacity: 0;
                }
            }
        `;
        document.head.appendChild(style);

        // Initialize page
        console.log('🍳 Fester is ready to craft amazing recipes!');