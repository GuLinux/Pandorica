/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Marco Gulino <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "usersmanagementpage.h"
#include "private/usersmanagementpage_p.h"
#include "utils/d_ptr_implementation.h"
#include "session.h"
#include <Wt/WDialog>
#include <Wt/WText>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/WTable>
#include <Wt/WComboBox>
#include <Wt/WPanel>
#include <Wt/WPushButton>
#include <Wt/WPopupMenu>
#include "Models/models.h"
#include "Wt-Commons/wt_helpers.h"

using namespace Wt;
using namespace std;
using namespace WtCommons;

UsersManagementPage::Private::Private( UsersManagementPage *q, Session *session ) : session( session ), q( q )
{
}

UsersManagementPage::Private::~Private()
{
}

UsersManagementPage::~UsersManagementPage( )
{
}

UsersManagementPage::UsersManagementPage( Session *session, Wt::WContainerWidget *parent )
  : d( this, session )
{
  addWidget( d->usersContainer = new WTable );
  d->populate();
}


void UsersManagementPage::Private::populate()
{
  usersContainer->clear();
  usersContainer->setHeaderCount(1);
  auto header = usersContainer->rowAt(0);
  header->elementAt(0)->addWidget(new WText("Username"));
  header->elementAt(1)->addWidget(new WText("Email"));
  header->elementAt(2)->addWidget(new WText("Groups"));
  Dbo::Transaction t( *session );
  auto users = session->find<AuthInfo>().resultList();
  auto groups = session->find<Group>().resultList();

  for( auto user : users )
  {
    addUserRow( user , groups );
  }
}



void UsersManagementPage::Private::addUserRow( const Dbo::ptr< AuthInfo > &user, const Wt::Dbo::collection<Wt::Dbo::ptr<Group>> &groupsList )
{
  WTableRow *row = usersContainer->insertRow( usersContainer->rowCount() );
  auto username = user->identity( "loginname" );
  row->elementAt( 0 )->addWidget( new WText( username ) );
  row->elementAt( 1 )->addWidget( new WText( user->email() ) );
  WPushButton *groupsButton = WW<WPushButton>();
  groupsButton->setMenu( new WPopupMenu );
  row->elementAt( 2 )->addWidget( groupsButton );

  WContainerWidget *groupsPanel = new WContainerWidget;

  for( auto group : groupsList )
  {
    auto groupItem = groupsButton->menu()->addItem( group->groupName() );
    groupItem->setCheckable( true );
    groupItem->setChecked( user->user()->groups.count( group ) > 0 );
    groupItem->triggered().connect( [ = ]( WMenuItem *, _n5 )
    {
      Dbo::Transaction t( *session );

      if( groupItem->isChecked() )
      {
        user->user().modify()->groups.insert( group );
      }
      else
      {
        user->user().modify()->groups.erase( group );
      }
    } );
  }
}


void UsersManagementPage::dialog( Session *session )
{
  WDialog *dialog = new WDialog;
  dialog->contents()->addWidget( new UsersManagementPage( session ) );
  dialog->setClosable( true );
  dialog->show();
}
