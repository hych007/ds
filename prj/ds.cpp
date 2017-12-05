#include "ds.h"
#include "hdsctx.h"

#define VERSION 7
#define RELEASEINFO "5.7.1 @ 2017/11/17"

DSSHARED_EXPORT int libversion()    { return VERSION; }
DSSHARED_EXPORT int libchar()       {
#if LAYOUT_TYPE_ONLY_OUTPUT
    return OOK_FOURCC('P', 'D', 'S', 'P');
#else
    return OOK_FOURCC('D', 'I', 'R', 'C');
#endif
}
DSSHARED_EXPORT int libtrace(int t) { return t; }

DSSHARED_EXPORT int libinit(const char* xml, void* task, void** ctx){
    qInstallMessageHandler(myLogHandler);
    qInfo("--------libinit version=%d,%s----------------", VERSION, RELEASEINFO);

    if(!xml || !task || !ctx)
        return -1;

    if(g_dsCtx)
    {
        *ctx = g_dsCtx;
        g_dsCtx->ref++;
        int mask  = SERVICE_POSITION_VIDEO_AFDEC;
        if(g_dsCtx->m_tInit.audio)
            mask |= SERVICE_POSITION_AUDIO_AFDEC;
        return mask;
    }

    int err = 0;
    do {
        g_dsCtx = new HDsContext;

//        if (g_dsCtx->parse_init_xml(xml) != 0){
//            //qWarning("parse_init_xml failed");
//            err = -1003;
//            break;
//        }

        task_info_s        * ti = (task_info_s        *)task;
        task_info_detail_s * tid = (task_info_detail_s *)ti->extra;

#if LAYOUT_TYPE_ONLY_OUTPUT
        DsSrvItem* item = g_dsCtx->getSrvItem(1);
        if (item){
            item->ifcb = ti->ifcb;
        }
#endif

        std::string strXmlPath = tid->cur_path;
        APPENDSEPARTOR(strXmlPath)
#if LAYOUT_TYPE_ONLY_MV
        strXmlPath += "director_service_mv.xml";
#elif LAYOUT_TYPE_ONLY_OUTPUT
        strXmlPath += "director_service_out.xml";
#elif LAYOUT_TYPE_OUTPUT_AND_MV
        strXmlPath += "director_service.xml";
#endif
        if(job_check_path(strXmlPath.c_str()) != 0)
        {
            qWarning("not found director_service.xml");
            err = -1004;
            break;
        }
        if (g_dsCtx->parse_layout_xml(strXmlPath.c_str()) != 0){
            qWarning("parse_layout_xml failed");
            err = -1005;
            break;
        }

        std::string img_path = tid->cur_path;
        APPENDSEPARTOR(img_path)
        img_path += "..";
        APPENDSEPARTOR(img_path)
        img_path += "img";
        APPENDSEPARTOR(img_path)
        img_path += "director_service";
        APPENDSEPARTOR(img_path)
        g_dsCtx->img_path = img_path;
        if(job_check_path(img_path.c_str()) == 0)
            g_dsCtx->initImg(img_path);

        std::string ttf_path = tid->cur_path;
        APPENDSEPARTOR(ttf_path)
        ttf_path += "..";
        APPENDSEPARTOR(ttf_path)
        ttf_path += "fonts";
        APPENDSEPARTOR(ttf_path)
        ttf_path += "default.ttf";
        if(job_check_path(ttf_path.c_str()) == 0)
            g_dsCtx->initFont(ttf_path, 24);

        int mask  = SERVICE_POSITION_VIDEO_AFDEC;
        if(g_dsCtx->m_tInit.audio)
            mask |= SERVICE_POSITION_AUDIO_AFDEC;

        *ctx = g_dsCtx;
        qInfo("============================libinit ok==========================");
        return mask;
    }while(0);

    if (g_dsCtx){
        delete g_dsCtx;
        g_dsCtx = NULL;
    }

    return err;
}

DSSHARED_EXPORT int libstop(void* ctx){
    qInfo("---------------libstop----------------------");
    if (!ctx)
        return -1;

    if (g_dsCtx){
        if (--g_dsCtx->ref == 0){
            g_dsCtx->quit();
//            delete g_dsCtx;
//            g_dsCtx = NULL;
        }
    }

    return 0;
}

