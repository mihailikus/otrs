#include "otrs.h"
//#include "ui_otrs.h"

otrs::otrs(QWidget *parent) :
    QMainWindow(parent)
{
    ///создаем главное окно и инициализируем модуль проверки тикетов
    this->resize(1024, 768);

    logView = new QTextBrowser();
    tickView =new QTextBrowser();

    make_actions();
    make_menus();
    make_tool_bar();

    make_central_widget();
    make_status_bar();

    answerHeader = "";
    answerFooter = "";
    dontSavePasswords = false;

    load_settings("./config.ini");

    otrsChecker = new Checker(otrsConfig, billConfig, mysqlconfig, this);
    connect(otrsChecker, SIGNAL(connected_success(bool )), SLOT(on_connected(bool )));
    connect(otrsChecker, SIGNAL(new_ticket(Ticket)), SLOT(on_newTicket(Ticket)));
    connect(otrsChecker, SIGNAL(remove_ticket(Ticket)), SLOT(on_delTicket(Ticket)));
    connect(otrsChecker, SIGNAL(update_ticket(Ticket)), this, SLOT(updateTicket(Ticket)));
    connect(otrsChecker, SIGNAL(logText(QString)), this, SLOT(on_logUpdate(QString)));

    icon = new QSystemTrayIcon(QIcon(":/share/images/resources/logo.png"), this);

    QClipboard *pcb = QApplication::clipboard();
    connect (pcb, SIGNAL(dataChanged()), this, SLOT(on_clipboard_changed()));

    icon->show();

    otrsChecker->run();

    worker = new OtrsWorker(otrsConfig);
    connect(worker, SIGNAL(unblocActions(bool)), this, SLOT(blockActions(bool)));

    on_actionCheck();

}

void otrs::make_actions() {
    ///создаем действия

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
    action_tray->setChecked(false);

    actionSpam = new QAction(tr("Move to spam"), this);
    connect(actionSpam, SIGNAL(triggered()), SLOT(on_actionSpam()));

    actionAnswer = new QAction(tr("Answer to ticket"), this);
    connect(actionAnswer, SIGNAL(triggered()), SLOT(on_actionAnswer()));

    actionClose = new QAction(tr("Close ticket"), this);
    connect(actionClose, SIGNAL(triggered()), SLOT(on_actionClose()));

    actionBlock = new QAction(tr("Block ticket"), this);
    connect(actionBlock, SIGNAL(triggered()), SLOT(on_actionBlock()));

    actionRemove = new QAction(tr("Manual remove"), this);
    connect(actionRemove, SIGNAL(triggered()), SLOT(on_actionRemove()));

    action_wizard = new QAction(tr("Wizard config"), this);
    connect(action_wizard, SIGNAL(triggered()), SLOT(on_action_wizard()));

    action_save_settings = new QAction(tr("Save settings"), this);
    action_save_settings->setShortcut(QKeySequence("Shift+F12"));
    connect(action_save_settings, SIGNAL(triggered()), SLOT(on_action_save_settings()));

    actionCheck = new QAction(tr("Check old tickets"), this);
    connect(actionCheck, SIGNAL(triggered()), SLOT(on_actionCheck()));

}

void otrs::make_menus() {
    ///привязываем действия к главному меню
    menuBar = new QMenuBar();
    menu = menuBar->addMenu(tr("File"));
    menu->addAction(action_exit);

    menuEdit = menuBar->addMenu(tr("Edit"));
    menuEdit->addAction(action_save_settings);
    menuEdit->addSeparator();
    menuEdit->addAction(action_wizard);

    this->setMenuBar(menuBar);


    //контекстное меню
    contextMenu = new QMenu();
    contextMenu->addAction(actionBlock);
    contextMenu->addSeparator();
    contextMenu->addAction(actionClose);
    contextMenu->addAction(actionAnswer);
    contextMenu->addSeparator();
    contextMenu->addAction(actionSpam);
    contextMenu->addSeparator();
    contextMenu->addAction(actionRemove);

}

void otrs::make_tool_bar() {
    ///привязываем действия на панель инструментов
    toolBar = new QToolBar(tr("Main toolbar"));
    toolBar->addAction(action_logo);
    toolBar->addAction(action_log);
    toolBar->addAction(action_tray);
    toolBar->addSeparator();
    toolBar->addAction(actionCheck);
    toolBar->addSeparator();
    toolBar->addAction(action_exit);
    this->addToolBar(toolBar);
}

