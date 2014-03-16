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

#include "pandorica.h"
#include "navigationbar.h"
#include "private/navigationbar_p.h"
#include "session.h"
#include "latestcommentsdialog.h"
#include "settingspage.h"
#include <Wt/Dbo/Transaction>
#include <Wt/WNavigationBar>
#include <Wt/WMenu>
#include <Wt/WTimer>
#include <Wt/WPopupMenu>
#include <Wt/WModelIndex>
#include <Wt/WSuggestionPopup>
#include <Wt/WLineEdit>
#include "Wt-Commons/wt_helpers.h"
#include <boost/format.hpp>
#include <Wt/WStackedWidget>
#include <Wt/WStringListModel>
#include <Wt/WSortFilterProxyModel>
#include "media/mediacollection.h"
#include "utils/d_ptr_implementation.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;

NavigationBar::Private::Private(Session* session, MediaCollection* mediaCollection, Settings *settings, NavigationBar* q)
  : q(q), session(session), mediaCollection(mediaCollection), settings(settings), playJS(q, "playSignal")
{
}

NavigationBar::~NavigationBar()
{
}

NavigationBar::NavigationBar(Session *session, MediaCollection *mediaCollection, Settings *settings, Wt::WContainerWidget* parent)
    : d(session, mediaCollection, settings, this)
{
  hide();
  addWidget(d->navigationBar = new WNavigationBar);
  d->playJS.connect([=](string mediaId, _n5) {
    if(mediaCollection->media(mediaId).valid())
      d->play.emit(mediaCollection->media(mediaId));
  });
}

void NavigationBar::setup(Dbo::Transaction& transaction, WStackedWidget* stackedWidget, NavigationBar::PagesMap pagesMap)
{
  show();
  for(auto item: pagesMap) log("notice") << "Item " << item.first << ": " << item.second;
  d->setupNavigationBar(transaction, stackedWidget, pagesMap);
  if(d->session->user()->isAdmin())
    d->setupAdminBar(transaction);
  d->setupSearchBar();
}

Signal< Media >& NavigationBar::play()
{
  return d->play;
}


Signal<>& NavigationBar::logout()
{
  return d->logout;
}

Signal<>& NavigationBar::configureApp()
{
  return d->configureApp;
}

Signal<>& NavigationBar::manageGroups()
{
  return d->manageGroups;
}

Signal< bool >& NavigationBar::mediaScanner()
{
  return d->mediaScanner;
}

Signal<>& NavigationBar::findOrphans()
{
  return d->findOrphans;
}

Signal<>& NavigationBar::viewAs()
{
  return d->viewAs;
}


Signal<>& NavigationBar::viewLoggedUsers()
{
  return d->viewLoggedUsers;
}

Signal<>& NavigationBar::viewUsersHistory()
{
  return d->viewUsersHistory;
}

Signal<>& NavigationBar::showUserSettings()
{
  return d->showUserSettings;
}

Signal< NoClass > &NavigationBar::usersManagement()
{
  return d->usersManagement;
}


auto onItemTriggered_noop = [](WMenuItem*,_n5) {};

WMenuItem* NavigationBar::Private::createItem(WMenu* menu, WString text, WWidget* parentWidget, OnItemTriggered onItemTriggered, string cssClass)
{
  WMenuItem *item = menu->addItem(text, parentWidget);
  item->triggered().connect(onItemTriggered);
  if(!cssClass.empty())
    item->addStyleClass(cssClass);
  return item;
}

void NavigationBar::Private::resetSelection(Wt::WMenu* menu)
{
  WTimer::singleShot(200, [=](WMouseEvent) {
    menu->select(previousItemIndex);
    currentItemIndex = previousItemIndex;
  }); 
}

void NavigationBar::updateUsersCount(int newUsersCount)
{
  if(d->activeUsersMenuItem) {
    d->activeUsersMenuItem->setText(wtr("menu.users").arg(newUsersCount));
    wApp->triggerUpdate();
  }
}

