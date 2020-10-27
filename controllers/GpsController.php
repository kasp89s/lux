<?php

namespace app\controllers;

use app\models\Device;
use app\models\Location;
use Yii;
use yii\filters\AccessControl;
use yii\httpclient\Client;
use yii\web\Controller;
use yii\web\Response;
use yii\filters\VerbFilter;
use app\models\LoginForm;
use app\models\ContactForm;

class GpsController extends Controller
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
        $serial = \Yii::$app->request->post('serial', false);
        if (!empty($serial)) {
            $device = Device::find()->where(['serial' => $serial])->one();

            if (!empty($device->imei))
                return $this->redirect(['gps/device/' . $serial]);
        }

        return $this->render('index', []);
    }

    /**
     * @param $name
     * @return string
     */
    public function actionDevice($name)
    {
        $limit = \Yii::$app->request->post('limit', 10);
        $device = Device::find()->where(['serial' => $name])->one();

        $lastLocation = Location::find()->where(['source' => $device->imei])->orderBy('id desc')->one();
        $trace = Location::find()->where(['source' => $device->imei])->limit($limit)->orderBy('id desc')->all();
//        var_dump($device->locations);
        return $this->render('device', compact('device', 'lastLocation', 'trace'));
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
