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
#include "utils/utils.h"
#include "session.h"
#include <Wt/WDialog>
#include <Wt/WText>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/WTable>
#include <Wt/WComboBox>
#include <Wt/WPanel>
#include <Wt/WPushButton>
#include <Wt/WPopupMenu>
#include <Wt/WMessageBox>
#include <Wt/WLineEdit>
#include <Wt/WCheckBox>
#include <Wt/WGroupBox>
#include <Wt/WGroupBox>
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
  WLineEdit *inviteEmailAddress = WW<WLineEdit>().setMargin(5);
  inviteEmailAddress->setPlaceholderText(wtr("usersmanagement_invite_invite_email_address"));
  Dbo::Transaction t(*session);
  auto inviteOnGroups = make_shared<vector<Dbo::ptr<Group>>>();
  WPushButton *groupsButton = d->groupsButton(t,
                                [=](const Dbo::ptr<Group> &group){ return false; },
                                [=](const Dbo::ptr<Group> &group, Dbo::Transaction &t){ inviteOnGroups->push_back(group); },
                                [=](const Dbo::ptr<Group> &group, Dbo::Transaction &t){ inviteOnGroups->erase(remove(begin(*inviteOnGroups), end(*inviteOnGroups), group));  }
                                );
  groupsButton->setMargin(5);
  WCheckBox *sendEmailCheckbox = WW<WCheckBox>(wtr("usersmanagement_invite_send_email")).setMargin(5);
  sendEmailCheckbox->setCheckState(Wt::Checked);    
  WPushButton *inviteButton = WW<WPushButton>(wtr("usersmanagement_invite_invite")).setMargin(5).css("btn btn-primary").onClick([=](WMouseEvent){
  // TODO: email validation;
    d->invite(inviteEmailAddress->text().toUTF8(), *inviteOnGroups);
    if(sendEmailCheckbox->checkState() == Wt::Checked)
      Utils::inviteUserEmail(inviteEmailAddress->text().toUTF8());
    for(auto menuItem: groupsButton->menu()->items())
      menuItem->setChecked(false);
    inviteOnGroups->clear();
    inviteEmailAddress->setText("");
    d->populate();
  });

  addWidget(WW<WGroupBox>(wtr("usersmanagement_invite_an_user")).css("fieldset-small").add(WW<WContainerWidget>().css("form-inline").add(inviteEmailAddress).add(groupsButton).add(inviteButton).add(sendEmailCheckbox)));
  addWidget(WW<WGroupBox>(wtr("usersmanagement_invited_users")).css("fieldset-small").add(d->invitedUsersContainer = WW<WTable>().css("table table-striped table-bordered table-hover") ));
  addWidget( WW<WGroupBox>(wtr("usersmanagement_registered_users")).css("fieldset-small").add(d->usersContainer = WW<WTable>().css("table table-striped table-bordered table-hover") ));
  d->populate();
}

void UsersManagementPage::Private::invite(std::string email, const std::vector<Wt::Dbo::ptr<Group>> &groups)
{
  Dbo::Transaction t(*session);
  long existingUsers = session->query<long>("SELECT COUNT(*) FROM auth_info WHERE email = ? OR unverified_email = ?");
  if(existingUsers > 0) {
    cerr << "User already existing: " << email << endl;
    // TODO: gui message
    return;
  }
  existingUsers = session->query<long>("SELECT COUNT(*) FROM user WHERE invited_email_address = ?");
  if(existingUsers > 0) {
    cerr << "User already invited: " << email << endl;
    // TODO: gui message
    return;
  }
  auto newUser = session->add(new User);
  for(auto group: groups)
    newUser.modify()->groups.insert(group);
  newUser.modify()->invitedEmailAddress = email;
}

void UsersManagementPage::Private::populate()
{
  Dbo::Transaction t( *session );
  {
    invitedUsersContainer->clear();
    invitedUsersContainer->setHeaderCount(1);
    auto header = usersContainer->rowAt(0);
    header->elementAt(0)->hide();
    header->elementAt(1)->addWidget(new WText("Email"));
    auto users = session->find<User>().where("invited_email_address IS NOT NULL").resultList();
    Dbo::ptr<AuthInfo> noAuthInfo;
    for(auto user: users) {
      addUserRow(noAuthInfo, user, invitedUsersContainer, t);
    }
  }
  usersContainer->clear();
  usersContainer->setHeaderCount(1);
  auto header = usersContainer->rowAt(0);
  header->elementAt(0)->addWidget(new WText(wtr("usersmanagement_username")));
  header->elementAt(1)->addWidget(new WText("Email"));
  auto users = session->find<AuthInfo>().resultList();

  for( auto authInfo : users )
  {
    if(!authInfo->user())
      continue;
    auto user = authInfo->user();
    addUserRow( authInfo, user, usersContainer , t );
  }
}



