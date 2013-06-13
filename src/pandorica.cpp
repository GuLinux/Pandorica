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





#include "pandorica.h"
#include "private/pandorica_p.h"

#include "player/player.h"
#include <Wt/WTemplate>
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

#include "playlist.h"
#include "session.h"
#include "loggedusersdialog.h"
#include "Wt-Commons/wt_helpers.h"

#include "commentscontainerwidget.h"
#include "Wt-Commons/whtmltemplateslocalizedstrings.h"
#include "mediacollection.h"
#include "mediacollectionbrowser.h"
#include "settings.h"
#include "settingspage.h"
#include "groupsdialog.h"
#include "latestcommentsdialog.h"
#include "MediaScanner/mediascannerdialog.h"
#include "utils.h"
#include "Models/models.h"


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
#include <Wt/WPopupMenu>
#include <Wt/WProgressBar>
#include <Wt/WIOService>
#include <Wt/WIOService>
#include <Wt/WLabel>
#include <Wt/WPanel>
#include <Wt/WComboBox>
#include <Wt/Dbo/QueryModel>
#include "authpage.h"
#include "findorphansdialog.h"
#include "selectdirectories.h"
#include "serversettingsdialog.h"
#include "navigationbar.h"
#include <Wt/WStringListModel>


using namespace Wt;
using namespace std;
using namespace boost;
namespace fs = boost::filesystem;
namespace P=PandoricaPrivate;
using namespace WtCommons;

typedef std::function<void(filesystem::path)> RunOnPath;


P::PandoricaPrivate::PandoricaPrivate(Pandorica *q) : q(q), playSignal(q, "playSignal"), queueSignal(q, "queueSignal") {
  playSignal.connect([=](string uid, _n5){
    queueAndPlay(mediaCollection->media(uid));
  });
  queueSignal.connect([=](string uid, _n5){
    queue(mediaCollection->media(uid));
  });
}



Pandorica::Pandorica( const Wt::WEnvironment& environment) : WApplication(environment), d(new P::PandoricaPrivate(this)) {
  useStyleSheet( wApp->resourcesUrl() + "form.css");
  useStyleSheet(Settings::staticPath("/Pandorica.css"));
  addMetaLink(Settings::staticPath("/icons/favicon.png"), "shortcut icon", string{}, string{}, string{}, string{}, false);
  requireJQuery(Settings::staticPath("/jquery.min.js"));
  require(Settings::staticPath("/bootstrap/js/bootstrap.min.js"));
  require(Settings::staticPath("/mediaelement/mediaelement-and-player.min.js"));
  useStyleSheet(Settings::staticPath("/mediaelement/mediaelementplayer.min.css"));
  setTheme(new WBootstrapTheme(this));
  try {
    d->session = new Session{true};
  } catch(std::exception &e) {
    root()->addWidget(new WText{string{"Database error: "} + e.what() + "<br />Please check your configuration and try again."});
    return;
  }  
  
  enableUpdates(true);
  WMessageResourceBundle *xmlResourcesBundle = new WMessageResourceBundle;
  xmlResourcesBundle->use(SHARED_FILES_DIR "/strings");
  setLocale(d->settings.locale());
  WCombinedLocalizedStrings* combinedLocalizedStrings = new WCombinedLocalizedStrings();
  combinedLocalizedStrings->add(xmlResourcesBundle);
  combinedLocalizedStrings->add(new WHTMLTemplatesLocalizedStrings(SHARED_FILES_DIR "/html_templates"));
  setLocalizedStrings(combinedLocalizedStrings);
  setTitle(wtr("site-title"));

  addMetaHeader("viewport", "width=device-width, initial-scale=1, maximum-scale=1");
  

  
  root()->addWidget(d->authPage = new AuthPage(d->session));
  d->authPage->loggedIn().connect(this, &Pandorica::authEvent);
  d->authPage->loggedOut().connect([=](_n6) {
    d->unregisterSession();
    delete d->mainWidget;
    d->mainWidget = 0;
    d->userId = -1;
  });
  d->authPage->initAuth();
}


