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
#include "player/wmediaplayerwrapper.h"
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WContainerWidget>
#include <Wt/WAnchor>
#include <Wt/WFileResource>
#include <Wt/WTimer>
#include <Wt/WEnvironment>
#include <Wt/WMenu>
#include <Wt/WImage>
#include <Wt/WText>
#include <Wt/Utils>
#include <Wt/Auth/AuthWidget>
#include <Wt/Auth/RegistrationModel>
#include <Wt/WPushButton>
#include <boost/algorithm/string.hpp>
#include <functional>
#include <iostream>
#include <fstream>
#include <Wt/Mail/Client>
#include <Wt/Mail/Mailbox>
#include <Wt/Mail/Message>
#include "authorizeduser.h"
#include "comment.h"

#include "playlist.h"
#include "session.h"
#include "adduserdialog.h"
#include "sessioninfo.h"
#include "loggedusersdialog.h"
#include "Wt-Commons/wt_helpers.h"

#include "sessiondetails.h"
#include "commentscontainerwidget.h"
#include "readbwstats.h"
#include "player/html5player.h"
#include "Wt-Commons/whtmltemplateslocalizedstrings.h"
#include "mediacollection.h"
#include "mediacollectionbrowser.h"


#include <Wt/WOverlayLoadingIndicator>
#include <Wt/WCombinedLocalizedStrings>
#include <Wt/WStackedWidget>
#include <boost/format.hpp>

using namespace Wt;
using namespace std;
using namespace boost;
namespace fs = boost::filesystem;

typedef std::function<void(filesystem::path)> RunOnPath;

typedef pair<Dbo::ptr<User>, StreamingApp*> StreamingAppSession;

class StreamingAppPrivate {
public:
  WLink linkFor(filesystem::path p);
  WMenu *menu = 0;
  string videosDir() const;
  Player *player = 0;
  string extensionFor(filesystem::path p);
  StreamingAppPrivate(StreamingApp* q);
  void parseFileParameter();
  Playlist *playlist;
  WContainerWidget* playerContainerWidget;
  void mailForUnauthorizedUser(string email, WString identity);
  void setupMenus(AuthorizedUser::Role role);
  void setupAdminMenus();
  Session session;
  SessionInfoPtr sessionInfo;
  bool mailSent;
  StreamingApp *q;
  Signal<StreamingAppSession> sessionAdded;
  Signal<StreamingAppSession> sessionRemoved;
  void clearContent();
  WContainerWidget *mainWidget = 0;
  WContainerWidget *authContainer;
  WContainerWidget *messagesContainer;
  WMenu* topMenu;
  MediaCollection *collection;
  WStackedWidget* widgetsStack;
  void play(Media media);
  void queue(Media media);
private:
  void addSubtitlesFor(filesystem::path path);
  void setupUserMenus();
  WLink lightySecDownloadLinkFor(string secDownloadPrefix, string secDownloadSecret, filesystem::path p) const;
  WLink nginxSecLinkFor(string secDownloadPrefix, string secDownloadSecret, filesystem::path p) const;
  WMenuItem* activeUsersMenuItem;
  WMenuItem* filesListMenuItem;
};

class Message : public WTemplate {
public:
  Message(WString text, WContainerWidget* parent = 0);
};

Message::Message(WString text, WContainerWidget* parent): WTemplate(parent)
{
  addStyleClass("alert");
  setTemplateText(WString("<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>{1}").arg(text), Wt::XHTMLUnsafeText);
}



StreamingAppPrivate::StreamingAppPrivate(StreamingApp *q) : q(q) {}


class AuthWidgetCustom : public Wt::Auth::AuthWidget {
public:
    AuthWidgetCustom(const Auth::AuthService& baseAuth, Auth::AbstractUserDatabase& users, Auth::Login& login, WContainerWidget* parent = 0)
      : Auth::AuthWidget(baseAuth, users, login, parent) {}
protected:
    virtual Auth::RegistrationModel* createRegistrationModel() {
      Auth::RegistrationModel *model = Auth::AuthWidget::createRegistrationModel();
      model->setEmailPolicy(Auth::RegistrationModel::EmailMandatory);
      return model;
    }
};

