#ifndef MIGRATIONS_CREATE_MIGRATION_TABLE_H
#define MIGRATIONS_CREATE_MIGRATION_TABLE_H
#include "Wt-Commons/migratedbo.h"

namespace Migrations
{
  CREATE_MIGRATION( createMigrationTable, "2013-09-06 17:10:13",
    {
      m.createTable("wt_migrations")
        .column<int64_t>("migration_id")
        .column<std::string>("migration_name")
        .column<boost::posix_time::ptime>("when_applied")
        .column<boost::posix_time::ptime>("when_created")
        .primaryKey("migration_id");
    }
  )
}
#endif
