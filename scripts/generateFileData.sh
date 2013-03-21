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

function saveMediaInfo() {
    file="$1"
    dataDir="$( mediaDataDir "$1" "$2")"
    mkdir -p "$dataDir"
    outfile="$dataDir/media.info"
    ffprobe -loglevel quiet -print_format json            -show_format -show_streams "$file" > "$outfile.json"
    ffprobe -loglevel quiet -print_format flat=sep_char=_ -show_format -show_streams "$file" > "$outfile"
    echo "$outfile"
}


function extractSubtitles() {
    file="$1"
    dataDir="$( mediaDataDir "$1" "$2")/subs"
    mkdir -p "$dataDir"
    echo "Extracting subtitles from $file to $dataDir"
    eval "$( cat "$3" )"
    for i in `seq 0 $(( $format_nb_streams-1))`; do
        stream_type="$( eval echo \$streams_stream_${i}_codec_type )"
        track_language="$( eval echo \$streams_stream_${i}_tags_language)"
	track_title="$( eval echo \$streams_stream_${i}_tags_title)"
	sub_filename="${track_language}.${track_title}"
        if test "x$stream_type" == "xsubtitle" && ! [ -r "$dataDir/${sub_filename}.vtt" ] ; then
            ffmpeg -loglevel quiet -y -i "$file" -map 0:$i -c srt "$dataDir/${sub_filename}.srt" 2>/dev/null
            curl -F "subrip_file=@${dataDir}/${sub_filename}.srt" http://atelier.u-sub.net/srt2vtt/index.php > "$dataDir/${sub_filename}.vtt"
        fi
    done
}

function createThumbnail() {
    file="$1"
    dataDir="$( mediaDataDir "$1" "$2")"
    if [ -r "$dataDir/preview.png" ]; then return; fi
    echo "Creating thumbnail for $file to $dataDir"
    eval "$( cat "$3")"
    if test "x$format_duration" == "xN/A" || test "x$format_duration" == "x"; then return; fi
    format_duration=$(echo $format_duration | cut -d. -f1)
    random_offset="$(( $RANDOM % $(( $format_duration / 30  ))  ))"
    if test "x$(( $RANDOM %2 ))" == "x0"; then
	random_offset="$(( $random_offset * -1 ))"
    fi
    mkdir -p "$dataDir"

    one_third="$(( $format_duration / 3 + $random_offset ))"
    echo "Random total duration: $format_duration, random offset: $random_offset, one_third: $one_third"

    set -x
    ffmpegthumbnailer -i "$file" -o "$dataDir/preview.png" -s 0 -t $one_third -f
    set +x
    if ! [ -r "$dataDir/preview.png" ]; then
	    cd "$dataDir"
	    mplayer --quiet "$( readlink -f "$file" )" -vo png:z=9 -ao null -frames 1 -ss "$one_third" 2>&1 > /dev/null
	    mv 00000001.png preview.png
	    rm 00000*.png
	    cd -
    fi
    if ! [ -r "$dataDir/preview.png" ]; then
	set -x
        ffmpeg -i "$( readlink -f "$file")" -loglevel error -ss $one_third -f image2 -vframes 1 "$dataDir/preview.png"
	set +x
    fi
    convert "$dataDir/preview.png" -scale 640 "$dataDir/preview_player.png"
    convert "$dataDir/preview.png" -scale 260 "$dataDir/preview_thumb.png"
}

function genFileData() {
    fname="$1"; shift
    medias_dir="$1"; shift
    case "$fname" in
        *.mp4|*.m4v)
        mediaInfo="$( saveMediaInfo "$fname" "$medias_dir" )"
        extractSubtitles "$fname" "$medias_dir" "$mediaInfo"
        createThumbnail "$fname" "$medias_dir" "$mediaInfo"
        ;;
        *)
#	echo "Unknown file type: $fname"
        ;;
    esac
    exit 0
}

if test "x$1" == "xfunct"; then
    shift
    functName="$1"; shift
    $functName "$@"
    exit 0
fi
if test "x$1" == "xgenFileData"; then
    genFileData "$2" "$3"
    exit 0
fi
MEDIAS_DIR="$1"; shift
DATA_DIR="${2-$MEDIAS_DIR/.media_data}"
DATA_DIR="$( readlink -f "$DATA_DIR")"

echo "Main data output directory: $DATA_DIR"

find -L "$MEDIAS_DIR" -type f -exec "$0" genFileData {} "$DATA_DIR" \;
#find -L "$MEDIAS_DIR" -type f | while read fname; do genFileData "$fname" "$DATA_DIR"; done

