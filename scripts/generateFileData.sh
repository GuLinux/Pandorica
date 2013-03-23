#!/bin/bash
#set -x

WORKDIR="/tmp/$( basename "$0").work"
INFO_FILE="$WORKDIR/media_info.flat"

function idFor() {
    echo -n "$@" | md5sum | cut -f1 -d' '
}

function sqlEscape() {
#printf "%q" "$*"
  echo "$@" | sed "s/'/''/g"
}



function saveMediaInfo() {
    ffprobe -loglevel quiet -print_format flat=sep_char=_ -show_format -show_streams "$1" > "$INFO_FILE"
    has_media_info="$( echo "select count(*) from media_properties WHERE media_id='$(idFor "$filename")';" | doSql_$2 )"
    if test $has_media_info -gt 0; then
      test "$quietMode" != "true" && echo "media_properties already existing; skipping"
      return
    fi
    eval "$( cat "$INFO_FILE")"
    if test "$format_duration" == "N/A" || test "$format_duration" == ""; then
      format_duration=-1
    fi
    format_duration=$(echo $format_duration | cut -d. -f1)
    echo "INSERT INTO media_properties(media_id, title, filename, duration, size, width, height)
    VALUES('$(idFor "$1")', 'todo', '$(sqlEscape "$format_filename")', $format_duration, $format_size, ${streams_stream_0_width--1}, ${streams_stream_0_height--1});" | doSql_$2
}

function file_to_hex() {
#od -A n -t x1 "$1"|tr -d '\r\n\t '
  xxd -p "$1" | tr -d '\n'
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

function doSql_psql() {
  sudo -u postgres psql videostreaming -t -A
}

function extractSubtitles() {
    file="$1"
    
    has_subtitles="$( echo "select count(*) from media_attachment WHERE type='subtitles' AND media_id='$(idFor "$file")';" | doSql_$sqlDriver )"
    if test $has_subtitles -gt 0; then
      test "$quietMode" != "true" && echo "subtitles already existing; skipping"
      return
    fi
    SQLFILE="$WORKDIR/tmpSql"
    truncate -s 0 "$SQLFILE"
    echo "BEGIN TRANSACTION;" > "$SQLFILE"
    echo "Extracting subtitles from $file to $WORKDIR"
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
        (1,'$(idFor "$1")', 'subtitles', '$(sqlEscape "$track_title")','$track_language', 'text/vtt', $(file_to_$sqlDriver "${sub_filename}.vtt") );" >> "$SQLFILE"
      fi
    done
    if test "$( cat "$SQLFILE" | wc -l)" == "1"; then return; fi
    echo "END TRANSACTION;" >> "$SQLFILE"
  cat "$SQLFILE" | doSql_$sqlDriver || mv "$SQLFILE" "/tmp/$( basename $0 ).failed.$(date +%s)"
}

function createThumbnail() {
    file="$1"
    has_preview="$( echo "select count(*) from media_attachment WHERE type='preview' AND media_id='$(idFor "$filename")';" | doSql_$sqlDriver )"
  if test $has_preview -gt 0; then
    test "$quietMode" != "true" && echo "previews already existing; skipping"
    return
  fi
  SQLFILE="$WORKDIR/tmpSql"
  truncate -s 0 "$SQLFILE"
  echo "BEGIN TRANSACTION;" > "$SQLFILE"
  eval "$(cat "$INFO_FILE" )"
  
  if test "x$format_duration" == "xN/A" || test "x$format_duration" == "x"; then return; fi
    format_duration=$(echo $format_duration | cut -d. -f1)
    echo "Format duration: $format_duration"
    if test "$format_duration" -lt 20; then
      random_offset="0"
    else
    random_offset="$(( $RANDOM % $(( $format_duration / 15  ))  ))"
  fi
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
  (1,'$(idFor "$1")', 'preview', 'full', 'image/png', '', $(file_to_$sqlDriver "$WORKDIR/preview.png") );" >> "$SQLFILE"
  echo "INSERT INTO media_attachment(version,media_id,type,name,mimetype,value,data) VALUES
  (1,'$(idFor "$1")', 'preview', 'player', 'image/png', '', $(file_to_$sqlDriver "$WORKDIR/preview_player.png") );" >> "$SQLFILE"
  echo "INSERT INTO media_attachment(version,media_id,type,name,mimetype,value,data) VALUES
  (1,'$(idFor "$1")', 'preview', 'thumbnail', 'image/png', '', $(file_to_$sqlDriver "$WORKDIR/preview_thumb.png") );" >> "$SQLFILE"
  echo "END TRANSACTION;" >> "$SQLFILE"
  cat "$SQLFILE" | doSql_$sqlDriver || mv "$SQLFILE" "/tmp/$( basename $0 ).failed.$(date +%s)"
}

function do_Help() {
  echo "Usage: $0 filename [options]"
  echo "Options:"
  echo "-d sqldriver | --driver sqldriver"
  echo "-q | --quiet"
  exit 0
}

filename="$1"; shift
if ! [ -r "$filename" ]; then do_Help; fi

rm -rf "$WORKDIR"
mkdir -p "$WORKDIR"

sqlDriver="sqlite"
quietMode="false"
while test "x$1" != "x"; do
  case "$1" in
    "-d"|"--driver")
    sqlDriver="$2"; shift
    ;;
    "-q"|"--quiet")
    quietMode="true"
    ;;
    *)
    do_Help
    ;;
  esac
  shift
done

test "$quietMode" != "true" && echo "Saving metadata to $sqlDriver db for $filename"
saveMediaInfo "$filename" "$sqlDriver"
extractSubtitles "$filename" "$sqlDriver"
createThumbnail "$filename" "$sqlDriver"

#cat "$SQLFILE" | doSql_$sqlDriver

