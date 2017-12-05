#include "hmainwidget.h"
#include "hrcloader.h"

HMainWidget::HMainWidget(QWidget *parent) : HWidget(parent){
    m_focusGLWdg = NULL;
    m_bMouseMoving = false;
    m_eOperate = EXCHANGE;

    initUI();
    initConnect();
}

void HMainWidget::initUI(){
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowTitle("Anystreaming Director");
    setFocus();

#if OPERATION_TYPE_MOUSE
    setMouseTracking(true);
#endif

    setBgFg(this, Qt::black, Qt::white);

    g_dsCtx->m_tLayout.width  = QApplication::desktop()->width();
    g_dsCtx->m_tLayout.height = QApplication::desktop()->height();
    setGeometry(0,0,g_dsCtx->m_tLayout.width, g_dsCtx->m_tLayout.height);
    qInfo("screen_w=%d,screen_h=%d", width(), height());

#if LAYOUT_TYPE_ONLY_OUTPUT
    g_dsCtx->m_tLayout.itemCnt = 1;
    g_dsCtx->m_tLayout.items[0].setRect(0,0,g_dsCtx->m_tLayout.width,g_dsCtx->m_tLayout.height);
#endif

#if LAYOUT_TYPE_ONLY_MV
    for (int i = 0; i < MAXNUM_LAYOUT; ++i){
        HGLWidget* wdg = new HGeneralGLWidget(this);
        wdg->wndid = i + 1;
        wdg->setTitleColor(g_dsCtx->m_tInit.titcolor);
        wdg->setOutlineColor(g_dsCtx->m_tInit.outlinecolor);
        m_vecGLWdg.push_back(wdg);
    }
    if (g_dsCtx->m_tInit.row == 0 || g_dsCtx->m_tInit.col == 0){
        g_dsCtx->m_tInit.row = 3;
        g_dsCtx->m_tInit.col = 3;
    }
    setLayout(g_dsCtx->m_tInit.row, g_dsCtx->m_tInit.col);

    m_toolbar = new HStyleToolbar(this);
    m_toolbar->setWindowFlags(Qt::Popup);
    m_toolbar->setAttribute(Qt::WA_TranslucentBackground, true);
    m_toolbar->setGeometry(0, height()-MAIN_TOOBAR_HEIGHT, width(), MAIN_TOOBAR_HEIGHT);
    m_toolbar->hide();
#else
    for (int i = 0; i < g_dsCtx->m_tLayout.itemCnt; ++i){
        HGLWidget* wdg;
        if (i == g_dsCtx->m_tLayout.itemCnt - 1){
            wdg = new HCombGLWidget(this);
            wdg->srvid = 1; // comb srvid = 1
        }else{
            wdg = new HGeneralGLWidget(this);
            wdg->srvid = 0;
        }
        wdg->wndid = i+1;
        wdg->setGeometry(g_dsCtx->m_tLayout.items[i]);
        wdg->setTitleColor(g_dsCtx->m_tInit.titcolor);
        wdg->setOutlineColor(g_dsCtx->m_tInit.outlinecolor);
        m_vecGLWdg.push_back(wdg);
    }
#endif

#if LAYOUT_TYPE_OUTPUT_AND_MV
    m_btnLeftExpand = new QPushButton(this);
    m_btnLeftExpand->setGeometry(width()-ICON_WIDTH-1, height()-ICON_HEIGHT-1, ICON_WIDTH, ICON_HEIGHT);
    m_btnLeftExpand->setIcon(QIcon(HRcLoader::instance()->icon_left_expand));
    m_btnLeftExpand->setIconSize(QSize(ICON_WIDTH, ICON_HEIGHT));
    m_btnLeftExpand->setFlat(true);

    QSize sz(ICON_WIDTH, ICON_HEIGHT);
    m_btnLeftExpand = genPushButton(sz, HRcLoader::instance()->icon_left_expand, this);
    m_btnLeftExpand->setGeometry(width()-ICON_WIDTH-1, height()-ICON_HEIGHT-1, ICON_WIDTH, ICON_HEIGHT);

    m_btnRightFold = genPushButton(sz, HRcLoader::instance()->icon_right_fold, this);
    m_btnRightFold->setGeometry(width()-ICON_WIDTH-1, height()-ICON_HEIGHT-1, ICON_WIDTH, ICON_HEIGHT);
    m_btnRightFold->hide();

    m_toolbar = new HMainToolbar(this);
    m_toolbar->setGeometry(0, height()-ICON_HEIGHT-1, width()-ICON_WIDTH, ICON_HEIGHT);
    setBgFg(m_toolbar, Qt::transparent);
    m_toolbar->hide();
#endif

    m_labelDrag = new QLabel(this);
    m_labelDrag->setStyleSheet("border:3px groove #FF8C00");
    m_labelDrag->hide();

    m_labelRect = new QLabel(this);
    m_labelRect->setStyleSheet("border:3px groove #FF8C00; background-color: rgba(0,0,0,0);");
    m_labelRect->hide();
}

