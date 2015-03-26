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
#include "threadpool.h"
#include "ffmpegmedia.h"
#include <Wt/WServer>
#include <Wt/WIOService>
#include <boost/thread.hpp>


using namespace Wt;
using namespace std;




SaveMediaThumbnail::SaveMediaThumbnail(const string& appSessionId) : appSessionId(appSessionId)
{

}

SaveMediaThumbnail::~SaveMediaThumbnail()
{

}

void SaveMediaThumbnail::save(const shared_ptr< FFMPEGMedia >& media, function< void(const Media&)> onSave, Session& session)
{
  Dbo::Transaction t(session);
  int images = session.query<int>("SELECT count(*) from media_attachment where type = 'preview' AND media_id = ?").bind(media->media().uid());
  if(images >= 3) {
    WServer::instance()->log("notice") << "Media propeties already found for " << media->media().path();
    return;
  }
  auto path = media->media().path();
  WServer::instance()->log("notice") << "Creating thumbnail for " << media->media().path();
  
  try {
//       auto mediaLock = media->media().lock();
    session.execute( "DELETE FROM media_attachment where type = 'preview' AND media_id = ?" ).bind( media->media().uid() );
    media->media().setImage(media->randomThumbnail(), t);
    t.commit();
    WServer::instance()->post(appSessionId, boost::bind(onSave, media->media() ));
  } catch(std::exception &e) {
    WServer::instance()->log("notice") << "Unable to create thumbnail for media " << media->media().path();
  }
}
