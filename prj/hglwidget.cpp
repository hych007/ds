#include "ds_global.h"
#include "hglwidget.h"
#include "hdsctx.h"
#include "hrcloader.h"
#include "ds.h"
#include <QTimer>
#include "hmainwidget.h"

HGLWidget::HGLWidget(QWidget *parent)
    : QGLWidgetImpl(parent)
{
    m_status = STOP;
    m_tmMousePressed = 0;

    m_bDrawInfo = true;

    m_snapshot = new QLabel(this);
    m_snapshot->setStyleSheet("border:5px double #ADFF2F");
    m_snapshot->hide();
}

HGLWidget::~HGLWidget(){

}

bool HGLWidget::showToolWidgets(bool bShow){
    return bShow;
}

void HGLWidget::onStart(){
    DsSvrItem* item = g_dsCtx->getItem(srvid);
    if (item && item->ifcb){
        qDebug("srvid=%d startplay", srvid);
        //item->bPause = false;
        if (srvid == 1){
            item->ifcb->onservice_callback(ifservice_callback::e_service_cb_chr, libchar(), OOK_FOURCC('P', 'A', 'U', 'S'), 0, 0, NULL);
        }else{
            item->ifcb->onservice_callback(ifservice_callback::e_service_cb_pause, libchar(), OOK_FOURCC('P', 'A', 'U', 'S'), 0, 0, NULL);
        }
    }

}

void HGLWidget::onPause(){    
    DsSvrItem* item = g_dsCtx->getItem(srvid);
    if (item && item->ifcb){
        qDebug("srvid=%d ifservice_callback::e_service_cb_pause", srvid);
        //item->bPause = true;
        if (srvid == 1){
            item->ifcb->onservice_callback(ifservice_callback::e_service_cb_chr, libchar(), OOK_FOURCC('P', 'A', 'U', 'S'), 0, 0, NULL);
        }else{
            item->ifcb->onservice_callback(ifservice_callback::e_service_cb_pause, libchar(), OOK_FOURCC('P', 'A', 'U', 'S'), 0, 1, NULL);
        }
    }
    setStatus(PAUSE | status(MINOR_STATUS_MASK));
}

void HGLWidget::onStop(){
    setStatus(STOP);
    m_mapIcons.clear();
    m_nPreFrame = 0;

    if (srvid != 1){// srvid=1 is comb,reserve
        srvid = 0;
    }
}

#include "hffmpeg.h"
#include <QDir>
#include <QDateTime>
void HGLWidget::snapshot(){
    //test snapshot
    DsSvrItem* item = g_dsCtx->getItem(srvid);
    if (item && item->tex_yuv.data){
        static const char* prefix = "/var/transcoder/snapshot/";
        QDir dir;
        if (!dir.exists(prefix)){
            dir.mkpath(prefix);
        }

        Texture* tex_yuv = &item->tex_yuv;
        uchar* rgb = (uchar*)malloc(tex_yuv->width * tex_yuv->height * 4);
        yuv2rgb32(tex_yuv->data, tex_yuv->width, tex_yuv->height, rgb);
        QImage img(rgb, tex_yuv->width, tex_yuv->height, QImage::Format_RGB32);

        QDateTime tm = QDateTime::currentDateTime();
        QString strTime = tm.toString("yyyyMMdd_hhmmss_zzz");
        QString strSavePath = prefix + strTime + ".jpg";
        qDebug(strSavePath.toLocal8Bit().data());
        img.save(strSavePath.toLocal8Bit().data());

        m_snapshot->setGeometry(width()/4, height()/4, width()/2, height()/2);
        m_snapshot->setPixmap(QPixmap::fromImage(img).scaled(width()/2,height()/2));
        m_snapshot->show();
        QTimer::singleShot(1000, m_snapshot, SLOT(hide()) );

        free(rgb);
    }
}

void HGLWidget::startRecord(){

}

void HGLWidget::stopRecord(){

}

void HGLWidget::mousePressEvent(QMouseEvent* event){
    m_tmMousePressed = event->timestamp();
    m_ptMousePressed = event->pos();
    event->ignore();
}

void HGLWidget::mouseReleaseEvent(QMouseEvent* event){
    if ((event->timestamp() - m_tmMousePressed < 200)){
        emit clicked();
    }

    event->ignore();
}

void HGLWidget::mouseMoveEvent(QMouseEvent* e){
    // add delay to prevent misopration
    if ((e->timestamp() - m_tmMousePressed < 100)){
        e->accept();
        return;
    }

    e->ignore();
}

void HGLWidget::addIcon(int type, int x, int y, int w, int h){
    if (m_mapIcons.find(type) == m_mapIcons.end()){
        DrawInfo di;
        di.left = x;
        di.top = y;
        di.right = x+w;
        di.bottom = y+h;
        m_mapIcons[type] = di;
        update();
    }
}

void HGLWidget::removeIcon(int type){
    std::map<int,DrawInfo>::iterator iter = m_mapIcons.find(type);
    if (iter != m_mapIcons.end()){
        m_mapIcons.erase(iter);
    }
}

Texture* HGLWidget::getTexture(int type){
    switch(type){
    case HAVE_AUDIO:
        return &HRcLoader::instance()->tex_sound;
    }

    return NULL;
}

void HGLWidget::drawVideo(){
    DsSvrItem* item = g_dsCtx->getItem(srvid);
    if (item){
        item->mutex.lock();
        if (item->tex_yuv.data && item->tex_yuv.width > 0 && item->tex_yuv.height > 0){
            drawYUV(&item->tex_yuv);
            m_nPreFrame = item->v_input;
        }
        item->mutex.unlock();
    }
}

