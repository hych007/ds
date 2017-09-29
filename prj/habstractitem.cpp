#include "habstractitem.h"
#include "hnetwork.h"
#include "hdsctx.h"

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
HAbstractItem::OPERATE  HAbstractItem::s_operate = HAbstractItem::OPERATE_NONE;
HAbstractItem* HAbstractItem::s_itemUndo = NULL;

HAbstractItem::HAbstractItem()
{
    type = NONE;
    id = -1;
}

HAbstractItem::~HAbstractItem()
{

}

void HAbstractItem::add(){
    s_operate = OPERATE_ADD;
}

void HAbstractItem::remove(){
    s_operate = OPERATE_REMOVE;
}

void HAbstractItem::modify(){
    s_operate = OPERATE_MODIFY;
}

void HAbstractItem::savePreStatus(){
    if (s_itemUndo){
        delete s_itemUndo;
    }
    s_itemUndo = HItemFactory::createItem(type);
    s_itemUndo->clone(this);
}

void HAbstractItem::undo(){
    if (s_itemUndo){
        if (s_operate == OPERATE_ADD){
            //s_itemUndo->remove();
        }
        else if (s_operate == OPERATE_REMOVE)
            s_itemUndo->add();
        else if (s_operate == OPERATE_MODIFY)
            s_itemUndo->modify();
    }
}

void HAbstractItem::onUndo(){
    if (s_itemUndo)
        s_itemUndo->undo();
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
HScreenItem::HScreenItem()
{
    type = SCREEN;

    srvid = 0;
    v = false;
    a = false;

    bMainScreen = false;
}

void HScreenItem::remove(){
    HAbstractItem::remove();

    DsScreenInfo si = g_dsCtx->m_tComb;
    if (si.items[id].srvid != 0){
        si.items[id].srvid = 0;
        si.items[id].a = false;
        HNetwork::instance()->postScreenInfo(si);
    }
}

void HScreenItem::modify(){
    HAbstractItem::modify();

    qDebug("mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm");

    DsScreenInfo si = g_dsCtx->m_tComb;
    si.items[id] = *this;
    HNetwork::instance()->postScreenInfo(g_dsCtx->m_tComb);
}

void HScreenItem::savePreStatus(){
    HAbstractItem::savePreStatus();

    g_dsCtx->m_tCombUndo = g_dsCtx->m_tComb;

    qDebug("ssssssssssssssssssssssssssssssssssssssss");
}

void HScreenItem::undo(){
    qDebug("uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu");
    HNetwork::instance()->postScreenInfo(g_dsCtx->m_tCombUndo);
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
HPictureItem::HPictureItem()
{
    type = PICTURE;
}

void HPictureItem::add(){
    HAbstractItem::add();

    HNetwork::instance()->addPicture(*this);
}

void HPictureItem::remove(){
    HAbstractItem::remove();

    HNetwork::instance()->removePicture(*this);
}

void HPictureItem::modify(){
    HAbstractItem::modify();

    HNetwork::instance()->modifyPicture(*this);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
HTextItem::HTextItem()
{
    type = TEXT;

    text_type = LABEL;
    font_size = 32;
    font_color = 0xFFFFFF;
}

void HTextItem::add(){
    HAbstractItem::add();

    HNetwork::instance()->addText(*this);
}

void HTextItem::remove(){
    HAbstractItem::remove();

    HNetwork::instance()->removeText(*this);
}

void HTextItem::modify(){
    HAbstractItem::modify();

    HNetwork::instance()->modifyText(*this);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


HAbstractItem* HItemFactory::createItem(HAbstractItem::TYPE type){
    switch (type){
    case HAbstractItem::SCREEN:
        return new HScreenItem;
    case HAbstractItem::PICTURE:
        return new HPictureItem;
    case HAbstractItem::TEXT:
        return new HTextItem;
    }

    return NULL;
}
