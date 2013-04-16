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
#include "settings.h"

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/QueryModel>
#include <Wt/WLabel>
#include <Wt/WTemplate>
#include <Wt/WMessageBox>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/WComboBox>
#include <Wt/WTreeView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <boost/filesystem.hpp>

using namespace Wt;
using namespace std;

class UsersInGroupDialog : public Wt::WDialog {
public:
  UsersInGroupDialog(Wt::Dbo::ptr<Group> group, Session *session);
private:
  Wt::Signal<> dataChanged;
};


class GroupDirectoriesDialog : public Wt::WDialog {
public:
  GroupDirectoriesDialog(Dbo::ptr< Group > group, Session* session, Settings* settings);
};

GroupsDialog::GroupsDialog(Session* session, Settings* settings): WDialog()
{
  setTitleBarEnabled(true);
  setResizable(true);
  setClosable(true);
  setWindowTitle(wtr("groups.dialog.title"));
  
  WLineEdit *newGroupName = WW<WLineEdit>().css("input-medium").setAttribute("placeholder", wtr("groups.new.placeholder").toUTF8()).setMargin(10);
  WTemplate *adminLabel = new WTemplate("<label class=\"checkbox\">${check.admin}${check.admin.label}</label>");
  adminLabel->setInline(true);
  WCheckBox *isAdmin = WW<WCheckBox>();
  
  adminLabel->bindWidget("check.admin", isAdmin);
  adminLabel->bindString("check.admin.label", wtr("group.is.admin"));
  WPushButton *addGroup = WW<WPushButton>(wtr("button.add")).css("btn btn-primary").setMargin(10);
  
  auto enableAddGroupButton = [=] {
    addGroup->setEnabled(!newGroupName->text().empty());
  };
  enableAddGroupButton();
  
  WTable *currentGroups = WW<WTable>().css("table table-striped table-bordered table-hover");
  
  auto saveNewGroup = [=] {
    Dbo::Transaction t(*session);
    Group* group = new Group(newGroupName->text().toUTF8(), isAdmin->checkState() == Checked);
    session->add(group);
    t.commit();
    isAdmin->setCheckState(Wt::Unchecked);
    newGroupName->setText("");
    dataChanged.emit();
  };
  
  newGroupName->keyWentUp().connect([=](WKeyEvent k) {
    enableAddGroupButton();
    if(k.key() == Wt::Key_Enter && addGroup->isEnabled() ) saveNewGroup();
  });
  addGroup->clicked().connect([=](WMouseEvent) { saveNewGroup(); });
  
  
  contents()->addWidget(
    WW<WContainerWidget>().css("form-inline").add(newGroupName).add(adminLabel).add(addGroup)
  );
  
  auto populateGroups = [=] {
    currentGroups->clear();
    Dbo::Transaction t(*session);
    Dbo::collection<GroupPtr> groups = session->find<Group>();
    int row{0};
    for(GroupPtr group: groups) {
      currentGroups->elementAt(row, 0)->addWidget(WW<WText>(group->groupName()));
      currentGroups->elementAt(row, 0)->setContentAlignment(AlignMiddle);
      currentGroups->elementAt(row, 1)->addWidget(WW<WPushButton>(wtr("groups.users")).css("btn btn-small btn-primary").onClick([=](WMouseEvent) {
        (new UsersInGroupDialog{group, session})->show(); 
      }));
      currentGroups->elementAt(row, 2)->addWidget(WW<WPushButton>(wtr("groups.paths")).css("btn btn-small btn-info").onClick([=](WMouseEvent) {
        (new GroupDirectoriesDialog{group, session, settings})->show();
      }));
      currentGroups->elementAt(row, 3)->addWidget(WW<WPushButton>(wtr("button.remove")).css("btn btn-small btn-danger").onClick([=](WMouseEvent) {
        if(WMessageBox::show(wtr("delete.group.title"), wtr("delete.group.text").arg(group->groupName()), Yes | No) != Yes) return;
        Dbo::Transaction t(*session);
        GroupPtr groupToDelete = session->find<Group>().where("id = ?").bind(group.id());
        for(UserPtr user: groupToDelete->users)
          groupToDelete.modify()->users.erase(user);
        groupToDelete.flush();
        groupToDelete.remove();
        t.commit();
        dataChanged.emit();
      }));
      for(int i=1; i<4; i++) currentGroups->elementAt(row, i)->setStyleClass("span1");
      row++;
    }
  };
  
  dataChanged.connect([=](_n6) {
    populateGroups();
  });

  contents()->addWidget(currentGroups);
  populateGroups();
}


#define SQL(...) #__VA_ARGS__


typedef boost::tuple<string,long> UserEntry;