void HGLWidget::drawAudio(){
    // draw sound average
    DsSvrItem* item = g_dsCtx->getItem(srvid);
    if (item){
        DrawInfo di;
        if (item->a_channels > 1){
            di.left = width()-4 - AUDIO_WIDTH*2;
            di.top = height()-2 - AUDIO_HEIGHT;
            di.right = di.left + AUDIO_WIDTH;
            di.bottom = di.top + AUDIO_HEIGHT;
            di.color = 0x00FFFF80;
            drawRect(&di, 1, true);

            di.left += 1;
            di.right -= 1;
            di.bottom -= 1;
            di.top = di.bottom - item->a_average[1] * AUDIO_HEIGHT / 65536;
            di.color = 0xFFFF0080;
            drawRect(&di, 1, true);
        }

        di.left = width()-2 - AUDIO_WIDTH;
        di.top = height()-2 - AUDIO_HEIGHT;
        di.right = di.left + AUDIO_WIDTH;
        di.bottom = di.top + AUDIO_HEIGHT;
        di.color = 0x00FFFF80;
        drawRect(&di, 1, true);

        di.left += 1;
        di.right -= 1;
        di.bottom -= 1;
        di.top = di.bottom - item->a_average[0] * AUDIO_HEIGHT / 65536;
        di.color = 0xFFFF0080;
        drawRect(&di, 1, true);

        item->bUpdateAverage = true;
    }
}

void HGLWidget::drawIcon(){
    std::map<int,DrawInfo>::iterator iter = m_mapIcons.begin();
    while (iter != m_mapIcons.end()){
        Texture *tex = getTexture(iter->first);
        DrawInfo di = iter->second;
        if (tex){
            drawTex(tex, &di);
        }
        ++iter;
    }
}

void HGLWidget::drawTitle(){
    DsSvrItem* item = g_dsCtx->getItem(srvid);
    if (item && item->title.length() > 0){
        DrawInfo di;
        di.left = 2;
        di.top = 2;
        di.color = m_titcolor;
        drawStr(g_dsCtx->m_pFont, item->title.c_str(), &di);
    }
}

void HGLWidget::drawOutline(){
    DrawInfo di;
    di.left = 0;
    di.top = 0;
    di.right = width() - 1;
    di.bottom = height() - 1;
    di.color = m_outlinecolor;
    drawRect(&di, 3);
}