void HMainWidget::initConnect(){
    QObject::connect( g_dsCtx, SIGNAL(actionChanged(int)), this, SLOT(onActionChanged(int)) );
    QObject::connect( g_dsCtx, SIGNAL(requestShow(int)), this, SLOT(onRequestShow(int)) );
    QObject::connect( g_dsCtx, SIGNAL(videoPushed(int,bool)), this, SLOT(onvideoPushed(int,bool)) );
    QObject::connect( g_dsCtx, SIGNAL(audioPushed(int)), this, SLOT(onAudioPushed(int)) );
    QObject::connect( g_dsCtx, SIGNAL(sigStop(int)), this, SLOT(onStop(int)) );
    QObject::connect( g_dsCtx, SIGNAL(quit()), this, SLOT(hide()) );
    QObject::connect( g_dsCtx, SIGNAL(sigProgressNty(int,int)), this, SLOT(onProgressNty(int,int)) );

    for (int i = 0; i < m_vecGLWdg.size(); ++i){
        HGLWidget* wdg = m_vecGLWdg[i];
        QObject::connect( wdg, SIGNAL(fullScreen(bool)), this, SLOT(onFullScreen(bool)) );
        QObject::connect( wdg, SIGNAL(clicked()), this, SLOT(onGLWdgClicked()) );
    }

#if LAYOUT_TYPE_OUTPUT_AND_MV
    QObject::connect( m_btnLeftExpand, SIGNAL(clicked(bool)), this, SLOT(expand()) );
    QObject::connect( m_btnRightFold, SIGNAL(clicked(bool)), this, SLOT(fold()) );
#endif

#if LAYOUT_TYPE_ONLY_MV
    QObject::connect( m_toolbar, SIGNAL(styleChanged(int,int)), this, SLOT(setLayout(int,int)) );
    QObject::connect( m_toolbar->m_btnMerge, SIGNAL(clicked(bool)), this, SLOT(onMerge()) );
#endif

    QObject::connect( &timer_repaint, SIGNAL(timeout()), this, SLOT(onTimerRepaint()) );
    if (g_dsCtx->m_tInit.display_mode == DISPLAY_MODE_TIMER){
        timer_repaint.start(1000 / g_dsCtx->m_tInit.fps);
    }
}

HGLWidget* HMainWidget::getGLWdgByWndid(int wndid){
    for (int i = 0; i < m_vecGLWdg.size(); ++i){
        if (m_vecGLWdg[i]->wndid == wndid)
            return m_vecGLWdg[i];
    }

    return NULL;
}

HGLWidget* HMainWidget::allocGLWdgForsrvid(int srvid){
    HGLWidget* wdg = getGLWdgBysrvid(srvid);
    if (wdg)
        return wdg;

    int min_wndid = 10000;
    for (int i = 0; i < m_vecGLWdg.size(); ++i){
        if (m_vecGLWdg[i]->isResetStatus() && m_vecGLWdg[i]->isVisible() && m_vecGLWdg[i]->wndid < min_wndid){
            wdg = m_vecGLWdg[i];
            min_wndid = wdg->wndid;
        }
    }

    if (wdg){
        wdg->srvid = srvid;
        qDebug("srvid:%d ==> wndid:%d", wdg->srvid, wdg->wndid);
    }

    return wdg;
}

