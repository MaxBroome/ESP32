// State
let currentNetwork = null;
let pollTimer = null;

// DOM elements
const networksList = document.getElementById('networks');
const rescanBtn = document.getElementById('rescan-btn');
const modal = document.getElementById('modal');
const modalTitle = document.getElementById('modal-title');
const modalSubtitle = document.getElementById('modal-subtitle');
const warning = document.getElementById('warning');
const usernameGroup = document.getElementById('username-group');
const usernameInput = document.getElementById('username');
const passwordGroup = document.getElementById('password-group');
const passwordInput = document.getElementById('password');
const connectForm = document.getElementById('connect-form');
const connectBtn = document.getElementById('connect-btn');
const cancelBtn = document.getElementById('cancel-btn');
const statusEl = document.getElementById('status');
const mainContainer = document.getElementById('main-container');
const successScreen = document.getElementById('success-screen');
const successIp = document.getElementById('success-ip');

function getSignalStrength(rssi) {
    if (rssi > -50) return 4;
    if (rssi > -65) return 3;
    if (rssi > -80) return 2;
    return 1;
}

function createSignalBars(rssi) {
    const strength = getSignalStrength(rssi);
    return `
        <div class="signal strength-${strength}">
            <span></span><span></span><span></span><span></span>
        </div>
    `;
}

function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// Deduplicate SSIDs, keep strongest signal per SSID
function consolidateNetworks(networks) {
    const map = new Map();
    for (const net of networks) {
        const existing = map.get(net.ssid);
        if (!existing || net.rssi > existing.rssi) {
            map.set(net.ssid, net);
        }
    }
    return Array.from(map.values()).sort((a, b) => b.rssi - a.rssi);
}

function renderNetworks(networks) {
    if (!networks || networks.length === 0) {
        networksList.innerHTML = `
            <div class="empty">
                <p><strong>No networks found</strong></p>
                <p style="font-size: 0.875rem; margin-top: 0.5rem;">
                    Try rescanning or move closer to a router
                </p>
            </div>
        `;
        return;
    }

    const consolidated = consolidateNetworks(networks);

    networksList.innerHTML = consolidated.map(net => {
        const escaped = escapeHtml(net.ssid);
        const badge = net.enterprise ? '<span class="badge">Enterprise</span>'
                    : !net.open      ? '<span class="badge">Secured</span>'
                    : '';
        return `
            <div class="network" data-ssid="${escaped}" data-open="${net.open}" data-enterprise="${net.enterprise || false}">
                <div class="network-info">
                    <div class="network-name">${escaped} ${badge}</div>
                    <div class="network-meta">${net.rssi} dBm · Ch ${net.ch}</div>
                </div>
                ${createSignalBars(net.rssi)}
            </div>
        `;
    }).join('');
}

// Event delegation for network clicks
networksList.addEventListener('click', (e) => {
    const networkEl = e.target.closest('.network');
    if (!networkEl) return;

    const ssid = networkEl.dataset.ssid;
    const isOpen = networkEl.dataset.open === 'true';
    const isEnterprise = networkEl.dataset.enterprise === 'true';
    openModal(ssid, isOpen, isEnterprise);
});

// Modal
function openModal(ssid, isOpen, isEnterprise) {
    currentNetwork = { ssid, isOpen, isEnterprise };

    modalTitle.textContent = ssid;
    modalSubtitle.textContent = isEnterprise ? 'Enterprise network (WPA2-Enterprise)'
                              : isOpen       ? 'Open network'
                              : 'Secured network';

    warning.style.display = isOpen ? 'block' : 'none';

    // Enterprise networks need username + password
    usernameGroup.style.display = isEnterprise ? 'block' : 'none';
    usernameInput.value = '';

    // Open networks don't need a password
    passwordGroup.style.display = isOpen ? 'none' : 'block';
    passwordInput.value = '';

    statusEl.className = 'status';
    statusEl.textContent = '';

    connectBtn.disabled = false;
    connectBtn.textContent = 'Connect';

    modal.classList.add('show');
    document.body.classList.add('scroll-lock');

    if (isEnterprise) {
        setTimeout(() => usernameInput.focus(), 100);
    } else if (!isOpen) {
        setTimeout(() => passwordInput.focus(), 100);
    }
}

