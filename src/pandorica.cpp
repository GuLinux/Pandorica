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
#include "utils/d_ptr_implementation.h"

#include "player/player.h"
#include <Wt/WContainerWidget>
#include <Wt/WAnchor>
#include <Wt/WTimer>
#include <Wt/WEnvironment>
#include <Wt/WMenu>
#include <Wt/WImage>
#include <Wt/WText>
#include <Wt/Utils>
#include <Wt/Auth/Dbo/AuthInfo>

#include "playlist.h"
#include "session.h"
#include "loggedusersdialog.h"
#include "Wt-Commons/wt_helpers.h"

#include "commentscontainerwidget.h"
#include "Wt-Commons/whtmltemplateslocalizedstrings.h"
#include "media/mediacollection.h"
#include "mediacollectionbrowser.h"
#include "settings.h"
#include "groupsdialog.h"
#include "latestcommentsdialog.h"
#include "MediaScanner/mediascanner.h"
#include "utils/utils.h"

#include "Models/models.h"


#include <Wt/WOverlayLoadingIndicator>
#include <Wt/WCombinedLocalizedStrings>
#include <Wt/WStackedWidget>
#include <boost/format.hpp>
#include <Wt/WBootstrapTheme>
#include <Wt/WIOService>
#include "authpage.h"
#include "findorphansdialog.h"
#include "selectdirectories.h"
#include "serversettingspage.h"
#include "navigationbar.h"
#include "settingspage.h"
#include "usersmanagementpage.h"
#include <Wt/WConfig.h>
#include <Wt/WStringListModel>
#include <Wt/WComboBox>
#include <Wt/WPushButton>
#include <Wt/WDefaultLoadingIndicator>
#include <mutex>
#include <boost/thread.hpp>


using namespace Wt;
using namespace std;
namespace fs = boost::filesystem;
namespace P=PandoricaPrivate;
using namespace WtCommons;

typedef std::function<void(fs::path)> RunOnPath;


Pandorica::Private::Private(Pandorica *q)
  : q(q), playSignal(q, "playSignal"), queueSignal(q, "queueSignal"), mediaCollection(&settings, session, q)
{
  playSignal.connect([=](string uid, _n5){
    queueAndPlay(mediaCollection.media(uid));
  });
  queueSignal.connect([=](string uid, _n5){
    queue(mediaCollection.media(uid));
  });
}


Signal<Wt::WApplication*>& Pandorica::aboutToQuit() const
{
  return d->aboutToQuit;
}

Pandorica::Pandorica( const Wt::WEnvironment& environment) : WApplication(environment), d(this) {
  useStyleSheet(Settings::staticPath("/Pandorica.css"));
  addMetaLink(Settings::staticPath("/icons/favicon.png"), "shortcut icon", {}, {}, {}, {}, false);
  requireJQuery(Settings::staticPath("/jquery.min.js"));
  require(Settings::staticPath("/bootstrap/js/bootstrap.min.js"));
  
  // MediaElement.js
  require(Settings::staticPath("/mediaelement/mediaelement-and-player.min.js"));
  useStyleSheet(Settings::staticPath("/mediaelement/mediaelementplayer.min.css"));
  
  // Video.js
  useStyleSheet(Settings::staticPath("/video-js/video-js.css"));
  require(Settings::staticPath("/video-js/video.js"));
  setTheme(new WBootstrapTheme(this));
  try {
    d->session = new Session{true};
  } catch(std::exception &e) {
    root()->addWidget(new WText{string{"Database error: "} + e.what() + "<br />Please check your configuration and try again."});
    return;
  }  
  
  enableUpdates(true);
  WMessageResourceBundle *xmlResourcesBundle = new WMessageResourceBundle;
  xmlResourcesBundle->use(Settings::sharedFilesDir("/strings"));
  setLocale(d->settings.locale());
  WCombinedLocalizedStrings* combinedLocalizedStrings = new WCombinedLocalizedStrings();
  combinedLocalizedStrings->add(xmlResourcesBundle);
  combinedLocalizedStrings->add(new WHTMLTemplatesLocalizedStrings(Settings::sharedFilesDir(PATH_SEP() + "html_templates")));
  setLocalizedStrings(combinedLocalizedStrings);
  setTitle(wtr("site-title"));

  addMetaHeader("viewport", "width=device-width, initial-scale=1, maximum-scale=1");
  internalPathChanged().connect([=](const string &p, _n5) { d->pathChanged(p); });

  
  root()->addWidget(d->authPage = new AuthPage(d->session));
  d->authPage->loggedIn().connect(this, &Pandorica::authEvent);
  d->authPage->loggedOut().connect([=](_n6) {
    d->navigationBar->animateHide({WAnimation::Fade});
    d->playlist->animateHide({WAnimation::Fade});
    d->mediaCollectionBrowser->animateHide({WAnimation::Fade});
    d->mainWidget->animateHide({WAnimation::Fade});
    if(string{"3.3.0"} == WT_VERSION_STR) {
      wApp->quit();
      wApp->redirect(wApp->bookmarkUrl("/"));
      return;
    }
    d->unregisterSession();
    WTimer::singleShot(500, [=](WMouseEvent) {
      delete d->mainWidget;
      d->mainWidget = 0;
      d->userId = -1;
    });
  });
  d->authPage->initAuth();
}


