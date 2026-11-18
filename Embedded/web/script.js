"use strict";

// ─── Element handles ───────────────────────────────────────────────
const powerToggle = document.getElementById("power-toggle");
const brightnessSlider = document.getElementById("brightness");
const brightnessValue = document.getElementById("brightness-value");
const themeGrid = document.getElementById("theme-grid");
const themeNameInput = document.getElementById("theme-name");
const themeModeSelect = document.getElementById("theme-mode");
const colorRows = document.getElementById("color-rows");
const addColorButton = document.getElementById("add-color");
const saveThemeButton = document.getElementById("save-theme");
const formMessage = document.getElementById("form-message");

// ─── API helpers ───────────────────────────────────────────────────
async function api(path, options) {
  const response = await fetch(path, options);
  return response;
}

const postJSON = (path, body) =>
  api(path, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(body),
  });

// ─── State ─────────────────────────────────────────────────────────
async function refresh() {
  const [stateResponse, themesResponse] = await Promise.all([fetch("/api/state"), fetch("/api/themes")]);
  const state = await stateResponse.json();
  const { themes } = await themesResponse.json();

  powerToggle.checked = state.power;
  brightnessSlider.value = state.brightness;
  brightnessValue.textContent = `${Math.round((state.brightness / 255) * 100)}%`;

  renderThemes(themes);
}

// ─── Theme cards ───────────────────────────────────────────────────
function previewBackground(colors, mode) {
  if (colors.length === 0) {
    return "#222";
  }
  if (mode === "solid") {
    return colors[0];
  }
  if (mode === "gradient") {
    return `linear-gradient(90deg, ${colors.join(", ")})`;
  }
  // stripes: hard color stops, evenly divided
  const segment = 100 / colors.length;
  const stops = colors.flatMap((color, i) => {
    const start = (i * segment).toFixed(2);
    const end = ((i + 1) * segment).toFixed(2);
    return [`${color} ${start}%`, `${color} ${end}%`];
  });
  return `linear-gradient(90deg, ${stops.join(", ")})`;
}

function renderThemes(themes) {
  themeGrid.innerHTML = "";

  for (const theme of themes) {
    const card = document.createElement("button");
    card.className = "theme-card" + (theme.active ? " active" : "");
    card.type = "button";
    card.innerHTML = `
      <div class="theme-preview" style="background: ${previewBackground(theme.colors, theme.mode)}"></div>
      <div class="theme-name"></div>
      <div class="theme-mode">${theme.mode}</div>
    `;
    card.querySelector(".theme-name").textContent = theme.name;
    card.addEventListener("click", async () => {
      await postJSON("/api/state", { theme: theme.name });
      refresh();
    });

    if (!theme.builtin) {
      const deleteButton = document.createElement("button");
      deleteButton.className = "theme-delete";
      deleteButton.type = "button";
      deleteButton.textContent = "×";
      deleteButton.title = "Delete theme";
      deleteButton.addEventListener("click", async (event) => {
        event.stopPropagation();
        await api(`/api/themes?name=${encodeURIComponent(theme.name)}`, { method: "DELETE" });
        refresh();
      });
      card.appendChild(deleteButton);
    }

    themeGrid.appendChild(card);
  }
}

// ─── Power & brightness ────────────────────────────────────────────
powerToggle.addEventListener("change", async () => {
  await postJSON("/api/state", { power: powerToggle.checked });
  refresh();
});

brightnessSlider.addEventListener("input", () => {
  brightnessValue.textContent = `${Math.round((brightnessSlider.value / 255) * 100)}%`;
});

brightnessSlider.addEventListener("change", async () => {
  await postJSON("/api/state", { brightness: Number(brightnessSlider.value) });
  refresh();
});

// ─── Custom theme editor ───────────────────────────────────────────
function addColorRow(hex = "#d6291f") {
  const row = document.createElement("div");
  row.className = "color-row";

  const picker = document.createElement("input");
  picker.type = "color";
  picker.value = hex;

  const hexLabel = document.createElement("span");
  hexLabel.className = "hex";
  hexLabel.textContent = hex.toUpperCase();

  picker.addEventListener("input", () => {
    hexLabel.textContent = picker.value.toUpperCase();
  });

  const remove = document.createElement("button");
  remove.type = "button";
  remove.textContent = "×";
  remove.title = "Remove color";
  remove.addEventListener("click", () => {
    if (colorRows.children.length > 1) {
      row.remove();
    }
  });

  row.append(picker, hexLabel, remove);
  colorRows.appendChild(row);
}

function currentColors() {
  return [...colorRows.querySelectorAll('input[type="color"]')].map((input) => input.value.toUpperCase());
}

addColorButton.addEventListener("click", () => addColorRow("#165b33"));
saveThemeButton.addEventListener("click", async () => {
  formMessage.classList.remove("error");
  const name = themeNameInput.value.trim();
  const colors = currentColors();

  if (!name) {
    formMessage.textContent = "Give the theme a name.";
    formMessage.classList.add("error");
    return;
  }

  const response = await postJSON("/api/themes", { name, mode: themeModeSelect.value, colors });
  if (!response.ok) {
    let message = "Theme rejected.";
    try {
      const body = await response.json();
      if (body.error) {
        message = body.error;
      }
    } catch {
      /* keep default message */
    }
    formMessage.textContent = message;
    formMessage.classList.add("error");
    return;
  }

  formMessage.textContent = `Saved "${name}".`;
  themeNameInput.value = "";
  refresh();
});

// ─── Boot ──────────────────────────────────────────────────────────
addColorRow("#d6291f");
addColorRow("#165b33");
refresh();
setInterval(refresh, 10_000);
