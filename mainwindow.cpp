#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow){
    ui -> setupUi(this);
    initialization();
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow:: initialization(){
    // GUI
    ui -> mainLayout -> setGeometry(QRect(0, 0, this -> geometry().width(), this -> geometry().height()));

    // Load Settings
    currentThread = nullptr;
    currentWorker = nullptr;
    QSettings settings("HouJikan", "YYSAutoClicker");
    QString adbPath = settings.value("adb_path", "").toString();
    ui -> ADBPathEdit -> setText(adbPath);

    int repeatTime = settings.value("repeat_time", 0).toInt();
    ui -> RepeatTime -> setValue(repeatTime);

    QString adbPort = settings.value("adb_port", "16384").toString();
    ui -> ADBPortEdit -> setText(adbPort);

    if (QFile:: exists("beginImg.png")){
        QImage image("beginImg.png");
        ui -> ScreenShot -> setScaledContents(image.width() > 256 || image.height() > 256);
        ui -> ScreenShot -> setPixmap(QPixmap::fromImage(image));
    }

    if (QFile:: exists("endImg.png")){
        QImage image("endImg.png");
        ui -> endImageLabel -> setScaledContents(image.width() > 256 || image.height() > 256);
        ui -> endImageLabel -> setPixmap(QPixmap::fromImage(image));
    }

    // Connect
    connect(ui -> SelectPathBtn, &QPushButton:: clicked, this, &MainWindow:: loadAdbButtonClicked);
    connect(ui -> ScreenShotBtn, &QPushButton:: clicked, this, &MainWindow:: screenShotButtonClicked);
    connect(ui -> StartBtn, &QPushButton:: clicked, this, &MainWindow:: startButtonClicked);    
}

