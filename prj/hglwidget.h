#ifndef HGLWIDGET_H
#define HGLWIDGET_H

#include "qglwidgetimpl.h"
#include "hdsctx.h"
#include "htitlebarwidget.h"
#include "htoolbarwidget.h"
#include "hnumselectwidget.h"
#include "hnetwork.h"
#include <QElapsedTimer>

#define AUDIO_WIDTH         16
#define AUDIO_HEIGHT        200

#define TASKINFO_HEIGHT     32

enum GLWND_STATUS{
    MAJOR_STATUS_MASK = 0x00FF,
    STOP        =  0x0001,
    PAUSE       =  0x0002,
    PLAYING     =  0x0004,
    NOSIGNAL    =  0x0008,

    MINOR_STATUS_MASK = 0xFF00,
    PLAY_VIDEO  =  0x0100,
    PLAY_AUDIO  =  0x0200,
};

enum GLWDG_ICONS{
    NONE = 0,

    HAVE_AUDIO  = 1,

    PICK        = 10,
    PROHIBIT,
    CHANGING,
};

class HGLWidget : public QGLWidgetImpl
{
    Q_OBJECT
public:
    enum TYPE{
        UNKOWN = 0,
        GENERAL = 1,
        COMB    = 2,
        EXTEND  = 3,
        LMIC = 4,
    };

    HGLWidget(QWidget* parent = Q_NULLPTR);
    virtual ~HGLWidget();

    // overwrite setGeometry for setFixedSize to avoid titlebar expand
    void setGeometry(QRect rc){
        setFixedSize(rc.size());
        move(rc.topLeft());
    }

    void setGeometry(int x, int y, int w, int h){
        setGeometry(QRect(x,y,w,h));
    }

    int status(int mask = 0xFFFF) {return m_status & mask;}
    void setStatus(int status, bool bRepaint = true){
        m_status = status;
        if (bRepaint){
            repaint();
        }
    }
    inline bool isResetStatus(){
        return srvid == 0 && m_status == STOP;
    }
    inline void resetStatus(){
        srvid = 0;
        m_status = STOP;
        fps = 0;
        m_mapIcons.clear();
        repaint();
    }

    void addIcon(int type, int x, int y, int w, int h);
    void removeIcon(int type);
    Texture* getTexture(int type);

    virtual bool showToolWidgets(bool bShow = true);
    virtual void updateToolWidgets() {}

    QRect videoArea(){
#if LAYOUT_TYPE_MULTI_INPUT
        return QRect(0,0,width()-(AUDIO_WIDTH*2+6),height()-TASKINFO_HEIGHT);
#else
        return QRect(0,0,width(),height());
#endif
    }

signals:
    void fullScreen(bool);
    void clicked();

public slots:
    void onFullScreen() {
        m_bFullScreen = true;
        emit fullScreen(true);
    }

    void onExitFullScreen(){
        m_bFullScreen = false;
        emit fullScreen(false);
    }

    void enableDrawInfo() {m_bDrawInfo = true;}
    void disableDrawInfo() {m_bDrawInfo = false;}

    void snapshot();
    void startRecord();
    void stopRecord();

    void onStart();
    void onPause();
    void onStop();

    void showPtz();

protected:
    virtual void drawVideo();
    virtual void drawAudio();
    virtual void drawAudioStyle1(QRect rc, int num);
    virtual void drawAudioStyle2(QRect rc, int num);
    virtual void drawIcon();
    virtual void drawTitle();
    virtual void drawTaskInfo();
    virtual void drawOutline();
    virtual void drawDebugInfo();
    virtual void paintGL();

    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void resizeEvent(QResizeEvent* e);
    virtual void showEvent(QShowEvent* e);
    virtual void hideEvent(QHideEvent* e);
    virtual void enterEvent(QEvent* e);
    virtual void leaveEvent(QEvent* e);

    void calFps();
    void drawFps();

public:
    TYPE type;
    int wndid;
    int srvid;
    int m_status;
    std::map<int ,DrawInfo> m_mapIcons; // type : DrawInfo

    QElapsedTimer timer_elapsed;
    int framecnt;
    int fps;

public:
    QLabel* m_snapshot;
    bool m_bDrawInfo;
    bool m_bFullScreen;

    ulong m_tmMousePressed;
    QPoint m_ptMousePressed;
};

class HGeneralGLWidget : public HGLWidget
{
    Q_OBJECT
public:
    HGeneralGLWidget(QWidget* parent = Q_NULLPTR);
    virtual ~HGeneralGLWidget();

