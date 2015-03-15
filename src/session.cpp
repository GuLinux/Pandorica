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
#ifdef HAVE_MYSQL
  #include <Wt/Dbo/backend/MySQL>
#endif
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/WApplication>
#include <Wt/WServer>
#include "private/session_p.h"

#include "Models/models.h"
#include "settings.h"

#include "utils/d_ptr_implementation.h"

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
  myAuthService.setAuthTokensEnabled(true, "logincookie");
  myAuthService.setEmailVerificationEnabled(true);
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
  : d()
{
  d->createConnection();
  d->connection->setProperty("show-queries", "false");
  setConnection(*d->connection);
  mapClass<User>("user");
  mapClass<Group>("group");
  mapClass<GroupPath>("group_path");
  mapClass<AuthInfo>("auth_info");
  mapClass<SessionInfo>("session_info");
  mapClass<SessionDetails>("session_details");
  mapClass<Comment>("comment");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");
  mapClass<MediaProperties>("media_properties");
  mapClass<MediaAttachment>("media_attachment");
  mapClass<MediaRating>("media_rating");
  mapClass<Setting>("settings");
  if(!full)
    return;
   
/*
  try {
    createTables();
    std::cerr << "Created database." << std::endl;
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "Using existing database";
  }
*/
  d->users = new UserDatabase(*this);
}

Wt::Dbo::SqlConnection *Session::connection() const
{
  return d->connection.get();
}


Auth::Login& Session::login()
{
  return d->login;
}

#ifdef HAVE_MYSQL
struct MySqlParams {
  bool isValid = false;
  string db;
  string dbUser;
  string dbPasswd;
  string dbHost = "localhost";
  int dbPort = 0;
  
  static MySqlParams readFromConfiguration() {
    bool isValid = true;
    string db, dbUser, dbPasswd, dbHost, dbPort;
    isValid &= WServer::instance()->readConfigurationProperty("mysql-db-name", db);
    isValid &= WServer::instance()->readConfigurationProperty("mysql-db-user", dbUser);
    isValid &= WServer::instance()->readConfigurationProperty("mysql-db-password", dbPasswd);
    MySqlParams params;
    params.isValid = isValid;
    params.db = db;
    params.dbUser = dbUser;
    params.dbPasswd = dbPasswd;
    if(WServer::instance()->readConfigurationProperty("mysql-db-hostname", dbHost)) {
      params.dbHost = dbHost;
    }
    if(WServer::instance()->readConfigurationProperty("mysql-db-port", dbPort)) {
      params.dbPort = boost::lexical_cast<int>(dbPort);
    }
    return params;
  }
};

#endif

void Session::Private::createConnection()
{
  string psqlConnParameters, mysqlConnParameters;
  bool havePostgresConfiguration = WServer::instance()->readConfigurationProperty("psql-connection", psqlConnParameters);
#ifdef HAVE_POSTGRES
  if(havePostgresConfiguration && !psqlConnParameters.empty()) {
    WServer::instance()->log("notice") << "Using postgresql connection";
    connection.reset(new dbo::backend::Postgres(psqlConnParameters));
    return;
  }
#endif
#ifdef HAVE_MYSQL
  MySqlParams mySqlParams = MySqlParams::readFromConfiguration();
  if(mySqlParams.isValid) {
    WServer::instance()->log("notice") << "Using mysql connection";
    connection.reset(new dbo::backend::MySQL(mySqlParams.db, mySqlParams.dbUser, mySqlParams.dbPasswd, mySqlParams.dbHost, mySqlParams.dbPort));
    return;
  }
#endif
  string sqlite3DatabasePath = Settings::sqlite3DatabasePath();
  WServer::instance()->log("notice") << "Using sqlite connection: " << sqlite3DatabasePath;
  
  connection.reset(new dbo::backend::Sqlite3(Settings::sqlite3DatabasePath()));
}


Session::~Session()
{
  delete d->users;
}

Wt::Auth::AbstractUserDatabase& Session::users()
{
  return *d->users;
}



dbo::ptr<User> Session::user()
{
  if (!d->login.loggedIn())
    return dbo::ptr<User>();
  dbo::ptr<AuthInfo> authInfo = d->users->find(d->login.user());
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

