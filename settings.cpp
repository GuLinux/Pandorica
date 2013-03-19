#include "settings.h"
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WDateTime>
#include <Wt/WFileResource>
#include <Wt/Utils>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include "player/html5player.h"
#include "player/wmediaplayerwrapper.h"

using namespace std;
using namespace Wt;
using namespace boost;

namespace fs=boost::filesystem;

class SettingsPrivate {
public:
  SettingsPrivate(Settings *q) : q(q) {}
    WLink lightySecDownloadLinkFor(string secDownloadPrefix, string secDownloadSecret, filesystem::path p);
    WLink nginxSecLinkFor(string secDownloadPrefix, string secDownloadSecret, filesystem::path p);
  Settings *q;
};

Settings::Settings() : d(new SettingsPrivate(this)) {}
Settings::~Settings() { delete d; }

string Settings::videosDir() const
{
  string videosDir = string(getenv("HOME")) + "/Videos";
  wApp->readConfigurationProperty("videos-dir", videosDir);
  return videosDir;
}

string Settings::value(string cookieName, string defaultValue)
{
  const string *value = wApp->environment().getCookieValue(cookieName);
  if(!value)
    return defaultValue;
  return *value;
}

Player* Settings::newPlayer()
{
  string playerSetting = value("player", "html5");
  if(playerSetting == "jPlayer")
    return new WMediaPlayerWrapper();
  return new HTML5Player();
}


WLink Settings::linkFor(filesystem::path p)
{
  string videosDeployDir;
  string secDownloadPrefix;
  string secDownloadSecret;
  
  if(wApp->readConfigurationProperty("seclink-prefix", secDownloadPrefix) && wApp->readConfigurationProperty("seclink-secret", secDownloadSecret)) {
    return d->nginxSecLinkFor(secDownloadPrefix, secDownloadSecret, p);
  }

  if(wApp->readConfigurationProperty("secdownload-prefix", secDownloadPrefix) &&
    wApp->readConfigurationProperty("secdownload-secret", secDownloadSecret)) {
    return d->lightySecDownloadLinkFor(secDownloadPrefix, secDownloadSecret, p);
  }
  
  
  if(wApp->readConfigurationProperty("videos-deploy-dir", videosDeployDir )) {
    string relpath = p.string();
    boost::replace_all(relpath, videosDir(), videosDeployDir);
    return WLink(relpath);
  }

   WLink link = WLink(new WFileResource(p.string(), wApp));
   wApp->log("notice") << "Generated url: " << link.url();
   return link;
}

WLink SettingsPrivate::lightySecDownloadLinkFor(string secDownloadPrefix, string secDownloadSecret, filesystem::path p)
{
    string filePath = p.string();
    boost::replace_all(filePath, q->videosDir(), "");
    string hexTime = (boost::format("%1$x") %WDateTime::currentDateTime().toTime_t()) .str();
    string token = Utils::hexEncode(Utils::md5(secDownloadSecret + filePath + hexTime));
    string secDownloadUrl = secDownloadPrefix + token + "/" + hexTime + filePath;
    wApp->log("notice") << "****** secDownload: filename= " << filePath;
    wApp->log("notice") << "****** secDownload: url= " << secDownloadUrl;
    return WLink(secDownloadUrl);
}

WLink SettingsPrivate::nginxSecLinkFor(string secDownloadPrefix, string secDownloadSecret, filesystem::path p)
{
    string filePath = p.string();
    boost::replace_all(filePath, q->videosDir(), "");
    long expireTime = WDateTime::currentDateTime().addSecs(20000).toTime_t();
    string token = Utils::base64Encode(Utils::md5( (boost::format("%s%s%d") % secDownloadSecret % filePath % expireTime).str() ), false);
    token = boost::replace_all_copy(token, "=", "");
    token = boost::replace_all_copy(token, "+", "-");
    token = boost::replace_all_copy(token, "/", "_");
    string secDownloadUrl = (boost::format("%s%s?st=%s&e=%d") % secDownloadPrefix % filePath % token % expireTime).str();
    wApp->log("notice") << "****** secDownload: filename= " << filePath;
    wApp->log("notice") << "****** secDownload: url= " << secDownloadUrl;
    return WLink(secDownloadUrl);
}

