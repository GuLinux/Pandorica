#include "MediaScanner/mediascannerstep.h"

MediaScannerStep::StepResult MediaScannerStep::result()
{
  return _result;
}

void MediaScannerStep::setResult( MediaScannerStep::StepResult result )
{
  _result = result;
}
