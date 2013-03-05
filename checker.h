#ifndef CHECKER_H
#define CHECKER_H
#include <QTimer>
#include <QObject>
#include <QNetworkAccessManager>
#include <QVector>
#include "ticket.h"
#include "login_config.h"
#include "mysql_config.h"
#include "OtrsModule.h"
#include "billingModule.h"
#include "timeChecker.h"

#include <QDebug>

class Checker : public QObject
{
Q_OBJECT
private:
    int stopValue;
    int blockValue;

    bool otrsConnected;
    LoginConfig otrsConfig, billConfig;
    MysqlConfig mysqlconfig;

    OtrsModule *otrsConnection;
    BillingModule *billConnection;

    int currentTicket;
    int currentId;

    bool workInBill;
    int workInBillId;
    int workInBillNumber;

    QVector<Ticket> tmpList, billList;
    QVector<Ticket> oldList, newList, curList;

    TimeChecker *timeChecker;


public:
    Checker (LoginConfig otrs, LoginConfig bill, MysqlConfig sql, QObject *parent);
    ~Checker();

    void run();

    void setOtrsConfig(LoginConfig otrs) {otrsConfig = otrs; }
    void setBillconfig(LoginConfig bill) {billConfig = bill; }

public slots:
    void stop();
    void connection_ready(int status);
    void on_logTxt(QString txt);

private slots:
    void check_new_tickets();    //отправляем http-запрос на проверку новых тикетов
    void new_tickets_list_ready(QStringList lst);

    void new_ticketes_checked(); //после завершения запроса оказываемся здесь

    void work_on_ticket_progress();
    void work_on_ticket_done(Ticket ticket);

    void work_on_bill_ticket();
    void work_on_bill_ticket_done(Ticket ticket);



signals:
    void finished ();
    void new_ticket (Ticket ticket);
    void remove_ticket (Ticket ticket);
    void update_ticket (Ticket ticket);

    void connected_success(bool status);

    void work_on_bill_finished(Ticket ticket);

    void logText(QString txt);


};

#endif // CHECKER_H
