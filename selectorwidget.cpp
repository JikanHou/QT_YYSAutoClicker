#include "selectorwidget.h"

QPixmap grabScreenshot(){
    QPixmap desktopPixmap = QPixmap(QApplication::primaryScreen() -> geometry().size());
    QPainter p(&desktopPixmap);

    for (QScreen* screen : QApplication::screens())
        p.drawPixmap(screen -> geometry().topLeft(), screen->grabWindow(0));

    return desktopPixmap;
}

SelectorWidget::SelectorWidget(QWidget* parent) : QDialog(parent, Qt::FramelessWindowHint){
    setAttribute(Qt::WA_TranslucentBackground);
    setGeometry(QApplication::primaryScreen() -> geometry());

    desktopPixmap = grabScreenshot();
}

void SelectorWidget::mousePressEvent(QMouseEvent* event){
    selectedRect.setTopLeft(event -> globalPos());
}

void SelectorWidget::mouseMoveEvent(QMouseEvent* event){
    selectedRect.setBottomRight(event -> globalPos());
    update();
}

void SelectorWidget::mouseReleaseEvent(QMouseEvent* event)
{
    selectedPixmap = desktopPixmap.copy(selectedRect.normalized());
    accept();
}

void SelectorWidget::paintEvent(QPaintEvent*)
{
    QPainter p (this);
    p.drawPixmap(0, 0, desktopPixmap);

    QPainterPath path;
    path.addRect(rect());
    path.addRect(selectedRect);
    p.fillPath(path, QColor::fromRgb(255,255,255,200));

    p.setPen(Qt::red);
    p.drawRect(selectedRect);
}

