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



#include "session.h"

#include "Wt/Auth/AuthService"
#include "Wt/Auth/HashFunction"
#include "Wt/Auth/PasswordService"
#include "Wt/Auth/PasswordStrengthValidator"
#include "Wt/Auth/PasswordVerifier"
#include "Wt/Auth/GoogleService"
#include "Wt/Auth/FacebookService"
#include "Wt/Auth/Dbo/AuthInfo"
#include "Wt/Auth/Dbo/UserDatabase"
#ifdef HAVE_POSTGRES
  #include <Wt/Dbo/backend/Postgres>
#endif
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/WApplication>
#include <Wt/WServer>
#include "private/session_p.h"

#include "Models/models.h"
#include "settings.h"
#include <boost/format.hpp>
#include "utils/d_ptr_implementation.h"
#define DATABASE_VERSION 1
#include "versions_compat.h"

namespace {
  class MyOAuth : public std::vector<const Wt::Auth::OAuthService *>
  {
  public:
    ~MyOAuth()
    {
      for (unsigned i = 0; i < size(); ++i)
    delete (*this)[i];
    }
  };

  Wt::Auth::AuthService myAuthService;
  Wt::Auth::PasswordService myPasswordService(myAuthService);
  MyOAuth myOAuthServices;
}


using namespace std;
using namespace Wt;

void Session::configureAuth()
{
  myOAuthServices.clear();
  myAuthService.setAuthTokensEnabled(true, "logincookie");
  bool emailVerificationMandatory = Settings::emailVerificationMandatory();
  myAuthService.setEmailVerificationEnabled(emailVerificationMandatory);
#ifdef WT_AUTH_NEWAPI
  myAuthService.setEmailVerificationRequired(emailVerificationMandatory);
#endif
  myAuthService.setIdentityPolicy(Wt::Auth::LoginNameIdentity);
  Wt::Auth::PasswordVerifier *verifier = new Wt::Auth::PasswordVerifier();
  verifier->addHashFunction(new Wt::Auth::BCryptHashFunction(7));
  myPasswordService.setVerifier(verifier);
  myPasswordService.setAttemptThrottlingEnabled(true);
  
  Auth::PasswordStrengthValidator *passwordValidator = new Auth::PasswordStrengthValidator();
  passwordValidator->setMandatory(true);
  passwordValidator->setMinimumLength(Auth::PasswordStrengthValidator::OneCharClass, 8);
  myPasswordService.setStrengthValidator(passwordValidator);
  
  if (Wt::Auth::GoogleService::configured())
    myOAuthServices.push_back(new Wt::Auth::GoogleService(myAuthService));

  if (Wt::Auth::FacebookService::configured())
    myOAuthServices.push_back(new Wt::Auth::FacebookService(myAuthService));
}


Session::Session(bool full)
  : d(this)
{
  d->createConnection();
  d->connection->setProperty("show-queries", "false");
  setConnection(*d->connection);
  mapClass<User>("user");
  mapClass<Group>("group");
  mapClass<GroupPath>("group_path");
  mapClass<AuthInfo>("auth_info");
  mapClass<Comment>("comment");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");
  mapClass<MediaProperties>("media_properties");
  mapClass<MediaAttachment>("media_attachment");
  mapClass<MediaRating>("media_rating");
  mapClass<Setting>("settings");
  mapClass<CollectionItemProperty>("collection_item_property");
  int db_version=Setting::value(Setting::DatabaseVersion, 0);
  WServer::instance()->log("notice") << "Found database version " << db_version;
  if(db_version == 0) {
    WServer::instance()->log("warning") << "error fetching database version";
    try {
      createTables();
    } catch(std::exception &e) {
//       WServer::instance()->log("warning") << "error creating new database: " << e.what();
    }
    Setting::write(Setting::DatabaseVersion, DATABASE_VERSION);
  }
  if(!full)
    return;
  d->users = new UserDatabase(*this);
#ifdef WT_AUTH_NEWAPI
  if(Settings::authenticationMode() == Settings::AuthenticateACL)
    d->users->setNewUserStatus(Auth::User::Disabled);
#endif
}

