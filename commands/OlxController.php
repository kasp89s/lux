<?php
namespace app\commands;
use yii\console\Controller;
use yii\httpclient\Client;

class OlxController extends Controller
{
    const REQUEST_PATTERN_TELEGRAM = 'https://api.telegram.org/bot:HASH:/:ACTION:';

    const POWER_BOT_HASH = '6968903777:AAHI_k6Gt6cOk8flDxFBKhgYp3UebOcC8oA';

    const POWER_CHAT_ID = '-1002196478879';

    const PRIVATE_BOT_HASH = '1089678800:AAF_JRk7mtCudhwzN7RCmAiq9PfO-kBImSg';
    const PRIVATE_CHAT_ID = '-1001307607119';
    protected $powerInformed = false;

    protected $_pdo;

    protected $_initialParams;

    protected $privateFilterList = [
        'robo_cleaner' => 'https://www.olx.ua/uk/elektronika/tehnika-dlya-doma/pylesosy/q-Xiaomi/?currency=UAH&search%5Border%5D=created_at:desc&search%5Bfilter_float_price:from%5D=100&search%5Bfilter_float_price:to%5D=1500',
    ];

    protected $powerFilterList = [
        'electronic' => 'https://www.olx.ua/uk/elektronika/kiev/?currency=UAH&search%5Bfilter_float_price:from%5D=free',
        // 'hobbi' => 'https://www.olx.ua/uk/hobbi-otdyh-i-sport/kiev/?currency=UAH&search%5Bfilter_float_price:from%5D=free',
        // 'detskiy' => 'https://www.olx.ua/uk/detskiy-mir/kiev/?currency=UAH&search%5Bfilter_float_price:from%5D=free',
        'dom' => 'https://www.olx.ua/uk/dom-i-sad/kiev/?currency=UAH&search%5Bfilter_float_price:from%5D=free',
        // 'shmot' => 'https://www.olx.ua/uk/moda-i-stil/kiev/?currency=UAH&search%5Bfilter_float_price:from%5D=free',
    ];

    protected $_informParams = [
        'notebooks' => [
            'chatID' => '-1001307607119',
            'listLink' => 'https://www.olx.ua/uk/elektronika/q-BLUETTI/?currency=UAH&search%5Bfilter_float_price:from%5D=1000&search%5Bfilter_float_price:to%5D=10000&search%5Bfilter_enum_type%5D%5B0%5D=ch_station',
        ],
//        'phones' => [
//            'chatID' => '-1001359657814',
//            'listLink' => 'https://www.olx.ua/elektronika/telefony-i-aksesuary/mobilnye-telefony-smartfony/samsung/?search%5Bfilter_float_price%3Ato%5D=1000',
//        ],
//        'video-cards' => [
//            'chatID' => '-1001483356841',
//            'listLink' => 'https://www.olx.ua/elektronika/kompyutery-i-komplektuyuschie/komplektuyuschie-i-aksesuary/videokarty/?search%5Bfilter_float_price%3Ato%5D=1000',
//        ],
//        'desktop' => [
//            'chatID' => '-1001405907012',
//            'listLink' => 'https://www.olx.ua/elektronika/igry-i-igrovye-pristavki/pristavki/q-playstation-4/?search%5Bfilter_float_price%3Ato%5D=2000',
//        ],
//        'monitor' => [
//            'chatID' => '-1001364285268',
//            'listLink' => 'https://www.olx.ua/list/q-Philips-32PFL9632D/',
//        ],
//        'tv' => [
//            'chatID' => '-1001206242449',
//            'listLink' => 'https://www.olx.ua/elektronika/tv-videotehnika/televizory/?search%5Bfilter_float_price%3Ato%5D=1000',
//        ]
    ];

    /**
     * Список доступных проксей.
     *
     * @var array
     */
    private $_proxyList = array(
        '195.138.77.24:80',
        '94.158.148.166:8080',
        '94.154.222.127:8080',
        '46.185.49.192:8080',
        '92.112.242.82:8080',
        '46.164.138.225:8080',
        '193.25.120.235:8080',
        '46.164.138.7:8080',

        '200.110.243.150:80',
        '218.203.54.8:80',
        '117.59.224.62:80',
        '112.220.65.195:80',
        '118.69.168.14:80',
        '117.59.224.58:80',
        '117.59.224.61:80',
        '183.238.55.236:80',
        '180.244.208.185:80',
    );

    public function actionGetUpdates($hash)
    {
        $client = new Client();
        $r = $client->createRequest()
            ->setMethod('post')
            ->setUrl('https://api.telegram.org/bot' . $hash . '/getUpdates')
            ->setHeaders([
                'Content-type' => 'application/json'
            ])
            ->send()->content;

        var_dump($r); exit;
    }

