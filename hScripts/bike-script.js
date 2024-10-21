const SERVER_ERROR_COOLDOWN = 300_000;
const SERVER_ERROR_RETRIES = 3;
const MAX_RETRIES = 22;
const gameConfig = {
    appToken: 'd28721be-fd2d-4b45-869e-9f253b554e50',
    promoId: '43e35910-c168-4634-ad4f-52fd764a843f',
    delay: 21_000,
    retry: 21_000,
    keys: 4,
};

async function delay(ms) {
    return new Promise((resolve) => setTimeout(resolve, ms));
}

function uuidv4() {
    return '10000000-1000-4000-8000-100000000000'.replace(
        /[018]/g,
        c => (c ^ crypto.getRandomValues(new Uint8Array(1))[0] & 15 >> c / 4).toString(16)
    );
}

async function fetchApi(path, authTokenOrBody = null, body = null, retry = 0) {
    const options = {
        method: 'POST',
        cache: 'no-store',
    };

    if (typeof authTokenOrBody === 'string') {
        options.headers = {
            ...(options.headers ?? {}),
            authorization: `Bearer ${authTokenOrBody}`,
        };
    }

    if ((authTokenOrBody !== null && typeof authTokenOrBody !== 'string') || body !== null) {
        options.headers = {
            ...(options.headers ?? {}),
            'content-type': 'application/json',
        };

        options.body = JSON.stringify(body ?? authTokenOrBody);
    }

    const url = `https://api.gamepromo.io${path}`;
    const res = await fetch(url, options);

    if (!res.ok) {
        if (retry < SERVER_ERROR_RETRIES && res.status >= 400) {
            await delay(SERVER_ERROR_COOLDOWN);
            return fetchApi(path, authTokenOrBody, body, retry + 1);
        }

        throw new Error(`${res.status} ${res.statusText}`);
    }

    return res.json();
}

async function getPromoCode() {
    const clientId = uuidv4();
    const loginClientData = await fetchApi('/promo/login-client', {
        appToken: gameConfig.appToken,
        clientId,
        clientOrigin: 'ios',
    });

    await delay(gameConfig.delay);

    const authToken = loginClientData.clientToken;
    let promoCode = null;

    for (let i = 0; i < MAX_RETRIES; i++) {
        const eventId = uuidv4();
        const registerEventData = await fetchApi('/promo/register-event', authToken, {
            promoId: gameConfig.promoId,
            eventId,
            eventOrigin: 'undefined'
        });

        if (!registerEventData.hasCode) {
            await delay(gameConfig.retry);
            continue;
        }

        const createCodeData = await fetchApi('/promo/create-code', authToken, {
            promoId: gameConfig.promoId,
        });

        promoCode = createCodeData.promoCode;
        break;
    }

    return promoCode;
}

document.getElementById('startBtn').addEventListener('click', async () => {
    const keyCount = parseInt(document.getElementById('keyCountSelect').value, 10);
    const loadingIcon = document.getElementById('loadingIcon');
    const keyContainer = document.getElementById('keyContainer');
    const generatedKeys = document.getElementById('generatedKeys');

    loadingIcon.classList.remove('hidden');
    generatedKeys.innerHTML = '';
    keyContainer.classList.add('hidden');

    try {
        for (let i = 0; i < keyCount; i++) {
            const promoCode = await getPromoCode();
            const keyElement = document.createElement('div');
            keyElement.className = 'input-group';
            keyElement.innerHTML = `
                <input type="text" id="promoCode-${i}" value="${promoCode}" readonly class="copy-target">
                <button type="button" class="copy-btn" data-clipboard-target="#promoCode-${i}">📋</button>
            `;
            generatedKeys.appendChild(keyElement);
        }
    } catch (error) {
        console.error('Error generating keys:', error);
        generatedKeys.innerHTML = '<div>Error generating keys. Please try again later.</div>';
    }

    loadingIcon.classList.add('hidden');
    keyContainer.classList.remove('hidden');
});

// Copy to clipboard
document.addEventListener('click', function(event) {
    if (event.target.classList.contains('copy-btn') || event.target.classList.contains('copy-target')) {
        const inputElement = event.target.classList.contains('copy-btn') ?
            document.querySelector(event.target.getAttribute('data-clipboard-target')) :
            event.target;
        inputElement.select();
        document.execCommand('copy');
    }
});