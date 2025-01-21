////////////////////
// DOM Elements
////////////////////
const introDiv = document.getElementById("introDiv");
const tutorialTitle = document.getElementById("tutorialTitle");
const tutorialContent = document.getElementById("tutorialContent");
const tutorialBackBtn = document.getElementById("tutorialBackBtn");
const tutorialNextBtn = document.getElementById("tutorialNextBtn");
const tutorialDoneBtn = document.getElementById("tutorialDoneBtn");

const scanListBtn = document.getElementById("scanListBtn");
const viewPartyBtn = document.getElementById("viewPartyBtn");
const downloadWigleBtn = document.getElementById("downloadWigleBtn");
const clearWigleBtn = document.getElementById("clearWigleBtn");

const bgMusic = document.getElementById("bgMusic");

const monsterListDiv = document.getElementById("monsterList");
const paginationDiv = document.getElementById("pagination");
const prevPageBtn = document.getElementById("prevPageBtn");
const nextPageBtn = document.getElementById("nextPageBtn");
const pageInfoSpan = document.getElementById("pageInfo");

const battleUI = document.getElementById("battleUI");
const partyHPSpan = document.getElementById("partyHP");
const wildHPSpan = document.getElementById("wildHP");
const battleLog = document.getElementById("battleLog");

////////////////////
// Global Data
////////////////////
let allMonsters = [];
let myParty = [];
let currentPage = 0;
const PAGE_SIZE = 10;
let battleEnded = false;
let musicPlaying = false; // Track if BG music is playing
let currentTutorialStep = 0; // Track the current step in the tutorial

////////////////////
// Tutorial Content
////////////////////
const tutorialSteps = [
  {
    title: "Welcome to Packet Pals!",
    content: "Packet Pals is an adventure game where you discover and battle creatures by scanning Wi-Fi networks!",
  },
  {
    title: "Discover Monsters!",
    content: "Scan Wi-Fi networks to uncover unique monsters. Each network generates a different monster based on its properties.",
  },
  {
    title: "Battles and Parties",
    content: "Battle wild monsters and add them to your party. Your party can hold up to 3 monsters at a time!",
  },
  {
    title: "Learn About Wigle",
    content: "Wigle is a platform where you can contribute Wi-Fi data to support global research and mapping.",
  },
  {
    title: "Upload Wigle Data",
    content: "You can upload your data to Wigle from the main menu using the 'Download Wigle Data' button.",
  },
];

////////////////////
// On Page Load
////////////////////
window.addEventListener("DOMContentLoaded", () => {
  // Tutorial setup
  const introDone = localStorage.getItem("introDone");
  if (introDone === "true") {
    introDiv.style.display = "none";
  } else {
    updateTutorialStep(0); // Show the first tutorial step
    introDiv.style.display = "block";
  }

  // Tutorial buttons
  tutorialNextBtn.addEventListener("click", () => {
    if (currentTutorialStep < tutorialSteps.length - 1) {
      currentTutorialStep++;
      updateTutorialStep(currentTutorialStep);
    }
  });

  tutorialBackBtn.addEventListener("click", () => {
    if (currentTutorialStep > 0) {
      currentTutorialStep--;
      updateTutorialStep(currentTutorialStep);
    }
  });

  tutorialDoneBtn.addEventListener("click", () => {
    localStorage.setItem("introDone", "true");
    introDiv.style.display = "none";
  });

  // Main buttons
  if (scanListBtn) scanListBtn.addEventListener("click", () => {
    playBGMusic();
    doScanAndList();
  });
  if (viewPartyBtn) viewPartyBtn.addEventListener("click", () => {
    playBGMusic();
    viewMyParty();
  });
  if (downloadWigleBtn) downloadWigleBtn.addEventListener("click", () => {
    playBGMusic();
    downloadWigle();
  });
  if (clearWigleBtn) clearWigleBtn.addEventListener("click", () => {
    playBGMusic();
    clearWigle();
  });

  // Pagination
  if (prevPageBtn) prevPageBtn.addEventListener("click", goPrevPage);
  if (nextPageBtn) nextPageBtn.addEventListener("click", goNextPage);

  // Battle UI
  if (battleUI) {
    battleUI.querySelectorAll("button[data-action]").forEach(btn => {
      btn.addEventListener("click", () => {
        const action = btn.getAttribute("data-action");
        handleBattleAction(action);
      });
    });
  }
});

