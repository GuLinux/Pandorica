/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

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
using namespace StreamingPrivate;

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
  : d(new SessionPrivate)
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

  try {
    createTables();
    std::cerr << "Created database." << std::endl;
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "Using existing database";
  }
  d->users = new UserDatabase(*this);
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

void SessionPrivate::createConnection()
{
  string psqlConnParameters, mysqlConnParameters;
  bool havePostgresConfiguration = WServer::instance()->readConfigurationProperty("psql-connection", psqlConnParameters);
#ifdef HAVE_POSTGRES
  if(havePostgresConfiguration && !psqlConnParameters.empty()) {
    WServer::instance()->log("notice") << "Using postgresql connection";
    connection = new dbo::backend::Postgres(psqlConnParameters);
    return;
  }
#endif
#ifdef HAVE_MYSQL
  MySqlParams mySqlParams = MySqlParams::readFromConfiguration();
  if(mySqlParams.isValid) {
    WServer::instance()->log("notice") << "Using mysql connection";
    connection = new dbo::backend::MySQL(mySqlParams.db, mySqlParams.dbUser, mySqlParams.dbPasswd, mySqlParams.dbHost, mySqlParams.dbPort);
    return;
  }
#endif
  connection = new dbo::backend::Sqlite3(Settings::sqlite3DatabasePath());
}


Session::~Session()
{
  delete d->users;
  delete d;
}

Wt::Auth::AbstractUserDatabase& Session::users()
{
  return *d->users;
}

dbo::ptr<User> Session::user()
{
  if (d->login.loggedIn()) {
    dbo::ptr<AuthInfo> authInfo = d->users->find(d->login.user());
    dbo::ptr<User> user = authInfo->user();
    if(!user) {
      user = add(new User());
      authInfo.modify()->setUser(user);
      authInfo.flush();
    }
    return authInfo->user();
  } else
    return dbo::ptr<User>();
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