typedef boost::function<void(StreamingAppSession)> SessionCallback;
struct StreamingAppSessionInfo {
  SessionCallback sessionAdded;
  SessionCallback sessionRemoved;
  StreamingAppSession session;
  static StreamingAppSessionInfo from(SessionCallback sessionAdded, SessionCallback sessionRemoved, StreamingAppSession session) {
    StreamingAppSessionInfo ret;
    ret.sessionAdded=sessionAdded;
    ret.sessionRemoved=sessionRemoved;
    ret.session=session;
    return ret;
  }
};

class StreamingAppSessions {
public:
  void registerSession(string sessionId, SessionCallback onSessionAdded, SessionCallback onSessionRemoved, StreamingAppSession newSession);
  void unregisterSession(string sessionId);
  ulong sessionCount();
private:
  map<string,StreamingAppSessionInfo> sessions;
};

ulong StreamingAppSessions::sessionCount()
{
  return sessions.size();
}


void StreamingAppSessions::registerSession(string sessionId, SessionCallback onSessionAdded, SessionCallback onSessionRemoved, StreamingAppSession newSession)
{
  sessions[sessionId] = StreamingAppSessionInfo::from(onSessionAdded, onSessionRemoved, newSession);
  for(pair<string,StreamingAppSessionInfo> session: sessions)
    WServer::instance()->post(session.first, boost::bind(session.second.sessionAdded, newSession));
}

void StreamingAppSessions::unregisterSession(string sessionId)
{
  StreamingAppSessionInfo oldSession = sessions[sessionId];
  sessions.erase(sessionId);
  for(pair<string,StreamingAppSessionInfo> session: sessions)
    WServer::instance()->post(session.first, boost::bind(session.second.sessionRemoved, oldSession.session));
}

StreamingAppSessions streamingAppSessions;

StreamingApp::StreamingApp ( const Wt::WEnvironment& environment) : WApplication(environment), d(new StreamingAppPrivate(this)) {
  useStyleSheet("http://gulinux.net/css/videostreaming.css");
  requireJQuery("http://ajax.googleapis.com/ajax/libs/jquery/1.8/jquery.min.js");
  useStyleSheet("http://gulinux.net/css/bootstrap/css/bootstrap.css");
  useStyleSheet("http://gulinux.net/css/bootstrap/css/bootstrap-responsive.css");
  require("http://gulinux.net/css/bootstrap/js/bootstrap.js");
//   useStyleSheet("http://vjs.zencdn.net/c/video-js.css");
//   require("http://vjs.zencdn.net/c/video.js");
  require("//cdn.jsdelivr.net/mediaelement/2.10.1/mediaelement-and-player.js");
  useStyleSheet("//cdn.jsdelivr.net/mediaelement/2.10.1/mediaelementplayer.css");
  enableUpdates(true);
  d->session.login().changed().connect(this, &StreamingApp::authEvent);
  WMessageResourceBundle *xmlResourcesBundle = new WMessageResourceBundle;
  xmlResourcesBundle->use("templates");
  WCombinedLocalizedStrings* combinedLocalizedStrings = new WCombinedLocalizedStrings();
  combinedLocalizedStrings->add(new WHTMLTemplatesLocalizedStrings("html_templates"));
  combinedLocalizedStrings->add(xmlResourcesBundle);
  setLocalizedStrings(combinedLocalizedStrings);

  addMetaHeader("viewport", "width=device-width, initial-scale=1, maximum-scale=1");
  
  d->authContainer = new WContainerWidget();
  d->authContainer->addWidget(WW(WText, WString("<h1 style=\"text-align: center;\">{1}</h1>").arg(WString::tr("site-title"))));
  root()->addWidget(d->authContainer);
  AuthWidgetCustom* authWidget = new AuthWidgetCustom(Session::auth(), d->session.users(), d->session.login());
  authWidget->model()->addPasswordAuth(&Session::passwordAuth());
  authWidget->model()->addOAuth(Session::oAuth());
  authWidget->setRegistrationEnabled(true);
  authWidget->processEnvironment();
  d->authContainer->addWidget(authWidget);
  d->authContainer->addWidget(d->messagesContainer = new WContainerWidget());
}


void StreamingAppPrivate::clearContent()
{
  delete mainWidget;
  mainWidget = 0;
  messagesContainer->clear();
}