void MainWindow:: loadAdbButtonClicked(){
    QString defaultPath;
    if (ui -> ADBPathEdit -> text().isEmpty()){
        defaultPath = QDir::currentPath();
    }
    else{
        defaultPath = ui -> ADBPathEdit -> text();
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open ADB"), defaultPath, tr("ADB Files (adb.exe)"));
    if (fileName.isEmpty()){
        return;
    }
    ui -> ADBPathEdit -> setText(fileName);
}

void MainWindow:: screenShotButtonClicked(){
    this -> hide();
    QThread:: msleep(200);
    SelectorWidget w;
    if (w.exec() == QDialog::Accepted){
        w.selectedPixmap.save("beginImg.png");
        //scale if too large
        ui -> ScreenShot -> setScaledContents(w.width() > 256 || w.height() > 256);
        ui -> ScreenShot -> setPixmap(w.selectedPixmap);
    }
    this -> show();
}

void MainWindow:: closeEvent (QCloseEvent *event){
    //Store Settings
    QSettings settings("HouJikan", "YYSAutoClicker");
    settings.setValue("adb_path", ui -> ADBPathEdit -> text());
    settings.setValue("repeat_time", ui -> RepeatTime -> value());
    settings.setValue("adb_port", ui -> ADBPortEdit -> text());
}

void MainWindow:: tap(int x, int y){
    callAdb(QString("shell input tap %1 %2").arg(x).arg(y));
}

void MainWindow:: callAdb(QString adbCmd){
    QString cmd = adbPath + " " + adbCmd;
//    system(cmd.toStdString().c_str());
//    QProcess:: execute("cmd.exe", cmd.toStdString().c_str());
    QProcess process;
//    process.start(adbPath, adbCmd.split(" "));
    process.startCommand(QString("cmd /c \"%1\"").arg(cmd));
    process.waitForFinished();
    qDebug() << process.readAllStandardOutput();
    qDebug() << process.readAllStandardError();
    qDebug() << cmd << "was called";
}


void MainWindow:: startButtonClicked(){
    if (ui -> ADBPathEdit -> text().isEmpty()){
        QMessageBox:: critical(this, "Error", "ADB path is empty.");
        return;
    }
    if (!QFile::exists(ui -> ADBPathEdit -> text())){
        QMessageBox:: critical(this, "Error", "ADB file does not exist.");
        return;
    }
    if (ui -> ADBPortEdit -> text().isEmpty()){
        QMessageBox:: critical(this, "Error", "ADB port is not specified.");
        return;
    }
    if (ui -> ScreenShot -> pixmap().isNull()){
        QMessageBox:: critical(this, "Error", "Screenshot is not specified.");
        return;
    }
    if (ui -> RepeatTime -> text() == "0" || ui -> RepeatTime -> text().isEmpty()){
        QMessageBox:: critical(this, "Error", "Repeat time cannot be zero or empty.");
        return;
    }
    // clean old info
    ui -> Info -> clear();

    // Start executing
    adbPath = ui -> ADBPathEdit -> text();
    adbPort = ui -> ADBPortEdit -> text();

    callAdb(QString("connect 127.0.0.1:%1").arg(adbPort));

    int maxTime = ui -> RepeatTime -> text().toInt();
    cv:: Mat beginImg, endImg;
    cv:: cvtColor(cv::imread("beginImg.png"), beginImg, cv:: COLOR_RGB2GRAY);
    cv:: cvtColor(cv::imread("endImg.png"), endImg, cv:: COLOR_RGB2GRAY);

    currentThread = new QThread;
    currentWorker = new BGWorker(adbPath, beginImg, endImg, maxTime);
    currentWorker -> moveToThread(currentThread);

    // connect(currentThread, &QThread::finished, currentWorker, &QObject:: deleteLater);
    connect(currentWorker, &BGWorker::showMsg, this, &MainWindow:: showMsg);
    connect(currentWorker, &BGWorker::processDone, this, &MainWindow:: processDone);
    connect(currentThread, &QThread::started, currentWorker, &BGWorker::run);
    currentThread -> start();


//    QProgressDialog progress("执行中……", "停止", 0, maxTime, this);
//    progress.setMinimumDuration(1000);
//    progress.setWindowModality(Qt::WindowModal);
//    progress.show();

//    int pressX, pressY, deltax, deltay;
//    deltax = ui -> ScreenShot -> pixmap().width() / 4;
//    deltay = ui -> ScreenShot -> pixmap().height() / 4;
//    // Convert screenshot to B & W

//    cv:: Mat templateImg, finishImg;
//    cv:: cvtColor(cv::imread("screenshot.png"), templateImg, cv:: COLOR_RGB2GRAY);
//    cv:: cvtColor(cv::imread("finish.png"), finishImg, cv:: COLOR_RGB2GRAY);
//    // Start Looping
//    for (int loopTime = 1; loopTime <= maxTime; loopTime ++){
//        progress.setValue(loopTime);
//        if (progress.wasCanceled()){
//            break;
//        }
//        while (true){
//            callAdb(QString("exec-out screencap -p > \"%1/gameScreenshot.png\"").arg(QDir:: currentPath()));
//            //
//            cv:: Mat srcImg;
//            cv:: cvtColor(cv::imread("gameScreenshot.png"), srcImg, cv:: COLOR_RGB2GRAY);
//            cv:: Mat result(srcImg.rows - templateImg.rows + 1, srcImg.cols - templateImg.cols + 1, CV_32FC1);

//            cv:: matchTemplate(srcImg, templateImg, result, cv:: TM_CCOEFF_NORMED);
//            cv:: Point maxLoc, minLoc;
//            double maxVal, minVal;
//            cv:: minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
//            if (maxVal > 0.8){
//                pressX = QRandomGenerator::global() -> bounded(maxLoc.x, maxLoc.x + templateImg.cols);
//                pressY = QRandomGenerator::global() -> bounded(maxLoc.y, maxLoc.y + templateImg.rows);
//                qDebug() << "success!" << maxVal;
//                break;
//            }
//            else{
//                qDebug() << "failed!" << maxVal;
//                // tap(pressX, pressY);
//            }
////            qDebug() << minVal << maxVal;
////            cv:: Point pt1(maxLoc.x, maxLoc.y);
////            cv:: Point pt2(maxLoc.x + templateImg.cols, maxLoc.y);
////            cv:: Point pt3(maxLoc.x, maxLoc.y + templateImg.rows);
////            cv:: Point pt4(maxLoc.x + templateImg.cols, maxLoc.y + templateImg.rows);

////            line(srcImg, pt1, pt2, cv::Scalar(255, 255, 255), 11, 1);
////            line(srcImg, pt2, pt4, cv::Scalar(255, 255, 255), 11, 1);
////            line(srcImg, pt4, pt3, cv::Scalar(255, 255, 255), 11, 1);
////            line(srcImg, pt3, pt1, cv::Scalar(255, 255, 255), 11, 1);
////            cv:: imwrite("result.png", srcImg);
////            cv:: imwrite("template.png", templateImg);
////          cv:: normalize( result, result, 0, 1, cv:: NORM_MINMAX, -1, cv:: Mat() );
////            break;
//            QThread:: msleep(100);
//        }
//        tap(pressX, pressY);
//        while (true){
//            callAdb(QString("exec-out screencap -p > %1/gameScreenshot.png").arg(QDir:: currentPath()));
//            cv:: Mat srcImg;
//            cv:: cvtColor(cv::imread("gameScreenshot.png"), srcImg, cv:: COLOR_RGB2GRAY);
//            cv:: Mat result(srcImg.rows - templateImg.rows + 1, srcImg.cols - templateImg.cols + 1, CV_32FC1);

//            cv:: matchTemplate(srcImg, finishImg, result, cv:: TM_CCOEFF_NORMED);
//            cv:: Point maxLoc, minLoc;
//            double maxVal, minVal;
//            cv:: minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
//            if (maxVal > 0.8){
//                pressX = QRandomGenerator::global() -> bounded(maxLoc.x, maxLoc.x + finishImg.cols);
//                pressY = QRandomGenerator::global() -> bounded(maxLoc.y, maxLoc.y + finishImg.rows);
//                qDebug() << "battle finished" << maxVal;
//                qDebug() << minVal << maxVal;
////                cv:: Point pt1(maxLoc.x, maxLoc.y);
////                cv:: Point pt2(maxLoc.x + templateImg.cols, maxLoc.y);
////                cv:: Point pt3(maxLoc.x, maxLoc.y + templateImg.rows);
////                cv:: Point pt4(maxLoc.x + templateImg.cols, maxLoc.y + templateImg.rows);

////                line(srcImg, pt1, pt2, cv::Scalar(255, 255, 255), 11, 1);
////                line(srcImg, pt2, pt4, cv::Scalar(255, 255, 255), 11, 1);
////                line(srcImg, pt4, pt3, cv::Scalar(255, 255, 255), 11, 1);
////                line(srcImg, pt3, pt1, cv::Scalar(255, 255, 255), 11, 1);
////                cv:: imwrite("result_finish.png", srcImg);
//                break;
//            }
//            else{
//                qDebug() << "battle continuing" << maxVal;
//            }
//            QThread:: msleep(100);
//        }
//        QThread:: msleep(QRandomGenerator::global() -> bounded(5000, 8000));
//        tap(pressX, pressY);

//    }
//    progress.setValue(maxTime);
}

void MainWindow::on_loadScreenshotBtn_clicked(){
    QString defaultPath = QDir::currentPath();
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Screenshot"), defaultPath, tr("images (*.png)"));
    if (fileName.isEmpty()){
        return;
    }
    QPixmap pixmap(fileName);
    ui -> ScreenShot -> setPixmap(pixmap);
    pixmap.save("beginImg.png");
}

void MainWindow:: showMsg(const QString msg){
    QDateTime time = QDateTime:: currentDateTime();
    //ui -> Title -> setText(msg);
    ui -> Info -> append(time.toString("【yyyy.MM.dd hh:mm:ss】 ") + msg);
    qDebug() << "signal received: " + msg;
}

void MainWindow:: processDone(const QString msg){
    QDateTime time = QDateTime:: currentDateTime();
    ui -> Info -> append(time.toString("【yyyy.MM.dd hh:mm:ss】 ") + msg);
    if (currentThread != nullptr){
        currentThread -> quit();
        currentThread -> wait();
        delete currentThread;
        currentThread = nullptr;
    }
    if (currentWorker != nullptr){
        delete currentWorker;
        currentWorker = nullptr;
    }
}

void MainWindow::on_stopBtn_clicked(){
    if (currentThread != nullptr){
        if (currentWorker != nullptr){
            currentWorker -> setStop();
            QThread:: msleep(200);
        }
        currentThread -> quit();
        currentThread -> wait();
        delete currentThread;
        currentThread = nullptr;
        showMsg("Task Terminated.");
    }
    if (currentWorker != nullptr){
        delete currentWorker;
        currentWorker = nullptr;
    }

}



void MainWindow::on_endScreenshotBtn_clicked(){
    this -> hide();
    QThread:: msleep(200);
    SelectorWidget w;
    if (w.exec() == QDialog::Accepted){
        w.selectedPixmap.save("endImg.png");
        //scale if too large
        ui -> endImageLabel -> setScaledContents(w.width() > 256 || w.height() > 256);
        ui -> endImageLabel -> setPixmap(w.selectedPixmap);
    }
    this -> show();
}


void MainWindow::on_endScreenshotLoad_clicked(){
    QString defaultPath = QDir::currentPath();
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Screenshot"), defaultPath, tr("images (*.png)"));
    if (fileName.isEmpty()){
        return;
    }
    QPixmap pixmap(fileName);
    ui -> endImageLabel -> setPixmap(pixmap);
    pixmap.save("endImg.png");
}

