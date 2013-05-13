#include "Models/models.h"

#include <Wt/WApplication>
#include <Wt/WMemoryResource>
#include <Wt/WFileResource>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <fstream>

using namespace Wt;
using namespace std;
namespace fs = boost::filesystem;

map<string, string> extensions {
  {"image/png", "png"},
};

Wt::WLink MediaAttachment::link(Dbo::ptr< MediaAttachment > myPtr, WObject* parent) const
{
  auto fallback = [=] { return new WMemoryResource{mimetype(), _data, parent}; };
  string cacheDir;
  string cacheDurationInSeconds{"7200"};
  string thumbnailsCacheServerMap;
  
  wApp->readConfigurationProperty("blobs_cache_dir", cacheDir);
  wApp->readConfigurationProperty("blobs_cache_server_map", thumbnailsCacheServerMap);
  
  if(cacheDir.empty() || !fs::is_directory(cacheDir))
    return {fallback()};
  auto createPath = [=](string prefix) {
    return (boost::format("%s/%s/%s_%d_%s_%s.%s")
      % prefix
      % type()
      % mediaId()
      % myPtr.id()
      % name()
      % value()
      % extensions[mimetype()]
    ).str();
  };
  
  fs::path cacheFile{createPath(cacheDir)};
  log("notice") << "Cache file path: " << cacheFile;
  
  if(!fs::is_directory(cacheFile.parent_path()) && ! fs::create_directory(cacheFile.parent_path()))
    return {fallback()};
  if(! fs::exists(cacheFile)) {
    ofstream myfile (cacheFile.string());
    for(auto c: _data)
      myfile << c;
    myfile.close();
  }
  if(thumbnailsCacheServerMap.empty()) {
    return {new WFileResource(mimetype(), cacheFile.string(), parent)};
  }
  return {createPath(thumbnailsCacheServerMap)};
}
