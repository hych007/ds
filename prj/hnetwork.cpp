#include "hnetwork.h"
#include "hdsctx.h"

const char* url_post_combinfo = "http://localhost/transcoder/index.php?controller=channels&action=Dragsave";
const char* url_query_overlay = "http://localhost/transcoder/index.php?controller=logo&action=allinfo";
const char* url_add_overlay = "http://localhost/transcoder/index.php?controller=logo&action=logoadd";
const char* url_remove_overlay = "http://localhost/transcoder/index.php?controller=logo&action=ddelete";
const char* url_modify_overlay = "http://localhost/transcoder/index.php?controller=logo&action=editpt";
const char* url_micphone = "http://localhost/transcoder/index.php?controller=channels&action=voiceover";
const char* dir_trans = "/var/www/transcoder/";

HNetwork* HNetwork::s_pNetwork = NULL;

HNetwork::HNetwork() : QObject()
{
    m_nam_post_screeninfo = new QNetworkAccessManager(this);

    m_nam_add_overlay = new QNetworkAccessManager(this);
    QObject::connect( m_nam_add_overlay, SIGNAL(finished(QNetworkReply*)), this, SLOT(queryOverlayInfo()) );

    m_nam_remove_overlay = new QNetworkAccessManager(this);
    QObject::connect( m_nam_remove_overlay, SIGNAL(finished(QNetworkReply*)), this, SLOT(queryOverlayInfo()) );

    m_nam_modify_overlay = new QNetworkAccessManager(this);
    QObject::connect( m_nam_modify_overlay, SIGNAL(finished(QNetworkReply*)), this, SLOT(queryOverlayInfo()) );

    m_nam_query_overlay = new QNetworkAccessManager(this);
    QObject::connect( m_nam_query_overlay, SIGNAL(finished(QNetworkReply*)), this, SLOT(onQueryOverlayReply(QNetworkReply*)) );

    m_nam_micphone = new QNetworkAccessManager(this);
    //QObject::connect( m_nam_micphone, SIGNAL(finished(QNetworkReply*)), this, SLOT(onQueryMicphone(QNetworkReply*)) );
}

HNetwork* HNetwork::instance(){
    if (s_pNetwork == NULL){
        s_pNetwork = new HNetwork;
    }
    return s_pNetwork;
}

void HNetwork::exitInstance(){
    if (s_pNetwork){
        delete s_pNetwork;
        s_pNetwork = NULL;
    }
}

void HNetwork::addItem(HAbstractItem* item){
    if (item->type == HAbstractItem::SCREEN){

    }else if (item->type == HAbstractItem::PICTURE){
        addPicture(*(HPictureItem*)item);
    }else if (item->type == HAbstractItem::TEXT){
        addText(*(HTextItem*)item);
    }
}

void HNetwork::removeItem(HAbstractItem* item){
    if (item->type == HAbstractItem::SCREEN){
        DsScreenInfo si = g_dsCtx->m_tComb;
        if (si.items[item->id].srvid != 0){
            si.items[item->id].srvid = 0;
            si.items[item->id].a = false;
            postScreenInfo(si);
        }
    }else if (item->type == HAbstractItem::PICTURE){
        removePicture(*(HPictureItem*)item);
    }else if (item->type == HAbstractItem::TEXT){
        removeText(*(HTextItem*)item);
    }
}

void HNetwork::modifyItem(HAbstractItem* item){
    if (item->type == HAbstractItem::SCREEN){
        DsScreenInfo si = g_dsCtx->m_tComb;
        si.items[item->id].rc = item->rc;
        HNetwork::instance()->postScreenInfo(si);
    }else if (item->type == HAbstractItem::PICTURE){
        modifyPicture(*(HPictureItem*)item);
    }else if (item->type == HAbstractItem::TEXT){
        modifyText(*(HTextItem*)item);
    }
}

void HNetwork::queryItem(HAbstractItem* item){

}

void HNetwork::postScreenInfo(DsScreenInfo& si){
    QJsonArray arr;
    for (int i = 0; i < si.itemCnt; ++i){
        HScreenItem& item = si.items[i];
        if (!item.v)
            continue;
        QJsonObject obj;
        obj.insert("x", item.rc.x());
        obj.insert("y", item.rc.y());
        obj.insert("w", item.rc.width());
        obj.insert("h", item.rc.height());
        obj.insert("v", item.srvid);
        obj.insert("a", item.a ? 1 : 0);
        arr.append(obj);
    }
    QJsonDocument doc;
    doc.setArray(arr);
    QByteArray bytes = doc.toJson();
    qDebug(bytes.constData());

    m_nam_post_screeninfo->post(QNetworkRequest(QUrl(url_post_combinfo)), bytes);
}

