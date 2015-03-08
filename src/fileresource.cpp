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

#include "fileresource.h"
#include <Wt/WFileResource>
#include <Wt/WApplication>

using namespace std;
using namespace Wt;

FileResource::FileResource(const string &fileName, WObject* parent)
  : _fileResource(new WFileResource{fileName})
{
  wApp->log("notice") << __PRETTY_FUNCTION__ << ": filename=" << this->fileName();
}

FileResource::FileResource(const string& mimeType, const string& fileName, WObject* parent)
  : _fileResource(new WFileResource{mimeType, fileName})
{
  wApp->log("notice") << __PRETTY_FUNCTION__ << ": filename=" << this->fileName() << ", mimeType: " << mimeType;
}

FileResource::~FileResource()
{
  wApp->log("notice") << __PRETTY_FUNCTION__ << ": filename=" << fileName();
}


void FileResource::handleRequest(const Http::Request& request, Http::Response& response)
{
  _fileResource->handleRequest(request, response);
}

const string& FileResource::fileName() const
{
  return _fileResource->fileName();
}

void FileResource::setFileName(const string& fileName)
{
  _fileResource->setFileName(fileName);
}
