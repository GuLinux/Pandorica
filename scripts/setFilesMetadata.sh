#!/bin/bash

source "$( dirname "$( readlink -f "$0")" )/sql_common.sh"
set -e

driver="${1-sqlite}"

if test "x$1" == "x"; then
echo "Usage: $0 [driver]"
echo "Drivers supported: sqlite, psql"
exit 1
fi

function getAllMedias() {
  cat <<EOF
  select media_id, filename, title from media_properties where title is not null AND length(title) > 0;
EOF
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

EOF
chmod +x "$new_shell_file"

getAllMedias | doSql_$driver | while read line; do
  media_id="$( echo "$line" | cut -d'|' -f1)"
  filename="$( echo "$line" | cut -d'|' -f2)"
  title="$( echo "$line" | cut -d'|' -f3)"
  extension="${filename##*.}"
  if ! [ -r "$filename" ]; then
    continue
  fi
  filename="$( readlink -f "$filename" )"
  
  echo "Media id=$media_id, extension=$extension,  title=$title"
  case $extension in
    "mp4"|"m4v"|"m4a")
      write_mp4 $media_id "$filename" "$title" >> "$new_shell_file"
      ;;
    *)
      write_ffmpeg $media_id "$filename" "$title" >> "$new_shell_file"
      ;;
  esac
done

"$new_shell_file"
