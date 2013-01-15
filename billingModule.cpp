#include "billingModule.h"
#include <QDebug>

BillingModule::BillingModule (LoginConfig cfg) {
    QNetworkRequest request;

    url = cfg.uri;
    url.replace("{User}", cfg.username);
    url.replace("{Password}", cfg.userpass);

    url2 = cfg.uri2;
    url2.replace("{User}", cfg.username);
    url2.replace("{Password}", cfg.userpass);


    QUrl bill = QUrl(url);

    request.setUrl(bill);

    connect(this, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(connected(QNetworkReply*)));
   this->get(request);
}

BillingModule::~BillingModule() {

}

void BillingModule::connected(QNetworkReply *) {
    disconnect(SIGNAL(finished(QNetworkReply*)));
}

void BillingModule::describe_ticket_by_email(Ticket ticket) {
    describeMethod = 1;
    QString url1 = url;
    host = ticket.host;
    mail = ticket.mail;
    id = QString::number(ticket.id);
    url1.replace("{email}", mail);
    QUrl bill = QUrl(url1);
    connect(this, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(describe_by_email_finished(QNetworkReply*)));

    this->get(QNetworkRequest(bill));
}

void BillingModule::describe_ticket_by_host(Ticket ticket) {
    describeMethod = 0;
    QString url1 = url2;
    host = ticket.host;
    mail = ticket.mail;
    id = QString::number(ticket.id);
    url1.replace("{host}", host);
    QUrl bill = QUrl(url1);
    connect(this, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(describe_by_email_finished(QNetworkReply*)));

    this->get(QNetworkRequest(bill));
}

void BillingModule::describe_by_email_finished(QNetworkReply *rpl) {
    disconnect(SIGNAL(finished(QNetworkReply*)));
    QByteArray page = rpl->readAll();
    QString pg = page;
    Ticket ticket;
    ticket.money = 0;
    ticket.server = -1;
    ticket.mail = mail;
    ticket.isChecked = true;

    if ( (!describeMethod && !pg.contains(mail))
            ||
         (describeMethod && !pg.contains("host=host"))            )
    {
        qDebug() << "email not found, method " << describeMethod;
        ticket.isMailinBase = false;
    } else {
        QString host1;
        int position = pg.indexOf("host=" + host);

        if (position == -1 ) {
            position = pg.indexOf("host=host");
        }

        host1 = pg.remove(0, position+5);
        host1.remove(host1.indexOf("\" "), host1.length()-1);

        ticket.host = host1;
        ticket.isMailinBase = true;

        //вычислим номер сервера, на котором находится аккаунт
        QString server = pg.remove(0, pg.indexOf("<td width=\"100px;\"><p>")+22);
        server.remove(server.indexOf(" "), server.length()-1);
        ticket.server = server.toInt();
        //emit logTxt("server " + QString::number(ticket.server));


        //узнаем баланс клиента
        QString money = pg.remove(0, pg.indexOf("<p style=\"font-weight: 700;\">")+29);
        money.remove(money.indexOf(" "), money.length()-1);
        emit logTxt("Ticket <a href=\"http://77.234.201.87/otrs/index.pl?Action=AgentTicketZoom&TicketID=" + id + "\">" + id + "</a> balans = " + money);
        ticket.money = money.toFloat();

    }
    emit describe_by_email_signal(ticket);
}
