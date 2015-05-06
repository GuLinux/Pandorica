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
#include <mutex>
#include "media/media.h"
#include <media/mediacollection.h>
#include "settings.h"
#include <session.h>
#include <boost/filesystem.hpp>
#include <Wt/WJavaScript>

class PlaylistItem;
class NavigationBar;
class AuthPage;
class Playlist;


class MediaCollectionBrowser;
class Player;
class Pandorica;

class Pandorica::Private {
public:
    Private(Pandorica* q);
    Player *player = 0;
    void parseInitParameter();
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
    MediaCollection mediaCollection;
    long userId = -1;
    NavigationBar* navigationBar  = nullptr;
    Wt::WContainerWidget * playerPage;
    Wt::WContainerWidget * collectionPage;
    Wt::WContainerWidget * userSettingsPage;
    Wt::Signal<PlaylistItem*> nowPlaying;
    Wt::WContainerWidget *notifications;
    void post(std::function<void(Pandorica *app)> f, bool includeMine = false);
    void pathChanged(const std::string &path) const;
    Wt::WDialog *rescanIndicator = 0;
    void initAuthPage();
private:
    void ratingFor(Media media, Wt::Dbo::Transaction t);
    Wt::WMenuItem* activeUsersMenuItem = 0;
    long sessionsCount = -1;
};
#endif
