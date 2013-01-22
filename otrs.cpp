#include "otrs.h"
//#include "ui_otrs.h"

otrs::otrs(QWidget *parent) :
    QMainWindow(parent)
{
    this->resize(1024, 768);

    logView = new QTextBrowser();
    tickView =new QTextBrowser();

    make_actions();
    make_menu();
    make_tool_bar();

    make_central_widget();
    make_status_bar();

    otrsChecker = new Checker(this);
    connect(otrsChecker, SIGNAL(connected_success()), SLOT(on_connected()));
    connect(otrsChecker, SIGNAL(new_ticket(Ticket)), SLOT(on_newTicket(Ticket)));
    connect(otrsChecker, SIGNAL(remove_ticket(Ticket)), SLOT(on_delTicket(Ticket)));
    connect(otrsChecker, SIGNAL(update_ticket(Ticket)), this, SLOT(updateTicket(Ticket)));
    connect(otrsChecker, SIGNAL(logText(QString)), this, SLOT(on_logUpdate(QString)));

    icon = new QSystemTrayIcon(QIcon(":/share/images/resources/logo.png"), this);

    QClipboard *pcb = QApplication::clipboard();
    connect (pcb, SIGNAL(dataChanged()), this, SLOT(on_clipboard_changed()));

    icon->show();

    otrsChecker->run();

}

void otrs::make_actions() {
    //выход из программы
    action_exit = new QAction(QIcon(":/share/images/resources/button_cancel.png"),
                             tr("Exit"), this);
    action_exit->setShortcut(QKeySequence("Ctrl+Q"));
    connect(action_exit, SIGNAL(triggered()), SLOT(on_action_exit()));

    action_logo = new QAction(QIcon(":/share/images/resources/logo.png"),
                              tr("Logo"), this);

    action_log = new QAction(tr("LOG"), this);
    action_log->setShortcut(QKeySequence("Ctrl+L"));
    action_log->setCheckable(true);
    connect(action_log, SIGNAL(triggered(bool)), SLOT(on_action_log(bool)));
    action_log->setChecked(false);
    on_action_log(false);

    action_tray = new QAction(tr("Tray"), this);
    action_tray->setShortcut(QKeySequence("Ctrl+T"));
    action_tray->setCheckable(true);
    action_tray->setChecked(true);



}

void otrs::make_menu() {
    menuBar = new QMenuBar();

    menu = menuBar->addMenu(tr("File"));
    menu->addAction(action_exit);

    this->setMenuBar(menuBar);
}

void otrs::make_tool_bar() {
    toolBar = new QToolBar(tr("Main toolbar"));
    toolBar->addAction(action_logo);
    toolBar->addAction(action_log);
    toolBar->addAction(action_tray);
    toolBar->addSeparator();
    toolBar->addAction(action_exit);
    this->addToolBar(toolBar);

}

void otrs::make_central_widget() {

    ui_tableWidget = new QTableWidget(0, 6);
    connect(ui_tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(on_mouse_click(int, int)));

    mainLayout = new QGridLayout();
    mainLayout->addWidget(ui_tableWidget, 0, 0);
    mainLayout->addWidget(logView);
    mainLayout->addWidget(tickView);

    tickView->setVisible(false);

    mainWidget = new QWidget();
    mainWidget->setLayout(mainLayout);

    this->setCentralWidget(mainWidget);

    //-----------------------------------
    //устанавливаем заголовки таблицы
    QStringList fields;
    fields << tr("ID")
           << tr("Date")
           << tr("Host")
           << tr("E-mail")
           << tr("in Base")
           << tr("Money")
           << tr("Server");

    ui_tableWidget->setColumnCount(fields.count());

    ui_tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    //устанавливаем заголовки главной таблицы
    QStandardItemModel *model = new QStandardItemModel(5,5, this);
    QTableView *tableView = new QTableView;
    tableView->setModel(model);
    QHeaderView *tableHeader = new QHeaderView(Qt::Horizontal, tableView);
    QStandardItemModel *header_model_1 = new QStandardItemModel(0,0, tableHeader);
    header_model_1->setHorizontalHeaderLabels(fields);

    tableHeader->setModel(header_model_1);
    ui_tableWidget->setHorizontalHeader(tableHeader);

    int w = ui_tableWidget->width() / fields.count();

    for (int i=0; i<fields.count(); i++) {
        ui_tableWidget->horizontalHeader()->resizeSection(i, 130);
            //qDebug() << "size = " << w;
    }
}

