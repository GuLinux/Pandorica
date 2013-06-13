/*
 * Copyright (c) year Marco Gulino <marco.gulino@gmail.com>
 *
 * This file is part of Pandorica: https://github.com/GuLinux/Pandorica
 *
 * Pandorica is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details (included the COPYING file).
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NAVIGATIONBARPRIVATE_H
#define NAVIGATIONBARPRIVATE_H
#include <string>
#include <Wt/WString>
#include <Wt/WSignal>
#include <Wt/WJavaScript>
#include <media.h>

class Settings;
class MediaCollection;
class Session;
namespace Wt {
class WNavigationBar;
class WMenu;
class WMenuItem;
}

namespace PandoricaPrivate {
class NavigationBarPrivate
{
public:
    NavigationBarPrivate(Session* session, MediaCollection* mediaCollection, Settings* settings, NavigationBar* q);
    virtual ~NavigationBarPrivate();
    Wt::WNavigationBar* navigationBar;
    Session *session;
    bool isShowingMediaCollectionBrowser = false;
    void setupNavigationBar();
    void setupAdminBar();
    void setupSearchBar();
    template<typename OnItemTriggered>
    Wt::WMenuItem *createItem(Wt::WMenu *menu, Wt::WString text, OnItemTriggered onItemTriggered, std::string cssClass = std::string{});

    Wt::Signal<> showMediaCollectionBrowser;
    Wt::Signal<> showPlayer;
    Wt::Signal<Media> play;
    Wt::Signal<> logout;
    
    Wt::Signal<> viewLoggedUsers;
    Wt::Signal<> viewUsersHistory;
    Wt::Signal<> manageGroups;
    Wt::Signal<> mediaScanner;
    Wt::Signal<> findOrphans;
    Wt::Signal<> viewAs;
    Wt::Signal<> configureApp;
    
    Wt::JSignal<std::string> playJS;
    
    NavigationBar::Page currentPage;
    void resetSelection(Wt::WMenu *menu);
    Wt::WMenu* mainMenu;
    Wt::WMenuItem* mediaListMenuItem;
    Wt::WMenuItem* activeUsersMenuItem = 0;
private:
    class NavigationBar* const q;
    MediaCollection *mediaCollection;
    Settings *settings;
};
}
#endif // NAVIGATIONBARPRIVATE_H
