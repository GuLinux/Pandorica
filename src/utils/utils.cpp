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




#include "utils/utils.h"

#include "private/utils_p.h"
#include "settings.h"
#include <fstream>
#include <Wt/Json/Parser>
#include <Wt/Json/Object>
#include <Wt/Json/Array>
#include <Wt/WServer>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <Wt/Mail/Client>
#include <Wt/Mail/Mailbox>
#include <Wt/Mail/Message>
#include <Wt/WImage>
#include <Wt/WTimer>
#include "Wt-Commons/wt_helpers.h"
#include "utils/d_ptr_implementation.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;

Utils::Private::Private( Utils *q ) : q( q )
{
}

Utils::Utils()
  : d( this )
{
}

Utils::~Utils()
{
}



void Utils::mailForNewAdmin( string email, WString identity )
{
  Mail::Client client;
  Mail::Message message;
  message.setFrom( Utils::Private::authMailbox() );
  message.setSubject( WString::tr( "new_admin_subject" ) );
  message.setBody( WString::tr( "new_admin_body" ).arg( identity ).arg( email ) );
  message.addRecipient( Mail::To, Utils::Private::adminMailbox() );
  client.connect();
  client.send( message );
}

void Utils::mailForUnauthorizedUser( string email, WString identity )
{
  Mail::Client client;
  Mail::Message message;
  message.setFrom( Utils::Private::authMailbox() );
  message.setSubject( WString::tr( "unauthorized_user_login_subject" ) );
  message.setBody( WString::tr( "unauthorized_user_login_body" ).arg( identity ).arg( email ).arg( wApp->makeAbsoluteUrl( wApp->bookmarkUrl( "/" ) ) ) );
  message.addRecipient( Mail::To, Utils::Private::adminMailbox() );
  client.connect();
  client.send( message );
}

void Utils::inviteUserEmail( string email )
{
  Mail::Client client;
  Mail::Message message;
  message.setFrom( Utils::Private::authMailbox() );
  message.setSubject( WString::tr( "invite_user_subject" ) );
  message.setBody( WString::tr( "invite_user_body" ).arg( wApp->makeAbsoluteUrl( wApp->bookmarkUrl( "/" ) ) ) );
  message.addHtmlBody( WString::tr( "invite_user_body_html" ).arg( wApp->makeAbsoluteUrl( wApp->bookmarkUrl( "/" ) ) ) );
  message.addRecipient( Mail::To, email );
  client.connect();
  client.send( message );
}


Mail::Mailbox Utils::Private::adminMailbox()
{
  return mailboxFor( "admin-mail-name", "admin-mail-address", {"admin@localhost"} );
}

Mail::Mailbox Utils::Private::authMailbox()
{
  return mailboxFor( "auth-mail-sender-name", "auth-mail-sender-address", {"noreply@localhost"} );
}


Mail::Mailbox Utils::Private::mailboxFor( string nameProperty, string addressProperty, Mail::Mailbox defaultMailbox )
{
  string name, address;
  WServer::instance()->readConfigurationProperty( nameProperty, name );
  WServer::instance()->readConfigurationProperty( addressProperty, address );

  if( name.empty() || address.empty() )
  {
    Wt::log( "warn" ) << "mailbox properties " << nameProperty << " or " << addressProperty << " are not configured correctly.";
    Wt::log( "warn" ) << "Please check your configuration file. returning a default address.";
    return defaultMailbox;
  }

  return {address, name};
}

WInteractWidget *Utils::help( string titleKey, string contentKey, string side, WLength size )
{
  WImage *image = WW<WImage>( Settings::staticPath( "/icons/help.png" ) ).css( "link-hand" )
                  .setAttribute( "data-toggle", "popover" )
                  .setAttribute( "data-placement", side )
                  .setAttribute( "data-html", "true" )
                  .setAttribute( "data-trigger", "manual" )
                  ;

  image->doJavaScript(
    ( boost::format( "$('#%s').popover({ title: %s, content: %s, }); " ) % image->id()
      % wtr( titleKey ).jsStringLiteral()
      % wtr( contentKey ).jsStringLiteral()
    ).str()
  );
  image->clicked().connect( [ = ]( WMouseEvent )
  {
    image->doJavaScript( ( boost::format( JS(
                                            var myself = '#%s';
                                            var shouldShow = ! $( myself ).hasClass( 'help-popover-shown' );
                                            $( '.help-popover-shown' ).popover( 'hide' );
                                            $( '.help-popover-shown' ).removeClass( 'help-popover-shown' );

                                            if( !shouldShow ) return;
                                            setTimeout( function()
  {
    $( myself )
      .addClass( 'help-popover-shown' );
      $( myself ).popover( 'show' );
    }, 100 );
                                          ) )
                           % image->id()
                         ).str() );
  } );
  image->resize( size, size );
  return image;
}


std::string Utils::titleHintFromFilename( std::string filename )
{
  for( FindAndReplace hint : FindAndReplace::from( Settings::sharedFilesDir( "/title_from_filename_replacements.json" ) ) )
  {
    try
    {
      filename = boost::regex_replace( filename, boost::regex {hint.regexToFind, boost::regex::icase}, hint.replacement );
    }
    catch
      ( runtime_error e )
    {
      WServer::instance()->log( "notice" ) << "exception parsing regex '" << hint.regexToFind << "': " << e.what();
    }
  }

  while( filename.find( "  " ) != string::npos )
    boost::replace_all( filename, "  ", " " );

  boost::algorithm::trim( filename );
  return filename;
}

string Utils::formatFileSize( long size )
{
  int maxSizeForUnit = 900;
  vector<string> units {"bytes", "KB", "MB", "GB"};
  double unitSize = size;

  for( string unit : units )
  {
    if( unitSize < maxSizeForUnit || unit == units.back() )
    {
      return ( boost::format( "%.2f %s" ) % unitSize % unit ).str();
    }

    unitSize /= 1024;
  }
}

vector< FindAndReplace > FindAndReplace::from( string filename )
{

  ifstream subfile( filename );

  if( !subfile.is_open() )
  {
    WServer::instance()->log( "notice" ) << "JSON Find/Replacement file " << filename << " missing, returning empty array";
    return {};
  }

  stringstream json;
  vector<FindAndReplace> parsedVector;
  json << subfile.rdbuf();
  subfile.close();

  try
  {
    Json::Value parsed;
    Json::parse( json.str(), parsed );
    Json::Array parsedArray = parsed.orIfNull( Json::Array {} );

    for( Json::Object value : parsedArray )
    {
      parsedVector.push_back( {value.get( "regex_to_find" ).toString(), value.get( "replacement" ).toString() } );
    }

    return parsedVector;
  }
  catch
    ( Json::ParseError error )
  {
    WServer::instance()->log( "notice" ) << "Error parsing " << filename << ": " << error.what();
    return {};
  }
}
