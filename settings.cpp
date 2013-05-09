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
#include "settings_p.h"

using namespace std;
using namespace Wt;
using namespace boost;
using namespace StreamingPrivate;

namespace fs=boost::filesystem;

const std::string Settings::downloadSource = "download_src";
const std::string Settings::mediaAutoplay = "media_autoplay";
const std::string Settings::preferredPlayer = "player";
const std::string Settings::guiLanguage = "gui_language";

map<string,string> defaultValues {
  {Settings::mediaAutoplay, "autoplay_always"},
  {Settings::downloadSource, "lighttpd"},
  {Settings::preferredPlayer, "html5"},
  {Settings::guiLanguage, "<browserdefault>"},
  };

Settings::Settings() : d(new SettingsPrivate(this)) {}
Settings::~Settings() { delete d; }


string Settings::videosDir() const
{
  string videosDir = string(getenv("HOME")) + "/Videos";
  wApp->readConfigurationProperty("videos-dir", videosDir);
  return videosDir;
}

filesystem::path Settings::mediaData() const
{
  string dataPath = videosDir() + "/.media_data";
  wApp->readConfigurationProperty("media_data", dataPath);
  return fs::path(dataPath);
}


string Settings::value(string cookieName)
{
  if(!d->sessionSettings[cookieName].empty())
    return d->sessionSettings[cookieName];
  
  const string *value = wApp->environment().getCookieValue(cookieName);
  if(!value) {
    wApp->log("notice") << "cookie " << cookieName << " not found; returning default";
    return defaultValues[cookieName];
  }
  wApp->log("notice") << "cookie " << cookieName << " found: " << *value;
  return *value;
}

string Settings::locale()
{
  string storedValue = value(guiLanguage);
  if(storedValue == "<browserdefault>") storedValue = wApp->environment().locale();
  return storedValue;
}


void Settings::setValue(string settingName, string value)
{
  wApp->setCookie(settingName, value, WDateTime::currentDateTime().addDays(365));
  d->sessionSettings[settingName] = value;
  if(settingName == guiLanguage) {
    wApp->setLocale(locale());
  }
}


Player* Settings::newPlayer()
{
  string playerSetting = value(Settings::preferredPlayer);
  if(playerSetting == "jplayer")
    return new WMediaPlayerWrapper();
  return new HTML5Player();
}

bool Settings::autoplay(const Media& media)
{
  string autoplay = value(Settings::mediaAutoplay);
  if(autoplay == "autoplay_always")
    return true;
  if(autoplay == "autoplay_audio_only")
    return media.mimetype().find("audio") != string::npos;
  if(autoplay == "autoplay_video_only")
    return media.mimetype().find("video") != string::npos;
  return false;
}



WLink Settings::linkFor(filesystem::path p)
{
  string videosDeployDir;
  string secLinkPrefix;
  string secLinkSecret;
  string secDownloadPrefix;
  string secDownloadSecret;
  
  bool has_lighttpd = (wApp->readConfigurationProperty("secdownload-prefix", secDownloadPrefix)
    && wApp->readConfigurationProperty("secdownload-secret", secDownloadSecret));
  bool has_nginx = (wApp->readConfigurationProperty("seclink-prefix", secLinkPrefix)
    && wApp->readConfigurationProperty("seclink-secret", secLinkSecret));

  if(has_lighttpd) {
    return d->lightySecDownloadLinkFor(secDownloadPrefix, secDownloadSecret, p);
  }
  
  if(has_nginx) {
    return d->nginxSecLinkFor(secLinkPrefix, secLinkSecret, p);
  }
  
  
  if(wApp->readConfigurationProperty("videos-deploy-dir", videosDeployDir )) {
    string relpath = p.string();
    boost::replace_all(relpath, videosDir(), videosDeployDir);
    return relpath;
  }

   WLink link{new WFileResource(p.string(), wApp)};
   wApp->log("notice") << "Generated url: " << link.url();
   return link;
}

WLink Settings::shareLink(string mediaId)
{
  return {wApp->makeAbsoluteUrl(wApp->bookmarkUrl("/") + string("?media=") + mediaId)};
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
    return secDownloadUrl;
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
    return secDownloadUrl;
}

map<Settings::Icons,string> iconsMap {
  {Settings::FolderBig, "http://gulinux.net/css/fs_icons/%s/directory-big.png"},
  {Settings::FolderSmall, "http://gulinux.net/css/fs_icons/%s/directory-small.png"},
  {Settings::VideoFile, "http://gulinux.net/css/fs_icons/%s/video.png"},
  {Settings::AudioFile, "http://gulinux.net/css/fs_icons/%s/audio.png"}
};

string Settings::icon(Settings::Icons icon)
{
  string theme {"humano2"};
  return (boost::format(iconsMap[icon]) % theme).str();
}


