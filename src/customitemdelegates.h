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


#ifndef ROLEITEMDELEGATE_H
#define ROLEITEMDELEGATE_H
#include <Wt/WItemDelegate>

namespace Wt {
  class WAbstractItemModel;
}

typedef std::function<std::string(std::string s)> StringTranformDelegateFunction;
class StringTransformDelegate : public Wt::WItemDelegate
{

public:
  StringTransformDelegate(StringTranformDelegateFunction function, Wt::WAbstractItemModel *model, Wt::WObject *parent = 0);
  virtual Wt::WWidget* update(Wt::WWidget* widget, const Wt::WModelIndex& index, Wt::WFlags< Wt::ViewItemRenderFlag > flags);
private:
  Wt::WAbstractItemModel *model;
  StringTranformDelegateFunction function;
};

class DateTimeDelegate : public Wt::WItemDelegate {
public:
    DateTimeDelegate(Wt::WAbstractItemModel *model, WObject* parent = 0) : Wt::WItemDelegate(parent), model(model) {}
    virtual Wt::WWidget* update(Wt::WWidget* widget, const Wt::WModelIndex& index, Wt::WFlags< Wt::ViewItemRenderFlag > flags);
private:
  Wt::WAbstractItemModel *model;
};


#endif // ROLEITEMDELEGATE_H
