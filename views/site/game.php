<?php
$header = 'https://cdn.cloudflare.steamstatic.com/steam/apps/::id::/header.jpg?t=';
?>
<div class="container">
    <div class="row mb-2">
        <div class="col-md-12">
            <div class="row no-gutters border rounded overflow-hidden flex-md-row mb-4 shadow-sm h-md-250 position-relative">
                <div class="col p-4 d-flex flex-column position-static">
                    <h3 class="mb-0"><?php echo $game->title?></h3>
                </div>
                <div class="col-auto d-none d-lg-block">
                    <img src="<?php echo str_replace('::id::', $game->id, $header)?>" />
                </div>
            </div>
        </div>

    </div>
</div>
<main role="main" class="container">
    <div class="row">
        <div class="col-md-8 blog-main">
            <?php foreach ($news as $new):?>
            <div class="blog-post">
                <h2 class="blog-post-title"><?= $new['title']?></h2>
                <p class="blog-post-meta">
                    <?= \Yii::$app->formatter->asDate($new['date']) ?> by
                    <a href="javascript:void(0)"><?php echo $new['feedname']?></a>
                </p>

                <?php echo $new['contents']?>
            </div><!-- /.blog-post -->
            <?php endforeach;?>
        </div><!-- /.blog-main -->

        <aside class="col-md-4 blog-sidebar">
        </aside><!-- /.blog-sidebar -->

    </div><!-- /.row -->

</main>