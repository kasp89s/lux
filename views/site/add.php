<script type="text/javascript">
    $( document ).ready(function() {
        var getResults = function (request) {
            $.post('/site/result-list', {request: request, '_csrf': $('meta[name=csrf-token]').attr('content')}, function (response) {
                $('.games-result-list').empty();
                for (var key in response) {
                    $('.games-result-list').append('<div class="media text-muted pt-3">' +
                        '<img src="' + response[key].image + '" style="margin: 0 20px 0 0">'+
                        '<p class="media-body pb-3 mb-0 small lh-125 border-bottom border-gray">'+
                        '<strong class="d-block text-gray-dark">' + response[key].title + '</strong>'+
                        '<a href="javascript:void(0)" class="add-game-link" data-id="' + response[key].id + '" data-title="' + response[key].title + '">Добавить</a>'+
                        '</p>'+
                        '</div>');
                }
                $('.games-result-list').show();
            });
        }
        $('.game-search').on('input', function () {
            $.post('/site/get-list', {request: this.value, '_csrf': $('meta[name=csrf-token]').attr('content')}, function (response) {
                $('.game-name-list').empty();
                if (typeof response[0] === 'string') {
                    for (var key in response) {
                        $('.game-name-list').append('<a class="dropdown-item" href="javascript:void(0)">' + response[key] + '</a>');
                    }
                    $('.dropdown-menu').dropdown('show');
                } else {
                    $('.dropdown-menu').dropdown('hide');
                }
            });
        });
        $('.game-name-list').on('click', '.dropdown-item', function () {

            var text = this.text;
            setTimeout(function () {
                $('.game-search').val(text);
                getResults(text);
                $('.dropdown-menu').dropdown('hide');
            }, 100)

        });
        $('.games-result-list').on('click', '.add-game-link', function () {
            $.post('/site/add-game', {
                id: $(this).data('id'),
                title: $(this).data('title'),
                '_csrf': $('meta[name=csrf-token]').attr('content')
            }, function (response) {
                location.href = '/site/game/' + response;
            });
        });
    });
</script>

<div class="container">
    <main role="main" class="container">
        <div class="d-flex align-items-center p-3 my-3 text-white-50 bg-purple rounded shadow-sm">
            <div class="dropdown mr-1">
                <input type="text" class="form-control game-search" placeholder="Название игры" aria-describedby="basic-addon1">
                <div class="dropdown-menu game-name-list" aria-labelledby="dropdownMenuOffset">
                </div>
            </div>
        </div>

        <div class="my-3 p-3 bg-white rounded shadow-sm games-result-list" style="display: none">

        </div>
    </main>
</div>