void otrs::make_central_widget() {
    ///создаем центральный виджет - таблицу, куда будут попадать тикеты

    ui_tableWidget = new QTableWidget(0, 6);
    connect(ui_tableWidget, SIGNAL(cellClicked(int,int)),
            this, SLOT(on_mouse_click(int, int)));
    connect(ui_tableWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(on_context_menu(QPoint )));

    ui_tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

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

    //будет использоваться для равномерного распределения колонок в зависимости от ширины окна
    int w = ui_tableWidget->width() / fields.count();

    for (int i=0; i<fields.count(); i++) {
        ui_tableWidget->horizontalHeader()->resizeSection(i, 130);
            //qDebug() << "size = " << w;
    }
}

void otrs::make_status_bar() {
    ///создаем строку состояния
    ui_bar = new QStatusBar();

    lbl = new QLabel;
    lbl->setText(tr("Connecting..."));
    ui_bar->addWidget(lbl);

    this->setStatusBar(ui_bar);
}

otrs::~otrs()
{

}

void otrs::on_connected(bool status) {
    if (status) {
        ///из модуля проверки тикетов получен сигнал об успешном подключении к сайту ОТРС
        lbl->setText(tr("Connected OK"));
    } else {
        //если подключение не произошло - вызываем мастера настроек
        lbl->setText(tr("Cannot access to OTRS"));
        if (show_wizard(tr("Check login/userpass"))) {
            lbl->setText(tr("New attempt to connect..."));
            otrsChecker->setBillconfig(billConfig);
            otrsChecker->setOtrsConfig(otrsConfig);
            worker->setConfig(otrsConfig);
            otrsChecker->run();
        }
    }

}

void otrs::on_newTicket(Ticket ticket) {
    ///из модуля проверки получен сигнал с новым тикетом

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

    ticket.isRemoved = false;
    ticket.updateTime.start();

    ticketList[ticket.id] = ticket;

}

void otrs::on_delTicket(Ticket ticket) {
    ///из модуля проверки получен сигнал об удалении тикета
    int ro = ui_tableWidget->rowCount();
    QTableWidgetItem *itm;
    for (int i = 0; i<ro; i++) {
        itm = ui_tableWidget->item(i, 0);
        if (itm->text().toInt() == ticket.id && itm->background() != QBrush(Qt::green))
        {
            on_logUpdate("Remove ticket: " + QString::number(ticket.id) + " " + ticket.host);
            ui_tableWidget->removeRow(i);
            ticketList[ticket.id].isRemoved = true;
            ro--;
            i--;
        }
    }
}

void otrs::updateTicket(Ticket ticket) {
    ///из модуля проверки получен сигнал с новыми данными по тикету
    QString text;
    QString m1 = tr("in base");
    QString m2 = tr("NOT in BASE");

    for (int i = 0; i<ui_tableWidget->rowCount(); i++) {
         text = ui_tableWidget->item(i, 0)->text();
         if (text.toInt() == ticket.id) {
             if (ticket.isMailinBase)
                 ui_tableWidget->item(i, 4)->setText(m1);
             else
                 ui_tableWidget->item(i, 4)->setText(m2);

            ui_tableWidget->item(i, 2)->setText(ticket.host);
            ui_tableWidget->item(i, 5)->setText(QString::number(ticket.money));
            ui_tableWidget->item(i, 6)->setText(QString::number(ticket.server));

         }
    }
    int id = ticket.id;
    ticketList[id].articleID = ticket.articleID;
    ticketList[id].money = ticket.money,
    ticketList[id].server = ticket.server;
    ticketList[id].host = ticket.host;
    ticketList[id].isMailinBase = ticket.isMailinBase;
}

void otrs::on_clipboard_changed() {
    ///следим за изменениями в буфере обмена.
    ///Если содержимое буфера совпадает с текстом некоторой ячейки, выделяется вся строка
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
    ///выходим из программы
    close();
}

void otrs::on_logUpdate(QString txt) {
    ///из модуля проверки получен сигнал с отладочным сообщением
    logView->insertHtml(txt + "<br>\n");
    logView->scrollToAnchor(txt);
}

void otrs::on_action_log(bool status) {
    ///включает-выключает отображение логов
    if (status) {
        logView->show();
        tickView->hide();
    } else {
        logView->hide();
    }
}