void NavigationBar::switchToPlayer()
{
  d->mainMenu->select(d->playerItem);
}


void NavigationBar::Private::setupNavigationBar(Dbo::Transaction& transaction,  WStackedWidget* stackedWidget, NavigationBar::PagesMap pagesMap)
{
  wApp->log("notice") << "Setting up topbar links";
  navigationBar->addStyleClass("navbar-static-top ");
  navigationBar->setTitle(wtr("site-title"));
  navigationBar->setResponsive(true);
  log("notice") << "Setting up navigation bar";
  
  mainMenu = new WMenu(stackedWidget);
  mainMenu->itemSelected().connect([=](WMenuItem *item, _n5) {
    previousItemIndex = currentItemIndex;
    currentItemIndex = mainMenu->indexOf(item);
  });
  navigationBar->addMenu(mainMenu);
// TODO: restore in some way...
//   mainMenu->itemSelected().connect([=](WMenuItem*, _n5){
//     WTimer::singleShot(1000, [=](WMouseEvent){
//       mainMenu->doJavaScript("$('.nav-collapse').collapse('hide');");
//     });
//   });
  
  WMenuItem *galleryItem = createItem(mainMenu, wtr("menu.mediaslist"), pagesMap[NavigationBar::MediaCollectionBrowser], onItemTriggered_noop, "menu-collection");
  galleryItem->setSelectable(true);

  playerItem = createItem(mainMenu, wtr("menu.back.to.media"), pagesMap[NavigationBar::Player], onItemTriggered_noop, "menu-player");
  
  createItem(mainMenu, wtr("menu.latest.comments"), 0, [=](WMenuItem *item, _n5) {
    LatestCommentsDialog *dialog = new LatestCommentsDialog{session, mediaCollection};
//     dialog->setAnchorWidget(item);
    dialog->animateShow({WAnimation::Fade|WAnimation::SlideInFromTop});
    dialog->mediaClicked().connect([=](Media media, _n5) { play.emit(media); });
    resetSelection(mainMenu);
  }, "menu-comments visible-lg visible-md");
  
  WMenuItem *userMenuItem = mainMenu->addItem(session->login().user().identity("loginname"));
  userMenuItem->addStyleClass("menu-user visible-lg visible-md");
  userMenuItem->setSubMenu(new WPopupMenu);
  
  createItem(userMenuItem->menu(), wtr("menu.settings"), 0, [=](WMenuItem *item, _n5) {
      SettingsPage::dialog(settings);
      resetSelection(mainMenu);    
  }, "menu-settings visible-lg visible-md");
  
  auto logout = [=](WMenuItem*, _n5) {
    this->logout.emit();
  };
  
  createItem(mainMenu, wtr("menu.settings"), pagesMap[NavigationBar::UserSettings], [=](WMenuItem*, _n5) { showUserSettings.emit(); }, "hidden-lg hidden-md menu-settings");
  createItem(userMenuItem->menu(), wtr("menu.logout"), 0, logout, "menu-logout");
  createItem(mainMenu, wtr("menu.logout"), 0, logout, "menu-logout hidden-lg hidden-md");
}


