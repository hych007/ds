#ifndef HMAINWIDGET_H
#define HMAINWIDGET_H

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
#include "hmaintoolbar.h"
#include "hglwidget.h"
#include "hchangecolorwidget.h"
#include "ds_global.h"
#include "hdsctx.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#define MAXNUM_GLWIDGET 8

#define DRAG_WIDTH      192
#define DRAG_HEIGHT     108

class HDsContext;
class HMainWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit HMainWidget(HDsContext *ctx, QWidget *parent = nullptr);
    ~HMainWidget();

protected:
    void initUI();
    void initConnect();
    HGLWidget* getGLWdgByPos(int x, int y);
    HGLWidget* getGLWdgBySvrid(int svrid);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

signals:


public slots:
    void onTimerRepaint();
    void onActionChanged(int action);
    void onvideoPushed(int svrid, bool bFirstFrame);
    void onAudioPushed(int svrid);
    void onStop(int svrid);
    void onProgressNty(int svrid, int progress);

    void onFullScreen();
    void onExitFullScreen();
    void onGLWdgClicked();

    void expand();
    void fold();

    void changeScreenSource(int index, int svrid);

private:
    HDsContext* m_ctx;
    std::vector<HGLWidget*> m_vecGLWdg;
    std::map<int, HGLWidget*> m_mapGLWdg; // svrid : HGLWidget
    HGLWidget* m_focusGLWdg;

    QPushButton* m_btnLeftExpand;
    QPushButton* m_btnRightFold;
    HMainToolbar* m_toolbar;

    QLabel* m_labelDrag;
    HGLWidget* m_dragSrcWdg;

    QTimer timer_repaint;

    QRect m_rcSavedGeometry;
};

#endif // HMAINWIDGET_H