    public function actionAdsInform()
    {
        $this->informAdsFromBot(self::PRIVATE_BOT_HASH, self::PRIVATE_CHAT_ID, $this->privateFilterList);
    }

    public function actionPower()
    {
        $this->informAdsFromBot(self::POWER_BOT_HASH, self::POWER_CHAT_ID, $this->powerFilterList);
    }

    /**
     * Информирует
     *
     * @param $botHash
     * @param $chatId
     * @param $adsRequestList
     * @return void
     *
     * @throws \yii\base\InvalidConfigException
     * @throws \yii\httpclient\Exception
     */
    protected function informAdsFromBot($botHash, $chatId, $adsRequestList)
    {
        $this->_initialParams = [
            'hash' => $botHash,
        ];

        foreach ($adsRequestList as $dbName => $listLink) {
            echo "$dbName process ...";

            $this->_initialParams['chatID'] = $chatId;
            $this->_initialParams['listLink'] = $listLink;

            $this->_pdo = new \PDO("sqlite:$dbName.db");
            $this->_pdo->query(
                'CREATE TABLE if not exists records
             (
             id INTEGER PRIMARY KEY,
             extId INTEGER UNIQUE
             )'
            );

            $response = $this->loadRecordsList($this->_initialParams['listLink']);

            $this->processPowerQueue($response);

            if ($this->powerInformed === true) {
                break;
            }
        }
    }

    protected function processPowerQueue($html)
    {
        $ads = $this->extractAdsData($html);

        foreach ($ads as $ad) {
            if ($ad['isPromoted'] === true)
                continue;

            $data = [];

            $data['name'] = '🔈 <i>' . $ad['title'] . '</i>';
            $data['price'] = ' 🆓 <b>' . $ad['price']['displayValue'] . '</b>' . "\n\n";
            $data['description'] =  strip_tags($ad['description']) . "\n\n";

            $data['place'] =  '📍' . $ad['location']['pathName'] . " \n";
//            $data['image'] = "[ ](" . $ad['photos'][0] . ") \n";
//            $data['link'] = '🔗 <a href="'.$ad['url'].'">Забрати</a>' . " \n";

            $isNew = $this->isNewRecord($ad['id']);

            if ($isNew) {
                $this->powerInformed = true;

                $this->sendPhoto(
                    $this->_initialParams['hash'],
                    $this->_initialParams['chatID'],
                    implode(' ', $data),
                    $ad['photos'][0],
                    $ad['url']
                );
                $this->saveRecord($ad['id']);

                echo "->";
                sleep(rand(5, 8));
            }
        }
    }

    public function actionTest()
    {
//        $this->sendMessage('1089678800:AAF_JRk7mtCudhwzN7RCmAiq9PfO-kBImSg',
//            '-1007330693045',
//            'https://t.me/+RpUfV9WjW-liYzJi');

        $this->sendVideo('1089678800:AAF_JRk7mtCudhwzN7RCmAiq9PfO-kBImSg',
            '-1001307607119', 'дрюк дрюк', 'https://v1.paradisehill.cc/video/5bd73c3eb991f_Nh5qYo0Q6L_part1.mp4');
    }
    public function actionInform()
    {
        $this->_initialParams = [
            'hash' => '1089678800:AAF_JRk7mtCudhwzN7RCmAiq9PfO-kBImSg',
        ];

        foreach ($this->_informParams as $dbName => $params) {
            echo "$dbName process ...";
            $this->_initialParams['chatID'] = $params['chatID'];
            $this->_initialParams['listLink'] = $params['listLink'];

            $this->_pdo = new \PDO("sqlite:$dbName.db");
            $this->_pdo->query(
                'CREATE TABLE if not exists records
             (
             id INTEGER PRIMARY KEY,
             link TEXT
             )'
            );
            $response = $this->loadRecordsList($this->_initialParams['listLink']);

            $this->processQueue($response);
        }


        echo 'done!';
    }

    /**
     * Отправляет сообщение в чат.
     *
     * @param $botHash
     * @param $chatID
     * @param $text
     * @throws \yii\base\InvalidConfigException
     * @throws \yii\httpclient\Exception
     */
    protected function sendMessage($botHash, $chatID, $text)
    {
        $client = new Client();
        $response = $client->createRequest()
            ->setMethod('post')
            ->setUrl('https://api.telegram.org/bot' . $botHash . '/sendMessage')
            ->setHeaders([
                'Content-type' => 'application/json'
            ])
            ->setContent(json_encode([
                'chat_id' => $chatID,
                'parse_mode' => 'HTML',
                'text' => $text
            ]))
            ->send();

        var_dump($response->content);
    }