void otrs::make_status_bar() {
    ui_bar = new QStatusBar();
    this->setStatusBar(ui_bar);

}

otrs::~otrs()
{

}

void otrs::on_connected() {
    QLabel *lbl = new QLabel(tr("Connected"));
    ui_bar->addWidget(lbl);
}

void otrs::on_newTicket(Ticket ticket) {
    int row = ui_tableWidget->rowCount();
    ui_tableWidget->setRowCount(row+1);

    QTableWidgetItem* item = new QTableWidgetItem(QString::number(ticket.id));
    ui_tableWidget->setItem(row, 0, item);

    item = new QTableWidgetItem(ticket.time.toString(tr("d MMMM yyyy, hh.mm")));
    ui_tableWidget->setItem(row, 1, item);

    item = new QTableWidgetItem(ticket.host);
    ui_tableWidget->setItem(row, 2, item);

    item = new QTableWidgetItem(ticket.mail);
    ui_tableWidget->setItem(row, 3, item);

    item = new QTableWidgetItem(" ");
    ui_tableWidget->setItem(row, 4, item);

    item = new QTableWidgetItem(ticket.isChecked);
    ui_tableWidget->setItem(row, 5, item);

    item = new QTableWidgetItem(" ");
    ui_tableWidget->setItem(row, 6, item);

    if (action_tray->isChecked())
        icon->showMessage(tr("OTRS"), tr("New ticket ready"));

    ticketList[ticket.id] = ticket;

}

void otrs::on_delTicket(Ticket ticket) {
    int ro = ui_tableWidget->rowCount();
    for (int i = 0; i<ro; i++) {
        if (ui_tableWidget->item(i, 0)->text().toInt() == ticket.id)
        {
            on_logUpdate("Remove ticket: " + QString::number(ticket.id) + " " + ticket.host);
            ui_tableWidget->removeRow(i);
            ro--;
            i--;
        }
    }
}

void otrs::updateTicket(Ticket ticket) {
    QString text;
    QString m1 = tr("in base");
    QString m2 = tr("NOT in BASE");
    QTableWidgetItem* itemMail = new QTableWidgetItem("status");
    QTableWidgetItem* itemMoney = new QTableWidgetItem(QString::number(ticket.money));
    QTableWidgetItem* itemServer = new QTableWidgetItem(QString::number(ticket.server));
    QTableWidgetItem* itemHost = new QTableWidgetItem(ticket.host);


    for (int i = 0; i<ui_tableWidget->rowCount(); i++) {
         text = ui_tableWidget->item(i, 0)->text();
         if (text.toInt() == ticket.id) {
             if (ticket.isMailinBase)
                 itemMail->setText(m1);
             else
                 itemMail->setText(m2);
            ui_tableWidget->setItem(i, 2, itemHost);
            ui_tableWidget->setItem(i, 4, itemMail);
            ui_tableWidget->setItem(i, 5, itemMoney);
            ui_tableWidget->setItem(i, 6, itemServer);
         }
    }
}

void otrs::on_clipboard_changed() {
    QClipboard *clipboard = QApplication::clipboard();
    QString originalText = clipboard->text();
    QString supertxt;
    bool confirm = false;
    int i=0;
    while (!confirm && i<ui_tableWidget->rowCount()) {
        supertxt = "";
        confirm = false;
        for (int j = 0; j<ui_tableWidget->columnCount(); j++) {
            QString txt = ui_tableWidget->item(i, j)->text();
            supertxt += txt + "\n";
            if (txt == originalText) {
                confirm = true;
            }
        }
        i++;
    }
    if (confirm) {
        ui_tableWidget->selectRow(i-1);
        if (action_tray->isChecked())
            icon->showMessage("OTRS", supertxt);
    }

}


void otrs::on_action_exit() {
    close();
}


void otrs::on_logUpdate(QString txt) {
    logView->insertHtml(txt + "<br>\n");
    logView->scrollToAnchor(txt);
}

void otrs::on_action_log(bool status) {
    if (status) {
        logView->show();
        tickView->hide();
    } else {
        logView->hide();
    }
}

void otrs::on_mouse_click(int x, int y) {
    if (y) {
        tickView->setVisible(false);
        return;
    }

    on_action_log(false);
    action_log->setChecked(false);

    tickView->setVisible(true);

    int id = ui_tableWidget->item(x, y)->text().toInt();
    QString txt;
    txt = "<b>" + ticketList[id].subject + "</b><br>\n";
    txt += ticketList[id].body;
   tickView->clear();
    tickView->insertHtml(txt);

}
