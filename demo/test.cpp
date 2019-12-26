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



std::atomic<uint64_t> g_all_count;
std::atomic<uint64_t> g_nallSetCnt;
std::atomic<uint64_t> g_nallGetCnt;
std::atomic<uint64_t> g_setErr;
std::atomic<uint64_t> g_getErr;
std::atomic<uint64_t> g_getEmpty;



MWRedisClient* redisClient = nullptr;

/*
**官网：https://github.com/acl-dev/acl/
**ACL-REDIS示例：https://github.com/acl-dev/acl/tree/master/lib_acl_cpp/samples/redis
**官方示例:https://zsxxsz.iteye.com/blog/2184744
**依赖库：acllib.a、aclredis.a
**demo author：Peter Hu
*/


void redis_set(std::string prex, uint64_t _loop)
{
    std::string error;
    uint64_t i;
    while ((i = g_nallSetCnt.fetch_add(1)) < _loop)
    {
        //存数据
        std::string pos = prex + "peter_" + std::to_string(i);
        std::string v1 =  pos;
        int ret = redisClient->set(pos, v1, error);
        if (0 != ret)
        {
            fprintf(stderr, "failed to set, ret:%d, error:%s\n", ret, error.c_str());
            ++g_setErr;
        }
    }
}

void redis_get(std::string prex, uint64_t _loop)
{
    std::string error;
    uint64_t i;
    while ((i = g_nallGetCnt.fetch_add(1))< _loop)
    {
        //取数据
        std::string pos = prex + "peter_" + std::to_string(i);
        std::string v1;

        int ret = redisClient->get(pos, v1, error);
        if (0 != ret)
        {
            fprintf(stderr, "failed to get, ret:%d, error:%s\n", ret, error.c_str());
            ++g_getErr;
        }
        else {
            if (v1.empty()) {
                ++g_getEmpty;
            }
        }
    }
}