void otrs::on_mouse_click(int x, int y) {


    on_action_log(false);
    action_log->setChecked(false);

    tickView->setVisible(true);

    int id = ui_tableWidget->item(x, 0)->text().toInt();
    QString txt;
    txt = "<b>" + ticketList[id].subject + "</b><br>\n<br>\n";
    txt += ticketList[id].body;
    tickView->clear();
    tickView->insertHtml(txt);

}

void otrs::on_context_menu(QPoint point) {

    QTableWidgetItem *item = ui_tableWidget->itemAt(point);
    if (!item)
        return;

    contextId      = ui_tableWidget->item(item->row(), 0)->text().toInt();
    contextMenu->exec(ui_tableWidget->mapToGlobal(point));

}

void otrs::on_actionSpam() {
    worker->spamTicket(contextId);
    blockActions(false);
}

void otrs::on_actionAnswer() {
    answerForm = new answer(ticketList[contextId], answerHeader, answerFooter, tr("Answer"));
    if (answerForm->exec()) {
        QString subject = answerForm->getSubject();
        QString body    = answerForm->getBody();
        worker->answTicket(contextId, ticketList[contextId].articleID, body, subject, ticketList[contextId].mail);
        blockActions(false);
    }
    delete answerForm;

}

void otrs::on_actionClose() {
    Ticket ticket;
    ticket.body    = tr("Closed OK");
    ticket.subject = tr("Close");

    answerForm = new answer(ticket, "", "", tr("Close"));
    if (answerForm->exec()) {
        QString body    = answerForm->getBody();
        worker->closeTicket(contextId, body);
        blockActions(false);
    }
    delete answerForm;
}

void otrs::on_actionBlock() {
    blockRow(contextId);
    worker->blockTicket(contextId);
    blockActions(false);
}

void otrs::blockRow(int id, QBrush brush) {
    QString text;
    for (int i = 0; i<ui_tableWidget->rowCount(); i++) {
         text = ui_tableWidget->item(i, 0)->text();
         if (text.toInt() == id) {
             for (int j = 0; j<ui_tableWidget->columnCount(); j++)
                ui_tableWidget->item(i, j)->setBackground(brush);
         }
    }
}

void otrs::on_actionRemove() {
    Ticket ticket;
    ticket.id = contextId;

    int ro = ui_tableWidget->rowCount();
    QTableWidgetItem *itm;
    for (int i = 0; i<ro; i++) {
        itm = ui_tableWidget->item(i, 0);
        if (itm->text().toInt() == contextId)
        {
            on_logUpdate("Remove ticket: " + QString::number(contextId));
            ui_tableWidget->removeRow(i);
            ticketList[contextId].isRemoved = true;
            ro--;
            i--;
        }
    }

}

void otrs::blockActions(bool status) {
    actionAnswer->setEnabled(status);
    actionClose->setEnabled(status);
    actionSpam->setEnabled(status);
}

void otrs::on_action_wizard() {
    show_wizard();
}

bool otrs::show_wizard(QString text) {
    wizard = new Wizard(otrsConfig, billConfig, answerHeader, answerFooter, text);
    if (!wizard->exec()) return false;
    answerHeader = wizard->getAnswerHeader();
    answerFooter = wizard->getAnswerFooter();

    LoginConfig otrscfg, billcfg;
    otrscfg = wizard->getOtrsConfig();
    billcfg = wizard->getBillConfig();
    dontSavePasswords = wizard->toSavePasswords();
    qDebug() << dontSavePasswords;

    otrsConfig.username = otrscfg.username;
    otrsConfig.userpass = otrscfg.userpass;
    billConfig.username = billcfg.username;
    billConfig.userpass = billcfg.userpass;

    delete wizard;
    return true;
}

void otrs::on_action_save_settings() {
    this->save_settings("config.ini");
}