Wt::Dbo::SqlConnection *Session::connection() const
{
  return d->connection.get();
}


Auth::Login& Session::login()
{
  return d->login;
}


namespace {
  shared_ptr<mutex> sqlite3_write_lock_mutex = make_shared<mutex>();
}


string Session::Private::psqlConnectionString() const {
  string hostname = Settings::postgresqlHost();
  string dbName = Settings::postgresqlDatabase();
  string username = Settings::postgresqlUsername();
  string password = Settings::postgresqlPassword();
  int port = Settings::postgresqlPort();
  string applicationName = Settings::postgresqlApplication();
  if(password.empty() )
    return {};
  WServer::instance()->log("notice") << "Using postgresql connection: " <<
  (boost::format("application_name=%s host=%s port=%d dbname=%s user=%s")
    % applicationName
    % hostname
    % port
    % dbName
    % username
  ).str();

  return (boost::format("application_name=%s host=%s port=%d dbname=%s user=%s password=%s")
    % applicationName
    % hostname
    % port
    % dbName
    % username
    % password
  ).str();
}
void Session::Private::createConnection()
{
#ifdef HAVE_POSTGRES
  static string psqlConnParameters = psqlConnectionString();
  if(Settings::databaseType() == Settings::PostgreSQL && !psqlConnParameters.empty()) {
    connection.reset(new dbo::backend::Postgres(psqlConnParameters));
    return;
  }
#endif
  mutex = sqlite3_write_lock_mutex;
  string sqlite3DatabasePath = Settings::sqlite3DatabasePath("Pandorica.sqlite");
  WServer::instance()->log("notice") << "Using sqlite connection: " << sqlite3DatabasePath;
  
  connection.reset(new dbo::backend::Sqlite3(sqlite3DatabasePath ));
  connection->executeSql("PRAGMA journal_mode=WAL;");
}

class Session::WriteLock {
public:
  WriteLock(const shared_ptr<mutex> &mutex);
  ~WriteLock();
private:
  shared_ptr<unique_lock<mutex>> lock;
};

Session::WriteLock::WriteLock(const shared_ptr<std::mutex> &mutex)
{
  if(!mutex)
    return;
  lock = make_shared<unique_lock<std::mutex>>(*mutex);
}

Session::WriteLock::~WriteLock()
{
}


shared_ptr< Session::WriteLock > Session::writeLock() const
{
  return make_shared<WriteLock>(d->mutex);
}



Session::~Session()
{
  delete d->users;
}

UserDatabase& Session::users()
{
  return *d->users;
}



dbo::ptr<User> Session::user()
{
  if (!d->login.loggedIn())
    return dbo::ptr<User>();
  return user(d->login.user());
}

dbo::ptr<User> Session::user(const Wt::Auth::User &authUser)
{
  Dbo::Transaction t(*this);
  dbo::ptr<AuthInfo> authInfo = d->users->find(authUser);
  dbo::ptr<User> user = authInfo->user();
  dbo::ptr<User> invitedUser = find<User>().where("invited_email_address = ?").bind(authInfo->email().empty() ? authInfo->unverifiedEmail() : authInfo->email());
  
  if(!user) {
    user = invitedUser ? invitedUser : add(new User());
    user.modify()->invitedEmailAddress.reset();
    authInfo.modify()->setUser(user);
    authInfo.flush();
  }    
  return authInfo->user();
}

const Wt::Auth::AuthService& Session::auth()
{
  return myAuthService;
}

const Wt::Auth::PasswordService& Session::passwordAuth()
{
  return myPasswordService;
}

const std::vector<const Wt::Auth::OAuthService *>& Session::oAuth()
{
  return myOAuthServices;
}

