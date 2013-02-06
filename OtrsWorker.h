#ifndef OTRSWORKER_H
#define OTRSWORKER_H
#include "OtrsModule.h"
#include "login_config.h"
#include "mysql_config.h"
#include "ticket.h"

class OtrsWorker : public OtrsModule {
    Q_OBJECT
public:
    OtrsWorker(LoginConfig otrs);

    void spamTicket  (int id);
    void answTicket  (int id);
    void closeTicket (int id, QString text);




private:
    LoginConfig otrsconfig;

    QList<int> tickets;

    bool isWorking;

    QString typeOfWork;

    QString currText;

private slots:

    void ticket_described_for_ChallengeToken(QNetworkReply *rpl);
    void ticket_described_for_closing(QNetworkReply *rpl);


    void move_to_spam(int Id, QString challange);

    void close_ticket(int Id, QString challange, QString formID);

    void work_ready(QNetworkReply *rpl);

signals:

    void unblocActions(bool status);




};



#endif // OTRSWORKER_H
