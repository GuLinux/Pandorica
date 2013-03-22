#!/bin/bash
#set -x

WORKDIR="/tmp/$( basename "$0").work"
INFO_FILE="$WORKDIR/media_info.flat"
SQLFILE="$WORKDIR/insert.sql"

truncate -s 0 "$SQLFILE"

function idFor() {
    echo -n "$@" | md5sum | cut -f1 -d' '
}

function saveMediaInfo() {
    ffprobe -loglevel quiet -print_format flat=sep_char=_ -show_format -show_streams "$1" > "$INFO_FILE"
    has_media_info="$( echo "select count(*) from media_properties WHERE media_id='$(idFor "$filename")';" | doSql_$2 )"
    if test $has_media_info -gt 0; then
      echo "media_properties already existing; skipping"
      return
    fi
    eval "$( cat "$INFO_FILE")"
    format_duration=$(echo $format_duration | cut -d. -f1)
    echo "INSERT INTO media_properties(media_id, title, filename, duration, size, width, height)
    VALUES('$(idFor "$1")', 'todo', '$format_filename', $format_duration, $format_size, $streams_stream_0_width, $streams_stream_0_height);" | doSql_$2
}

function file_to_hex() {
  od -A n -t x1 "$1"|tr -d '\r\n\t '
}

function file_to_sqlite() {
  echo "X'$(file_to_hex "$1" )'"
}

function file_to_psql() {
  echo "E'\\\\x$(file_to_hex "$1" )'"
}

function doSql_sqlite() {
  sqlite3  "${SQLITE_FILE-build/videostreaming.sqlite}"
}

function extractSubtitles() {
    file="$1"
    driver="$2"
    
    has_subtitles="$( echo "select count(*) from media_attachment WHERE type='subtitles' AND media_id='$(idFor "$file")';" | doSql_$driver )"
    if test $has_subtitles -gt 0; then
      echo "subtitles already existing; skipping"
      return
    fi
    
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
        echo "INSERT INTO media_attachment(version,media_id,type,name,value,mimetype,data) VALUES
        (1,'$(idFor "$1")', 'subtitles', '$track_title','$track_language', 'text/vtt', $(file_to_$driver "${sub_filename}.vtt") );"  | doSql_$driver
      fi
    done
}

function createThumbnail() {
    file="$1"
    driver="$2"
    has_preview="$( echo "select count(*) from media_attachment WHERE type='preview' AND media_id='$(idFor "$filename")';" | doSql_$driver )"
    if test $has_subtitles -gt 0; then
      echo "previews already existing; skipping"
      return
    fi
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
    echo "INSERT INTO media_attachment(version,media_id,type,name,mimetype,value,data) VALUES
    (1,'$(idFor "$1")', 'preview', 'full', 'image/png', '', $(file_to_$driver "$WORKDIR/preview.png") );" | doSql_$driver 
    echo "INSERT INTO media_attachment(version,media_id,type,name,mimetype,value,data) VALUES
    (1,'$(idFor "$1")', 'preview', 'player', 'image/png', '', $(file_to_$driver "$WORKDIR/preview_player.png") );" | doSql_$driver 
    echo "INSERT INTO media_attachment(version,media_id,type,name,mimetype,value,data) VALUES
    (1,'$(idFor "$1")', 'preview', 'thumbnail', 'image/png', '', $(file_to_$driver "$WORKDIR/preview_thumb.png") );" | doSql_$driver 
}

filename="$1"
if ! [ -r "$filename" ]; then
  echo "Usage: $0 filename sql_driver"
  exit 1
fi

rm -rf "$WORKDIR"
mkdir -p "$WORKDIR"
sqlDriver="${2-sqlite}"

saveMediaInfo "$filename" "$sqlDriver"
extractSubtitles "$filename" "$sqlDriver"
createThumbnail "$filename" "$sqlDriver"

#cat "$SQLFILE" | doSql_$sqlDriver