void StreamingApp::authEvent()
{
  if(!d->session.login().loggedIn()) {
    streamingAppSessions.unregisterSession(wApp->sessionId());
    d->clearContent();
    d->authContainer->show();
    return;
  }
  log("notice") << "User logged in";
//   changeSessionId();
  Auth::User user = d->session.login().user();
  WPushButton *refreshButton = WW(WPushButton, "Retry").css("btn btn-link").onClick([this](WMouseEvent) {
    authEvent();
  }).setAttribute("data-dismiss", "alert");
  if(user.email().empty()) {
    log("notice") << "User email empty, unconfirmed?";
    Message *message = WW(Message, "You need to verify your email address before logging in.<br />\
    Please check your inbox.<br />${refresh}").addCss("alert-block");
    message->bindWidget("refresh", refreshButton);
    d->messagesContainer->addWidget(message);
    return;
  }
  log("notice") << "User email confirmed";
  Dbo::Transaction t(d->session);
  AuthorizedUserPtr authUser = d->session.find<AuthorizedUser>().where("email = ?").bind(user.email());
  if(!authUser) {
    Message *message = WW(Message, "Your user is not yet authorized for viewing videos.<br />\
    If you think this is an error, contact me at marco.gulino (at) gmail.com<br />${refresh}").addCss("alert-block");
    if(!d->mailSent) {
      d->mailForUnauthorizedUser(user.email(), user.identity(Auth::Identity::LoginName));
      d->mailSent = true;
    }
    d->messagesContainer->addWidget(message);
    message->bindWidget("refresh", refreshButton);
    return;
  }
  log("notice") << "Clearing root and creating widgets";
  d->authContainer->hide();
  root()->addWidget(d->mainWidget = new WContainerWidget() );
  Dbo::ptr< User > myUser = d->session.user();
  SessionInfo* sessionInfo = new SessionInfo(myUser, sessionId(), wApp->environment().clientAddress());
  Dbo::collection< SessionInfoPtr > oldSessions = d->session.find<SessionInfo>().where("user_id = ? and session_ended = 0").bind(myUser.id());
  for(SessionInfoPtr oldSessionInfo: oldSessions) {
    oldSessionInfo.modify()->end();
    oldSessionInfo.flush();
  }
  d->sessionInfo = d->session.add(sessionInfo);
  t.commit();
  d->setupMenus(authUser->role());
  d->collection = new MediaCollection(d->videosDir(), this);
  setupGui();
  auto sessionAddedCallback = [this](StreamingAppSession newSession) { d->sessionAdded.emit(newSession); wApp->triggerUpdate(); wApp->log("notice") << "*** Session added (userId=" << newSession.first.id() << ")"; };
  auto sessionRemovedCallback = [this](StreamingAppSession sessionRemoved) { d->sessionRemoved.emit(sessionRemoved); wApp->triggerUpdate(); wApp->log("notice") << "*** Session removed (userId=" << sessionRemoved.first.id() << ")"; };
  streamingAppSessions.registerSession(sessionId(), sessionAddedCallback, sessionRemovedCallback, StreamingAppSession(myUser,this) );
}


