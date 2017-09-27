#ifndef HTITLEBARWIDGET_H
#define HTITLEBARWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class HTitlebarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HTitlebarWidget(QWidget *parent = 0);

protected:
    void initUI();
    void initConnection();
    virtual bool event(QEvent *e);

public:
    QLabel* m_label;

    QPushButton* m_btnNum;

    QPushButton* m_btnMicphoneOpened;
    QPushButton* m_btnMicphoneClosed;

    QPushButton* m_btnMute;
    QPushButton* m_btnVoice;

    QPushButton* m_btnStartRecord;
    QPushButton* m_btnStopRecord;

    QPushButton* m_btnSnapshot;

    QPushButton* m_btnDrawInfo;

    QPushButton* m_btnFullScreen;
    QPushButton* m_btnExitFullScreen;
};

class HCombTitlebarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HCombTitlebarWidget(QWidget *parent = 0);

protected:
    void initUI();
    void initConnection();
    virtual bool event(QEvent *e);

public:
    QLabel* m_label;
    QPushButton* m_btnFullScreen;
    QPushButton* m_btnExitFullScreen;
    QPushButton* m_btnPinb;
    QPushButton* m_btnPinr;
    QPushButton* m_btnDrawInfo;
    QPushButton* m_btnSnapshot;
    QPushButton* m_btnStartRecord;
    QPushButton* m_btnStopRecord;
};

#endif // HTITLEBARWIDGET_H
