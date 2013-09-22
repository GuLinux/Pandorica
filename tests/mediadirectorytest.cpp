#include "boost_unit_tests_helper.h"
#include "mediacollectionbrowser.h"
#include <media/mediadirectory.h>
#include <boost/filesystem.hpp>
using namespace std;
namespace fs = boost::filesystem;
struct Paths
{
#ifndef WIN32
  fs::path rootMediaDirectoryPath {"/mediadirectory"};
  fs::path mediaInRootDirectoryPath {"/mediadirectory/mymedia.mp4"};

  fs::path mediaInDifferentPath {"/anotherMediaDirectoryPath/anotherMedia.mp4"};

  fs::path subDirectoryPath {"/mediadirectory/subdirectory"};
  fs::path mediaInSubDirectoryPath {"/mediadirectory/subdirectory/media.mp4"};
  fs::path secondMediaInSubDirectoryPath {"/mediadirectory/subdirectory/media.mp4"};

  fs::path secondSubDirectoryPath {"/mediadirectory/secondsubdirectory"};
  fs::path mediaInSecondSubDirectoryPath {"/mediadirectory/secondsubdirectory/media.mp4"};

  fs::path deepDirectoryPath {"/mediadirectory/subdirectory/anotherSubDirectory"};
  fs::path mediaInDeepDirectoryPath {"/mediadirectory/subdirectory/anotherSubDirectory/media.mp4"};
#else
  fs::path rootMediaDirectoryPath {"C:\\mediadirectory"};
  fs::path mediaInRootDirectoryPath {"C:\\mediadirectory\\mymedia.mp4"};

  fs::path mediaInDifferentPath {"C:\\anotherMediaDirectoryPath\\anotherMedia.mp4"};

  fs::path subDirectoryPath {"C:\\mediadirectory\\subdirectory"};
  fs::path mediaInSubDirectoryPath {"C:\\mediadirectory\\subdirectory\\media.mp4"};
  fs::path secondMediaInSubDirectoryPath {"C:\\mediadirectory\\subdirectory\\media.mp4"};

  fs::path secondSubDirectoryPath {"C:\\mediadirectory\\secondsubdirectory"};
  fs::path mediaInSecondSubDirectoryPath {"C:\\mediadirectory\\secondsubdirectory\\media.mp4"};

  fs::path deepDirectoryPath {"C:\\mediadirectory\\subdirectory\\anotherSubDirectory"};
  fs::path mediaInDeepDirectoryPath {"C:\\mediadirectory\\subdirectory\\anotherSubDirectory\\media.mp4"};
#endif
  MediaDirectory mediaDirectory {rootMediaDirectoryPath};
};
BOOST_FIXTURE_TEST_CASE( TestAddMedia, Paths )
{
  Media media( mediaInRootDirectoryPath );
  BOOST_REQUIRE( mediaDirectory.medias().empty() );
  mediaDirectory.add( media );

  BOOST_REQUIRE_EQUAL( 1, mediaDirectory.medias().size() );
  BOOST_REQUIRE_EQUAL( 1, mediaDirectory.allMedias().size() );
  BOOST_REQUIRE_EQUAL( 0, mediaDirectory.subDirectories().size() );
  BOOST_REQUIRE_EQUAL( media, mediaDirectory.medias().front() );
  BOOST_REQUIRE_EQUAL( media, mediaDirectory.allMedias().front() );
}

BOOST_FIXTURE_TEST_CASE( TestDontAddMediaIfBelongingToOtherDirectory, Paths )
{
  BOOST_REQUIRE( mediaDirectory.medias().empty() );
  mediaDirectory.add( Media {mediaInDifferentPath} );

  BOOST_REQUIRE_EQUAL( 0, mediaDirectory.medias().size() );
  BOOST_REQUIRE_EQUAL( 0, mediaDirectory.allMedias().size() );
  BOOST_REQUIRE_EQUAL( 0, mediaDirectory.subDirectories().size() );
}

