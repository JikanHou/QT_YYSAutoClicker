#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QImage>
#include <QFile>
#include <QPixmap>
#include <QFileDialog>
#include <QThread>
#include <QMessageBox>
#include <QProgressDialog>
#include <QRandomGenerator>
#include <QProcess>
#include <QTextDocument>
#include "ui_mainwindow.h"
#include "selectorwidget.h"
#include "BGWorker.h"
#include "opencv.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString adbPath;
    QString adbPort;
    BGWorker *currentWorker;
    QThread *currentThread;

    void initialization();
    void closeEvent (QCloseEvent *event) override;
    void callAdb(QString adbCmd);
    void tap(int x, int y);

private slots:
    void loadAdbButtonClicked();
    void screenShotButtonClicked();
    void startButtonClicked();
    void on_loadScreenshotBtn_clicked();
    void showMsg(const QString msg);
    void processDone(const QString msg);
    void on_stopBtn_clicked();
    void on_endScreenshotBtn_clicked();
    void on_endScreenshotLoad_clicked();
};
#endif // MAINWINDOW_H