DSSHARED_EXPORT int liboper(int media_type, int data_type, int opt, void* param, void * ctx){
    if (!ctx)
        return -1;

    switch(media_type){
    case MediaTypeUnknown:
    {
        if (data_type == SERVICE_DATATYPE_DEP){
            switch(opt)
            {
            case SERVICE_OPT_DISPLAY:
                qInfo("SERVICE_OPT_DISPLAY=%d", *(int*)param);
                if(g_dsCtx->init == 0)
                {
                    g_dsCtx->init = 1;
                    g_dsCtx->start_gui_thread();
                }
                g_dsCtx->setAction(*(int*)param);
                break;
            case SERVICE_OPT_TASKSTATUS:
                if (param){
                    std::string * str = (std::string *)param;
                    g_dsCtx->parse_taskinfo_xml(str->c_str());
                }
                break;
            case SERVICE_OPT_TASKSTATUSREQ:
                if (param){
#if LAYOUT_TYPE_ONLY_OUTPUT
                    int srvid = 1;
#else
                    int srvid = *(int*)param;
#endif
                    if (srvid < 1 || srvid > DIRECTOR_MAX_SERVS)
                        return -2;

                    int span = 10000;
                    if (srvid == 1)
                        span = 1000;
                    unsigned long tick = (unsigned long)chsc_gettick();
                    if (tick > g_dsCtx->getSrvItem(srvid)->tick + span){
                        g_dsCtx->req_srvid = srvid;
                        g_dsCtx->getSrvItem(srvid)->tick = tick;
                        return 1;
                    }
                }
                break;
            case SERVICE_OPT_PREVSTOP:
                if(param)
                {
#if LAYOUT_TYPE_ONLY_OUTPUT
                    int srvid = 1;
#else
                    int srvid = *(int*)param;
#endif
                    if (srvid < 1 || srvid > DIRECTOR_MAX_SERVS)
                        return -2;
                    g_dsCtx->stop(srvid);
                }
                break;
            case SERVICE_OPT_SPACERTYPE:
                qDebug("SERVICE_OPT_SPACERTYPE");
                if (*(int *)param > 1){ // backup stream
                    DsSrvItem* pItem = g_dsCtx->getSrvItem(1);
                    if (pItem && pItem->ifcb){
                        pItem->ifcb->onservice_callback(ifservice_callback::e_service_cb_stampcacu, libchar(), 0, 0, 1, NULL);
                    }
                }
                break;
            default:
                if(opt > 0x1000)
                {
#if LAYOUT_TYPE_ONLY_OUTPUT
                    int srvid = 1;
#else
                    int srvid = opt - 0x1000;
#endif
                    if (srvid < 1 || srvid > DIRECTOR_MAX_SERVS)
                        return -2;

                    if (param){
                        const char* title = (const char*)param;
                        if (is_ascii_string(title)){
                            qInfo("srvid=%d ascii=%s strlen=%d", srvid, title, strlen(title));
                            g_dsCtx->setTitle(srvid, title);
                        }else{
                            ANSICODE2UTF8 a2u(title);
                            if (a2u.c_str() == NULL || strlen(a2u.c_str()) == 0){
                                qCritical("title format error!");
                                return -111;
                            }
                            qInfo("srvid=%d utf8=%s strlen=%d", srvid, a2u.c_str(), strlen(a2u.c_str()));
                            g_dsCtx->setTitle(srvid, a2u.c_str());
                        }
                    }
                }
                break;
            }
        }else if (data_type == SERVICE_DATATYPE_CHR){
            if (opt != libchar())
                return -2;

            const director_service_cont * dsc = (const director_service_cont *)param;
#if LAYOUT_TYPE_ONLY_OUTPUT
            int srvid = 1;
#else
            int srvid = dsc->servid;
#endif

            if(srvid < 1 || srvid > DIRECTOR_MAX_SERVS)
                return -2;

            DsSrvItem* item = g_dsCtx->getSrvItem(srvid);

            if(dsc->action == OOK_FOURCC('S', 'V', 'C', 'B')){
                qInfo("srvid=%d OOK_FOURCC('S', 'V', 'C', 'B')", srvid);
                if (item){
                    item->ifcb = (ifservice_callback *)dsc->ptr;
                }
            }else if(dsc->action == OOK_FOURCC('L', 'O', 'U', 'T')){
                qInfo("srvid=%d OOK_FOURCC('L', 'O', 'U', 'T')", srvid);
                g_dsCtx->parse_comb_xml((const char *)dsc->ptr);
            }else if (dsc->action == OOK_FOURCC('S', 'R', 'C', 'L')){
                qInfo("srvid=%d OOK_FOURCC('S', 'R', 'C', 'L')", srvid);
                if (strcmp((const char*)dsc->ptr, "file") == 0){
                    item->src_type = SRC_TYPE_FILE;
                }
            }else if (dsc->action == OOK_FOURCC('P', 'L', 'Y', 'R')){
                int progress = *(int*)dsc->ptr;
                qDebug("OOK_FOURCC('P', 'L', 'Y', 'R') progress=%d", progress);
                emit g_dsCtx->sigProgressNty(srvid, progress);
            }else if (dsc->action == OOK_FOURCC('S', 'M', 'I', 'X')){
                qDebug("srvid=%d OOK_FOURCC('S', 'M', 'I', 'X')", srvid);
                g_dsCtx->parse_audio_xml((const char *)dsc->ptr);
            }

        }
    }
        break;
    case MediaTypeVideo:
    {
        if(data_type != SERVICE_DATATYPE_PIC)
            return -4;

        const av_picture * pic = (const av_picture *)param;
        if (!pic)
            return -6;

#if LAYOUT_TYPE_ONLY_OUTPUT
        int srvid = 1;
#else
        int srvid = 0;
        if(opt == OOK_FOURCC('L', 'M', 'I', 'C')){
            if(!pic->data[0]){
                // 连麦用户离开
                srvid = g_dsCtx->lmicid2srvid(pic->track);
                if (srvid < 0)
                    return -5;
                qDebug("LMIC------STOP-----%d", srvid);
                g_dsCtx->stop(srvid);
                g_dsCtx->freeLmicid(pic->track);
                return 0;
            }

            srvid = g_dsCtx->lmicid2srvid(pic->track);
            if (srvid < 0){
                srvid = g_dsCtx->allocLmicid(pic->track);
                qDebug("LMIC-----------%d=>%d", pic->track, srvid);
            }
        }else{
            srvid = opt;
        }
#endif
        if (srvid < 1 || srvid > DIRECTOR_MAX_SERVS)
            return -5;

        if (g_dsCtx->getSrvItem(srvid)->v_input < 1){
            char c[5] = {0};
            memcpy(c, &pic->fourcc, 4);
            qInfo("pic[%d] type=%s w=%d h=%d", srvid, c, pic->width, pic->height);
        }

        ++g_dsCtx->getSrvItem(srvid)->v_input;
        g_dsCtx->push_video(srvid, pic);
    }
        break;
    case MediaTypeAudio:
    {
        if(data_type != SERVICE_DATATYPE_PCM)
            return -2;

        const av_pcmbuff * pcm = (const av_pcmbuff *)param;
        if (!pcm)
            return -6;

#if LAYOUT_TYPE_ONLY_OUTPUT
        int srvid = 1;
#else
        int srvid = 0;
        if(opt == OOK_FOURCC('L', 'M', 'I', 'C')){
            srvid = g_dsCtx->lmicid2srvid(pcm->track);
        }else{
            srvid = opt;
        }
#endif
        if (srvid < 1 || srvid > DIRECTOR_MAX_SERVS)
            return -5;

        if (g_dsCtx->getSrvItem(srvid)->a_input < 1){
            qInfo("pcm[%d] channel=%d, sample=%d len=%d", srvid, pcm->channels, pcm->samplerate, pcm->pcmlen);
        }

        ++g_dsCtx->getSrvItem(srvid)->a_input;
        g_dsCtx->push_audio(srvid, pcm);
    }
        break;
    default:
        return -9;
    }

    return 0;
}
