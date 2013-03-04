#include "wizard.h"
#include "ui_wizard.h"

Wizard::Wizard(LoginConfig otrsConfig, LoginConfig billConfig, QWidget *parent) :
    QWizard(parent),
    ui(new Ui::Wizard)
{
    ui->setupUi(this);

//    editSubject   = qFindChild<QLineEdit*>(this, "lineEdit");
    editOtrsLogin   = qFindChild<QLineEdit*>(this, "editOtrsLogin");
    editOtrsPass    = qFindChild<QLineEdit*>(this, "editOtrsPass");

    editBillLogin   = qFindChild<QLineEdit*>(this, "editBillLogin");
    editBillPass    = qFindChild<QLineEdit*>(this, "editBillPass");

    otrscfg = otrsConfig;
    billcfg = billConfig;

    editOtrsLogin->setText(otrscfg.username);
    editOtrsPass->setText (otrscfg.userpass);

    editBillLogin->setText(billcfg.username);
    editBillPass->setText(billcfg.userpass);




}

Wizard::~Wizard()
{
    delete ui;
}
