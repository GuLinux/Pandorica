#include "whtmltemplateslocalizedstrings.h"
#include <boost/filesystem.hpp>
#include <Wt/WApplication>
#include <iostream>
#include <fstream>

using namespace Wt;
using namespace boost;
using namespace std;

namespace fs = boost::filesystem;

WHTMLTemplatesLocalizedStrings::WHTMLTemplatesLocalizedStrings(const string& resourcesDir, WObject* parent): WObject(parent)
{
  fs::path p(resourcesDir);
  for(fs::directory_iterator it(p); it != fs::directory_iterator(); it++) {
      wApp->log("notice") << "found path: " << it->path().string() << ", extension=" << it->path().extension();
      if(fs::is_regular_file(it->path()) && it->path().extension().string() == ".html")
        processHtmlTemplate(it->path());
  }
}

void WHTMLTemplatesLocalizedStrings::processHtmlTemplate(filesystem::path path)
{
  string key = path.filename().replace_extension().string();
  wApp->log("notice") << "processing file: " << path.string();
  wApp->log("notice") << "key: " << key;
  ifstream file(path.string());
  stringstream s;
  s << file.rdbuf();
  translationMap[key] = s.str();
  
  wApp->log("notice") << "key: " << key << " mapped to " << translationMap[key];
}


bool WHTMLTemplatesLocalizedStrings::resolveKey(const std::string& key, std::string& result)
{
  if(translationMap.count(key)>0) {
    result = translationMap[key];
    return true;
  }
  return false;
}

WHTMLTemplatesLocalizedStrings::~WHTMLTemplatesLocalizedStrings()
{

}

