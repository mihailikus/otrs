#include "answer.h"
#include "ui_answer.h"

answer::answer(Ticket ticket, QString action, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::answer)
{
    ui->setupUi(this);

    ticketSubject = qFindChild<QLabel*>(this, "label");
    mailtoLabel = qFindChild<QLabel*>(this, "label_2");

    editSubject   = qFindChild<QLineEdit*>(this, "lineEdit");
    editBody   = qFindChild<QTextEdit*>(this, "textEdit");

    //editBrowser = qFindChild<QTextBrowser*>(this,"textBrowser");


    ticketSubject->setText(tr("Action: ") + action);
    mailtoLabel->setText(ticket.mail);
    editSubject->setText(tr("Re: ") + ticket.subject);

    QString body = ticket.body;
    body.replace("\n", "\n> ");
    body.replace("<br/>", "");
    body = tr("Hello! \n\n\n") + body + tr("\n\n==\nBest regards, \nMichael Volkov,\nTechnical support \n[Hostland.RU]\nhttp://www.hostland.ru/contacts/");
    editBody->setText(body);

    //editBrowser->setText(body);

    editBody->setFocus();
//    int pos = editBody->verticalScrollBar()->value();
//    editBody->verticalScrollBar()->setValue(pos+1);

}

answer::~answer()
{
    delete ui;
}

QString answer::getBody() {
    return editBody->toPlainText();
}

QString answer::getSubject() {

    return editSubject->text();
}
