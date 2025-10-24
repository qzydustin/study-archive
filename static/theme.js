document.addEventListener('DOMContentLoaded', (event) => {
    const savedTheme = localStorage.getItem('theme') || 'light-theme';
    const sun = document.querySelector('.sun');
    const moon = document.querySelector('.moon');
    if (savedTheme === 'dark-theme') {
        sun.style.display = 'none';
        moon.style.display = 'block';
    }else {
        sun.style.display = 'block';
        moon.style.display = 'none';
    }
    document.body.classList.add(savedTheme);
});

function toggleTheme() {
    const body = document.body;
    const sun = document.querySelector('.sun');
    const moon = document.querySelector('.moon');

    if (body.classList.contains('light-theme')) {
        body.classList.replace('light-theme', 'dark-theme');
        localStorage.setItem('theme', 'dark-theme');
        sun.style.display = 'none';
        moon.style.display = 'block';
    } else {
        body.classList.replace('dark-theme', 'light-theme');
        localStorage.setItem('theme', 'light-theme');
        sun.style.display = 'block';
        moon.style.display = 'none';
    }
}