void HGLWidget::paintGL(){
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // drawVideo->drawAudio->drawTitle->drawOutline

    DrawInfo di;
    switch (m_status & MAJOR_STATUS_MASK) {
    case STOP:
        di.left = width()/2 - 50;
        di.top = height()/2 - 20;
        di.color = 0xFFFFFFFF;
        drawStr(g_dsCtx->m_pFont, "NO VIDEO!", &di);
        break;
    case NOSIGNAL:
        di.left = width()/2 - 50;
        di.top = height()/2 - 20;
        di.color = 0xFFFFFFFF;
        drawStr(g_dsCtx->m_pFont, "NO SIGNAL!", &di);
        break;
    case PAUSE:
    case PLAYING:
        if (m_status & PLAY_VIDEO){
            drawVideo();
        }

        if (m_status & PLAY_AUDIO){
            if (m_bDrawInfo){
                drawAudio();
            }
        }
        break;
    }

    if (m_bDrawInfo){
        drawTitle();
    }

    drawOutline();
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
HGeneralGLWidget::HGeneralGLWidget(QWidget* parent)
    : HGLWidget(parent)
{
    initUI();
    initConnect();
}

HGeneralGLWidget::~HGeneralGLWidget(){

}

void HGeneralGLWidget::initUI(){
    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->setMargin(2);

    m_titlebar = new HTitlebarWidget;
    m_titlebar->setFixedHeight(TITLE_BAR_HEIGHT);
    m_titlebar->hide();
    vbox->addWidget(m_titlebar);

    vbox->addStretch();

    m_toolbar = new HToolbarWidget;
    m_toolbar->setFixedHeight(TOOL_BAR_HEIGHT);
    m_toolbar->hide();
    vbox->addWidget(m_toolbar);

    setLayout(vbox);

    m_numSelector = new HNumSelectWidget(this);
    m_numSelector->setWindowFlags(Qt::Popup);
    m_numSelector->setAttribute(Qt::WA_TranslucentBackground, true);
}

void HGeneralGLWidget::initConnect(){
    QObject::connect( m_titlebar->m_btnFullScreen, SIGNAL(clicked(bool)), this, SIGNAL(fullScreen()) );
    QObject::connect( m_titlebar->m_btnExitFullScreen, SIGNAL(clicked(bool)), this, SIGNAL(exitFullScreen()) );
    QObject::connect( m_titlebar->m_btnDrawInfo, SIGNAL(clicked(bool)), this, SLOT(toggleDrawInfo()) );
    QObject::connect( m_titlebar->m_btnSnapshot, SIGNAL(clicked(bool)), this, SLOT(snapshot()) );
    //QObject::connect( m_titlebar->m_btnStartRecord, SIGNAL(clicked(bool)), this, SLOT(startRecord()) );
    //QObject::connect( m_titlebar->m_btnStopRecord, SIGNAL(clicked(bool)), this, SLOT(stopRecord()) );
    QObject::connect( m_titlebar->m_btnNum, SIGNAL(clicked(bool)), this, SLOT(showNumSelector()) );
    QObject::connect( m_titlebar->m_btnMicphoneOpened, SIGNAL(clicked(bool)), this, SLOT(closeMicphone()) );
    QObject::connect( m_titlebar->m_btnMicphoneClosed, SIGNAL(clicked(bool)), this, SLOT(openMicphone()) );
    QObject::connect( m_titlebar->m_btnVoice, SIGNAL(clicked(bool)), this, SLOT(onVoice()) );
    QObject::connect( m_titlebar->m_btnMute, SIGNAL(clicked(bool)), this ,SLOT(onMute()) );

    QObject::connect( m_toolbar->m_btnStart, SIGNAL(clicked(bool)), this, SLOT(onStart()) );
    QObject::connect( m_toolbar->m_btnPause, SIGNAL(clicked(bool)), this, SLOT(onPause()) );
    QObject::connect( m_toolbar, SIGNAL(progressChanged(int)), this, SLOT(onProgressChanged(int)) );

    QObject::connect( m_numSelector, SIGNAL(numSelected(int)), this, SLOT(onNumSelected(int)) );
    QObject::connect( m_numSelector, SIGNAL(numCanceled(int)), this, SLOT(onNumCanceled(int)) );
}

void HGeneralGLWidget::showTitlebar(bool bShow){
    if (bShow){
        HNetwork::instance()->queryVoice();

        DsSvrItem* item = g_dsCtx->getItem(srvid);
        if (item){
            m_titlebar->m_label->setText(item->title.c_str());
        }

        m_titlebar->m_btnMicphoneOpened->hide();
        m_titlebar->m_btnMicphoneClosed->hide();
        HScreenItem* screen = g_dsCtx->getHScreenItem(srvid);
        if (m_status & PLAY_AUDIO){
            if (screen){
                if (!screen->v && screen->a){
                    m_titlebar->m_btnMicphoneOpened->show();
                }
            }else{
                m_titlebar->m_btnMicphoneClosed->show();
            }
        }

        m_titlebar->m_btnVoice->hide();
        m_titlebar->m_btnMute->hide();
        if (item){
            if (item->bVoice){
                m_titlebar->m_btnVoice->show();
            }else{
                m_titlebar->m_btnMute->show();
            }
        }
        m_titlebar->show();
    }else{
        m_titlebar->hide();
    }
}

void HGeneralGLWidget::showToolbar(bool bShow){
    if (bShow){
        if ((m_status & MAJOR_STATUS_MASK) == PLAYING){
            m_toolbar->m_btnStart->hide();
            m_toolbar->m_btnPause->show();
        }
        if ((m_status & MAJOR_STATUS_MASK) == PAUSE){
            m_toolbar->m_btnStart->show();
            m_toolbar->m_btnPause->hide();
        }

        if (g_dsCtx->getItem(srvid)->src_type == SRC_TYPE_FILE){
            m_toolbar->m_slider->show();
        }else{
            m_toolbar->m_slider->hide();
        }
            m_toolbar->show();
    }else{
        m_toolbar->hide();
    }
}

bool HGeneralGLWidget::showToolWidgets(bool bShow){
    HGLWidget::showToolWidgets(bShow);

    if (bShow){
        if (!(status(MAJOR_STATUS_MASK) == PAUSE || status(MAJOR_STATUS_MASK) == PLAYING))
            return false;
    }

    showTitlebar(bShow);
    DsSvrItem* item = g_dsCtx->getItem(srvid);
    if (item){
        if (g_dsCtx->getItem(srvid)->src_type == SRC_TYPE_FILE){
            showToolbar(bShow);
        }
    }

    return bShow;
}

void HGeneralGLWidget::onNumSelected(int num){
    g_dsCtx->m_preselect[num-1] = srvid;
}

void HGeneralGLWidget::onNumCanceled(int num){
    g_dsCtx->m_preselect[num-1] = 0;
}

void HGeneralGLWidget::showNumSelector(){
    for (int i = 0; i < MAX_NUM_ICON; ++i){
        if (g_dsCtx->m_preselect[i] == srvid){
            m_numSelector->m_numSelects[i]->hide();
            m_numSelector->m_numCancels[i]->show();
        }else{
            m_numSelector->m_numCancels[i]->hide();
            m_numSelector->m_numSelects[i]->show();
        }
    }
    int w = m_numSelector->width();
    int h = m_numSelector->height();
    m_numSelector->setGeometry((width()-w)/2+x(), (height()-h)/2+y(), w, h);
    m_numSelector->show();
}

void HGeneralGLWidget::onProgressChanged(int progress){
    DsSvrItem* item = g_dsCtx->getItem(srvid);
    if (item && item->ifcb){
        qDebug("srvid=%d progress=%d ifservice_callback::e_service_cb_playratio", srvid, progress);
        item->ifcb->onservice_callback(ifservice_callback::e_service_cb_playratio, libchar(), 0, 0, progress, NULL);
    }
}

void HGeneralGLWidget::openMicphone(){
    HNetwork::instance()->setMicphone(srvid);
}

void HGeneralGLWidget::closeMicphone(){
    HNetwork::instance()->setMicphone(0);
}

void HGeneralGLWidget::onVoice(){
    HNetwork::instance()->setVoice(srvid, 0);
}

void HGeneralGLWidget::onMute(){
    HNetwork::instance()->setVoice(srvid, 1);
}

void HGeneralGLWidget::drawSelectNum(){
    DrawInfo di;
    di.top = height()-48;
    di.bottom = height()-1;
    di.left = 1;
    di.right = 48;
    for (int i = 0; i < MAX_NUM_ICON; ++i){
        if (g_dsCtx->m_preselect[i] == srvid){
            drawTex(&HRcLoader::instance()->tex_numr[i], &di);
            di.left += 48;
            di.right += 48;
        }
    }
}

void HGeneralGLWidget::drawSound(){
    HScreenItem* item = g_dsCtx->getHScreenItem(srvid);
    if (item && item->a){
        DrawInfo di;
        Texture *tex = getTexture(HAVE_AUDIO);
        di.right = width() - 1;
        di.top = 1;
        di.left = di.right - 32 + 1;
        di.bottom = di.top + 32 - 1;
        drawTex(tex, &di);
    }
}

void HGeneralGLWidget::drawOutline(){
    DrawInfo di;
    di.left = 0;
    di.top = 0;
    di.right = width() - 1;
    di.bottom = height() - 1;
    if (m_titlebar->isVisible()){
        di.color = g_dsCtx->m_tInit.focus_outlinecolor;
    }else{
        di.color = m_outlinecolor;
    }
    drawRect(&di, 3);
}

void HGeneralGLWidget::paintGL(){
    HGLWidget::paintGL();

    if (status(MAJOR_STATUS_MASK) == PLAYING || status(MAJOR_STATUS_MASK) == PAUSE){
        drawSelectNum();
        drawSound();
    }
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//void HOperateTarget::initWidget(HOperateTargetWidget* wdg){
//    if (pItem->type == HAbstractItem::TEXT){
//        QString str;
//        HTextItem* pItem = (HTextItem*)this->pItem;
//        if (pItem->text_type == HTextItem::LABEL){
//            str = pItem->text;
//        }else if (pItem->text_type == HTextItem::TIME){
//            str = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
//        }else if (pItem->text_type == HTextItem::WATCHER){
//            str = "00:00:00:0";
//        }else if (pItem->text_type == HTextItem::SUBTITLE){
//            str = "字幕";
//        }
//        wdg->setText(str);
//        QFont font = wdg->font();
//        font.setPointSize(pItem->font_size*0.8);
//        font.setLetterSpacing(QFont::AbsoluteSpacing,2);
//        wdg->setFont(font);

//        QFontMetrics fm(font);
//        int w = fm.width(str) + 20;
//        int h = fm.height();
//        wdg->setGeometry(rcDraw);
//        wdg->show();
//    }else if (pItem->type == HAbstractItem::PICTURE){
//        HPictureItem* pItem = (HPictureItem*)this->pItem;
//        wdg->src_pixmap.load(pItem->src);
//        if (wdg->src_pixmap.isNull())
//            return;

//        QPixmap pixmap = wdg->src_pixmap;
//        int w = pixmap.width();
//        int h = pixmap.height();
//        if (w > EXPRE_MAX_WIDTH || h > EXPRE_MAX_HEIGHT){
//            w = EXPRE_MAX_WIDTH;
//            h = EXPRE_MAX_HEIGHT;
//            pixmap = pixmap.scaled(QSize(w,h));
//        }

//        wdg->setFixedSize(w,h);
//        wdg->setPixmap(pixmap);
//        wdg->show();
//    }else if (pItem->type == HAbstractItem::SCREEN){
//        HScreenItem* pItem = (HScreenItem*)this->pItem;
//    }
//}

HCombGLWidget::HCombGLWidget(QWidget* parent)
    : HGLWidget(parent)
{
    m_bMouseMoving = false;
    m_target = NULL;
    m_targetPrev = NULL;
    m_targetShow = NULL;
    m_bLockToolbar = false;

    m_virtualTarget = new HOperateTarget(NULL);

    initUI();
    initConnect();
}

HCombGLWidget::~HCombGLWidget(){
    if (m_virtualTarget){
        delete m_virtualTarget;
        m_virtualTarget = NULL;
    }
}

void HCombGLWidget::initUI(){
    m_targetWdg = new HOperateTargetWidget(this);
    m_targetWdg->hide();

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->setMargin(2);

    m_titlebar = new HCombTitlebarWidget;
    m_titlebar->setFixedHeight(TOOL_BAR_HEIGHT);
    m_titlebar->hide();
    vbox->addWidget(m_titlebar);

    vbox->addStretch();

    m_toolbar = new HCombToolbarWidget;
    m_toolbar->setFixedHeight(TOOL_BAR_HEIGHT);
    m_toolbar->hide();
    vbox->addWidget(m_toolbar);

    setLayout(vbox);

    m_wdgTrash = new HChangeColorWidget(this);
    m_wdgTrash->setPixmap(HRcLoader::instance()->icon_trash_big);
    m_wdgTrash->hide();

    m_wdgExpre = new HExpreWidget(this);
    m_wdgExpre->setWindowFlags(Qt::Popup);
}

void HCombGLWidget::initConnect(){
    QObject::connect( g_dsCtx, SIGNAL(combChanged()), this, SLOT(onCombChanged()) );
    QObject::connect( HNetwork::instance(), SIGNAL(overlayChanged()), this, SLOT(onOverlayChanged()) );

    QObject::connect( m_titlebar->m_btnFullScreen, SIGNAL(clicked(bool)), this, SIGNAL(fullScreen()) );
    QObject::connect( m_titlebar->m_btnExitFullScreen, SIGNAL(clicked(bool)), this, SIGNAL(exitFullScreen()) );
    QObject::connect( m_titlebar->m_btnPinb, SIGNAL(clicked(bool)), this, SLOT(lockTools()) );
    QObject::connect( m_titlebar->m_btnPinr, SIGNAL(clicked(bool)), this, SLOT(unlockTools()) );
    QObject::connect( m_titlebar->m_btnDrawInfo, SIGNAL(clicked(bool)), this, SLOT(toggleDrawInfo()) );
    QObject::connect( m_titlebar->m_btnSnapshot, SIGNAL(clicked(bool)), this, SLOT(snapshot()) );
    //QObject::connect( m_titlebar->m_btnStartRecord, SIGNAL(clicked(bool)), this, SLOT(startRecord()) );
    //QObject::connect( m_titlebar->m_btnStopRecord, SIGNAL(clicked(bool)), this, SLOT(stopRecord()) );

    QObject::connect( m_toolbar->m_btnStart, SIGNAL(clicked(bool)), this, SLOT(onStart()) );
    QObject::connect( m_toolbar->m_btnPause, SIGNAL(clicked(bool)), this, SLOT(onPause()) );
    QObject::connect( m_toolbar->m_btnUndo, SIGNAL(clicked(bool)), this, SLOT(onUndo()) );
    QObject::connect( m_toolbar->m_btnTrash, SIGNAL(clicked(bool)), this, SLOT(onTrash()) );
    QObject::connect( m_toolbar->m_btnExpre, SIGNAL(clicked(bool)), this, SLOT(showExpre()) );
    QObject::connect( m_toolbar->m_btnOK, SIGNAL(clicked(bool)), this, SLOT(onOK()) );
    QObject::connect( m_toolbar->m_btnText, SIGNAL(clicked(bool)), this, SLOT(showText()) );
    QObject::connect( m_toolbar->m_btnSetting, SIGNAL(clicked(bool)), this, SLOT(onSetting()) );
    QObject::connect( m_toolbar->m_btnZoomIn, SIGNAL(clicked(bool)), this, SLOT(onZoomIn()) );
    QObject::connect( m_toolbar->m_btnZoomOut, SIGNAL(clicked(bool)), this, SLOT(onZoomOut()) );

    QObject::connect( m_wdgExpre, SIGNAL(expreSelected(QString&)), this, SLOT(onExpreSelected(QString&)) );
}

void HCombGLWidget::showTitlebar(bool bShow){
    if (bShow){
        DsSvrItem* item = g_dsCtx->getItem(srvid);
        if (item){
            m_titlebar->m_label->setText(item->title.c_str());
        }
        m_titlebar->show();
    }else{
        m_titlebar->hide();
    }
}

void HCombGLWidget::showToolbar(bool bShow){
    if (bShow){
        if ((m_status & MAJOR_STATUS_MASK) == PLAYING){
            m_toolbar->m_btnStart->hide();
            m_toolbar->m_btnPause->show();
        }
        if ((m_status & MAJOR_STATUS_MASK) == PAUSE){
            m_toolbar->m_btnStart->show();
            m_toolbar->m_btnPause->hide();
        }
        m_toolbar->show();
    }else{
        m_toolbar->hide();
    }
}

bool HCombGLWidget::showToolWidgets(bool bShow){
    if (m_bLockToolbar)
        return true;

    HGLWidget::showToolWidgets(bShow);

    showTitlebar(bShow);
    showToolbar(bShow);
    //m_wdgTrash->setVisible(bShow);

    return bShow;
}

void HCombGLWidget::onTargetChanged(){
    if (m_targetPrev && m_targetPrev->wdg){
        m_targetPrev->wdg->setStyleSheet("border:3px dashed white;");
        if (m_targetPrev->pItem->type == HAbstractItem::TEXT){
            HTextItem* pItem = (HTextItem*)m_targetPrev->pItem;
            QPalette pal = m_targetPrev->wdg->palette();
            pal.setColor(QPalette::Foreground, pItem->font_color);
            m_targetPrev->wdg->setPalette(pal);
        }
    }

    if (m_target && m_target->wdg){
        m_target->wdg->setStyleSheet("border:3px dashed red;");
        if (m_target->pItem->type == HAbstractItem::TEXT){
            HTextItem* pItem = (HTextItem*)m_target->pItem;
            QPalette pal = m_target->wdg->palette();
            pal.setColor(QPalette::Foreground, pItem->font_color);
            m_target->wdg->setPalette(pal);
        }
    }
}

#define LOCATION_PADDING    32
#define MIN_LOCATION        96
int HCombGLWidget::getLocation(QPoint pt, QRect rc){
    int loc = NotIn;
    if (rc.contains(pt)){
         if (rc.width() < MIN_LOCATION || rc.height() < MIN_LOCATION)
             return Center;

         if (pt.x() - rc.left() < LOCATION_PADDING)
             loc |= Left;
         if (rc.right() - pt.x() < LOCATION_PADDING)
             loc |= Right;
         if (pt.y() - rc.top() < LOCATION_PADDING)
             loc |= Top;
         if (rc.bottom() - pt.y() < LOCATION_PADDING)
             loc |= Bottom;
         return loc == NotIn ? Center : loc;
    }

    return NotIn;
}

HOperateTarget* HCombGLWidget::getItemByPos(QPoint pt, HAbstractItem::TYPE type){
    // 优先顺序: TEMPORARY > OVERLAY > SCREEN
    // TEXT > PICTURE
    // SUB_SCREEN > MAIN_SCREEN

    if (type == HAbstractItem::ALL){
        if (m_virtualTarget && m_virtualTarget->wdg){
            if (m_virtualTarget->wdg->isVisible() && m_virtualTarget->wdg->geometry().contains(pt))
                return m_virtualTarget;
        }

        if (m_targetShow && m_targetShow->wdg){
            if (m_targetShow->wdg->isVisible() && m_targetShow->wdg->geometry().contains(pt))
                return m_targetShow;
        }
    }

    if (type == HAbstractItem::ALL || type == HAbstractItem::TEXT){
        for (int i = m_vecTexts.size()-1; i >=0; --i){
            if (m_vecTexts[i].rcDraw.contains(pt)){
                return &m_vecTexts[i];
            }
        }
    }

    if (type == HAbstractItem::ALL || type == HAbstractItem::PICTURE){
        for (int i = m_vecPictures.size()-1; i >=0; --i){
            if (m_vecPictures[i].rcDraw.contains(pt)){
                return &m_vecPictures[i];
            }
        }
    }

    if (type == HAbstractItem::ALL || type == HAbstractItem::SCREEN){
        for (int i = m_vecScreens.size()-1; i >= 0; --i){
            if (m_vecScreens[i].rcDraw.contains(pt)){
                return &m_vecScreens[i];
            }
        }
    }

    return NULL;
}

QRect HCombGLWidget::adjustPos(QRect rc){
    int x = rc.x();
    int y = rc.y();
    int w = rc.width();
    int h = rc.height();
    if (rc.left() < 0)
        x = 0;
    if (rc.right() >= width())
        x = width()-w;
    if (rc.top() < 0)
        y = 0;
    if (rc.bottom() >= height())
        y = height()-h;

    return QRect(x,y,w,h);
}

QRect HCombGLWidget::scaleToOrigin(QRect rc){
    if (width() == 0 || height() == 0)
        return rc;
    int x = rc.x() * g_dsCtx->m_tComb.width / width();
    int w = rc.width() * g_dsCtx->m_tComb.width / width();
    int y = rc.y() * g_dsCtx->m_tComb.height / height();
    int h = rc.height() * g_dsCtx->m_tComb.height / height();
    return QRect(x,y,w,h);
}

QRect HCombGLWidget::scaleToDraw(QRect rc){
    if (g_dsCtx->m_tComb.width == 0 || g_dsCtx->m_tComb.height == 0)
        return rc;

    int x = rc.x() * width() / g_dsCtx->m_tComb.width;
    int w = rc.width() * width() / g_dsCtx->m_tComb.width;
    int y = rc.y() * height() / g_dsCtx->m_tComb.height;
    int h = rc.height() * height() / g_dsCtx->m_tComb.height;
    return QRect(x,y,w,h);
}

void HCombGLWidget::onCombChanged(){
    m_vecScreens.clear();
    for (int i = 0; i < g_dsCtx->m_tComb.itemCnt; ++i){
        if (g_dsCtx->m_tComb.items[i].v){
            HOperateTarget target(&g_dsCtx->m_tComb.items[i]);
            target.rcDraw = scaleToDraw(target.pItem->rc);
            m_vecScreens.push_back(target);
        }
    }

    HNetwork::instance()->queryOverlayInfo();
}

void HCombGLWidget::onOverlayChanged(){
    m_vecPictures.clear();
    std::vector<HPictureItem>& pics = HNetwork::instance()->m_vecPictures;
    for (int i = 0; i < pics.size(); ++i){        
        HOperateTarget target(&pics[i]);
        target.rcDraw = scaleToDraw(target.pItem->rc);
        m_vecPictures.push_back(target);
    }

    m_vecTexts.clear();
    std::vector<HTextItem>& texts = HNetwork::instance()->m_vecTexts;
    for (int i = 0; i < texts.size(); ++i){        
        HOperateTarget target(&texts[i]);
        target.rcDraw = scaleToDraw(target.pItem->rc);
        m_vecTexts.push_back(target);
    }

    m_target = NULL;
}

void HCombGLWidget::onUndo(){
    HAbstractItem::onUndo();
}

void HCombGLWidget::onTrash(){
    if (m_target){
        if (m_target->wdg){
            m_target->wdg->hide();
            m_target->detachItem();
            m_target->detachWidget();
        }else{
            m_target->pItem->savePreStatus();
            m_target->pItem->remove();
        }

        m_target = NULL;
    }
}

void HCombGLWidget::onOK(){
    if (m_target && m_target->wdg && m_target->isModifiable()){
        m_target->wdg->hide();
        m_target->pItem->savePreStatus();
        m_target->pItem->rc = scaleToOrigin(m_target->wdg->geometry());
        m_target->pItem->addOrMod();

        m_target->detachItem();
        m_target->detachWidget();
        m_target = NULL;
    }
}

void HCombGLWidget::onZoomIn(){
    showTargetWidget();

    if (m_target && m_target->wdg){
        QRect rc = m_target->wdg->geometry();
        QPoint ptCenter = rc.center();
        int w = rc.width() * 1.1;
        int h = rc.height() * 1.1;
        if (w > width())
            w = width();
        if (h > height())
            h = height();
        QRect rcDst = adjustPos(QRect(ptCenter.x() - w/2, ptCenter.y() - h/2, w, h ));
        m_target->wdg->setGeometry(rcDst);
    }
}

void HCombGLWidget::onZoomOut(){
    showTargetWidget();

    if (m_target && m_target->wdg){
        QRect rc = m_target->wdg->geometry();
        QPoint ptCenter = rc.center();
        int w = rc.width() * 0.9;
        int h = rc.height() * 0.9;
        if (w < 2)
            w = 2;
        if (h < 2)
            h = 2;
        QRect rcDst = adjustPos(QRect(ptCenter.x() - w/2, ptCenter.y() - h/2, w, h ));
        m_target->wdg->setGeometry(rcDst);
    }
}

#include "hsettingwidget.h"
void HCombGLWidget::onSetting(){
    HSettingWidget dlg(this);
    dlg.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    int w = dlg.width();
    int h = dlg.height();
    dlg.move(x() + (width() - w)/2, y() + m_toolbar->y() - h);
    if (dlg.exec() == QDialog::Accepted){
        //...
    }
}

void HCombGLWidget::showText(){
    HAddTextWidget dlg(this);
    dlg.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    int w = dlg.width();
    int h = dlg.height();
    dlg.move(x() + (width() - w)/2, y() + m_toolbar->y() - h);
    if (dlg.exec() == QDialog::Accepted){
        HTextItem* pItem = new HTextItem(dlg.m_TextItem);
        QString str;
        if (pItem->text_type == HTextItem::LABEL){
            str = pItem->text;
        }else if (pItem->text_type == HTextItem::TIME){
            str = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        }else if (pItem->text_type == HTextItem::WATCHER){
            str = "00:00:00:0";
        }else if (pItem->text_type == HTextItem::SUBTITLE){
            str = "字幕";
        }
        m_virtualTarget->attachItem(pItem);
        m_virtualTarget->attachWidget(m_targetWdg);
        m_virtualTarget->wdg->setPixmap(QPixmap());
        m_virtualTarget->wdg->setText(str);
        QFont font = m_virtualTarget->wdg->font();
        font.setPointSize(pItem->font_size*0.8);
        font.setLetterSpacing(QFont::AbsoluteSpacing,2);
        m_virtualTarget->wdg->setFont(font);

        QPalette pal = m_virtualTarget->wdg->palette();
        pal.setColor(QPalette::Foreground, pItem->font_color);
        m_virtualTarget->wdg->setPalette(pal);

        QFontMetrics fm(font);
        int w = fm.width(str) + 20;
        int h = fm.height();
        m_virtualTarget->wdg->setGeometry(QRect((width()-w)/2, (height()-h)/2, w, h));
        m_virtualTarget->wdg->show();

        m_target = m_virtualTarget;
        onTargetChanged();
    }
}

void HCombGLWidget::showExpre(){
    int w = m_wdgExpre->width();
    int h = m_wdgExpre->height();
    m_wdgExpre->move(x() + (width() - w)/2, y() + m_toolbar->y() - h);
    m_wdgExpre->show();
}

#define EXPRE_MAX_WIDTH     128
#define EXPRE_MAX_HEIGHT    128
void HCombGLWidget::onExpreSelected(QString& filepath){
    HPictureItem* pItem = new HPictureItem;
    strncpy(pItem->src, filepath.toLocal8Bit().constData(), MAXLEN_STR);

    m_virtualTarget->attachItem(pItem);
    m_virtualTarget->attachWidget(m_targetWdg);

    QPixmap pixmap;
    pixmap.load(filepath);
    m_virtualTarget->wdg->setPixmap(pixmap);

    int w = pixmap.width();
    int h = pixmap.height();
    if (w > EXPRE_MAX_WIDTH || h > EXPRE_MAX_HEIGHT){
        w = EXPRE_MAX_WIDTH;
        h = EXPRE_MAX_HEIGHT;
    }
    m_virtualTarget->wdg->setGeometry(QRect((width()-w)/2, (height()-h)/2, w, h));
    m_virtualTarget->wdg->show();

    m_target = m_virtualTarget;
    onTargetChanged();
}

bool HCombGLWidget::showTargetWidget(){
    if (!m_target || !m_target->isModifiable())
        return false;

    if (m_targetShow && m_targetShow->wdg){
        if (m_targetShow->wdg->isVisible() && m_target == m_targetShow)
            return true;
        else
            m_targetShow->detachWidget();
    }

    if (m_target->wdg == NULL && m_target->isExist()){
        m_target->attachWidget(m_targetWdg);
        if (m_target->pItem->type == HAbstractItem::PICTURE){
            HPictureItem* pItem = (HPictureItem*)m_target->pItem;
            QPixmap pixmap;
            pixmap.load(pItem->src);
            m_target->wdg->setPixmap(pixmap);
        }else{
            m_target->wdg->setPixmap(grab(m_target->rcDraw));
        }
        m_target->wdg->setGeometry(m_target->rcDraw);
        m_target->wdg->show();

        m_targetShow = m_target;
    }

    return true;
}

void HCombGLWidget::drawOutline(){
    DrawInfo di;
    di.left = 0;
    di.top = 0;
    di.right = width() - 1;
    di.bottom = height() - 1;
    di.color = m_outlinecolor;
    drawRect(&di, 3);
}

void HCombGLWidget::drawTaskInfo(){
    DrawInfo di;
    if (g_dsCtx->m_pFont){
        int oldSize = g_dsCtx->m_pFont->FaceSize();
        g_dsCtx->m_pFont->FaceSize(32);
        separator sept(g_dsCtx->m_strTaskInfo.c_str(), "\r\n");
        di.top = 10;
        di.left = 10;
        di.color = g_dsCtx->m_tInit.infcolor;
        for (int i = 0; i < sept.size(); ++i){
            drawStr(g_dsCtx->m_pFont, sept[i], &di);
            di.top += g_dsCtx->m_pFont->LineHeight() + 10;
        }
        g_dsCtx->m_pFont->FaceSize(oldSize);
    }
}

void HCombGLWidget::drawScreenInfo(){
    DrawInfo di;
    for (int i = 0; i < m_vecScreens.size(); ++i){
        QRect rc= m_vecScreens[i].rcDraw;
        // draw comb NO.
        di.left = rc.left() + 1;
        di.bottom = rc.bottom() - 1;
        di.top = di.bottom - 48 + 1;
        di.right = di.left + 48 - 1;
        drawTex(&HRcLoader::instance()->tex_numr[i], &di);

        // draw comb outline
        di.left = rc.left();
        di.top = rc.top();
        di.right = rc.right();
        di.bottom = rc.bottom();
        di.color = m_outlinecolor;
        drawRect(&di);
    }
}

void HCombGLWidget::drawPictureInfo(){
    DrawInfo di;
    for (int i = 0; i < m_vecPictures.size(); ++i){
        QRect rc= m_vecPictures[i].rcDraw;
        // draw picture outline
        di.left = rc.left();
        di.top = rc.top();
        di.right = rc.right();
        di.bottom = rc.bottom();
        di.color = m_outlinecolor;
        drawRect(&di);
    }
}

void HCombGLWidget::drawTextInfo(){
    DrawInfo di;
    for (int i = 0; i < m_vecTexts.size(); ++i){
        QRect rc= m_vecTexts[i].rcDraw;
        // draw text outline
        di.left = rc.left();
        di.top = rc.top();
        di.right = rc.right();
        di.bottom = rc.bottom();
        di.color = m_outlinecolor;
        drawRect(&di);
    }
}

void HCombGLWidget::paintGL(){
    HGLWidget::paintGL();

    if (g_dsCtx->m_tInit.info){
        drawTaskInfo();
    }

    if (m_bDrawInfo){
        drawScreenInfo();
        drawPictureInfo();
        drawTextInfo();
    }

    // draw focused target outline
    if (m_target && m_target->wdg == NULL){
        DrawInfo di;
        QRect rc = m_target->rcDraw;
        di.left = rc.x();
        di.top = rc.y();
        di.right = rc.right();
        di.bottom = rc.bottom();
        di.color = g_dsCtx->m_tInit.focus_outlinecolor;
        drawRect(&di, 3);
    }
}

void HCombGLWidget::resizeEvent(QResizeEvent *e){
    m_wdgTrash->setGeometry(width()-128-1, height()/2-64, 128, 128);

    onCombChanged();

    m_target = NULL;

    HGLWidget::resizeEvent(e);
}

void HCombGLWidget::mousePressEvent(QMouseEvent* e){
    HGLWidget::mousePressEvent(e);

    m_targetPrev = m_target;
    m_target = getItemByPos(e->pos());

    if (m_target != m_targetPrev)
        onTargetChanged();
}

void HCombGLWidget::mouseMoveEvent(QMouseEvent *e){
    HGLWidget::mouseMoveEvent(e);
    if (e->isAccepted())
        return;

    if (!rect().contains(e->pos())){
        e->accept();
        return;
    }

    if (!m_bMouseMoving){
        m_bMouseMoving = true;
        // moveBegin
        showTargetWidget();
        if (m_target && m_target->wdg){
            m_location = getLocation(e->pos(), m_target->wdg->geometry());
        }
    }

    if (m_target && m_target->wdg){
        QRect rc = m_target->wdg->geometry();

        if ((m_location & Center) || m_target->pItem->type == HAbstractItem::TEXT){
            // move
            int w = rc.width();
            int h = rc.height();
            rc.setRect(e->x()-w/2, e->y()-h/2, w, h);
            rc = adjustPos(rc);
        }else{
            // resize
            if (m_location & Left){
                if (e->x() < rc.right() - 2*LOCATION_PADDING)
                    rc.setLeft(e->x());
            }else if (m_location & Right){
                if (e->x() > rc.left() + 2*LOCATION_PADDING)
                rc.setRight(e->x());
            }
            if (m_location & Top){
                if (e->y() < rc.bottom() - 2*LOCATION_PADDING)
                    rc.setTop(e->y());
            }else if (m_location & Bottom){
                if (e->y() > rc.top() + 2*LOCATION_PADDING)
                    rc.setBottom(e->y());
            }
        }

        m_target->wdg->setGeometry(rc);
    }

    if (m_wdgTrash->isVisible()){
        if (m_wdgTrash->geometry().contains(e->pos())){
            m_wdgTrash->changeColor(QColor(255, 0, 0, 128));
        }else{
            m_wdgTrash->changeColor(Qt::transparent);
        }
    }
}

void HCombGLWidget::mouseReleaseEvent(QMouseEvent *e){
    HGLWidget::mouseReleaseEvent(e);

    if (m_bMouseMoving){
        m_bMouseMoving = false;
        // moveEnd

        if (m_wdgTrash->isVisible() && m_wdgTrash->geometry().contains(e->pos())){
            onTrash();
        }

        if (!m_toolbar->isVisible()){
            onOK();
        }
    }
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
