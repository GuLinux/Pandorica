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


#include "groupsdialog.h"
#include "Wt-Commons/wt_helpers.h"
#include "session.h"
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WCheckBox>
#include <Wt/WText>
#include <Wt/WTable>

#include "group.h"
#include "sessioninfo.h"
#include "user.h"
#include "sessiondetails.h"
#include "comment.h"

#include <Wt/Dbo/Dbo>
#include <Wt/WLabel>
#include <Wt/WTemplate>
#include <Wt/WMessageBox>

using namespace Wt;

GroupsDialog::GroupsDialog(Session *session): WDialog(), session(session)
{
  setTitleBarEnabled(true);
  setClosable(true);
  setResizable(true);
  setWindowTitle("Groups");
  
  WLineEdit *newGroupName = WW<WLineEdit>().css("input-medium").setMargin(10);
  WTemplate *adminLabel = new WTemplate("<label class=\"checkbox\">${check.admin}${check.admin.label}</label>");
  adminLabel->setInline(true);
  WCheckBox *isAdmin = WW<WCheckBox>();
  
  adminLabel->bindWidget("check.admin", isAdmin);
  adminLabel->bindString("check.admin.label", "Admin");
  WPushButton *addGroup = WW<WPushButton>("Add").css("btn").setMargin(10);
  
  auto enableAddGroupButton = [=] {
    addGroup->setEnabled(!newGroupName->text().empty());
  };
  enableAddGroupButton();
  
  currentGroups = WW<WTable>().css("table table-striped table-bordered table-hover");
  
  auto saveNewGroup = [=] {
    Dbo::Transaction t(*session);
    Group* group = new Group(newGroupName->text().toUTF8(), isAdmin->checkState() == Checked);
    session->add(group);
    t.commit();
    isAdmin->setCheckState(Wt::Unchecked);
    newGroupName->setText("");
    populateGroups();
  };
  
  newGroupName->keyWentUp().connect([=](WKeyEvent k) {
    enableAddGroupButton();
    if(k.key() == Wt::Key_Enter && addGroup->isEnabled() ) saveNewGroup();
  });
  addGroup->clicked().connect([=](WMouseEvent) { saveNewGroup(); });
  
  
  contents()->addWidget(
    WW<WContainerWidget>().css("form-inline").add(newGroupName).add(adminLabel).add(addGroup)
  );
  
  

  contents()->addWidget(currentGroups);
  populateGroups();
}

void GroupsDialog::populateGroups()
{
  currentGroups->clear();
  Dbo::Transaction t(*session);
  Dbo::collection<GroupPtr> groups = session->find<Group>();
  int row=0;
  for(GroupPtr group: groups) {
    currentGroups->elementAt(row, 0)->addWidget(WW<WText>(group->groupName()));
    currentGroups->elementAt(row, 0)->setContentAlignment(AlignMiddle);
    currentGroups->elementAt(row, 1)->addWidget(WW<WPushButton>("Users").css("btn btn-small btn-primary"));
    currentGroups->elementAt(row, 2)->addWidget(WW<WPushButton>("Paths").css("btn btn-small btn-info"));
    currentGroups->elementAt(row, 3)->addWidget(WW<WPushButton>("Remove").css("btn btn-small btn-danger").onClick([=](WMouseEvent) {
      if(WMessageBox::show(wtr("delete.group.title"), wtr("delete.group.text"), Yes | No) != Yes) return;
      Dbo::Transaction t(*session);
      GroupPtr groupToDelete = session->find<Group>().where("id = ?").bind(group.id());
//       groupToDelete.modify()->users.clear();
      groupToDelete.remove();
      t.commit();
      populateGroups();
    }));
    for(int i=1; i<4; i++) currentGroups->elementAt(row, i)->setStyleClass("span1");
    row++;
  }
}



