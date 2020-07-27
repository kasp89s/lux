<?php
namespace app\commands;
use yii\console\Controller;
use yii\httpclient\Client;

class OlxController extends Controller
{
    const REQUEST_PATTERN_TELEGRAM = 'https://api.telegram.org/bot:HASH:/:ACTION:';

    protected $_pdo;

    protected $_initialParams;

    protected $_informParams = [
        'notebooks' => [
            'chatID' => '-1001307607119',
            'listLink' => 'https://www.olx.ua/elektronika/noutbuki-i-aksesuary/noutbuki/?search%5Bfilter_float_price%3Ato%5D=1000',
        ],
//        'phones' => [
//            'chatID' => '-1001359657814',
//            'listLink' => 'https://www.olx.ua/elektronika/telefony-i-aksesuary/mobilnye-telefony-smartfony/samsung/?search%5Bfilter_float_price%3Ato%5D=1000',
//        ],
//        'video-cards' => [
//            'chatID' => '-1001483356841',
//            'listLink' => 'https://www.olx.ua/elektronika/kompyutery-i-komplektuyuschie/komplektuyuschie-i-aksesuary/videokarty/?search%5Bfilter_float_price%3Ato%5D=1000',
//        ],
        'desktop' => [
            'chatID' => '-1001405907012',
            'listLink' => 'https://www.olx.ua/elektronika/kompyutery-i-komplektuyuschie/komplektuyuschie-i-aksesuary/materinskie-platy/?search%5Bfilter_float_price%3Ato%5D=1000',
        ],
//        'monitor' => [
//            'chatID' => '-1001364285268',
//            'listLink' => 'https://www.olx.ua/elektronika/kompyutery-i-komplektuyuschie/monitory/?search%5Bfilter_float_price%3Ato%5D=1000',
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
        $client->createRequest()
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
     * @param $link
     * @return bool
     */
    protected function saveRecord($link)
    {
        $isNew = false;
        $STH = $this->_pdo->prepare("SELECT * FROM records WHERE link = :link");
        $STH->execute(['link' => $link]);
        $STH->setFetchMode(\PDO::FETCH_OBJ);
        $item = $STH->fetch();

        if (empty($item)) {
            $isNew = true;
            $STH = $this->_pdo->prepare(
                "INSERT INTO records (link) VALUES (:link)"
            );
            $STH->execute(['link' => $link]);
        }

        return $isNew;
    }

    protected function processQueue($html)
    {
        preg_match_all('|<table(.*)</table>|isU', $html, $matches);

        foreach ($matches[0] as $item) {
            $data = [];
            preg_match('|<p class="price">(.*)</p>|isU', $item, $out);
            if (empty($out[0]))
                continue;

            preg_match('|<span class="inlblk icon paid type2 abs zi2" title="ТОП">|isU', $item, $top);

            if (!empty($top))
                continue;

            preg_match('|<img class="fleft" src="(.*)" alt="(.*)">|isU', $item, $match);
            preg_match_all('|href="(.*)"|isU', $item, $match1);
            preg_match('|<p class="lheight16">(.*)</p>|isU', $item, $match2);

            if (empty($match[2]))
                continue;

            $data['name'] = '<i>' . $match[2] . '</i>';
            $data['price'] = '<b>' . preg_replace('/[^0-9]/', '', $out[0]) . ' грн</b>';
            $data['place'] = trim(preg_replace('/\s+/', ' ', strip_tags($match2[0])));
            $data['image'] = $match[1];
            $data['link'] = $match1[1][1];

            $isNew = $this->saveRecord($data['image']);

            if ($isNew)
                $this->sendMessage($this->_initialParams['hash'], $this->_initialParams['chatID'], implode(' ', $data));
        }
    }
}