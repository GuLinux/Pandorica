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
  enum StepResult { Complete, ToRedo, Skip };
  virtual StepResult run(FFMPEGMedia *ffmpegMedia, Media *media, Wt::WContainerWidget *container) = 0;
};

#endif // MEDIASCANNERSTEP_H
