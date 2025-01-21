// Countdown Timer 
const countdown = () => {
  const launchDate = new Date("2025-06-01T00:00:00").getTime(); // Adjust the date as needed
  const now = new Date().getTime();
  const timeLeft = launchDate - now;

  const days = Math.floor(timeLeft / (1000 * 60 * 60 * 24));
  const hours = Math.floor((timeLeft % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60));
  const minutes = Math.floor((timeLeft % (1000 * 60 * 60)) / (1000 * 60));
  const seconds = Math.floor((timeLeft % (1000 * 60)) / 1000);

  const timerElement = document.getElementById("timer");
  if (timerElement) {
      timerElement.innerHTML = `${days} Days ${hours} Hours ${minutes} Minutes ${seconds} Seconds`;
  
      if (timeLeft < 0) {
        clearInterval(timerInterval);
        timerElement.innerHTML = "We are live!";
      }
  } else {
      console.error('Element with ID "timer" not found.');
  }
};

const timerInterval = setInterval(countdown, 1000);
countdown(); // Initial call

// FAQ Toggle
document.querySelectorAll('.faq-item').forEach(item => {
item.addEventListener('click', () => {
  const answer = item.querySelector('.answer');
  if (answer) {
      answer.style.display = answer.style.display === 'block' ? 'none' : 'block';
  } else {
      console.error('Answer element not found within faq-item.');
  }
});
});

// Scroll-to-Top Button
const scrollToTopBtn = document.getElementById('scrollToTop');
if (scrollToTopBtn) {
  window.addEventListener('scroll', () => {
      if (window.scrollY > 200) {
          scrollToTopBtn.style.display = 'block';
      } else {
          scrollToTopBtn.style.display = 'none';
      }
  });

  scrollToTopBtn.addEventListener('click', () => {
      window.scrollTo({ top: 0, behavior: 'smooth' });
  });
} else {
  console.error('Element with ID "scrollToTop" not found.');
}

// Particle Effect
const canvas = document.getElementById("particles");
if (canvas) {
  const ctx = canvas.getContext("2d");
  if (ctx) {
      canvas.width = window.innerWidth;
      canvas.height = window.innerHeight;
      
      let particlesArray = [];
      class Particle {
          constructor(x, y, size, color, speedX, speedY) {
              this.x = x;
              this.y = y;
              this.size = size;
              this.color = color;
              this.speedX = speedX;
              this.speedY = speedY;
          }
          draw() {
              ctx.beginPath();
              ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
              ctx.fillStyle = this.color;
              ctx.fill();
          }
          update() {
              this.x += this.speedX;
              this.y += this.speedY;
              if (this.size > 0.2) this.size -= 0.1;
          }
      }
      function initParticles() {
          particlesArray = [];
          for (let i = 0; i < 100; i++) {
              const x = Math.random() * canvas.width;
              const y = Math.random() * canvas.height;
              const size = Math.random() * 5 + 1;
              const color = `rgba(${Math.floor(Math.random() * 256)}, ${Math.floor(Math.random() * 256)}, ${Math.floor(Math.random() * 256)}, 0.8)`;
              const speedX = Math.random() * 3 - 1.5;
              const speedY = Math.random() * 3 - 1.5;
              particlesArray.push(new Particle(x, y, size, color, speedX, speedY));
          }
      }
      function animateParticles() {
          ctx.clearRect(0, 0, canvas.width, canvas.height);
          for (let i = 0; i < particlesArray.length; i++) {
              particlesArray[i].draw();
              particlesArray[i].update();
              if (particlesArray[i].size <= 0.2) {
                  particlesArray.splice(i, 1);
                  i--;
              }
          }
          requestAnimationFrame(animateParticles);
      }
      initParticles();
      animateParticles();
      window.addEventListener("resize", debounce(() => {
          canvas.width = window.innerWidth;
          canvas.height = window.innerHeight;
          initParticles();
      }, 250));
  } else {
      console.error('Failed to get 2D context from canvas.');
  }
} else {
  console.error('Canvas element with ID "particles" not found.');
}

// Debounce Function to Optimize Resize Events
function debounce(func, delay) {
  let debounceTimer;
  return function() {
      const context = this;
      const args = arguments;
      clearTimeout(debounceTimer);
      debounceTimer = setTimeout(() => func.apply(context, args), delay);
  };
}

// Interactive Easter Egg Implementation
const easterEggTrigger = document.getElementById('easter-egg-trigger');
const easterEggPopup = document.getElementById('easter-egg-popup');

if (easterEggTrigger && easterEggPopup) {
  let clickCount = 0;
  const requiredClicks = 5;

  easterEggTrigger.addEventListener('click', () => {
      clickCount++;
      if (clickCount === requiredClicks) {
          activateEasterEgg();
          clickCount = 0;
      }
  });

  function activateEasterEgg() {
      // Populate the popup with mini-game content
      easterEggPopup.innerHTML = `
          <h2>🎉 Congratulations! 🎉</h2>
          <p>You discovered the hidden Packet Pal!</p>
          <img src="images/special_pal.png" alt="Special Packet Pal" class="special-pal">
          <button id="close-easter-egg">Close</button>
      `;
      easterEggPopup.style.display = 'block';

      // Add event listener to close button
      const closeButton = document.getElementById('close-easter-egg');
      if (closeButton) {
          closeButton.addEventListener('click', () => {
              easterEggPopup.style.display = 'none';
          });
      }

      // Optional: Add a simple interactive animation or game here
      // For example, animate the special Packet Pal or provide a fun fact
  }
} else {
  if (!easterEggTrigger) {
      console.error('Element with ID "easter-egg-trigger" not found.');
  }
  if (!easterEggPopup) {
      console.error('Element with ID "easter-egg-popup" not found.');
  }
}

// DOMContentLoaded for Pal of the Day Features
document.addEventListener("DOMContentLoaded", () => {
  // Packet Pals Array
  const pals = [
      {
          name: "Antenna Bot",
          image: "antenna_bot.png",
          description: "Antenna Bot uses its signal to uncover hidden networks."
      },
      {
          name: "Bubble Bot",
          image: "bubble_bot.png",
          description: "Bubble Bot floats through networks, creating protective shields."
      },
      {
          name: "Buzz Droid",
          image: "buzz_droid.png",
          description: "Buzz Droid emits electric pulses to battle foes."
      },
      {
          name: "Circuit Rabbit",
          image: "circuit_rabbit.png",
          description: "Circuit Rabbit hops through data streams with lightning speed."
      },
      // Add more pals as needed
  ];

  // Function to select Pal of the Day based on current date
  function getPalOfTheDay(pals) {
      const today = new Date();
      const year = today.getFullYear();
      const month = today.getMonth() + 1; // Months are 0-based
      const day = today.getDate();
      const dateNumber = year * 10000 + month * 100 + day;
      const index = dateNumber % pals.length;
      return pals[index];
  }

  // Get Pal of the Day
  const palOfTheDay = getPalOfTheDay(pals);

  // Update Pal of the Day Section
  const palImage = document.getElementById("pal-image");
  const palName = document.getElementById("pal-name");
  const palDescription = document.getElementById("pal-description");

  if (palImage && palName && palDescription) {
      palImage.src = `images/${palOfTheDay.image}`;
      palImage.alt = palOfTheDay.name;
      palName.textContent = palOfTheDay.name;
      palDescription.textContent = palOfTheDay.description;
  } else {
      console.error('Pal of the Day elements not found.');
  }

  // Optional: Preload Pal Image for Better Performance
  const preloadImage = new Image();
  preloadImage.src = `images/${palOfTheDay.image}`;
});
