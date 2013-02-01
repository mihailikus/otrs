#include "OtrsWorker.h"

#include <QDebug>


OtrsWorker::OtrsWorker (LoginConfig otrs) :
    OtrsModule(otrs)
{
    qDebug() << "worker started";
    isWorking = false;

}

void OtrsWorker::delTicket(int id) {
    qDebug() << "Ticket with ID " << id;

}

void OtrsWorker::spamTicket(int id) {
    qDebug() << "Move to spam: " << id;
    tickets << id;
    if (!isWorking) {
        isWorking = true;
        typeOfWork = "spam";
        connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(ticket_described_for_ChallengeToken(QNetworkReply*)));
        this->describe_ticket(tickets.first());
    }

}

void OtrsWorker::answTicket(int id) {
    qDebug() << "Answwer: " << id;

}


void OtrsWorker::ticket_described_for_ChallengeToken(QNetworkReply *rpl) {
    disconnect(SIGNAL(finished(QNetworkReply*)));
    Ticket ticket;
    ticket.id = 777;
    ticket.server = 0;
    ticket.isChecked = false;
    ticket.body = "No message";

    QByteArray page = rpl->readAll();
    QString pg = page;

    pg.remove(0, pg.indexOf("ChallengeToken"));
    pg.remove(0, pg.indexOf("=\"")+2);
    pg.remove(pg.indexOf("\""), pg.length());

    qDebug() << pg;

    if (typeOfWork == "spam") {
        move_to_spam(tickets.first(), pg);
    }

}

void OtrsWorker::move_to_spam(int Id, QString challange) {

    connect(this, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(work_ready(QNetworkReply*)));

    QString url = "http://77.234.201.87/otrs/index.pl?ChallengeToken={challange}&Action=AgentTicketMove&QueueID=7&TicketID={ID}&DestQueueID=6";

    url.replace("{challange}", challange);
    url.replace("{ID}", QString::number(Id));

    qDebug() << "url: " << url;

    this->get(QNetworkRequest(QUrl(url)));




}


void OtrsWorker::work_ready(QNetworkReply *rpl) {
    disconnect(SIGNAL(finished(QNetworkReply*)));

    tickets.removeFirst();
    isWorking = false;
}
