#include "Models/models.h"

#include <Wt/WApplication>
#include <Wt/WMemoryResource>
#include <Wt/WFileResource>
#include <Wt/WServer>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <functional>

using namespace Wt;
using namespace std;
namespace fs = boost::filesystem;

map<string, function<string(string)>> extensions {
  {"image/png", [](string){ return "png"; }},
  {"image/jpeg", [](string) { return "jpg"; }},
  {"text/plain", [](string type) {
    if(type ==  "subtitles")
      return "srt";
    return "txt";
  }},
};

Wt::WLink MediaAttachment::link(Dbo::ptr< MediaAttachment > myPtr, WObject* parent) const
{
  auto fallback = [=] { return new WMemoryResource{mimetype(), _data, parent}; };
  string cacheDir;
  string thumbnailsCacheServerMap;
  
  log("notice") << "has blobs_cache_dir property: " << WServer::instance()->readConfigurationProperty("blobs_cache_dir", cacheDir);
  log("notice") << "has blobs_cache_server_map property: " << WServer::instance()->readConfigurationProperty("blobs_cache_server_map", thumbnailsCacheServerMap);
  log("notice") << "cacheDir: " << cacheDir;
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
      % extensions[mimetype()](type())
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