UsersInGroupDialog::UsersInGroupDialog(GroupPtr group, Session* session): WDialog()
{
  setTitleBarEnabled(true);
  setClosable(true);
  setResizable(true);
  setWindowTitle(wtr("users.in.group.title").arg(group->groupName()));
  WTable *usersTable = WW<WTable>().css("table table-striped table-bordered table-hover");
  
  WPushButton *addButton = WW<WPushButton>(wtr("button.add")).css("btn btn-small btn-primary").setMargin(10, Side::Left);
  WComboBox *usersSelect = WW<WComboBox>().css("span4");
  auto enableAddbutton = [=] {
    addButton->setEnabled(usersSelect->count());
  };
  
  auto query = session->query<UserEntry>(
    SQL(SELECT identity || ' (' || email || ')' as display_name, "user".id from "user"
    INNER JOIN auth_info ON "user".id = "auth_info".user_id
    INNER JOIN auth_identity ON auth_identity.auth_info_id = auth_info.id
    WHERE auth_identity.provider = 'loginname'
    AND "user".id NOT IN (select user_id from groups_users WHERE group_id = ? )
  )).bind(group.id());
  
  auto model = new Dbo::QueryModel<UserEntry>(this);
  model->setQuery(query);
  model->addAllFieldsAsColumns();
  usersSelect->setModel(model);
  contents()->addWidget(
    WW<WContainerWidget>().css("form-inline").setMargin(10).add(usersSelect).add(addButton)
  );
  
  addButton->clicked().connect([=](WMouseEvent) {
    Dbo::Transaction t(*session);
    long userId = boost::any_cast<long>(model->data(usersSelect->currentIndex(), 1));
    UserPtr user = session->find<User>().where("id = ?").bind(userId);
    group.modify()->users.insert(user);
    t.commit();
    dataChanged.emit();
  });
  
  auto populateUsers = [=] {
    usersTable->clear();
    Dbo::Transaction t(*session);
    int row{0};
    for(UserPtr user: group->users) {
      Dbo::ptr<AuthInfo> authInfo = session->find<AuthInfo>().where("user_id = ?").bind(user.id());
      usersTable->elementAt(row, 0)->addWidget(new WText{authInfo->identity("loginname")});
      usersTable->elementAt(row, 1)->addWidget(new WText{authInfo->email()});
      usersTable->elementAt(row, 2)->addWidget(WW<WPushButton>(wtr("button.remove")).css("btn btn-danger").onClick([=](WMouseEvent) {
        Dbo::Transaction t(*session);
        if(WMessageBox::show(wtr("delete.user.title"), wtr("delete.user.text")
          .arg(authInfo->identity("loginname")).arg(group->groupName()), Yes | No) != Yes) return;
        group.modify()->users.erase(user);
        t.commit();
        dataChanged.emit();
      }));
      usersTable->elementAt(row, 2)->addStyleClass("span1");
      row++;
    }
  };
  
  dataChanged.connect([=](_n6) {
    model->reload();
    populateUsers();
  });
  enableAddbutton();
  populateUsers();
  contents()->addWidget(usersTable);
}

using namespace boost::filesystem;
GroupDirectoriesDialog::GroupDirectoriesDialog(Dbo::ptr< Group > group, Session* session, Settings *settings): WDialog()
{
  setTitleBarEnabled(true);
  setResizable(true);
  setClosable(true);
  setWindowTitle(wtr("directories.for.group.dialog.title").arg(group->groupName()));
  WTreeView *tree = new WTreeView();
  setHeight(400);
  WStandardItemModel *model = new WStandardItemModel(this);
  tree->setMinimumSize(400, WLength::Auto);
  tree->setModel(model);
  tree->setHeight(320);
  contents()->addWidget(tree);
  tree->setRootIsDecorated(false);
  
  tree->doubleClicked().connect([=](WModelIndex index, WMouseEvent, _n4){
    tree->setExpanded(index, !tree->isExpanded(index));
  });
    
  auto folderItem = [=] (path p) {
    Dbo::Transaction t(*session);
    string folderName{p.filename().string()};
    WStandardItem* item = new WStandardItem{Settings::icon(Settings::FolderSmall), folderName};
    item->setCheckable(true);
    item->setStyleClass("tree-directory-item link-hand");
    item->setLink("");
    item->setToolTip(wtr("tree.double.click.to.expand"));
    for(Dbo::ptr<GroupPath> groupPath: group->groupPaths)
      if(groupPath->path() == p.string())
        item->setChecked(true);
    item->setData(p);
    return item;
  };
  
  auto populateTree = [=] {
    model->clear();
    path videosDir{settings->videosDir()};
    map<path, WStandardItem*> items{
      {videosDir, folderItem(videosDir)}
    };
    model->appendRow(items[videosDir]);
    recursive_directory_iterator it{videosDir, symlink_option::recurse};
    while(it != recursive_directory_iterator() ) {
      if(is_directory(*it)) {
        WStandardItem *item = folderItem(*it);
        items[it->path()] = item;
        items[it->path().parent_path()]->appendRow(item);
      }
      it++;
    }
    tree->expandToDepth(1);
  };
  model->itemChanged().connect([=](WStandardItem *item, _n5) {
    Dbo::Transaction t(*session);
    bool itemChecked = (item->checkState() == Wt::Checked);
    string itemPath = boost::any_cast<path>(item->data()).string();
    if(itemChecked) {
      group.modify()->groupPaths.insert(new GroupPath{itemPath});
    } else {
      for(Dbo::ptr<GroupPath> groupPath: group->groupPaths) {
        if(groupPath->path() == itemPath) {
          groupPath.remove();
        }
      }
    }
    t.commit();
  });
  
  populateTree();
}