    protected function sendVideo($botHash, $chatID, $text, $video)
    {
        $client = new Client();
        $response = $client->createRequest()
            ->setMethod('post')
            ->setUrl('https://api.telegram.org/bot' . $botHash . '/sendVideo')
            ->setHeaders([
                'Content-type' => 'application/json'
            ])
            ->setContent(json_encode([
                'chat_id' => $chatID,
                'parse_mode' => 'HTML',
                'caption' => $text,
                'video' => $video
            ]))
            ->send();

        var_dump($response->content);
    }

    /**
     * Отправляет сообщение в чат.
     *
     * @param $botHash
     * @param $chatID
     * @param $text
     * @throws \yii\base\InvalidConfigException
     * @throws \yii\httpclient\Exception
     */
    protected function sendPhoto($botHash, $chatID, $text, $photo, $link)
    {
        $client = new Client();
        $response = $client->createRequest()
            ->setMethod('post')
            ->setUrl('https://api.telegram.org/bot' . $botHash . '/sendPhoto')
            ->setHeaders([
                'Content-type' => 'application/json'
            ])
            ->setContent(json_encode([
                'chat_id' => $chatID,
                'parse_mode' => 'HTML',
                'photo' => $photo,
                'caption' => $text,
                'reply_markup' => json_encode(array(
                    'inline_keyboard' => array(
                        array(
                            array(
                                'text' => '🤝 Забрати 🤝',
                                'url' => $link
                            )
                        )
                    ),
                )),
            ]))
            ->send();

//        var_dump($response->content);
    }

    /**
     * Загрузка списка обьяв.
     *
     * @param $url
     * @return string
     * @throws \yii\base\InvalidConfigException
     * @throws \yii\httpclient\Exception
     */
    protected function loadRecordsList($url)
    {
        $client = new Client();
        $proxy = $this->_proxyList[array_rand($this->_proxyList)];

        return $client->createRequest()
            ->setMethod('get')
            ->setUrl($url)
            ->setOptions([CURLOPT_PROXY, $proxy])
            ->send()->content;
    }

    /**
     * Проверка нова ли запись
     *
     * @param $id
     * @return bool
     */
    protected function isNewRecord($id)
    {
        $isNew = false;
        $STH = $this->_pdo->prepare("SELECT * FROM records WHERE extId = :extId");
        $STH->execute(['extId' => $id]);
        $STH->setFetchMode(\PDO::FETCH_OBJ);
        $item = $STH->fetch();

        if (empty($item)) {
            $isNew = true;
        }

        return $isNew;
    }

    /**
     * @param $link
     * @return bool
     */
    protected function saveRecord($id)
    {
        $isNew = false;
        $STH = $this->_pdo->prepare("SELECT * FROM records WHERE extId = :extId");
        $STH->execute(['extId' => $id]);
        $STH->setFetchMode(\PDO::FETCH_OBJ);
        $item = $STH->fetch();

        if (empty($item)) {
            $isNew = true;
            $STH = $this->_pdo->prepare(
                "INSERT INTO records (extId) VALUES (:extId)"
            );
            $STH->execute(['extId' => $id]);
        }

        return $isNew;
    }

    private function extractAdsData($content)
    {
        $content = strstr($content, 'window.__PRERENDERED_STATE__');
        $content = strstr($content, 'window.__TAURUS__', true);
        $content = trim($content);
        $content = str_replace('window.__PRERENDERED_STATE__= "', '', $content);
        $content = str_replace('";', '', $content);
        $content = stripslashes($content);

        $json = json_decode($content, true);

        return $json['listing']['listing']['ads'];
    }

    protected function processQueue($html)
    {
        $ads = $this->extractAdsData($html);

        foreach ($ads as $ad) {
            if ($ad['isPromoted'] === true)
                continue;

            $data = [];

            $data['name'] = '<i>' . $ad['title'] . '</i>' . "\n";
            $data['price'] = '<b>' . $ad['price']['displayValue'] . '</b>' . "\n";
            $data['place'] =  $ad['location']['pathName'] . "\n";
            $data['image'] = "[ ](" . $ad['photos'][0] . ") \n";
            $data['link'] = $ad['url'] . "\n";

            $isNew = $this->saveRecord($ad['id']);

            if ($isNew){
                $this->sendMessage($this->_initialParams['hash'], $this->_initialParams['chatID'], implode(' ', $data));
            }
        }
    }
}