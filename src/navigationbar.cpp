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
#include <Wt/WStringListModel>
#include <Wt/WSortFilterProxyModel>
#include "mediacollection.h"

using namespace Wt;
using namespace std;
using namespace PandoricaPrivate;

NavigationBarPrivate::NavigationBarPrivate(Session* session, MediaCollection* mediaCollection, Settings *settings, NavigationBar* q)
  : q(q), session(session), mediaCollection(mediaCollection), settings(settings), playJS(q, "playSignal")
{
}
NavigationBarPrivate::~NavigationBarPrivate()
{
}

NavigationBar::~NavigationBar()
{
    delete d;
}

NavigationBar::NavigationBar(Session *session, MediaCollection *mediaCollection, Settings *settings, Wt::WContainerWidget* parent)
    : d(new NavigationBarPrivate(session, mediaCollection, settings, this))
{
  hide();
  addWidget(d->navigationBar = new WNavigationBar);
  d->playJS.connect([=](string mediaId, _n5) {
    if(mediaCollection->media(mediaId).valid())
      d->play.emit(mediaCollection->media(mediaId));
  });
}

void NavigationBar::setup(Dbo::Transaction& transaction)
{
  show();
  d->setupNavigationBar(transaction);
  if(d->session->user()->isAdmin())
    d->setupAdminBar(transaction);
  d->setupSearchBar();
}

Signal<>& NavigationBar::showMediaCollectionBrowser()
{
  return d->showMediaCollectionBrowser;
}

