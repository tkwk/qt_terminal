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

    terminal = new DummyPty(this);
    QGraphicsView *view = new QGraphicsView(terminal);
    setCentralWidget(view);
    view->show();

    terminal->runProcess(command,args,QString("patate"));
}

MainWindow::~MainWindow()
{
    delete ui;
}
