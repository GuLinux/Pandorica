#include "Models/models.h"

using namespace Wt;
using namespace std;
namespace dbo = Wt::Dbo;


void SessionInfo::endStale(Dbo::Transaction& transaction)
{
  transaction.session().execute("UPDATE session_info SET session_ended = ? WHERE session_id <> ? AND session_ended IS NULL")
    .bind(WDateTime::currentDateTime().toPosixTime())
    .bind(wApp->sessionId())
  ;
}

void SessionInfo::end()
{
 _sessionEnded = WDateTime::currentDateTime().toPosixTime();
}


