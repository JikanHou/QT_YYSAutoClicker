#ifndef BGWORKER_H
#define BGWORKER_H

#include <QThread>
#include <QObject>
#include <QDateTime>
#include <QTime>
#include <QProcess>
#include <QRandomGenerator>
#include <QDir>
#include "opencv.hpp"

class BGWorker : public QObject{
    Q_OBJECT
public:
    explicit BGWorker(const QString adbPath, const cv:: Mat beginImg, const cv:: Mat endImg, int loopTime = 0);
public slots:
    void run();

private:
    int loopTime;
    double positiveThreshold = 0.8, negativeThreshold = 0.5;
    QString adbPath;
    cv:: Mat beginImg, endImg;
    bool stop = false;

    void screenshot(const QString &filename);
    bool clickImgUntilDisappear(const cv:: Mat &img, int timeOut = 30);
    void templateMatch(const cv:: Mat &src, const cv:: Mat &tmp, double* factor, cv:: Point* location);
    void callAdb(QString adbCmd);
    void press(int x, int y);

signals:
    void showMsg(const QString msg);
    void processDone(const QString msg);

public slots:
    void setStop();
};

#endif // BGWORKER_H
