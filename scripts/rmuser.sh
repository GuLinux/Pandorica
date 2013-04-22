#!/bin/bash

source "$( dirname "$( readlink -f "$0")" )/sql_common.sh"


driver="${2-sqlite}"
email="$1"

if test "x$1" == "x"; then
  echo "Usage: $0 email [driver]"
  echo "Drivers supported: sqlite, psql"
  exit 1
fi

user_id="$( echo "select user_id from auth_info where email = '$email';" | doSql_$driver )"
auth_info_id="$( echo "select id from auth_info where email = '$email';" | doSql_$driver )"
echo "user_id: $user_id"
echo "auth info: $auth_info_id"

# exit

function genSql() {
cat <<EOF
begin transaction;
delete from auth_token where auth_info_id = $auth_info_id;
delete from auth_identity  where auth_info_id = $auth_info_id;
delete from comment where user_id = $user_id;
delete from groups_users where user_id = $user_id;
delete from session_details where session_info_session_id in (
  select session_id from session_info where user_id = $user_id
);

delete from session_info where user_id = $user_id;
delete from auth_info where id = $auth_info_id;
delete from "user" where id = $user_id;

end transaction;

EOF
}

genSql | doSql_$driver
