#ifndef MEDIACOLLECTION_H
#define MEDIACOLLECTION_H

#include <Wt/WObject>
#include <boost/filesystem.hpp>
class MediaCollectionPrivate;
class MediaPrivate;
class Media {
public:
  Media(boost::filesystem::path path);
  Media(const Media &other);
  ~Media();
  std::string fullPath() const;
  std::string filename() const;
  std::string extension() const;
  std::string mimetype() const;
  std::string uid() const;
  boost::filesystem::path path() const;
private:
  MediaPrivate *const d;
};

class MediaCollection : public Wt::WObject
{
public:
    MediaCollection(std::string basePath, Wt::WObject* parent = 0);
    virtual ~MediaCollection();
    void rescan();
    
private:
  MediaCollectionPrivate *const d;
};

#endif // MEDIACOLLECTION_H
