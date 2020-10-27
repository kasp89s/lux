<!--pk.eyJ1Ijoia2FzcDg5cyIsImEiOiJja2dzM2N2MjcwN3VyMnFtbm80eWxscmxhIn0.80CaxkT948tYlNaRPFRKjQ-->
<link rel="stylesheet" href="https://unpkg.com/leaflet@1.7.1/dist/leaflet.css" />
<script src="https://unpkg.com/leaflet@1.7.1/dist/leaflet.js"></script>
<style>
    .info {
        height: 50px;
        width: 60%;
        margin-left: auto;
        margin-right: auto;
        margin-bottom: 100px;
    }
    #mapid {
        height: 500px;
        width: 60%;
        margin-left: auto;
        margin-right: auto;
    }
</style>
<div class="info">
    IMEI: <?= $device->imei?>
    S/N: <?= $device->serial?>
    BATTERY: <?= $device->battery / 1000 ?>V
    NETWORK: <?= $device->network?>
    TRACE:
    <form method="post">
        <input type="hidden" name="<?=Yii::$app->request->csrfParam; ?>" value="<?=Yii::$app->request->getCsrfToken(); ?>" />
        <select name="limit">
            <option value="10">10 positions</option>
            <option value="50">50 positions</option>
            <option value="100">100 positions</option>
        </select>
        <input type="submit" value="show" />
    </form>
</div>
<div id="mapid"></div>
<script type="text/javascript">

    var osmUrl = 'http://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png',
        osmAttribution = 'Map data &copy; <a href="http://openstreetmap.org">OpenStreetMap</a> contributors, <a href="http://creativecommons.org/licenses/by-sa/2.0/">CC-BY-SA</a>',
        osmLayer = new L.TileLayer(osmUrl, {maxZoom: 18, attribution: osmAttribution});

    var map = new L.Map('mapid');

    map.setView(new L.LatLng(<?= $lastLocation->lat?>, <?= $lastLocation->lon?>), 16);
    map.addLayer(osmLayer);

    var marker = L.marker([<?= $lastLocation->lat?>, <?= $lastLocation->lon?>]).addTo(map);
    marker.bindTooltip("Last time: " + '<?= $lastLocation->time?>').openTooltip();
    // create a red polyline from an array of LatLng points
    var latlngs = [
        <?php foreach ($trace as $location):?>
        [<?= $location->lat?>, <?= $location->lon?>],
        <?php endforeach; ?>
    ];

    var polyline = L.polyline(latlngs, {color: 'red'}).addTo(map);

    // zoom the map to the polyline
    map.fitBounds(polyline.getBounds());
</script>