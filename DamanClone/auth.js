document.addEventListener('DOMContentLoaded', () => {
    // --- Supabase Client Initialization ---
    const SUPABASE_URL = 'https://ekzjpztnjvgluknevnve.supabase.co';
    const SUPABASE_ANON_KEY = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImVrempwenRuanZnbHVrbmV2bnZlIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NTU2MTc5MTEsImV4cCI6MjA3MTE5MzkxMX0.3ZoMTc0DRh1_YuTBmjMb9onJQ6zTCy575T7StKwfUQY';

    const { createClient } = supabase;
    const supabaseClient = createClient(SUPABASE_URL, SUPABASE_ANON_KEY);

    // --- Element Selection ---
    const loginView = document.getElementById('login-view');
    const registerView = document.getElementById('register-view');
    const goToRegisterBtn = document.getElementById('goToRegister');
    const goToLoginBtn = document.getElementById('goToLogin');
    const messageBox = document.getElementById('message-box');

    // Login form elements
    const loginEmailInput = document.querySelector('#email-login input[type="email"]');
    const loginPasswordInput = document.querySelector('#email-login input[type="password"]');
    const loginButton = document.querySelector('#login-view .btn-primary-auth');

    // Register form elements
    const registerEmailInput = document.querySelector('#register-view input[type="email"]');
    const registerPasswordInput = document.querySelector('#register-view input[placeholder="Set password"]');
    const registerConfirmPasswordInput = document.querySelector('#register-view input[placeholder="Confirm password"]');
    const registerButton = document.querySelector('#register-view .btn-primary-auth');
    const termsCheckbox = document.getElementById('terms-reg');

    // --- Message Display Function ---
    function showMessage(message, type = 'error') {
        messageBox.textContent = message;
        messageBox.className = type;
        messageBox.classList.remove('hidden');
        messageBox.classList.add('show');
        
        setTimeout(() => { 
            messageBox.classList.remove('show');
            setTimeout(() => {
                messageBox.classList.add('hidden');
            }, 300);
        }, 4000);
    }

    // --- Authentication Functions ---
    async function handleRegister(e) {
        e.preventDefault();
        if (registerButton.disabled) return;

        const email = registerEmailInput.value.trim();
        const password = registerPasswordInput.value;
        const confirmPassword = registerConfirmPasswordInput.value;

        if (password !== confirmPassword) {
            showMessage("Passwords do not match.", 'error');
            return;
        }

        if (password.length < 6) {
            showMessage("Password must be at least 6 characters long.", 'error');
            return;
        }

        // Disable button during registration
        registerButton.disabled = true;
        registerButton.textContent = 'Registering...';

        try {
            const { data, error } = await supabaseClient.auth.signUp({
                email: email,
                password: password,
            });

            if (error) {
                showMessage(error.message, 'error');
            } else {
                showMessage("Registration successful! Please check your email to verify your account.", 'success');
                // Clear form
                registerEmailInput.value = '';
                registerPasswordInput.value = '';
                registerConfirmPasswordInput.value = '';
                termsCheckbox.checked = false;
                
                setTimeout(() => { 
                    registerView.style.display = 'none';
                    loginView.style.display = 'block';
                }, 2000);
            }
        } catch (error) {
            showMessage("An unexpected error occurred. Please try again.", 'error');
        } finally {
            registerButton.disabled = false;
            registerButton.textContent = 'Register';
            checkRegisterFormValidity();
        }
    }

    async function handleLogin(e) {
    e.preventDefault();
    if (loginButton.disabled) return;
    
    const phoneTabActive = document.querySelector('[data-tab="phone-login"]').classList.contains('active');
    
    if (phoneTabActive) {
        showMessage("Login with phone feature will come in the future. For now, you can log in with email only.", 'error');
        return; 
    }

    const email = loginEmailInput.value;
    const password = loginPasswordInput.value;

    // First, we attempt to sign in the user
    const { data: signInData, error: signInError } = await supabaseClient.auth.signInWithPassword({
        email: email,
        password: password
    });

    if (signInError) {
        // If login fails, check if it's because the user doesn't exist
        if (signInError.message === 'Invalid login credentials') {
            
            // User doesn't exist, so now we try to register them automatically
            const { data: signUpData, error: signUpError } = await supabaseClient.auth.signUp({
                email: email,
                password: password,
            });

            if (signUpError) {
                // If sign UP fails (e.g., password too short), show that error
                showMessage(signUpError.message);
            } else {
                // If sign UP succeeds, welcome them
                showMessage("Welcome! Your account has been created successfully.", 'success');
                localStorage.setItem('isLoggedIn', 'true');
                setTimeout(() => { window.location.href = 'index.html'; }, 2000);
            }

        } else {
            // If the error was something else (like wrong password), show the original error
            showMessage(signInError.message);
        }
    } else {
        // If the initial login was successful, proceed as normal
        showMessage("Login successful!", 'success');
        localStorage.setItem('isLoggedIn', 'true');
        setTimeout(() => { window.location.href = 'index.html'; }, 1500);
    }
}

    // --- Form Validity Checks ---
    function checkLoginFormValidity() {
        const phoneTabActive = document.querySelector('[data-tab="phone-login"]').classList.contains('active');
        
        if (phoneTabActive) {
            // For phone tab, keep button enabled but handle in click event
            loginButton.disabled = false;
            loginButton.classList.add('enabled');
        } else {
            // For email tab
            const email = loginEmailInput ? loginEmailInput.value.trim() : '';
            const password = loginPasswordInput ? loginPasswordInput.value.trim() : '';
            
            const isValid = email.includes('@') && email.length > 0 && password.length > 0;
            
            loginButton.disabled = !isValid;
            if (isValid) {
                loginButton.classList.add('enabled');
            } else {
                loginButton.classList.remove('enabled');
            }
        }
    }

    function checkRegisterFormValidity() {
        const email = registerEmailInput.value.trim();
        const password = registerPasswordInput.value;
        const confirmPassword = registerConfirmPasswordInput.value;
        
        const isValid = email.includes('@') && 
                        email.length > 0 &&
                        password.length >= 6 &&
                        confirmPassword.length >= 6 &&
                        password === confirmPassword &&
                        termsCheckbox.checked;

        registerButton.disabled = !isValid;
        if (isValid) {
            registerButton.classList.add('enabled');
        } else {
            registerButton.classList.remove('enabled');
        }
    }

    // --- Event Listeners ---
    registerButton.addEventListener('click', handleRegister);
    loginButton.addEventListener('click', handleLogin);

    // Add input event listeners for real-time validation
    if (registerEmailInput) registerEmailInput.addEventListener('input', checkRegisterFormValidity);
    if (registerPasswordInput) registerPasswordInput.addEventListener('input', checkRegisterFormValidity);
    if (registerConfirmPasswordInput) registerConfirmPasswordInput.addEventListener('input', checkRegisterFormValidity);
    if (termsCheckbox) termsCheckbox.addEventListener('change', checkRegisterFormValidity);

    if (loginEmailInput) loginEmailInput.addEventListener('input', checkLoginFormValidity);
    if (loginPasswordInput) loginPasswordInput.addEventListener('input', checkLoginFormValidity);

    // --- UI View Switching Logic ---
    goToRegisterBtn.addEventListener('click', () => {
        loginView.style.display = 'none';
        registerView.style.display = 'block';
        // Reset and check form validity when switching views
        setTimeout(checkRegisterFormValidity, 100);
    });

    goToLoginBtn.addEventListener('click', (e) => {
        e.preventDefault();
        registerView.style.display = 'none';
        loginView.style.display = 'block';
        // Reset and check form validity when switching views
        setTimeout(checkLoginFormValidity, 100);
    });

    // Handle URL parameters
    const urlParams = new URLSearchParams(window.location.search);
    if (urlParams.get('view') === 'register') {
        loginView.style.display = 'none';
        registerView.style.display = 'block';
    }

    // Tab switching logic
    const tabLinks = document.querySelectorAll('.tab-link');
    const tabContents = document.querySelectorAll('.tab-content');
    tabLinks.forEach(link => {
        link.addEventListener('click', () => {
            tabLinks.forEach(item => item.classList.remove('active'));
            tabContents.forEach(item => item.classList.remove('active'));
            link.classList.add('active');
            document.getElementById(link.getAttribute('data-tab')).classList.add('active');
            
            // Recheck form validity when switching tabs
            setTimeout(checkLoginFormValidity, 100);
        });
    });

    // Initialize button states on page load
    setTimeout(() => {
        checkLoginFormValidity();
        checkRegisterFormValidity();
    }, 100);
});