#!/bin/bash

function doSql_sqlite() {
  sqlite3  "${SQLITE_FILE-build/videostreaming.sqlite}"
}

function doSql_psql() {
  psql -A -t -h localhost -p 5432 videostreaming videostreaming
}


function seed() {
	cat <<EOF
INSERT INTO \`group\` (version,group_name,is_admin) VALUES(1,'Admin', 1);
EOF
}


seed | doSql_$1