Signal<>& NavigationBar::showPlayer()
{
  return d->showPlayer;
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

Signal<>& NavigationBar::mediaScanner()
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


void NavigationBar::setPage(NavigationBar::Page page)
{
  if(page == Player) {
    d->showPlayer.emit();
    d->mediaListMenuItem->setText(wtr("menu.mediaslist"));
  }
  if(page == MediaCollectionBrowser) {
    d->showMediaCollectionBrowser.emit();
    d->mediaListMenuItem->setText(wtr("menu.back.to.media"));
  }
  d->resetSelection(d->mainMenu);
  d->currentPage = page;
}




template<typename OnItemTriggered>
WMenuItem* NavigationBarPrivate::createItem(WMenu* menu, WString text, OnItemTriggered onItemTriggered, string cssClass)
{
  WMenuItem *item = menu->addItem(text);
  item->triggered().connect(onItemTriggered);
  if(!cssClass.empty())
    item->addStyleClass(cssClass);
  return item;
}

void NavigationBarPrivate::resetSelection(Wt::WMenu* menu)
{
  WTimer::singleShot(200, [=](WMouseEvent) {
    menu->select(-1);
  }); 
}

void NavigationBar::updateUsersCount(int newUsersCount)
{
  if(d->activeUsersMenuItem) {
    d->activeUsersMenuItem->setText(wtr("menu.users").arg(newUsersCount));
    wApp->triggerUpdate();
  }
}



void NavigationBarPrivate::setupNavigationBar(Dbo::Transaction& transaction)
{
  wApp->log("notice") << "Setting up topbar links";
  navigationBar->addStyleClass("navbar-static-top ");
  navigationBar->setTitle(wtr("site-title"));
  navigationBar->setResponsive(true);
  
  mainMenu = new WMenu();
  navigationBar->addMenu(mainMenu);
  mainMenu->itemSelected().connect([=](WMenuItem*, _n5){
    mainMenu->doJavaScript("$('.nav-collapse').collapse('hide');");
  });
  
  mediaListMenuItem = createItem(mainMenu, wtr("menu.mediaslist"), [=](WMenuItem*, _n5){
    q->setPage(currentPage == NavigationBar::MediaCollectionBrowser ? NavigationBar::Player : NavigationBar::MediaCollectionBrowser);
  }, "menu-media-list");
  
  createItem(mainMenu, wtr("menu.latest.comments"), [=](WMenuItem *item, _n5) {
    LatestCommentsDialog *dialog = new LatestCommentsDialog{session, mediaCollection};
    dialog->setAnchorWidget(item);
    dialog->animateShow({WAnimation::Fade|WAnimation::SlideInFromTop});
    dialog->mediaClicked().connect([=](Media media, _n5) { play.emit(media); });
    resetSelection(mainMenu);
  }, "menu-comments");
  
  WMenuItem *userMenuItem = mainMenu->addItem(session->login().user().identity("loginname"));
  userMenuItem->addStyleClass("menu-user");
  userMenuItem->setSubMenu(new WPopupMenu);
  
  createItem(userMenuItem->menu(), wtr("menu.settings"), [=](WMenuItem *item, _n5) {
    SettingsDialog* settingsDialog = new SettingsDialog{settings};
    settingsDialog->setAnchorWidget(item);
    settingsDialog->animateShow({WAnimation::Fade|WAnimation::SlideInFromTop});
    resetSelection(mainMenu);    
  }, "menu-settings visible-desktop");
  
  auto logout = [=](WMenuItem*, _n5) {
    this->logout.emit();
  };
  
  
  createItem(userMenuItem->menu(), wtr("menu.logout"), logout, "menu-logout");
  createItem(mainMenu, wtr("menu.logout"), logout, "menu-logout hidden-desktop");
}


void NavigationBarPrivate::setupAdminBar(Dbo::Transaction& transaction)
{
  WMenuItem *adminMenuItem = mainMenu->addItem(wtr("menu.admin"));
  WPopupMenu *adminMenu = new WPopupMenu();
  adminMenuItem->setMenu(adminMenu);
  adminMenuItem->addStyleClass("hidden-phone menu-admin");

  // Popover if Media Collection is empty
  adminMenuItem->setAttributeValue("data-toggle", "popover");
  mediaCollection->scanned().connect([=](_n6) {
    if(!mediaCollection->collection().empty()) return;
    adminMenuItem->doJavaScript((boost::format(JS(
      $('#%s').popover({placement: 'bottom', trigger: 'manual', html: true, title: %s, content: %s});
      $('#%s').popover('show');
    ))
      % adminMenuItem->id()
      % wtr("empty_media_collection_title").jsStringLiteral()
      % wtr("empty_media_collection_message").jsStringLiteral()
      % adminMenuItem->id()
    ).str());
    adminMenuItem->clicked().connect([=](WMouseEvent) { adminMenuItem->doJavaScript(
      (boost::format("$('#%s').popover('hide');") % adminMenuItem->id() ).str() );
    });
  });
  // Popover if Media Collection is empty END

  activeUsersMenuItem = createItem(adminMenu, wtr("menu.users"), [=](WMenuItem*, _n5) { viewLoggedUsers.emit();}, "menu-loggedusers");
  createItem(adminMenu, wtr("users.history.title"), [=](WMenuItem*, _n5) { viewUsersHistory.emit();}, "menu-users-log");
  createItem(adminMenu, wtr("menu.groups"), [=](WMenuItem*, _n5) { manageGroups.emit();}, "menu-groups");
  createItem(adminMenu, wtr("mediascanner.title"), [=](WMenuItem*, _n5) { mediaScanner.emit();});
  createItem(adminMenu, wtr("cleanup.orphans"), [=](WMenuItem*, _n5) { findOrphans.emit();});
  createItem(adminMenu, wtr("menu.viewas"), [=](WMenuItem*, _n5) { viewAs.emit();});
  createItem(adminMenu, wtr("menu.configure.app"), [=](WMenuItem*, _n5) { configureApp.emit();});
}

void NavigationBarPrivate::setupSearchBar()
{
  WLineEdit *searchBox = new WLineEdit();
  searchBox->setStyleClass("search-query");
  searchBox->setAttributeValue("placeholder", wtr("menu.search"));

  navigationBar->addSearch(searchBox, Wt::AlignRight);

  
  string jsMatcher = JS( function (editElement) {
    return function(suggestion) {
      if(suggestion==null) return editElement.value;
      return { match :true, suggestion: suggestion.replace(new RegExp("(" + editElement.value + ")", "gi"), "<u><b>$1</b></u>") };
    }
  });
  string jsReplace = (boost::format(JS( function (editElement, suggestionText, suggestionValue) {
    console.log("suggestionValue: " + suggestionValue);
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


