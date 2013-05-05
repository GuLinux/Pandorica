#ifndef MEDIASCANNERSTEP_H
#define MEDIASCANNERSTEP_H
#include <Wt/WSignal>

class Media;
class FFMPEGMedia;
namespace Wt {
class WContainerWidget;
class WApplication;
}

#define guiRun(app, f) WServer::instance()->post(app->sessionId(), f)

class MediaScannerStep
{
public:
  enum StepResult { Waiting, Done, Skip, Redo };
  virtual void run(FFMPEGMedia *ffmpegMedia, Media *media, Wt::WContainerWidget *container) = 0;
  virtual StepResult result() = 0;
  virtual void save() = 0;
};

#endif // MEDIASCANNERSTEP_H
