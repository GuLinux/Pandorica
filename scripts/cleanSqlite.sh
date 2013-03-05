#!/bin/bash
test "x$1" != "x-k" && rm build/videostreaming.sqlite
read -p "Premere invio per prepopolare il database"
echo "insert into authorized_users(version,email,role) VALUES(1,'marco.gulino@gmail.com',1);" | sqlite3 build/videostreaming.sqlite
