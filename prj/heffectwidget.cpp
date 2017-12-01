#include "heffectwidget.h"
#include "hrcloader.h"

HEffectWidget::HEffectWidget(QWidget *parent) : HWidget(parent){
    initUI();
    initConnect();
}

void HEffectWidget::initUI(){
    setFixedSize(200, 98);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setMargin(1);
    hbox->setSpacing(5);

    QSize sz(96,96);

    m_btnMosaic = genPushButton(sz, HRcLoader::instance()->icon_mosaic);
    hbox->addWidget(m_btnMosaic);

    m_btnBlur = genPushButton(sz, HRcLoader::instance()->icon_blur);
    hbox->addWidget(m_btnBlur);

    setLayout(hbox);
}

void HEffectWidget::initConnect(){
    QObject::connect(m_btnMosaic, SIGNAL(clicked(bool)), this, SLOT(hide()) );
    QObject::connect(m_btnBlur, SIGNAL(clicked(bool)), this, SLOT(hide()) );

    QObject::connect(m_btnMosaic, SIGNAL(clicked(bool)), this, SLOT(onMosaic()) );
    QObject::connect(m_btnBlur, SIGNAL(clicked(bool)), this, SLOT(onBlur()) );
}

void HEffectWidget::onMosaic(){
    HPictureItem item;
    item.pic_type = HPictureItem::MOSAIC;
    emit effectSelected(item);
}

void HEffectWidget::onBlur(){
    HPictureItem item;
    item.pic_type = HPictureItem::BLUR;
    emit effectSelected(item);
}
