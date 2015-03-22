/*
 * Copyright 2015 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "savemediainformation.h"
#include "media/media.h"
#include "Models/models.h"
#include "session.h"
#include "ffmpegmedia.h"
#include "utils/utils.h"
#include "threadpool.h"
#include <Wt/WServer>
#include <Wt/WIOService>
#include <boost/thread.hpp>

using namespace Wt;
using namespace std;

void SaveMediaInformation::save(const Media& media, function<void(const Media &media)> onSave)
{
  Dbo::Transaction t(session);
  string appSession = wApp->sessionId();
  MediaPropertiesPtr mediaProperties = session.find<MediaProperties>().where("media_id = ?").bind(media.uid()).resultValue();
  if(mediaProperties) {
    wApp->log("notice") << "Media propeties already found for " << media.path();
    return;
  }
  ThreadPool::instance()->post([media,appSession,onSave] () mutable {
    WServer::instance()->log("notice") << "Fetching information for " << media.path();
    auto lock_task = ThreadPool::lock("media_lock");
    auto mediaLock = media.lock();
    Session session;
    auto lock = session.writeLock();
    Dbo::Transaction transaction(session);
    FFMPEGMedia ffmpegMedia(media);
    string titleSuggestion = ffmpegMedia.metadata( "title" ).empty() ? Utils::titleHintFromFilename( media.filename() ) : ffmpegMedia.metadata( "title" );
    transaction.session().execute( "DELETE FROM media_properties WHERE media_id = ?" ).bind( media.uid() );
    pair<int, int> resolution = ffmpegMedia.resolution();
    auto mediaProperties = new MediaProperties {media.uid(), titleSuggestion, media.fullPath(), ffmpegMedia.durationInSeconds(),
	static_cast<int64_t>(boost::filesystem::file_size( media.path())), resolution.first, resolution.second};
    transaction.session().add( mediaProperties );
    transaction.commit();
    WServer::instance()->post(appSession, boost::bind(onSave, media));
  });
}

SaveMediaInformation::SaveMediaInformation(Session& session)
  : session(session)
{
}


SaveMediaInformation::~SaveMediaInformation()
{

}
