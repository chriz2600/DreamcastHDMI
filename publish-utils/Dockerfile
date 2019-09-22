FROM alpine:latest

RUN apk update && apk upgrade && \
    apk add --upgrade --no-cache python py-pip && \
    pip install -U --no-cache-dir jinja2

ADD files/ /

