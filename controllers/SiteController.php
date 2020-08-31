<?php

namespace app\controllers;

use Codeception\PHPUnit\Constraint\Page;
use Yii;
use yii\filters\AccessControl;
use yii\httpclient\Client;
use yii\web\Controller;
use yii\web\Response;
use yii\filters\VerbFilter;
use app\models\LoginForm;
use app\models\ContactForm;

class SiteController extends Controller
{
    protected $_pdo;
    /**
     * {@inheritdoc}
     */
    public function behaviors()
    {
        return [
            'access' => [
                'class' => AccessControl::className(),
                'only' => ['logout', 'add'],
                'rules' => [
                    [
                        'actions' => ['logout', 'add'],
                        'allow' => true,
                        'roles' => ['@'],
                    ],
                ],
            ],
            'verbs' => [
                'class' => VerbFilter::className(),
                'actions' => [
//                    'logout' => ['post'],
                ],
            ],
        ];
    }

    /**
     * {@inheritdoc}
     */
    public function actions()
    {
        return [
            'error' => [
                'class' => 'yii\web\ErrorAction',
            ],
            'captcha' => [
                'class' => 'yii\captcha\CaptchaAction',
                'fixedVerifyCode' => YII_ENV_TEST ? 'testme' : null,
            ],
        ];
    }

    /**
     * Displays homepage.
     *
     * @return string
     */
    public function actionIndex()
    {
//        var_dump(Yii::$app->user->isGuest); exit;
        $this->_pdo = new \PDO("sqlite:games.db");
        $STH = $this->_pdo->prepare("SELECT * FROM games ORDER BY lastNewTime DESC");
        $STH->execute();
        $STH->setFetchMode(\PDO::FETCH_OBJ);
        $games = $STH->fetchAll();

        return $this->render('index', ['games' => $games]);
    }

    public function actionGame($id)
    {
//        http://api.steampowered.com/ISteamNews/GetNewsForApp/v0002/?appid=440&count=3&maxlength=300&format=json
        $response = $this->loadGameNews($id);

        $this->_pdo = new \PDO("sqlite:games.db");
        $STH = $this->_pdo->prepare("SELECT * FROM games WHERE id = :id");
        $STH->execute(['id' => $id]);
        $STH->setFetchMode(\PDO::FETCH_OBJ);
        $game = $STH->fetch();

//        var_dump($response['appnews']['newsitems']); exit;
        return $this->render('game', [
            'game' => $game,
            'news' => $response['appnews']['newsitems'],
        ]);
    }

    protected function loadGameNews($gameId)
    {
        $client = new Client();
        $response = $client->createRequest()
            ->setMethod('get')
            ->setUrl('http://api.steampowered.com/ISteamNews/GetNewsForApp/v0002/?appid=' . $gameId . '&count=3&maxlength=5000&format=json')
            ->send()->content;

        $response = json_decode($response, true);

        return $response;
    }

    /**
     * Displays about page.
     *
     * @return string
     */
    public function actionAdd()
    {
        return $this->render('add');
    }

    public function actionFavorites()
    {
        $this->_pdo = new \PDO("sqlite:games.db");
        $STH = $this->_pdo->prepare("SELECT * FROM favorites ORDER BY `time` DESC");
        $STH->execute();
        $STH->setFetchMode(\PDO::FETCH_OBJ);
        $favorites = $STH->fetchAll();
        return $this->render('favorites', ['favorites' => $favorites]);
    }

    public function actionAddFavorite()
    {
        \Yii::$app->response->format = \yii\web\Response::FORMAT_JSON;

        $data = json_decode(urldecode(\Yii::$app->request->post('data')));

        $this->_pdo = new \PDO("sqlite:games.db");
        $this->_pdo->query(
            'CREATE TABLE if not exists favorites
             (
             `id` INTEGER PRIMARY KEY,
             `gid` TEXT,
             `userID` TEXT,
             `time` TEXT,
             `data` TEXT
             )'
        );

        $STH = $this->_pdo->prepare(
            "INSERT INTO favorites (`gid`, `userID`, `time`, `data`) VALUES (:gid, :userID, :time, :data)"
        );
        $STH->execute([
            'gid' => $data->gid,
            'userID' => \Yii::$app->user->identity->id,
            'time' => $data->date,
            'data' => \Yii::$app->request->post('data')
        ]);

        return [];
    }