function closeModal() {
    modal.classList.remove('show');
    document.body.classList.remove('scroll-lock');
    currentNetwork = null;
    stopPolling();
}

function showStatus(type, message) {
    statusEl.className = `status show ${type}`;
    statusEl.textContent = message;
}

function restoreForm() {
    if (!currentNetwork) return;
    cancelBtn.style.display = '';
    if (currentNetwork.isEnterprise) usernameGroup.style.display = 'block';
    if (!currentNetwork.isOpen) passwordGroup.style.display = 'block';
    if (currentNetwork.isOpen) warning.style.display = 'block';
}

function showSuccessScreen(ip) {
    stopPolling();
    mainContainer.style.display = 'none';
    modal.classList.remove('show');
    document.body.classList.remove('scroll-lock');
    successIp.textContent = ip;
    successScreen.classList.add('show');
}

// -- Status polling ---------------------------------------------------------

function stopPolling() {
    if (pollTimer) {
        clearInterval(pollTimer);
        pollTimer = null;
    }
}

function startPolling() {
    stopPolling();
    pollTimer = setInterval(pollStatus, 1500);
}

async function pollStatus() {
    try {
        const res = await fetch('/api/status');
        const data = await res.json();

        if (data.status === 'connected') {
            showSuccessScreen(data.ip);
        } else if (data.status === 'failed') {
            stopPolling();
            restoreForm();
            showStatus('error', data.msg || 'Connection failed.');
            connectBtn.disabled = false;
            connectBtn.textContent = 'Connect';
        }
        // 'connecting' — keep polling
    } catch {
        // Fetch failed (AP might be going down) — stop polling silently
        stopPolling();
    }
}

// -- Network scan -----------------------------------------------------------

async function scanNetworks() {
    networksList.innerHTML = `
        <div class="loading">
            <div class="spinner"></div>
            <p>Scanning...</p>
        </div>
    `;
    rescanBtn.disabled = true;
    rescanBtn.textContent = 'Scanning...';

    try {
        const response = await fetch('/api/scan');
        const networks = await response.json();
        renderNetworks(networks);
    } catch {
        networksList.innerHTML = `
            <div class="empty">
                <p><strong>Scan failed</strong></p>
                <p style="font-size: 0.875rem; margin-top: 0.5rem;">Please try again</p>
            </div>
        `;
    } finally {
        rescanBtn.disabled = false;
        rescanBtn.textContent = 'Rescan';
    }
}

// -- Connect ----------------------------------------------------------------

async function handleConnect(e) {
    e.preventDefault();
    if (!currentNetwork) return;

    connectBtn.disabled = true;
    connectBtn.textContent = 'Connecting...';
    cancelBtn.style.display = 'none';
    statusEl.className = 'status';

    // Hide inputs so iOS captive portal webview can't re-focus them.
    usernameGroup.style.display = 'none';
    passwordGroup.style.display = 'none';
    warning.style.display = 'none';
    document.activeElement.blur();

    const payload = {
        ssid: currentNetwork.ssid,
        pass: passwordInput.value
    };

    if (currentNetwork.isEnterprise) {
        payload.user = usernameInput.value;
        payload.enterprise = 'true';
    }

    try {
        const response = await fetch('/api/connect', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        const result = await response.json();

        if (result.ok === false) {
            // Immediate validation error (missing SSID, enterprise not supported, etc.)
            restoreForm();
            showStatus('error', result.msg || 'Connection failed.');
            connectBtn.disabled = false;
            connectBtn.textContent = 'Connect';
            return;
        }

        // Server accepted the request and started connecting — poll for result.
        startPolling();
    } catch {
        restoreForm();
        showStatus('error', 'Request failed. Please try again.');
        connectBtn.disabled = false;
        connectBtn.textContent = 'Connect';
    }
}

// Event listeners
rescanBtn.addEventListener('click', scanNetworks);
cancelBtn.addEventListener('click', closeModal);
connectForm.addEventListener('submit', handleConnect);
modal.addEventListener('click', (e) => {
    if (e.target === modal) closeModal();
});

// Kick off initial scan
scanNetworks();
