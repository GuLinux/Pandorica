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
#include "Wt-Commons/wt_helpers.h"

#include "commentscontainerwidget.h"
#include "Wt-Commons/whtmltemplateslocalizedstrings.h"
#include <Wt-Commons/wobjectscope.h>
#include "media/mediacollection.h"
#include "mediacollectionbrowser.h"
#include "settings.h"
#include "groupsdialog.h"
#include "latestcommentsdialog.h"
#include "googlepicker.h"
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
#include <Wt/WMessageBox>
#include <Wt/WButtonGroup>
#include <Wt/WRadioButton>
#include <Wt/WVBoxLayout>
#include <Wt/WProgressBar>
#include <mutex>
#include <boost/thread.hpp>

#include <features.h>
#ifdef __GLIBC__
#include <execinfo.h>
#include <cxxabi.h>
#endif

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
  mediaCollection.scanning().connect([=](_n6) {
    rescanIndicator = new WDialog(wtr("mediacollection_rescanning_title"));
    rescanIndicator->setClosable(false);
    rescanIndicator->setModal(true);
    rescanIndicator->contents()->addWidget(new WText{wtr("mediacollection_rescanning_message")});
    rescanIndicator->show();
  });
  mediaCollection.scanned().connect([=](_n6) {
    if(!rescanIndicator)
      return;
    rescanIndicator->hide();
    delete rescanIndicator;
    rescanIndicator = 0;
  });
}


Pandorica* Pandorica::instance()
{
  return reinterpret_cast<Pandorica*>(wApp);
}



