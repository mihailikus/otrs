#include "wizard.h"
#include "ui_wizard.h"

Wizard::Wizard(LoginConfig otrsConfig,
               LoginConfig billConfig,
               QString header,
               QString footer,
               QString text,
               QWidget *parent) :
    QWizard(parent),
    ui(new Ui::Wizard)
{
    ui->setupUi(this);

    editOtrsLogin   = qFindChild<QLineEdit*>(this, "editOtrsLogin");
    editOtrsPass    = qFindChild<QLineEdit*>(this, "editOtrsPass");

    editBillLogin   = qFindChild<QLineEdit*>(this, "editBillLogin");
    editBillPass    = qFindChild<QLineEdit*>(this, "editBillPass");

    textEditHeader  = qFindChild<QTextEdit*>(this, "textEditHeader");
    textEditFooter  = qFindChild<QTextEdit*>(this, "textEditFooter");

    checkToSavePass = qFindChild<QCheckBox*>(this, "checkToSavePass");

    lbl             = qFindChild<QLabel*>(this, "lbl");

    otrscfg = otrsConfig;
    billcfg = billConfig;

    editOtrsPass->setEchoMode(QLineEdit::Password);
    editBillPass->setEchoMode(QLineEdit::Password);

    editOtrsLogin->setText(otrscfg.username);
    editOtrsPass->setText (otrscfg.userpass);

    editBillLogin->setText(billcfg.username);
    editBillPass->setText(billcfg.userpass);

    textEditHeader->setText(header);
    textEditFooter->setText(footer);

    lbl->setText(text);

}

Wizard::~Wizard()
{
    delete ui;
}

LoginConfig Wizard::getOtrsConfig() {
    LoginConfig otrs;
    otrs.username = editOtrsLogin->text();
    otrs.userpass = editOtrsPass->text();
    return otrs;
}

LoginConfig Wizard::getBillConfig() {
    LoginConfig bill;
    bill.username = editBillLogin->text();
    bill.userpass = editBillPass->text();
    return bill;
}

QString Wizard::getAnswerHeader() {
    return textEditHeader->toPlainText();
}

QString Wizard::getAnswerFooter() {
    return textEditFooter->toPlainText();
}

bool Wizard::toSavePasswords() {
    return checkToSavePass->isChecked();
}
