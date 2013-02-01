#ifndef OTRS_H
#define OTRS_H

#include <QMainWindow>
#include <QApplication>
#include <QLabel>
#include <QStatusBar>
#include <QTableWidget>
#include <QClipboard>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QTextBrowser>
#include <QMap>
#include <QFile>
#include <QHeaderView>
#include "checker.h"
#include "OtrsWorker.h"

namespace Ui {
class otrs;
}

class otrs : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit otrs(QWidget *parent = 0);
    ~otrs();

public slots:
    void on_connected();
    void on_newTicket(Ticket ticket);
    void on_delTicket(Ticket ticket);
    void updateTicket (Ticket ticket);

    void on_logUpdate(QString txt);
    
private:
    QStatusBar *ui_bar;
    QTableWidget *ui_tableWidget;
    QTextBrowser *logView, *tickView;

    Checker *otrsChecker;

    QClipboard *pcb;

    QSystemTrayIcon *icon;

    QMenuBar *menuBar;
    QMenu *menu;
    QToolBar *toolBar;
    QGridLayout *mainLayout;
    QWidget *mainWidget;

    QAction *action_exit;
    QAction *action_logo;
    QAction *action_log;
    QAction *action_tray;

    //--------------
    QMenu *contextMenu;
    QAction *actionDel;
    QAction *actionSpam;
    QAction *actionAnswer;
    int      contextId;

    QMap<int, Ticket> ticketList;

    OtrsWorker *worker;

    LoginConfig otrsConfig, billConfig;
    MysqlConfig mysqlconfig;

private slots:
    void on_clipboard_changed();

    void make_central_widget();
    void make_status_bar();
    void make_actions();
    void make_menus();
    void make_tool_bar();

    void on_action_exit();
    void on_action_log(bool status);

    void on_mouse_click(int x, int y);

    void on_context_menu(QPoint point);
    void on_actionDel();
    void on_actionSpam();
    void on_actionAnswer();


};

#endif // OTRS_H
