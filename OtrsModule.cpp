#include "OtrsModule.h"
#include <QDebug>


OtrsModule::OtrsModule (LoginConfig cfg) {

    QString postData = cfg.post;
    postData.replace("{User}", cfg.username);
    postData.replace("{Password}", cfg.userpass);

    requestUrl = cfg.uri2;
    zoomUrl    = cfg.zoom;

    otrsCurrentPage = 1;

    allPages = false;

    QNetworkRequest request;
    request.setUrl(QUrl(cfg.uri));

    QByteArray bytes;
    bytes.append(postData);

    request.setRawHeader("Content-type", "application/x-www-form-urlencoded");

    connect(this, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(connected(QNetworkReply*)));
    this->post(request, bytes);

}
OtrsModule::~OtrsModule() {

}

void OtrsModule::connected(QNetworkReply* rpl) {

    QByteArray page = rpl->readAll();

    QString pg = page;

    if (pg.contains("302 Moved")) {
        //типа когда успешно залогинимся, поменяем
        emit connection_established(1);
        qDebug() << "Connected OK";
        disconnect(SIGNAL(finished(QNetworkReply*)));

    } else {
        emit connection_established(0);
    }

}


void OtrsModule::get_current_tickets() {
    connect(this, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(current_tickets_ready(QNetworkReply*)));

    QString url = requestUrl;
    url.replace("{Page}", QString::number(otrsCurrentPage));
    this->get(QNetworkRequest(QUrl(url)));

}

void OtrsModule::current_tickets_ready(QNetworkReply *rpl) {
    disconnect(SIGNAL(finished(QNetworkReply*)));
    QByteArray page = rpl->readAll();
    QString pg = page;
    tickets.clear();

    QStringList lines = pg.split("\n");
    for (int i = 0; i<lines.count(); i++) {
        if (lines.at(i).contains("Action=AgentTicketZoom&TicketID=") && lines.at(i).contains("onmouseover")) {
            tickets << lines.at(i).split("TicketID=").at(1).split("\"  onmouseover").at(0);
       }
    }

    otrsCurrentPage += 15;
    QString nextUrl;
    nextUrl = requestUrl;
    nextUrl.remove("http://");
    nextUrl.remove(0, nextUrl.indexOf("/"));
    nextUrl.replace("{Page}", QString::number(otrsCurrentPage));

    if (pg.contains(nextUrl)) {
        allPages = false;
    } else {
        otrsCurrentPage = 1;
        allPages = true;
    }


//    QString line;
//    tickets.clear();
//    bool currPage = true;
//    QString nextUrl;


//    while (!rpl->atEnd()) {
//        line = rpl->readLine();
//        if (line.contains("Action=AgentTicketZoom&TicketID=") && line.contains("onmouseover")) {
//            tickets << line.split("TicketID=").at(1).split("\"  onmouseover").at(0);
//       }


//        if (currPage) {
//            if (line.contains(nextUrl)) {
//                qDebug() << "Next url: " << nextUrl;
//                //qDebug() << 1;
//                currPage = false;
//                allPages = false;
//                //если содержится ссылка на следующую страницу - прибавляем номер страницы, иначе 1
//            } else {
//                qDebug() << "OLD url: " << nextUrl;
//                otrsCurrentPage = 1;
//            }
//        }

//    }
//    if (currPage)
//        allPages = true;

    emit ticket_list_ready(tickets);

}

void OtrsModule::describe_ticket(int id) {
    //qDebug() << "Desc id " << id;
    connect(this, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(ticket_described(QNetworkReply*)));
    QString url = zoomUrl + QString::number(id);
    this->get(QNetworkRequest(QUrl(url)));
}

void OtrsModule::ticket_described(QNetworkReply *rpl) {
    disconnect(SIGNAL(finished(QNetworkReply*)));
    Ticket ticket;
    ticket.id = 777;
    ticket.server = 0;
    ticket.isChecked = false;
    ticket.body = "No message";

    QByteArray page = rpl->readAll();

    QString pg = page;

    //определяем адрес отправителя
    QString sender = pg.remove(0, pg.indexOf("contentvalue"));
    sender = sender.remove(0, sender.indexOf("title=")+7);
    sender = sender.remove(sender.indexOf("\">"), sender.length()-1);
    if (sender.contains("&lt;")) {
        sender.remove(0, sender.indexOf("&lt;") + 4);
        sender.remove(sender.indexOf("&gt;"), sender.length()-1);
    }
    ticket.mail = sender;

    //определяем дату тикета
    pg = page;
    QString datetime = pg.remove(0, pg.indexOf("mainvalue\"> ")+12);
    datetime = datetime.remove(datetime.indexOf("</td>"), datetime.length()-1);
    ticket.time = QDateTime::fromString(datetime, "dd.MM.yyyy hh:mm:ss");

    //определяем хост тикета, если возможно
    pg = page;
    //тут надо бы с регулярными выражениями, чтоб искало и в скобках, и в пробелах
//    QString host = pg.remove(0, pg.indexOf(QRegExp("[( ]host")));
//    host = host.remove(host.indexOf(QRegExp("[ )]")), host.length()-1);
    QString host = pg.remove(0, pg.indexOf("(host")+1);
    host = host.remove(host.indexOf(")"), host.length()-1);
    if (host.length() > 12) host = "n/a";
    ticket.host = host;
    //qDebug() << "Host: " << ticket.host;

    //определяем сообщение в теле тикета
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    pg = codec->toUnicode(page);
    pg.remove(0, pg.indexOf("<div class=\"message\">"));
    pg.remove(pg.indexOf("</div"), pg.length()-1);
    ticket.body = pg;

    //определяем тему тикета
    pg = codec->toUnicode(page);
    pg.remove(0, pg.indexOf("class=\"mainhead\">")+18);
    pg.remove(0, pg.indexOf("\n"));
    pg.remove(pg.indexOf("</td>")-1, pg.length()-1);
    ticket.subject = pg.trimmed();
    //qDebug() << "subject: " << ticket.subject;

    emit ticket_described_signal(ticket);
}

bool OtrsModule::isAllPages() {
    //qDebug() << "All pages " << allPages;
    return allPages;
}
