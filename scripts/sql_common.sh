#!/bin/bash


function doSql_sqlite() {
  sqlite3  "${SQLITE_FILE-build/videostreaming.sqlite}"
}

function doSql_psql() {
  psql -A -t -h localhost -p 5432 videostreaming videostreaming
}

function bool_sqlite() {
  if test "$1" == "true"; then
    echo 1
  else
    echo 0
  fi
}

function bool_psql() {
  echo "$1"
}
