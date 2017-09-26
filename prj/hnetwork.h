#ifndef HNETWORK_H
#define HNETWORK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "ds_def.h"

class HNetwork : public QObject
{
    Q_OBJECT
private:
    HNetwork();

public:
    static HNetwork* instance();
    static void exitInstance();

    void addItem(HAbstractItem* item);
    void removeItem(HAbstractItem* item);
    void modifyItem(HAbstractItem* item);
    void queryItem(HAbstractItem* item);

    void postScreenInfo(DsScreenInfo& info);

    void addPicture(HPictureItem& item);
    void addText(HTextItem& item);

    void removePicture(HPictureItem& item);
    void removeText(HTextItem& item);

    void modifyPicture(HPictureItem& item);
    void modifyText(HTextItem& item);

    //void queryMicphone();
    void setMicphone(int srvid);

signals:
    void overlayChanged();

public slots:
    void queryOverlayInfo();
    void onQueryOverlayReply(QNetworkReply* reply);
    //void onQueryMicphone(QNetworkReply*);

public:
    std::vector<HScreenItem> m_vecScreens;
    std::vector<HPictureItem> m_vecPictures;
    std::vector<HTextItem> m_vecTexts;

private:
    static HNetwork* s_pNetwork;
    QNetworkAccessManager* m_nam_post_screeninfo;
    QNetworkAccessManager* m_nam_add_overlay;
    QNetworkAccessManager* m_nam_query_overlay;
    QNetworkAccessManager* m_nam_remove_overlay;
    QNetworkAccessManager* m_nam_modify_overlay;
    QNetworkAccessManager* m_nam_micphone;
};

#endif // HNETWORK_H
