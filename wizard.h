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
    explicit Wizard(LoginConfig otrsConfig,
                    LoginConfig billConfig,
                    QString     header,
                    QString     footer,
                    QWidget *parent = 0);
    ~Wizard();

    LoginConfig getOtrsConfig();
    LoginConfig getBillConfig();

    QString     getAnswerHeader();
    QString     getAnswerFooter();

    bool        toSavePasswords();
    
private:
    Ui::Wizard *ui;

    LoginConfig otrscfg, billcfg;
    QLineEdit *editOtrsLogin, *editOtrsPass, *editBillLogin, *editBillPass;
    QTextEdit *textEditHeader, *textEditFooter;


};

#endif // WIZARD_H
