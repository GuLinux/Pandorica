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




#include "customitemdelegates.h"
#include <Wt/WAbstractItemModel>
#include <Wt/WText>
#include <Wt/WDateTime>

using namespace Wt;
using namespace boost;
using namespace std;

StringTransformDelegate::StringTransformDelegate(StringTranformDelegateFunction function, Wt::WAbstractItemModel* model, Wt::WObject* parent)
  : WItemDelegate(parent), function(function), model(model)
{
}


Wt::WWidget* StringTransformDelegate::update(Wt::WWidget* widget, const Wt::WModelIndex& index, Wt::WFlags< Wt::ViewItemRenderFlag > flags)
{
  string value = any_cast<string>(model->data(index));
  if(!widget)
    return new WText(function(value));
  ((WText*)widget)->setText(function(value));
  return widget;
}




WWidget* DateTimeDelegate::update(WWidget* widget, const WModelIndex& index, WFlags< ViewItemRenderFlag > flags)
{
  long timeT = any_cast<long>(model->data(index));
  string label = timeT ? WDateTime::fromTime_t(timeT).toString("dd/M/yyyy HH:mm").toUTF8() : "-";
  if(!widget) {
    WText* labelWidget = new WText(label);
    labelWidget->setStyleClass("small-text");
    return labelWidget;
  }
  ((WText*) widget)->setText(label);
  return widget;
}



