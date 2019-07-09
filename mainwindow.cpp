#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
    {
    ui->setupUi(this);
    _myLog = SeqLogger::Instance();
    _myLog->SetMinimumLogLevel(SeqLogger::LogLevel::DebugLevel);
    _myLog->Debug("Got some data here: {data}", {"this is data"});
    }

MainWindow::~MainWindow()
    {
    delete ui;
    }
