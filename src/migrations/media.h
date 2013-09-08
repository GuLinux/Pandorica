#ifndef MIGRATIONS_ADDCREATIONTIMECOLUMNTOMEDIAINFO_TABLE_H
#define MIGRATIONS_ADDCREATIONTIMECOLUMNTOMEDIAINFO_TABLE_H
#include "Wt-Commons/migratedbo.h"
#include <boost/filesystem.hpp>

namespace Migrations
{
  namespace Media
  {
    typedef boost::tuple<std::string,std::string> MediaPair;
    CREATE_MIGRATION( addCreationTimeColumnToMediaInfo, "2013-09-08 14:11:37",
      {
        m.addColumn<boost::posix_time::ptime>("media_properties", "creation_time");
        Wt::Dbo::collection<MediaPair> allFiles = t.session().query<MediaPair>("select media_id, filename from media_properties;");
        for(auto file: allFiles) {
          boost::filesystem::path filePath(boost::get<1>(file));
          if(!boost::filesystem::exists(filePath))
            continue;
          boost::posix_time::ptime lastWrite = boost::posix_time::from_time_t(boost::filesystem::last_write_time(filePath));
          t.session().execute("update \"media_properties\" set \"creation_time\" = ? where \"media_id\" = ?").bind(lastWrite).bind(boost::get<0>(file));
        }
      }
    )

    // migrations_placeholder
  }
}
#endif

