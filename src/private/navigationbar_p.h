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
#include "media/media.h"
#include "Wt-Commons/wt_helpers.h"
#include "navigationbar.h"

class Settings;
class MediaCollection;
class Session;
namespace Wt
{
  namespace Dbo
  {

    class Transaction;
  }

  class WNavigationBar;
  class WMenu;
  class WMenuItem;
}

typedef std::function<void( Wt::WMenuItem *, _n5 )> OnItemTriggered;
class NavigationBar::Private
{
  public:
    Private( Session *session, MediaCollection *mediaCollection, Settings *settings, NavigationBar *q );
    Wt::WNavigationBar *navigationBar;
    Session *session;
    bool isShowingMediaCollectionBrowser = false;
    void setupNavigationBar( Wt::Dbo::Transaction &transaction, Wt::WStackedWidget *stackedWidget, NavigationBar::PagesMap pagesMap );
    void setupAdminBar( Wt::Dbo::Transaction &transaction );
    void setupSearchBar();
    Wt::WMenuItem *createItem( Wt::WMenu *menu, Wt::WString text, Wt::WWidget *parentWidget, OnItemTriggered onItemTriggered, std::string cssClass = std::string {} );

    Wt::Signal<Media> play;
    Wt::Signal<> logout;

    Wt::Signal<> viewLoggedUsers;
    Wt::Signal<> viewUsersHistory;
    Wt::Signal<> manageGroups;
    Wt::Signal<> mediaScanner;
    Wt::Signal<> findOrphans;
    Wt::Signal<> viewAs;
    Wt::Signal<> configureApp;
    Wt::Signal<> showUserSettings;
    Wt::Signal<> usersManagement;

    Wt::JSignal<std::string> playJS;

    NavigationBar::Page currentPage;
    void resetSelection( Wt::WMenu *menu );
    Wt::WMenu *mainMenu;
    Wt::WMenuItem *activeUsersMenuItem = 0;
    int previousItemIndex, currentItemIndex = -1;
    Wt::WMenuItem *playerItem;
  private:
    class NavigationBar *const q;
    MediaCollection *mediaCollection;
    Settings *settings;
};
#endif // NAVIGATIONBARPRIVATE_H