void Pandorica::notify( const WString &text, Pandorica::NotificationType notificationType, int autocloseAfterSeconds )
{
  static map<NotificationType,string> notificationTypes {
    { Alert, "" },
    { Error, "alert-error" },
    { Success, "alert-success" },
    { Information, "alert-information" },
  };
  WPushButton *closeButton = WW<WPushButton>().setTextFormat(Wt::XHTMLText).setText("&times;").css("close").setAttribute("data-dismiss", "alert");
  WContainerWidget *notification = WW<WContainerWidget>().addCss("alert alert-block pandorica-notification").addCss(notificationTypes[notificationType]).add(closeButton).add(new WText(text));
  d->notifications->addWidget(notification);
  if(autocloseAfterSeconds > 0) {
    WTimer::singleShot(autocloseAfterSeconds * 1000, [=](WMouseEvent) {
      delete notification;
    });
  }
}



void Pandorica::authEvent()
{
  Dbo::Transaction t(*d->session);
  d->userId = d->session->user().id();
  log("notice") << "Clearing root and creating widgets";
  root()->addWidget(d->mainWidget = new WContainerWidget() );
  auto myUser = d->session->user();
  SessionInfo* sessionInfo = new SessionInfo(myUser, sessionId(), wApp->environment().clientAddress());
  SessionInfo::endStale(t);
  auto sessionInfoPtr = d->session->add(sessionInfo);
  wApp->log("notice") << "created sessionInfo with sessionId=" << sessionInfoPtr->sessionId();
  d->mediaCollection.setUserId(myUser.id());
  d->mainWidget->addWidget(d->navigationBar = new NavigationBar(d->session, &d->mediaCollection, &d->settings));
  d->mainWidget->addWidget(d->notifications = WW<WContainerWidget>());
  d->widgetsStack = new WStackedWidget();
  d->widgetsStack->setTransitionAnimation({WAnimation::Fade});
  d->navigationBar->setup(t, d->widgetsStack, {
    {NavigationBar::Player, d->playerPage = new WContainerWidget},
    {NavigationBar::MediaCollectionBrowser, d->collectionPage = new WContainerWidget},
    {NavigationBar::UserSettings, d->userSettingsPage = new WContainerWidget},
    {NavigationBar::MediaScanner, d->mediaScannerPage = new WContainerWidget},
  });
  d->widgetsStack->addWidget(d->mediaScannerPage);
  t.commit();
  d->registerSession();
  setupGui();
}

void Pandorica::Private::registerSession()
{
  {
    PandoricaInstances sessions = Private::instances();
    sessions->push_back(q);
  }
  post([=](Pandorica *app){ app->d->updateUsersCount(); }, true);
}
void Pandorica::Private::unregisterSession()
{
  {
    PandoricaInstances sessionsP = Private::instances();
    list<Pandorica*> &sessions  = *sessionsP;
    WServer::instance()->log("notice") << "**** * instances size: " << sessions.size() << ", q: " << q;
    for(auto instance: sessions)
      WServer::instance()->log("notice") << "**** * instance: " << instance;
      
    sessions.erase(remove(begin(sessions), end(sessions), q));
  }
  post([=](Pandorica *app){ app->d->updateUsersCount(); }, false);
}


PandoricaInstances Pandorica::Private::instances()
{
  static list<Pandorica*> _instances {};
  static mutex instancesMutex;

  shared_ptr<unique_lock<mutex>> instancesMutexLock = make_shared<unique_lock<mutex>>(instancesMutex);

  WServer::instance()->log("notice") << "**** * creating instances mutex";
  return PandoricaInstances(&_instances, [instancesMutexLock](void *) {
    (void)instancesMutexLock;
    WServer::instance()->log("notice") << "**** * unlocking instances mutex";
  });
}

