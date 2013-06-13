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


#ifndef STREAMINGAPP_PRIVATE_H
#define STREAMINGAPP_PRIVATE_H
#include <string>
#include "media.h"
#include "settings.h"
#include <session.h>
#include <boost/filesystem.hpp>
#include <Wt/WJavaScript>

class AuthPage;
class Playlist;


class MediaCollection;
class MediaCollectionBrowser;
class Player;
class Pandorica;
namespace PandoricaPrivate {
  class PandoricaPrivate;
  
  struct PandoricaSession {
    std::string sessionId;
    Pandorica *app;
    PandoricaPrivate *d;
  };
  std::vector<PandoricaSession> pandoricaSessions;
  
  class PandoricaPrivate {
  public:
    PandoricaPrivate(Pandorica* q);
    Player *player = 0;
    std::string extensionFor(boost::filesystem::path p);
    void parseFileParameter();
    Playlist *playlist;
    Wt::WContainerWidget* playerContainerWidget;
    void setupMenus(bool isAdmin);
    void setupAdminMenus(Wt::WMenu* mainMenu);
    Session *session;
    Pandorica *q;
    Wt::WContainerWidget *mainWidget = 0;
    AuthPage *authPage;
    Wt::WTemplate* topBarTemplate;
    Wt::WStackedWidget* widgetsStack;
    void queue(Media media, bool autoplay = true);
    void queueAndPlay(Media media);
    void play(Media media);
    Wt::JSignal<std::string> playSignal;
    Wt::JSignal<std::string> queueSignal;
    Settings settings;
    MediaCollectionBrowser* mediaCollectionBrowser;
    MediaCollection* mediaCollection;
    long userId = -1;
    void updateUsersCount();
    void registerSession();
    void unregisterSession();
  private:
    void setupUserMenus(Wt::WMenu* mainMenu);
    void ratingFor(Media media, Wt::Dbo::Transaction t);
    Wt::WMenuItem* activeUsersMenuItem;
    Wt::WNavigationBar* navigationBar;
    Wt::WMenuItem* mediaListMenuItem;
    long sessionsCount = -1;
  };
}
#endif