////////////////////
// Tutorial Logic
////////////////////
function updateTutorialStep(stepIndex) {
  const step = tutorialSteps[stepIndex];
  tutorialTitle.textContent = step.title;
  tutorialContent.textContent = step.content;

  // Update button visibility
  tutorialBackBtn.style.display = stepIndex > 0 ? "inline-block" : "none";
  tutorialNextBtn.style.display = stepIndex < tutorialSteps.length - 1 ? "inline-block" : "none";
  tutorialDoneBtn.style.display = stepIndex === tutorialSteps.length - 1 ? "inline-block" : "none";
}

////////////////////
// BG Music
////////////////////
function playBGMusic() {
  if (!musicPlaying) {
    bgMusic.play().then(() => {
      musicPlaying = true;
      showNotification("Background music started!", "success");
    }).catch(err => {
      showNotification("Failed to play background music. Tap to retry.", "error");
      console.error("BG music failed to play:", err);
    });
  }
}

function showNotification(message, type) {
  const notif = document.createElement("div");
  notif.textContent = message;
  notif.className = `notification ${type}`;
  document.body.appendChild(notif);
  setTimeout(() => notif.remove(), 3000);
}

////////////////////
// Wigle Data
////////////////////
function downloadWigle() {
  window.location.href = "/downloadWigle";
}
async function clearWigle() {
  let resp = await fetch("/clearWigle");
  if (resp.ok) {
    alert("Wigle data cleared!");
  } else {
    let txt = await resp.text();
    alert("Error clearing wigle data: " + txt);
  }
}

////////////////////
// Scanning & Listing
////////////////////
async function doScanAndList() {
  monsterListDiv.textContent = "Scanning...";
  paginationDiv.style.display = "none";
  try {
    await fetch("/scan");
    let resp = await fetch("/monsters");
    let data = await resp.json();
    allMonsters = data.monsters || [];
    currentPage = 0;
    renderMonstersWithPagination();
  } catch (err) {
    monsterListDiv.textContent = "Error scanning: " + err;
  }
}
function renderMonstersWithPagination() {
  battleUI.style.display = "none";
  if (allMonsters.length === 0) {
    monsterListDiv.textContent = "No monsters found.";
    paginationDiv.style.display = "none";
    return;
  }
  if (allMonsters.length <= PAGE_SIZE) {
    paginationDiv.style.display = "none";
  } else {
    paginationDiv.style.display = "block";
  }
  displayPage(currentPage);
}

function displayPage(pageIndex) {
  monsterListDiv.innerHTML = "";
  let start = pageIndex * PAGE_SIZE;
  let end = start + PAGE_SIZE;
  let slice = allMonsters.slice(start, end);

  slice.forEach((m, localIndex) => {
    let wIdx = start + localIndex;
    const row = document.createElement("div");
    row.textContent = `[${wIdx}] ${m.name} (Lv ${m.level}) `;
    const battleBtn = document.createElement("button");
    battleBtn.textContent = "Battle";
    battleBtn.onclick = () => startBattle(wIdx, 0);
    row.appendChild(battleBtn);
    monsterListDiv.appendChild(row);
  });
  let totalPages = Math.ceil(allMonsters.length / PAGE_SIZE);
  pageInfoSpan.textContent =
    `Page ${pageIndex + 1} of ${totalPages} (monsters ${start + 1}..${Math.min(end, allMonsters.length)})`;
}

function goPrevPage() {
  if (currentPage > 0) {
    currentPage--;
    displayPage(currentPage);
  }
}
function goNextPage() {
  let totalPages = Math.ceil(allMonsters.length / PAGE_SIZE);
  if (currentPage < totalPages - 1) {
    currentPage++;
    displayPage(currentPage);
  }
}

