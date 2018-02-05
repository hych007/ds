#include "hglwidget.h"
#include "hdsctx.h"
#include "hrcloader.h"

HGLWidget::HGLWidget(QWidget *parent)
    : QGLWidgetImpl(parent)
{
    wndid = 0;
    srvid = 0;

    fps = 0;
    framecnt = 0;
    m_status = STOP;
    m_tmMousePressed = 0;

    m_bDrawInfo = true;
    m_bFullScreen = false;

    m_snapshot = new QLabel(this);
    m_snapshot->setStyleSheet("border:5px double #ADFF2F");
    m_snapshot->hide();

    if (g_dsCtx->m_tInit.mouse)
        setMouseTracking(true);
}

HGLWidget::~HGLWidget(){

}

bool HGLWidget::showToolWidgets(bool bShow){
    updateToolWidgets();
    return bShow;
}

void HGLWidget::onStart(){
    g_dsCtx->pause(srvid, false);
}

void HGLWidget::onPause(){    
    g_dsCtx->pause(srvid, true);
    setStatus(PAUSE);
}

void HGLWidget::onStop(){
    resetStatus();
}

#include "hffmpeg.h"
#include <QDir>
#include <QDateTime>
void HGLWidget::snapshot(){
    //test snapshot
    DsSrvItem* item = g_dsCtx->getSrvItem(srvid);
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
    if (!g_dsCtx->m_tInit.mouse){
        if ((e->timestamp() - m_tmMousePressed < 100)){
            e->accept();
            return;
        }
    }

    e->ignore();
}

void HGLWidget::mouseDoubleClickEvent(QMouseEvent* e){
    if (g_dsCtx->m_tInit.mouse){
        m_bFullScreen = !m_bFullScreen;
        emit fullScreen(m_bFullScreen);
    }
}

void HGLWidget::resizeEvent(QResizeEvent* e){    
    if (!isResetStatus() && type != EXTEND){
        g_dsCtx->onWndSizeChanged(srvid, videoArea());
    }

    QGLWidgetImpl::resizeEvent(e);
}

void HGLWidget::showEvent(QShowEvent* e){
    if (!isResetStatus() && type != EXTEND){
        g_dsCtx->onWndVisibleChanged(srvid, true);
    }
}

void HGLWidget::hideEvent(QHideEvent* e){    
    if (!isResetStatus() && type != EXTEND){
        g_dsCtx->onWndVisibleChanged(srvid, false);
    }
}

void HGLWidget::enterEvent(QEvent* e){    
    if (g_dsCtx->m_tInit.mouse)
        showToolWidgets(true);
}

void HGLWidget::leaveEvent(QEvent* e){
    if (g_dsCtx->m_tInit.mouse)
        showToolWidgets(false);
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
        return &rcloader->tex_sound;
    }

    return NULL;
}

void HGLWidget::calFps(){
    if (framecnt == 0)
        timer_elapsed.restart();

    if (timer_elapsed.elapsed() > 1000){
        fps = framecnt;
        framecnt = 0;
    }else{
        ++framecnt;
    }
}

void HGLWidget::drawFps(){
    DrawInfo di;
    di.left = width() - 100;
    di.top = 0;
    di.color = 0x0000FFFF;
    char szFps[8];
    snprintf(szFps, 8, "FPS:%d", fps);
    drawStr(szFps, &di);
}

void HGLWidget::drawVideo(){
    DsSrvItem* item = g_dsCtx->getSrvItem(srvid);
    if (item){
        if (item->tex_yuv.data && item->tex_yuv.width != 0 && item->tex_yuv.height != 0) 
            drawYUV(&item->tex_yuv);
    }
}

void HGLWidget::drawAudioStyle1(QRect rc, int num){
    DrawInfo di;
    di.left = rc.left();
    di.right = rc.right();
    di.top = rc.top();
    di.bottom = rc.bottom();
    di.color = g_dsCtx->m_tInit.audiocolor_bg;
    drawRect(&di, 1, true);

    di.left += 1;
    di.right -= 1;
    di.bottom -= 1;
    int h = rc.height() * num / 65536;
    di.top = di.bottom - h;
    if (num < 65536 * 0.6)
        di.color = g_dsCtx->m_tInit.audiocolor_fg_low;
    else if(num < 65536 * 0.8)
        di.color = g_dsCtx->m_tInit.audiocolor_fg_high;
    else
        di.color = g_dsCtx->m_tInit.audiocolor_fg_top;
    drawRect(&di, 1, true);
}

void HGLWidget::drawAudioStyle2(QRect rc, int num){
    DrawInfo di;
    di.left = rc.left();
    di.right = rc.right();
    di.bottom = rc.bottom();
    di.top = di.bottom;
    double percent = (double)num / 65536;
    double cnt = 0.0f;
    double span = 2.0f / 100;
    int cell_h = rc.height() * span;
    int cell_color_h = cell_h * 2 / 3;
    di.color = g_dsCtx->m_tInit.audiocolor_fg_low;
    while (cnt < percent && cnt < 0.6){
        di.top = di.bottom - cell_color_h;
        drawRect(&di, 1, true);
        di.bottom -= cell_h;
        cnt += span;
    }

    di.color = g_dsCtx->m_tInit.audiocolor_fg_high;
    while (cnt < percent && cnt < 0.8){
        di.top = di.bottom - cell_color_h;
        drawRect(&di, 1, true);
        di.bottom -= cell_h;
        cnt += span;
    }

    di.color = g_dsCtx->m_tInit.audiocolor_fg_top;
    while (cnt < percent && cnt <= 1.0){
        di.top = di.bottom - cell_color_h;
        drawRect(&di, 1, true);
        di.bottom -= cell_h;
        cnt += span;
    }
}

