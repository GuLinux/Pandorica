/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "session.h"
#include "sessioninfo.h"
#include "sessiondetails.h"
#include "Wt/Auth/AuthService"
#include "Wt/Auth/HashFunction"
#include "Wt/Auth/PasswordService"
#include "Wt/Auth/PasswordStrengthValidator"
#include "Wt/Auth/PasswordVerifier"
#include "Wt/Auth/GoogleService"
#include "Wt/Auth/FacebookService"
#include "Wt/Auth/Dbo/AuthInfo"
#include "Wt/Auth/Dbo/UserDatabase"
#include <Wt/Dbo/backend/Postgres>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/WApplication>
#include <iostream>
#include <fstream>
#include "comment.h"
#include "mediaattachment.h"

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

class SessionPrivate {
public:
  void createConnection();
  dbo::SqlConnection *connection;
  UserDatabase *users;
  Wt::Auth::Login login;
};

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

Session::Session()
  : d(new SessionPrivate)
{
  d->createConnection();
  d->connection->setProperty("show-queries", "true");
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
  ofstream schema;
  schema.open("schema.sql");
  schema << tableCreationSql();
  schema.close();
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


void SessionPrivate::createConnection()
{
  string psqlConnParameters = "";
  wApp->readConfigurationProperty("psql-connection", psqlConnParameters);
  if(!psqlConnParameters.empty()) {
        connection = new dbo::backend::Postgres(psqlConnParameters);
    return;
  }
    connection = new dbo::backend::Sqlite3("videostreaming.sqlite");
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
