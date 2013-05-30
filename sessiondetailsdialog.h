#ifndef SESSIONDETAILSDIALOG_H
#define SESSIONDETAILSDIALOG_H

#include <Wt/WDialog>
#include <Wt/Dbo/ptr>
#include <Wt/Dbo/Query>
#include <boost/tuple/tuple.hpp>

class Settings;
class Session;
class SessionDetails;

typedef boost::tuple<std::string,long,long,std::string> SessionDetailsTuple;
class SessionDetailsDialog : public Wt::WDialog
{

public:
    SessionDetailsDialog(std::string id, Session *session, Settings *settings);
    SessionDetailsDialog(long userId, Session *session, Settings *settings);
    
private:
    SessionDetailsDialog(const Wt::Dbo::Query<SessionDetailsTuple> &query, Settings *settings);
};

#endif // SESSIONDETAILSDIALOG_H
