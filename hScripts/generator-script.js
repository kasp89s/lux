const SERVER_ERROR_COOLDOWN = 300_000;
const SERVER_ERROR_RETRIES = 3;
const MAX_RETRIES = 22;

const config = {
    bike: {
        appToken: 'd28721be-fd2d-4b45-869e-9f253b554e50',
        promoId: '43e35910-c168-4634-ad4f-52fd764a843f',
        delay: 21_000,
        retry: 21_000,
        keys: 4,
    },
    clone: {
        appToken: '74ee0b5b-775e-4bee-974f-63e7f4d5bacb',
        promoId: 'fe693b26-b342-4159-8808-15e3ff7f8767',
        delay: 120_000,
        retry: 120_000,
        keys: 4,
    },
    cube: {
        appToken: 'd1690a07-3780-4068-810f-9b5bbf2931b2',
        promoId: 'b4170868-cef0-424f-8eb9-be0622e8e8e3',
        delay: 20_000,
        retry: 20_000,
        keys: 4,
    },
    merge: {
        appToken: '8d1cc2ad-e097-4b86-90ef-7a27e19fb833',
        promoId: 'dc128d28-c45b-411c-98ff-ac7726fbaea4',
        delay: 20_000,
        retry: 20_000,
        keys: 4,
    },
    train: {
        appToken: '82647f43-3f87-402d-88dd-09a90025313f',
        promoId: 'c4480ac7-e178-4973-8061-9ed5b2e17954',
        delay: 20_000,
        retry: 20_000,
        keys: 4,
    },
    twerk: {
        appToken: '61308365-9d16-4040-8bb0-2f4a4c69074c',
        promoId: '61308365-9d16-4040-8bb0-2f4a4c69074c',
        delay: 20_000,
        retry: 20_000,
        keys: 4,
    },
    polysphere: {
        appToken: '2aaf5aee-2cbc-47ec-8a3f-0962cc14bc71',
        promoId: '2aaf5aee-2cbc-47ec-8a3f-0962cc14bc71',
        delay: 10_000,
        retry: 10_000,
        keys: 4,
    }
};

let gameConfig = {};

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
    const selectedGame = document.getElementById('gameSelect').value;

    gameConfig = config[selectedGame];

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
                <button type="button" class="copy-btn" data-clipboard-target="#promoCode-${i}">💾</button>
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