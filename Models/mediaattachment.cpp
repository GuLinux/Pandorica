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
  {"text/vtt", [](string) {return "vtt"; }},
  {"text/plain", [](string type) {
    if(type ==  "subtitles")
      return "srt";
    return "txt";
  }},
};

Wt::WLink MediaAttachment::link(Dbo::ptr< MediaAttachment > myPtr, Dbo::Transaction &transaction, WObject* parent, bool useCacheIfAvailable) const
{
  auto fallback = [=] { return new WMemoryResource{mimetype(), _data, parent}; };
  if(!useCacheIfAvailable || ! Setting::value(Setting::useCache(), transaction, false) )
    return fallback();
  string cacheDir = Setting::value<string>(Setting::cacheDirectory(), transaction);
  string thumbnailsCacheServerMap = Setting::value<string>(Setting::cacheDeployPath(), transaction);
  
  if(cacheDir.empty() || !fs::is_directory(cacheDir))
    return {fallback()};
  auto createPath = [=](string prefix) {
    string extension = "bin";
    if(extensions.count(mimetype())>0) {
      extension = extensions[mimetype()](type()); 
    }
    return (boost::format("%s/%s/%s_%d_%s_%s.%s")
      % prefix
      % type()
      % mediaId()
      % myPtr.id()
      % name()
      % value()
      % extension
    ).str();
  };
  
  fs::path cacheFile{createPath(cacheDir)};
  
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
