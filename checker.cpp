#include "checker.h"


Checker::Checker (QObject *parent)
    : QObject (parent = 0)

{
    stopValue = 1;
    blockValue = 1;

    workOnTicket = false;
    otrsConnected = false;

    workInBill = false;

    oldList.clear();
    curList.clear();
    newList.clear();

    //читаем файл конфигурации config.ini
    otrsConfig.uri = "http://77.234.201.87/otrs/index.pl";
    otrsConfig.post ="Action=Login&RequestedURL=&Lang=ru&TimeOffset=-240&User={User}&Password={Password}";
    otrsConfig.username = "";
    otrsConfig.userpass = "";

    billConfig.uri = "http://{User}:{Password}@billing.hostland.ru/info/public/index.php?account=&something={email}&status=1/";
    billConfig.post = "";
    billConfig.username = "";
    billConfig.userpass = "";

    QFile *file = new QFile("./config.ini");
    if (file->open(QFile::ReadOnly))
    {
    //читаем название организации из файла
    QString name;
    QByteArray line;
    while (!file->atEnd()) {
        line = file->readLine();
        if (line.at(0) != '#') {
            if (line.contains("otrs.username")) {
                name = line;
                if (name.split("=").count()>1) {
                    name = name.split("=").at(1).trimmed();
                    otrsConfig.username = name;
                }
            }
            if (line.contains("otrs.userpass")) {
                name = line;
                if (name.split("=").count()>1) {
                    name = name.split("=").at(1).trimmed();
                    otrsConfig.userpass = name;
                }
            }
            if (line.contains("bill.username")) {
                name = line;
                if (name.split("=").count()>1) {
                    name = name.split("=").at(1).trimmed();
                    billConfig.username = name;
                }
            }
            if (line.contains("bill.userpass")) {
                name = line;
                if (name.split("=").count()>1) {
                    name = name.split("=").at(1).trimmed();
                    billConfig.userpass = name;
                }
            }

        }
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
    workOnTicket = true;

    currentId = newList.at(currentTicket).id;
    otrsConnection->describe_ticket(currentId);
}

void Checker::work_on_ticket_done(Ticket ticket) {

    ticket.id =currentId;
    newList[currentTicket] = ticket;

    currentTicket++;
    workOnTicket = false;

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
        //qDebug() << "i=" << i;
        j++;
        if (!oldList.at(i).isChecked && !workInBill) {
            //qDebug() << oldList.at(i).id;
            workInBill = true;
            workInBillId = oldList.at(i).id;
            workInBillNumber = i;
            billConnection->describe_ticket_by_email(oldList[i]);
        }
    }
    if (j == oldList.count())
        workInBill = true;

}

void Checker::work_on_bill_ticket_done(Ticket ticket) {
    qDebug() << "Update ticket " << "server " + QString::number(ticket.server);
    for (int i = 0; i<oldList.count(); i++) {
        //qDebug() << "mail " << ticket.mail << oldList[i].mail;
        if (oldList[i].mail == ticket.mail) {
            oldList[i].isChecked = true;
            oldList[i].isMailinBase = ticket.isMailinBase;
            oldList[i].money = ticket.money;
            oldList[i].server = ticket.server;

            //ticket.id = oldList[i].id;
            emit update_ticket(oldList[i]);
        }

    }

    //oldList[workInBillNumber].isChecked = true;
    //ticket.id = workInBillId;
    //emit update_ticket(ticket);
    workInBill = false;
    work_on_bill_ticket();
}

void Checker::on_logTxt(QString txt) {
    //qDebug() << "LOG1" << txt;
    emit logText(txt);
}
