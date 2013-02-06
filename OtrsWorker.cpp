#include "OtrsWorker.h"

#include <QDebug>


OtrsWorker::OtrsWorker (LoginConfig otrs) :
    OtrsModule(otrs)
{
    qDebug() << "worker started";
    isWorking = false;

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

void OtrsWorker::closeTicket(int id, QString text) {
    tickets << id;
    currText = text;
    if (!isWorking) {
        isWorking = true;
        typeOfWork = "close";
        //qDebug() << "The first " << tickets.first() << id;
        connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(ticket_described_for_closing(QNetworkReply*)));
        //this->describe_ticket(tickets.first());

        QString closeUrl = "http://77.234.201.87/otrs/index.pl?Action=AgentTicketClose&TicketID=";
        QString url = closeUrl + QString::number(tickets.first());
        this->get(QNetworkRequest(QUrl(url)));
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


    if (typeOfWork == "spam") {
        move_to_spam(tickets.first(), pg);
    }
//    if (typeOfWork == "close") {
//        close_ticket(tickets.first(), pg);
//    }

}

void OtrsWorker::ticket_described_for_closing(QNetworkReply *rpl) {
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

    QString token = pg;

    pg = page;
    pg.remove(0, pg.indexOf("FormID")+15);
    pg.remove(pg.indexOf("/")-1, pg.length()-1);

    qDebug() << "form ID" << pg << token;

//    if (typeOfWork == "spam") {
//        move_to_spam(tickets.first(), pg);
//    }
    if (typeOfWork == "close") {
        close_ticket(tickets.first(), token, pg);
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

void OtrsWorker::close_ticket(int Id, QString challange, QString formID) {

    QString Body = currText;
    QString subject = "Close";
    qDebug() << "Ticket " << Id << formID <<  " will be closed by reason: " << currText;

    QString url = "http://77.234.201.87/otrs/index.pl?";

    QString postData = "ChallengeToken={challange}&Action=AgentTicketClose&Subaction=Store&TicketID={ticketID}&Expand=&FormID={formID}&Subject={Close}&Body={Body}&file_upload=&ArticleTypeID=9&NewStateID=2&TimeUnits=.";
    postData.replace("{challange}", challange);
    postData.replace("{ticketID}",  QString::number(Id));
    postData.replace("{formID}",    formID);
    postData.replace("{Close}",     subject);
    postData.replace("{Body}",      Body);



    QNetworkRequest request;
    request.setUrl(QUrl(url));

    QByteArray bytes;
    bytes.append(postData);

    request.setRawHeader("Content-type", "application/x-www-form-urlencoded");

    connect(this, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(work_ready(QNetworkReply*)));
    this->post(request, bytes);
}


void OtrsWorker::work_ready(QNetworkReply *rpl) {
    disconnect(SIGNAL(finished(QNetworkReply*)));

    //QByteArray page = rpl->readAll();
    //QString pg = page;

    //qDebug() << pg;

    tickets.removeFirst();
    isWorking = false;
    emit unblocActions(true);
}
