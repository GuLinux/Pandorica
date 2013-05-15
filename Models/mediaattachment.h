#ifndef MEDIA_ATTACHMENT_H
#define MEDIA_ATTACHMENT_H

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WLink>

namespace Wt {
class WObject;
class WResource;
}


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
  MediaAttachment() = default;
  MediaAttachment(std::string type, std::string name, std::string value, std::string mediaId, std::string mimetype, std::vector<unsigned char> data)
    : _type(type), _name(name), _value(value), _mediaId(mediaId), _mimetype(mimetype), _data(data)
  {}
  inline std::string type() const { return _type; }
  inline std::string name() const { return _name; }
  inline std::string value() const { return _value; }
  inline std::string mediaId() const { return _mediaId; }
  inline std::string mimetype() const { return _mimetype; }
  inline unsigned long size() const { return _data.size(); }
  inline std::vector<unsigned char> data() const { return _data; }
  
  Wt::WLink link(Wt::Dbo::ptr< MediaAttachment > myPtr, Wt::WObject* parent = 0, bool useCacheIfAvailable = true) const;
private:
  std::string _type;
  std::string _name;
  std::string _value;
  std::string _mediaId;
  std::string _mimetype;
  std::vector<unsigned char> _data;
};


typedef Wt::Dbo::ptr<MediaAttachment> MediaAttachmentPtr;
#endif