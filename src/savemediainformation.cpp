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

void SaveMediaInformation::save(const shared_ptr< FFMPEGMedia >& media, function< void(const Media&) > onSave, Session& session)
{
  Dbo::Transaction t(session);
  
  MediaPropertiesPtr existingMediaProperties = session.find<MediaProperties>().where("media_id = ?").bind(media->media().uid()).resultValue();
  if(existingMediaProperties) {
    WServer::instance()->log("notice") << "Media propeties already found for " << media->media().path();
    return;
  }
  WServer::instance()->log("notice") << "Fetching information for " << media->media().path();
//     auto mediaLock = media->media().lock();
  string titleSuggestion = media->metadata( "title" ).empty() ? Utils::titleHintFromFilename( media->media().filename() ) : media->metadata( "title" );
  session.execute( "DELETE FROM media_properties WHERE media_id = ?" ).bind( media->media().uid() );
  pair<int, int> resolution = media->resolution();
  auto mediaProperties = new MediaProperties {media->media().uid(), titleSuggestion, media->media().fullPath(), media->durationInSeconds(),
      static_cast<int64_t>(boost::filesystem::file_size( media->media().path())), resolution.first, resolution.second};
  session.add( mediaProperties );
  t.commit();
  WServer::instance()->post(appSessionId, boost::bind(onSave, media->media()));
}

SaveMediaInformation::SaveMediaInformation(const std::string &appSessionId)
  : appSessionId(appSessionId)
{
}


SaveMediaInformation::~SaveMediaInformation()
{

}
