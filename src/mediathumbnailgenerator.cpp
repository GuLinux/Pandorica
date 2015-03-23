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

#include "mediathumbnailgenerator.h"
#include <chrono>
#include <random>
#include "ffmpegmedia.h"
#include <boost/format.hpp>
#include "videothumbnailer.h"
#include <filmstripfilter.h>

using namespace ffmpegthumbnailer;
using namespace Wt;
using namespace std;
using namespace std::chrono;
namespace {
  time_point<high_resolution_clock> serverStartTimeForRandomSeeding {high_resolution_clock::now()};
  mt19937_64 randomEngine {( uint64_t ) serverStartTimeForRandomSeeding.time_since_epoch().count()};
  
  struct ThumbnailPosition
  {
    int percent;
    std::string timing;
    static ThumbnailPosition from( int timeInSeconds );
  };

  ThumbnailPosition randomPosition(long int media_duration)
  {
    auto randomNumber = randomEngine();

    if( media_duration < 100 )
    {
      int percent = randomNumber % 100;

      if( percent < 10 )
	percent += 10;
      if( percent > 80 )
	percent -= 20;
      return {percent};
    }

    int percent {0};
    int position {0};

    while( percent < 10 || percent > 80 )
    {
      randomNumber = randomEngine();
      position = randomNumber % media_duration;
      percent = position * 100.0 / media_duration;
    }

    return ThumbnailPosition::from( position );
  }
  
  ThumbnailPosition ThumbnailPosition::from( int timeInSeconds )
  {
    int remaining = 0;
    int hours = timeInSeconds / 3600;
    timeInSeconds %= 3600;
    int minutes = timeInSeconds / 60;
    timeInSeconds %= 60;
    string currentTimeStr = ( boost::format( "%.2d:%.2d:%.2d" ) % hours % minutes % timeInSeconds ).str();
    return { -1, currentTimeStr};
  }
}

MediaThumbnailGenerator::MediaThumbnailGenerator(const shared_ptr<FFMPEGMedia> &media) : media(media)
{
  media_duration = media->durationInSeconds();
  resolution = media->resolution();
}

MediaThumbnailGenerator::~MediaThumbnailGenerator()
{

}

Image MediaThumbnailGenerator::image(int quality) const
{
  quality /= 10;
  auto currentPosition = randomPosition(media_duration);
  int fullSize = max( resolution.first, resolution.second );
  VideoThumbnailer videoThumbnailer( fullSize, false, true, quality, true );
  FilmStripFilter filmStripFilter;
  if( media->media().mimetype().find( "video" ) != string::npos )
    videoThumbnailer.addFilter( &filmStripFilter );
  if( currentPosition.percent > 0 )
    videoThumbnailer.setSeekPercentage( currentPosition.percent );
  else
    videoThumbnailer.setSeekTime( currentPosition.timing );
  ImageBlob fullImage;
  videoThumbnailer.generateThumbnail( media->media().fullPath(), ThumbnailerImageType::Png, fullImage );
  return Image{fullImage};
}
