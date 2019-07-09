#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "seqlogger.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private:
        Ui::MainWindow *ui;
        SeqLogger *_myLog;
    };

#endif // MAINWINDOW_H
