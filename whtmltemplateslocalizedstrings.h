#ifndef WHTMLTEMPLATESLOCALIZEDSTRINGS_H
#define WHTMLTEMPLATESLOCALIZEDSTRINGS_H
#include <boost/filesystem/path.hpp>

#include <Wt/WLocalizedStrings>
#include <Wt/WObject>


class WHTMLTemplatesLocalizedStrings : public Wt::WLocalizedStrings, public Wt::WObject
{

public:
  WHTMLTemplatesLocalizedStrings(const std::string &resourcesDir, Wt::WObject *parent = 0);
  virtual bool resolveKey(const std::string& key, std::string& result);
  virtual ~WHTMLTemplatesLocalizedStrings();
private:
    void processHtmlTemplate(boost::filesystem::path path);
    std::map<std::string,std::string> translationMap;
};

#endif // WHTMLTEMPLATESLOCALIZEDSTRINGS_H