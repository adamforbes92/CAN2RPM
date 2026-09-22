document.addEventListener('DOMContentLoaded', initApp);

let cachedSettings = {};

// ─────────────────────────────────────────────────────────────────
function initApp() {
    initNavigation();
    initControls();
    initCollapsibleCards();
    initGaugeUI();
    fetchSettings();
    fetchStatus();
    setInterval(fetchStatus, 1000);
}

// ─────────────────────────────────────────────────────────────────
function initNavigation() {
    const tabs  = document.querySelectorAll('.nav-tab');
    const pages = document.querySelectorAll('.page');

    tabs.forEach(tab => {
        tab.addEventListener('click', () => {
            const page = tab.dataset.page;
            tabs.forEach(t => t.classList.remove('active'));
            tab.classList.add('active');
            pages.forEach(p => p.classList.remove('active'));
            document.getElementById(`${page}-page`).classList.add('active');
        });
    });
}

// ─────────────────────────────────────────────────────────────────
function initControls() {
    // Needle sweep test button
    const testBtn = document.getElementById('testNeedleSweep');
    if (testBtn) testBtn.addEventListener('click', () => pushAction('needleSweep'));

    // Simple checkbox / number inputs
    ['hasNeedleSweep', 'sweepSpeed'].forEach(id => {
        const el = document.getElementById(id);
        if (el) {
            el.addEventListener('change', () => {
                const value = el.type === 'checkbox' ? el.checked : el.value;
                pushControl(id, value);
            });
        }
    });

    // Test mode checkbox
    const testCheckbox = document.getElementById('tempDiagTest');
    if (testCheckbox) {
        testCheckbox.addEventListener('change', () => {
            pushControl('tempDiagTest', testCheckbox.checked);
            const display = document.getElementById('tempRPM-display');
            if (display) display.style.color = testCheckbox.checked ? 'orange' : '';
        });
    }

    const ecuModeEl = document.getElementById('ecuMode');
    const aftermarketCard = document.getElementById('aftermarketCard');
    if (ecuModeEl) {
        ecuModeEl.addEventListener('change', () => {
            const mode = Number(ecuModeEl.value) === 1 ? 1 : 0;
            pushControl('ecuMode', mode);
            if (aftermarketCard) {
                aftermarketCard.style.display = mode === 1 ? '' : 'none';
            }
        });
    }

    const afmCanIdEl = document.getElementById('aftermarketCanIdHex');
    if (afmCanIdEl) {
        afmCanIdEl.addEventListener('change', () => {
            const clean = afmCanIdEl.value.replace(/^0x/i, '').replace(/[^0-9a-fA-F]/g, '').toUpperCase();
            afmCanIdEl.value = clean;
            pushControl('aftermarketCanIdHex', clean);
        });
    }

    ['aftermarketByteLow', 'aftermarketByteHigh', 'aftermarketMultiplier', 'aftermarketAddition'].forEach(id => {
        const el = document.getElementById(id);
        if (!el) return;
        el.addEventListener('change', () => {
            const num = id === 'aftermarketMultiplier' ? parseFloat(el.value) : Number(el.value);
            pushControl(id, Number.isFinite(num) ? num : 0);
        });
    });

    // tempRPM slider
    const tempRPMEl = document.getElementById('tempRPM');
    if (tempRPMEl) {
        tempRPMEl.addEventListener('change', () => pushControl('tempRPM', Number(tempRPMEl.value)));
        tempRPMEl.addEventListener('input', () => {
            const d = document.getElementById('tempRPM-display');
            if (d) d.textContent = tempRPMEl.value;
        });
    }

    // clusterRPMLimit slider
    const clusterEl = document.getElementById('clusterRPMLimit');
    if (clusterEl) {
        clusterEl.addEventListener('change', () => {
            pushControl('clusterRPMLimit', Number(clusterEl.value));
            cachedSettings.clusterRPMLimit = Number(clusterEl.value);
        });
        clusterEl.addEventListener('input', () => {
            const d = document.getElementById('clusterRPMLimit-display');
            if (d) d.textContent = clusterEl.value;
        });
    }

    // maxRPM slider
    const maxRPMEl = document.getElementById('maxRPM');
    if (maxRPMEl) {
        maxRPMEl.addEventListener('change', () => {
            pushControl('maxRPM', Number(maxRPMEl.value));
            cachedSettings.maxRPM = Number(maxRPMEl.value);
        });
        maxRPMEl.addEventListener('input', () => {
            const d = document.getElementById('maxRPM-display');
            if (d) d.textContent = maxRPMEl.value;
        });
    }

    // Reset buttons
    const resetClusterBtn = document.getElementById('resetClusterRPM');
    if (resetClusterBtn) {
        resetClusterBtn.addEventListener('click', () => {
            if (clusterEl) {
                clusterEl.value = 7000;
                document.getElementById('clusterRPMLimit-display').textContent = '7000';
                cachedSettings.clusterRPMLimit = 7000;
            }
            pushAction('resetClusterRPM');
        });
    }

    const resetScalingBtn = document.getElementById('resetRPMScaling');
    if (resetScalingBtn) {
        resetScalingBtn.addEventListener('click', () => {
            if (maxRPMEl) {
                maxRPMEl.value = 230;
                document.getElementById('maxRPM-display').textContent = '230';
                cachedSettings.maxRPM = 230;
            }
            pushAction('resetRPMScaling');
        });
    }
}

