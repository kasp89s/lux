<div class="row">
    <?php foreach ($favorites as $favorite):?>
        <?php
        $favorite = json_decode(urldecode($favorite->data));
        ?>
    <div class="col-md-4">
        <h2><?= $favorite->title?></h2>
        <p><?= $favorite->contents?></p>
        <p><a class="btn btn-secondary" href="<?= $favorite->url?>" role="button">Источник »</a></p>
    </div>
    <?php endforeach; ?>
</div>
