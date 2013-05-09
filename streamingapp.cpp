/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2013  Marco Gulino <email>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "streamingapp.h"
#include "player/player.h"
#include <Wt/WTemplate>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WContainerWidget>
#include <Wt/WAnchor>
#include <Wt/WFileResource>
#include <Wt/WTimer>
#include <Wt/WEnvironment>
#include <Wt/WFileResource>
#include <Wt/WMenu>
#include <Wt/WImage>
#include <Wt/WText>
#include <Wt/Utils>
#include <Wt/Auth/AuthWidget>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/WPushButton>
#include <boost/algorithm/string.hpp>
#include <functional>
#include <iostream>
#include <fstream>
#include "comment.h"

#include "playlist.h"
#include "session.h"
#include "sessioninfo.h"
#include "loggedusersdialog.h"
#include "Wt-Commons/wt_helpers.h"

#include "sessiondetails.h"
#include "commentscontainerwidget.h"
#include "Wt-Commons/whtmltemplateslocalizedstrings.h"
#include "mediacollection.h"
#include "mediacollectionbrowser.h"
#include "settings.h"
#include "mediaattachment.h"
#include "settingspage.h"
#include "groupsdialog.h"
#include "latestcommentsdialog.h"
#include "MediaScanner/mediascannerdialog.h"
#include "utils.h"


#include <Wt/WOverlayLoadingIndicator>
#include <Wt/WCombinedLocalizedStrings>
#include <Wt/WStackedWidget>
#include <Wt/WLineEdit>
#include <boost/format.hpp>
#include <Wt/WSuggestionPopup>
#include <Wt/WStringListModel>
#include <Wt/WSortFilterProxyModel>
#include <Wt/WMemoryResource>
#include <Wt/WBootstrapTheme>
#include <Wt/WNavigationBar>
#include <Wt/WPopupMenu>
#include <Wt/WProgressBar>
#include <Wt/WIOService>
#include <Wt/WIOService>
#include <Wt/WLabel>
#include "private/streamingapp_p.h"
#include "authpage.h"

using namespace Wt;
using namespace std;
using namespace boost;
using namespace StreamingPrivate;
namespace fs = boost::filesystem;

typedef std::function<void(filesystem::path)> RunOnPath;


StreamingAppPrivate::StreamingAppPrivate(StreamingApp *q) : q(q), playSignal(q, "playSignal"), queueSignal(q, "queueSignal") {
  playSignal.connect([=](string uid, _n5){
    queueAndPlay(mediaCollection->media(uid));
  });
  queueSignal.connect([=](string uid, _n5){
    queue(mediaCollection->media(uid));
  });
}



StreamingApp::StreamingApp( const Wt::WEnvironment& environment) : WApplication(environment), d(new StreamingAppPrivate(this)) {
  useStyleSheet( wApp->resourcesUrl() + "form.css");
  useStyleSheet("http://gulinux.net/css/videostreaming.css");
  requireJQuery("http://ajax.googleapis.com/ajax/libs/jquery/1.8/jquery.min.js");
  require("http://gulinux.net/css/bootstrap/js/bootstrap.js");

  require("http://gulinux.net/css/mediaelement/mediaelement-and-player.js");
  useStyleSheet("http://gulinux.net/css/mediaelement/mediaelementplayer.css");
  setTheme(new WBootstrapTheme(this));
  enableUpdates(true);
  WMessageResourceBundle *xmlResourcesBundle = new WMessageResourceBundle;
  xmlResourcesBundle->use("strings");
  setLocale(d->settings.locale());
  WCombinedLocalizedStrings* combinedLocalizedStrings = new WCombinedLocalizedStrings();
  combinedLocalizedStrings->add(xmlResourcesBundle);
  combinedLocalizedStrings->add(new WHTMLTemplatesLocalizedStrings("html_templates"));
  setLocalizedStrings(combinedLocalizedStrings);

  addMetaHeader("viewport", "width=device-width, initial-scale=1, maximum-scale=1");
  
  root()->addWidget(d->authPage = new AuthPage(&d->session));
  d->authPage->loggedIn().connect(this, &StreamingApp::authEvent);
  d->authPage->loggedOut().connect(d, &StreamingAppPrivate::clearContent);
}


void StreamingAppPrivate::clearContent()
{
  delete mainWidget;
  mainWidget = 0;
}


