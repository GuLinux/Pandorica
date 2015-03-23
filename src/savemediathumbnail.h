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

#ifndef SAVEMEDIATHUMBNAIL_H
#define SAVEMEDIATHUMBNAIL_H
#include <functional>

class Media;
class FFMPEGMedia;
class Session;
#include <memory>
class SaveMediaThumbnail
{
public:
    SaveMediaThumbnail(Session &session);
    ~SaveMediaThumbnail();
    void save(const std::shared_ptr<FFMPEGMedia> &media, std::function<void(const Media &media)> onSave);
private:
  Session &session;
};

#endif // SAVEMEDIATHUMBNAIL_H
