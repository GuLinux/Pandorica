#!/bin/bash

function extractSubtitles() {
    file="$1"
    echo "Extracting subtitles from $file"
}

function createThumbnail() {
    file="$1"
    echo "Creating thumbnail for $file"
}

function genFileData() {
    fname="$1"; shift
    case "$fname" in
        *.mp4)
        extractSubtitles "$fname"
        createThumbnail "$fname"
        ;;
        *)
        exit 0
        ;;
    esac
}


if test "x$1" == "xgen-file-data"; then
        shift
        genFileData "$@"
        exit 0
fi

MEDIAS_DIR="$1"; shift

find "$MEDIAS_DIR" -exec "$0" gen-file-data {} \;

