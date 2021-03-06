#ifndef TICKET_H
#define TICKET_H
#include <QString>
#include <QDateTime>

class Ticket {
public:
    QString number;
    bool isChecked; //проверен ли уже автор тикета по базе
    int id;
    QDateTime time; //время создания тикета
    QString host;
    QString mail;
    int server;

    bool isMailinBase;

    QString userName;
    float   money;  //текущий баланс
    float   paid;   //сколько денег было оплачено
    bool    isAccBlocked;

    QString subject;
    QString body;
    int     articleID;

    bool operator < (const Ticket &other) const
    {
        if (time < other.time)
          return true;

        return false;
    }

    //для мониторинга залежалых тикетов
    bool isRemoved;
    QTime updateTime;

};


#endif // TICKET_H