void StreamingApp::authEvent()
{
  Dbo::Transaction t(d->session);
  log("notice") << "Clearing root and creating widgets";
  root()->addWidget(d->mainWidget = new WContainerWidget() );
  auto myUser = d->session.user();
  SessionInfo* sessionInfo = new SessionInfo(myUser, sessionId(), wApp->environment().clientAddress());
  Dbo::collection<SessionInfoPtr> oldSessions = d->session.find<SessionInfo>().where("user_id = ? and session_ended = 0").bind(myUser.id());
  wApp->log("notice") << "Searching for old sessions for this user: " << oldSessions.size();
  for(SessionInfoPtr oldSessionInfo: oldSessions) {
    wApp->log("notice") << "Found stale session " << oldSessionInfo->sessionId() << ", started " << oldSessionInfo->sessionStarted().toString();
    oldSessionInfo.modify()->end();
    oldSessionInfo.flush();
  }
  auto sessionInfoPtr = d->session.add(sessionInfo);
  wApp->log("notice") << "created sessionInfo with sessionId=" << sessionInfoPtr->sessionId();
  d->mediaCollection = new MediaCollection(d->settings.videosDir(), &d->session, this);

  d->setupMenus(d->session.user()->isAdmin());
  t.commit();
  setupGui();
}


void StreamingAppPrivate::setupMenus(bool isAdmin)
{
  wApp->log("notice") << "Setting up topbar links";
  navigationBar = new WNavigationBar();
  navigationBar->addStyleClass("navbar-static-top");
  navigationBar->setTitle(wtr("site-title"));
  navigationBar->setResponsive(true);
  
  WMenu *items = new WMenu();
  mediaListMenuItem = items->addItem(wtr("menu.videoslist"));
  mediaListMenuItem->addStyleClass("menu-media-list");
  WMenuItem *commentsMenuItem = items->addItem(wtr("menu.latest.comments"));
  commentsMenuItem->addStyleClass("menu-comments");
  WMenuItem *settingsMenuItem = items->addItem(wtr("menu.settings"));
  settingsMenuItem->addStyleClass("menu-settings");
  
  navigationBar->addMenu(items);
  
  mainWidget->addWidget(navigationBar);
  
  auto resetSelection = [=] { WTimer::singleShot(200, [=](WMouseEvent) { items->select(-1); }); };
  
  mediaListMenuItem->triggered().connect([=](WMenuItem*, _n5){
    if(widgetsStack->currentIndex()) {
      mediaListMenuItem->setText(wtr("menu.videoslist"));
      widgetsStack->setCurrentIndex(0);
    } else {
      mediaListMenuItem->setText(wtr("menu.back.to.video"));
      widgetsStack->setCurrentIndex(1);
      mediaCollectionBrowser->reload();
    }
    resetSelection();
  });
  
  
  
  WContainerWidget* latestCommentsBody = WW<WContainerWidget>().css("modal-body");
  WContainerWidget* latestCommentsContainer = WW<WContainerWidget>().css("modal fade hide comments-modal").add(latestCommentsBody);
  
  commentsMenuItem->triggered().connect([=](WMenuItem*, _n5){
    LatestCommentsDialog *dialog = new LatestCommentsDialog{&session, mediaCollection};
    dialog->setAnchorWidget(commentsMenuItem);
    dialog->animateShow({WAnimation::Fade|WAnimation::SlideInFromTop});
    dialog->mediaClicked().connect([=](Media media, _n5) { queueAndPlay(media);});
    resetSelection();
  });
  
  
  settingsMenuItem->triggered().connect([=](WMenuItem*, _n5) {
    SettingsDialog* settingsDialog = new SettingsDialog{&settings};
    settingsDialog->setAnchorWidget(commentsMenuItem);
    settingsDialog->animateShow({WAnimation::Fade|WAnimation::SlideInFromTop});
    resetSelection();
  });
  
  activeUsersMenuItem = new WMenuItem(wtr("menu.users").arg(""));
  activeUsersMenuItem->addStyleClass("menu-loggedusers");
  auto updateUsersCount = [=](WMouseEvent) {
    wApp->log("notice") << "refreshing users count";
    string query = "SELECT COUNT(*) from session_info WHERE session_ended = 0";
    Dbo::Transaction t(session);
    long sessionsCount = session.query<long>(query);
    activeUsersMenuItem->setText(wtr("menu.users").arg(sessionsCount));
  };
  
  WTimer *updateUsersCountTimer = new WTimer{q};
  updateUsersCountTimer->setInterval(5000);
  updateUsersCountTimer->timeout().connect(updateUsersCount);
  WTimer::singleShot(1500, updateUsersCount);
  updateUsersCountTimer->start();
  
  WMenuItem *logout = items->addItem(wtr("menu.logout"));
  logout->addStyleClass("menu-logout");
  if(isAdmin) {
    setupAdminMenus(items);
  }
  else {
    setupUserMenus(items);
  }
  
  logout->triggered().connect([=](WMenuItem*, _n5) {
    session.login().logout();
    wApp->quit();
    wApp->redirect(wApp->bookmarkUrl("/")); 
  });

  WLineEdit *searchBox = new WLineEdit();
  searchBox->setStyleClass("search-query");
  searchBox->setAttributeValue("placeholder", wtr("menu.search"));

  navigationBar->addSearch(searchBox, Wt::AlignRight);
  
  string jsMatcher = JS( function (editElement) {
    return function(suggestion) {
      if(suggestion==null) return editElement.value;
      var matches = suggestion.match(new RegExp(".*" + editElement.value + ".*", "gi"));
      return { match :matches != null, suggestion: suggestion.replace(new RegExp("(" + editElement.value + ")", "gi"), "<u><b>$1</b></u>") };
    }
  });
  string jsReplace = (boost::format(JS( function (editElement, suggestionText, suggestionValue) {
    editElement.value = "";
    %s
  })) % playSignal.createCall("suggestionValue")).str();
  
  WSuggestionPopup* suggestions = new WSuggestionPopup(jsMatcher, jsReplace, wApp->root());
  auto addSuggestions = [=](_n6) {
    for(pair<string,Media> media: mediaCollection->collection()) {
      WString title{media.second.title(&session)};
      suggestions->addSuggestion(title, media.first);
      if(title.toUTF8() != media.second.filename())
        suggestions->addSuggestion(media.second.filename(), media.first); // TODO check di consistenza
    }
    suggestions->forEdit(searchBox);
  };
  
  
  mediaCollection->scanned().connect(addSuggestions );
  
}

