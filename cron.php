<?php
defined('YII_DEBUG') or define('YII_DEBUG', true);
defined('YII_ENV') or define('YII_ENV', 'dev');

require __DIR__ . '/vendor/autoload.php';
require __DIR__ . '/vendor/yiisoft/yii2/Yii.php';

$config = require __DIR__ . '/config/console.php';

$_SERVER['argv'] = [
    0 => 'yii',
    1 => 'olx/inform'
];

$application = new yii\console\Application($config);
$exitCode = $application->run();
exit($exitCode);