HGLWidget* HMainWidget::getGLWdgBysrvid(int srvid){
    for (int i = 0; i < m_vecGLWdg.size(); ++i){
        HGLWidget* wdg = m_vecGLWdg[i];
        if (wdg->srvid == srvid)
            return wdg;
    }

    return NULL;
}

HGLWidget* HMainWidget::getGLWdgByPos(QPoint pt){
    return getGLWdgByPos(pt.x(), pt.y());
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
        case Qt::Key_Escape:
        {
            g_dsCtx->setAction(0);
            event->accept();
        }
            break;
        defualt:
            break;
    }

    QWidget::keyPressEvent(event);
}

void HMainWidget::mousePressEvent(QMouseEvent *event){
    m_ptMousePressed = event->pos();
}

void HMainWidget::mouseMoveEvent(QMouseEvent *event){
    if (event->buttons() == Qt::NoButton){
#if LAYOUT_TYPE_ONLY_OUTPUT
        // LAYOUT_TYPE_ONLY_OUTPUT no toolbar
#else
        if (event->y() > height() - MAIN_TOOBAR_HEIGHT && !m_toolbar->isVisible()){
                m_toolbar->show();
        }
#endif
    }else{
        HGLWidget* wdg = getGLWdgByPos(event->x(), event->y());
        if (!wdg)
            return;

        if (!m_bMouseMoving){            
            m_bMouseMoving = true;
            // move begin

            m_dragSrcWdg = wdg;

            if (m_eOperate == EXCHANGE){
                if (wdg->srvid <= 1)
                    return;

                if (!wdg->isResetStatus()){
                    m_labelDrag->setPixmap( QPixmap::fromImage(wdg->grabFramebuffer()).scaled(DRAG_WIDTH, DRAG_HEIGHT) );
                    m_labelDrag->show();
                    setCursor(QCursor(Qt::DragMoveCursor));
                }
            }else if (m_eOperate == MERGE){
                m_labelRect->show();
            }
        }

        if (m_labelRect->isVisible()){
            QRect rc;
            rc.setTopLeft(QPoint(qMin(m_ptMousePressed.x(), event->x()), qMin(m_ptMousePressed.y(), event->y())));
            rc.setBottomRight(QPoint(qMax(m_ptMousePressed.x(), event->x()), qMax(m_ptMousePressed.y(), event->y())));
            m_labelRect->setGeometry(rc);
        }

        if (m_labelDrag->isVisible()){
            m_labelDrag->setGeometry(event->x()-DRAG_WIDTH/2, event->y()-DRAG_HEIGHT, DRAG_WIDTH,DRAG_HEIGHT);
        }
    }
}

void HMainWidget::mouseReleaseEvent(QMouseEvent *event){
    if (m_bMouseMoving){
        m_bMouseMoving = false;
        setCursor(QCursor(Qt::ArrowCursor));
        // move end
        if (m_eOperate == EXCHANGE){
            if (!m_labelDrag->isVisible())
                return;
            m_labelDrag->hide();
            HGLWidget* wdg = getGLWdgByPos(event->x(), event->y());
            if (wdg && m_dragSrcWdg != wdg){
                if (wdg->srvid == 1){
                    // changeScreenSource
                    HOperateTarget* target = ((HCombGLWidget*)wdg)->getItemByPos(QPoint(event->x()-wdg->x(), event->y()-wdg->y()), HAbstractItem::SCREEN);
                    if (target)
                        changeScreenSource(target->pItem->id, m_dragSrcWdg->srvid);
                    else
                        addScreenSource(m_dragSrcWdg->srvid);
                }else{
                    // exchange wndid for order
                    std::swap(m_dragSrcWdg->wndid, wdg->wndid);
                    // exchange pos
                    QRect rcSrc = m_dragSrcWdg->geometry();
                    QRect rcDst = wdg->geometry();
                    m_dragSrcWdg->setGeometry(rcDst);
                    wdg->setGeometry(rcSrc);
                    qDebug("exchange srvid:%d ==> wndid:%d && srvid:%d ==> wndid:%d",
                           m_dragSrcWdg->srvid, m_dragSrcWdg->wndid,
                           wdg->srvid, wdg->wndid);
                }
            }
        }else if (m_eOperate == MERGE){
            if (!m_labelRect->isVisible())
                return;
            m_labelRect->hide();
            m_eOperate = EXCHANGE;
            HGLWidget* ltWdg = getGLWdgByPos(m_labelRect->geometry().topLeft());
            HGLWidget* rbWdg = getGLWdgByPos(m_labelRect->geometry().bottomRight());
            if (ltWdg && rbWdg && ltWdg != rbWdg){
                mergeGLWdg(ltWdg->wndid, rbWdg->wndid);
            }
        }
    }else{
        // click release
    }
}

