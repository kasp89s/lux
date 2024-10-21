FROM php:7.4-fpm-alpine

RUN apk update && apk add bash
RUN apk add --no-cache zip libzip-dev
RUN docker-php-ext-configure zip
RUN docker-php-ext-install zip
RUN docker-php-ext-install pdo pdo_mysql

COPY ./crontasks/crontasks /var/spool/cron/crontabs/root
COPY entrypoint.bash /usr/sbin
RUN chmod a+x /usr/sbin/entrypoint.bash

WORKDIR /var/www/lux

ENTRYPOINT /usr/sbin/entrypoint.bash