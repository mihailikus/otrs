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

            qDebug() << i++ << params << value;
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

        }
    }
    file->close();



    billConnection = new BillingModule(billConfig);
    connect(billConnection, SIGNAL(describe_by_email_signal(Ticket)),
            this, SLOT(work_on_bill_ticket_done(Ticket)));
    connect(billConnection, SIGNAL(logTxt(QString)),
            this, SLOT(on_logTxt(QString)));

}

void Checker::run() {
    if (stopValue && blockValue) {
        blockValue = 0;

        //запускаем таймер не раз в период, а на один раз, т.к. неизвестно время возврата из http-request
        QTimer::singleShot(2000, this, SLOT(check_new_tickets()));

    }
}

void Checker::connection_ready(int status) {
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
    //static int i = 0;
    //qDebug() << "checked " << i++;
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
        //QStringList tList =
        otrsConnection->get_current_tickets();
    }

}

void Checker::new_tickets_list_ready(QStringList lst) {
    //qDebug() << lst;
    Ticket ticket;
    ticket.isChecked = false;

    newList.clear();
    for (int i = 0; i<lst.count(); i++) {
        ticket.id = lst.at(i).toInt();
        newList.append(ticket);
    }

    curList = newList;

    //из нового (свежего) списка убрать те, которые уже есть в старом
    for (int i = 0; i<oldList.count(); i++) {
        int c = newList.count();
        for (int j = 0; j<c; j++) {
            //qDebug() << "i j c " << i << j << c << oldList.at(i).id << newList.at(j).id;
            if (oldList.at(i).id == newList.at(j).id) {
                newList.remove(j);
                j--;
                c--;
            }
        }
    }

    //qDebug() << "New list count " << curList.count() << oldList.count();


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
                oldList.remove(i);
                i--;
                c--;
            }
        }
    }


    //qDebug() << "New tickets count: " << newList.count();

    if (newList.count()) {
        currentTicket = 0;
        this->work_on_ticket_progress();
    } else {
        new_ticketes_checked();
    }

}
void Checker::work_on_ticket_progress() {
    currentId = newList.at(currentTicket).id;
    otrsConnection->describe_ticket(currentId);
}

void Checker::work_on_ticket_done(Ticket ticket) {

    ticket.id =currentId;
    newList[currentTicket] = ticket;

    currentTicket++;

    emit new_ticket(ticket);

    if (currentTicket<newList.count()) {
        work_on_ticket_progress();
    }
    else {
      new_ticketes_checked();
    }


}



void Checker::new_ticketes_checked() {
    //и наконец - в старый список добавить новые тикеты
    oldList += newList;

    newList.clear();
    work_on_bill_ticket();
    this->run();

}

Checker::~Checker() {

}

void Checker::stop() {
    stopValue = 0;
}


void Checker::work_on_bill_ticket() {
    if (workInBill) return;
    int j = 1;
    for (int i = 0; i<oldList.count(); i++) {
        j++;
        if (!oldList.at(i).isChecked && !workInBill) {
            workInBill = true;
            workInBillId = oldList.at(i).id;
            workInBillNumber = i;
            if (oldList[i].host.startsWith("host")) {
                billConnection->describe_ticket_by_host(oldList[i]);
            } else {
                billConnection->describe_ticket_by_email(oldList[i]);
            }
        }
    }
    if (j == oldList.count())
        workInBill = true;

}

void Checker::work_on_bill_ticket_done(Ticket ticket) {
    for (int i = 0; i<oldList.count(); i++) {
        if (oldList[i].mail == ticket.mail) {
            oldList[i].isChecked = true;
            oldList[i].isMailinBase = ticket.isMailinBase;
            oldList[i].money = ticket.money;
            oldList[i].server = ticket.server;
            oldList[i].host = ticket.host;

            emit update_ticket(oldList[i]);
        }

    }

    workInBill = false;
    work_on_bill_ticket();
}

void Checker::on_logTxt(QString txt) {
    emit logText(txt);
}
