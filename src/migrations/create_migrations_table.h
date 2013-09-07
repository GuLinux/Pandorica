#ifndef MIGRATIONS_CREATE_MIGRATION_TABLE_H
#define MIGRATIONS_CREATE_MIGRATION_TABLE_H
#include "Wt-Commons/migratedbo.h"

namespace Migrations
{
  CREATE_MIGRATION( createMigrationTable, "2013-09-06 17:10:13",
    {
      m.createTable("wt_migrations").column<int64_t>("migration_id").column<boost::posix_time::ptime>("when").column<std::string>("migration_name").primaryKey("migration_id");
    }
  )
  CREATE_MIGRATION( addNameToMigrationTable, "2013-09-07 10:10:13",
    {
      try {
        m.addColumn<std::string>("wt_migrations", "migration_name", {"''"});
      } catch(...) {
      }
    }
  )
  CREATE_MIGRATION( renameWhenColumnToWhenApplied, "2013-09-07 12:10:13",
    {
      m.renameColumn("wt_migrations", "when", "when_applied");
    }
  )
  CREATE_MIGRATION( addMigrationCreationColumn, "2013-09-07 17:10:13",
    {
      m.addColumn<boost::posix_time::ptime>("wt_migrations", "when_created");
    }
  )
}
#endif
