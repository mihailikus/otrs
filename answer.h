#ifndef ANSWER_H
#define ANSWER_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QScrollBar>
//#include <QTextBrowser>

#include "ticket.h"

namespace Ui {
class answer;
}

class answer : public QDialog
{
    Q_OBJECT
    
public:
    explicit answer(Ticket ticket,
                    QString header = "",
                    QString footer = "",
                    QString action = "",
                    QWidget *parent = 0);
    ~answer();

public slots:
    QString getSubject();
    QString getBody();
    
private:
    Ui::answer *ui;

    QLabel    *ticketSubject, *mailtoLabel;
    QLineEdit *editSubject;
    QTextEdit *editBody;

    //QTextBrowser *editBrowser;
};

#endif // ANSWER_H