void StreamingAppPrivate::setupUserMenus(WMenu *mainMenu)
{
  mainMenu->addItem(activeUsersMenuItem);
  activeUsersMenuItem->triggered().connect([=](WMenuItem*, _n5) {
    mainMenu->select(-1);
  });
}


void StreamingAppPrivate::setupAdminMenus(WMenu *mainMenu)
{
  WPopupMenu *adminMenu = new WPopupMenu();
  adminMenu->addItem(activeUsersMenuItem);
  
  WMenuItem *allLog = adminMenu->addItem(wtr("users.history.title"));
  allLog->addStyleClass("menu-users-log");
  WMenuItem *groupsDialog = adminMenu->addItem(wtr("menu.groups"));
  WMenuItem *mediaCollectionScanner = adminMenu->addItem(wtr("mediascanner.title"));
  groupsDialog->addStyleClass("menu-groups");
  
  allLog->triggered().connect([=](WMenuItem*, _n5){
    (new LoggedUsersDialog{&session, true})->show();
  });
  
  groupsDialog->triggered().connect([=](WMenuItem*, _n5) {
    (new GroupsDialog(&session, &settings))->show();
  });
  
  mediaCollectionScanner->triggered().connect([=](WMenuItem*, _n5) {
    auto dialog = new MediaScannerDialog(&settings, mediaCollection, q);
    dialog->run();
  });
  
  auto activeUsersConnection = activeUsersMenuItem->triggered().connect([=](WMenuItem*, _n5){
    (new LoggedUsersDialog{&session})->show();
  });
  WMenuItem *adminMenuItem = mainMenu->addItem(wtr("menu.admin"));
  adminMenuItem->addStyleClass("hidden-phone");
  adminMenuItem->clicked().connect([=](WMouseEvent) {
    WTimer::singleShot(100, [=](WMouseEvent){
      if(!adminMenuItem->widget(0)->hasStyleClass("active")) {
      wApp->log("notice") << "trying to reset menu";
      mainMenu->removeItem(adminMenuItem);
      activeUsersConnection.disconnect();
      setupAdminMenus(mainMenu);
      }
    });
  });
  adminMenuItem->setMenu(adminMenu);
  adminMenuItem->addStyleClass("menu-admin");
}


