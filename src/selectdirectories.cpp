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




#include "selectdirectories.h"
#include "private/selectdirectories_p.h"
#include "settings.h"
#include <Wt/WTreeView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WContainerWidget>
#include <Wt/WIOService>
#include <boost/filesystem.hpp>
#include "Wt-Commons/wt_helpers.h"
#include "utils/d_ptr_implementation.h"

using namespace std;
using namespace Wt;

namespace fs = boost::filesystem;

SelectDirectories::Private::Private( SelectDirectories *q, vector< string > selectedPaths, SelectDirectories::SelectionType selectionType )
  : q( q ), selectedPaths( selectedPaths ), selectionType( selectionType )
{
}

SelectDirectories::SelectDirectories( vector< string > rootPaths, string selectedPath, OnPathClicked onPathSelected, WContainerWidget* parent )
  : SelectDirectories( rootPaths, {selectedPath}, onPathSelected, []( string ) {}, Single, parent )
{

}


SelectDirectories::SelectDirectories( vector< string > rootPaths, vector< string > selectedPaths, OnPathClicked onPathSelected, OnPathClicked onPathUnselected, SelectDirectories::SelectionType selectionType, WContainerWidget* parent )
  : WCompositeWidget( parent ), d( this, selectedPaths, selectionType )
{
  WContainerWidget *container = new WContainerWidget;
  setImplementation(container);
  WTreeView *tree = new WTreeView();
  container->addWidget(tree);
  d->tree = tree;
  d->app = wApp;
  d->model = new WStandardItemModel( this );
  tree->setMinimumSize( 400, WLength::Auto );
  tree->setHeaderHeight( 0 );
  tree->setSortingEnabled( false );
  tree->setModel( d->model );
  tree->setRootIsDecorated( true );

  tree->doubleClicked().connect( [ = ]( WModelIndex index, WMouseEvent, _n4 )
  {
    tree->setExpanded( index, !tree->isExpanded( index ) );
  } );

  if( selectionType == Single )
  {
    d->tree->clicked().connect( [ = ]( WModelIndex index, WMouseEvent, _n4 )
    {
      WStandardItem *item = d->model->itemFromIndex( index );
      fs::path p = boost::any_cast<fs::path>( item->data() );
      onPathSelected( p.string() );
    } );
  }

  d->model->itemChanged().connect( [ = ]( WStandardItem * item, _n5 )
  {
    bool itemChecked = ( item->checkState() == Wt::Checked );
    string itemPath = boost::any_cast<fs::path>( item->data() ).string();

    if( itemChecked )
    {
      onPathSelected( itemPath );
    }
    else
    {
      onPathUnselected( itemPath );
    }
  } );

  tree->expanded().connect( [ = ]( WModelIndex index, _n5 )
  {
    WStandardItem *item = d->model->itemFromIndex( index );
    WServer::instance()->ioService().post( [ = ]
    {
      for( int i = 0; i < item->rowCount(); i++ )
      {
        d->addSubItems( item->child( i ) )
        ;
      }
    } );
  } );

  for( string path : rootPaths )
    d->populateTree( path );

  // TODO: expand selected items
}

void SelectDirectories::setHeight( WLength height )
{
  d->tree->setHeight( height );
}

void SelectDirectories::setWidth( WLength width )
{
  d->tree->setWidth( width );
}


void SelectDirectories::Private::populateTree( std::string path )
{
  model->appendRow( buildStandardItem( path, true ) );

  for( string p : selectedPaths )
  {
    if( items.count( fs::path( p ) ) > 0 )
    {
      WStandardItem *item = items[fs::path( p )];

      while( item->parent() != 0 )
      {
        item = item->parent();
        tree->expand( model->indexFromItem( item ) );
      }
    }
  }
};

WStandardItem *SelectDirectories::Private::buildStandardItem( boost::filesystem::path path, bool shouldAddSubItems )
{
  string folderName {path.filename().string()};
#ifdef WIN32

  if( folderName == "/" )
    folderName = path.string();

#endif
  WStandardItem *item = new WStandardItem {Settings::icon( Settings::FolderSmall ), folderName};
  item->setCheckable( selectionType == SelectDirectories::Multiple );
  item->setStyleClass( "tree-directory-item link-hand" );
  item->setLink( "" );
  item->setToolTip( wtr( "tree.double.click.to.expand" ) );
  item->setData( path );
  bool hasSelectedSubItems = false;

  for( string pathSelected : selectedPaths )
  {
    hasSelectedSubItems |= pathSelected.find( path.parent_path().string() ) != string::npos;

    if( pathSelected == path.string() )
      item->setChecked( true );
  }

  if( shouldAddSubItems || hasSelectedSubItems )
  {
    addSubItems( item, true );
  }

  items[path] = item;
  return item;
}

void SelectDirectories::Private::addSubItems( WStandardItem *item, bool sync )
{
  fs::path path = boost::any_cast<fs::path>( item->data() );

  try
  {
    fs::directory_iterator it {path};
    vector<fs::path> paths;
//     log( "notice" ) << "Filtering directory tree for path: " << path.string();
    copy_if( it, fs::directory_iterator(), back_inserter( paths ), [ = ]( fs::path p )
    {
      try
      {
        return fs::is_directory( p ) && p.filename().string()[0] != '.';
      }
      catch
        ( std::exception &e )
      {
        log( "warning" ) << "Error filtering path " << p.string() << ": " << e.what();
      }
    } );
//     log( "notice" ) << "Sorting tree for path: " << path.string();
    sort( begin(paths), end(paths), [ = ]( fs::path a, fs::path b )
    {
      return a.filename() < b.filename();
    } );
    auto addItems = [ = ]
    {
      for_each( begin(paths), end(paths), [ = ]( fs::path p )
      {
//         log( "notice" ) << "Adding subpath " << p.string() << " to path " << path.string();

        if( items.count( p ) > 0 )
          return;

        item->appendRow( buildStandardItem( p, false ) );
      } );
    };

    if( sync )
    {
      addItems();
    }
    else
    {
      WServer::instance()->post( app->sessionId(), [ = ]
      {
        addItems();
        app->triggerUpdate();
      } );
    }
  }
  catch
    ( std::exception &e )
  {
//     log( "warning" ) << "Error adding subdirectories for path " << path << ": " << e.what();
  }
}


SelectDirectories::~SelectDirectories()
{
}