    public function actionGetList()
    {
        \Yii::$app->response->format = \yii\web\Response::FORMAT_JSON;

        $client = new Client();
        $response = $client->createRequest()
            ->setMethod('get')
            ->setUrl('https://store.steampowered.com/search/suggest?term='.\Yii::$app->request->post('request').'&cc=UA&realm=1&l=russian&excluded_content_descriptors%5B0%5D=3&excluded_content_descriptors%5B1%5D=4')
            ->send()->content;

        preg_match_all('|<li>(.*)</li>|isU', $response, $matches);

        return $matches[1];
    }

    public function actionResultList()
    {
        \Yii::$app->response->format = \yii\web\Response::FORMAT_JSON;

        $result = [];
        $client = new Client();
        $response = $client->createRequest()
            ->setMethod('get')
            ->setUrl('https://store.steampowered.com/search/results?term='.\Yii::$app->request->post('request'))
            ->send()->content;

        preg_match_all('|<a href="(.*)</a>|isU', $response, $matches);
        foreach ($matches[0] as $item) {
            preg_match('|data-ds-appid="(.*)"|isU', $item, $match);
            if (empty($match[1]))
                continue;

            preg_match('|href="(.*)"|isU', $item, $link);
            $urlElements = explode('/', $link[1]);

            preg_match('|<img src="(.*)"|isU', $item, $image);
            preg_match('|<span class="title">(.*)</span>|isU', $item, $title);

            foreach ($urlElements as $element) {
                if (is_numeric($element))
                $result[] = [
                    'id' => $element,
                    'image' => $image[1],
                    'title' => $title[1],
                ];
            }
        }

        return $result;
    }

    public function actionAddGame()
    {
        \Yii::$app->response->format = \yii\web\Response::FORMAT_JSON;

        return $this->saveGame([
            'id' => \Yii::$app->request->post('id'),
            'title' => \Yii::$app->request->post('title')
        ]);
    }

    protected function saveGame($data)
    {
        $this->_pdo = new \PDO("sqlite:games.db");
        $this->_pdo->query(
            'CREATE TABLE if not exists games
             (
             id INTEGER PRIMARY KEY,
             lastNewTitle TEXT,
             lastNewTime TEXT,
             title TEXT
             )'
        );

        $news = $this->loadGameNews($data['id']);

        if (!empty($news['appnews']['newsitems'][0])) {
            $lastNew = $news['appnews']['newsitems'][0];
        } else {
            $lastNew = [
                'title' => null,
                'date' => null,
            ];
        }

        $STH = $this->_pdo->prepare(
            "INSERT INTO games (id, title, lastNewTitle, lastNewTime) VALUES (:id, :title, :lastNewTitle, :lastNewTime)"
        );
        $STH->execute([
            'id' => $data['id'],
            'title' => $data['title'],
            'lastNewTitle' => $lastNew['title'],
            'lastNewTime' => $lastNew['date']
        ]);

        return $data['id'];
    }

    /**
     * Login action.
     *
     * @return Response|string
     */
    public function actionLogin()
    {
        if (!Yii::$app->user->isGuest) {
            return $this->goHome();
        }

        $model = new LoginForm();
        if ($model->load(Yii::$app->request->post()) && $model->login()) {
            return $this->goBack();
        }

        $model->password = '';
        return $this->render('login', [
            'model' => $model,
        ]);
    }

    /**
     * Logout action.
     *
     * @return Response
     */
    public function actionLogout()
    {
        Yii::$app->user->logout();

        return $this->goHome();
    }

    /**
     * Displays contact page.
     *
     * @return Response|string
     */
    public function actionContact()
    {
        $model = new ContactForm();
        if ($model->load(Yii::$app->request->post()) && $model->contact(Yii::$app->params['adminEmail'])) {
            Yii::$app->session->setFlash('contactFormSubmitted');

            return $this->refresh();
        }
        return $this->render('contact', [
            'model' => $model,
        ]);
    }


}