void HNetwork::queryOverlayInfo(){
    m_nam_query_overlay->get(QNetworkRequest(QUrl(url_query_overlay)));
}

#include "hdsctx.h"
void HNetwork::onQueryOverlayReply(QNetworkReply* reply){
    QByteArray bytes = reply->readAll();
    qDebug(bytes.constData());

    QJsonParseError err;
    QJsonDocument dom = QJsonDocument::fromJson(bytes, &err);
    qDebug("err=%d,offset=%d",err.error, err.offset);
    QJsonObject obj_root;
    obj_root = dom.object();
    if (obj_root.contains("picture")){
        QJsonArray arr_picture = obj_root.value("picture").toArray();
        m_vecPictures.clear();
        for (int i = 0; i < arr_picture.size(); ++i){
            QJsonObject obj_picture = arr_picture[i].toObject();
            HPictureItem item;
            if (obj_picture.contains("id")){
                item.id = obj_picture.value("id").toString().toInt();
            }

            if (obj_picture.contains("x_pos") && obj_picture.contains("y_pos") &&
                obj_picture.contains("width") && obj_picture.contains("height")){
                int x = obj_picture.value("x_pos").toInt();
                int y = obj_picture.value("y_pos").toInt();
                int w = obj_picture.value("width").toInt();
                int h = obj_picture.value("height").toInt();
                item.rc.setRect(x,y,w,h);
            }

            if (obj_picture.contains("path")){
                item.src = dir_trans;
                item.src += obj_picture.value("path").toString();
            }

            m_vecPictures.push_back(item);
            qDebug("id=%d,x=%d,y=%d,w=%d,h=%d,src=%s", item.id, item.rc.x(), item.rc.y(), item.rc.width(), item.rc.height(),
                   item.src.toLocal8Bit().constData());
        }
    }

    if (obj_root.contains("text")){
        QJsonArray arr_text = obj_root.value("text").toArray();
        m_vecTexts.clear();
        for (int i = 0; i < arr_text.size(); ++i){
            QJsonObject obj_text = arr_text[i].toObject();
            HTextItem item;
            if (obj_text.contains("id")){
                item.id = obj_text.value("id").toString().toInt();
            }

            if (obj_text.contains("x_pos") && obj_text.contains("y_pos") &&
                obj_text.contains("width") && obj_text.contains("height")){
                int x = obj_text.value("x_pos").toInt();
                int y = obj_text.value("y_pos").toInt();
                int w = obj_text.value("width").toInt();
                int h = obj_text.value("height").toInt();
                item.rc.setRect(x,y,w,h);
            }

            if (obj_text.contains("content")){
                item.text = obj_text.value("content").toString();
            }

            if (obj_text.contains("font_size")){
                item.font_size = obj_text.value("font_size").toInt();
            }

            if (obj_text.contains("font_color")){
                item.font_color = obj_text.value("font_color").toString().toInt(NULL, 16);
            }

            QFont font;
            font.setPointSize(item.font_size*0.8);
            font.setLetterSpacing(QFont::AbsoluteSpacing, 2);
            QFontMetrics fm(font);
            int h = fm.height();
            int w = 256;
            if (item.text.contains("__%%TIMER%%__")){
                item.text_type = HTextItem::TIME;
                w = fm.width("2017-09-10 12:34:56");
            }else if (item.text.contains("__%%WATCHER%%__")){
                item.text_type = HTextItem::WATCHER;
                w = fm.width("00:00:00:0");
            }else if (item.text.contains("__%%subtitle_index%%__")){
                item.text_type = HTextItem::SUBTITLE;
                w = 360;
            }else{
                item.text_type = HTextItem::LABEL;
                w = fm.width(item.text);
            }
            int x = item.rc.x();
            int y = g_dsCtx->m_tComb.height - item.rc.y() - h;//y_pos is from bottom
            item.rc.setRect(x,y,w,h);

            m_vecTexts.push_back(item);
            qDebug("id=%d,x=%d,y=%d,w=%d,h=%d,content=%s,font_size=%d,font_color=0x%x", item.id, item.rc.x(), item.rc.y(), item.rc.width(), item.rc.height(),
                   item.text.toLocal8Bit().constData(),item.font_size, item.font_color);
        }
    }

    emit overlayChanged();

    reply->deleteLater();
}