void StreamingAppPrivate::setupMenus(AuthorizedUser::Role role)
{
  wApp->log("notice") << "Setting up topbar links";
  topMenu = new WMenu(Wt::Horizontal);
  topMenu->setRenderAsList(true);
  topMenu->setStyleClass("nav");
  filesListMenuItem = topMenu->addItem(WString::tr("menu.videoslist"), 0);
  
  activeUsersMenuItem = new WMenuItem("Active Users", 0);
  
  auto setLoggedUsersTitle = [this](StreamingAppSession, _n5){
    activeUsersMenuItem->setText(WString("Active Users ({1})").arg(streamingAppSessions.sessionCount()));
  };
  
  sessionAdded.connect(setLoggedUsersTitle);
  sessionRemoved.connect(setLoggedUsersTitle);
  
  if(role == AuthorizedUser::Admin)
    setupAdminMenus();
  else {
    setupUserMenus();
  }
  
  
//   WMenuItem *refresh = topMenu->addItem("Refresh", 0);
  
  string serverStatusUrl;
  if(wApp->readConfigurationProperty("server-status-url", serverStatusUrl))
    new ReadBWStats(topMenu->addItem("", 0), serverStatusUrl, q);
  
  WMenuItem *logout = topMenu->addItem("Logout", 0);
  logout->itemWidget()->parent()->addStyleClass("pull-right");
  
  topMenu->itemSelected().connect([logout/*,refresh*/,this](WMenuItem *selected,_n5){
    if(selected==logout) {
      session.login().logout();
      wApp->redirect(wApp->bookmarkUrl("/"));
    }
    /*
    if(selected==refresh) {
      WOverlayLoadingIndicator* indicator = new WOverlayLoadingIndicator();
      wApp->root()->addWidget(indicator->widget());
      collection->rescan();
      WTimer::singleShot(1000, [indicator,this](WMouseEvent) { delete indicator;});
    }
    */
    if(selected == filesListMenuItem) {
      if(widgetsStack->currentIndex()) {
        filesListMenuItem->setText(WString::tr("menu.videoslist"));
        widgetsStack->setCurrentIndex(0);
      } else {
        filesListMenuItem->setText(WString::tr("menu.back.to.video"));
        widgetsStack->setCurrentIndex(1);
      }
    }
  });
  
  WContainerWidget* navBar = new WContainerWidget();
  WContainerWidget* navbarInner = new WContainerWidget();

  navBar->addWidget(navbarInner);
  navbarInner->addWidget(WW(WText,WString::tr("site-title")).css("brand"));
  
  navbarInner->addWidget(new WText(HTML(<a class="btn btn-navbar" data-toggle="collapse" data-target=".nav-collapse">
  <span class="icon-bar"></span>
  <span class="icon-bar"></span>
  <span class="icon-bar"></span>
  </a>), Wt::XHTMLUnsafeText));
  
  navbarInner->addWidget(WW(WContainerWidget).css("nav-collapse collapse").add(topMenu));
  
  navBar->setStyleClass("navbar navbar-static-top navbar-inverse");
  navbarInner->setStyleClass("navbar-inner");
  mainWidget->addWidget(navBar);
}

void StreamingAppPrivate::setupUserMenus()
{
    topMenu->addItem(activeUsersMenuItem);
}


void StreamingAppPrivate::setupAdminMenus()
{
  WMenuItem *addUserMenu = topMenu->addItem("Add User", 0);
  topMenu->addItem(activeUsersMenuItem);
  WMenuItem *allLog = topMenu->addItem("Users Log", 0);
  const string *addUserParameter = wApp->environment().getParameter("add_user_email");
  
  auto displayAddUserDialog = [addUserParameter,this](WMouseEvent){
    string addUserEmail =  addUserParameter? *addUserParameter: string();
    AddUserDialog *dialog = new AddUserDialog(&session, addUserEmail);
    dialog->show();
  };
  
  if(addUserParameter)
    WTimer::singleShot(1000, displayAddUserDialog);
  
    topMenu->itemSelected().connect([addUserMenu,allLog,displayAddUserDialog,this](WMenuItem *selected, _n5){
    if(selected == addUserMenu) {
      displayAddUserDialog(WMouseEvent());
    }
    if(selected == activeUsersMenuItem) {
      WDialog *dialog = new LoggedUsersDialog(&session);
      dialog->show();
    }
    if(selected == allLog) {
      WDialog *dialog = new LoggedUsersDialog(&session, true);
      dialog->show();
    }
  });
}


void StreamingAppPrivate::mailForUnauthorizedUser(string email, WString identity)
{
  Mail::Client client;
  Mail::Message message;
  message.setFrom(Mail::Mailbox("noreply@gulinux.net", "Videostreaming Gulinux"));
  message.setSubject("VideoStreaming: unauthorized user login");
  message.setBody(WString("The user {1} ({2}) just tried to login.\n\
Since it doesn't appear to be in the authorized users list, it needs to be moderated.\n\
Visit {3} to do it.").arg(identity).arg(email).arg(wApp->makeAbsoluteUrl(wApp->bookmarkUrl("/")) + "?add_user_email=" + email));
  message.addRecipient(Mail::To, Mail::Mailbox("marco.gulino@gmail.com", "Marco Gulino"));
  client.connect();
  client.send(message);
}

