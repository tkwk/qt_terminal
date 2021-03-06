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

    setFocusPolicy( Qt::NoFocus);

    terminal = new DummyPty(this);
    QGraphicsView *view = new QGraphicsView(terminal);
    view->setSceneRect(0, 0, 2000, 1000);
    setCentralWidget(view);
    view->show();

    terminal->runProcess(command,args,QString("patate"));
}

MainWindow::~MainWindow()
{
    delete ui;
}
