#include "hmainwidget.h"
#include "hrcloader.h"

HMainWidget::HMainWidget(HDsContext* ctx, QWidget *parent) : QWidget(parent)
{
    m_ctx = ctx;

    initUI();
    initConnect();
}

HMainWidget::~HMainWidget(){

}

void HMainWidget::initUI(){
    qDebug("");
    if (m_ctx->width == 0 || m_ctx->height == 0){
       m_ctx->width  = QApplication::desktop()->width();
       m_ctx->height = QApplication::desktop()->height();
    }
    setGeometry(0,0,m_ctx->width, m_ctx->height);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(0,0,0));
    setPalette(pal);

    for (int i = 0; i < m_ctx->m_cntItem; ++i){
        // last is cock window,svrid = 1

        HGLWidget* wdg;
        if (i == m_ctx->m_cntItem - 1){
            wdg = new HCockGLWidget(this);
            wdg->svrid = 1;
            m_mapGLWdg[1] = wdg;
        }else{
            wdg = new HGLWidget(this);
            wdg->svrid = 0;
        }
        wdg->setGeometry(m_ctx->m_rcItems[i]);
        wdg->setTitleColor(m_ctx->titcolor);
        wdg->setOutlineColor(m_ctx->outlinecolor);
        m_vecGLWdg.push_back(wdg);
    }

    qDebug("screen_w=%d,screen_h=%d", width(), height());
    m_btnLeftExpand = new QPushButton(this);
    m_btnLeftExpand->setGeometry(width() - ICON_WIDTH, height() - ICON_HEIGHT, ICON_WIDTH, ICON_HEIGHT);
    m_btnLeftExpand->setIcon(QIcon(HRcLoader::instance()->icon_left_expand));
    m_btnLeftExpand->setIconSize(QSize(ICON_WIDTH, ICON_HEIGHT));
    m_btnLeftExpand->setFlat(true);
    m_btnLeftExpand->show();

    m_btnRightFold = new QPushButton(this);
    m_btnRightFold->setGeometry(width() - ICON_WIDTH, height() - ICON_HEIGHT, ICON_WIDTH, ICON_HEIGHT);
    m_btnRightFold->setIcon(QIcon(HRcLoader::instance()->icon_right_fold));
    m_btnRightFold->setIconSize(QSize(ICON_WIDTH, ICON_HEIGHT));
    m_btnRightFold->setFlat(true);
    m_btnRightFold->hide();


    m_webView = new HWebView(this);
    m_webView->setWindowFlags(Qt::ToolTip);
    m_webView->setWindowOpacity(0.5);
    //m_webView->setGeometry(QRect(0, height()-ICON_HEIGHT, 0, ICON_HEIGHT));
    m_webView->setGeometry(0, height()-ICON_HEIGHT, width()-ICON_WIDTH, ICON_HEIGHT);
    //m_webView->load(QUrl("http://127.0.0.1/transcoder/index.php?controller=channels&action=Panel"));
    m_webView->load(QUrl("http://192.168.1.112/transcoder/index.php?controller=channels&action=Panel"));
    //m_webView->load(QUrl("https://www.baidu.com/"));
    m_webView->hide();

    m_dragWdg = new HGLWidget(this);
    m_dragWdg->setOutlineColor(0x00FF00FF);
    m_dragWdg->hide();
}

void HMainWidget::initConnect(){
    qDebug("");
    setFocus(); // set key focus

    QObject::connect( m_ctx, SIGNAL(actionChanged(int)), this, SLOT(onActionChanged(int)) );
    QObject::connect( m_ctx, SIGNAL(titleChanged(int)), this, SLOT(onTitleChanged(int)) );
    QObject::connect( m_ctx, SIGNAL(videoPushed(int,bool)), this, SLOT(onvideoPushed(int,bool)) );
    QObject::connect( m_ctx, SIGNAL(audioPushed(int)), this, SLOT(onAudioPushed(int)) );
    QObject::connect( m_ctx, SIGNAL(sourceChanged(int,bool)), this, SLOT(onSourceChanged(int,bool)) );
    QObject::connect( m_ctx, SIGNAL(sigStop(int)), this, SLOT(onStop(int)) );
    QObject::connect( m_ctx, SIGNAL(quit()), this, SLOT(close()) );

    for (int i = 0; i < m_vecGLWdg.size(); ++i){
        QObject::connect( m_vecGLWdg[i], SIGNAL(fullScreen()), this, SLOT(onFullScreen()) );
        QObject::connect( m_vecGLWdg[i], SIGNAL(exitFullScreen()), this, SLOT(onExitFullScreen()) );
        QObject::connect( m_vecGLWdg[i], SIGNAL(clicked()), this, SLOT(onGLWdgClicked()) );
    }

    QObject::connect( m_btnLeftExpand, SIGNAL(clicked(bool)), this, SLOT(showToolbar()) );
    QObject::connect( m_btnRightFold, SIGNAL(clicked(bool)), this, SLOT(hideToolbar()) );

    timer_click.setSingleShot(true);
    //QObject::connect( &timer_click, SIGNAL(timeout()), this, SLOT(clearOpt()) );
}

