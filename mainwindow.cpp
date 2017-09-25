#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);
    char * command = "bash";
    char * args[] = {command,0};
    ui->terminal->runProcess(command,args,QString("patate"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress && obj != this)
    {
        //if(obj == ui->gridLayout)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            {
                ui->terminal->processInput(keyEvent->text().toStdString().c_str());
            }
        }
    }
    return QObject::eventFilter(obj, event);
}
