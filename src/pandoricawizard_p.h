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

#ifndef PRIVATE_H
#define PRIVATE_H
#include "pandoricawizard.h"
#include <Wt/WStackedWidget>
#include <map>
#include <functional>

class Group;
class PandoricaWizard::Private
{
public:
    Private(PandoricaWizard* q);
    virtual ~Private();
    void addPandoricaModePage();
    void addFileSystemChooser();
    void addDatabaseSetup();
    enum Page { None, PandoricaModePage, FileSystemChooserPage, DatabaseSetup };
    Wt::WStackedWidget *stack;
    std::map<Page, std::function<void()>> showPage;
    Page previousPage;
    Page nextPage;
    Wt::WPushButton *previous, *next, *finish;
    Wt::Signal<> finished;
    Wt::Dbo::ptr<Group> adminGroup(Wt::Dbo::Transaction &transaction);
    Wt::WRadioButton *addRadio(const std::string &textKey, Wt::WContainerWidget *container, Wt::WButtonGroup *buttonGroup);
    void displayPage(Wt::WWidget *widget, Page previousPage, Page nextPage);
private:
    class PandoricaWizard* const q;
};

#endif // PRIVATE_H
