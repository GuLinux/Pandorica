#ifndef MIGRATIONS_CREATE_MIGRATION_TABLE_H
#define MIGRATIONS_CREATE_MIGRATION_TABLE_H
#include "Wt-Commons/dbmigrationmanager.h"
#include <boost/format.hpp>
#include <Wt/Dbo/Transaction>
#include <Wt/Dbo/SqlConnection>
#include <Wt/Dbo/Session>

namespace Migrations
{
  WtCommons::Migrations createMigrationTable
  {
    []( Wt::Dbo::Transaction & t, Wt::Dbo::SqlConnection * c )
    {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
      boost::format query {"\
          create table \"wt_migrations\"(\
            \"migration_id\" %s,\
            \"when\" %s,\
            primary key( \"migration_id\" )\
          )"
      };
      query % Wt::Dbo::sql_value_traits<int64_t>().type( c, -1 )
            % c->dateTimeType( Wt::Dbo::SqlDateTime );
      t.session().execute( query.str() );
    },
  };
}
#endif