void StreamingApp::setupGui()
{
  WContainerWidget* contentWidget = new WContainerWidget;

  d->playerContainerWidget = new WContainerWidget;
  d->playerContainerWidget->setContentAlignment(AlignCenter);
  d->playlist = new Playlist{&d->session};
  d->playlist->setList(true);
  d->playlist->addStyleClass("accordion-inner");
  
  WContainerWidget *playlistAccordion = WW<WContainerWidget>().css("accordion-body collapse").add(d->playlist);
  
  WContainerWidget *playlistContainer = WW<WContainerWidget>().css("accordion-group playlist").setContentAlignment(AlignCenter);
  WAnchor *playlistLink = WW<WAnchor>(string("#") + playlistAccordion->id(), wtr("playlist.accordion"))
    .setAttribute("data-toggle", "collapse")
    .setAttribute("data-parent",string("#") + contentWidget->id())
    .css("link-hand accordion-toggle playlist-toggle");
  playlistContainer->addWidget(WW<WContainerWidget>().css("accordion-heading").add(playlistLink));
  playlistContainer->addWidget(playlistAccordion);
  
  
  contentWidget->addStyleClass("accordion");
  contentWidget->addWidget(playlistContainer);
  contentWidget->addWidget(d->playerContainerWidget);
  
  d->mediaCollectionBrowser = new MediaCollectionBrowser{d->mediaCollection, &d->settings, &d->session};
  d->mediaCollectionBrowser->play().connect([=](Media media, _n5){
    d->queueAndPlay(media.path());
  });
  d->mediaCollectionBrowser->queue().connect([=](Media media, _n5){
    d->queue(media.path(), false);
  });
  
  d->widgetsStack = new WStackedWidget();
  
  d->mainWidget->addWidget(d->widgetsStack);
  d->widgetsStack->addWidget(contentWidget);
  d->widgetsStack->addWidget(d->mediaCollectionBrowser);

  
  d->playlist->next().connect(d, &StreamingAppPrivate::play);
  WTimer::singleShot(500, [=](WMouseEvent) { 
    d->mediaCollection->rescan();
    d->parseFileParameter();
  });
}


void StreamingAppPrivate::parseFileParameter() {
  if(wApp->environment().getParameter("media")) {
    log("notice") << "Got parameter file: " << *wApp->environment().getParameter("media");
    string fileHash = * wApp->environment().getParameter("media");
    queue(mediaCollection->media(fileHash).path());
  }
}


void StreamingApp::refresh() {
  Wt::WApplication::refresh();
  if(!d->session.login().loggedIn())
    return;
  d->parseFileParameter();
  if(d->player)
    d->player->refresh();
}


string StreamingAppPrivate::extensionFor ( filesystem::path p ) {
  string extension = p.extension().string();
  boost::algorithm::to_lower(extension);
  return extension;
}



void StreamingAppPrivate::queue(Media media, bool autoplay)
{
  if(!media.valid()) return;
  playlist->queue(media);
  if( (!player || !player->playing()) && autoplay) {
    WTimer::singleShot(500, [=](WMouseEvent) {
      play(playlist->first());
    });
  }
}

void StreamingAppPrivate::queueAndPlay(Media media)
{
  if(!media.valid()) return;
  playlist->reset();
  if(player && player->playing()) {
    player->stop();
    delete player;
    player = 0;
  }
  queue(media);
}


map<string,string> defaultLabels { 
  {"it", "Italiano"}, {"en", "English"}, {"und", "Undefined"}, {"", "Undefined"}
};

map<string,string> threeLangCodeToTwo {
  {"ita", "it"}, {"eng", "en"}
};

std::string defaultLabelFor(string language) {
  if(! defaultLabels.count(language))
    return defaultLabels["und"];
  return defaultLabels[language];
}