void Pandorica::authEvent()
{
  Dbo::Transaction t(*d->session);
  d->userId = d->session->user().id();
  log("notice") << "Clearing root and creating widgets";
  root()->addWidget(d->mainWidget = new WContainerWidget() );
  auto myUser = d->session->user();
  SessionInfo* sessionInfo = new SessionInfo(myUser, sessionId(), wApp->environment().clientAddress());
  Dbo::collection<SessionInfoPtr> oldSessions = d->session->find<SessionInfo>().where("user_id = ? and session_ended = 0").bind(myUser.id());
  wApp->log("notice") << "Searching for old sessions for this user: " << oldSessions.size();
  for(SessionInfoPtr oldSessionInfo: oldSessions) {
    wApp->log("notice") << "Found stale session " << oldSessionInfo->sessionId() << ", started " << oldSessionInfo->sessionStarted().toString();
    oldSessionInfo.modify()->end();
    oldSessionInfo.flush();
  }
  auto sessionInfoPtr = d->session->add(sessionInfo);
  wApp->log("notice") << "created sessionInfo with sessionId=" << sessionInfoPtr->sessionId();
  d->mediaCollection = new MediaCollection(&d->settings, d->session, this);
  d->mainWidget->addWidget(d->navigationBar = new NavigationBar(d->session, d->mediaCollection, &d->settings));

  d->navigationBar->setup(t);
  t.commit();
  d->registerSession();
  setupGui();
}

void updateSessions() {
  for(P::PandoricaSession session: P::pandoricaSessions) {
    WServer::instance()->post(session.sessionId, boost::bind(&P::PandoricaPrivate::updateUsersCount, session.d));
  }
}

void P::PandoricaPrivate::registerSession()
{
  P::pandoricaSessions.push_back({wApp->sessionId(), q, this});
  updateSessions();
}
void P::PandoricaPrivate::unregisterSession()
{
  P::pandoricaSessions.erase(remove_if( P::pandoricaSessions.begin(), P::pandoricaSessions.end(), [this](P::PandoricaSession s) { return q->sessionId() ==  s.sessionId; } ), P::pandoricaSessions.end() );
  updateSessions();
}


