/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#ifndef GOOGLEPICKER_H
#define GOOGLEPICKER_H

#include <Wt/WCompositeWidget>

class GooglePicker : public Wt::WCompositeWidget
{
public:
    ~GooglePicker();
    GooglePicker(const Wt::WString &buttonText, Wt::WContainerWidget* parent = 0);
    Wt::JSignal<Wt::WString> &imageChosen();
    void pick();
    GooglePicker *searchString(const Wt::WString &search) { _searchString = search; }
private:
  Wt::JSignal<Wt::WString> _imageChosen;
  Wt::WString _searchString;
  Wt::WPushButton *_button;
  std::string _developerKey;
};

#endif // GOOGLEPICKER_H
