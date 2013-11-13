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
#include "pandorica.h"

#include <string>
#include "media/media.h"
#include "settings.h"
#include <session.h>
#include <boost/filesystem.hpp>
#include <Wt/WJavaScript>

class PlaylistItem;
class NavigationBar;
class AuthPage;
class Playlist;


class MediaCollection;
class MediaCollectionBrowser;
class Player;
class MediaScanner;
class Pandorica;
typedef std::shared_ptr<std::vector<Pandorica*>> PandoricaInstances;

class Pandorica::Private {
public:
    Private(Pandorica* q);
    Player *player = 0;
    std::string extensionFor(boost::filesystem::path p);
    void parseFileParameter();
    Playlist *playlist;
    Wt::WContainerWidget* playerContainerWidget;
    void adminActions();
    Session *session;
    Pandorica *q;
    Wt::WContainerWidget *mainWidget = 0;
    AuthPage *authPage;
    Wt::WTemplate* topBarTemplate;
    Wt::WStackedWidget* widgetsStack;
    void queue(Media media, bool autoplay = true);
    void queueAndPlay(Media media);
    void play(PlaylistItem *playlistItem);
    Wt::JSignal<std::string> playSignal;
    Wt::JSignal<std::string> queueSignal;
    Settings settings;
    MediaCollectionBrowser* mediaCollectionBrowser;
    MediaCollection* mediaCollection;
    long userId = -1;
    void updateUsersCount();
    void registerSession();
    void unregisterSession();
    NavigationBar* navigationBar;
    Wt::WContainerWidget * playerPage;
    Wt::WContainerWidget * collectionPage;
    Wt::WContainerWidget * userSettingsPage;
    Wt::Signal<PlaylistItem*> nowPlaying;
    Wt::Signal<Wt::WApplication*> aboutToQuit;
    Wt::WContainerWidget *notifications;
    std::shared_ptr<MediaScanner> mediaScanner;
    PandoricaInstances instances();
    void post(std::function<void(Pandorica *app)> f, bool includeMine = false);
private:
    void ratingFor(Media media, Wt::Dbo::Transaction t);
    Wt::WMenuItem* activeUsersMenuItem = 0;
    long sessionsCount = -1;
};

#endif
