#ifndef MEDIA_ATTACHMENT_H
#define MEDIA_ATTACHMENT_H

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>

class MediaProperties {
public:
  template<class Action>
  void persist(Action& a) {
    Wt::Dbo::id(a, _mediaId, "media_id");
    Wt::Dbo::field(a, _title, "title");
    Wt::Dbo::field(a, _filename, "filename");
    Wt::Dbo::field(a, _duration, "duration");
    Wt::Dbo::field(a, _size, "size");
    Wt::Dbo::field(a, _width, "width");
    Wt::Dbo::field(a, _height, "height");
  }
  inline std::string mediaId() const { return _mediaId; }
  inline std::string title() const { return _title; }
  inline std::string filename() const { return _filename; }
  inline int64_t duration() const { return _duration; }
  inline int64_t size() const { return _size; }
  inline int width() const { return _width; }
  inline int height() const { return _height; }
  void setTitle(std::string title) { _title = title; }
  MediaProperties() = default;
  MediaProperties(std::string mediaId, std::string title, std::string filename, int64_t duration, int64_t size, int width, int height)
    : _mediaId(mediaId), _title(title), _filename(filename), _duration(duration), _size(size), _width(width), _height(height) {}
private:
  std::string _mediaId;
  std::string _title;
  std::string _filename;
  int64_t _duration;
  int64_t _size;
  int _width;
  int _height;
};

class MediaAttachment {
public:
  template<class Action>
  void persist(Action& a) {
    Wt::Dbo::field(a, _mediaId, "media_id");
    Wt::Dbo::field(a, _type, "type");
    Wt::Dbo::field(a, _name, "name");
    Wt::Dbo::field(a, _value, "value");
    Wt::Dbo::field(a, _mimetype, "mimetype");
    Wt::Dbo::field(a, _data, "data");
  }
  
  inline std::string type() const { return _type; }
  inline std::string name() const { return _name; }
  inline std::string value() const { return _value; }
  inline std::string mediaId() const { return _mediaId; }
  inline std::string mimetype() const { return _mimetype; }
  inline std::vector<unsigned char> data() const { return _data; }
  
private:
  std::string _type;
  std::string _name;
  std::string _value;
  std::string _mediaId;
  std::string _mimetype;
  std::vector<unsigned char>_data;
};


namespace Wt {
  namespace Dbo {
    template<>
    struct dbo_traits<MediaProperties> {
      typedef std::string IdType;
      static IdType invalidId() { return {}; }
      static const char *surrogateIdField() { return 0; }
      static const char *versionField() { return 0; }
    };
  }
}

typedef Wt::Dbo::ptr<MediaAttachment> MediaAttachmentPtr;
typedef Wt::Dbo::ptr<MediaProperties> MediaPropertiesPtr;
#endif