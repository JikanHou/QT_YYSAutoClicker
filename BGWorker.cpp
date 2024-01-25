#include "BGWorker.h"

BGWorker::BGWorker(const QString adbPath, const cv:: Mat beginImg, const cv:: Mat endImg, int loopTime){
    this -> beginImg = beginImg;
    this -> endImg = endImg;
    this -> loopTime = loopTime;
    this -> adbPath = adbPath;
}

int randomNumber(int low, int high){
    return QRandomGenerator:: global() -> bounded(low, high);
}

void BGWorker:: templateMatch(const cv:: Mat &src, const cv:: Mat &tmp, double* factor, cv:: Point* location){
    cv:: Mat res(src.rows - tmp.rows + 1, src.cols - tmp.cols + 1, CV_32FC1);
    cv:: matchTemplate(src, tmp, res, cv:: TM_CCOEFF_NORMED);
    cv:: minMaxLoc(res, nullptr, factor, nullptr, location);
}

void BGWorker:: callAdb(QString adbCmd){
    QString cmd = adbPath + " " + adbCmd;
    QProcess process;
    process.startCommand(QString("cmd /c \"%1\"").arg(cmd));
    process.waitForFinished();
    qDebug() << process.readAllStandardOutput();
    qDebug() << process.readAllStandardError();
    qDebug() << cmd << "was called";
}

void BGWorker:: press(int x, int y){
    callAdb(QString("shell input tap %1 %2").arg(x).arg(y));
}

void BGWorker:: screenshot(const QString& filename){
    callAdb(QString("exec-out screencap -p > \"%1/%2\"").arg(QDir:: currentPath()).arg(filename));
}

bool BGWorker:: clickImgUntilDisappear(const cv:: Mat &img, int timeOut){
    QDateTime startTime = QDateTime:: currentDateTime();
    double factor;
    cv:: Mat src;
    cv:: Point location;

    while (!stop && startTime.secsTo(QDateTime:: currentDateTime()) <= timeOut){
        QThread:: msleep(50);
        // take current screenshot
        screenshot("gameScreenshot.png");
        cv:: cvtColor(cv:: imread("gameScreenshot.png"), src, cv:: COLOR_RGB2GRAY);

        //match
        templateMatch(src, img, &factor, &location);

        // image does not exist
        if (factor < positiveThreshold){
            continue;
        }

        //wait until there is any response
        int deltax, deltay, x, y;

        do{
            QThread:: msleep(50);
            deltax = randomNumber(0, img.cols);
            deltay = randomNumber(0, img.rows);
            x = location.x + deltax;
            y = location.y + deltay;
            press(x, y);

            // check for resp
            QThread:: msleep(randomNumber(200, 300));
            screenshot("gameScreenshot.png");
            cv:: cvtColor(cv:: imread("gameScreenshot.png"), src, cv:: COLOR_RGB2GRAY);
            templateMatch(src, img, &factor, &location);

            //resp shown
            if (factor < negativeThreshold){
                return true;
            }
        }while(!stop && startTime.secsTo(QDateTime:: currentDateTime()) <= timeOut);

    }
    return false;
}

void BGWorker:: run(){
//    for (int i = 1; i <= 10; i ++){
//        emit showMsg(QString:: number(i));
//        QThread:: msleep(500);
//    }
//    emit processDone("Done");
//    return;
    QDateTime taskStartTime = QDateTime:: currentDateTime();
    emit showMsg("Task starts.");
    if (beginImg.empty() || endImg.empty()){
        emit showMsg("Missing Images!");
        emit processDone("Task Aborted.");
        return;
    }
    for (int i = 1; i <= loopTime && !stop; i ++){
        emit showMsg(QString("\nBattle %1 / %2 starts.\nLooking for Start Image...").arg(i).arg(loopTime));
        if (clickImgUntilDisappear(beginImg, 10 * 60)){
            emit showMsg("Battle Begins.");
            QDateTime battleStartTime = QDateTime:: currentDateTime();
            emit showMsg("Waiting for End Image.");
            if (clickImgUntilDisappear(endImg, 10 * 60)){
                emit showMsg("Battle Ends.");
                QDateTime battleEndTime = QDateTime:: currentDateTime();
                emit showMsg("Time Cost: " + QString:: number(battleStartTime.secsTo(battleEndTime)) + " seconds.");
            }
            else if (!stop){
                emit processDone("Fail to End.");
                emit processDone("Task Aborted");
                return;
            }
        }
        else if (!stop){ // fail to begin
            emit showMsg("Fail to Begin.");
            emit processDone("Task Aborted");
            return;
        }
    }
    if (!stop){
        emit processDone("Task finished.");
        QDateTime taskEndTime = QDateTime:: currentDateTime();
        emit showMsg("Task Time Cost: " + QString:: number(taskStartTime.secsTo(taskEndTime)) + " seconds.");
    }
}

void BGWorker:: setStop(){
    stop = true;
}
