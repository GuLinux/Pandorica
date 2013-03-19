#!/bin/bash
#set -x

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
    dataDir="$( mediaDataDir "$1" "$2")/subs"
    mkdir -p "$dataDir"
    echo "Extracting subtitles from $file to $dataDir"
    eval "$( ffprobe -loglevel quiet -of flat=sep_char=_ -show_format -show_streams "$file" )"
    for i in `seq 0 $(( $format_nb_streams-1))`; do
        stream_type="$( eval echo \$streams_stream_${i}_codec_type )"
        track_language=$( eval echo \$streams_stream_${i}_tags_language)
        if test "x$stream_type" == "xsubtitle" && ! [ -r "$dataDir/${track_language}.vtt" ] ; then
            ffmpeg -i "$file" -loglevel quiet -map 0:$i -c srt "$dataDir/${track_language}.srt"
            curl -F "subrip_file=@${dataDir}/${track_language}.srt" http://atelier.u-sub.net/srt2vtt/index.php > "$dataDir/${track_language}.vtt"
        fi
    done
}

function createThumbnail() {
    file="$1"
    dataDir="$( mediaDataDir "$1" "$2")"
    if [ -r "$dataDir/preview.png" ]; then return; fi
    echo "Creating thumbnail for $file to $dataDir"
    eval "$( ffprobe -loglevel quiet -of flat=sep_char=_ -show_format "$file" )"
    if test "x$format_duration" == "xN/A"; then return; fi
    one_third="$(( $(echo $format_duration | cut -d. -f1) / 3 ))"
    mkdir -p "$dataDir"
    ffmpeg -i "$file" -loglevel quiet -ss $one_third -f image2 -vframes 1 "$dataDir/preview.png"
}

function genFileData() {
    fname="$1"; shift
    medias_dir="$1"; shift
    case "$fname" in
        *.mp4|*.m4v)
        extractSubtitles "$fname" "$medias_dir"
        createThumbnail "$fname" "$medias_dir"
        ;;
        *)
#	echo "Unknown file type: $fname"
	return
        ;;
    esac
}

if test "x$1" == "xfunct"; then
    shift
    functName="$1"; shift
    $functName "$@"
    exit 0
fi
if test "x$1" == "xgenFileData"; then
    genFileData "$1" "$2"
    exit 0
fi
MEDIAS_DIR="$1"; shift
DATA_DIR="${2-$MEDIAS_DIR/.media_data}"
DATA_DIR="$( readlink -f "$DATA_DIR")"

echo "Main data output directory: $DATA_DIR"

#find -L "$MEDIAS_DIR" -type f -exec "$0" genFileData {} "$DATA_DIR" \;
find -L "$MEDIAS_DIR" -type f | while read fname; do genFileData "$fname" "$DATA_DIR"; done

