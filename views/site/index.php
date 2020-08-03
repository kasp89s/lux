<?php
$header = 'https://cdn.cloudflare.steamstatic.com/steam/apps/::id::/header.jpg?t=';
?>
<main role="main">
    <div class="album py-5 bg-light">
        <div class="container">
<div class="row">
    <?php if (!empty($games)):?>
        <?php foreach ($games as $game):?>
        <div class="col-md-4">
            <div class="card mb-4 shadow-sm">
                <img src="<?php echo str_replace('::id::', $game->id, $header)?>" />
                <div class="card-body">
                    <p class="card-text"><b><?php echo $game->title ?></b></p>
                    <div class="d-flex justify-content-between align-items-center">
                        <div class="btn-group">
                            <a href="/site/game/<?= $game->id?>" type="button" class="btn btn-sm btn-outline-secondary">Перейти</a>
                        </div>
                    </div>
                </div>
            </div>
        </div>
        <?php endforeach; ?>
    <?php endif; ?>
</div>
        </div>
    </div>

</main>