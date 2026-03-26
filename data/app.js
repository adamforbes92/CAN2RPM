document.addEventListener('DOMContentLoaded', initApp);

let cachedSettings = {};

// ─────────────────────────────────────────────────────────────────
function initApp() {
    initNavigation();
    initControls();
    initOTA();
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
        rpmEl.textContent  = data.vehicleRPM !== undefined ? data.vehicleRPM : '--';
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
        document.getElementById('canPresent').textContent   = canOk ? 'Healthy' : 'Unhealthy';
        document.getElementById('testModeStatus').textContent = testActive ? 'Active' : 'Off';
        document.getElementById('testModeStatus').style.color = testActive ? 'orange' : '';

        // Advanced live diagnostics
        const liveRPM    = document.getElementById('liveCANRPM');
        const liveHz     = document.getElementById('liveOutputHz');
        const liveStatus = document.getElementById('liveCANStatus');
        const liveTruth  = document.getElementById('liveRPMTruth');

        if (liveRPM) {
            liveRPM.textContent = data.vehicleRPM !== undefined ? data.vehicleRPM : '--';
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

// ─────────────────────────────────────────────────────────────────
// OTA
// ─────────────────────────────────────────────────────────────────
function initOTA() {
    fetchOTAInfo();
    const fileInput = document.getElementById('otaFileInput');
    const uploadBtn = document.getElementById('otaUploadBtn');

    if (fileInput) {
        fileInput.addEventListener('change', e => {
            const file = e.target.files[0];
            if (!file) return;
            if (!file.name.endsWith('.bin')) {
                showNotification('Please select a .bin file', 'error');
                fileInput.value = '';
                document.getElementById('otaFileName').textContent = 'No file selected';
                uploadBtn.disabled = true;
                return;
            }
            document.getElementById('otaFileName').textContent =
                file.name + ` (${(file.size / 1024 / 1024).toFixed(2)} MB)`;
            uploadBtn.disabled = false;
        });
    }

    if (uploadBtn) uploadBtn.addEventListener('click', startOTAUpdate);
}

async function fetchOTAInfo() {
    try {
        const resp = await fetch('/api/ota/info');
        const data = await resp.json();
        document.getElementById('otaBoard').textContent          = data.board    || 'Unknown';
        document.getElementById('otaHardware').textContent       = data.hardware || 'Unknown';
        document.getElementById('otaCurrentVersion').textContent = data.version  || 'Unknown';
    } catch (e) {
        console.log('OTA info error:', e);
    }
}

async function startOTAUpdate() {
    const fileInput         = document.getElementById('otaFileInput');
    const file              = fileInput.files[0];
    if (!file) { showNotification('Please select a file', 'error'); return; }

    const uploadBtn         = document.getElementById('otaUploadBtn');
    const progressContainer = document.getElementById('otaProgressContainer');
    const progressFill      = document.getElementById('otaProgressFill');
    const progressPercent   = document.getElementById('otaProgressPercent');
    const statusMsg         = document.getElementById('otaStatusMessage');

    uploadBtn.disabled      = true;
    fileInput.disabled      = true;
    progressContainer.style.display = 'block';
    statusMsg.style.display = 'none';

    const formData = new FormData();
    formData.append('file', file);

    try {
        const xhr = new XMLHttpRequest();

        xhr.upload.addEventListener('progress', e => {
            if (e.lengthComputable) {
                const pct = Math.round((e.loaded / e.total) * 100);
                progressFill.style.width   = pct + '%';
                progressPercent.textContent = pct + '%';
                document.getElementById('otaProgressLabel').textContent =
                    `Uploading… ${(e.loaded / 1024 / 1024).toFixed(2)} / ${(e.total / 1024 / 1024).toFixed(2)} MB`;
            }
        });

        xhr.addEventListener('load', () => {
            const result = JSON.parse(xhr.responseText);
            progressContainer.style.display = 'none';
            statusMsg.style.display = 'block';

            if (xhr.status === 200) {
                statusMsg.className   = 'status-message success';
                statusMsg.textContent = result.message || 'Update successful! Rebooting…';
                showNotification('Firmware update started!', 'success');
                setTimeout(() => {
                    fileInput.value = '';
                    document.getElementById('otaFileName').textContent = 'No file selected';
                    uploadBtn.disabled = true;
                    fileInput.disabled = false;
                    fetchOTAInfo();
                }, 3000);
            } else {
                statusMsg.className   = 'status-message error';
                statusMsg.textContent = result.message || 'Update failed.';
                showNotification('Update failed: ' + (result.message || 'Unknown error'), 'error');
                uploadBtn.disabled = false;
                fileInput.disabled = false;
            }
        });

        xhr.addEventListener('error', () => {
            progressContainer.style.display = 'none';
            statusMsg.style.display  = 'block';
            statusMsg.className      = 'status-message error';
            statusMsg.textContent    = 'Network error during upload.';
            showNotification('Network error', 'error');
            uploadBtn.disabled = false;
            fileInput.disabled = false;
        });

        xhr.open('POST', '/api/ota/upload');
        xhr.send(formData);

    } catch (e) {
        progressContainer.style.display = 'none';
        statusMsg.style.display  = 'block';
        statusMsg.className      = 'status-message error';
        statusMsg.textContent    = 'Error: ' + e.message;
        uploadBtn.disabled = false;
        fileInput.disabled = false;
    }
}

// ─────────────────────────────────────────────────────────────────
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