void HMainWidget::onTimerRepaint(){
    for (int i = 0; i < m_vecGLWdg.size(); ++i){
        HGLWidget* wdg = m_vecGLWdg[i];
        DsSrvItem* item = g_dsCtx->getSrvItem(wdg->srvid);
        if (item){
            if (!wdg->isResetStatus() ){
                if (g_dsCtx->pop_video(wdg->srvid) == 0)
                    wdg->repaint();
            }
        }   
    }
}

void HMainWidget::onActionChanged(int action){
    if (action == 0){
        hide();
    }else if (action == 1){
        showFullScreen();

        // when hide,status change but not repaint
        for (int i = 0; i < m_vecGLWdg.size(); ++i){
            m_vecGLWdg[i]->update();
        }
    }
}

void HMainWidget::onRequestShow(int srvid){
    HGLWidget* wdg = allocGLWdgForsrvid(srvid);
    if (wdg == NULL)
        return;

    g_dsCtx->onRequestShowSucceed(srvid, wdg->size());
}

void HMainWidget::onvideoPushed(int srvid, bool bFirstFrame){
    HGLWidget* wdg = getGLWdgBysrvid(srvid);
    if (wdg == NULL){
        onRequestShow(srvid);
        return;
    }

    bool bRepainter = false;
    if (g_dsCtx->m_tInit.display_mode == DISPLAY_MODE_REALTIME){
        g_dsCtx->pop_video(srvid);
        bRepainter = true;
    }

    wdg->setStatus(PLAYING | wdg->status(MINOR_STATUS_MASK) | PLAY_VIDEO, bRepainter);
}

void HMainWidget::onAudioPushed(int srvid){
    HGLWidget* wdg = getGLWdgBysrvid(srvid);
    if (wdg == NULL){
        return;
    }

    // audio not repaint
    wdg->setStatus(PLAYING | wdg->status(MINOR_STATUS_MASK) | PLAY_AUDIO, false);
}

void HMainWidget::onStop(int srvid){
    HGLWidget* wdg = getGLWdgBysrvid(srvid);
    if (wdg == NULL)
        return;

    wdg->onStop();
}

void HMainWidget::onProgressNty(int srvid, int progress){
    HGLWidget* wdg = getGLWdgBysrvid(srvid);
    if (wdg == NULL)
        return;

    ((HGeneralGLWidget*)wdg)->setProgress(progress);
}

#if LAYOUT_TYPE_OUTPUT_AND_MV
void HMainWidget::expand(){
    m_btnLeftExpand->hide();
    m_btnRightFold->show();
    m_toolbar->show();
}

void HMainWidget::fold(){
    m_btnLeftExpand->show();
    m_btnRightFold->hide();
    m_toolbar->hide();
}
#endif

void HMainWidget::onFullScreen(bool  bFullScreen){
#if LAYOUT_TYPE_ONLY_OUTPUT
    if (!bFullScreen){
        g_dsCtx->setAction(0);
    }
#else
    HGLWidget* pSender = (HGLWidget*)sender();

    if (bFullScreen){
        m_rcSavedGeometry = pSender->geometry();
        pSender->setWindowFlags(Qt::Window);
        pSender->showFullScreen();

        DsSrvItem* pItem = g_dsCtx->getSrvItem(pSender->srvid);
        if (pItem && g_dsCtx->m_tInit.fps != pItem->framerate){
            timer_repaint.stop();
            timer_repaint.start(1000/pItem->framerate);
        }
    }else{
        pSender->setWindowFlags(Qt::SubWindow);
        pSender->setGeometry(m_rcSavedGeometry);
        pSender->showNormal();

        DsSrvItem* pItem = g_dsCtx->getSrvItem(pSender->srvid);
        if (pItem && g_dsCtx->m_tInit.fps != pItem->framerate){
            timer_repaint.stop();
            timer_repaint.start(1000/g_dsCtx->m_tInit.fps);
        }
    }
#endif
}