////////////////////
// Battles
////////////////////
async function startBattle(wIdx, pIdx) {
  let url = `/startBattle?wildIndex=${wIdx}&partyIndex=${pIdx}`;
  try {
    let resp = await fetch(url);
    if (!resp.ok) {
      let txt = await resp.text();
      alert("Error starting battle: " + txt);
      return;
    }
    let data = await resp.json();
    battleEnded = false;
    partyHPSpan.textContent = data.partyHP;
    wildHPSpan.textContent = data.wildHP;
    battleLog.textContent =
      `Battle started!\nYour monster: ${data.partyName} (Lv ${data.partyLevel}, HP ${data.partyHP})\n` +
      `Wild monster: ${data.wildName} (Lv ${data.wildLevel}, HP ${data.wildHP})\n`;
    battleUI.style.display = "block";
    monsterListDiv.style.display = "none";
  } catch (err) {
    alert("Error: " + err);
  }
}

async function handleBattleAction(action) {
  if (battleEnded) {
    battleLog.textContent = "Battle ended.\n";
    return;
  }
  let resp = await fetch(`/battleAction?action=${action}`);
  if (!resp.ok) {
    let txt = await resp.text();
    battleLog.textContent = `Error: ${txt}\n`;
    return;
  }
  let data = await resp.json();
  battleLog.textContent =
    `${data.message}\nParty HP: ${data.partyHP}, Wild HP: ${data.wildHP}\n`;

  partyHPSpan.textContent = data.partyHP;
  wildHPSpan.textContent = data.wildHP;

  if (data.battleEnd) {
    battleEnded = true;
    battleLog.textContent += "Battle ended!\n";
    setTimeout(() => {
      location.reload();
    }, 2000);
  }
}

////////////////////
// Party
////////////////////
async function viewMyParty() {
  battleUI.style.display = "none";
  monsterListDiv.innerHTML = "Loading your party...";
  await fetchMyParty();
  if (myParty.length === 0) {
    monsterListDiv.textContent = "Your party is empty!";
    return;
  }
  monsterListDiv.innerHTML = "<h2>Your Party:</h2>";
  myParty.forEach((p, i) => {
    const row = document.createElement("div");
    row.textContent = `Slot ${i}: ${p.name} (Lv ${p.level}, HP ${p.hp}, Def ${p.defense}) `;
    const removeBtn = document.createElement("button");
    removeBtn.textContent = "Remove";
    removeBtn.onclick = () => removeFromParty(i);
    row.appendChild(removeBtn);

    const swapBtn = document.createElement("button");
    swapBtn.textContent = "Swap";
    swapBtn.onclick = () => {
      let other = prompt(`Swap slot ${i} with slot? [0..${myParty.length - 1}]`);
      if (other === null) return;
      let s2 = parseInt(other, 10);
      if (isNaN(s2) || s2 < 0 || s2 >= myParty.length) {
        alert("Invalid slot");
        return;
      }
      swapPartySlots(i, s2);
    };
    row.appendChild(swapBtn);
    monsterListDiv.appendChild(row);
  });
}

async function fetchMyParty() {
  let resp = await fetch("/myParty");
  if (!resp.ok) {
    monsterListDiv.textContent = "Error: " + (await resp.text());
    myParty = [];
    return;
  }
  let data = await resp.json();
  myParty = data.party || [];
}

async function removeFromParty(slot) {
  let resp = await fetch(`/removeFromParty?slot=${slot}`);
  if (!resp.ok) {
    let txt = await resp.text();
    alert("Error removing monster: " + txt);
    return;
  }
  let d = await resp.json();
  alert(d.message);
  viewMyParty();
}

async function swapPartySlots(s1, s2) {
  let resp = await fetch(`/swapPartySlots?slot1=${s1}&slot2=${s2}`);
  if (!resp.ok) {
    let txt = await resp.text();
    alert("Error swapping: " + txt);
    return;
  }
  let d = await resp.json();
  alert(d.message);
  viewMyParty();
}
