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


#include "customitemdelegates.h"
#include "authorizeduser.h"
#include <Wt/WAbstractItemModel>
#include <Wt/WText>
#include <Wt/WDateTime>

using namespace Wt;
using namespace boost;
using namespace std;

RoleItemDelegate::RoleItemDelegate(Wt::WAbstractItemModel* model, Wt::WObject* parent): WItemDelegate(parent), model(model)
{
}


Wt::WWidget* RoleItemDelegate::update(Wt::WWidget* widget, const Wt::WModelIndex& index, Wt::WFlags< Wt::ViewItemRenderFlag > flags)
{
  int role = any_cast<int>(model->data(index));
  string roleLabel = "Unknown";
  if(role==AuthorizedUser::Admin)
    roleLabel = "Admin";
  if(role==AuthorizedUser::NormalUser)
    roleLabel = "User";
  if(!widget)
    return new WText(roleLabel);
  ((WText*)widget)->setText(roleLabel);
  return widget;
}



WWidget* DateTimeDelegate::update(WWidget* widget, const WModelIndex& index, WFlags< ViewItemRenderFlag > flags)
{
  long timeT = any_cast<long>(model->data(index));
  string label = timeT ? WDateTime::fromTime_t(timeT).toString("dd/M/yyyy HH:mm").toUTF8() : "Active Session";
  if(!widget) {
    WText* labelWidget = new WText(label);
    labelWidget->setStyleClass("small-text");
    return labelWidget;
  }
  ((WText*) widget)->setText(label);
  return widget;
}



