#ifndef MIGRATIONS_H
#define MIGRATIONS_H
#include "Wt-Commons/dbmigrationmanager.h"
#include "migrations/create_migrations_table.h"
namespace Migrations
{
  WtCommons::MigrationsList migrations
  {
    createMigrationTable,
  };
}
#endif