void StreamingApp::setupGui()
{
  WContainerWidget* contentWidget = new WContainerWidget();

  d->playerContainerWidget = new WContainerWidget();
  d->playerContainerWidget->setContentAlignment(AlignCenter);
  d->playlist = new Playlist();
  d->playlist->setList(true);
  d->playlist->addStyleClass("accordion-inner");
  
  WContainerWidget *playlistAccordion = WW(WContainerWidget).css("accordion-body collapse").add(d->playlist);
  
  WContainerWidget *playlistContainer = WW(WContainerWidget).css("accordion-group playlist").setContentAlignment(AlignCenter);
  playlistContainer->addWidget(WW(WContainerWidget).css("accordion-heading").add(WW(WAnchor, string("#") + playlistAccordion->id(),"Playlist")
    .setAttribute("data-toggle", "collapse").setAttribute("data-parent",string("#") + contentWidget->id()).css("accordion-toggle")));
  playlistContainer->addWidget(playlistAccordion);
  
  
  contentWidget->addStyleClass("accordion");
  contentWidget->addWidget(playlistContainer);
  contentWidget->addWidget(d->playerContainerWidget);
  
  MediaCollectionBrowser* browser = new MediaCollectionBrowser(d->collection);
  browser->play().connect([this](Media media, _n5){
    d->play(media.path());
  });
  browser->queue().connect([this](Media media, _n5){
    d->queue(media.path());
  });
  
  d->widgetsStack = new WStackedWidget();
  
  d->mainWidget->addWidget(d->widgetsStack);
  d->widgetsStack->addWidget(contentWidget);
  d->widgetsStack->addWidget(browser);

  d->parseFileParameter();
  
  d->playlist->next().connect(d, &StreamingAppPrivate::play);
}


void StreamingAppPrivate::parseFileParameter() {
  if(wApp->environment().getParameter("file")) {
    log("notice") << "Got parameter file: " << *wApp->environment().getParameter("file");
    WTimer::singleShot(1000, [this](WMouseEvent&) {
      string fileHash = * wApp->environment().getParameter("file");
      queue(collection->media(fileHash).path());
    });
  }    
}


void StreamingApp::refresh() {
  Wt::WApplication::refresh();
  d->parseFileParameter();
}

WLink StreamingAppPrivate::linkFor ( filesystem::path p ) {
  string videosDeployDir;
  string secDownloadPrefix;
  string secDownloadSecret;
  
  if(wApp->readConfigurationProperty("secdownload-prefix", secDownloadPrefix) &&
    wApp->readConfigurationProperty("secdownload-secret", secDownloadSecret)) {
    return lightySecDownloadLinkFor(secDownloadPrefix, secDownloadSecret, p);
  }
  
  if(wApp->readConfigurationProperty("seclink-prefix", secDownloadPrefix) && wApp->readConfigurationProperty("seclink-secret", secDownloadSecret)) {
    return nginxSecLinkFor(secDownloadPrefix, secDownloadSecret, p);
  }
  
  if(wApp->readConfigurationProperty("videos-deploy-dir", videosDeployDir )) {
    string relpath = p.string();
    boost::replace_all(relpath, videosDir(), videosDeployDir);
    return WLink(relpath);
  }

   WLink link = WLink(new WFileResource(p.string(), wApp));
   wApp->log("notice") << "Generated url: " << link.url();
   return link;
}

WLink StreamingAppPrivate::lightySecDownloadLinkFor(string secDownloadPrefix, string secDownloadSecret, filesystem::path p) const
{
    string filePath = p.string();
    boost::replace_all(filePath, videosDir(), "");
    string hexTime = (boost::format("%1$x") %WDateTime::currentDateTime().toTime_t()) .str();
    string token = Utils::hexEncode(Utils::md5(secDownloadSecret + filePath + hexTime));
    string secDownloadUrl = secDownloadPrefix + token + "/" + hexTime + filePath;
    wApp->log("notice") << "****** secDownload: filename= " << filePath;
    wApp->log("notice") << "****** secDownload: url= " << secDownloadUrl;
    return WLink(secDownloadUrl);
}

WLink StreamingAppPrivate::nginxSecLinkFor(string secDownloadPrefix, string secDownloadSecret, filesystem::path p) const
{
    string filePath = p.string();
    boost::replace_all(filePath, videosDir(), "");
    long expireTime = WDateTime::currentDateTime().addSecs(20000).toTime_t();
    string token = Utils::base64Encode(Utils::md5( (boost::format("%s%s%d") % secDownloadSecret % filePath % expireTime).str() ), false);
    token = boost::replace_all_copy(token, "=", "");
    token = boost::replace_all_copy(token, "+", "-");
    token = boost::replace_all_copy(token, "/", "_");
    string secDownloadUrl = (boost::format("%s%s?st=%s&e=%d") % secDownloadPrefix % filePath % token % expireTime).str();
    wApp->log("notice") << "****** secDownload: filename= " << filePath;
    wApp->log("notice") << "****** secDownload: url= " << secDownloadUrl;
    return WLink(secDownloadUrl);
}



