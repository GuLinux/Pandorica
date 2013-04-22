#!/bin/bash

source "$( dirname "$( readlink -f "$0")" )/sql_common.sh"

function seed() {
	cat <<EOF
INSERT INTO "group" (version,group_name,is_admin) VALUES(1,'Admin', $( bool_${1} true));
EOF
}


seed $1 | doSql_$1