int do_test(int argc, char* argv[])
{
    if (argc != 6)
    {
        printf("Usage:%s poolisze workthreadnum setallnum getallnum nretrylimit\n", argv[0]);
        printf("Usage:%s 8 8 0 100000 15\n", argv[0]);
        return -1;
    }

    int poolsize = atoi(argv[1]);
    int workthreadnum = atoi(argv[2]);
    uint64_t setAllnum = static_cast<uint64_t>(atol(argv[3]));
    uint64_t getAllnum = static_cast<uint64_t>(atol(argv[4]));
    int nretrylimit = atoi(argv[5]);
    redisClient = MWRedisClientFactory::New();

    do 
    {
        //////////////////////////////////////////////////////////////////////////
        //初始化redis连接池
//        std::string redis_cluster = "192.169.0.163:6379"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
//        std::string redis_pwd = "nking"; //redis访问密码，默认为空

//      std::string redis_cluster = "192.169.0.60:7000"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
        std::string redis_cluster = "192.169.7.135:7001,192.169.7.135:7002,192.169.7.136:7001,192.169.7.136:7002,192.169.7.137:7001,192.169.7.137:7002"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380

        std::string redis_pwd = ""; //redis访问密码，默认为空
        int nConnTimeout = 30; //连接超时时间，默认为30秒,最大值60
        int nRWTimeout = 30;  //读写超时时间，默认为30秒,最大值60
        int nRetryLimit = nretrylimit; //设置重定向的最大阀值，若重定向次数超过此阀值则报错,最大值30
        int nRetryInterval = 1; //当某个连接池结点出问题，设置探测该连接结点是否恢复的时间间隔(秒)，当该值为 0 时，则不检测
        int nRetrySleep = 500; //当重定向次数 >= 2 时每次再重定向此函数设置休息的时间(毫秒)，默认为500毫秒
        int nPoolSize = poolsize; // 最大线程数，默认为4，REDIS最大支持16线程

        int ret = redisClient->initRedis(redis_cluster, redis_pwd,
            nConnTimeout, nRWTimeout,
            nRetryLimit, nRetryInterval,
            nRetrySleep, nPoolSize, true, false);
        if (0 != ret)
        {
            fprintf(stderr, "failed to init redis, ret:%d\n", ret);
            break;
        }

        printf("starting...\n");

        //启动测试线程
        std::vector<std::thread> thrs;
        int nThreadCnt = workthreadnum;

        if (setAllnum > 0)
        {
            for (int i = 0; i < nThreadCnt; i++)
            {
                thrs.push_back(std::thread(redis_set, "", setAllnum));
            }
        }

        if (getAllnum > 0)
        {
            for (int i = 0; i < nThreadCnt; i++)
            {
                thrs.push_back(std::thread(redis_get, "", getAllnum));
            }
        }

        //计算实时速率
        g_all_count.store(0);
        g_nallSetCnt.store(0);
        g_nallGetCnt.store(0);
        g_getErr.store(0);
        g_setErr.store(0);
        g_getEmpty.store(0);

        uint64_t last_nallSetCnt = 0;
        uint64_t last_nallGetCnt = 0;
        while ((setAllnum >= g_nallSetCnt && setAllnum > 0) || (getAllnum >= g_nallGetCnt && getAllnum>0)) {
        //while (1) {
        printf("speed: set %lu (1s) seterr num:%lu, get %lu (1s) geterr num:%lu g_getempty num:%lu \n", 
            g_nallSetCnt - last_nallSetCnt, g_setErr - 0, g_nallGetCnt - last_nallGetCnt, g_getErr - 0, g_getEmpty - 0);
            last_nallSetCnt = g_nallSetCnt;
            last_nallGetCnt = g_nallGetCnt;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        printf("poolisze:%d workthreadnum:%d setallnum:%lu getallnum:%lu nretrylimit:%d\n", poolsize, workthreadnum, setAllnum, getAllnum, nretrylimit);
        printf("finished\n");


        //等待线程平滑退出
        for (auto item = thrs.begin(); item != thrs.end(); item++)
        {
            if (item->joinable())
                item->join();
        }

    } while (0);

    MWRedisClientFactory::Destroy(redisClient);
    getchar();
	return 0;
}


int test_set2getdb(int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("Usage:%s poolisze workthreadnum nretrylimit\n", argv[0]);
        printf("Usage:%s 8 8 15\n", argv[0]);
        return -1;
    }

    int poolsize = atoi(argv[1]);
    //int workthreadnum = atoi(argv[2]);
    int nretrylimit = atoi(argv[3]);
    redisClient = MWRedisClientFactory::New();


    do
    {
        //////////////////////////////////////////////////////////////////////////
        //初始化redis连接池
        //        std::string redis_cluster = "192.169.0.163:6379"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
        //        std::string redis_pwd = "nking"; //redis访问密码，默认为空

        //        std::string redis_cluster = "192.169.0.60:7000"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
        std::string redis_cluster = "192.169.7.135:7001,192.169.7.135:7002,192.169.7.136:7001,192.169.7.136:7002,192.169.7.137:7001,192.169.7.137:7002"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
        std::string redis_pwd = ""; //redis访问密码，默认为空
        int nConnTimeout = 3; //连接超时时间，默认为30秒,最大值60
        int nRWTimeout = 3;  //读写超时时间，默认为30秒,最大值60
        int nRetryLimit = nretrylimit; //设置重定向的最大阀值，若重定向次数超过此阀值则报错,最大值30
        int nRetryInterval = 1; //当某个连接池结点出问题，设置探测该连接结点是否恢复的时间间隔(秒)，当该值为 0 时，则不检测
        int nRetrySleep = 500; //当重定向次数 >= 2 时每次再重定向此函数设置休息的时间(毫秒)，默认为500毫秒
        int nPoolSize = poolsize; // 最大线程数，默认为4，REDIS最大支持16线程

        int ret = redisClient->initRedis(redis_cluster, redis_pwd,
            nConnTimeout, nRWTimeout,
            nRetryLimit, nRetryInterval,
            nRetrySleep, nPoolSize, true, false);
        if (0 != ret)
        {
            fprintf(stderr, "failed to init redis, ret:%d\n", ret);
            break;
        }

        printf("starting...\n");

        std::string value;
        std::string error;
        ret = redisClient->get("h1", value, error);
        if (0 != ret){
            printf("ret:%d, error:%s\n", ret, error.c_str());
        }
        printf("ret:%d\n", ret);


        getchar();
    } while (0);


    MWRedisClientFactory::Destroy(redisClient);
    return 0;
}