void endSessionOnDatabase(string sessionId, long userId) {
  Session privateSession;
  Dbo::Transaction t(privateSession);
  WServer::instance()->log("notice") << "Ending session on database ( sessionId = " << sessionId << ")";
  
  SessionInfoPtr sessionInfo = privateSession.find<SessionInfo>().where("session_id = ?").bind(sessionId);
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

Pandorica::~Pandorica() {
  WServer::instance()->log("notice") << "Destroying app";
  if(d->session->login().loggedIn()) {
    WServer::instance()->ioService().post(boost::bind(endSessionOnDatabase, sessionId(), d->userId));
  }
  d->unregisterSession();
  delete d;
  WServer::instance()->log("notice") << "Deleted-pointer";
}


void P::PandoricaPrivate::updateUsersCount()
{
  wApp->log("notice") << "refreshing users count";
  navigationBar->updateUsersCount(P::pandoricaSessions.size());
}


void P::PandoricaPrivate::adminActions()
{
  
  navigationBar->viewLoggedUsers().connect([=](_n6) {   (new LoggedUsersDialog{session, &settings})->show(); });
  navigationBar->viewUsersHistory().connect([=](_n6) { (new LoggedUsersDialog{session, &settings, true})->show(); });
  navigationBar->findOrphans().connect([=](_n6) { (new FindOrphansDialog(mediaCollection, session, &settings))->run(); });
  navigationBar->manageGroups().connect([=](_n6) {  (new GroupsDialog(session, &settings))->show(); });
  navigationBar->configureApp().connect([=](_n6) { (new ServerSettingsDialog{&settings, session, mediaCollection} )->run(); });
  navigationBar->mediaScanner().connect([=](_n6) {
    auto dialog = new MediaScannerDialog(session, &settings, mediaCollection, q);
    dialog->scanFinished().connect([=](_n6) {
      mediaCollectionBrowser->reload();
    });
    dialog->run();
  });
  navigationBar->viewAs().connect([=](_n6) {
    WDialog *dialog = new WDialog;
    Dbo::Transaction t(*session);
    dialog->setTitleBarEnabled(true);
    dialog->setWindowTitle(wtr("menu.viewas"));
    WComboBox *combo = WW<WComboBox>(dialog->contents()).css("span5");
    WStringListModel *model = new WStringListModel(combo);
    combo->setModel(model);
    
    Dbo::collection<Dbo::ptr<AuthInfo>> usersList = session->find<AuthInfo>().where("email <> '' AND email is not null");
    for(Dbo::ptr<AuthInfo> userEntry: usersList) {
      model->addString(userEntry->identity("loginname") + " (" + userEntry->email() + ")");
      model->setData(model->rowCount() -1, 0, userEntry->user().id(), UserRole);
      if(mediaCollection->viewingAs() == userEntry->user().id())
        combo->setCurrentIndex(model->rowCount());
    }
    dialog->show();
    dialog->footer()->addWidget(WW<WPushButton>(wtr("button.cancel")).css("btn btn-inverse").onClick([=](WMouseEvent) { dialog->reject(); }));
    dialog->footer()->addWidget(WW<WPushButton>(wtr("button.ok")).css("btn btn-primary").onClick([=](WMouseEvent) { dialog->accept(); }));
    dialog->finished().connect([=](WDialog::DialogCode code, _n5) {
      Dbo::Transaction t(*session);
      long userId = session->user().id();
      if(code == WDialog::Accepted) {
        userId = boost::any_cast<long long>(model->data(model->index(combo->currentIndex(), 0), UserRole));
      }
      mediaCollection->setUserId(userId);
      WServer::instance()->ioService().post([=] {
        Session threadSession;
        Dbo::Transaction t(threadSession);
        mediaCollection->rescan(t);
      });
    });
  });
}


void Pandorica::setupGui()
{
  WContainerWidget* contentWidget = new WContainerWidget;

  d->playerContainerWidget = new WContainerWidget;
  d->playerContainerWidget->setContentAlignment(AlignCenter);
  d->playlist = new Playlist{d->session};
  
  contentWidget->addWidget(WW<WContainerWidget>().add(d->playlist).setContentAlignment(AlignCenter));
  contentWidget->addWidget(d->playerContainerWidget);
  
  d->mediaCollectionBrowser = new MediaCollectionBrowser{d->mediaCollection, &d->settings, d->session};
  d->mediaCollectionBrowser->play().connect([=](Media media, _n5){
    d->queueAndPlay(media.path());
  });
  d->mediaCollectionBrowser->queue().connect([=](Media media, _n5){
    d->queue(media.path(), false);
  });
  
  d->widgetsStack = new WStackedWidget();
  
  d->navigationBar->showMediaCollectionBrowser().connect(boost::bind(&WStackedWidget::setCurrentIndex, d->widgetsStack, 1));
  d->navigationBar->showPlayer().connect(boost::bind(&WStackedWidget::setCurrentIndex, d->widgetsStack, 0));
  d->navigationBar->play().connect([=](Media media, _n5) {d->queueAndPlay(media);});
  d->navigationBar->logout().connect([=](_n6) {
    d->session->login().logout();
  });
  d->adminActions();
  
  d->mainWidget->addWidget(d->widgetsStack);
  d->widgetsStack->addWidget(contentWidget);
  d->widgetsStack->addWidget(d->mediaCollectionBrowser);

  
  d->playlist->next().connect(d, &P::PandoricaPrivate::play);
  string sessionId = wApp->sessionId();
  WServer::instance()->ioService().post([=]{
    Session threadSession;
    Dbo::Transaction t(threadSession);
    d->mediaCollection->rescan(t);
    WServer::instance()->post(sessionId, [=] { d->parseFileParameter(); });
  });
}


void P::PandoricaPrivate::parseFileParameter() {
  if(wApp->environment().getParameter("media")) {
    log("notice") << "Got parameter file: " << *wApp->environment().getParameter("media");
    string fileHash = * wApp->environment().getParameter("media");
    queue(mediaCollection->media(fileHash).path());
  }
}


void Pandorica::refresh() {
  Wt::WApplication::refresh();
  if(!d->session->login().loggedIn())
    return;
  d->parseFileParameter();
  if(d->player)
    d->player->refresh();
}


string P::PandoricaPrivate::extensionFor ( filesystem::path p ) {
  string extension = p.extension().string();
  boost::algorithm::to_lower(extension);
  return extension;
}



void P::PandoricaPrivate::queue(Media media, bool autoplay)
{
  if(!media.valid()) return;
  playlist->queue(media);
  if( (!player || !player->playing()) && autoplay) {
    WTimer::singleShot(500, [=](WMouseEvent) {
      play(playlist->first());
    });
  }
}

void P::PandoricaPrivate::queueAndPlay(Media media)
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


void P::PandoricaPrivate::play(Media media) {
  navigationBar->setPage(NavigationBar::Player);
  widgetsStack->setCurrentIndex(0);
  log("notice") << "Playing file " << media.path();
  if(player) {
    player->stop();
    delete player;
  }
  player = settings.newPlayer();
  Dbo::Transaction t(*session);
  
  WLink mediaLink = settings.linkFor( media.path() , session);
  log("notice") << "found mediaLink: " << mediaLink.url();
  player->addSource( {mediaLink.url(), media.mimetype()} );
  player->setAutoplay(settings.autoplay(media));
  auto preview = media.preview(t, Media::PreviewPlayer);
  WContainerWidget *container = new WContainerWidget;
  if(preview) {
    if(media.mimetype().find("audio") == string::npos)
      player->setPoster(preview->link(preview, t, container));
    else {
      container->addWidget(WW<WImage>(preview->link(preview, t, container)).css("album-cover"));
    }
  }
  for(MediaAttachmentPtr subtitle : media.subtitles(t)) {
    string lang = threeLangCodeToTwo[subtitle->value()];
    wApp->log("notice") << "Found subtitle " << subtitle.id() << ", " << lang;
    string label = subtitle->name().empty() ? defaultLabelFor(lang) : subtitle->name();
    player->addSubtitles( { subtitle->link(subtitle, t, container).url(), lang, label} );
  }
  player->ended().connect([=](_n6){
    wApp->setTitle( wtr("site-title"));
    Dbo::Transaction t(*session);
    SessionInfoPtr sessionInfo = session->find<SessionInfo>().where("session_id = ?").bind(q->sessionId());
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
  playerContainerWidget->addWidget(new CommentsContainerWidget{media.uid(), session});
  infoBox->addWidget(new WText{media.title(t)});
  
  WContainerWidget *ratingWidget = WW<WContainerWidget>().css("rating-container");
  WContainerWidget *avgRatingWidget = WW<WContainerWidget>().css("rating-box");
  WContainerWidget *myRating = WW<WContainerWidget>().css("rating-box");
  
  auto populateRating = [=] (Dbo::Transaction &transaction) {
    avgRatingWidget->clear();
    Ratings rating = MediaRating::ratingFor(media, transaction);
    avgRatingWidget->addWidget(new WText( WString::trn("player.avg.ratings", rating.users).arg(rating.users) ));
    for(int i=1; i<=5; i++) {
      avgRatingWidget->addWidget(WW<WImage>(Settings::staticPath("/icons/rating.png")).css(rating.ratingAverage <i ? "rating-unrated" : ""));
    }
  };
  auto setRating = [=] (int rating) {
    Dbo::Transaction t(*session);
    User::rate(session->user(), media, rating, t);
    populateRating(t);
    t.commit();
  };
  populateRating(t);
  
  myRating->addWidget(new WText(wtr("player.do.rate")));
  for(int i=1; i<=5; i++) {
    myRating->addWidget(WW<WImage>(Settings::staticPath("/icons/rating.png")).css("rate-star link-hand").onClick([=](WMouseEvent) {
      setRating(i);
    }));
  }
  
  ratingWidget->addWidget(avgRatingWidget);
  ratingWidget->addWidget(myRating);
  infoBox->addWidget(ratingWidget);
  container->destroyed().connect([=](WObject*, _n5){
    wApp->setTitle(wtr("site-title"));
  });
  
  infoBox->addWidget(WW<WAnchor>(settings.shareLink(media.uid()), wtr("player.sharelink")).css("btn btn-success btn-mini"));
  infoBox->addWidget(new WText{" "});
  WAnchor *downloadLink = WW<WAnchor>(mediaLink, wtr("player.downloadlink")).css("btn btn-success btn-mini");
  downloadLink->setTarget(Wt::TargetNewWindow);
  downloadLink->setAttributeValue("data-toggle","tooltip");
  downloadLink->setAttributeValue("title", wtr("player.downloadlink.tooltip"));
  downloadLink->doJavaScript((boost::format("$('#%s').tooltip();") % downloadLink->id()).str() );
  infoBox->addWidget(downloadLink);
  wApp->setTitle( wtr("site-title") + " - " + media.title(t) );
  log("notice") << "using url " << mediaLink.url();
  SessionInfoPtr sessionInfo = session->find<SessionInfo>().where("session_id = ?").bind(q->sessionId());
  for(auto detail : sessionInfo.modify()->sessionDetails())
    detail.modify()->ended();
  sessionInfo.modify()->sessionDetails().insert(new SessionDetails{media.path()});
  sessionInfo.flush();
  t.commit();
  playlist->setCollapsed(true);
}


