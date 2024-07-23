FROM php:7.4-fpm-alpine

RUN apk update && apk add bash
RUN apk add --no-cache zip libzip-dev
RUN docker-php-ext-configure zip
RUN docker-php-ext-install zip
RUN docker-php-ext-install pdo pdo_mysql

WORKDIR /var/www/lux

CMD php-fpm