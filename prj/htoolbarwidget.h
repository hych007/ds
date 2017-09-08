#ifndef HTOOLBARWIDGET_H
#define HTOOLBARWIDGET_H

#include <QWidget>
#include <QSlider>

#define TOOLBAR_ICON_WIDTH          64
#define TOOLBAR_ICON_HEIGHT         64

class HToolbarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HToolbarWidget(QWidget *parent = 0);

signals:
    void progressChanged(int progress);

public slots:
    void onSlider();
    void onSlider(int action);

protected:
    void initUI();
    void initConnection();
    virtual bool event(QEvent *e);

public:
    QPushButton* m_btnStart;
    QPushButton* m_btnPause;
    QPushButton* m_btnStop;
    QSlider* m_slider;
};

class HCombToolbarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HCombToolbarWidget(QWidget *parent = 0);

protected:
    void initUI();
    void initConnection();
    virtual bool event(QEvent *e);

public:
    QPushButton* m_btnStart;
    QPushButton* m_btnPause;
    QPushButton* m_btnStop;

    QPushButton* m_btnText;
    QPushButton* m_btnTime;
    QPushButton* m_btnExpre;

    QPushButton* m_btnOK;
    QPushButton* m_btnTrash;
    QPushButton* m_btnUndo;
};

#endif // HTOOLBARWIDGET_H