void StreamingAppPrivate::play ( Media media ) {
  mediaListMenuItem->setText(wtr("menu.videoslist"));
  widgetsStack->setCurrentIndex(0);
  log("notice") << "Playing file " << media.path();
  if(player) {
    player->stop();
    delete player;
  }
  player = settings.newPlayer();
  
  WLink mediaLink = settings.linkFor( media.path() );
  player->addSource( {mediaLink.url(), media.mimetype()} );
  player->setAutoplay(settings.autoplay(media));
  auto preview = media.preview(&session, Media::PreviewPlayer);
  if(preview) {
    auto resource = new WMemoryResource{preview->mimetype(), preview->data(), q};
//     resource->setInternalPath(wApp->sessionId() + "-preview-big-" + media.uid());
    player->setPoster(resource->url());
  }
  WContainerWidget *container = new WContainerWidget;
  Dbo::Transaction t(session);
  for(MediaAttachmentPtr subtitle : media.subtitles(&t)) {
    string lang = threeLangCodeToTwo[subtitle->value()];
    wApp->log("notice") << "Found subtitle " << subtitle.id() << ", " << lang;
    string label = subtitle->name().empty() ? defaultLabelFor(lang) : subtitle->name();
    WMemoryResource *resource = new WMemoryResource{subtitle->mimetype(), subtitle->data(), container};
    player->addSubtitles( {resource->url(), lang, label} );
  }
  player->ended().connect([=,&t](_n6){
    Dbo::Transaction t(session);
    SessionInfoPtr sessionInfo = session.find<SessionInfo>().where("session_id = ?").bind(q->sessionId());
    for(auto detail : sessionInfo.modify()->sessionDetails())
      detail.modify()->ended();
    sessionInfo.flush();
    t.commit();
    playlist->nextItem();
  });

  playerContainerWidget->clear();
  container->addWidget(player->widget());
  playerContainerWidget->addWidget(container);
  WContainerWidget* infoBox = new WContainerWidget;
  playerContainerWidget->addWidget(infoBox);
  string fileId {Utils::hexEncode(Utils::md5(media.path().string()))};
  playerContainerWidget->addWidget(new CommentsContainerWidget{fileId, &session});
  infoBox->addWidget(new WText{media.title(&session)});
  /** TODO: apparently unsupported :(
  infoBox->addWidget(new WBreak() );
  WAnchor *resizeSmall = WW(WAnchor, "#", wtr("player.resizeSmall")).css("btn btn-info btn-mini").onClick([=](WMouseEvent){player->setPlayerSize(640);});
  WAnchor *resizeMedium = WW(WAnchor, "#", wtr("player.resizeMedium")).css("btn btn-info btn-mini").onClick([=](WMouseEvent){player->setPlayerSize(900);});
  WAnchor *resizeLarge = WW(WAnchor, "#", wtr("player.resizeLarge")).css("btn btn-info btn-mini").onClick([=](WMouseEvent){player->setPlayerSize(1420);});
  infoBox->addWidget(WW(WContainerWidget).add(resizeSmall).add(resizeMedium).add(resizeLarge));
  */
  infoBox->addWidget(new WBreak );
  infoBox->addWidget(WW<WAnchor>(settings.shareLink(fileId), wtr("player.sharelink")).css("btn btn-success btn-mini"));
  infoBox->addWidget(new WText{" "});
  WAnchor *downloadLink = WW<WAnchor>(mediaLink, wtr("player.downloadlink")).css("btn btn-success btn-mini");
  downloadLink->setTarget(Wt::TargetNewWindow);
  downloadLink->setAttributeValue("data-toggle","tooltip");
  downloadLink->setAttributeValue("title", wtr("player.downloadlink.tooltip"));
  downloadLink->doJavaScript((boost::format("$('#%s').tooltip();") % downloadLink->id()).str() );
  infoBox->addWidget(downloadLink);
  wApp->setTitle( media.title(&session) );
  log("notice") << "using url " << mediaLink.url();
  SessionInfoPtr sessionInfo = session.find<SessionInfo>().where("session_id = ?").bind(q->sessionId());
  for(auto detail : sessionInfo.modify()->sessionDetails())
    detail.modify()->ended();
  sessionInfo.modify()->sessionDetails().insert(new SessionDetails{media.path()});
  sessionInfo.flush();
  t.commit();
}

void endSessionOnDatabase(string sessionId, long userId) {
  Session session;
  Dbo::Transaction t(session);
  WServer::instance()->log("notice") << "Ending session on database ( sessionId = " << sessionId << ")";

  SessionInfoPtr sessionInfo = session.find<SessionInfo>().where("session_id = ?").bind(sessionId);
  if(!sessionInfo) {
    WServer::instance()->log("notice") << "stale session not found";
    return;
  }
  WServer::instance()->log("notice") << "Session found, ending" << sessionInfo->sessionId();
  sessionInfo.modify()->end();
  for(auto detail : sessionInfo.modify()->sessionDetails()) {
    detail.modify()->ended();
    detail.flush();
  }
  sessionInfo.flush();
  t.commit();
}

StreamingApp::~StreamingApp() {
  WServer::instance()->log("notice") << "Destroying app";
  if(d->session.login().loggedIn()) {
    WServer::instance()->ioService().post(boost::bind(endSessionOnDatabase, sessionId(), d->session.user().id()));
  }
  delete d;
  WServer::instance()->log("notice") << "Deleted-pointer";
}