string StreamingAppPrivate::extensionFor ( filesystem::path p ) {
  string extension = p.extension().string();
  boost::algorithm::to_lower(extension);
  return extension;
}


string StreamingAppPrivate::videosDir() const {
  string videosDir = string(getenv("HOME")) + "/Videos";
  wApp->readConfigurationProperty("videos-dir", videosDir);
  return videosDir;
}


void StreamingAppPrivate::queue(Media media)
{
  if(!media.valid()) return;
  playlist->queue(media);
  if(!player || !player->playing()) {
    WTimer::singleShot(500, [this](WMouseEvent) {
      play(playlist->first());
    });
  }
}


void StreamingAppPrivate::play ( Media media ) {
  filesListMenuItem->setText(WString::tr("menu.videoslist"));
  widgetsStack->setCurrentIndex(0);
  log("notice") << "Playing file " << media.path();
  if(player) {
    player->stop();
    delete player;
  }
  player = new HTML5Player();
  
  player->addSource( Source(linkFor( media.path() ).url(), media.mimetype()) );
  player->setAutoplay(true);
  
  player->ended().connect([this](_n6){
    Dbo::Transaction t(session);
    for(auto detail : sessionInfo.modify()->sessionDetails())
      detail.modify()->ended();
    sessionInfo.flush();
    t.commit();
    playlist->nextItem();
  });

  addSubtitlesFor(media.path());
  playerContainerWidget->clear();
  playerContainerWidget->addWidget(player->widget());
  WContainerWidget* infoBox = new WContainerWidget();
  playerContainerWidget->addWidget(infoBox);
  string fileId = Utils::hexEncode(Utils::md5(media.path().string()));
  playerContainerWidget->addWidget(new CommentsContainerWidget(fileId, &session));
  infoBox->addWidget(new WText(string("File: ") + media.filename()));
  WLink shareLink(wApp->bookmarkUrl("/") + string("?file=") + fileId);
  infoBox->addWidget(new WBreak() );
  infoBox->addWidget(new WAnchor(shareLink, "Link per la condivisione"));
  wApp->setTitle( media.filename() );
  log("notice") << "using url " << linkFor( media.path() ).url();
  Dbo::Transaction t(session);
  for(auto detail : sessionInfo.modify()->sessionDetails())
    detail.modify()->ended();
  sessionInfo.modify()->sessionDetails().insert(new SessionDetails(media.path()));
  sessionInfo.flush();
  t.commit();
}

void StreamingAppPrivate::addSubtitlesFor(filesystem::path path)
{
  wApp->log("notice") << "Adding subtitles for " << path;
  fs::path subsdir(path.parent_path().string() + "/.subs");
  wApp->log("notice") << "subs path: " << subsdir;
  if(!fs::exists(subsdir)) {
    wApp->log("notice") << "subs path: " << subsdir << " not existing, exiting";
    return;
  }
  vector<filesystem::path> v;
  copy(filesystem::directory_iterator(subsdir), filesystem::directory_iterator(), back_inserter(v));
  sort(v.begin(), v.end());
  for(filesystem::path langPath: v) {
    string lang = langPath.filename().string();
    string name = "Subtitles (unknown language)";
    if(lang == "it")
      name = "Italiano";
    if(lang == "en")
      name = "English";
    wApp->log("notice") << "subs lang path: " << langPath;
    fs::path mySub = fs::path(langPath.string() + "/" + path.filename().string() + ".vtt");
    if(fs::exists(mySub)) {
      wApp->log("notice") << "sub found: lang= " << lang << ", path= " << mySub;
      player->addSubtitles(Track(linkFor(mySub).url(), lang, name));
    }
  }
}



StreamingApp::~StreamingApp() {
  if(d->sessionInfo) {
    Dbo::Transaction t(d->session);
    d->sessionInfo.modify()->end();
    for(auto detail : d->sessionInfo.modify()->sessionDetails())
      detail.modify()->ended();
    t.commit();
  }
    streamingAppSessions.unregisterSession(sessionId());
  delete d;
}

