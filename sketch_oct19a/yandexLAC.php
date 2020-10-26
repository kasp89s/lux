	$yandexKey = '6fee90b9-91a6-449e-8edb-fa562afe7d99-';
	$mmc = substr($_GET['s'], 0, 3);
	$mnc = substr($_GET['s'], 3, 2);
	$lac = hexdec(substr($_GET['s'], 5, 4));
	$cellid = hexdec(substr($_GET['s'], 9, 4));
	//hexdec
	/**
	{
    "token": "5cb6261ed31763",
    "radio": "gsm",
    "mcc": 310,
    "mnc": 404,
    "cells": [{
        "lac": 7033,
        "cid": 17811
    }],
    "wifi": [{
        "bssid": "00:17:c5:cd:ca:aa",
        "channel": 11,
        "frequency": 2412,
        "signal": -51
    }, {
        "bssid": "d8:97:ba:c2:f0:5a"
    }],
    "address": 1
}
	*/
	$json = [
		'token' => 'pk.2d74ddefd0dfee66969be3acd0fdd39a',
		'radio' => 'gsm',
		'mcc' => $mmc,
		'mnc' => $mnc,
		'cells' => [
			'lac' => $lac,
			'cid' => $cellid,
		],
		'address' => '1'
	];
	/**
	$xmlData = [
		'common' => [
			'version' => '1.0',
			'api_key' => $yandexKey,
		],
		'gsm_cells' => [
			'cell' => [
				'countrycode' => $mmc,
				'operatorid' => $mnc,
				'cellid' => $cellid,
				'lac' => $lac,
				'signal_strength' => $_GET['n']
			]
		],
		'ip' => [
			'address_v4' => $_SERVER['REMOTE_ADDR']
		]
	];
	
	function array_to_xml( $data, &$xml_data ) {
		foreach( $data as $key => $value ) {
			if( is_array($value) ) {
				if( is_numeric($key) ){
					$key = 'item'.$key; //dealing with <0/>..<n/> issues
				}
				$subnode = $xml_data->addChild($key);
				array_to_xml($value, $subnode);
			} else {
				$xml_data->addChild("$key",htmlspecialchars("$value"));
			}
		 }
	}

	$xml = new SimpleXMLElement('<ya_lbs_request></ya_lbs_request>');
	array_to_xml($xmlData,$xml);
	
	print $xml->asXML();
*/
	// URL API 
	$url = 'https://eu1.unwiredlabs.com/v2/process.php'; 
	// Создаем новый ресурс cURL 
	$ch = curl_init($url); 
	// Присоединить закодированную строку JSON к полям POST 
	//curl_setopt($ch , CURLOPT_POSTFIELDS, 'xml=' . $xml->asXML()); 
	curl_setopt($ch , CURLOPT_POSTFIELDS, json_encode($json)); 
	// Установите тип содержимого на application / json 
	curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']); 
	// Вернуть ответ вместо вывода 
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, true); 
	// Выполняем запрос POST 
	$result = curl_exec($ch); 
	// Закрыть ресурс cURL 
	curl_close($ch);

	$fp = fopen(date('Y-m-d') . 'file.txt', 'a');
	fwrite($fp, var_export($result, true) . PHP_EOL);
	fclose($fp);
