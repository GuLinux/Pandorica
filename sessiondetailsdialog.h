#ifndef SESSIONDETAILSDIALOG_H
#define SESSIONDETAILSDIALOG_H

#include <Wt/WDialog>
#include <Wt/Dbo/ptr>
#include <Wt/Dbo/Query>
#include <boost/tuple/tuple.hpp>

class Session;
class SessionDetails;

typedef boost::tuple<std::string,long,long,std::string> SessionDetailsTuple;
class SessionDetailsDialog : public Wt::WDialog
{

public:
    SessionDetailsDialog(std::string id, Session *session);
    SessionDetailsDialog(long userId, Session *session);
    
private:
    SessionDetailsDialog(const Wt::Dbo::Query<SessionDetailsTuple> &query);
};

#endif // SESSIONDETAILSDIALOG_H
