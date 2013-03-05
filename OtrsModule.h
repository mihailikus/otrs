#ifndef CONNECT_MODULE_H
#define CONNECT_MODULE_H
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QUrl>
#include <QStringList>
#include <QTextCodec>
#include "login_config.h"
#include "ticket.h"

class OtrsModule : public QNetworkAccessManager {
Q_OBJECT
public:
    OtrsModule (LoginConfig cfg);
    ~OtrsModule ();
    bool isAllPages();
    bool is_connected();
    void reconnect(LoginConfig cfg);


private:
    int otrsCurrentPage;
    QStringList tickets;
    QString requestUrl;
    QString zoomUrl;
    bool allPages;
    bool isConnected;


private slots:
    void connected(QNetworkReply *rpl);
    void current_tickets_ready(QNetworkReply *prl);
    void ticket_described(QNetworkReply *rpl);

public slots:
    void get_current_tickets();
    void describe_ticket(int id);

signals:
    void connection_established(int status);
    void ticket_list_ready(QStringList lst);
    void ticket_described_signal(Ticket ticket);


};


#endif // CONNECT_MODULE_H
