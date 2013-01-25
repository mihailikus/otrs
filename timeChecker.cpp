#include <QDebug>
#include "timeChecker.h"

TimeChecker::TimeChecker (LoginConfig otrs, MysqlConfig sql) :
    OtrsModule(otrs)
{
    ///отправляем в ОТРС конфиг для подключения
    ///подключаемся к базе данных

    otrsconfig = otrs;
    dbconfig = sql;

    //устанавливаем соединение с базой данных
    db = QSqlDatabase::addDatabase("QMYSQL", "my_mysql");
    db.setHostName(dbconfig.dbHost);
    db.setDatabaseName(dbconfig.dbName);
    db.setUserName(dbconfig.dbUser);
    db.setPassword(dbconfig.dbPass);

    if (db.open()) {
        this->db_is_ready = true;

    } else {
        this->db_is_ready = false;
    }
}

void TimeChecker::getLastTime() {
    ///проверяем очередь запрос на поиск времени последней работы с тикетом (из истории)
    if (!db_is_ready) {
        //db is not ready - return;
        return;
    }

    if (!tickets.count()) {
        return;
    }

    currentID = tickets.first();

    connect(this, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(lastTimeReady(QNetworkReply *)));
    QString url = otrsconfig.hist + QString::number(currentID);
    this->get(QNetworkRequest(QUrl(url)));
}

void TimeChecker::getLastTime(Ticket ticket) {
    ///это перегруженная функция, предоставлена для удобства. Ставит тикет в очередь на запрос
    tickets << ticket.id;
    this->getLastTime();
}

void TimeChecker::getLastTime(int id) {
    ///это перегруженная функция, предоставлена для удобства.
    ///Ставит тикет с идентификатором id в очередь на запрос
    tickets << id;
    this->getLastTime();
}

void TimeChecker::lastTimeReady(QNetworkReply * rpl) {
    ///получен ответ из ОТРС с историей тикета
    disconnect(SIGNAL(finished(QNetworkReply*)));

    tickets.removeFirst();
    QByteArray page = rpl->readAll();

    QString pg = page;

    //определяем время последнего доступа к тикету
    pg.remove(pg.lastIndexOf("contenthead"),pg.length()-1);
    pg.remove(0, pg.lastIndexOf("<td>")+5);
    QString time = pg.trimmed().split("\n").at(0);
    QDateTime ticketTime = QDateTime::fromString(time, "dd.MM.yyyy hh:mm:ss");

    //работаем с базой данных
    QString query = "SELECT id, last FROM tickets WHERE id=" + QString::number(currentID) + " LIMIT 1";
    QSqlQuery res = db.exec(query);
    if (res.next()) {
        QString tm = res.value(1).toString();
        QDateTime savedTime = QDateTime::fromString(tm, "yyyy-MM-ddTHH:mm:ss"); //dd.MM.yyyy HH:mm:ss")

        if (ticketTime > savedTime) {
            //значит, тикет обновлялся - надо его в базе обновить
            int delta = savedTime.secsTo(QDateTime::currentDateTime());
            query = "UPDATE `{dbName}`.`{table}` SET `last` = DATE_FORMAT('{date}', '%Y-%m-%d %H:%i:%s'), `delta` = '{delta}' WHERE `{table}`.`id` = {ID}";
            query.replace("{dbName}", dbconfig.dbName);
            query.replace("{table}", dbconfig.table);
            query.replace("{date}", ticketTime.toString("yyyy-MM-dd hh:mm:ss"));
            query.replace("{ID}", QString::number(currentID));
            query.replace("{delta}", QString::number(delta));
            db.exec(query);

        } else {
            //просто обновить разницу во времени
            query = "UPDATE `{dbName}`.`{table}` SET `delta` = '{delta}' WHERE `{table}`.`id` = {ID}";
            query.replace("{dbName}", dbconfig.dbName);
            query.replace("{table}", dbconfig.table);
            query.replace("{ID}", QString::number(currentID));
            int delta = ticketTime.secsTo(QDateTime::currentDateTime());
            query.replace("{delta}", QString::number(delta));
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
        db.exec(query);
    }
    getLastTime();
}

void TimeChecker::moveTicket(int id) {
    ///получен запрос на перемещение тикета из текущей очереди в историю

    QString query = "SELECT last, delta FROM tickets WHERE id=" + QString::number(id) + " LIMIT 1";
    QSqlQuery res = db.exec(query);
    if (res.next()) {
        QDateTime savedTime = res.value(0).toDateTime();
        int delta = savedTime.secsTo(QDateTime::currentDateTime());

        QString query = "DELETE FROM `{dbName}`.`{table}` WHERE `{table}`.`id` = {ID}";
        query.replace("{dbName}", dbconfig.dbName);
        query.replace("{table}", dbconfig.table);
        query.replace("{ID}", QString::number(id));
        query.replace("{delta}", QString::number(delta));
        db.exec(query);

        query = "INSERT INTO `{dbName}`.`{table}_hist` (`id`, `date`, `delta`) VALUES ('{ID}', CURRENT_TIMESTAMP, '{delta}')";
        query.replace("{dbName}", dbconfig.dbName);
        query.replace("{table}", dbconfig.table);
        query.replace("{ID}", QString::number(id));
        query.replace("{delta}", QString::number(delta));
        db.exec(query);
    }
}

void TimeChecker::moveTicket(Ticket ticket) {
    ///это перегруженная функция. отправляет запрос на перемещение тикета
    this->moveTicket(ticket.id);
}