Pandorica::Pandorica( const Wt::WEnvironment& environment) : WApplication(environment), d(this) {
  useStyleSheet(Settings::staticPath("/Pandorica.css"));
  addMetaLink(Settings::staticPath("/icons/favicon.png"), "shortcut icon", {}, {}, {}, {}, false); // TODO: wt config file?
  requireJQuery(Settings::staticPath("/jquery.min.js"));
  require(Settings::staticPath("/bootstrap/js/bootstrap.min.js"));
  
  // MediaElement.js
  require(Settings::staticPath("/mediaelement/mediaelement-and-player.min.js"));
  useStyleSheet(Settings::staticPath("/mediaelement/mediaelementplayer.min.css"));
  
  // Video.js
  useStyleSheet(Settings::staticPath("/video-js/video-js.css"));
  require(Settings::staticPath("/video-js/video.js"));
  auto theme = new WBootstrapTheme(this);
  theme->setVersion(WBootstrapTheme::Version3);
  setTheme(theme);
  try {
    d->session = new Session{true};
  } catch(std::exception &e) {
    root()->addWidget(new WText{string{"Database error: "} + e.what() + "<br />Please check your configuration and try again."});
    return;
  }  
  
  enableUpdates(true);
  WMessageResourceBundle *xmlResourcesBundle = new WMessageResourceBundle;
  xmlResourcesBundle->use(Settings::sharedFilesDir("/strings"));
  if(!d->settings.locale().empty())
    setLocale(d->settings.locale());
  WCombinedLocalizedStrings* combinedLocalizedStrings = new WCombinedLocalizedStrings();
  combinedLocalizedStrings->add(xmlResourcesBundle);
  combinedLocalizedStrings->add(new WHTMLTemplatesLocalizedStrings(Settings::sharedFilesDir(PATH_SEP() + "html_templates")));
  setLocalizedStrings(combinedLocalizedStrings);
  setTitle(wtr("site-title"));

  addMetaHeader("viewport", "width=device-width, initial-scale=1, maximum-scale=1");
  internalPathChanged().connect([=](const string &p, _n5) { d->pathChanged(p); });

  d->authPage = new AuthPage(d->session);
  root()->addWidget(d->authPage); 
//  root()->addWidget(d->authPage = new AuthPage(d->session));
  d->authPage->loggedIn().connect(this, &Pandorica::authEvent);
  d->authPage->loggedOut().connect([=](_n6) {
    d->navigationBar->animateHide({WAnimation::Fade});
    d->playlist->animateHide({WAnimation::Fade});
    d->mediaCollectionBrowser->animateHide({WAnimation::Fade});
    d->mainWidget->animateHide({WAnimation::Fade});
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
    { Alert, "alert-warning" },
    { Error, "alert-danger" },
    { Success, "alert-success" },
    { Information, "alert-information" },
  };
  WPushButton *closeButton = WW<WPushButton>().setTextFormat(Wt::XHTMLText).setText("&times;").css("close").setAttribute("data-dismiss", "alert");
  WContainerWidget *notification = WW<WContainerWidget>()
    .addCss("alert alert-block pandorica-notification")
    .addCss(notificationTypes[notificationType]).add(closeButton).add(new WText(text));
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
  d->mediaCollection.setUserId(myUser.id());
  d->mainWidget->addWidget(d->navigationBar = new NavigationBar(d->session, &d->mediaCollection, &d->settings));
  d->mainWidget->addWidget(d->notifications = WW<WContainerWidget>());
  d->widgetsStack = WW<WStackedWidget>().addCss("pandorica-content");
  d->widgetsStack->setTransitionAnimation({WAnimation::Fade});
  d->navigationBar->setup(t, d->widgetsStack, {
    {NavigationBar::Player, d->playerPage = WW<WContainerWidget>().css("pandorica-player-tab")},
    {NavigationBar::MediaCollectionBrowser, d->collectionPage = new WContainerWidget},
    {NavigationBar::UserSettings, d->userSettingsPage = new WContainerWidget},
  });
  t.commit();
  setupGui();
}


#include "utils/stacktrace.h"
#include "ffmpegmedia.h"
#include "threadpool.h"
void Pandorica::notify( const WEvent &e )
{
  try {
    Wt::WApplication::notify( e );
  } catch(std::exception &exception) {
    log("warning") << "Exception caught: " << exception.what() << " on event: " << e.eventType();
    print_stacktrace();
    throw exception;
  }
}


Pandorica::~Pandorica() {
}

void Pandorica::Private::adminActions()
{
  navigationBar->findOrphans().connect([=](_n6) { (new FindOrphansDialog(&mediaCollection, session, &settings))->run(); });
  navigationBar->manageGroups().connect([=](_n6) {  (new GroupsDialog(session, &settings))->show(); });
  navigationBar->configureApp().connect([=](_n6) { ServerSettingsPage::dialog(&settings, session, &mediaCollection); });
  navigationBar->usersManagement().connect([=](_n6) { UsersManagementPage::dialog(session); });
  navigationBar->viewAs().connect([=](_n6) {
    WDialog *dialog = new WDialog;
    Dbo::Transaction t(*session);
    dialog->setTitleBarEnabled(true);
    dialog->setWindowTitle(wtr("menu.viewas"));
    WComboBox *combo = WW<WComboBox>(dialog->contents()).css("col-md-5");
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
      mediaCollection.rescan([]{});
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
  
  d->mediaCollection.rescan([=]{
    d->pathChanged(initialInternalPath);
    d->parseInitParameter();
  });

  d->mainWidget->animateShow({WAnimation::Fade});
  
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
  auto ffmpegMedia = make_shared<FFMPEGMedia>(media);
  navigationBar->switchToPlayer();
  q->log("notice") << "Playing file " << media.path();
  if(player) {
    player->stop();
    delete player;
  }
  Dbo::Transaction t(*session);
  player = settings.newPlayer(media.mimetype());
  {
    auto lock = ThreadPool::lock(media.uid());
    if(media.subtitles_count(t) < ffmpegMedia->streams(FFMPEG::Subtitles).size()) {
      auto writeLock = session->writeLock();
      auto dialog = new WDialog(WString::tr("player.subtitles.extraction.title"));
      dialog->contents()->setLayout(new WVBoxLayout);
      dialog->contents()->layout()->addWidget(new WText{WString::tr("player.subtitles.extraction")});
      WProgressBar *extractionProgress = new WProgressBar;
      dialog->contents()->layout()->addWidget(extractionProgress);
      dialog->show();
      auto dialog_existing = make_shared<bool>(true);
      new WObjectScope([=]{ *dialog_existing = false; }, dialog);
      auto app = wApp;
      ThreadPool::instance()->post([=]{
        Session session;
        Dbo::Transaction t(session);
        ffmpegMedia->extractSubtitles([=](double percent){
          WServer::instance()->post(app->sessionId(), [=]{
            if(!dialog_existing) return;
            extractionProgress->setValue(percent);
            app->triggerUpdate(); 
          });
        } );
        for(auto subtitle: ffmpegMedia->streams(FFMPEG::Subtitles)) {
          session.add(new MediaAttachment( "subtitles", subtitle.metadata["title"], subtitle.metadata["language"], media.uid(), "text/plain", subtitle.data ));
        }
        WServer::instance()->post(app->sessionId(), [=]{
          dialog->hide();
          delete dialog;
          play(playlistItem);
          app->triggerUpdate();
        });
      });
      return;
    }
  }
  wApp->setLoadingIndicator(0); // TODO: improve
  WLink mediaLink = settings.linkFor( media.path(), media.mimetype() , session, player->widget());
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
  
  infoBox->addWidget(WW<WPushButton>(wtr("player.sharelink")).css("btn btn-info btn-xs").onClick([=](WMouseEvent){
    Wt::Dbo::Transaction t( *session );
    WW<WMessageBox>(wtr( "mediabrowser.share" ),
		    wtr( "mediabrowser.share.dialog" )
		      .arg( media.title( t ) )
		      .arg( settings.shareLink( media.uid() ).url() ),
		    Wt::Information, Ok)
    .button(Ok, [=](WMessageBox *msgBox){ msgBox->accept(); }).get()->show();
  }));
  infoBox->addWidget(new WText{" "});
  
  WPushButton *downloadLink = WW<WPushButton>(wtr("player.downloadlink")).css("btn btn-info btn-xs").onClick([=](WMouseEvent){
    WDialog *downloadDialog = new WDialog(wtr("player.downloadlink"));
    downloadDialog->contents()->addWidget(new WText{wtr("player.downloadlink.message").arg(mediaLink.url()), XHTMLUnsafeText});
    downloadDialog->contents()->addWidget(new WText{wtr("player.downloadlink.message.closeplayer"), XHTMLUnsafeText});
    downloadDialog->footer()->addWidget(WW<WPushButton>(wtr("button.close")).css("btn btn-danger").onClick([=](WMouseEvent){
      downloadDialog->reject();
    }));
    downloadDialog->footer()->addWidget(WW<WPushButton>(wtr("button.ok")).css("btn btn-primary").onClick([=](WMouseEvent){
      downloadDialog->accept();
      playerContainerWidget->clear();
      player = 0;
    }));
    downloadDialog->show();
  });
  infoBox->addWidget(downloadLink);
  wApp->setTitle( wtr("site-title") + " - " + media.title(t) );
  q->log("notice") << "using url " << mediaLink.url();
  t.commit();
  nowPlaying.emit(playlistItem);
}
