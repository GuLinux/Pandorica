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
#include <Wt/WMenu>
#include <Wt/WImage>
#include <Wt/WText>
#include <Wt/Utils>
#include <Wt/Auth/AuthWidget>
#include <Wt/Auth/RegistrationModel>
#include <Wt/Auth/Dbo/AuthInfo>
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
#include "Wt-Commons/whtmltemplateslocalizedstrings.h"
#include "mediacollection.h"
#include "mediacollectionbrowser.h"
#include "settings.h"


#include <Wt/WOverlayLoadingIndicator>
#include <Wt/WCombinedLocalizedStrings>
#include <Wt/WStackedWidget>
#include <Wt/WLineEdit>
#include <boost/format.hpp>
#include <Wt/WSuggestionPopup>
#include <Wt/WStringListModel>
#include <Wt/WSortFilterProxyModel>

using namespace Wt;
using namespace std;
using namespace boost;
namespace fs = boost::filesystem;

typedef std::function<void(filesystem::path)> RunOnPath;

typedef pair<Dbo::ptr<User>, StreamingApp*> StreamingAppSession;

class StreamingAppPrivate {
public:
  StreamingAppPrivate(StreamingApp* q);
  Player *player = 0;
  string extensionFor(filesystem::path p);
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
  WTemplate* topBarTemplate;
  MediaCollection *collection;
  WStackedWidget* widgetsStack;
  void queue(Media media);
  void queueAndPlay(Media media);
  void play(Media media);
  JSignal<string> playSignal;
  JSignal<string> queueSignal;
  Settings settings;
private:
  void addSubtitlesFor(filesystem::path path);
  void setupUserMenus();
  WText* activeUsersMenuItem;
  WText* filesListMenuItem;
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


StreamingAppPrivate::StreamingAppPrivate(StreamingApp *q) : q(q), playSignal(q, "playSignal"), queueSignal(q, "queueSignal") {
  playSignal.connect([this](string uid, _n5){
    queueAndPlay(collection->media(uid));
  });
  queueSignal.connect([this](string uid, _n5){
    queue(collection->media(uid));
  });
}


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
    if(sessionId != session.first)
      WServer::instance()->post(session.first, boost::bind(session.second.sessionRemoved, oldSession.session));
}

StreamingAppSessions streamingAppSessions;

StreamingApp::StreamingApp( const Wt::WEnvironment& environment) : WApplication(environment), d(new StreamingAppPrivate(this)) {
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

  d->collection = new MediaCollection(d->settings.videosDir(), this);
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
  setupGui();
  auto sessionAddedCallback = [this](StreamingAppSession newSession) { d->sessionAdded.emit(newSession); wApp->triggerUpdate(); wApp->log("notice") << "*** Session added (userId=" << newSession.first.id() << ")"; };
  auto sessionRemovedCallback = [this](StreamingAppSession sessionRemoved) { d->sessionRemoved.emit(sessionRemoved); wApp->triggerUpdate(); wApp->log("notice") << "*** Session removed (userId=" << sessionRemoved.first.id() << ")"; };
  streamingAppSessions.registerSession(sessionId(), sessionAddedCallback, sessionRemovedCallback, StreamingAppSession(myUser,this) );
}


