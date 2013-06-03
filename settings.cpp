#include "settings.h"
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WDateTime>
#include <Wt/WFileResource>
#include <Wt/Utils>
#include <Wt/WServer>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include "player/html5player.h"
#include "player/wmediaplayerwrapper.h"
#include "private/settings_p.h"

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


vector< string > Settings::mediasDirectories() const
{
  if(d->mediaDirectories.empty()) {
    string mediasDir = string(getenv("HOME")) + "/Videos";
    wApp->readConfigurationProperty("medias-dir", mediasDir);
    d->mediaDirectories = {mediasDir};
  }
  return d->mediaDirectories;
}

void Settings::addMediaDirectory(string directory)
{
  d->mediaDirectories.push_back(directory);
}

void Settings::removeMediaDirectory(string directory)
{
  remove_if(d->mediaDirectories.begin(), d->mediaDirectories.end(), [=](string d) { return d == directory; });
}



string Settings::relativePath(string mediaPath, bool removeTrailingSlash) const
{
  for(string mediaDirectory: mediasDirectories()) {
    if(mediaPath.find(mediaDirectory) == string::npos) continue;
    string relPath = boost::replace_all_copy(mediaPath, mediaDirectory, "");
    if(removeTrailingSlash && relPath[0] == '/') {
      boost::replace_first(relPath, "/", "");
    }
    return relPath;
  }
  return mediaPath;
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
  string mediasDeployDir;
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
  
  // TODO: ripristinare deploy-dir, magari da configurazione
  /*
  if(wApp->readConfigurationProperty("medias-deploy-dir", mediasDeployDir )) {
    string relpath = p.string();
    boost::replace_all(relpath, mediasDirectories(), mediasDeployDir);
    return relpath;
  }
  */

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
  /* TODO: restore
    string filePath = p.string();
    boost::replace_all(filePath, q->mediasDirectories(), "");
    string hexTime = (boost::format("%1$x") %WDateTime::currentDateTime().toTime_t()) .str();
    string token = Utils::hexEncode(Utils::md5(secDownloadSecret + filePath + hexTime));
    string secDownloadUrl = secDownloadPrefix + token + "/" + hexTime + filePath;
    wApp->log("notice") << "****** secDownload: filename= " << filePath;
    wApp->log("notice") << "****** secDownload: url= " << secDownloadUrl;
    return secDownloadUrl;
    */
}

WLink SettingsPrivate::nginxSecLinkFor(string secDownloadPrefix, string secDownloadSecret, filesystem::path p)
{
  /* TODO: restore
    string filePath = p.string();
    boost::replace_all(filePath, q->mediasDirectories(), "");
    long expireTime = WDateTime::currentDateTime().addSecs(20000).toTime_t();
    string token = Utils::base64Encode(Utils::md5( (boost::format("%s%s%d") % secDownloadSecret % filePath % expireTime).str() ), false);
    token = boost::replace_all_copy(token, "=", "");
    token = boost::replace_all_copy(token, "+", "-");
    token = boost::replace_all_copy(token, "/", "_");
    string secDownloadUrl = (boost::format("%s%s?st=%s&e=%d") % secDownloadPrefix % filePath % token % expireTime).str();
    wApp->log("notice") << "****** secDownload: filename= " << filePath;
    wApp->log("notice") << "****** secDownload: url= " << secDownloadUrl;
    return secDownloadUrl;
    */
}

map<Settings::Icons,string> iconsMap {
  {Settings::FolderBig, "%s/icons/filesystem/directory-big.png"},
  {Settings::FolderSmall, "%s/icons/filesystem/directory-small.png"},
  {Settings::VideoFile, "%s/icons/filesystem/video.png"},
  {Settings::AudioFile, "%s/icons/filesystem/audio.png"}
};

string Settings::icon(Settings::Icons icon)
{
  string staticDeployPath{Settings::staticDeployPath()};
  return (boost::format(iconsMap[icon]) % staticDeployPath).str();
}

string Settings::staticDeployPath()
{
  string staticDeployPath{"/static"};
  WServer::instance()->readConfigurationProperty("static_deploy_path", staticDeployPath);
  return staticDeployPath;
}

string Settings::staticPath(const string& relativeUrl)
{
  return staticDeployPath() + relativeUrl;
}