HGLWidget* HMainWidget::getGLWdgBySvrid(int svrid){
    std::map<int, HGLWidget*>::iterator iter = m_mapGLWdg.find(svrid);
    if (iter != m_mapGLWdg.end()){
        HGLWidget* wdg = iter->second;
        if (wdg->svrid == svrid){
            return wdg;
        }
    }

    for (int i = 0; i < m_vecGLWdg.size(); ++i){
        if (m_vecGLWdg[i]->svrid == 0){
            m_vecGLWdg[i]->svrid = svrid;
            m_mapGLWdg[svrid] = m_vecGLWdg[i];
            return m_vecGLWdg[i];
        }
    }

    return NULL;
}

HGLWidget* HMainWidget::getGLWdgByPos(int x, int y){
    for (int i = 0; i < m_vecGLWdg.size(); ++i){
        QRect rc = m_vecGLWdg[i]->geometry();
        if (rc.contains(x,y)){
            return m_vecGLWdg[i];
        }
    }

    return NULL;
}

void HMainWidget::keyPressEvent(QKeyEvent *event){
    switch (event->key()){
        case Qt::Key_Backspace:
        {
            m_ctx->setAction(0);
            event->accept();
        }
            break;
        case Qt::Key_Escape:
        {
            close();
            event->accept();
        }
            break;
        defualt:
            break;
    }

    QWidget::keyPressEvent(event);
}

void HMainWidget::mousePressEvent(QMouseEvent *event){
    qDebug("%d,%d", event->x(), event->y());
}

void HMainWidget::mouseMoveEvent(QMouseEvent *event){
    qDebug("%d,%d", event->x(), event->y());

    HGLWidget* wdg = getGLWdgByPos(event->x(), event->y());
    if (wdg && wdg->svrid != 1 &&           // cock can not drag
        (wdg->status() & MAJOR_STATUS_MASK) == PLAYING){
        if (m_dragWdg->isVisible() == false){
            m_dragWdg->setVisible(true);
            m_dragWdg->svrid = wdg->svrid;
            m_dragWdg->setStatus(wdg->status());
            m_dragSrcWdg = wdg;
        }
    }

    if (m_dragWdg->isVisible()){
        m_dragWdg->setGeometry(event->x()-DRAG_WIDTH/2, event->y()-DRAG_HEIGHT, DRAG_WIDTH,DRAG_HEIGHT);
    }
}

void HMainWidget::mouseReleaseEvent(QMouseEvent *event){
    qDebug("%d,%d", event->x(), event->y());

    // drag release
    if (m_dragWdg->isVisible()){
        m_dragWdg->hide();

        HGLWidget* wdg = getGLWdgByPos(event->x(), event->y());
        if (wdg == NULL)
            return;

        if (m_dragSrcWdg != wdg){
            if (wdg->svrid == 1){
                // pick cock's source
                DsEvent evt;
                evt.type = DS_EVENT_PICK;
                evt.src_svrid = m_dragSrcWdg->svrid;
                evt.dst_svrid = 1;
                evt.dst_x = event->x() - wdg->x();
                evt.dst_y = event->y() - wdg->y();
                m_ctx->handle_event(evt);
            }else{
                // exchange position
                QRect rcSrc = m_dragSrcWdg->geometry();
                QRect rcDst = wdg->geometry();
                wdg->setGeometry(rcSrc);
                m_dragSrcWdg->setGeometry(rcDst);
            }
        }
    }else{ // normal clicked
        HGLWidget* wdg = getGLWdgByPos(event->x(), event->y());
        if (wdg == NULL)
            return;
    }
}

void HMainWidget::mouseDoubleClickEvent(QMouseEvent *event){
    qDebug("");
    HGLWidget* wdg = getGLWdgByPos(event->x(), event->y());
    if (wdg == NULL)
        return;

    if (wdg->svrid == 1){
        DsEvent evt;
        evt.type = DS_EVENT_STOP;
        evt.dst_svrid = 1;
        evt.dst_x = event->x() - wdg->x();
        evt.dst_y = event->y() - wdg->y();
        m_ctx->handle_event(evt);
    }
}

