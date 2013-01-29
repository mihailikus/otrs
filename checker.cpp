#define UPDATE_PERIOD 3 //период обновления в секундах

#include "checker.h"


Checker::Checker (QObject *parent)
    : QObject (parent = 0)

{
    stopValue = 1;
    blockValue = 1;

    otrsConnected = false;

    workInBill = false;

    oldList.clear();
    curList.clear();
    newList.clear();

    //читаем файл конфигурации config.ini
    QFile *file = new QFile("./config.ini");
    if (!file->open(QFile::ReadOnly)) {
        emit logText(tr("Cannot open config file config.ini"));
        return;
    }

    QByteArray line;
    QString params;
    QString value;
    int i = 0;
    while (!file->atEnd()) {
        line = file->readLine();
        if (line.at(0) != '#') {
            params = line;
            params.remove(0, params.indexOf("=")+1);
            params = params.trimmed();

            value = line;
            value.remove(value.indexOf("="), value.length()-1);
            value = value.trimmed();

            if (value == "otrs.username")
                otrsConfig.username = params;

            if (value == "otrs.url")
                otrsConfig.uri = params;

            if (value == "otrs.url2")
                otrsConfig.uri2 = params;

            if (value == "otrs.post")
                otrsConfig.post = params;

            if (value == "otrs.zoom")
                otrsConfig.zoom = params;

            if (value == "otrs.hist")
                otrsConfig.hist = params;

            if (value == "bill.url")
                billConfig.uri = params;

            if (value == "bill.url2")
                billConfig.uri2 = params;

            if (value == "otrs.userpass")
                otrsConfig.userpass = params;

            if (value == "bill.username")
                billConfig.username = params;

            if (value == "bill.userpass")
                billConfig.userpass = params;

            if (value == "dbHost")
                mysqlconfig.dbHost = params;

            if (value == "dbName")
                mysqlconfig.dbName = params;

            if (value == "dbUser")
                mysqlconfig.dbUser = params;

            if (value == "dbPass")
                mysqlconfig.dbPass = params;

            if (value == "table")
                mysqlconfig.table = params;

        }
    }
    file->close();

    billConnection = new BillingModule(billConfig);
    connect(billConnection, SIGNAL(describe_by_email_signal(Ticket)),
            this, SLOT(work_on_bill_ticket_done(Ticket)));
    connect(billConnection, SIGNAL(logTxt(QString)),
            this, SLOT(on_logTxt(QString)));

    timeChecker = new TimeChecker(otrsConfig, mysqlconfig);
}

void Checker::run() {
    ///запускаем таймер не раз в период, а на один раз, т.к. неизвестно время возврата из http-request

    if (stopValue && blockValue) {
        blockValue = 0;
        QTimer::singleShot(UPDATE_PERIOD*1000, this, SLOT(check_new_tickets()));

    }
}

void Checker::connection_ready(int status) {
    ///получен ответ из ОТРС об установлении соединения

    if (status == 1) {
        otrsConnected = true;
        qDebug() << "connected";
        this->run();
        emit connected_success();
    } else {
        qDebug() << "Cannot connect";
        emit logText(tr("Cannot connect. Check your config.ini file"));
    }
}

void Checker::check_new_tickets() {
    ///отправляем запрос в ОТРС на получение очередной страницы со списком тикетов

    blockValue = 1;

    if (!otrsConnected) {
        //если не подключено - запускаем модуль подключения
        otrsConnection = new OtrsModule(otrsConfig);
        connect(otrsConnection, SIGNAL(connection_established(int)), this,
                SLOT(connection_ready(int )));
        connect(otrsConnection, SIGNAL(ticket_list_ready(QStringList)), this,
                SLOT(new_tickets_list_ready(QStringList )));
        connect(otrsConnection, SIGNAL(ticket_described_signal(Ticket)), this,
                SLOT(work_on_ticket_done(Ticket)));

    } else {
        //выполняется, если ранее было установлено соединение
        otrsConnection->get_current_tickets();
    }

}

