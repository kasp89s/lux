<?php

namespace app\models;

use Yii;

/**
 * This is the model class for table "gsmLocation".
 *
 * @property int $id
 * @property int $source
 * @property int|null $mmc
 * @property int|null $mnc
 * @property int|null $lac
 * @property int|null $cellid
 * @property string|null $ip
 * @property string|null $time
 *
 * @property Device $source0
 */
class GsmLocation extends \yii\db\ActiveRecord
{
    /**
     * {@inheritdoc}
     */
    public static function tableName()
    {
        return 'gsmLocation';
    }

    /**
     * {@inheritdoc}
     */
    public function rules()
    {
        return [
            [['source'], 'required'],
            [['source', 'mmc', 'mnc', 'lac', 'cellid'], 'integer'],
            [['time'], 'safe'],
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
            'mmc' => Yii::t('app', 'Mmc'),
            'mnc' => Yii::t('app', 'Mnc'),
            'lac' => Yii::t('app', 'Lac'),
            'cellid' => Yii::t('app', 'Cellid'),
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
