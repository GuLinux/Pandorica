#!/bin/bash
function idFor() {
    echo -n "$@" | md5sum | cut -f1 -d' '
}

function mediaDataDir() {
    fname="$1"
    medias_dir="$2"
    echo "$medias_dir/$( idFor "$fname")"
}

function extractSubtitles() {
    file="$1"
    dataDir="$( mediaDataDir "$1" "$2")"
    echo "Extracting subtitles from $file to $dataDir"
}

function createThumbnail() {
    file="$1"
    dataDir="$( mediaDataDir "$1" "$2")"
    echo "Creating thumbnail for $file to $dataDir"
}

function genFileData() {
    fname="$1"; shift
    medias_dir="$1"; shift
    case "$fname" in
        *.mp4)
        extractSubtitles "$fname" "$medias_dir"
        createThumbnail "$fname" "$medias_dir"
        ;;
        *)
        exit 0
        ;;
    esac
}

if test "x$1" == "xfunct"; then
    shift
    functName="$1"; shift
    $functName $@
    exit 0
fi

MEDIAS_DIR="$1"; shift
DATA_DIR="${2-$MEDIAS_DIR/.media_data}"
DATA_DIR="$( readlink -f "$DATA_DIR")"

find "$MEDIAS_DIR" -exec "$0" funct genFileData {} "$DATA_DIR" \;

