#ifndef TIMECHECKER_H
#define TIMECHECKER_H
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "OtrsModule.h"
#include "login_config.h"
#include "mysql_config.h"
#include "ticket.h"

class TimeChecker : public OtrsModule {
    Q_OBJECT
public:
    TimeChecker(LoginConfig otrs, MysqlConfig sql);
    //~TimeChecker ();
    void getLastTime(Ticket ticket);
    void getLastTime(int id);
    void getLastTime();

    void moveTicket(int id);
    void moveTicket(Ticket ticket);


private:
    LoginConfig otrsconfig;
    MysqlConfig dbconfig;

    QSqlDatabase db;
    bool db_is_ready;

    int currentID;

    bool isWorking;

    QList<int> tickets;

private slots:
    void lastTimeReady(QNetworkReply *rpl);

//signals:
//    lastTimeReady(Ticket ticket);


};


#endif // TIMECHECKER_H
