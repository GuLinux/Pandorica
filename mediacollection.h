#ifndef MEDIACOLLECTION_H
#define MEDIACOLLECTION_H

#include <Wt/WObject>
#include <boost/filesystem.hpp>
class MediaCollectionPrivate;
class MediaPrivate;
class Media {
public:
  Media(const boost::filesystem::path &path);
  Media();
  ~Media();
  std::string fullPath() const;
  std::string filename() const;
  std::string extension() const;
  std::string mimetype() const;
  std::string uid() const;
  boost::filesystem::path path() const;
  bool valid() const;
private:
  boost::filesystem::path m_path;
  std::string m_uid;
};

class MediaCollection : public Wt::WObject
{
public:
    MediaCollection(std::string basePath, Wt::WObject* parent = 0);
    virtual ~MediaCollection();
    void rescan();
    std::map<std::string,Media> collection() const;
    Media media(std::string uid) const;
private:
  MediaCollectionPrivate *const d;
};

#endif // MEDIACOLLECTION_H