// ─────────────────────────────────────────────────────────────────
async function fetchSettings() {
    try {
        const resp = await fetch('/api/settings');
        const data = await resp.json();
        cachedSettings = data;

        document.getElementById('hasNeedleSweep').checked = data.hasNeedleSweep || false;
        document.getElementById('sweepSpeed').value       = data.sweepSpeed     || 18;

        const clusterVal = data.clusterRPMLimit || 7000;
        document.getElementById('clusterRPMLimit').value                  = clusterVal;
        document.getElementById('clusterRPMLimit-display').textContent    = clusterVal;

        const maxVal = data.maxRPM || 230;
        document.getElementById('maxRPM').value              = maxVal;
        document.getElementById('maxRPM-display').textContent = maxVal;

        document.getElementById('tempDiagTest').checked  = data.tempDiagTest || false;
        document.getElementById('tempRPM').value         = data.tempRPM      || 0;

        const ecuModeVal = Number(data.ecuMode) === 1 ? 1 : 0;
        const ecuModeEl = document.getElementById('ecuMode');
        const aftermarketCard = document.getElementById('aftermarketCard');
        if (ecuModeEl) ecuModeEl.value = String(ecuModeVal);
        if (aftermarketCard) aftermarketCard.style.display = ecuModeVal === 1 ? '' : 'none';

        const afmCanIdEl = document.getElementById('aftermarketCanIdHex');
        if (afmCanIdEl) {
            afmCanIdEl.value = (data.aftermarketCanIdHex || '1001').toString().replace(/^0x/i, '').toUpperCase();
        }
        const afmLo = document.getElementById('aftermarketByteLow');
        const afmHi = document.getElementById('aftermarketByteHigh');
        const afmMult = document.getElementById('aftermarketMultiplier');
        const afmAdd = document.getElementById('aftermarketAddition');
        if (afmLo) afmLo.value = data.aftermarketByteLow ?? 2;
        if (afmHi) afmHi.value = data.aftermarketByteHigh ?? 3;
        if (afmMult) afmMult.value = data.aftermarketMultiplier ?? 1;
        if (afmAdd) afmAdd.value = data.aftermarketAddition ?? 0;

        const tempDisplay = document.getElementById('tempRPM-display');
        tempDisplay.textContent = data.tempRPM || 0;
        tempDisplay.style.color = data.tempDiagTest ? 'orange' : '';

        document.getElementById('fwVersion').textContent =
            'FW: ' + (data.FW_VERSION || '--');

    } catch (e) {
        console.log('Settings error:', e);
    }
}

