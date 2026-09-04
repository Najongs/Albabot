#!/bin/bash

filename=$AD1_MAP_NAME
str_blank="";

if [ $filename = "$str_blank" ]
then
    echo "맵파일명이 없습니다."
    $filename="$HOME/default.yaml"
fi

echo $filename

