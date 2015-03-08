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

#ifndef FILERESOURCE_H
#define FILERESOURCE_H

#include <Wt/WStreamResource>

namespace Wt {
  class WFileResource;
}

class FileResource : public Wt::WStreamResource
{
public:
    FileResource(const std::string &fileName, Wt::WObject* parent);
    FileResource(const std::string& mimeType, const std::string &fileName, Wt::WObject* parent);
    virtual ~FileResource();
    virtual void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response);
    void setFileName (const std::string &fileName);
    const std::string &fileName() const;
private:
  std::unique_ptr<Wt::WFileResource> _fileResource;
};

#endif // FILERESOURCE_H