// ─────────────────────────────────────────────────────────────────
async function fetchStatus() {
    try {
        const resp = await fetch('/api/status');
        const data = await resp.json();

        const testActive = data.tempDiagTest || false;
        const canOk      = data.hasCAN       || false;

        // Dashboard gauges
        const rpmEl = document.getElementById('rpm');
        const rpmValue = testActive ? data.tempRPM : data.vehicleRPM;
        rpmEl.textContent  = rpmValue !== undefined ? rpmValue : '--';
        rpmEl.style.color  = testActive ? 'orange' : '';
        rpmEl.title        = testActive ? 'Test Mode: ' + (data.tempRPM || 0) + ' RPM' : '';

        const hzEl = document.getElementById('outputHz');
        hzEl.textContent  = data.outputHz !== undefined ? data.outputHz : '--';
        hzEl.style.color  = testActive ? 'orange' : '';

        // Header status badge
        const badge = document.getElementById('canStatus');
        badge.textContent = canOk ? 'CAN: Healthy' : 'CAN: Unhealthy';
        badge.className   = 'status-badge ' + (canOk ? 'connected' : 'error');

        // Dashboard system status
        const canPresentEl = document.getElementById('canPresent');
        canPresentEl.textContent = canOk ? 'Healthy' : 'Unhealthy';
        canPresentEl.className   = 'status-value pill ' + (canOk ? 'ok' : 'bad');
        const tmEl = document.getElementById('testModeStatus');
        tmEl.textContent = testActive ? 'Active' : 'Off';
        tmEl.className   = 'status-value pill' + (testActive ? ' warn' : '');
        tmEl.style.color = '';
        updateTileGauges();

        // Advanced live diagnostics
        const liveRPM    = document.getElementById('liveCANRPM');
        const liveHz     = document.getElementById('liveOutputHz');
        const liveStatus = document.getElementById('liveCANStatus');
        const liveTruth  = document.getElementById('liveRPMTruth');

        if (liveRPM) {
            const liveRPMValue = testActive ? data.tempRPM : data.vehicleRPM;
            liveRPM.textContent = liveRPMValue !== undefined ? liveRPMValue : '--';
            liveRPM.style.color = testActive ? 'orange' : '';
        }
        if (liveHz) {
            liveHz.textContent = data.outputHz !== undefined ? data.outputHz : '--';
            liveHz.style.color = testActive ? 'orange' : '';
        }
        if (liveStatus) {
            liveStatus.textContent  = canOk ? 'Healthy' : 'Unhealthy';
            liveStatus.style.color  = canOk ? '' : 'var(--danger)';
        }
        if (liveTruth) {
            const truth = data.rpmTruth === true;
            liveTruth.textContent = truth ? 'Valid' : 'Invalid';
            liveTruth.style.color = truth ? '' : 'var(--danger)';
        }

    } catch (e) {
        console.log('Status error:', e);
    }
}

// ─────────────────────────────────────────────────────────────────
function pushControl(key, value) {
    fetch('/api/control', {
        method:  'POST',
        headers: { 'Content-Type': 'application/json' },
        body:    JSON.stringify({ key, value })
    }).catch(e => console.log('Control error:', e));
}

function pushAction(action) {
    fetch('/api/action', {
        method:  'POST',
        headers: { 'Content-Type': 'application/json' },
        body:    JSON.stringify({ action })
    }).catch(e => console.log('Action error:', e));
}

// ---- Collapsible cards (config/advanced collapse by default) -------------
function initCollapsibleCards() {
    ['configuration-page', 'advanced-page'].forEach((pageId) => {
        const page = document.getElementById(pageId);
        if (!page) return;
        page.querySelectorAll('.card').forEach((card) => {
            if (card.classList.contains('no-collapse')) return;
            card.classList.add('collapsible', 'collapsed');
            const h2 = card.querySelector('h2');
            if (h2) h2.addEventListener('click', () => card.classList.toggle('collapsed'));
        });
    });
}

/* =======================================================================
   Per-tile dial gauges (ported from the OpenHaldex theme). Any dashboard
   .gauge tile below can render as a 270 degree dial instead of a number,
   toggled per-tile in Display Options and saved in this browser.
   ======================================================================= */
const GAUGE_TILES = [
    { id: 'rpm',      label: 'RPM',       min: 0, max: 8000, unit: 'rpm' },
    { id: 'outputHz', label: 'Frequency', min: 0, max: 300,  unit: 'Hz' },
];
const TG_R = 40;
const TG_CIRC = 2 * Math.PI * TG_R;
const TG_ARC = TG_CIRC * 0.75;
const TG_GAP = TG_CIRC - TG_ARC;
const GAUGE_PREFS_KEY = 'can2rpmGaugePrefs';
const GAUGE_DEFAULTS = { tiles: ['rpm', 'outputHz'] };
let gaugePrefs = loadGaugePrefs();