/*
void HMainWidget::onStart(){
    m_btnStart->hide();
    m_btnPause->show();
}

void HMainWidget::onPause(){
    m_btnStart->show();
    m_btnPause->hide();
    if(m_ctx->ifcb[0])
    {
        qDebug("");
        m_ctx->ifcb[0]->onservice_callback(ifservice_callback::e_service_cb_chr, libchar(), OOK_FOURCC('P', 'A', 'U', 'S'), 0, 0, NULL);
    }
    m_mapGLWdg[1]->setStatus(PAUSE);
}
*/

#include <QGraphicsEffect>
#include <QPropertyAnimation>
void HMainWidget::showToolbar(){
    qDebug("");

    m_btnLeftExpand->hide();
    m_btnRightFold->show();

//    QPropertyAnimation *animation = new QPropertyAnimation(m_webView, "geometry");
//    animation->setDuration(300);
//    animation->setStartValue(QRect(width()-ICON_WIDTH, height()-ICON_HEIGHT, 0, ICON_HEIGHT));
//    animation->setEndValue(QRect(0, height()-ICON_HEIGHT, width()-ICON_WIDTH, ICON_HEIGHT));
//    animation->start();

    m_webView->reload();
    m_webView->show();
}

void HMainWidget::hideToolbar(){
    qDebug("");

    m_btnLeftExpand->show();
    m_btnRightFold->hide();

//    QPropertyAnimation *animation = new QPropertyAnimation(m_webView, "geometry");
//    animation->setDuration(300);
//    animation->setStartValue(QRect(0, height()-ICON_HEIGHT, width()-ICON_WIDTH, ICON_HEIGHT));
//    animation->setEndValue(QRect(width()-ICON_WIDTH, height()-ICON_HEIGHT, 0, ICON_HEIGHT));
//    animation->start();

    m_webView->hide();

    setFocus();
}

void HMainWidget::onActionChanged(int action){
    if (action == 0){
        hide();
    }else if (action == 1){
        showFullScreen();

        // when hide,status change but not repaint
        for (int i = 0; i < m_vecGLWdg.size(); ++i){
            m_vecGLWdg[i]->repaint();
        }
    }
}

void HMainWidget::onTitleChanged(int svrid){
    qDebug("");

    HGLWidget* wdg = getGLWdgBySvrid(svrid);
    if (wdg == NULL)
        return;

    wdg->setTitle(m_ctx->m_title[svrid].c_str());
}

void HMainWidget::onvideoPushed(int svrid, bool bFirstFrame){
    HGLWidget* wdg = getGLWdgBySvrid(svrid);
    if (wdg == NULL)
        return;

    wdg->setStatus((wdg->status() & MINOR_STATUS_MASK) | PLAYING | PLAY_VIDEO);
}

void HMainWidget::onAudioPushed(int svrid){
    if (svrid != 1){
        HGLWidget* wdg = getGLWdgBySvrid(svrid);
        if (wdg == NULL)
            return;

        // audio not repaint
        wdg->setStatus((wdg->status() & MINOR_STATUS_MASK) | PLAYING | PLAY_AUDIO, false);
    }
}

void HMainWidget::onSourceChanged(int svrid, bool bSucceed){
    qDebug("");

    HGLWidget* wdg = getGLWdgBySvrid(svrid);
    if (wdg == NULL)
        return;

    wdg->removeIcon(CHANGING);
    if (!bSucceed){
        //
    }
}

void HMainWidget::onStop(int svrid){
    HGLWidget* wdg = getGLWdgBySvrid(svrid);
    if (wdg == NULL)
        return;

    qDebug("");
    wdg->setStatus(STOP);
    wdg->svrid = 0;
}

void HMainWidget::onFullScreen(){
    qDebug("");

    QWidget* pSender = (QWidget*)sender();

    m_rcSavedGeometry = pSender->geometry();

    pSender->setWindowFlags(Qt::Window);
    pSender->showFullScreen();
}

void HMainWidget::onExitFullScreen(){
    qDebug("");

    QWidget* pSender = (QWidget*)sender();

    pSender->setWindowFlags(Qt::SubWindow);
    pSender->setGeometry(m_rcSavedGeometry);
    pSender->showNormal();
}

void HMainWidget::onGLWdgClicked(){
    HGLWidget* pSender = (HGLWidget*)sender();

    for (int i = 0; i < m_vecGLWdg.size(); ++i){
        if (m_vecGLWdg[i] != pSender){
            m_vecGLWdg[i]->showTitlebar(false);
            m_vecGLWdg[i]->showToolbar(false);
        }
    }
}
