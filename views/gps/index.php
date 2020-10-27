<style>
    .input-serial {
        height: 200px;
        width: 300px;
        border: 1px solid #1b1e21;
        margin-left: auto;
        margin-right: auto;
        padding: 30px;
    }
</style>
<div class="input-serial">
    <form method="post">
        <input type="hidden" name="<?=Yii::$app->request->csrfParam; ?>" value="<?=Yii::$app->request->getCsrfToken(); ?>" />
        Device S/N
        <input type="text"  class="form-control" name="serial" autofocus="" aria-required="true" aria-invalid="true">
        <br/>
        <button type="submit" class="btn btn-primary" name="login-button">GO</button>
    </form>
</div>