void HNetwork::addPicture(HPictureItem &item){
    QString src = item.src.right(item.src.length() - strlen(dir_trans));
    QJsonObject obj;
    obj.insert("src", src);
    obj.insert("x", item.rc.x());
    obj.insert("y", item.rc.y());
    obj.insert("w", item.rc.width());
    obj.insert("h", item.rc.height());
    QJsonArray arr;
    arr.append(obj);
    QJsonObject obj_pic;
    obj_pic.insert("picture", arr);
    QJsonDocument dom;
    dom.setObject(obj_pic);
    QByteArray bytes = dom.toJson();
    qDebug(bytes.constData());
    m_nam_add_overlay->post(QNetworkRequest(QUrl(url_add_overlay)), bytes);
}

void HNetwork::addText(HTextItem& item){
    QJsonObject obj;
    obj.insert("content", item.text);
    obj.insert("x", item.rc.x());
    obj.insert("y", g_dsCtx->m_tComb.height - item.rc.bottom());
    obj.insert("font_size", item.font_size);
    char color[32];
    sprintf(color, "0x%x", item.font_color);
    obj.insert("font_color", color);
    QJsonArray arr;
    arr.append(obj);
    QJsonObject obj_pic;
    obj_pic.insert("text", arr);
    QJsonDocument dom;
    dom.setObject(obj_pic);
    QByteArray bytes = dom.toJson();
    qDebug(bytes.constData());
    m_nam_add_overlay->post(QNetworkRequest(QUrl(url_add_overlay)), bytes);
}

void HNetwork::modifyPicture(HPictureItem& item){
    QJsonObject obj;
    obj.insert("id", item.id);
    obj.insert("x", item.rc.x());
    obj.insert("y", item.rc.y());
    obj.insert("w", item.rc.width());
    obj.insert("h", item.rc.height());
    QJsonArray arr;
    arr.append(obj);
    QJsonObject obj_pic;
    obj_pic.insert("picture", arr);
    QJsonDocument dom;
    dom.setObject(obj_pic);
    QByteArray bytes = dom.toJson();
    qDebug(bytes.constData());
    m_nam_modify_overlay->post(QNetworkRequest(QUrl(url_modify_overlay)), bytes);
}

void HNetwork::modifyText(HTextItem& item){
    QJsonObject obj;
    obj.insert("id", item.id);
    obj.insert("x", item.rc.x());
    obj.insert("y", g_dsCtx->m_tComb.height - item.rc.bottom());
    obj.insert("font_size", item.font_size);
    char color[32];
    sprintf(color, "0x%x", item.font_color);
    obj.insert("font_color", color);
    QJsonArray arr;
    arr.append(obj);
    QJsonObject obj_pic;
    obj_pic.insert("text", arr);
    QJsonDocument dom;
    dom.setObject(obj_pic);
    QByteArray bytes = dom.toJson();
    qDebug(bytes.constData());
    m_nam_add_overlay->post(QNetworkRequest(QUrl(url_modify_overlay)), bytes);
}

void HNetwork::removePicture(HPictureItem& item){
    QJsonObject obj;
    obj.insert("id", item.id);
    obj.insert("type", "picture");
    QJsonArray arr;
    arr.append(obj);
    QJsonDocument dom;
    dom.setArray(arr);
    QByteArray bytes = dom.toJson();
    qDebug(bytes.constData());
    m_nam_remove_overlay->post(QNetworkRequest(QUrl(url_remove_overlay)), bytes);
}

void HNetwork::removeText(HTextItem& item){
    QJsonObject obj;
    obj.insert("id", item.id);
    obj.insert("type", "text");
    QJsonArray arr;
    arr.append(obj);
    QJsonDocument dom;
    dom.setArray(arr);
    QByteArray bytes = dom.toJson();
    qDebug(bytes.constData());
    m_nam_remove_overlay->post(QNetworkRequest(QUrl(url_remove_overlay)), bytes);
}

//void HNetwork::onQueryMicphone(QNetworkReply* reply){
//    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
//    if (doc.isObject()){
//        QJsonObject obj = doc.object();
//        if (obj.contains("id")){
//            int micphone = obj.value("id").toInt();
//        }
//    }

//    reply->deleteLater();
//}

//void HNetwork::queryMicphone(){
//    m_nam_micphone->get(QNetworkRequest(QUrl(url_micphone)));
//}

void HNetwork::setMicphone(int srvid){
    QString json = QString::asprintf("{\"id\":%d}", srvid);
    qDebug(json.toUtf8().constData());
    m_nam_micphone->post(QNetworkRequest(QUrl(url_micphone)), json.toUtf8());
}

