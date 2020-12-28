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
#include "mw/mwredisslicelist.h"



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
        bool ret = redisClient->set(pos, v1, error);
        if (false == ret)
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

        bool ret = redisClient->get(pos, v1, error);
        if (false == ret)
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
        {
            //////////////////////////////////////////////////////////////////////////
            //初始化redis连接池
            //        std::string redis_cluster = "192.169.0.163:6379"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
            //        std::string redis_pwd = "nking"; //redis访问密码，默认为空

            //      std::string redis_cluster = "192.169.0.60:7000"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
            std::string redis_cluster = "192.169.7.135:7001,192.169.7.135:7002,192.169.7.136:7001,192.169.7.136:7002,192.169.7.137:7001,192.169.7.137:7002"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
            //std::string redis_cluster = "192.169.6.150:6379";


            std::string redis_pwd = ""; //redis访问密码，默认为空
            int nConnTimeout = 30; //连接超时时间，默认为30秒,最大值60
            int nRWTimeout = 30;  //读写超时时间，默认为30秒,最大值60
            int nRetryLimit = nretrylimit; //设置重定向的最大阀值，若重定向次数超过此阀值则报错,最大值30
            int nRetryInterval = 1; //当某个连接池结点出问题，设置探测该连接结点是否恢复的时间间隔(秒)，当该值为 0 时，则不检测
            int nRetrySleep = 500; //当重定向次数 >= 2 时每次再重定向此函数设置休息的时间(毫秒)，默认为500毫秒
            int nPoolSize = poolsize; // 最大线程数，默认为4，REDIS最大支持16线程
            std::string error;
            int ret = redisClient->initRedis(redis_cluster, redis_pwd,
                nConnTimeout, nRWTimeout,
                nRetryLimit, nRetryInterval,
                nRetrySleep, nPoolSize, true, false, error);
            if (0 != ret)
            {
                fprintf(stderr, "failed to init redis, ret:%d, error:%s\n", ret, error.c_str());
                break;
            }
        }
        {
            std::string key = "fep:batchmsg:HJL001:1234567895g";
            std::string filed_name = "refcnt";
            std::string error;
            long long int result = 0;
            int ret = redisClient->hincrby(key, filed_name, 1, &result, error);
            if (ret < 0) {
                printf("failed to hincrby, ret:%d, error:%s\n", ret, error.c_str());
            }
        }

        {
            std::string sets_key = "sets_test2";
            std::string error;
            std::vector<std::string> members = { "hello1", "hello2" };
            int ret = redisClient->sadd(sets_key, members, error);
            if (ret < 0) {
                printf("failed to sadd, ret:%d, error:%s\n", ret, error.c_str());
            }

            std::string buf;
            ret = redisClient->spop(sets_key, buf, error);
            if (ret < 0) { //失败
                printf("failed to spop, ret:%d, error:%s\n", ret, error.c_str());
            }
            else {
                if (!buf.empty()) { //成功spop随机一个元素
                    printf("called spop successfully, buf:%s\n", buf.c_str());
                }
                else {
                    //列队为空了
                }
            }
        }

        {
            std::string index_key = "gl:{batchmsg}:cmdindex";
            std::string error;
            int ret = redisClient->set(index_key, "1", error);
            if (0 != ret) {
                printf("failed to set:%d\n", ret);
            }

            std::string cmd_key = "gl:{batchmsg}:cmdtask";
            ret = redisClient->zadd(cmd_key, "peter", 1593508112, error);
            if (0 != ret) {
                printf("failed to zadd:%d\n", ret);
            }

            cmd_key = "gl2:{batchmsg}:cmdtask2";
            ret = redisClient->zadd(cmd_key, "peter", 1593508112, error);
            if (0 != ret) {
                printf("failed to zadd:%d\n", ret);
            }

            cmd_key = "gl2:{batchmsg}:3434cmdtask9343";
            ret = redisClient->zadd(cmd_key, "peter", 1593508112, error);
            if (0 != ret) {
                printf("failed to zadd:%d\n", ret);
            }
            return 0;
        }

        {
            {
                std::string key = "test:{peter}";
                std::map<std::string, std::string> kv;
                kv["age"] = "30";
                kv["name"] = "peter hu";
                kv["sex"] = "1";
                std::string error;
                int ret = redisClient->hmset(key, kv, error);
                if (0 != ret) {
                    printf("failed to hmset:%d\n", ret);
                }
            }
 
            {
                std::string key = "test:{peter}";
                std::map<std::string, std::string> p;
                std::string error;
                int ret = redisClient->hgetall(key, p, error);
                if (0 != ret) {
                    printf("failed to hgetall:%d\n", ret);
                }
            }

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

    printf("press any key to exit...\n");
    getchar();
	return 0;
}


int test_set2getdb(int argc, char* argv[])
{
    if (argc != 6)
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
        {
            //////////////////////////////////////////////////////////////////////////
            //初始化redis连接池
            //        std::string redis_cluster = "192.169.0.163:6379"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
            //        std::string redis_pwd = "nking"; //redis访问密码，默认为空

            //        std::string redis_cluster = "192.169.0.60:7000"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
            //std::string redis_cluster = "192.169.7.135:7001,192.169.7.135:7002,192.169.7.136:7001,192.169.7.136:7002,192.169.7.137:7001,192.169.7.137:7002"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
            std::string redis_cluster = "192.169.6.150:6379";
            std::string redis_pwd = ""; //redis访问密码，默认为空
            int nConnTimeout = 3; //连接超时时间，默认为30秒,最大值60
            int nRWTimeout = 3;  //读写超时时间，默认为30秒,最大值60
            int nRetryLimit = nretrylimit; //设置重定向的最大阀值，若重定向次数超过此阀值则报错,最大值30
            int nRetryInterval = 1; //当某个连接池结点出问题，设置探测该连接结点是否恢复的时间间隔(秒)，当该值为 0 时，则不检测
            int nRetrySleep = 500; //当重定向次数 >= 2 时每次再重定向此函数设置休息的时间(毫秒)，默认为500毫秒
            int nPoolSize = poolsize; // 最大线程数，默认为4，REDIS最大支持16线程
            std::string error;
            int ret = redisClient->initRedis(redis_cluster, redis_pwd,
                nConnTimeout, nRWTimeout,
                nRetryLimit, nRetryInterval,
                nRetrySleep, nPoolSize, true, false, error);
            if (0 != ret)
            {
                fprintf(stderr, "failed to init redis, ret:%d, error:%s\n", ret, error.c_str());
                break;
            }
        }

        printf("starting...\n");

        {
            std::string error;
            bool bret = redisClient->set("h1", "hello", error);
            if (false == bret){
                printf("set error:%s\n", error.c_str());
            }
            else {
                printf("set h1 ok\n");
            }
        }

        {
            std::string value;
            std::string error;
            bool bret = redisClient->get("h1", value, error);
            if (false == bret){
                printf("get error:%s\n", error.c_str());
            }
            else {
                printf("get value:%s\n", value.c_str());
            }
        }


        getchar();
    } while (0);


    MWRedisClientFactory::Destroy(redisClient);
    return 0;
}

//在接收到的指针上调用delte[]的函数
void MWRedisDeleter(MWRedisClient* e) {
    e->unintRedis();
    MWRedisClientFactory::Destroy(e);
}

