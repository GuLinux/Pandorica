/***********************************************************************
Copyright (c) 2013 "Marco Gulino <marco.gulino@gmail.com>"

This file is part of Pandorica: https://github.com/GuLinux/Pandorica

Pandorica is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (included the COPYING file).

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/


#include "settings.h"
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WDateTime>
#include <Wt/WFileResource>
#include <Wt/Utils>
#include <Wt/WServer>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <Wt/WPanel>
#include "player/html5player.h"
#include "player/wmediaplayerwrapper.h"
#include "private/settings_p.h"
#include "Models/setting.h"
#include "session.h"
#include "utils/d_ptr_implementation.h"
#include "fileresource.h"

using namespace std;
using namespace Wt;

namespace fs=boost::filesystem;

const std::string Settings::mediaAutoplay = "media_autoplay";
const std::string Settings::preferredPlayer = "player";
const std::string Settings::guiLanguage = "gui_language";

map<string,string> defaultValues {
  {Settings::mediaAutoplay, "autoplay_always"},
  {Settings::preferredPlayer, "mediaelementjs"},
  {Settings::guiLanguage, "<browserdefault>"},
  };

Settings::Settings() : d(this) {}
Settings::~Settings() {}

vector< string > Settings::mediasDirectories() {
  return Setting::values<string>(Setting::MediaDirectories);
}

const string PATH_SEP()
{
#ifdef WIN32
  return "\\";
#else
  return "/";
#endif
}


void Settings::addMediaDirectory(string directory)
{
  auto _mediaDirectories = Settings::mediasDirectories();
  _mediaDirectories.push_back(directory);
  Setting::write<string>(Setting::MediaDirectories, _mediaDirectories);
}

void Settings::removeMediaDirectory(string directory)
{
  auto _mediaDirectories = Settings::mediasDirectories();
  
  _mediaDirectories.erase(remove_if(begin(_mediaDirectories), end(_mediaDirectories), [=](string d) { return d == directory; }), end(_mediaDirectories));
  Setting::write<string>(Setting::MediaDirectories, _mediaDirectories);
}

string Settings::sharedFilesDir(std::string append)
{
  return string{SHARED_FILES_DIR} + append;
}


string Settings::relativePath(string mediaPath, Dbo::Session* session, bool removeTrailingSlash) const
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
  return storedValue != defaultValues[guiLanguage] ? storedValue : string();
}


void Settings::setValue(string settingName, string value)
{
  wApp->setCookie(settingName, value, WDateTime::currentDateTime().addDays(365));
  d->sessionSettings[settingName] = value;
  if(settingName == guiLanguage) {
    wApp->setLocale(locale());
  }
}


Player* Settings::newPlayer(const string &mimetype)
{
  string playerSetting = value(Settings::preferredPlayer);
  if(playerSetting == "videojs" && mimetype.find("audio") == 0) {
    playerSetting = "purehtml5";
  }
  if(playerSetting == "jplayer")
    return new WMediaPlayerWrapper();
  if(playerSetting == "purehtml5")
    return new HTML5Player(HTML5Player::PureHTML5);
  if(playerSetting == "videojs")
    return new HTML5Player(HTML5Player::VideoJs);
  if(playerSetting == "mediaelementjs")
    return new HTML5Player(HTML5Player::MediaElementJs);
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



WLink Settings::linkFor(boost::filesystem::path p, const string& mimetype, Dbo::Session* session, WObject *parent)
{
  Dbo::Transaction t(*session);
   WFileResource *resource = new WFileResource(mimetype, p.string(), parent);
   resource->suggestFileName(p.filename().string());
   WLink link{resource};
   wApp->log("notice") << "Generated url: " << link.url();
   return link;
}

WLink Settings::shareLink(string mediaId)
{
  return {wApp->makeAbsoluteUrl(wApp->bookmarkUrl("/") + string("?media=") + mediaId)};
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

string staticFilesDeployPath{"/static"};
string Settings::staticDeployPath()
{
  return staticFilesDeployPath;
}

string Settings::staticPath(const string& relativeUrl)
{
  return staticDeployPath() + relativeUrl;
}

string sqlite3DatabasePath_;
void Settings::init(boost::program_options::variables_map commandLineOptions)
{
  if( boost::any_cast<string>(commandLineOptions["server-mode"].value()) == "managed")
    staticFilesDeployPath = boost::any_cast<string>(commandLineOptions["static-deploy-path"].value()) ;
  sqlite3DatabasePath_ = boost::any_cast<string>(commandLineOptions["sqlite3-databases-path"].value());
}

string Settings::sqlite3DatabasePath(const std::string &databaseName)
{
  auto path = boost::filesystem::path(sqlite3DatabasePath_);
  boost::filesystem::create_directories(path);
  return (path / databaseName).string();
}

bool Settings::emailVerificationMandatory()
{
  return authenticationMode() != NoAuth && Setting::value<bool>(Setting::EmailVerificationMandatory, false);
}


map<Settings::AnimationType, Settings::Animation> Settings::animations {
  {Settings::PanelAnimation, { {WAnimation::SlideInFromTop, WAnimation::EaseOut, 200}, {WAnimation::SlideInFromTop, WAnimation::Linear, 100}  }},
  {Settings::PlaylistAnimation, { {WAnimation::SlideInFromTop | WAnimation::Fade, WAnimation::EaseOut, 600}, {WAnimation::SlideInFromTop, WAnimation::Linear, 100}  }},
  {Settings::ShowMediaInfoAnimation, { {0}, {WAnimation::SlideInFromTop, WAnimation::Linear, 100}  }},
  {Settings::HideMediaInfoAnimation, { {0}, {WAnimation::SlideInFromBottom, WAnimation::Linear, 100}  }},
};

WAnimation Settings::Animation::get()
{
  return (wApp->environment().agentIsIEMobile() || wApp->environment().agentIsMobileWebKit() )
  ? mobile : desktop;
}

void Settings::databaseType(Settings::DatabaseType type)
{
  Setting::write(Setting::DatabaseType, static_cast<int>(type));
}

Settings::DatabaseType Settings::databaseType()
{
  return static_cast<DatabaseType>(Setting::value<int>(Setting::DatabaseType, static_cast<int>(DatabaseType::Sqlite3)));
}

void Settings::authenticationMode(Settings::AuthenticationMode type)
{
  Setting::write(Setting::AuthenticationMode, static_cast<int>(type));
}

Settings::AuthenticationMode Settings::authenticationMode()
{
  if(pandoricaMode() == Simple)
    return NoAuth;
  return static_cast<AuthenticationMode>(Setting::value<int>(Setting::Setting::AuthenticationMode, static_cast<int>(AuthenticationMode::NoAuth)));
}


void Settings::pandoricaMode(Settings::PandoricaMode mode)
{
  Setting::write(Setting::PandoricaMode, static_cast<int>(mode));
}

Settings::PandoricaMode Settings::pandoricaMode()
{
  return static_cast<PandoricaMode>(Setting::value<int>(Setting::PandoricaMode, static_cast<int>(PandoricaMode::Simple)));
}

string Settings::postgresqlHost()
{
  return Setting::value<string>(Setting::PostgreSQL_Hostname, "localhost");
}

int Settings::postgresqlPort()
{
  return Setting::value<int>(Setting::PostgreSQL_Port, 5432);
}

string Settings::postgresqlApplication()
{
  return Setting::value<string>(Setting::PostgreSQL_Application, "Pandorica");
}

string Settings::postgresqlDatabase()
{
  return Setting::value<string>(Setting::PostgreSQL_Database, "Pandorica");
}

string Settings::postgresqlUsername()
{
  return Setting::value<string>(Setting::PostgreSQL_Username, "Pandorica");
}

string Settings::postgresqlPassword()
{
  return Setting::value<string>(Setting::PostgreSQL_Password);
}