void StreamingAppPrivate::setupMenus(AuthorizedUser::Role role)
{
  wApp->log("notice") << "Setting up topbar links";
  topBarTemplate = new WTemplate(WString::tr("navbar"));
  topBarTemplate->addFunction("tr", &WTemplate::Functions::tr);
  filesListMenuItem = new WText(WString::tr("menu.videoslist"));
  WText *latestCommentsMenuItem = new WText(WString::tr("menu.latest.comments"));
  
  WContainerWidget* latestCommentsBody = WW(WContainerWidget).css("modal-body");
  WContainerWidget* latestCommentsContainer = WW(WContainerWidget).css("modal fade hide comments-modal").add(latestCommentsBody);
  
  
  latestCommentsMenuItem->clicked().connect([latestCommentsMenuItem,latestCommentsContainer,latestCommentsBody,this](WMouseEvent){
    latestCommentsBody->clear();
    wApp->log("notice") << "************** latestComments is visible: " << latestCommentsBody->isVisible();
    Dbo::Transaction t(session);
    Dbo::collection<CommentPtr> latestComments = session.find<Comment>().orderBy("last_updated desc").limit(5);
    for(CommentPtr comment: latestComments) {
      WContainerWidget* commentWidget = new WContainerWidget;
      Media media = collection->media(comment->videoId());
      
      WContainerWidget *header = WW(WContainerWidget);
      header->setContentAlignment(AlignCenter);
      
      WAnchor *videoLink = new WAnchor("#", media.filename());
      videoLink->setStyleClass("label label-info comment-box-element");
      header->addWidget(videoLink);
      Dbo::ptr<Auth::Dbo::AuthInfo<User>> authInfo = session.find<Auth::Dbo::AuthInfo<User>>().where("user_id = ?").bind(comment->user().id());
      header->addWidget(WW(WText,WString("{1} ({2})").arg(authInfo->identity("loginname")).arg(comment->lastUpdated().toString()))
        .css("label label-success comment-box-element"));
      commentWidget->addWidget(header);
      videoLink->clicked().connect([latestCommentsContainer, latestCommentsMenuItem, media,this](WMouseEvent){
        string hidejs = (boost::format(JS( $('#%s').modal('hide'); )) % latestCommentsContainer->id()).str();
        latestCommentsMenuItem->doJavaScript(hidejs);
        queueAndPlay(media);
      });
      commentWidget->addWidget(WW(WText, WString::fromUTF8(comment->content())).css("well comment-text comment-box-element").setInline(false));
      latestCommentsBody->addWidget(WW(WContainerWidget).css("comment-text").add(commentWidget));
    }
  });
  wApp->root()->addWidget(latestCommentsContainer);
  
  latestCommentsMenuItem->clicked().connect([latestCommentsMenuItem,latestCommentsContainer,this](WMouseEvent){
    string togglejs = (boost::format(JS( $('#%s').modal('toggle'); )) % latestCommentsContainer->id()).str();
    latestCommentsMenuItem->doJavaScript(togglejs);
  });
  activeUsersMenuItem = new WText("Users");
  
  auto setLoggedUsersTitle = [this](StreamingAppSession, _n5){
    activeUsersMenuItem->setText(WString("Users: {1}").arg(streamingAppSessions.sessionCount()));
  };
  
  topBarTemplate->bindWidget("latest.comments", latestCommentsMenuItem);
  topBarTemplate->bindWidget("users.count", activeUsersMenuItem);
  topBarTemplate->bindWidget("media.list", filesListMenuItem);
  
  sessionAdded.connect(setLoggedUsersTitle);
  sessionRemoved.connect(setLoggedUsersTitle);
  
  if(role == AuthorizedUser::Admin)
    setupAdminMenus();
  else {
    setupUserMenus();
  }
  
  
//   WMenuItem *refresh = topMenu->addItem("Refresh", 0);
  
  string serverStatusUrl; // TODO
  if(false || wApp->readConfigurationProperty("server-status-url", serverStatusUrl)) {
    WText *bwStatsItem = new WText();
    new ReadBWStats(bwStatsItem, serverStatusUrl, q);
  }
  
  WText *logout = new WText("Logout");
  topBarTemplate->bindWidget("logout", logout);
  
  logout->clicked().connect([this](WMouseEvent) {
    session.login().logout();
    wApp->redirect(wApp->bookmarkUrl("/")); 
  });
  filesListMenuItem->clicked().connect([this](WMouseEvent){
    if(widgetsStack->currentIndex()) {
      filesListMenuItem->setText(WString::tr("menu.videoslist"));
      widgetsStack->setCurrentIndex(0);
    } else {
      filesListMenuItem->setText(WString::tr("menu.back.to.video"));
      widgetsStack->setCurrentIndex(1);
    }
  });
  

  WLineEdit *searchBox = new WLineEdit();
  searchBox->setStyleClass("search-query");
  searchBox->setAttributeValue("placeholder", "Search");
  
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
  auto addSuggestions = [this,suggestions,searchBox](_n6) {
    for(pair<string,Media> media: collection->collection()) {
      suggestions->addSuggestion(media.second.filename(), media.first);
    }
    suggestions->forEdit(searchBox);
  };
  
  topBarTemplate->bindWidget("search", searchBox);
  
  collection->scanned().connect(addSuggestions);
  
  mainWidget->addWidget(topBarTemplate);
}

void StreamingAppPrivate::setupUserMenus()
{
  topBarTemplate->setCondition("is-user", true);
  topBarTemplate->setCondition("is-admin", false);
}


void StreamingAppPrivate::setupAdminMenus()
{
  topBarTemplate->setCondition("is-user", false);
  topBarTemplate->setCondition("is-admin", true);
  WText *allLog = new WText("Users Log");
  WText *addUserMenu = new WText("Add User");
  topBarTemplate->bindWidget("users.log", allLog);
  topBarTemplate->bindWidget("users.add", addUserMenu);
  
  const string *addUserParameter = wApp->environment().getParameter("add_user_email");
  
  auto displayAddUserDialog = [addUserParameter,this](WMouseEvent){
    string addUserEmail =  addUserParameter? *addUserParameter: string();
    AddUserDialog *dialog = new AddUserDialog(&session, addUserEmail);
    dialog->show();
  };
  
  if(addUserParameter)
    WTimer::singleShot(1000, displayAddUserDialog);

  activeUsersMenuItem->clicked().connect([this](WMouseEvent){
    WDialog *dialog = new LoggedUsersDialog(&session);
    dialog->show();
  });
  addUserMenu->clicked().connect([displayAddUserDialog,this](WMouseEvent){
      displayAddUserDialog(WMouseEvent());
  });
  allLog->clicked().connect([this](WMouseEvent){
      WDialog *dialog = new LoggedUsersDialog(&session, true);
      dialog->show();
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
  
  d->collection->rescan();
  MediaCollectionBrowser* browser = new MediaCollectionBrowser(d->collection, &d->settings);
  browser->play().connect([this](Media media, _n5){
    d->queueAndPlay(media.path());
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


string StreamingAppPrivate::extensionFor ( filesystem::path p ) {
  string extension = p.extension().string();
  boost::algorithm::to_lower(extension);
  return extension;
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


void StreamingAppPrivate::play ( Media media ) {
  filesListMenuItem->setText(WString::tr("menu.videoslist"));
  widgetsStack->setCurrentIndex(0);
  log("notice") << "Playing file " << media.path();
  if(player) {
    player->stop();
    delete player;
  }
  player = settings.newPlayer();
  
  player->addSource( Source(settings.linkFor( media.path() ).url(), media.mimetype()) );
  player->setAutoplay(settings.autoplay());
  fs::path preview = media.preview(&settings);
  if(fs::exists(preview)) {
    player->setPoster(settings.linkFor(preview));
  }
  for(fs::path subtitle: media.subtitles(&settings)) {
    // TODO: translation map, for instance ita=Italiano, eng=English etc
    string fileBareName = subtitle.filename().replace_extension().string();
    player->addSubtitles(Track( settings.linkFor(subtitle).url(), fileBareName, fileBareName ));
  }
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
  log("notice") << "using url " << settings.linkFor( media.path() ).url();
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
      player->addSubtitles(Track(settings.linkFor(mySub).url(), lang, name));
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

