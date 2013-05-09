/*
 * Copyright 2013 Marco Gulino <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "utils.h"
#include "private/utils_p.h"
#include <fstream>
#include <Wt/Json/Parser>
#include <Wt/Json/Object>
#include <Wt/Json/Array>
#include <Wt/WServer>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <Wt/Mail/Client>
#include <Wt/Mail/Mailbox>
#include <Wt/Mail/Message>
using namespace std;
using namespace Wt;

using namespace StreamingPrivate;

UtilsPrivate::UtilsPrivate(Utils* q) : q(q)
{
}
UtilsPrivate::~UtilsPrivate()
{
}

Utils::Utils()
    : d(new UtilsPrivate(this))
{
}

Utils::~Utils()
{
    delete d;
}

void Utils::mailForNewAdmin(string email, WString identity)
{
  Mail::Client client;
  Mail::Message message;
  message.setFrom(UtilsPrivate::authMailbox());
  message.setSubject(WString::tr("new_admin_subject"));
  message.setBody(WString::tr("new_admin_body").arg(identity).arg(email));
  message.addRecipient(Mail::To, UtilsPrivate::adminMailbox());
  client.connect();
  client.send(message);
}

void Utils::mailForUnauthorizedUser(string email, WString identity)
{
  Mail::Client client;
  Mail::Message message;
  message.setFrom(UtilsPrivate::authMailbox());
  message.setSubject(WString::tr("unauthorized_user_login_subject"));
  message.setBody(WString::tr("unauthorized_user_login_body").arg(identity).arg(email).arg(wApp->makeAbsoluteUrl(wApp->bookmarkUrl("/"))));
  message.addRecipient(Mail::To, UtilsPrivate::adminMailbox());
  client.connect();
  client.send(message);
}


Mail::Mailbox UtilsPrivate::adminMailbox()
{
  return mailboxFor("admin-mail-name", "admin-mail-address", {"admin@localhost"});
}

Mail::Mailbox UtilsPrivate::authMailbox()
{
  return mailboxFor("auth-mail-sender-name", "auth-mail-sender-address", {"noreply@localhost"});
}


Mail::Mailbox UtilsPrivate::mailboxFor(string nameProperty, string addressProperty, Mail::Mailbox defaultMailbox)
{
  string name, address;
  WServer::instance()->readConfigurationProperty(nameProperty, name);
  WServer::instance()->readConfigurationProperty(addressProperty, address);
  if(name.empty() || address.empty()) {
    Wt::log("warn") << "mailbox properties " << nameProperty << " or " << addressProperty << " are not configured correctly.";
    Wt::log("warn") << "Please check your configuration file. returning a default address.";
    return defaultMailbox;
  }
  return {address, name};
}


std::string Utils::titleHintFromFilename(std::string filename)
{
  for(FindAndReplace hint: FindAndReplace::from("title_from_filename_replacements.json")) {
    try {
      filename = boost::regex_replace(filename, boost::regex{hint.regexToFind, boost::regex::icase}, hint.replacement);
    } catch(runtime_error e) {
      WServer::instance()->log("notice") << "exception parsing regex '" << hint.regexToFind << "': " << e.what();
    }
  }
  while(filename.find("  ") != string::npos)
    boost::replace_all(filename, "  ", " ");
  boost::algorithm::trim(filename);
  return filename;
}


vector< FindAndReplace > FindAndReplace::from(string filename)
{
  
  ifstream subfile(filename);
  if(!subfile.is_open()) {
    WServer::instance()->log("notice") << "JSON Find/Replacement file " << filename << " missing, returning empty array";
    return {};
  }
  stringstream json;
  vector<FindAndReplace> parsedVector;
  json << subfile.rdbuf();
  subfile.close();
  try {
    Json::Value parsed;
    Json::parse(json.str(), parsed);
    Json::Array parsedArray = parsed.orIfNull(Json::Array{});
    for(Json::Object value: parsedArray) {
      parsedVector.push_back({value.get("regex_to_find").toString(), value.get("replacement").toString() });
    }
    return parsedVector;
  } catch(Json::ParseError error) {
    WServer::instance()->log("notice") << "Error parsing " << filename << ": " << error.what();
    return {};
  }
}