    virtual bool showToolWidgets(bool bShow = true);
    virtual void updateToolWidgets();
    void setProgress(int progress) {m_toolbar->setProgress(progress);}

public slots:
    void onNumSelected(int num);
    void onNumCanceled(int num);
    void showNumSelector();
    void openMicphone();
    void closeMicphone();
    void onVoice();
    void onMute();

    void onProgressChanged(int progress);

protected:
    void drawSelectNum();
    void drawSound();
    virtual void drawOutline();
    virtual void paintGL();
    void initUI();
    void initConnect();

public:
    HTitlebarWidget* m_titlebar;
    HToolbarWidget*  m_toolbar;
    HNumSelectWidget* m_numSelector;
};


//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#include "hchangecolorwidget.h"
#include "hexprewidget.h"
#include "haddtextwidget.h"
#include "heffectwidget.h"
#include "hoperatetarget.h"

class HCombGLWidget : public HGLWidget
{
    Q_OBJECT
public:
    enum LOCATION{
        NotIn = 0,
        Left = 0x01,
        Right = 0x02,
        Top = 0x04,
        Bottom = 0x08,
        Center = 0x10,
    };
    int getLocation(QPoint pt, QRect rc);


    HCombGLWidget(QWidget* parent = Q_NULLPTR);
    ~HCombGLWidget();

    HOperateObject getObejctByPos(QPoint pt, HAbstractItem::TYPE type = HAbstractItem::ALL);

    virtual bool showToolWidgets(bool bShow = true);
    virtual void updateToolWidgets();
#if LAYOUT_TYPE_ONLY_OUTPUT
#define LIMC_WNDID_BASE 100
    HGLWidget* allocGLWdgForLimc(){
        int cell_w = width()/3;
        int cell_h = height()/2;
        QRect rc[6];
        int x = 0;
        int y = 0;
        for (int r = 0; r < 2; ++r){
            x = 0;
            for (int c = 0; c < 3; ++c){
                rc[r*3+c].setRect(x, y, cell_w, cell_h);
                x += cell_w;
            }
            y += cell_h;
        }

        int sort[6] = {5, 2, 3, 0, 4, 1};
        for (int i = 0; i < m_vecGLWdgForLimc.size(); ++i){
            if (m_vecGLWdgForLimc[i]->isResetStatus()){
                m_vecGLWdgForLimc[i]->setGeometry(rc[sort[i]]);
                m_vecGLWdgForLimc[i]->wndid = LIMC_WNDID_BASE + sort[i];
                return m_vecGLWdgForLimc[i];
            }
        }

        return NULL;
    }
#endif

public slots:
    void onCombChanged();
    void onOverlayChanged();

    void lockTools() {m_bLockToolbar = true;}
    void unlockTools() {m_bLockToolbar = false;}

    void onUndo();
    void onTrash();
    void onOK();
    void onCancel();
    void onZoomIn();
    void onZoomOut();
    void showExpre();
    void onExpreSelected(QString& filepath);
    void showText();
    void onTextAccepted(HTextItem item);
    void onSetting();
    void showEffect();
    void onEffectSelected(HPictureItem item);

protected:
    void initUI();
    void initConnect();

    virtual void drawOutline();
    virtual void drawTaskInfo();
    void drawScreenInfo();
    void drawPictureInfo();
    void drawTextInfo();
    virtual void paintGL();
    virtual void resizeEvent(QResizeEvent *e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

    QRect adjustPos(QRect rc);
    QRect scaleToOrigin(QRect rc);
    QRect scaleToDraw(QRect rc);

    void onTargetChanged();
    bool updateOperateTarget();
    bool showOperateTarget();

private:
    std::vector<HOperateObject> m_vecScreens;
    std::vector<HOperateObject> m_vecPictures;
    std::vector<HOperateObject> m_vecTexts;
    HOperateTarget m_target;

private:
#if LAYOUT_TYPE_ONLY_OUTPUT
    std::vector<HGLWidget*> m_vecGLWdgForLimc;
#endif
    QLabel* m_label;
    HOperateWidget* m_targetWdg;
    HCombTitlebarWidget* m_titlebar;
    HCombToolbarWidget*  m_toolbar;
    HChangeColorWidget* m_wdgTrash;
    HExpreWidget* m_wdgExpre;
    HAddTextWidget* m_wdgText;
    HEffectWidget* m_wdgEffect;

    int m_location;
    bool m_bMouseMoving;
    bool m_bLockToolbar;
};
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#endif // HGLWIDGET_H