void Checker::new_tickets_list_ready(QStringList lst) {
    ///получен ответ из ОТРС со списком ID тикетов в текущей странице

    Ticket ticket;
    ticket.isChecked = false;

    tmpList.clear();
    for (int i = 0; i<lst.count(); i++) {
        ticket.id = lst.at(i).toInt();
        tmpList << ticket;
    }

    curList << tmpList;

    if (otrsConnection->isAllPages()) {
     //если проверены все страницы
        newList = curList;

        //из нового (свежего) списка убрать те, которые уже есть в старом
        for (int i = 0; i<oldList.count(); i++) {
            int c = newList.count();
            for (int j = 0; j<c; j++) {
                if (oldList.at(i).id == newList.at(j).id) {
                    timeChecker->getLastTime(oldList[i].id);
                    newList.remove(j);
                    j--;
                    c--;
                }
            }
        }

        //из старого списка убрать те, которых уже нет в новом
        if (!workInBill) {
            int c = oldList.count();
            for (int i = 0; i<c; i++) {
                bool exists = false;
                for (int j=0; j<curList.count(); j++) {
                    if (curList[j].id == oldList[i].id) {
                        exists = true;
                    }
                }
                if (!exists) {
                    emit remove_ticket (oldList[i]);
                    timeChecker->moveTicket(oldList[i]);
                    oldList.remove(i);
                    i--;
                    c--;
                }
            }

        }
        if (newList.count()) {
            currentTicket = 0;
            //new_ticketes_checked();
            this->work_on_ticket_progress();
        } else {
            new_ticketes_checked();
        }

    } else {
        //если проверены еще не все страницы
        this->run();
    }

}

void Checker::work_on_ticket_progress() {
    ///начинаем запрос на подробное описание тикета
    currentId = newList.at(currentTicket).id;
    otrsConnection->describe_ticket(currentId);
}

void Checker::work_on_ticket_done(Ticket ticket) {
    ///получен ответ с подробным описанием тикета
    ticket.id =currentId;
    newList[currentTicket] = ticket;

    billList << ticket;

    currentTicket++;

    emit new_ticket(ticket);
    timeChecker->getLastTime(currentId);

    if (currentTicket<newList.count()) {
        work_on_ticket_progress();
    }
    else {
      new_ticketes_checked();
    }
}

void Checker::new_ticketes_checked() {
    ///вызывается после обработки всех тикетов

    //и наконец - в старый список добавить новые тикеты
    oldList += curList;

    curList.clear();
    //qDebug() << "Clearing cur list";
    work_on_bill_ticket();
    this->run();

}

Checker::~Checker() {

}

void Checker::stop() {
    stopValue = 0;
}


void Checker::work_on_bill_ticket() {
    ///отправляем запросы в биллинг на описание тикета

    if (workInBill) return;
    if (!billConnection->is_connected()) return;
    int j = 1;
    for (int i = 0; i<billList.count(); i++) {
        j++;
        if (!billList.at(i).isChecked && !workInBill) {
            workInBill = true;
            workInBillId = billList.at(i).id;
            workInBillNumber = i;
            if (billList[i].host.startsWith("host")) {
                billConnection->describe_ticket_by_host(billList[i]);
            } else {
                billConnection->describe_ticket_by_email(billList[i]);
            }

        }
    }
    if (j == billList.count())
        workInBill = true;

}

void Checker::work_on_bill_ticket_done(Ticket ticket) {
    ///получен ответ из биллинга с подробностями

    for (int i = 0; i<billList.count(); i++) {
        if (billList[i].mail == ticket.mail) {
            billList[i].isChecked = true;
            billList[i].isMailinBase = ticket.isMailinBase;
            billList[i].money = ticket.money;
            billList[i].server = ticket.server;
            billList[i].host = ticket.host;

            emit update_ticket(billList[i]);
        }

    }

    workInBill = false;
    work_on_bill_ticket();
}

void Checker::on_logTxt(QString txt) {
    ///используется для логирования в главном окне програмы

    emit logText(txt);
}
