#ifndef HGLWIDGET_H
#define HGLWIDGET_H

#include "qglwidgetimpl.h"
#include "ds_global.h"
#include "htitlebarwidget.h"
#include "htoolbarwidget.h"
#include "hnumselectwidget.h"
#include "hdsctx.h"

#define TITLE_BAR_HEIGHT    50
#define TOOL_BAR_HEIGHT     66

#define AUDIO_WIDTH         16
#define AUDIO_HEIGHT        160

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
    HGLWidget(QWidget* parent = Q_NULLPTR);
    virtual ~HGLWidget();

    int status(int mask = 0xFFFF) {return m_status & mask;}
    void setStatus(int status, bool bRepaint = true){
        m_status = status;
        if (bRepaint){
            repaint();
        }
    }

    void addIcon(int type, int x, int y, int w, int h);
    void removeIcon(int type);
    Texture* getTexture(int type);

    void setTitleColor(int color) {m_titcolor = color;}
    void setOutlineColor(int color) {m_outlinecolor = color;}

    virtual void showToolWidgets(bool bShow = true);

signals:
    void fullScreen();
    void exitFullScreen();
    void clicked();

public slots:
    void snapshot();
    void startRecord();
    void stopRecord();

    void onStart();
    void onPause();
    void onStop();

protected:
    virtual void drawVideo();
    virtual void drawAudio();
    virtual void drawIcon();
    virtual void drawTitle();
    virtual void drawOutline();
    virtual void paintGL();

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* e);

public:
    int svrid;

    bool m_bDrawTitle;
    bool m_bDrawAudio;
    bool m_bShowTools;

    QLabel* m_snapshot;

    int m_titcolor;
    int m_outlinecolor;

    int m_status;
    int m_nPreFrame; // previous frame cnt;

    std::map<int ,DrawInfo> m_mapIcons; // type : DrawInfo

    ulong m_tmMousePressed;
    QPoint m_ptMousePressed;
};

class HGeneralGLWidget : public HGLWidget
{
    Q_OBJECT
public:
    HGeneralGLWidget(QWidget* parent = Q_NULLPTR);
    virtual ~HGeneralGLWidget();

    void showTitlebar(bool bShow = true);
    void showToolbar(bool bShow = true);
    virtual void showToolWidgets(bool bShow = true);
    void setProgress(int progress) {m_toolbar->m_slider->setValue(progress);}

public slots:
    void onNumSelected(int num);
    void onNumCanceled(int num);
    void showNumSelector();

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

#include "hchangecolorwidget.h"
class HCockGLWidget : public HGLWidget
{
    Q_OBJECT
public:
    HCockGLWidget(QWidget* parent = Q_NULLPTR);
    ~HCockGLWidget();

    enum LOCATION{
        NotIn = 0,
        Left = 0x01,
        Right = 0x02,
        Top = 0x04,
        Bottom = 0x08,
        Center = 0x10,
    };
    int getLocation(QPoint pt, QRect rc);

    int getCockByPos(QPoint pt);
    void showTitlebar(bool bShow = true);
    void showToolbar(bool bShow = true);
    virtual void showToolWidgets(bool bShow = true);
    void adjustPos(QRect& rc);

signals:
    void cockChanged(DsCockInfo ci);
    void undo();

public slots:
    void onCockChanged();

protected:
    void initUI();
    void initConnect();

    virtual void drawOutline();
    virtual void drawTaskInfo();
    virtual void drawCockInfo();
    virtual void paintGL();
    virtual void resizeEvent(QResizeEvent *e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

    void reposCock(int index, QRect rc);
    void stopCock(int index);

private:
    std::vector<QRect> m_vecCocks;
    int m_cockoutlinecolor;

    int m_indexCock;
    int m_location;

    HCockTitlebarWidget* m_titlebar;
    HCockToolbarWidget*  m_toolbar;

    QLabel* m_labelDrag;
    HChangeColorWidget* m_wdgTrash;

    QLabel* m_labelResize;
    QPixmap m_pixmapCock;

    enum COCK_TYPE{
        PIP = 1,
        TILED = 2,
    };

    int m_cocktype;
};

#endif // HGLWIDGET_H
