#!/bin/bash
#set -x

WORKDIR="/tmp/$( basename "$0").work"
INFO_FILE="$WORKDIR/media_info.flat"

function idFor() {
    echo -n "$@" | md5sum | cut -f1 -d' '
}

function saveMediaInfo() {
    ffprobe -loglevel quiet -print_format flat=sep_char=_ -show_format -show_streams "$1" > "$INFO_FILE"
}


function extractSubtitles() {
    file="$1"
    echo "Extracting subtitles from $file to $dataDir"
    eval "$(cat "$INFO_FILE")"
    for i in `seq 0 $(( $format_nb_streams-1))`; do
      stream_type="$( eval echo \$streams_stream_${i}_codec_type )"
      track_language="$( eval echo \$streams_stream_${i}_tags_language)"
      track_title="$( eval echo \$streams_stream_${i}_tags_title)"
      sub_filename="$WORKDIR/${i}"
      if test "x$stream_type" == "xsubtitle" && ! [ -r "${sub_filename}.vtt" ] ; then
        ffmpeg -loglevel quiet -y -i "$file" -map 0:$i -c srt "${sub_filename}.srt" 2>/dev/null
        curl -F "subrip_file=@${sub_filename}.srt" http://atelier.u-sub.net/srt2vtt/index.php > "${sub_filename}.vtt"
      fi
    done
}

function createThumbnail() {
    file="$1"
    echo "Creating thumbnail for $file"
    eval "$(cat "$INFO_FILE" )"
    
    if test "x$format_duration" == "xN/A" || test "x$format_duration" == "x"; then return; fi
      format_duration=$(echo $format_duration | cut -d. -f1)
      random_offset="$(( $RANDOM % $(( $format_duration / 15  ))  ))"
    if test "x$(( $RANDOM %2 ))" == "x0"; then
      random_offset="$(( $random_offset * -1 ))"
    fi

    one_third="$(( $format_duration / 4 + $random_offset ))"
    echo "Random total duration: $format_duration, random offset: $random_offset, one_third: $one_third"

    set -x
    ffmpegthumbnailer -i "$file" -o "$WORKDIR/preview.png" -s 0 -t $( printf "%02d:%02d:%02d\n" "$(($one_third/3600%24))" "$(($one_third/60%60))" "$(($one_third%60))" ) -f
    set +x
    if ! [ -r "$WORKDIR/preview.png" ]; then return; fi
    convert "$WORKDIR/preview.png" -scale 640x264 "$WORKDIR/preview_player.png"
    convert "$WORKDIR/preview.png" -scale 260x264 "$WORKDIR/preview_thumb.png"
}

filename="$1"
if ! [ -r "$filename" ]; then
  echo "Usage: $0 filename sql_driver"
  exit 1
fi

rm -rf "$WORKDIR"
mkdir -p "$WORKDIR"

saveMediaInfo "$filename"
extractSubtitles "$filename"
createThumbnail "$filename"
