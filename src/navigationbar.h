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

#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <Wt/WContainerWidget>
#include "media/media.h"
#include <map>
#include "utils/d_ptr.h"

class Settings;
class MediaCollection;
class Session;
namespace Wt {
namespace Dbo {
class Transaction;
}
class WStackedWidget;
}

class NavigationBar : public Wt::WContainerWidget
{
public:
    enum Page { Player, MediaCollectionBrowser, UserSettings };

    ~NavigationBar();
    NavigationBar(Session* session, MediaCollection* mediaCollection, Settings* settings, Wt::WContainerWidget* parent = 0);
    typedef std::map<Page, Wt::WWidget*> PagesMap;

    void setup(Wt::Dbo::Transaction &transaction, Wt::WStackedWidget* stackedWidget, PagesMap pagesMap = PagesMap{} );

    Wt::Signal<Media> &play();
    Wt::Signal<> &logout();
    
    Wt::Signal<> &viewLoggedUsers();
    Wt::Signal<> &viewUsersHistory();
    Wt::Signal<> &manageGroups();
    Wt::Signal<bool> &mediaScanner();
    Wt::Signal<> &findOrphans();
    Wt::Signal<> &viewAs();
    Wt::Signal<> &configureApp();
    Wt::Signal<> &showUserSettings();
    
    void switchToPlayer();
    void updateUsersCount(int newUsersCount);

private:
  D_PTR;
};

#endif // NAVIGATIONBAR_H
