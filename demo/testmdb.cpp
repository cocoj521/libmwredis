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
#include "wbsredisdb.h"
#include "combizredisdb.h"

MRedisDB mRedisDb;



int test_mredisdb(int argc, char* argv[])
{
    if (argc != 5)
    {
        printf("Usage:%s poolisze workthreadnum nretrylimit\n", argv[0]);
        printf("Usage:%s 8 8 15\n", argv[0]);
        return -1;
    }

    int poolsize = atoi(argv[2]);
    int workthreadnum = atoi(argv[3]);
    int nretrylimit = atoi(argv[4]);

    do
    {
        //////////////////////////////////////////////////////////////////////////
        //初始化redis连接池
        //        std::string redis_cluster = "192.169.0.163:6379"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
        //        std::string redis_pwd = "nking"; //redis访问密码，默认为空

        //        std::string redis_cluster = "192.169.0.60:7000"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
        //std::string redis_cluster = "192.169.7.135:7001,192.169.7.135:7002,192.169.7.136:7001,192.169.7.136:7002,192.169.7.137:7001,192.169.7.137:7002"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
        std::string redis_cluster = argv[1];
        std::string redis_pwd = ""; //redis访问密码，默认为空
        int nConnTimeout = 30; //连接超时时间，默认为30秒,最大值60
        int nRWTimeout = 30;  //读写超时时间，默认为30秒,最大值60
        int nRetryLimit = nretrylimit; //设置重定向的最大阀值，若重定向次数超过此阀值则报错,最大值30
        int nRetryInterval = 1; //当某个连接池结点出问题，设置探测该连接结点是否恢复的时间间隔(秒)，当该值为 0 时，则不检测
        int nRetrySleep = 500; //当重定向次数 >= 2 时每次再重定向此函数设置休息的时间(毫秒)，默认为500毫秒
        int nPoolSize = poolsize; // 最大线程数，默认为4，REDIS最大支持16线程
        std::string error;
        int ret = mRedisDb.InitRedisDB(redis_cluster, redis_pwd,
            nConnTimeout, nRWTimeout,
            nRetryLimit, nRetryInterval,
            nRetrySleep, nPoolSize, true, false, error);
        if (0 != ret)
        {
            fprintf(stderr, "failed to init redis, ret:%d\n", ret);
            break;
        }

        printf("starting...\n");

        {
            srand(time(0));
            std::string strFeeKey = mRedisDb.makeUseridFeeKey(11, "HJL001");
            for (int i = 0; i < 2; i++) {

                std::string error;
                std::string mgateid = "8888_index_" + std::to_string(i);
                int64_t inserttm = time(0) + rand()%10;
                int64_t invalidtm = inserttm + 100;
                if (i == 0) {
                    invalidtm = inserttm - 100;
                }
                int ret = mRedisDb.WriteBalance(strFeeKey,
                    mgateid,
                    std::to_string(rand()%100),
                    std::to_string(100000 + rand() % 100),
                    std::to_string(inserttm),
                    std::to_string(invalidtm),
                    1,
                    0,
                    error);
                if (0 != ret)
                {
                    fprintf(stderr, "failed to WriteBalance, ret:%d, error:%s\n", ret, error.c_str());
                    break;
                }
            }

            int64_t amount = 0;
            ret = mRedisDb.ReadBalance("234", 
                static_cast<int64_t>(time(0)),
                amount,
                1,
                0,
                error);
            if (0 != ret)
            {
                fprintf(stderr, "failed to ReadBalance, ret:%d, error:%s\n", ret, error.c_str());
                break;
            }
        }


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




        {
            std::string rmsMsgid = "rmmsgid22345678";
            mt_cache_rms rmsDetail;
            std::string error;
            ret = mRedisDb.QueryMtRms(rmsMsgid, rmsDetail, error);
            printf("QueryMtRms, ret:%d\n", ret);
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

int test_wbsredisdb(int argc, char* argv[])
{
    if (argc < 5)
    {
        printf("Usage:%s cluster poolisze nretrylimit stdoutopen\n", argv[0]);
        printf("Usage:%s cluster poolisze nretrylimit stdoutopen password\n", argv[0]);
        printf("Usage:%s 192.169.7.135:7001,192.169.7.135:7002,192.169.7.136:7001,192.169.7.136:7002,192.169.7.137:7001,192.169.7.137:7002 16 15 0\n", argv[0]);
        printf("Usage:%s \"192.169.7.197:7001;192.169.7.197:7004;192.169.7.198:7002;192.169.7.198:7003;192.169.7.198:7005;192.169.7.197:7006\" 16 15 0 123456\n", argv[0]);
        return -1;
    }

    //std::string redis_cluster = "192.169.7.135:7001;192.169.7.135:7002;192.169.7.136:7001;192.169.7.136:7002;192.169.7.137:7001;192.169.7.137:7002"; //redis集群地址，多地址之间用;分号隔开。如:192.169.1.22:6379;192.169.1.23:6380
    //192.169.7.197:7001; 192.169.7.197:7004; 192.169.7.198:7002; 192.169.7.198:7003; 192.169.7.198:7005; 192.169.7.197:7006; pwd=123456
    std::string redis_cluster = argv[1];
    int poolsize = atoi(argv[2]);

    int nretrylimit = atoi(argv[3]);
    bool bStdoutOpen = atoi(argv[4]) == 0 ? false : true;
    std::string pwd;
    if (argc >= 6) {
        pwd = argv[5];
    }

    WbsRedisDB mWbsRedisDb;

    std::string redis_pwd = pwd; //redis访问密码，默认为空
    int nConnTimeout = 3; //连接超时时间，默认为30秒,最大值60
    int nRWTimeout = 3;  //读写超时时间，默认为30秒,最大值60
    int nRetryLimit = nretrylimit; //设置重定向的最大阀值，若重定向次数超过此阀值则报错,最大值30
    int nRetryInterval = 1; //当某个连接池结点出问题，设置探测该连接结点是否恢复的时间间隔(秒)，当该值为 0 时，则不检测
    int nRetrySleep = 500; //当重定向次数 >= 2 时每次再重定向此函数设置休息的时间(毫秒)，默认为500毫秒
    int nPoolSize = poolsize; // 最大线程数，默认为4，REDIS最大支持16线程

    printf("redis_cluster:%s,\n nConnTimeout:%d, nRWTimeout:%d, poolisze:%d, nretrylimit:%d, nRetryInterval:%d, nRetrySleep:%d\n",
        redis_cluster.c_str(),
        nConnTimeout,
        nRWTimeout,
        poolsize,
        nretrylimit,
        nRetryInterval,
        nRetrySleep);

    size_t start_tm = time(0);
    int ret = mWbsRedisDb.InitRedisDB(redis_cluster, redis_pwd,
        nConnTimeout, nRWTimeout,
        nRetryLimit, nRetryInterval,
        nRetrySleep, nPoolSize, true, bStdoutOpen);
    size_t end_tm = time(0);
    if (0 != ret)
    {
        fprintf(stderr, "failed to init redis, ret:%d, cost:%lu(s)\n", ret, end_tm - start_tm);
        return -1;
    }

    /*
    do
    {
        printf("starting...\n");
      
        std::string key = "peter";
        std::string strToSendNum = "888"; //ARGV[1]
        std::string strPeriodTmSec = "5"; //ARGV[2]
        std::string strPeriodNum = "100"; //ARGV[3]
        std::string strTotalnum = "1000"; //ARGV[4]
        std::string strNowTm = std::to_string(time(NULL)); //ARGV[5]
        std::string strIsFirstInsertedResult; //OUT[1]
        std::string strCurrentBeginTmResult; //OUT[2]
        std::string strNeedAutoRplResult; //OUT[3]
        std::string strAutoRplStatusResult; //OUT[4]
        std::string error;
        
        int ret = mWbsRedisDb.UpdateSms2RmsMtCountToRedis(key,
            strToSendNum, strPeriodTmSec, strPeriodNum, strTotalnum, strNowTm,
            strIsFirstInsertedResult, strCurrentBeginTmResult, strNeedAutoRplResult, strAutoRplStatusResult,
            error);

        assert(ret >= 0);

        printf("finished\n");
    } while (0);
    */
    getchar();
    return 0;
}


int test_combizredisdb(int argc, char* argv[])
{
    //192.169.7.197:7001,192.169.7.197:7004,192.169.7.197:7006 32 3 0
    if (argc < 5)
    {
        printf("Usage:%s cluster poolisze nretrylimit stdoutopen\n", argv[0]);
        printf("Usage:%s cluster poolisze nretrylimit stdoutopen password\n", argv[0]);
        printf("Usage:%s 192.169.7.135:7001,192.169.7.135:7002,192.169.7.136:7001,192.169.7.136:7002,192.169.7.137:7001,192.169.7.137:7002 16 15 0\n", argv[0]);
        printf("Usage:%s \"192.169.7.197:7001;192.169.7.197:7004;192.169.7.198:7002;192.169.7.198:7003;192.169.7.198:7005;192.169.7.197:7006\" 16 15 0 123456\n", argv[0]);
        return -1;
    }

    //std::string redis_cluster = "192.169.7.135:7001;192.169.7.135:7002;192.169.7.136:7001;192.169.7.136:7002;192.169.7.137:7001;192.169.7.137:7002"; //redis集群地址，多地址之间用;分号隔开。如:192.169.1.22:6379;192.169.1.23:6380
    //192.169.7.197:7001; 192.169.7.197:7004; 192.169.7.198:7002; 192.169.7.198:7003; 192.169.7.198:7005; 192.169.7.197:7006; pwd=123456
    std::string redis_cluster = argv[1];
    int poolsize = atoi(argv[2]);

    int nretrylimit = atoi(argv[3]);
    bool bStdoutOpen = atoi(argv[4]) == 0 ? false : true;
    std::string pwd;
    if (argc >= 6) {
        pwd = argv[5];
    }

    ComBizRedisDB mCombizRedisDb;

    std::string redis_pwd = pwd; //redis访问密码，默认为空
    int nConnTimeout = 3; //连接超时时间，默认为30秒,最大值60
    int nRWTimeout = 3;  //读写超时时间，默认为30秒,最大值60
    int nRetryLimit = nretrylimit; //设置重定向的最大阀值，若重定向次数超过此阀值则报错,最大值30
    int nRetryInterval = 1; //当某个连接池结点出问题，设置探测该连接结点是否恢复的时间间隔(秒)，当该值为 0 时，则不检测
    int nRetrySleep = 500; //当重定向次数 >= 2 时每次再重定向此函数设置休息的时间(毫秒)，默认为500毫秒
    int nPoolSize = poolsize; // 最大线程数，默认为4，REDIS最大支持16线程

    printf("redis_cluster:%s,\n nConnTimeout:%d, nRWTimeout:%d, poolisze:%d, nretrylimit:%d, nRetryInterval:%d, nRetrySleep:%d\n",
        redis_cluster.c_str(),
        nConnTimeout,
        nRWTimeout,
        poolsize,
        nretrylimit,
        nRetryInterval,
        nRetrySleep);

    size_t start_tm = time(0);
    int ret = mCombizRedisDb.InitRedisDB(redis_cluster, redis_pwd,
        nConnTimeout, nRWTimeout,
        nRetryLimit, nRetryInterval,
        nRetrySleep, nPoolSize, true, bStdoutOpen);
    size_t end_tm = time(0);
    if (0 != ret)
    {
        fprintf(stderr, "failed to init redis, ret:%d, cost:%lu(s)\n", ret, end_tm - start_tm);
        return -1;
    }

    do
    {
        printf("starting...\n");

        /*
        {
        std::string key = "peter";
        std::string strToSendNum = "888"; //ARGV[1]
        std::string strPeriodTmSec = "5"; //ARGV[2]
        std::string strPeriodNum = "100"; //ARGV[3]
        std::string strTotalnum = "1000"; //ARGV[4]
        std::string strNowTm = std::to_string(time(NULL)); //ARGV[5]
        std::string strIsFirstInsertedResult; //OUT[1]
        std::string strCurrentBeginTmResult; //OUT[2]
        std::string strNeedAutoRplResult; //OUT[3]
        std::string strAutoRplStatusResult; //OUT[4]
        std::string error;

        int ret = mCombizRedisDb.CalSms2RmsMetaData(key,
        strToSendNum, strPeriodTmSec, strPeriodNum, strTotalnum, strNowTm,
        strIsFirstInsertedResult, strCurrentBeginTmResult, strNeedAutoRplResult, strAutoRplStatusResult,
        error);
        assert(ret >= 0);

        }*/

        {
            std::string error;
            int ret = mCombizRedisDb.UpdateMsgidListTimout("zname", "-inf", "+inf", 123456789, error);
            assert(ret >= 0);
            printf("UpdateMsgidListTimout, ret:%d\n", ret);
        }

        printf("finished\n");
    } while (0);
    getchar();
    return 0;
}