void otrs::save_settings(QString fileName) {
    QFile *file = new QFile(fileName);
    if (!file->open(QFile::ReadOnly)) {
        logView->insertHtml(tr("Cannot open file: ") + fileName + "<br>");
        return;
    }
    QByteArray line;
    QStringList lines;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    while (!file->atEnd()) {
        line = file->readLine();
        lines.append(codec->toUnicode(line));
    }
    file->close();

    if (!file->open(QFile::WriteOnly)) {
        logView->insertHtml(tr("Cannot re-write file: ") + fileName + "<br>");
        return;
    }

    lines = replace_line_in_config(lines, "otrs.username", otrsConfig.username);
    lines = replace_line_in_config(lines, "otrs.userpass", otrsConfig.userpass);
    lines = replace_line_in_config(lines, "bill.username", billConfig.username);
    lines = replace_line_in_config(lines, "bill.userpass", billConfig.userpass);
    lines = replace_line_in_config(lines, "answer.header", answerHeader);
    lines = replace_line_in_config(lines, "answer.footer", answerFooter);
    qDebug() << dontSavePasswords;

    if (dontSavePasswords) {
        lines = replace_line_in_config(lines, "otrs.userpass", "otrsConfig.userpass");
        lines = replace_line_in_config(lines, "bill.userpass", "billConfig.userpass");
    }

    QString content = "";
    for (int i =0; i<lines.count(); i++) content += lines.at(i);
    QTextStream out(file);
    out.setCodec(codec);
    out << content;
    file->close();

}

QStringList otrs::replace_line_in_config(QStringList lines, QString param, QString newValue) {
    QString line;
    QStringList newlines;
    bool exists = false;
    newValue.replace("\n", "0x%13");
    for (int i =0; i<lines.count(); i++) {
        line = lines.at(i);
        if (line.at(0) != '#') {
            if (line.trimmed().startsWith(param)) {
                line = param + " = " + newValue + "\n";
                exists = true;
            }
        }
        newlines << line;
    }
    if (!exists) {
        line = param + " = " + newValue + "\n";
        newlines << line;
    }
    return newlines;
}

void otrs::load_settings(QString fileName) {
    //читаем файл конфигурации config.ini
    QFile *file = new QFile(fileName);
    if (!file->open(QFile::ReadOnly)) {
        logView->insertHtml(tr("Cannot open config file: ") + fileName + "<br>\n");
        return;
    }

    QByteArray line;
    QString params;
    QString value;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");

    while (!file->atEnd()) {
        line = file->readLine();
        if (line.at(0) != '#') {
            params = line;
            params.remove(0, params.indexOf("=")+1);
            params = params.trimmed();

            value = line;
            value.remove(value.indexOf("="), value.length()-1);
            value = value.trimmed();

            if (value == "otrs.username")
                otrsConfig.username = params;

            if (value == "otrs.url")
                otrsConfig.uri = params;

            if (value == "otrs.url2")
                otrsConfig.uri2 = params;

            if (value == "otrs.post")
                otrsConfig.post = params;

            if (value == "otrs.zoom")
                otrsConfig.zoom = params;

            if (value == "otrs.hist")
                otrsConfig.hist = params;

            if (value == "bill.url")
                billConfig.uri = params;

            if (value == "bill.url2")
                billConfig.uri2 = params;

            if (value == "otrs.userpass")
                otrsConfig.userpass = params;

            if (value == "bill.username")
                billConfig.username = params;

            if (value == "bill.userpass")
                billConfig.userpass = params;

            if (value == "dbHost")
                mysqlconfig.dbHost = params;

            if (value == "dbName")
                mysqlconfig.dbName = params;

            if (value == "dbUser")
                mysqlconfig.dbUser = params;

            if (value == "dbPass")
                mysqlconfig.dbPass = params;

            if (value == "table")
                mysqlconfig.table = params;

            if (value == "answer.header") {
                params = codec->toUnicode(line);
                params.remove(0, params.indexOf("=")+1);
                params = params.trimmed();
                answerHeader = params;
                answerHeader.replace("0x%13", "\n");
            }

            if (value == "answer.footer") {
                params = codec->toUnicode(line);
                params.remove(0, params.indexOf("=")+1);
                params = params.trimmed();
                answerFooter = params;
                answerFooter.replace("0x%13", "\n");
            }

        }
    }
    file->close();

}

void otrs::on_actionCheck() {
    Ticket ticket;
    qDebug() << ticketList.count();
    int tm;
    foreach (ticket, ticketList) {
        tm =  ticket.updateTime.elapsed();
        if (!ticket.isRemoved && tm > 3600000) {
            blockRow(ticket.id, QBrush(Qt::red));
        }

    }
    QTimer::singleShot(5*60*1000, this, SLOT(on_actionCheck()));
}
