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

#include "savemediathumbnail.h"
#include "media/media.h"
#include "ffmpegmedia.h"
#include "session.h"
#include "mediathumbnailgenerator.h"
#include <Wt/WServer>
#include <Wt/WIOService>
#include <boost/thread.hpp>


using namespace Wt;
using namespace std;




SaveMediaThumbnail::SaveMediaThumbnail(Session& session) : session(session)
{

}

SaveMediaThumbnail::~SaveMediaThumbnail()
{

}

void SaveMediaThumbnail::save(const Media& media, std::function< void(const Media &media) > onSave)
{
  string appSession = wApp->sessionId();
  Dbo::Transaction t(session);
  int images = session.query<int>("SELECT count(*) from media_attachment where type = 'preview' AND media_id = ?").bind(media.uid());
  if(images >= 3) {
    wApp->log("notice") << "Media propeties already found for " << media.path();
    return;
  }
  auto path = media.path();
  boost::async([media, onSave, appSession] () mutable {
    WServer::instance()->log("notice") << "Creating thumbnail for " << media.path();
    MediaThumbnailGenerator thumbnailGenerator(media);
    try {
      auto mediaLock = media.lock();
      Session session;
      auto lock = session.writeLock();
      Dbo::Transaction transaction(session);
      transaction.session().execute( "DELETE FROM media_attachment where type = 'preview' AND media_id = ?" ).bind( media.uid() );
      media.setImage(thumbnailGenerator.image(), transaction);
      transaction.commit();
      WServer::instance()->post(appSession, boost::bind(onSave, media));
    } catch(std::exception &e) {
      WServer::instance()->log("notice") << "Unable to create thumbnail for media " << media.path();
    }
  });
}
