#include <QDebug>
#include "timeChecker.h"

TimeChecker::TimeChecker (LoginConfig otrs, MysqlConfig sql) :
    OtrsModule(otrs)
{
    qDebug() << "Time checker created " << otrs.hist;
    otrsconfig = otrs;
    dbconfig = sql;

    //устанавливаем соединение с базой данных
    qDebug() << "Connection to mysql";
    db = QSqlDatabase::addDatabase("QMYSQL", "my_mysql");
    qDebug() << "settings 555";
    db.setHostName(dbconfig.dbHost);
    db.setDatabaseName(dbconfig.dbName);
    db.setUserName(dbconfig.dbUser);
    db.setPassword(dbconfig.dbPass);
    qDebug() << "settings 8989" << dbconfig.dbPass;

    if (db.open()) {
        this->db_is_ready = true;
        qDebug() << "connection established";

    } else {
        this->db_is_ready = false;
    }

}

void TimeChecker::getLastTime() {
    if (!db_is_ready) {
        //db is not ready - return;
        return;
    }

    if (!tickets.count()) {
        qDebug() << "No more work - exit";
        return;
    }



    currentID = tickets.first();

    connect(this, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(lastTimeReady(QNetworkReply *)));
    QString url = otrsconfig.hist + QString::number(currentID);
    this->get(QNetworkRequest(QUrl(url)));
}

void TimeChecker::getLastTime(Ticket ticket) {
    tickets << ticket.id;
    this->getLastTime();
}

void TimeChecker::getLastTime(int id) {
    tickets << id;
    this->getLastTime();
}

void TimeChecker::lastTimeReady(QNetworkReply * rpl) {
    disconnect(SIGNAL(finished(QNetworkReply*)));

    tickets.removeFirst();
    QByteArray page = rpl->readAll();

    QString pg = page;

    //определяем время последнего доступа к тикету
    pg.remove(pg.lastIndexOf("contenthead"),pg.length()-1);
    pg.remove(0, pg.lastIndexOf("<td>")+5);
    QString time = pg.trimmed().split("\n").at(0);
    QDateTime ticketTime = QDateTime::fromString(time, "dd.MM.yyyy hh:mm:ss");

//    //определяем время создания тикета
//    pg = page;
//    pg.remove(0, pg.indexOf("searchactive"));
//    pg.remove(0, pg.indexOf("<div")+4);
//    pg.remove(0, pg.indexOf("<div")+4);
//    pg.remove(0, pg.indexOf("<td>"+5));
//    pg.remove(pg.indexOf("<td>")-1);
//    time = pg.trimmed();
//    qDebug() << time;
//    QDateTime createTime = QDateTime::fromString(time, "dd.MM.yyyy hh:mm:ss");


//    qDebug() << "Time: " << time << createTime << ticketTime;

    //работаем с базой данных
    QString query = "SELECT id, last FROM tickets WHERE id=" + QString::number(currentID) + " LIMIT 1";
    //qDebug() << "query = " << query;
    QSqlQuery res = db.exec(query);
    if (res.next()) {
        //QString id = res.value(0).toString();
        QString tm = res.value(1).toString();
        QDateTime savedTime = QDateTime::fromString(tm, "yyyy-MM-ddTHH:mm:ss"); //dd.MM.yyyy HH:mm:ss")
        //qDebug() << "id tm " << id << tm << savedTime << ticketTime;

        if (ticketTime > savedTime) {
            //значит, тикет обновлялся - надо его в базе обновить
            int delta = savedTime.secsTo(QDateTime::currentDateTime());
            query = "UPDATE `{dbName}`.`{table}` SET `last` = DATE_FORMAT('{date}', '%Y-%m-%d %H:%i:%s'), `delta` = '{delta}' WHERE `{table}`.`id` = {ID}";
            query.replace("{dbName}", dbconfig.dbName);
            query.replace("{table}", dbconfig.table);
            query.replace("{date}", ticketTime.toString("yyyy-MM-dd hh:mm:ss"));
            query.replace("{ID}", QString::number(currentID));
            query.replace("{delta}", QString::number(delta));
            //qDebug() << "1sql = " << query;
            db.exec(query);

        } else {
            //просто обновить разницу во времени
            query = "UPDATE `{dbName}`.`{table}` SET `delta` = '{delta}' WHERE `{table}`.`id` = {ID}";
            query.replace("{dbName}", dbconfig.dbName);
            query.replace("{table}", dbconfig.table);
            query.replace("{ID}", QString::number(currentID));
            int delta = ticketTime.secsTo(QDateTime::currentDateTime());
            query.replace("{delta}", QString::number(delta));
            //qDebug() << "2sql = " << query;
            db.exec(query);
        }

    } else {
        //если такого тикета еще нет в базе данных

        int delta = ticketTime.secsTo(QDateTime::currentDateTime());


        query = "INSERT INTO `{dbName}`.`{table}` (`id`, `last`, `delta`) VALUES ('{ID}', DATE_FORMAT('{date}', '%Y-%m-%d %H:%i:%s'), '{delta}')";
        query.replace("{dbName}", dbconfig.dbName);
        query.replace("{table}", dbconfig.table);
        query.replace("{date}", ticketTime.toString("yyyy-MM-dd hh:mm:ss"));
        query.replace("{ID}", QString::number(currentID));
        query.replace("{delta}", QString::number(delta));
        //qDebug() << "insert = " << query;
        db.exec(query);

    }

    getLastTime();

}

void TimeChecker::moveTicket(int id) {
    //qDebug() << "Ticket to move to archive: " << id;

    QString query = "SELECT last, delta FROM tickets WHERE id=" + QString::number(id) + " LIMIT 1";
    //qDebug() << "query = " << query;
    QSqlQuery res = db.exec(query);
    if (res.next()) {
        int deltaOld = res.value(1).toInt();
        QDateTime savedTime = res.value(0).toDateTime();
        qDebug () << "Delta " << deltaOld;
        int delta = savedTime.secsTo(QDateTime::currentDateTime());
        qDebug () << "Delta " << delta;

        QString query = "DELETE FROM `{dbName}`.`{table}` WHERE `{table}`.`id` = {ID}";
        query.replace("{dbName}", dbconfig.dbName);
        query.replace("{table}", dbconfig.table);
        query.replace("{ID}", QString::number(id));
        query.replace("{delta}", QString::number(delta));
        //qDebug() << "Del sql = " << query;
        db.exec(query);

        query = "INSERT INTO `{dbName}`.`{table}_hist` (`id`, `date`, `delta`) VALUES ('{ID}', CURRENT_TIMESTAMP, '{delta}')";
        query.replace("{dbName}", dbconfig.dbName);
        query.replace("{table}", dbconfig.table);
        query.replace("{ID}", QString::number(id));
        query.replace("{delta}", QString::number(delta));
        //qDebug() << "move sql = " << query;
        db.exec(query);

    }




}

void TimeChecker::moveTicket(Ticket ticket) {
    this->moveTicket(ticket.id);
}
