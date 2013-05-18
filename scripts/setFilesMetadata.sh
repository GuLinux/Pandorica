#!/bin/bash

source "$( dirname "$( readlink -f "$0")" )/sql_common.sh"
set -e

driver="$1"
filter_folder="$2"
if test "x$filter_folder" != "x"; then
  filter_folder="$( readlink -f "$filter_folder" )"
fi

if test "x$1" == "x"; then
echo "Usage: $0 driver [filter-folder]"
echo "Drivers supported: sqlite, psql"
echo "If filter folder is supplied, only files belonging to that folder will be written"
exit 1
fi

function getMediasCount() {
  echo "select count(media_id) from media_properties where title is not null AND length(title) > 0;"
}

function getAllMedias() {
  echo "select media_id, filename, title from media_properties where title is not null AND length(title) > 0;"
}

function backupFile() {
  filename="$1"
  extension="${filename##*.}"
  in_file="$( dirname "$filename")/__original__$( basename "$filename" .$extension).$extension"
  echo "$in_file"
}

function write_ffmpeg() {
  media_id="$1"
  filename="$2"
  title="$3"
  in_file="$( backupFile "$filename")"
  cat <<EOF
  mv "$filename" "$in_file"
  ffmpeg -i "$in_file" -c copy -y -metadata title="$title" -metadata tool="streaming_mediaid=$media_id" "$filename"
EOF
}

function write_mp4() {
  media_id="$1"
  filename="$2"
  title="$3"
  in_file="$( backupFile "$filename")"
  cat <<EOF
  mv "$filename" "$in_file"
  MP4Box "$in_file" -unhint -itags name="$title":tool=streaming_mediaid=$media_id -out "$filename"
EOF
}

new_shell_file="/tmp/$( basename "$0")_tmp.sh"
cat > "$new_shell_file" <<EOF
#!/bin/bash

MEDIAS_COUNT=__MEDIAS_COUNT_PLACEHOLDER__
EOF
chmod +x "$new_shell_file"

medias_count=0
getAllMedias | doSql_$driver | while read line; do
  media_id="$( echo "$line" | cut -d'|' -f1)"
  filename="$( echo "$line" | cut -d'|' -f2)"
  title="$( echo "$line" | cut -d'|' -f3)"
  extension="${filename##*.}"
  if ! [ -r "$filename" ]; then
    continue
  fi
  filename="$( readlink -f "$filename" )"
  
  
  eval "$( ffprobe "$filename" -of flat=s=_ -loglevel quiet -show_format)"
  saved_media_id="${format_tags_tool##*=}"
  if test "x$filter_folder" != "x" && ! echo "$filename" | grep -q "$filter_folder"; then
    echo "Media not belonging to $filter_folder; skipping"
    continue
  fi
  if test "x$saved_media_id" == "x$media_id" && test "x$title" == "x$format_tags_title"; then
    echo "Media already correctly tagged, skipping"
    continue
    else
      echo "expecting title='$title',  found='$format_tags_title'"
      echo "expecting media_id='$media_id',  found='$saved_media_id'"
  fi
  medias_count=$(( $medias_count + 1 ))
  echo "$medias_count" > "/tmp/$( basename "$0")_medias_count"
  echo "echo \"[$medias_count/\$MEDIAS_COUNT] - \$( echo \"$medias_count * 100 / \$MEDIAS_COUNT\" | bc )%\"" >> "$new_shell_file"

  case $extension in
    "mp4"|"m4v"|"m4a")
      write_mp4 $media_id "$filename" "$title" >> "$new_shell_file"
      ;;
    *)
      write_ffmpeg $media_id "$filename" "$title" >> "$new_shell_file"
      ;;
  esac
done
medias_count=$( cat "/tmp/$( basename "$0")_medias_count"); rm "/tmp/$( basename "$0")_medias_count"
sed -i "s/__MEDIAS_COUNT_PLACEHOLDER__/$medias_count/g" "$new_shell_file"

echo "Writing now"
"$new_shell_file"