void Pandorica::Private::post( function<void(Pandorica *app)> f, bool includeMine )
{
  auto pandoricaInstances = instances();
  for(Pandorica *i: *pandoricaInstances) {
    if( i != q || includeMine)
      WServer::instance()->post(i->sessionId(), [=]{
	f(i);
	i->triggerUpdate();
      });
  }
}

void Pandorica::Private::updateUsersCount()
{
  wApp->log("notice") << "refreshing users count";
  navigationBar->updateUsersCount(Private::instances()->size());
}


void Pandorica::notify( const WEvent &e )
{
  try {
    Wt::WApplication::notify( e );
  } catch(std::exception &exception) {
    log("notice") << "Exception caught: " << exception.what() << " on event: " << e.eventType();
    throw exception;
  }
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
  d->unregisterSession();
  d->aboutToQuit.emit(this);
  WServer::instance()->log("notice") << "Destroying app";
  if(d->session->login().loggedIn()) {
    WServer::instance()->ioService().post(boost::bind(endSessionOnDatabase, sessionId(), d->userId));
  }
}

// #define MEDIASCANNER_AS_DIALOG


void Pandorica::Private::adminActions()
{
  navigationBar->viewLoggedUsers().connect([=](_n6) {   (new LoggedUsersDialog{session, &settings})->show(); });
  navigationBar->viewUsersHistory().connect([=](_n6) { (new LoggedUsersDialog{session, &settings, true})->show(); });
  navigationBar->findOrphans().connect([=](_n6) { (new FindOrphansDialog(&mediaCollection, session, &settings))->run(); });
  navigationBar->manageGroups().connect([=](_n6) {  (new GroupsDialog(session, &settings))->show(); });
  navigationBar->configureApp().connect([=](_n6) { ServerSettingsPage::dialog(&settings, session, &mediaCollection); });
  navigationBar->usersManagement().connect([=](_n6) { UsersManagementPage::dialog(session); });
  navigationBar->mediaScanner().connect([=](bool onlyCurrentDirectory, _n5) {
#ifdef MEDIASCANNER_AS_DIALOG
    mediaScanner->dialog();
#else
    widgetsStack->setCurrentWidget(mediaScannerPage); // TODO: fix
#endif
    if(onlyCurrentDirectory)
      mediaScanner->scan([=](Media& m) {
        return mediaCollectionBrowser->currentDirectoryHas(m);
      });
    else
      mediaScanner->scan();
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
      if(mediaCollection.viewingAs() == userEntry->user().id())
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
      mediaCollection.setUserId(userId);
      WServer::instance()->ioService().post([=] {
        Session threadSession;
        Dbo::Transaction t(threadSession);
        mediaCollection.rescan(t);
      });
    });
  });
}


