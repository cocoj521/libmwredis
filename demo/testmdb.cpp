// demo.cpp : 定义控制台应用程序的入口点。
//

#if defined(_WIN32) || defined(_WIN64)
#include "stdafx.h"
#endif

#include <iostream>
#include <cassert>
#include <atomic>
#include <vector>
#include <thread>
#include <sstream>
#include <assert.h>
#include "mredisdb.h"

MRedisDB mRedisDb;

int test_mredisdb(int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("Usage:%s poolisze workthreadnum nretrylimit\n", argv[0]);
        printf("Usage:%s 8 8 15\n", argv[0]);
        return -1;
    }

    int poolsize = atoi(argv[1]);
    int workthreadnum = atoi(argv[2]);
    int nretrylimit = atoi(argv[3]);

    do
    {
        //////////////////////////////////////////////////////////////////////////
        //初始化redis连接池
        //        std::string redis_cluster = "192.169.0.163:6379"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
        //        std::string redis_pwd = "nking"; //redis访问密码，默认为空

        //        std::string redis_cluster = "192.169.0.60:7000"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
        std::string redis_cluster = "192.169.7.135:7001,192.169.7.135:7002,192.169.7.136:7001,192.169.7.136:7002,192.169.7.137:7001,192.169.7.137:7002"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
        std::string redis_pwd = ""; //redis访问密码，默认为空
        int nConnTimeout = 30; //连接超时时间，默认为30秒,最大值60
        int nRWTimeout = 30;  //读写超时时间，默认为30秒,最大值60
        int nRetryLimit = nretrylimit; //设置重定向的最大阀值，若重定向次数超过此阀值则报错,最大值30
        int nRetryInterval = 1; //当某个连接池结点出问题，设置探测该连接结点是否恢复的时间间隔(秒)，当该值为 0 时，则不检测
        int nRetrySleep = 500; //当重定向次数 >= 2 时每次再重定向此函数设置休息的时间(毫秒)，默认为500毫秒
        int nPoolSize = poolsize; // 最大线程数，默认为4，REDIS最大支持16线程

        int ret = mRedisDb.InitRedisDB(redis_cluster, redis_pwd,
            nConnTimeout, nRWTimeout,
            nRetryLimit, nRetryInterval,
            nRetrySleep, nPoolSize, true, false);
        if (0 != ret)
        {
            fprintf(stderr, "failed to init redis, ret:%d\n", ret);
            break;
        }

        printf("starting...\n");


               //TEST:CheckAndSetSuppedSmsState
        {
            std::string rmsMsgid = "person";
            std::string cmpSmsState = "0";
            std::string newSuppSmsState = "2";
            std::string error;
            ret = mRedisDb.CheckAndSetSuppedSmsState(rmsMsgid, cmpSmsState, newSuppSmsState, error);
            if (0 == ret) {
                //无更新
                printf("no update\n");
            }
            else if (1 == ret) {
                //已更新
                printf("updated\n");
            }
            else {
                printf("ret:%d, error:%s", ret, error.c_str());
            }
        }

        //TEST:InsertMtRms
        {
            std::string rmsMsgid = "rmmsgid22345678";
            mt_cache_rms rmsDetail;
            rmsDetail.tmpl_type = "2";
            rmsDetail.tmpl_id = "1111111111";   //模板ID
            rmsDetail.tmpl_type = "2"; //1:动态模板 2:静态模板
            rmsDetail.sms_msgid="1234"; //补发短信的流水号
            rmsDetail.mt_exdata = ""; //扩展字段, Base64编码
            rmsDetail.reserve = "";   //预留字段, Base64编码

            rmsDetail.chk_expflg = "2"; /*业务网关检测超时报告标识 :
                                    0x0：不检测超时
                                    0x1：检测通知报告超时；【暂未使用】
                                    0x2：检测下载报告超时
                                    组合示例：0x3 = (0x1 | 0x2) :
                                    同时检测通知报告和下载报告超时
                                    */

            rmsDetail.supp_privilige = "3";     /*用户补发短信权限 : 通知 0x1, 下载0x2
                                            0x1：通知报告失败补发短信权限；
                                            0x2：下载报告失败补发短信权限；
                                            组合示例：0x3 = (0x1 | 0x2) :
                                            通知报告失败或下载报告失败时补发短信
                                            */

            rmsDetail.supp_sms_state = "0";     
                                            /*已补发短信状态
                                            0：未补发
                                            1：通知报告失败已补发
                                            2：下载报告失败已补发
                                            3：通知报告超时已补发
                                            4：下载报告超时已补发
                                            */
            std::string error;
            ret = mRedisDb.InsertMtRms(rmsMsgid, rmsDetail, error);
            printf("InsertMtRms, ret:%d\n", ret);
        }
    
        //TEST:ExpireMtRms
        {
            std::string error;
            std::string rmsMsgid = "rmmsgid22345678";
            ret = mRedisDb.ExpireMtRms(rmsMsgid, 30*1000, error);
            printf("ExpireMtRms, ret:%d\n", ret);
        }

        getchar();


        //TEST:InsertMtRms
        {
            std::string rmsMsgid = "rmmsgid22345678";
            mt_cache_rms rmsDetail;
            rmsDetail.tmpl_type = "2";
            rmsDetail.tmpl_id = "1111111111";   //模板ID
            rmsDetail.tmpl_type = "2"; //1:动态模板 2:静态模板
            rmsDetail.sms_msgid = "1234"; //补发短信的流水号
            rmsDetail.mt_exdata = ""; //扩展字段, Base64编码
            rmsDetail.reserve = "";   //预留字段, Base64编码

            rmsDetail.chk_expflg = "0"; /*业务网关检测超时报告标识 :
                                        0x0：不检测超时
                                        0x1：检测通知报告超时；【暂未使用】
                                        0x2：检测下载报告超时
                                        组合示例：0x3 = (0x1 | 0x2) :
                                        同时检测通知报告和下载报告超时
                                        */

            rmsDetail.supp_privilige = "3";     /*用户补发短信权限 : 通知 0x1, 下载0x2
                                                0x1：通知报告失败补发短信权限；
                                                0x2：下载报告失败补发短信权限；
                                                组合示例：0x3 = (0x1 | 0x2) :
                                                通知报告失败或下载报告失败时补发短信
                                                */

            rmsDetail.supp_sms_state = "0";
            /*已补发短信状态
            0：未补发
            1：通知报告失败已补发
            2：下载报告失败已补发
            3：通知报告超时已补发
            4：下载报告超时已补发
            */
            std::string error;
            ret = mRedisDb.InsertMtRms(rmsMsgid, rmsDetail, error);
            printf("InsertMtRms, ret:%d\n", ret);
        }



        //TEST:MarkRecivedRdnDlgRpt
        {
            std::string rmsMsgid = "rmmsgid22345678";
            std::string error;
            ret = mRedisDb.MarkRecivedRdnDldRpt(rmsMsgid, error);
            printf("MarkRecivedRdnDlgRpt, ret:%d\n", ret);
        }

        getchar();


        //TEST:InsertRmsExpireQuque
        {
            std::string rmsMsgid = "rmmsgid22345678";
            int timeoutSec = 30;
            std::string error;
            ret = mRedisDb.InsertRmsExpireQueue(rmsMsgid, timeoutSec, error);
            if (0 == ret) {
                printf("InsertRmsExpireQuque ok, rmsMsgid:%s, timeoutSec:%d\n", rmsMsgid.c_str(), timeoutSec);
            }
            else {
                printf("failed to InsertRmsExpireQuque, rmsMsgid:%s, timeoutSec:%d, ret:%d, error:%s\n", rmsMsgid.c_str(), timeoutSec, ret, error.c_str());
            }
        }

        getchar();

        //TEST:ScanRmsExpireQuque
        {
            std::vector<std::string> rmsMsgidList;
            std::string error;
            ret = mRedisDb.ScanRmsExpireQuque(1, rmsMsgidList, error);
            if (0 == ret) {
                printf("ScanRmsExpireQuque ok, size:%u\n", rmsMsgidList.size());
            }
            else {
                printf("failed to ScanRmsExpireQuque, size:%u, ret:%d, error:%s\n", rmsMsgidList.size(), ret, error.c_str());
            }
        }

   
        
        printf("finished\n");
    } while (0);
    getchar();
    return 0;
}

