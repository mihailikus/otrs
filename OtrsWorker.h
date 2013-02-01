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

    void delTicket(int id);
    void spamTicket(int id);
    void answTicket(int id);




private:
    LoginConfig otrsconfig;

    QList<int> tickets;

    bool isWorking;

    QString typeOfWork;

private slots:

    void ticket_described_for_ChallengeToken(QNetworkReply *rpl);

    void move_to_spam(int Id, QString challange);

    void work_ready(QNetworkReply *rpl);




};



#endif // OTRSWORKER_H
