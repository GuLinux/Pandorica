#ifndef PROJ_WT_HELPERS
#define PROJ_WT_HELPERS

#define _n1 Wt::NoClass
#define _n2 Wt::NoClass, _n1
#define _n3 Wt::NoClass, _n2
#define _n4 Wt::NoClass, _n3
#define _n5 Wt::NoClass, _n4
#define _n6 Wt::NoClass, _n5
namespace Wt {
  class WLayout;
}

#include <Wt/WWidget>
#include <Wt/WEvent>
#include <functional>
#include <Wt/WServer>

#define wLog (wApp ? wApp->log("notice") : Wt::WServer::instance()->log("notice"))
#define JS(...) #__VA_ARGS__
#define HTML(...) #__VA_ARGS__

typedef std::function<void(Wt::WMouseEvent&)> MouseEventListener;

template<class WidgetType>
class _WW {
public:
  _WW(WidgetType *widget) :widget(widget) {}
  _WW &css(std::string css) {
    widget->setStyleClass(css);
    return *this;
  }
  
  _WW &addCss(std::string css) {
    widget->addStyleClass(css);
    return *this;
  }
  
  WidgetType *get() {
    return widget;
  }
  
  _WW &add(Wt::WWidget *newWidget) {
    widget->addWidget(newWidget);
    return *this;
  }
  
  _WW &padding(const Wt::WLength& padding, Wt::WFlags<Wt::Side > sides = Wt::All) {
    widget->setPadding(padding, sides);
    return *this;
  }
  
template<class OtherType>  
  _WW &add(_WW<OtherType> &newWidget) {
    widget->addWidget(newWidget.get());
    return *this;
  }
  
  _WW &onClick(MouseEventListener listener) {
    widget->clicked().connect(listener);
    return *this;
  }
  _WW &setContentAlignment(Wt::WFlags<Wt::AlignmentFlag> contentAlignment) {
    widget->setContentAlignment(contentAlignment);
    return *this;
  }
  _WW &setVerticalAlignment(Wt::AlignmentFlag alignment, const Wt::WLength & length = Wt::WLength()) {
    widget->setVerticalAlignment(alignment, length);
    return *this;
  }
  _WW &setInline(bool isInline) {
    widget->setInline(isInline);
    return *this;
  }
  _WW &resize(Wt::WLength &w, Wt::WLength &h) {
    widget->resize(w,h);
    return *this;
  }
  _WW &setLayout(Wt::WLayout *layout) {
    widget->setLayout(layout);
    return *this;
  }
  _WW &setAttribute(const std::string &attrName, const std::string &attrValue) {
    widget->setAttributeValue(attrName, attrValue);
    return *this;
  }
  _WW &setMargin(const Wt::WLength& margin, Wt::WFlags<Wt::Side> sides = Wt::All) {
    widget->setMargin(margin, sides);
    return *this;
  }
  
  template<class T>
  _WW &setText(T text) {
    widget->setText(text);
    return *this;
  }
  
  _WW &setTextFormat(Wt::TextFormat format) {
    widget->setTextFormat(format);
    return *this;
  }
  
  operator WidgetType*() {
    return widget;
  }
   
  _WW &setEnabled(bool enable) {
    widget->setEnabled(enable);
    return *this;
  }
   
  _WW &enable() {
    widget->enable();
    return *this;
  }
   
  _WW &disable() {
    widget->disable();
    return *this;
  }
private:
  WidgetType *widget;
};

#define WW(WidgetType, ...) _WW<WidgetType>(new WidgetType(__VA_ARGS__) )
#endif