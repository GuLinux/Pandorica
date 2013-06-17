#include "Models/models.h"

using namespace Wt;
using namespace std;
namespace dbo = Wt::Dbo;


void SessionInfo::endStale(Dbo::Transaction& transaction)
{
  transaction.session().execute("UPDATE session_info SET session_ended = ? WHERE session_id <> ?")
    .bind(SessionInfo::now())
    .bind(wApp->sessionId())
  ;
}

void SessionInfo::end()
{
 _sessionEnded = SessionInfo::now();
}


time_t SessionInfo::now()
{
  return WDateTime::currentDateTime().toTime_t();
}

