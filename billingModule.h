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
    QString url;
    QString mail;
    QString host;
    QString id;

public slots:
    void describe_ticket_by_email(Ticket ticket);

signals:
    void describe_by_email_signal(Ticket ticket);
    void logTxt(QString txt);




};



#endif // BILLINGMODULE_H