void NavigationBar::Private::setupAdminBar(Dbo::Transaction& transaction)
{
  WMenuItem *adminMenuItem = mainMenu->addItem(wtr("menu.admin"));
  WPopupMenu *adminMenu = new WPopupMenu();
  adminMenuItem->setMenu(adminMenu);
  adminMenuItem->addStyleClass("hidden-xs menu-admin");

  mediaCollection->scanned().connect([=](_n6){
    if(mediaCollection->collection().empty())
      pApp->notify(wtr("empty_media_collection_message"), Pandorica::NotificationType::Information);
  });

  activeUsersMenuItem = createItem(adminMenu, wtr("menu.users"), 0, [=](WMenuItem*, _n5) { viewLoggedUsers.emit();}, "menu-loggedusers");
  createItem(adminMenu, wtr("users.history.title"), 0, [=](WMenuItem*, _n5) { viewUsersHistory.emit();}, "menu-users-log");
  createItem(adminMenu, wtr("menu.groups"), 0, [=](WMenuItem*, _n5) { manageGroups.emit();}, "menu-groups");
  createItem(adminMenu, wtr("menu.usersmanagement"), 0, [=](WMenuItem*, _n5) { usersManagement.emit();}, "menu-loggedusers");
  createItem(adminMenu, wtr("mediascanner.title"), 0, [=](WMenuItem*, _n5) { mediaScanner.emit(false);});
  createItem(adminMenu, wtr("mediascanner.cd.title"), 0, [=](WMenuItem*, _n5) { mediaScanner.emit(true);});
  createItem(adminMenu, wtr("cleanup.orphans"), 0, [=](WMenuItem*, _n5) { findOrphans.emit();});
  createItem(adminMenu, wtr("menu.viewas"), 0, [=](WMenuItem*, _n5) { viewAs.emit();});
  createItem(adminMenu, wtr("menu.configure.app"), 0, [=](WMenuItem*, _n5) { configureApp.emit();});
}

void NavigationBar::Private::setupSearchBar()
{
  WLineEdit *searchBox = WW<WLineEdit>().css("form-control");
  searchBox->setTextSize(0);
  //searchBox->setStyleClass("search-query");
  searchBox->setAttributeValue("placeholder", wtr("menu.search"));

  //navigationBar->addSearch(searchBox, Wt::AlignRight);
  

    navigationBar->addWidget(WW<WContainerWidget>().css("form-group").add(searchBox), AlignRight);
  
  string jsMatcher = JS( function (editElement) {
    return function(suggestion) {
      if(suggestion==null) return editElement.value;
      return { match :true, suggestion: suggestion.replace(new RegExp("(" + editElement.value + ")", "gi"), "<u><b>$1</b></u>") };
    }
  });
  string jsReplace = (boost::format(JS( function (editElement, suggestionText, suggestionValue) {
    editElement.value = "";
    %s
  })) % playJS.createCall("suggestionValue")).str();
  WStringListModel *suggestionsModel = new WStringListModel(wApp);
  WSortFilterProxyModel *suggestionFilterModel = new WSortFilterProxyModel(wApp);
  suggestionFilterModel->setFilterFlags(RegExpFlag::MatchCaseInsensitive);
  suggestionFilterModel->setFilterKeyColumn(0);
  suggestionFilterModel->setSourceModel(suggestionsModel);
  suggestionFilterModel->setFilterRole(Wt::UserRole+1);
  WSuggestionPopup* suggestions = new WSuggestionPopup(jsMatcher, jsReplace, wApp->root());
  suggestions->filterModel().connect([=](WString &filter, _n5) {
    WString filterRegex = WString(".*{1}.*").arg(filter);
    suggestionFilterModel->setFilterRegExp(filterRegex);
  });
  suggestions->setFilterLength(-1);
  auto addSuggestions = [=](_n6) {
    Dbo::Transaction t(*session);
    suggestionsModel->setStringList({});
    searchBox->setText({});
    for(pair<string,Media> media: mediaCollection->collection()) {
      int row = suggestionsModel->rowCount();
      WString title{media.second.title(t)};
      suggestionsModel->addString(title);
      suggestionsModel->setData(row, 0, media.first, Wt::UserRole);
      suggestionsModel->setData(row, 0, media.second.filename() + ";;" + title.toUTF8(), Wt::UserRole+1);
    }
    suggestionsModel->sort(0);
    suggestions->setModel(suggestionFilterModel);
    suggestions->forEdit(searchBox, WSuggestionPopup::Editing);
  };
  

  mediaCollection->scanned().connect(addSuggestions );
}