function loadGaugePrefs() {
    try {
        const raw = localStorage.getItem(GAUGE_PREFS_KEY);
        if (raw) {
            const p = JSON.parse(raw);
            return { tiles: Array.isArray(p.tiles) ? p.tiles : GAUGE_DEFAULTS.tiles.slice() };
        }
    } catch (e) { /* defaults */ }
    return { tiles: GAUGE_DEFAULTS.tiles.slice() };
}
function saveGaugePrefs() {
    try { localStorage.setItem(GAUGE_PREFS_KEY, JSON.stringify(gaugePrefs)); } catch (e) {}
}
function ensureTileGauge(tile) {
    if (tile.querySelector('.tile-gauge')) return;
    const wrap = document.createElement('div');
    wrap.className = 'tile-gauge';
    wrap.innerHTML =
        `<svg viewBox="0 0 100 100" aria-hidden="true">` +
        `<circle class="tg-track" cx="50" cy="50" r="${TG_R}" transform="rotate(135 50 50)" ` +
        `stroke-dasharray="${TG_ARC.toFixed(2)} ${TG_GAP.toFixed(2)}"/>` +
        `<circle class="tg-fill" cx="50" cy="50" r="${TG_R}" transform="rotate(135 50 50)" ` +
        `stroke-dasharray="0 ${TG_CIRC.toFixed(2)}"/>` +
        `<text class="tg-val" x="50" y="52" text-anchor="middle">--</text>` +
        `<text class="tg-unit" x="50" y="66" text-anchor="middle"></text>` +
        `<text class="tg-min" x="24" y="92" text-anchor="middle">0</text>` +
        `<text class="tg-max" x="76" y="92" text-anchor="middle">0</text>` +
        `</svg>`;
    tile.appendChild(wrap);
}
function applyGaugePrefs() {
    GAUGE_TILES.forEach((t) => {
        const el = document.getElementById(t.id);
        if (!el) return;
        const tile = el.closest('.gauge');
        if (!tile) return;
        ensureTileGauge(tile);
        const on = gaugePrefs.tiles.includes(t.id);
        tile.classList.toggle('as-gauge', on);
        if (on) {
            const unitEl = tile.querySelector('.tg-unit');
            const srcUnit = tile.querySelector('.gauge-unit');
            if (unitEl) unitEl.textContent = srcUnit ? srcUnit.textContent.trim() : t.unit;
            const minEl = tile.querySelector('.tg-min');
            const maxEl = tile.querySelector('.tg-max');
            if (minEl) minEl.textContent = t.min;
            if (maxEl) maxEl.textContent = t.max;
        }
    });
}
function updateTileGauges() {
    GAUGE_TILES.forEach((t) => {
        const el = document.getElementById(t.id);
        if (!el) return;
        const tile = el.closest('.gauge');
        if (!tile || !tile.classList.contains('as-gauge')) return;
        const raw = parseFloat(el.textContent);
        const valEl = tile.querySelector('.tg-val');
        const fillEl = tile.querySelector('.tg-fill');
        if (!valEl || !fillEl) return;
        const gaugeWrap = tile.querySelector('.tile-gauge');
        if (gaugeWrap) gaugeWrap.classList.toggle('warn', el.style.color === 'orange');
        if (Number.isNaN(raw)) {
            valEl.textContent = '--';
            fillEl.style.strokeDasharray = `0 ${TG_CIRC.toFixed(2)}`;
            return;
        }
        valEl.textContent = el.textContent;
        const frac = Math.max(0, Math.min(1, (raw - t.min) / (t.max - t.min || 1)));
        fillEl.style.strokeDasharray = `${(TG_ARC * frac).toFixed(2)} ${TG_CIRC.toFixed(2)}`;
    });
}
function initGaugeUI() {
    const host = document.getElementById('gaugeCustomizer');
    if (host) {
        host.innerHTML = '';
        GAUGE_TILES.forEach((t) => {
            const label = document.createElement('label');
            label.className = 'tile-opt';
            const cb = document.createElement('input');
            cb.type = 'checkbox';
            cb.checked = gaugePrefs.tiles.includes(t.id);
            cb.addEventListener('change', () => {
                const set = new Set(gaugePrefs.tiles);
                if (cb.checked) set.add(t.id); else set.delete(t.id);
                gaugePrefs.tiles = [...set];
                saveGaugePrefs();
                applyGaugePrefs();
            });
            const span = document.createElement('span');
            span.textContent = t.label;
            label.appendChild(cb);
            label.appendChild(span);
            host.appendChild(label);
        });
    }
    applyGaugePrefs();
}

function showNotification(message, type = 'success') {
    const n = document.createElement('div');
    n.textContent = message;
    n.style.cssText = [
        'position:fixed', 'top:20px', 'left:50%', 'transform:translateX(-50%)',
        'padding:1rem 2rem',
        `background:${type === 'error' ? 'var(--danger)' : 'var(--success)'}`,
        'color:#fff', 'border-radius:8px', 'z-index:10000',
        'font-weight:600', 'box-shadow:0 4px 12px rgba(0,0,0,.3)'
    ].join(';');
    document.body.appendChild(n);
    setTimeout(() => {
        n.style.transition = 'opacity 0.3s';
        n.style.opacity    = '0';
        setTimeout(() => n.remove(), 300);
    }, 3000);
}