BOOST_FIXTURE_TEST_CASE( TestAddSubDirectoryMedia, Paths )
{
  Media media( mediaInSubDirectoryPath );
  BOOST_REQUIRE( mediaDirectory.medias().empty() );
  mediaDirectory.add( media );

  BOOST_REQUIRE_EQUAL( 0, mediaDirectory.medias().size() );
  BOOST_REQUIRE_EQUAL( 1, mediaDirectory.subDirectories().size() );
  BOOST_REQUIRE_EQUAL( MediaDirectory {subDirectoryPath}, *mediaDirectory.subDirectories().front() );
  BOOST_REQUIRE_EQUAL( 1, mediaDirectory.allMedias().size() );
  BOOST_REQUIRE_EQUAL( media, mediaDirectory.allMedias().front() );
}


BOOST_FIXTURE_TEST_CASE( TestComplexSubDirectoryStructure, Paths )
{
  Media mediaInRoot {mediaInRootDirectoryPath};
  Media mediaInSubDirectory {mediaInSubDirectoryPath};
  Media secondMediaInSubDirector {secondMediaInSubDirectoryPath};
  Media mediaInSecondSubDirectory {mediaInSecondSubDirectoryPath};
  Media deepMedia {mediaInDeepDirectoryPath};

  mediaDirectory.add( Media {mediaInDifferentPath} );
  mediaDirectory.add( mediaInSubDirectory );
  mediaDirectory.add( mediaInRoot );
  mediaDirectory.add( mediaInSecondSubDirectory );
  mediaDirectory.add( secondMediaInSubDirector );
  mediaDirectory.add( deepMedia );
  BOOST_REQUIRE_EQUAL( 1, mediaDirectory.medias().size() );
  BOOST_REQUIRE_EQUAL( mediaInRoot, mediaDirectory.medias().front() );
  BOOST_REQUIRE_EQUAL( 5, mediaDirectory.allMedias().size() );
  BOOST_REQUIRE_EQUAL( 2, mediaDirectory.subDirectories().size() );

  shared_ptr<MediaDirectory> firstSubDirectory = mediaDirectory.subDirectories().back();
  BOOST_REQUIRE_EQUAL( MediaDirectory {subDirectoryPath}, *firstSubDirectory );
  BOOST_REQUIRE_EQUAL( 2, firstSubDirectory->medias().size() );
  BOOST_REQUIRE_EQUAL( mediaInSubDirectory, firstSubDirectory->medias().front() );
  BOOST_REQUIRE_EQUAL( secondMediaInSubDirector, firstSubDirectory->medias().back() );
  BOOST_REQUIRE_EQUAL( 3, firstSubDirectory->allMedias().size() );
  BOOST_REQUIRE_EQUAL( 1, firstSubDirectory->subDirectories().size() );

  shared_ptr<MediaDirectory> deepDirectory = firstSubDirectory->subDirectories().front();
  BOOST_REQUIRE_EQUAL( MediaDirectory {deepDirectoryPath}, *deepDirectory );
  BOOST_REQUIRE_EQUAL( 1, deepDirectory->allMedias().size() );
  BOOST_REQUIRE_EQUAL( 1, deepDirectory->medias().size() );
  BOOST_REQUIRE_EQUAL( 0, deepDirectory->subDirectories().size() );
  BOOST_REQUIRE_EQUAL( deepMedia, deepDirectory->medias().front() );

  shared_ptr<MediaDirectory> secondSubDirectory = mediaDirectory.subDirectories().front();
  BOOST_REQUIRE_EQUAL( MediaDirectory {secondSubDirectoryPath}, *secondSubDirectory );
  BOOST_REQUIRE_EQUAL( 1, secondSubDirectory->allMedias().size() );
  BOOST_REQUIRE_EQUAL( 1, secondSubDirectory->medias().size() );
  BOOST_REQUIRE_EQUAL( 0, secondSubDirectory->subDirectories().size() );
  BOOST_REQUIRE_EQUAL( mediaInSecondSubDirectory, secondSubDirectory->medias().front() );
}
