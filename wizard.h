#ifndef WIZARD_H
#define WIZARD_H

#include <QWizard>
#include <QLineEdit>
#include <QTextEdit>

#include "login_config.h"

namespace Ui {
class Wizard;
}

class Wizard : public QWizard
{
    Q_OBJECT
    
public:
    explicit Wizard(LoginConfig otrsConfig, LoginConfig billConfig, QWidget *parent = 0);
    ~Wizard();
    
private:
    Ui::Wizard *ui;

    LoginConfig otrscfg, billcfg;
    QLineEdit *editOtrsLogin, *editOtrsPass, *editBillLogin, *editBillPass;
    QTextEdit *textEditHeader, *textEditFooter;


};

#endif // WIZARD_H
