#ifndef MEDIASCANNERPAGE_H
#define MEDIASCANNERPAGE_H

#include <Wt/WContainerWidget>

class MediaScannerPage : public Wt::WContainerWidget
{
public:
  MediaScannerPage(Wt::WContainerWidget* parent = 0) : Wt::WContainerWidget(parent) {}
  virtual void run() = 0;
  inline Wt::Signal<> &finished() { return _finished; }
  
private:
  Wt::Signal<> _finished;
};

#endif // MEDIASCANNERPAGE_H
