<?php

namespace app\models;

use Yii;

/**
 * This is the model class for table "location".
 *
 * @property int $id
 * @property int $source
 * @property string|null $lat
 * @property string|null $lon
 * @property string|null $ip
 * @property string|null $time
 *
 * @property Device $source0
 */
class Location extends \yii\db\ActiveRecord
{
    /**
     * {@inheritdoc}
     */
    public static function tableName()
    {
        return 'location';
    }

    /**
     * {@inheritdoc}
     */
    public function rules()
    {
        return [
            [['source'], 'required'],
            [['source'], 'integer'],
            [['time'], 'safe'],
            [['lat', 'lon'], 'string', 'max' => 16],
            [['ip'], 'string', 'max' => 255],
            [['source'], 'exist', 'skipOnError' => true, 'targetClass' => Device::className(), 'targetAttribute' => ['source' => 'imei']],
        ];
    }

    /**
     * {@inheritdoc}
     */
    public function attributeLabels()
    {
        return [
            'id' => Yii::t('app', 'ID'),
            'source' => Yii::t('app', 'Source'),
            'lat' => Yii::t('app', 'Lat'),
            'lon' => Yii::t('app', 'Lon'),
            'ip' => Yii::t('app', 'Ip'),
            'time' => Yii::t('app', 'Time'),
        ];
    }

    /**
     * Gets query for [[Source0]].
     *
     * @return \yii\db\ActiveQuery
     */
    public function getSource0()
    {
        return $this->hasOne(Device::className(), ['imei' => 'source']);
    }
}
