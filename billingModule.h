#ifndef BILLINGMODULE_H
#define BILLINGMODULE_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QUrl>
#include <QStringList>

#include "ticket.h"
#include "login_config.h"

class BillingModule : public QNetworkAccessManager {
Q_OBJECT
public:
    BillingModule (LoginConfig cfg);
    ~BillingModule ();

    QStringList tickets;

private slots:
    void connected(QNetworkReply *rpl);
    void describe_by_email_finished(QNetworkReply *rpl);

private:
    QString url, url2;
    QString mail;
    QString host;
    QString id;

    bool isConnected;

    int describeMethod;  //определяет, какая процедура запущена для идентификации

public slots:
    void describe_ticket_by_email(Ticket ticket);
    void describe_ticket_by_host(Ticket ticket);
    bool is_connected();


signals:
    void describe_by_email_signal(Ticket ticket);
    void logTxt(QString txt);




};



#endif // BILLINGMODULE_H
