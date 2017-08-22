#ifndef HTITLEBARWIDGET_H
#define HTITLEBARWIDGET_H

#include <QWidget>

#define TITLEBAR_ICON_WIDTH          48
#define TITLEBAR_ICON_HEIGHT         48

class HTitlebarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HTitlebarWidget(QWidget *parent = 0);

signals:
    void fullScreen();
    void exitFullScreen();

    void numSelected(int num);
    void numUnselected(int num);

public slots:
    void onFullScreen();
    void onExitFullScreen();
    void onStartRecord();
    void onStopRecord();

    void onNumB1();
    void onNumR1();
    void onNumB2();
    void onNumR2();
    void onNumB3();
    void onNumR3();

public:
    void setTitle(const char* title) {m_label->setText(title);}

protected:
    void initUI();
    void initConnection();
    virtual bool event(QEvent *e);

public:
    QLabel* m_label;
    QPushButton* m_btnFullScreen;
    QPushButton* m_btnExitFullScreen;
    QPushButton* m_btnSnapshot;
    QPushButton* m_btnStartRecord;
    QPushButton* m_btnStopRecord;
    QPushButton* m_btnNumB[3];
    QPushButton* m_btnNumR[3];
};

#endif // HTITLEBARWIDGET_H
