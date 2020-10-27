<?php

namespace app\models;

use Yii;

/**
 * This is the model class for table "device".
 *
 * @property int $imei Имей
 * @property string $serial Серийный номер
 * @property int|null $battery Заряд батареи
 * @property int|null $network Уровень сети
 * @property string|null $time
 *
 * @property GsmLocation[] $gsmLocations
 * @property Location[] $locations
 */
class Device extends \yii\db\ActiveRecord
{
    /**
     * {@inheritdoc}
     */
    public static function tableName()
    {
        return 'device';
    }

    /**
     * {@inheritdoc}
     */
    public function rules()
    {
        return [
            [['imei', 'serial'], 'required'],
            [['imei', 'battery', 'network'], 'integer'],
            [['time'], 'safe'],
            [['serial'], 'string', 'max' => 255],
            [['imei'], 'unique'],
        ];
    }

    /**
     * {@inheritdoc}
     */
    public function attributeLabels()
    {
        return [
            'imei' => Yii::t('app', 'Imei'),
            'serial' => Yii::t('app', 'Serial'),
            'battery' => Yii::t('app', 'Battery'),
            'network' => Yii::t('app', 'Network'),
            'time' => Yii::t('app', 'Time'),
        ];
    }

    /**
     * Gets query for [[GsmLocations]].
     *
     * @return \yii\db\ActiveQuery
     */
    public function getGsmLocations()
    {
        return $this->hasMany(GsmLocation::class, ['source' => 'imei']);
    }

    /**
     * Gets query for [[Locations]].
     *
     * @return \yii\db\ActiveQuery
     */
    public function getLocations()
    {
        return $this->hasMany(Location::class, ['source' => 'imei']);
    }
}