void UsersManagementPage::Private::addUserRow( Dbo::ptr< AuthInfo > &authInfo, Dbo::ptr<User> &user, WTable *table, Dbo::Transaction &transaction )
{
  WTableRow *row = table->insertRow( table->rowCount() );
  WString username = authInfo ? authInfo->identity( "loginname" ) : WString("");
  auto userEmail = authInfo ? authInfo->email() : user->invitedEmailAddress.get();
  if( username.empty() )
    row->elementAt( 0 )->hide();
  else
    row->elementAt( 0 )->addWidget( new WText( username ) );
  row->elementAt( 1 )->addWidget( new WText( userEmail ) );
  row->elementAt( 2 )->addWidget( groupsButton(transaction,
                                [=](const Dbo::ptr<Group> &group){ return user->groups.count( group ) > 0; },
                                [=](const Dbo::ptr<Group> &group, Dbo::Transaction &t){ 
    user.modify()->groups.insert( group ); },
                                [=](const Dbo::ptr<Group> &group, Dbo::Transaction &t){ 
    user.modify()->groups.erase( group ); }
                                ));
  row->elementAt( 3 )->addWidget( WW<WPushButton>(wtr("button.remove")).css("btn btn-danger").onClick([=](WMouseEvent) {
    auto confirmation = WMessageBox::show(wtr("usersmanagement_remove_user"),
                                          wtr("usersmanagement_remove_user_confirm_text")
                                          .arg(username).arg(userEmail), StandardButton::Ok | StandardButton::Cancel );
    if(confirmation != StandardButton::Ok)
      return;
    Dbo::Transaction t(*session);
    auto user_id = user.id();
    session->execute("DELETE FROM \"groups_users\" WHERE user_id = ?").bind(user_id);
    session->execute("DELETE FROM \"comment\" WHERE user_id = ?").bind(user_id);
    session->execute("DELETE FROM \"media_rating\" WHERE user_id = ?").bind(user_id);
    for(Dbo::ptr<SessionInfo> sessionInfo: session->find<SessionInfo>().where("user_id = ?").bind(user_id).resultList() )
      session->execute("DELETE FROM \"session_details\" WHERE session_info_session_id = ?").bind(sessionInfo.id());
    session->execute("DELETE FROM \"session_info\" WHERE user_id = ?").bind(user_id);
    session->execute("DELETE FROM \"user\" WHERE id = ?").bind(user_id);
    if(authInfo) {
      session->execute("DELETE FROM \"auth_token\" WHERE auth_info_id = ?").bind( authInfo.id());
      session->execute("DELETE FROM \"auth_identity\" WHERE auth_info_id = ?").bind( authInfo.id());
      session->execute("DELETE FROM \"auth_info\" WHERE id = ?").bind( authInfo.id());
    }
    populate();
  }));
}


WPushButton *UsersManagementPage::Private::groupsButton( Dbo::Transaction &transaction, GroupSelection groupSelection, GroupModTrigger onGroupChecked, UsersManagementPage::Private::GroupModTrigger onGroupUnchecked )
{
  WPushButton *groupsButton = WW<WPushButton>(wtr("menu.groups")).css("btn");
  groupsButton->setMenu( new WPopupMenu );
  for( auto group: transaction.session().find<Group>().resultList())
  {
    auto groupItem = groupsButton->menu()->addItem( group->groupName() );
    groupItem->setCheckable( true );
    groupItem->setChecked( groupSelection(group) );
    groupItem->triggered().connect( [ = ]( WMenuItem *, _n5 )
    {
      Dbo::Transaction t( *session );

      if( groupItem->isChecked() )
        onGroupChecked(group, t);
      else
        onGroupUnchecked(group, t);
    } );
  }
  return groupsButton;
}


void UsersManagementPage::dialog( Session *session )
{
  WDialog *dialog = new WDialog;
  dialog->setCaption(wtr("menu.usersmanagement"));
  dialog->contents()->addWidget( new UsersManagementPage( session ) );
  dialog->setClosable( true );
  dialog->show();
}