void HGLWidget::drawAudio(){
    // draw sound average
    DsSrvItem* item = g_dsCtx->getSrvItem(srvid);
    if (item){
        if (item->a_average[0] > 0 || item->a_average[1] > 0){
            if (g_dsCtx->m_tInit.audiostyle == 1){
                QRect rc(width()-2 - AUDIO_WIDTH, height()-2 - AUDIO_HEIGHT, AUDIO_WIDTH, AUDIO_HEIGHT);
                drawAudioStyle1(rc, item->a_average[0]);

                if (item->a_channels > 1){
                    QRect rc(width()-4 - AUDIO_WIDTH*2, height()-2 - AUDIO_HEIGHT, AUDIO_WIDTH, AUDIO_HEIGHT);
                    drawAudioStyle1(rc, item->a_average[1]);
                }
            }else if(g_dsCtx->m_tInit.audiostyle == 2){
                QRect rc(width()-2 - AUDIO_WIDTH,  38, AUDIO_WIDTH, height() - 40);
                drawAudioStyle2(rc, item->a_average[0]);

                if (item->a_channels > 1){
                    QRect rc(width()-4 - AUDIO_WIDTH*2, 38, AUDIO_WIDTH, height() - 40);
                    drawAudioStyle2(rc, item->a_average[1]);
                }
            }
        }

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
    DsSrvItem* item = g_dsCtx->getSrvItem(srvid);
    if (item && item->title.length() > 0){
        DrawInfo di;
        di.left = 2;
        di.top = 2;
        di.color = g_dsCtx->m_tInit.titcolor;
        drawStr(item->title.c_str(), &di);
    }
}

void HGLWidget::drawTaskInfo(){
    DsSrvItem* item = g_dsCtx->getSrvItem(srvid);
    if (item && QGLWidgetImpl::m_pFont){
        DrawInfo di;
        di.left = 0;
        di.top = height() - QGLWidgetImpl::m_pFont->LineHeight() - 8;
        di.right = width()-1;
        di.bottom = height()-1;
        di.color = 0x000000FF;
        drawRect(&di, 1, true);

        QFont font;
        font.setPixelSize(QGLWidgetImpl::m_pFont->FaceSize());
        QFontMetrics fm(font);
        int w = fm.width(item->taskinfo);
        int x = (di.right-di.left-w)/2;
        di.left += x > 2 ? x : 2;
        di.color = g_dsCtx->m_tInit.infcolor;
        drawStr(item->taskinfo.toLocal8Bit().data(), &di);
    }
}

void HGLWidget::drawOutline(){
    DrawInfo di;
    di.left = 0;
    di.top = 0;
    di.right = width() - 1;
    di.bottom = height() - 1;
    di.color = g_dsCtx->m_tInit.outlinecolor;
    drawRect(&di, 3);
}

void HGLWidget::drawDebugInfo(){
    char info[256];

    DrawInfo di;
    di.left = 2;
    di.top = 2;
    di.color = 0xFFFF00FF;
    sprintf(info, "wnd: %02d %d*%d => srvid: %02d", wndid, width(), height(), srvid);
    drawStr(info, &di);

    int span = QGLWidgetImpl::m_pFont->LineHeight() + 8;
    DsSrvItem* item = g_dsCtx->getSrvItem(srvid);
    if (item){
        di.top += span;
        sprintf(info, "src: %s", item->src_addr.toLocal8Bit().data());
        drawStr(info, &di);

        di.top += span;
        sprintf(info, "pic: %d*%d | %d", item->pic_w, item->pic_h, item->framerate);
        drawStr(info, &di);

        di.top += span;
        sprintf(info, "show: %d*%d scale: %d*%d", item->show_w, item->show_h, item->tex_yuv.width, item->tex_yuv.height);
        drawStr(info, &di);

        di.top += span;
        sprintf(info, "input: %u | %u fps: %d", item->a_input, item->v_input, fps);
        drawStr(info, &di);
    }
}

void HGLWidget::paintGL(){
    calFps();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // drawVideo->drawAudio->drawTitle->drawOutline

    DrawInfo di;
    switch (m_status & MAJOR_STATUS_MASK) {
    case STOP:
        di.left = width()/2 - 50;
        di.top = height()/2 - 20;
        di.color = 0xFFFFFFFF;
        drawStr("无视频", &di);
        break;
    case NOSIGNAL:
        di.left = width()/2 - 50;
        di.top = height()/2 - 20;
        di.color = 0xFFFFFFFF;
        drawStr("无信号", &di);
        break;
    case PAUSE:
    case PLAYING:
        if (m_status & PLAY_VIDEO){
            drawVideo();
            if (m_bDrawInfo && g_dsCtx->m_tInit.drawfps)
                drawFps();
        }

        if (m_bDrawInfo && g_dsCtx->m_tInit.drawtitle){
            drawTitle();
        }

        if (m_bDrawInfo && g_dsCtx->m_tInit.drawinfo){
            drawTaskInfo();
        }

        if (m_status & PLAY_AUDIO){
            if (m_bDrawInfo && g_dsCtx->m_tInit.drawaudio){
                drawAudio();
            }
        }

        break;
    }

    if (g_dsCtx->m_tInit.drawoutline){
        drawOutline();
    }

    if (g_dsCtx->m_tInit.drawDebugInfo){
        drawDebugInfo();
    }
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
HGeneralGLWidget::HGeneralGLWidget(QWidget* parent)
    : HGLWidget(parent)
{
    type = GENERAL;
    initUI();
    initConnect();
}

HGeneralGLWidget::~HGeneralGLWidget(){

}

void HGeneralGLWidget::initUI(){
    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->setMargin(2);

    m_titlebar = new HTitlebarWidget;
    m_titlebar->hide();
    vbox->addWidget(m_titlebar);

    vbox->addStretch();

    m_toolbar = new HToolbarWidget;
    m_toolbar->hide();
    vbox->addWidget(m_toolbar);

    setLayout(vbox);

    m_numSelector = new HNumSelectWidget(this);
    m_numSelector->setWindowFlags(Qt::Popup);
    m_numSelector->setAttribute(Qt::WA_TranslucentBackground, true);
}

void HGeneralGLWidget::initConnect(){
    QObject::connect( m_titlebar->m_btnFullScreen, SIGNAL(clicked(bool)), this, SLOT(onFullScreen()) );
    QObject::connect( m_titlebar->m_btnExitFullScreen, SIGNAL(clicked(bool)), this, SLOT(onExitFullScreen()) );
    QObject::connect( m_titlebar->m_btnSnapshot, SIGNAL(clicked(bool)), this, SLOT(snapshot()) );
    //QObject::connect( m_titlebar->m_btnStartRecord, SIGNAL(clicked(bool)), this, SLOT(startRecord()) );
    //QObject::connect( m_titlebar->m_btnStopRecord, SIGNAL(clicked(bool)), this, SLOT(stopRecord()) );
#if LAYOUT_TYPE_OUTPUT_AND_MV
    QObject::connect( m_titlebar->m_btnNum, SIGNAL(clicked(bool)), this, SLOT(showNumSelector()) );
    QObject::connect( m_titlebar->m_btnMicphoneOpened, SIGNAL(clicked(bool)), this, SLOT(closeMicphone()) );
    QObject::connect( m_titlebar->m_btnMicphoneClosed, SIGNAL(clicked(bool)), this, SLOT(openMicphone()) );
#endif
    QObject::connect( m_titlebar->m_btnVoice, SIGNAL(clicked(bool)), this, SLOT(onVoice()) );
    QObject::connect( m_titlebar->m_btnMute, SIGNAL(clicked(bool)), this ,SLOT(onMute()) );

    QObject::connect( m_toolbar->m_btnStart, SIGNAL(clicked(bool)), this, SLOT(onStart()) );
    QObject::connect( m_toolbar->m_btnPause, SIGNAL(clicked(bool)), this, SLOT(onPause()) );
    QObject::connect( m_toolbar, SIGNAL(progressChanged(int)), this, SLOT(onProgressChanged(int)) );

    QObject::connect( m_numSelector, SIGNAL(numSelected(int)), this, SLOT(onNumSelected(int)) );
    QObject::connect( m_numSelector, SIGNAL(numCanceled(int)), this, SLOT(onNumCanceled(int)) );
}

void HGeneralGLWidget::updateToolWidgets(){
    DsSrvItem* item = g_dsCtx->getSrvItem(srvid);
    if (!item)
        return;

    QString str = g_dsCtx->m_tInit.title_format;
    char szWndid[5];
    sprintf(szWndid, "%02d", wndid);
    str.replace("%wndid", szWndid);
    str.replace("%title", item->title.c_str());
    str.replace("%src", item->src_addr);
    m_titlebar->m_label->setText(str);

#if LAYOUT_TYPE_OUTPUT_AND_MV
    if (m_status | PLAY_AUDIO){
        if (item->bVoice){
            m_titlebar->m_btnMute->hide();
            m_titlebar->m_btnVoice->show();

            HCombItem* ci = g_dsCtx->getCombItem(srvid);
            if (ci){
                if (!ci->v && ci->a){
                    m_titlebar->m_btnMicphoneClosed->hide();
                    m_titlebar->m_btnMicphoneOpened->show();
                }else{
                    m_titlebar->m_btnMicphoneOpened->hide();
                    m_titlebar->m_btnMicphoneClosed->hide();
                }
            }else{
                m_titlebar->m_btnMicphoneOpened->hide();
                m_titlebar->m_btnMicphoneClosed->show();
            }
        }else{
            m_titlebar->m_btnVoice->hide();
            m_titlebar->m_btnMute->show();
            m_titlebar->m_btnMicphoneClosed->hide();
            m_titlebar->m_btnMicphoneOpened->hide();
        }
    }else{
        m_titlebar->m_btnMute->hide();
        m_titlebar->m_btnVoice->hide();
        m_titlebar->m_btnMicphoneClosed->hide();
        m_titlebar->m_btnMicphoneOpened->hide();
    }
#endif

#if LAYOUT_TYPE_ONLY_MV
    if (m_status | PLAY_AUDIO){
        if (g_dsCtx->playaudio_srvid == srvid){
            m_titlebar->m_btnMute->hide();
            m_titlebar->m_btnVoice->show();
        }else{
            m_titlebar->m_btnVoice->hide();
            m_titlebar->m_btnMute->show();
        }
    }else{
        m_titlebar->m_btnVoice->hide();
        m_titlebar->m_btnMute->hide();
    }
#endif
    if (item->src_type == SRC_TYPE_LMIC){
        m_titlebar->m_btnVoice->hide();
        m_titlebar->m_btnMute->hide();
        m_titlebar->m_btnMicphoneClosed->hide();
        m_titlebar->m_btnMicphoneOpened->hide();
    }

#if LAYOUT_TYPE_ONLY_MV
#else
    m_titlebar->m_btnExitFullScreen->setVisible(m_bFullScreen);
    m_titlebar->m_btnFullScreen->setVisible(!m_bFullScreen);
#endif

    if ((m_status & MAJOR_STATUS_MASK) == PLAYING){
        m_toolbar->m_btnStart->hide();
        m_toolbar->m_btnPause->show();
    }
    if ((m_status & MAJOR_STATUS_MASK) == PAUSE){
        m_toolbar->m_btnStart->show();
        m_toolbar->m_btnPause->hide();
    }

    if (item->src_type == SRC_TYPE_FILE){
        m_toolbar->m_slider->show();
    }else{
        m_toolbar->m_slider->hide();
    }
}

bool HGeneralGLWidget::showToolWidgets(bool bShow){
    HGLWidget::showToolWidgets(bShow);

    if (isResetStatus()){
        m_titlebar->hide();
        m_toolbar->hide();
        return false;
    }

    m_titlebar->setVisible(bShow);
    DsSrvItem* item = g_dsCtx->getSrvItem(srvid);
    if (item && item->src_type == SRC_TYPE_FILE){
        m_toolbar->setVisible(bShow);
    }

    return bShow;
}

void HGeneralGLWidget::onNumSelected(int num){
    g_dsCtx->m_preselect[num-1] = OUTER_SRVID(srvid);
}

void HGeneralGLWidget::onNumCanceled(int num){
    g_dsCtx->m_preselect[num-1] = 0;
}

void HGeneralGLWidget::showNumSelector(){
    for (int i = 0; i < MAX_NUM_ICON; ++i){
        if (g_dsCtx->m_preselect[i] == OUTER_SRVID(srvid)){
            m_numSelector->m_numSelects[i]->hide();
            m_numSelector->m_numCancels[i]->show();
        }else{
            m_numSelector->m_numCancels[i]->hide();
            m_numSelector->m_numSelects[i]->show();
        }
    }
    int w = m_numSelector->width();
    int h = m_numSelector->height();
    m_numSelector->move(x() + (width()-w)/2, y() + (height()-h)/2);
    m_numSelector->show();
}

void HGeneralGLWidget::onProgressChanged(int progress){    
    g_dsCtx->setPlayProgress(srvid, progress);
}

void HGeneralGLWidget::openMicphone(){
    dsnetwork->setMicphone(srvid);
    g_dsCtx->pre_micphone_srvid = srvid;
}

void HGeneralGLWidget::closeMicphone(){
    dsnetwork->setMicphone(0);
    g_dsCtx->pre_micphone_srvid = 0;
}

void HGeneralGLWidget::onVoice(){
#if LAYOUT_TYPE_OUTPUT_AND_MV
        dsnetwork->setVoice(srvid, 0);
#endif

#if LAYOUT_TYPE_ONLY_MV
        g_dsCtx->setPlayaudioSrvid(0);
#endif
}

void HGeneralGLWidget::onMute(){   
#if LAYOUT_TYPE_OUTPUT_AND_MV
    dsnetwork->setVoice(srvid, 1);
#endif

#if LAYOUT_TYPE_ONLY_MV
        g_dsCtx->setPlayaudioSrvid(srvid);
#endif
}

void HGeneralGLWidget::drawSelectNum(){
    DrawInfo di;
    di.top = height()-48;
    di.bottom = height()-1;
    di.left = 1;
    di.right = 48;
    for (int i = 0; i < MAX_NUM_ICON; ++i){
        if (g_dsCtx->m_preselect[i] == OUTER_SRVID(srvid)){
            drawTex(&rcloader->tex_numr[i], &di);
            di.left += 48;
            di.right += 48;
        }
    }
}

void HGeneralGLWidget::drawSound(){
    DrawInfo di;
    Texture *tex = getTexture(HAVE_AUDIO);
    di.right = width() - 1;
    di.top = 1;
    di.left = di.right - 32 + 1;
    di.bottom = di.top + 32 - 1;
    drawTex(tex, &di);
}

void HGeneralGLWidget::drawOutline(){
    DrawInfo di;
    di.left = 0;
    di.top = 0;
    di.right = width() - 1;
    di.bottom = height() - 1;
    if (m_titlebar->isVisible() && !isResetStatus()){
        di.color = g_dsCtx->m_tInit.focus_outlinecolor;
    }else{
        di.color = g_dsCtx->m_tInit.outlinecolor;
    }
    drawRect(&di, 3);
}

void HGeneralGLWidget::paintGL(){
    HGLWidget::paintGL();

    if (!isResetStatus()){
#if LAYOUT_TYPE_OUTPUT_AND_MV
        HCombItem* item = g_dsCtx->getCombItem(srvid);
        if (item && item->a){
            drawSound();
        }
        if (m_bDrawInfo && g_dsCtx->m_tInit.drawnum)
            drawSelectNum();
#endif

#if LAYOUT_TYPE_ONLY_MV
        if (g_dsCtx->playaudio_srvid == srvid)
            drawSound();
#endif
    }
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

HCombGLWidget::HCombGLWidget(QWidget* parent)
    : HGLWidget(parent)
{
    type = COMB;

    m_bMouseMoving = false;
    m_bLockToolbar = false;

    initUI();
    initConnect();
}

HCombGLWidget::~HCombGLWidget(){

}

void HCombGLWidget::initUI(){
    m_targetWdg = new HOperateWidget(this);
    m_targetWdg->hide();
    m_target.pWdg = m_targetWdg;

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->setMargin(2);
    vbox->setSpacing(1);

    m_titlebar = new HCombTitlebarWidget;
    m_titlebar->hide();
    vbox->addWidget(m_titlebar);

    vbox->addStretch();

    m_wdgText = new HAddTextWidget;
    m_wdgText->hide();
    vbox->addWidget(m_wdgText);
    vbox->setAlignment(m_wdgText, Qt::AlignCenter);

    vbox->addStretch();

    m_toolbar = new HCombToolbarWidget;
    m_toolbar->hide();
    vbox->addWidget(m_toolbar);

    setLayout(vbox);

//    m_wdgTrash = new HChangeColorWidget(this);
//    m_wdgTrash->setPixmap(rcloader->get(RC_TRASH_BIG));
//    m_wdgTrash->hide();

    m_wdgExpre = HExpreWidget::instance();
    //m_wdgExpre = new HExpreWidget(this);
    //m_wdgExpre->setWindowFlags(Qt::Popup | Qt::WindowDoesNotAcceptFocus);

    m_wdgEffect = HEffectWidget::instance();
    //m_wdgEffect = new HEffectWidget(this);
    //m_wdgEffect->setWindowFlags(Qt::Popup | Qt::WindowDoesNotAcceptFocus);
}

void HCombGLWidget::initConnect(){
    QObject::connect( g_dsCtx, SIGNAL(combChanged()), this, SLOT(onCombChanged()) );
    QObject::connect( dsnetwork, SIGNAL(overlayChanged()), this, SLOT(onOverlayChanged()) );

    QObject::connect( m_titlebar->m_btnFullScreen, SIGNAL(clicked(bool)), this, SLOT(onFullScreen()) );
    QObject::connect( m_titlebar->m_btnExitFullScreen, SIGNAL(clicked(bool)), this, SLOT(onExitFullScreen()) );
    QObject::connect( m_titlebar->m_btnPinb, SIGNAL(clicked(bool)), this, SLOT(lockTools()) );
    QObject::connect( m_titlebar->m_btnPinr, SIGNAL(clicked(bool)), this, SLOT(unlockTools()) );
    QObject::connect( m_titlebar->m_btnInfob, SIGNAL(clicked(bool)), this, SLOT(enableDrawInfo()) );
    QObject::connect( m_titlebar->m_btnInfor, SIGNAL(clicked(bool)), this, SLOT(disableDrawInfo()) );
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
    QObject::connect( m_toolbar->m_btnEffect, SIGNAL(clicked(bool)), this, SLOT(showEffect()) );

    QObject::connect( m_wdgExpre, SIGNAL(expreSelected(QString&)), this, SLOT(onExpreSelected(QString&)) );
    QObject::connect( m_wdgText, SIGNAL(newTextItem(HTextItem)), this, SLOT(onTextAccepted(HTextItem)) );
    QObject::connect( m_wdgEffect, SIGNAL(effectSelected(HPictureItem)), this, SLOT(onEffectSelected(HPictureItem)) );
}

void HCombGLWidget::updateToolWidgets(){
    DsSrvItem* item = g_dsCtx->getSrvItem(srvid);
    if (item){
        m_titlebar->m_label->setText(item->title.c_str());

        if (!item->spacer){
            m_toolbar->m_btnStart->hide();
            m_toolbar->m_btnPause->hide();
        }else if (item->spacer_activate){
            m_toolbar->m_btnStart->show();
            m_toolbar->m_btnPause->hide();
        }else{
            m_toolbar->m_btnStart->hide();
            m_toolbar->m_btnPause->show();
        }
    }
}

bool HCombGLWidget::showToolWidgets(bool bShow){
    if (m_bLockToolbar)
        return true;

    if(!bShow && m_targetWdg->isVisible())
        return true;

    HGLWidget::showToolWidgets(bShow);

    m_titlebar->setVisible(bShow);
    m_toolbar->setVisible(bShow);
    //m_wdgTrash->setVisible(bShow);

    return bShow;
}

void HCombGLWidget::onTargetChanged(){
    //...
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

HOperateObject HCombGLWidget::getObejctByPos(QPoint pt, HAbstractItem::TYPE type){
    // 优先顺序: TEMPORARY > OVERLAY > SCREEN
    // TEXT > PICTURE
    // SUB_SCREEN > MAIN_SCREEN

    if (type == HAbstractItem::ALL){
        if (m_target.pWdg && m_target.pWdg->isVisible()){
            return m_target.obj;
        }
    }

    if (type == HAbstractItem::ALL || type == HAbstractItem::TEXT){
        for (int i = m_vecTexts.size()-1; i >=0; --i){
            if (m_vecTexts[i].rcDraw.contains(pt)){
                return m_vecTexts[i];
            }
        }
    }

    if (type == HAbstractItem::ALL || type == HAbstractItem::PICTURE){
        for (int i = m_vecPictures.size()-1; i >=0; --i){
            if (m_vecPictures[i].rcDraw.contains(pt)){
                return m_vecPictures[i];
            }
        }
    }

    if (type == HAbstractItem::ALL || type == HAbstractItem::SCREEN){
        for (int i = m_vecScreens.size()-1; i >= 0; --i){
            if (m_vecScreens[i].rcDraw.contains(pt)){
                return m_vecScreens[i];
            }
        }
    }

    return HOperateObject(NULL);
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
    onCancel();

    m_vecScreens.clear();
    for (int i = 0; i < g_dsCtx->m_tComb.itemCnt; ++i){
        if (g_dsCtx->m_tComb.items[i].v){
            HOperateObject target(&g_dsCtx->m_tComb.items[i]);
            target.rcDraw = scaleToDraw(target.pItem->rc);
            m_vecScreens.push_back(target);
        }
    }
}

void HCombGLWidget::onOverlayChanged(){
    onCancel();

    m_vecPictures.clear();
    for (int i = 0; i < g_dsCtx->m_pics.itemCnt; ++i){
        HOperateObject target(&g_dsCtx->m_pics.items[i]);
        target.rcDraw = scaleToDraw(target.pItem->rc);
        m_vecPictures.push_back(target);
    }

    m_vecTexts.clear();
    for (int i = 0; i < g_dsCtx->m_texts.itemCnt; ++i){
        HOperateObject target(&g_dsCtx->m_texts.items[i]);
        target.rcDraw = scaleToDraw(target.pItem->rc);
        m_vecTexts.push_back(target);
    }
}

void HCombGLWidget::onCancel(){
    m_target.cancel();
}

void HCombGLWidget::onUndo(){
    if (m_target.isOperating()){
        m_target.cancel();
        return;
    }

    if (HItemUndo::instance()->undo() <= 0){
        //m_toolbar->m_btnUndo->setDisabled(true);
    }
}

void HCombGLWidget::onTrash(){    
    if (m_target.isNull())
        return;

    if (m_target.isOperating()){
        m_target.cancel();
        return;
    }

    HAbstractItem* item = HItemFactory::createItem(m_target.obj.pItem->type);
    item->clone(m_target.obj.pItem);
    HItemUndo::instance()->save(item);
    item->remove();
}

void HCombGLWidget::onOK(){
    if (m_target.isOperating() && m_target.obj.isModifiable()){
        if (m_target.obj.isExist()){
            HAbstractItem* item = HItemFactory::createItem(m_target.obj.pItem->type);
            item->clone(m_target.obj.pItem);
            HItemUndo::instance()->save(item);

            m_target.obj.pItem->rc = scaleToOrigin(m_target.pWdg->geometry());
            m_target.obj.pItem->modify();
        }else{
            // add don't need to call clone, pItem is newed already
            m_target.obj.pItem->rc = scaleToOrigin(m_target.pWdg->geometry());
            HItemUndo::instance()->save(m_target.obj.pItem);
            m_target.obj.pItem->add();
        }

        m_target.pWdg->hide();
        m_target.obj.pItem = NULL; // we call HItemUndo::instance()->save, don't delete it.
    }
}

void HCombGLWidget::onZoomIn(){
    showOperateTarget();

    if (!m_target.isNull()){
        QRect rc = m_target.pWdg->geometry();
        if (m_target.obj.pItem->type == HAbstractItem::TEXT){
            HTextItem* item = (HTextItem*)(m_target.obj.pItem);
            if (item->font_size < 96){
                item->font_size += 2;
                updateOperateTarget();
            }
        }else{
            QPoint ptCenter = rc.center();
            int w = rc.width() * 1.1;
            int h = rc.height() * 1.1;
            if (w > width())
                w = width();
            if (h > height())
                h = height();
            QRect rcDst = adjustPos(QRect(ptCenter.x() - w/2, ptCenter.y() - h/2, w, h ));
            m_target.pWdg->setGeometry(rcDst);
        }
    }
}

void HCombGLWidget::onZoomOut(){
    showOperateTarget();

    if (!m_target.isNull()){
        QRect rc = m_target.pWdg->geometry();
        if (m_target.obj.pItem->type == HAbstractItem::TEXT){
            HTextItem* item = (HTextItem*)(m_target.obj.pItem);
            if (item->font_size > 8){
                item->font_size -= 2;
                updateOperateTarget();
            }
        }else{
            QPoint ptCenter = rc.center();
            if (rc.width() <= 32 && rc.height() <= 32)
                return;
            int w = rc.width() * 0.9;
            int h = rc.height() * 0.9;
            if (w < 32)
                w = 32;
            if (h < 32)
                h = 32;
            QRect rcDst = adjustPos(QRect(ptCenter.x() - w/2, ptCenter.y() - h/2, w, h ));
            m_target.pWdg->setGeometry(rcDst);
        }
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
    m_wdgText->setVisible(!m_wdgText->isVisible());
}

void HCombGLWidget::showEffect(){
    int w = m_wdgEffect->width();
    int h = m_wdgEffect->height();
    QPoint ptLeftTop = m_toolbar->mapToGlobal(QPoint(0,0));
    m_wdgEffect->setWindowFlags(Qt::Popup | Qt::WindowDoesNotAcceptFocus);
    m_wdgEffect->move(x() + (width() - w)/2, ptLeftTop.y() - h);
    m_wdgEffect->show();
}

void HCombGLWidget::onTextAccepted(HTextItem item){
    HTextItem* pItem = new HTextItem(item);
    m_target.obj.attachItem(pItem);
    showOperateTarget();
    int w = m_target.pWdg->width();
    int h = m_target.pWdg->height();
    m_target.pWdg->move((width()-w)/2, (height()-h)/2);

    onTargetChanged();
}

void HCombGLWidget::showExpre(){
    int w = m_wdgExpre->width();
    int h = m_wdgExpre->height();
    QPoint ptLeftTop = m_toolbar->mapToGlobal(QPoint(0,0));
    m_wdgExpre->setWindowFlags(Qt::Popup | Qt::WindowDoesNotAcceptFocus);
    m_wdgExpre->move(x() + (width() - w)/2, ptLeftTop.y() - h);
    m_wdgExpre->show();
}

#define EXPRE_MAX_WIDTH     128
#define EXPRE_MAX_HEIGHT    128

#define EXPRE_POLICY_ORIGIN_SIZE    1

void HCombGLWidget::onExpreSelected(QString& filepath){
    HPictureItem* pItem = new HPictureItem;
    strncpy(pItem->src, filepath.toLocal8Bit().constData(), MAXLEN_STR);

    m_target.obj.attachItem(pItem);

    QPixmap pixmap;
    pixmap.load(filepath);
    int w = pixmap.width();
    int h = pixmap.height();
    if (g_dsCtx->m_tInit.expre_policy == EXPRE_POLICY_ORIGIN_SIZE){
        QRect rc = scaleToDraw(QRect(0,0,w,h));
        w = rc.width();
        h = rc.height();
    }else{
        if (w > EXPRE_MAX_WIDTH && h > EXPRE_MAX_HEIGHT){
            w = EXPRE_MAX_WIDTH;
            h = EXPRE_MAX_HEIGHT;
        }
    }
    m_target.pWdg->setGeometry(adjustPos(QRect((width()-w)/2, (height()-h)/2, w, h)));
    m_target.pWdg->setPixmap(pixmap);
    m_target.pWdg->show();

    onTargetChanged();
}

void HCombGLWidget::onEffectSelected(HPictureItem item){
    HPictureItem* pItem = new HPictureItem(item);
    m_target.obj.attachItem(pItem);

    if (pItem->pic_type == HPictureItem::MOSAIC){
        m_target.pWdg->setPixmap(rcloader->get(RC_MOSAIC));
    }else if (pItem->pic_type == HPictureItem::BLUR){
        m_target.pWdg->setPixmap(rcloader->get(RC_BLUR));
    }

    int w = EXPRE_MAX_WIDTH;
    int h = EXPRE_MAX_HEIGHT;
    m_target.pWdg->setGeometry(QRect((width()-w)/2, (height()-h)/2, w, h));
    m_target.pWdg->show();

    onTargetChanged();
}

bool HCombGLWidget::updateOperateTarget(){
    if (m_target.isNull() || !m_target.obj.isModifiable())
        return false;

    if (m_target.obj.pItem->type == HAbstractItem::PICTURE){
        HPictureItem* pItem = (HPictureItem*)m_target.obj.pItem;
        QPixmap pixmap;
        pixmap.load(pItem->src);
        m_target.pWdg->setPixmap(pixmap);
    }if (m_target.obj.pItem->type == HAbstractItem::TEXT){
        HTextItem* pItem = (HTextItem*)m_target.obj.pItem;
        m_target.pWdg->setPixmap(QPixmap());
        QFont font = m_targetWdg->font();
        font.setPixelSize(pItem->font_size * width() / g_dsCtx->m_tComb.width);
        font.setLetterSpacing(QFont::AbsoluteSpacing,0);
        QString str;
        if (pItem->text_type == HTextItem::LABEL){
            str = pItem->text;
            font.setLetterSpacing(QFont::AbsoluteSpacing,4);
        }else if (pItem->text_type == HTextItem::TIME){
            str = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        }else if (pItem->text_type == HTextItem::WATCHER){
            str = "00:00:00:0";
        }else if (pItem->text_type == HTextItem::SUBTITLE){
            str = "字幕";
        }
        m_targetWdg->setPixmap(QPixmap());
        m_targetWdg->setText(str);
        m_targetWdg->setFont(font);
        QPalette pal = m_targetWdg->palette();
        pal.setColor(QPalette::Foreground, pItem->font_color);
        m_targetWdg->setPalette(pal);

        QFontMetrics fm(font);
        int w = fm.width(str)+20;
        int h = fm.height();
        m_target.pWdg->resize(w,h);
    }else if (m_target.obj.pItem->type == HAbstractItem::SCREEN){
        HCombItem* pItem = (HCombItem*)m_target.obj.pItem;
        if (pItem->v){
            //m_target.pWdg->setPixmap(grab(m_target->rcDraw));
            m_target.pWdg->setPixmap(QPixmap::fromImage(grabFramebuffer().copy(m_target.obj.rcDraw)));
        }else{
            m_target.pWdg->setPixmap(QPixmap());
        }
    }

    return true;
}

bool HCombGLWidget::showOperateTarget(){
    if (m_target.isNull() || !m_target.obj.isModifiable())
        return false;

    if (m_target.obj.isExist() && !m_target.pWdg->isVisible()){
        m_target.pWdg->setGeometry(m_target.obj.rcDraw);
    }

    updateOperateTarget();

    m_target.pWdg->show();
}

void HCombGLWidget::drawOutline(){
    DrawInfo di;
    di.left = 0;
    di.top = 0;
    di.right = width() - 1;
    di.bottom = height() - 1;
    di.color = g_dsCtx->m_tInit.outlinecolor;
    drawRect(&di, 3);
}

void HCombGLWidget::drawTaskInfo(){
    DsSrvItem* item = g_dsCtx->getSrvItem(srvid);
    if (item && QGLWidgetImpl::m_pFont){
        DrawInfo di;
        int oldSize = QGLWidgetImpl::m_pFont->FaceSize();
        QStringList sept = item->taskinfo.split("\r\n");
        di.top = 10;
        di.left = 10;
        di.color = g_dsCtx->m_tInit.infcolor;
        if (sept.size() < 8){
            QGLWidgetImpl::m_pFont->FaceSize(32);
        }else{
            QGLWidgetImpl::m_pFont->FaceSize(28);
        }
        for (int i = 0; i < sept.size(); ++i){
            drawStr(sept[i].toLocal8Bit().data(), &di);
            di.top += QGLWidgetImpl::m_pFont->LineHeight() + 10;
        }
        QGLWidgetImpl::m_pFont->FaceSize(oldSize);
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
        if (i < MAX_NUM_ICON)
            drawTex(&rcloader->tex_numr[i], &di);

        // draw comb outline
        di.left = rc.left();
        di.top = rc.top();
        di.right = rc.right();
        di.bottom = rc.bottom();
        di.color = g_dsCtx->m_tInit.outlinecolor;
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
        di.color = g_dsCtx->m_tInit.outlinecolor;
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
        di.color = g_dsCtx->m_tInit.outlinecolor;
        drawRect(&di);
    }
}

void HCombGLWidget::paintGL(){
    HGLWidget::paintGL();

    if (g_dsCtx->m_tInit.drawinfo){
#if LAYOUT_TYPE_ONLY_OUTPUT
        if (m_bDrawInfo)
            drawTaskInfo();
#else
        drawTaskInfo();
#endif
    }

    if (m_bDrawInfo){
        drawScreenInfo();
        drawPictureInfo();
        drawTextInfo();
    }

    // draw focused target outline
    if (!m_target.isNull() && m_target.obj.isModifiable() && !m_target.isOperating()){
        if (m_toolbar->isVisible()){
            DrawInfo di;
            QRect rc = m_target.obj.rcDraw;
            di.left = rc.x();
            di.top = rc.y();
            di.right = rc.right();
            di.bottom = rc.bottom();
            di.color = g_dsCtx->m_tInit.focus_outlinecolor;
            drawRect(&di, 3);
        }
    }
}

void HCombGLWidget::resizeEvent(QResizeEvent *e){
    //m_wdgTrash->setGeometry(width()-128-1, height()/2-64, 128, 128);

    onCombChanged();
    onOverlayChanged();

    HGLWidget::resizeEvent(e);
}

void HCombGLWidget::mousePressEvent(QMouseEvent* e){
    HGLWidget::mousePressEvent(e);

    m_target.obj = getObejctByPos(e->pos());
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
        showOperateTarget();
        if (!m_target.isNull()){
            m_location = getLocation(e->pos(), m_target.pWdg->geometry());
        }
    }

    if (!m_target.isNull()){
        QRect rc = m_target.pWdg->geometry();

        if ((m_location & Center) || m_target.obj.pItem->type == HAbstractItem::TEXT){
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

        m_target.pWdg->setGeometry(rc);
    }

//    if (m_wdgTrash->isVisible()){
//        if (m_wdgTrash->geometry().contains(e->pos())){
//            m_wdgTrash->changeColor(QColor(255, 0, 0, 128));
//        }else{
//            m_wdgTrash->changeColor(Qt::transparent);
//        }
//    }
}

void HCombGLWidget::mouseReleaseEvent(QMouseEvent *e){
    HGLWidget::mouseReleaseEvent(e);

    if (m_bMouseMoving){
        m_bMouseMoving = false;
        // moveEnd

//        if (m_wdgTrash->isVisible() && m_wdgTrash->geometry().contains(e->pos())){
//            onTrash();
//        }

        if (!m_toolbar->isVisible()){
            onOK();
        }
    }
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
