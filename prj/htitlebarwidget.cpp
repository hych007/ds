#include "htitlebarwidget.h"
#include "hrcloader.h"
#include "hdsctx.h"

HTitlebarWidget::HTitlebarWidget(QWidget *parent) : HWidget(parent){
    initUI();
    initConnect();
}

void HTitlebarWidget::initUI(){
    setBgFg(this, MASK_BG);

    QHBoxLayout* hbox = genHBoxLayout();
    hbox->setDirection(QBoxLayout::RightToLeft);
    hbox->setSpacing(g_dsCtx->m_tInit.spacing);

    QSize sz(g_dsCtx->m_tInit.titlebar_height,g_dsCtx->m_tInit.titlebar_height);

    m_btnFullScreen = genPushButton(sz, rcloader->get(RC_FULLSCREEN));
    hbox->addWidget(m_btnFullScreen);
#if LAYOUT_TYPE_MULTI_INPUT
    m_btnFullScreen->hide();
#endif

    m_btnExitFullScreen = genPushButton(sz, rcloader->get(RC_EXIT_FULLSCREEN));
    m_btnExitFullScreen->hide();
    hbox->addWidget(m_btnExitFullScreen);

    m_btnSnapshot = genPushButton(sz, rcloader->get(RC_SNAPSHOT));
    m_btnSnapshot->hide();
    hbox->addWidget(m_btnSnapshot);

    m_btnVoice = genPushButton(sz, rcloader->get(RC_VOICE));
    m_btnVoice->hide();
    hbox->addWidget(m_btnVoice);

    m_btnMute = genPushButton(sz, rcloader->get(RC_MUTE));
    m_btnMute->hide();
    hbox->addWidget(m_btnMute);

#if LAYOUT_TYPE_OUTPUT_AND_INPUT
    m_btnMicphoneOpened = genPushButton(sz, rcloader->get(RC_MICPHONE));
    m_btnMicphoneOpened->hide();
    hbox->addWidget(m_btnMicphoneOpened);

    m_btnMicphoneClosed = genPushButton(sz, rcloader->get(RC_MICPHONE_GRAY));
    m_btnMicphoneClosed->hide();
    hbox->addWidget(m_btnMicphoneClosed);

    m_btnNum = genPushButton(sz, rcloader->get(RC_NUM));
    hbox->addWidget(m_btnNum);
#endif

    m_btnPtz = genPushButton(sz, rcloader->get(RC_PTZ));
    m_btnPtz->hide();
    hbox->addWidget(m_btnPtz);

    hbox->addStretch();

    m_label = new QLabel;
    QFont font = m_label->font();
    font.setPixelSize(g_dsCtx->m_tInit.fontsize);
    m_label->setFont(font);
    hbox->addWidget(m_label);

    setLayout(hbox);
}

void HTitlebarWidget::initConnect(){
    connectButtons(m_btnFullScreen, m_btnExitFullScreen);

#if LAYOUT_TYPE_MULTI_INPUT
    connectButtons(m_btnVoice, m_btnMute);
#endif
}

bool HTitlebarWidget::event(QEvent *e){
    switch (e->type()){
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
        return true;
    default:
        return QWidget::event(e);
    }
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

HCombTitlebarWidget::HCombTitlebarWidget(QWidget *parent) : HWidget(parent){
    initUI();
    initConnect();
}

void HCombTitlebarWidget::initUI(){
    setBgFg(this, MASK_BG);

    QHBoxLayout* hbox = genHBoxLayout();
    hbox->setSpacing(g_dsCtx->m_tInit.spacing);

    m_label = new QLabel;
    hbox->addWidget(m_label);

    hbox->addStretch();

    QSize sz(g_dsCtx->m_tInit.output_titlebar_height,g_dsCtx->m_tInit.output_titlebar_height);

    m_btnPtz = genPushButton(sz, rcloader->get(RC_PTZ));
    m_btnPtz->hide();
    hbox->addWidget(m_btnPtz);

    m_btnSnapshot = genPushButton(sz, rcloader->get(RC_SNAPSHOT));
    m_btnSnapshot->hide();
    hbox->addWidget(m_btnSnapshot);

    m_btnInfob = genPushButton(sz, rcloader->get(RC_INFOB));
    m_btnInfob->hide();
    hbox->addWidget(m_btnInfob);

    m_btnInfor = genPushButton(sz, rcloader->get(RC_INFOR));
    hbox->addWidget(m_btnInfor);

    m_btnPinb = genPushButton(sz, rcloader->get(RC_PINB));
    hbox->addWidget(m_btnPinb);

    m_btnPinr = genPushButton(sz, rcloader->get(RC_PINR));
    m_btnPinr->hide();
    hbox->addWidget(m_btnPinr);

    m_btnVoice = genPushButton(sz, rcloader->get(RC_VOICE));
    hbox->addWidget(m_btnVoice);

    m_btnMute = genPushButton(sz, rcloader->get(RC_MUTE));
    m_btnMute->hide();
    hbox->addWidget(m_btnMute);

    m_btnFullScreen = genPushButton(sz, rcloader->get(RC_FULLSCREEN));
    hbox->addWidget(m_btnFullScreen);

    m_btnExitFullScreen = genPushButton(sz, rcloader->get(RC_EXIT_FULLSCREEN));
    m_btnExitFullScreen->hide();
    hbox->addWidget(m_btnExitFullScreen);

    setLayout(hbox);

#if LAYOUT_TYPE_ONLY_OUTPUT
    m_btnFullScreen->hide();
    m_btnExitFullScreen->show();
#endif
}

void HCombTitlebarWidget::initConnect(){
#if !LAYOUT_TYPE_ONLY_OUTPUT
    connectButtons(m_btnFullScreen, m_btnExitFullScreen);
#endif
    connectButtons(m_btnPinb, m_btnPinr);
    connectButtons(m_btnInfob, m_btnInfor);
    connectButtons(m_btnVoice, m_btnMute);
}

bool HCombTitlebarWidget::event(QEvent *e){
    switch (e->type()){
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
        return true;
    default:
        return QWidget::event(e);
    }
}
