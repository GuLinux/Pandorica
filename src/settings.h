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


#ifndef SETTINGS_H
#define SETTINGS_H
#include <string>
#include <Wt/WLink>
#include <boost/filesystem.hpp>
#include <boost/program_options/variables_map.hpp>
#include <Wt/WEnvironment>
#include <Wt/WAnimation>
#include <Wt/WApplication>
#include "media.h"

namespace Wt {
class WApplication;
}

class Session;
class Player;
namespace PandoricaPrivate {
  class SettingsPrivate;
}
template<class WidgetType>
class SetAnimation {
public:
  void set(WidgetType *widget, Wt::WAnimation animation);
};
  
namespace PandoricaPrivate {

  struct Animation {
    Wt::WAnimation desktop;
    Wt::WAnimation mobile;
    Wt::WAnimation get();
  };
}

class Settings
{
public:
    Settings();
    virtual ~Settings();
    std::vector<std::string> mediasDirectories(Wt::Dbo::Session *session) const;
    void addMediaDirectory(std::string directory, Wt::Dbo::Session *session);
    void removeMediaDirectory(std::string directory, Wt::Dbo::Session *session);

    std::string value(std::string cookieName);
    std::string locale();
    void setValue(std::string settingName, std::string value);
    Wt::WLink linkFor(boost::filesystem::path p, Wt::Dbo::Session *session);
    Wt::WLink shareLink(std::string mediaId);
    Player *newPlayer();
    bool autoplay(const Media &media);
    static const std::string guiLanguage;
    static const std::string mediaAutoplay;
    static const std::string preferredPlayer;
    enum Icons { FolderBig, FolderSmall, VideoFile, AudioFile };
    enum DeployType { Internal, Static, LighttpdSecureDownload, NginxSecureLink, Undefined = 0xFF };
    static std::string icon(Icons icon);
    static std::string staticPath(const std::string &relativeUrl);
    static std::string staticDeployPath();
    std::string relativePath(std::string mediaPath, Wt::Dbo::Session *session, bool removeTrailingSlash = false) const;
    static void init(boost::program_options::variables_map commandLineOptions);
    static std::string sqlite3DatabasePath();
    static bool emailVerificationMandatory();
    
    enum AnimationType {
      PanelAnimation, ShowMediaInfoAnimation, HideMediaInfoAnimation,
      PlaylistAnimation
    };
    template<class WidgetType> inline void setAnimation(AnimationType animationType, WidgetType *widget) {
      widget->setAnimation(animations[animationType].get());
    }
    template<class WidgetType> inline void animateShow(AnimationType animationType, WidgetType *widget) {
      widget->animateShow(animations[animationType].get());
    }
    template<class WidgetType> inline void animateHide(AnimationType animationType, WidgetType *widget) {
      widget->animateHide(animations[animationType].get());
    }
    
private:
  PandoricaPrivate::SettingsPrivate *const d;
  static std::map<AnimationType, PandoricaPrivate::Animation> animations;
};

#endif // SETTINGS_H
