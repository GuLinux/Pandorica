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
