#ifndef HRCLOADER_H
#define HRCLOADER_H

#include "qglwidgetimpl.h"

#define MAX_NUM_ICON    3

#define FOR_EACH_ICON(F) \
    F(RC_OK, "ok.png") \
    F(RC_CLOSE, "close.png") \
    \
    F(RC_LEFT, "left_expand.png") \
    F(RC_RIGHT, "right_fold.png") \
    \
    F(RC_FULLSCREEN, "fullscreen.png") \
    F(RC_EXIT_FULLSCREEN, "nofullscreen.png") \
    F(RC_INFO, "info.png") \
    F(RC_SNAPSHOT, "snapshot.png") \
    F(RC_RECORD, "record.png") \
    F(RC_RECORDING, "recording.png") \
    F(RC_NUM, "123.png") \
    F(RC_NUM_B1, "b1_64.png") \
    F(RC_NUM_B2, "b2_64.png") \
    F(RC_NUM_B3, "b3_64.png") \
    F(RC_NUM_R1, "r1_64.png") \
    F(RC_NUM_R2, "r2_64.png") \
    F(RC_NUM_R3, "r3_64.png") \
    F(RC_PINB, "pinb.png") \
    F(RC_PINR, "pinr.png") \
    F(RC_VOICE, "voice.png") \
    F(RC_MUTE, "mute.png") \
    F(RC_MICPHONE, "micphone.png") \
    F(RC_MICPHONE_GRAY, "micphone_gray.png") \
    \
    F(RC_START, "start.png") \
    F(RC_PAUSE, "pause.png") \
    F(RC_STOP, "stop.png") \
    \
    F(RC_UNDO, "undo.png") \
    F(RC_TRASH, "trash.png") \
    F(RC_TRASH_BIG, "trash_big.png") \
    F(RC_ZOOMIN, "zoomin.png") \
    F(RC_ZOOMOUT, "zoomout.png") \
    F(RC_SETTING, "setting.png") \
    F(RC_TEXT, "text.png") \
    F(RC_EXPRE, "expre.png") \
    F(RC_EFFECT, "effect.png") \
    \
    F(RC_ADD, "add.png") \
    F(RC_SUB, "sub.png") \
    F(RC_MKDIR, "mkdir.png") \
    F(RC_RMDIR, "rmdir.png") \
    \
    F(RC_MOSAIC, "mosaic.png") \
    F(RC_BLUR, "blur.png") \

#define FOR_EACH_ICON_MV(F) \
    F(RC_STYLE1, "style1.png") \
    F(RC_STYLE2, "style2.png") \
    F(RC_STYLE4, "style4.png") \
    F(RC_STYLE9, "style9.png") \
    F(RC_STYLE16, "style16.png") \
    F(RC_STYLE25, "style25.png") \
    F(RC_STYLE36, "style36.png") \
    F(RC_STYLE49, "style49.png") \
    F(RC_STYLE64, "style64.png") \
    F(RC_MERGE, "merge.png") \
    F(RC_RETURN, "return.png") \

#define FOR_EACH_TEXTURE(F)

#define FOR_EACH_RC(F) \
    FOR_EACH_ICON(F) \
    FOR_EACH_ICON_MV(F) \
    FOR_EACH_TEXTURE(F)

#define ENUM_RCID(rcid, filename) rcid,

enum RCID{
    RC_FIRST = 1,
    FOR_EACH_RC(ENUM_RCID)
    RC_LAST
};

#include "singleton.h"
#include <QMap>
class HRcLoader
{
    DECLARE_SINGLETON(HRcLoader)

public:
    void loadIcon();
    void loadTexture();

    static void bindTexture(Texture* tex, QImage* img);

    QPixmap get(RCID rcid){
        return m_mapIcon[rcid];
    }

public:
    QMap<RCID, QPixmap> m_mapIcon;

    Texture tex_sound;
    Texture tex_numr[MAX_NUM_ICON];
};

#endif // HRCLOADER_H