void HMainWidget::onGLWdgClicked(){
    HGLWidget* pSender = (HGLWidget*)sender();

    if (m_focusGLWdg == pSender){
        if (!m_focusGLWdg->showToolWidgets(false))
            m_focusGLWdg = NULL;
    }else{
        if (m_focusGLWdg){
            m_focusGLWdg->showToolWidgets(false);
        }
        if (pSender->showToolWidgets(true))
            m_focusGLWdg = pSender;
    }
}

void HMainWidget::changeScreenSource(int index, int srvid){
    DsScreenInfo si = g_dsCtx->m_tComb;
    if (si.items[index].srvid != srvid){
        si.items[index].srvid = srvid;
        if (srvid == 0){
            si.items[index].a = false;
        }
        HNetwork::instance()->postScreenInfo(si);
    }
}

void HMainWidget::addScreenSource(int srvid){
    DsScreenInfo old = g_dsCtx->m_tComb;
    DsScreenInfo si;
    si.items[0].srvid = srvid;
    si.items[0].v = true;
    si.items[0].a = true;
    si.items[0].rc.setRect(0,0,old.width,old.height);
    for (int i = 0; i < old.itemCnt; ++i){
        si.items[i+1] = old.items[i];
    }
    si.itemCnt = old.itemCnt + 1;
    HNetwork::instance()->postScreenInfo(si);
}

void HMainWidget::onMerge(){
    if (m_eOperate != MERGE){
        m_eOperate = MERGE;
        setCursor(QCursor(Qt::CrossCursor));
    }
    else
        m_eOperate = EXCHANGE;
}

void HMainWidget::setLayout(int row, int col){
    qInfo("setLayout %d*%d", row, col);
    m_layout.init(row, col);
    updateGLWdgsByLayout();
}

void HMainWidget::updateGLWdgsByLayout(){
    int w = width();
    int h = height();
    int col = m_layout.col;
    int row = m_layout.row;
    int cell_w = w / col >> 2 << 2;
    int cell_h = h / row >> 2 << 2;
    int margin_x = (w - cell_w*col) / 2;
    int margin_y = (h - cell_h*row) / 2;
    qDebug("sw=%d, sh=%d, cell_w=%d, cell_h=%d", w, h, cell_w, cell_h);

    for (int i = 0; i < MAXNUM_LAYOUT; ++i){
        HGLWidget* wdg = m_vecGLWdg[i];
        HLayoutCell cell;
        if (m_layout.getLayoutCell(wdg->wndid, cell)){
            wdg->setGeometry(margin_x + (cell.c1-1)*cell_w, margin_y + (cell.r1-1)*cell_h, cell.getColspan()*cell_w, cell.getRowspan()*cell_h);
            wdg->show();
            wdg->update();
        }else{
            wdg->hide();
        }
    }
}

void HMainWidget::mergeGLWdg(int lt, int rb){
    qDebug("merge:%d ~ %d", lt, rb);
    HLayoutCell cell = m_layout.merge(lt, rb);
    qDebug("%d~%d %d~%d", cell.r1, cell.r2, cell.c1, cell.c2);
    // select first to merge, other reset.
    int select_srvid = 0;
    for (int r = cell.r1; r <= cell.r2; ++r)
        for (int c = cell.c1; c <= cell.c2; ++c){
            int id = (r-1)*m_layout.col + c;
            HGLWidget* wdg = getGLWdgByWndid(id);
            if (wdg && !wdg->isResetStatus()){
                if (select_srvid == 0)
                    select_srvid = wdg->srvid;
                wdg->resetStatus();
            }
        }

    if (select_srvid != 0)
        getGLWdgByWndid(lt)->srvid = select_srvid;

    updateGLWdgsByLayout();
}