void Pandorica::setupGui()
{
  d->mainWidget->setHidden(true);
  d->playerContainerWidget = new WContainerWidget;
  d->playerContainerWidget->setContentAlignment(AlignCenter);
  d->playlist = new Playlist{d->session, &d->settings};
  d->nowPlaying.connect(d->playlist, &Playlist::playing);
  d->playerPage->addWidget(WW<WContainerWidget>().add(d->playlist).setContentAlignment(AlignCenter));
  d->playerPage->addWidget(d->playerContainerWidget);
  
  d->mediaCollectionBrowser = new MediaCollectionBrowser{&d->mediaCollection, &d->settings, d->session};
  d->mediaCollectionBrowser->play().connect([=](Media media, _n5){
    d->queueAndPlay(media.path());
  });
  d->mediaCollectionBrowser->queue().connect([=](Media media, _n5){
    d->queue(media.path(), false);
  });
  
  d->navigationBar->play().connect([=](Media media, _n5) {d->queueAndPlay(media);});
  d->navigationBar->logout().connect([=](_n6) {
    d->session->login().logout();
  });
  d->navigationBar->showUserSettings().connect([=](_n6) {
    d->userSettingsPage->clear();
    d->userSettingsPage->addWidget(new SettingsPage(&d->settings));
  });
  d->adminActions();
  
  d->collectionPage->addWidget(d->mediaCollectionBrowser);
  d->mainWidget->addWidget(d->widgetsStack);
  
  d->userSettingsPage->setPadding(20);

  
  d->playlist->play().connect([=](PlaylistItem *item, _n5){ d->play(item);});
  string sessionId = wApp->sessionId();
  string initialInternalPath = internalPath();
  boost::thread([=]{
    Session threadSession;
    Dbo::Transaction t(threadSession);
    d->mediaCollection.rescan(t);
    WServer::instance()->post(sessionId, [=] {
      d->pathChanged(initialInternalPath);
      d->parseInitParameter();
    });
  });
  d->mainWidget->animateShow({WAnimation::Fade});
  d->mediaScanner = make_shared<MediaScanner>(d->session, &d->settings, &d->mediaCollection);
  d->mediaScanner->scanFinished().connect([=](_n6) {
    d->widgetsStack->setCurrentIndex(0);
    d->mediaCollectionBrowser->reload();
    d->pathChanged(internalPath());
  });
  
  WContainerWidget *mediaScannerContent = new WContainerWidget;
  WContainerWidget *mediaScannerFooter = WW<WContainerWidget>().padding(5);
  d->mediaScannerPage->addWidget(mediaScannerFooter);
  d->mediaScannerPage->addWidget(mediaScannerContent);
#ifndef MEDIASCANNER_AS_DIALOG
  d->mediaScanner->setup(mediaScannerContent, mediaScannerFooter);
#endif
}
void Pandorica::Private::pathChanged( const std::string &path ) const
{
  q->log("notice") << __PRETTY_FUNCTION__ << ", path=" << path;
  if(!session->login().loggedIn())
    return;
  mediaCollectionBrowser->browse(mediaCollection.find(path));
}


void Pandorica::Private::parseInitParameter() {
  if(wApp->environment().getParameter("media")) {
    wApp->log("notice") << "Got parameter file: " << *wApp->environment().getParameter("media");
    string fileHash = * wApp->environment().getParameter("media");
    queue(mediaCollection.media(fileHash).path());
  }
  if(wApp->environment().getParameter("dir")) {
    wApp->log("notice") << "Got parameter dir: " << *wApp->environment().getParameter("dir");
    string dirHash = * wApp->environment().getParameter("dir");
    auto dir = mediaCollection.find(dirHash);
    mediaCollectionBrowser->browse(mediaCollection.find(dirHash));
  }
}


void Pandorica::refresh() {
  Wt::WApplication::refresh();
  if(!d->session->login().loggedIn())
    return;
  d->parseInitParameter();
  if(d->player)
    d->player->refresh();
}


string Pandorica::Private::extensionFor ( fs::path p ) {
  string extension = p.extension().string();
  boost::algorithm::to_lower(extension);
  return extension;
}



void Pandorica::Private::queue(Media media, bool autoplay)
{
  if(!media.valid()) return;
  PlaylistItem *item = playlist->queue(media);
  if( (!player || !player->playing()) && autoplay) {
    WTimer::singleShot(500, [=](WMouseEvent) {
      playlist->play(item);
    });
  }
}

void Pandorica::Private::queueAndPlay(Media media)
{
  if(!media.valid()) return;
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


void Pandorica::Private::play(PlaylistItem *playlistItem) {
  Media media = playlistItem->media();
  navigationBar->switchToPlayer();
  q->log("notice") << "Playing file " << media.path();
  if(player) {
    player->stop();
    delete player;
  }
  player = settings.newPlayer();
  Dbo::Transaction t(*session);
  wApp->setLoadingIndicator(0); // TODO: improve
  WLink mediaLink = settings.linkFor( media.path() , session);
  q->log("notice") << "found mediaLink: " << mediaLink.url();
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
    wApp->setLoadingIndicator(new WDefaultLoadingIndicator());
    wApp->setTitle( wtr("site-title"));
    Dbo::Transaction t(*session);
    SessionInfoPtr sessionInfo = session->find<SessionInfo>().where("session_id = ?").bind(q->sessionId());
    for(auto detail : sessionInfo.modify()->sessionDetails())
      detail.modify()->ended();
    sessionInfo.flush();
    t.commit();
    playlist->next();
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
  q->log("notice") << "using url " << mediaLink.url();
  SessionInfoPtr sessionInfo = session->find<SessionInfo>().where("session_id = ?").bind(q->sessionId());
  for(auto detail : sessionInfo.modify()->sessionDetails())
    detail.modify()->ended();
  sessionInfo.modify()->sessionDetails().insert(new SessionDetails{media.path()});
  t.commit();
  nowPlaying.emit(playlistItem);
}
