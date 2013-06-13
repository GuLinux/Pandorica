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
#include "media.h"

class Settings;
class MediaCollection;
class Session;
namespace Wt {
namespace Dbo {
class Transaction;
}
}

namespace PandoricaPrivate { class NavigationBarPrivate; }

class NavigationBar : public Wt::WContainerWidget
{
public:
    enum Page { Player, MediaCollectionBrowser };

    ~NavigationBar();
    NavigationBar(Session* session, MediaCollection* mediaCollection, Settings* settings, Wt::WContainerWidget* parent = 0);
    void setup(Wt::Dbo::Transaction &transaction);

    Wt::Signal<> &showMediaCollectionBrowser();
    Wt::Signal<> &showPlayer();
    Wt::Signal<Media> &play();
    Wt::Signal<> &logout();
    
    void setPage(Page page);
    void updateUsersCount(int newUsersCount);

private:
    PandoricaPrivate::NavigationBarPrivate* const d;
};

#endif // NAVIGATIONBAR_H
