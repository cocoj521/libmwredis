//#include <StdAfx.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <atomic>
#include <vector>
#include <thread>
#include <sstream>
#include <iostream>
#include <memory>
#include "redistest.h"


template <class Container>
void split3(const std::string& str, Container& cont,
    const std::string& delim = " ")
{
    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string::npos) {
        cont.push_back(str.substr(previous, current - previous));
        previous = current + delim.length();
        current = str.find(delim, previous);
    }
    cont.push_back(str.substr(previous, current - previous));
}

int doTestRedis(int argc, char* argv[])
{
    if (argc < 5)
    {
        printf("Usage:%s cluster poolisze nretrylimit stdoutopen\n", argv[0]);
        printf("Usage:%s 192.169.7.135:7001,192.169.7.135:7002,192.169.7.136:7001,192.169.7.136:7002,192.169.7.137:7001,192.169.7.137:7002 16 15 0\n", argv[0]);
        return -1;
    }

    //std::string redis_cluster = "192.169.7.135:7001,192.169.7.135:7002,192.169.7.136:7001,192.169.7.136:7002,192.169.7.137:7001,192.169.7.137:7002"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380

    std::string redis_cluster = argv[1];
    int poolsize = atoi(argv[2]);
    int nretrylimit = atoi(argv[3]);
    bool bStdoutOpen = atoi(argv[4])==0?false:true;


    TestRedis tester;
    std::string redis_pwd = ""; //redis访问密码，默认为空
    int nConnTimeout = 30; //连接超时时间，默认为30秒,最大值60
    int nRWTimeout = 30;  //读写超时时间，默认为30秒,最大值60
    int nRetryLimit = nretrylimit; //设置重定向的最大阀值，若重定向次数超过此阀值则报错,最大值30
    int nRetryInterval = 1; //当某个连接池结点出问题，设置探测该连接结点是否恢复的时间间隔(秒)，当该值为 0 时，则不检测
    int nRetrySleep = 500; //当重定向次数 >= 2 时每次再重定向此函数设置休息的时间(毫秒)，默认为500毫秒
    int nPoolSize = poolsize; // 最大线程数，默认为4，REDIS最大支持16线程
    
    printf("redis_cluster:%s, poolisze:%d, nretrylimit:%d\n", 
        redis_cluster.c_str(),
        poolsize,
        nretrylimit);

    int ret = tester.InitRedisDB(redis_cluster, redis_pwd,
        nConnTimeout, nRWTimeout,
        nRetryLimit, nRetryInterval,
        nRetrySleep, nPoolSize, true, bStdoutOpen);
    if (0 != ret)
    {
        fprintf(stderr, "failed to init redis, ret:%d\n", ret);
        return -1;
    }

    
    //print heart
    for (float y = 1.5f; y > -1.5f; y -= 0.1f)
    { 
        for (float x = -1.5f; x < 1.5f; x += 0.05f) 
        {
            float z = x * x + y * y - 1; 
            float f = z * z * z - x * x * y * y * y;
            putchar(f <= 0.0f ? ".:-=+*#%@"[(int)(f * -8.0f)] : ' '); 
        }
        putchar('\n');
    }

    printf("connected redis server successfully\n");

    for (;;)
    {
        printf("\n");
        printf("---------------------------redis test--------------------------\n");
        printf("0:exit\n");
        printf("1:set test\n");
        printf("2:get test\n");
        printf("3:list lpush test\n");
        printf("4:list lpop test\n");
        printf("5:mt test\n");
        printf("6:slice list\n");
        printf("---------------------------------------------------------------\n");
        printf("\n");
        printf("input please:\n");

        bool fshutdown = false;

        char data;
        std::cin >> data;
        switch (data)
        {
        case '1':
        {
            int workingthrnum = 0;
            uint64_t allnum = 0;
            std::string prefix;

            for (;;)
            {
                printf("please input set working thread number which is less than or equal poolsize:\n");
                std::cin >> workingthrnum;
                if (workingthrnum > 0 && workingthrnum <= nPoolSize) 
                {
                    break;
                }
                printf("error! workingthrnum is TOO much, max pool size:%d\n", nPoolSize);
            } 
            
            printf("please input total number:\n");
            std::cin >> allnum;
            printf("please input total set prefix:\n");
            std::cin >> prefix;
            printf("You has chosen workingthrnum:%d, allnum:%lu, prefix:%s\n", workingthrnum, allnum, prefix.c_str());  
            printf("now starting...\n");
            int64_t starttm = time(NULL);
            tester.test_set(prefix, workingthrnum, allnum);
            int64_t endtm = time(NULL);
            printf("workingthrnum:%d, allnum:%lu, cost:%ld(s), finished!\n", workingthrnum, allnum, endtm - starttm);
        }
        break;
        case '2':
        {
            int workingthrnum = 0;
            uint64_t allnum = 0;
            std::string prefix;

            for (;;)
            {
                printf("please input set working thread number which is less than or equal poolsize:\n");
                std::cin >> workingthrnum;
                if (workingthrnum > 0 && workingthrnum <= nPoolSize)
                {
                    break;
                }
                printf("error! workingthrnum is TOO much, max pool size:%d\n", nPoolSize);
            }

            printf("please input total number:\n");
            std::cin >> allnum;
            printf("please input total get prefix:\n");
            std::cin >> prefix;
            printf("You has chosen workingthrnum:%d, allnum:%lu, prefix:%s\n", workingthrnum, allnum, prefix.c_str());
            printf("now starting...\n");
            int64_t starttm = time(NULL);
            tester.test_get(prefix, workingthrnum, allnum);
            int64_t endtm = time(NULL);
            printf("workingthrnum:%d, allnum:%lu, cost:%ld(s), finished!\n", workingthrnum, allnum, endtm - starttm);
        }
        break;
        case '3':
        {
            int workingthrnum = 0;
            uint64_t allnum = 0;
            std::string key;
            
            for (;;)
            {
                printf("please input set working thread number which is less than or equal poolsize:\n");
                std::cin >> workingthrnum;
                if (workingthrnum > 0 && workingthrnum <= nPoolSize)
                {
                    break;
                }
                printf("error! workingthrnum is TOO much, max pool size:%d\n", nPoolSize);
            }

            printf("please input total number:\n");
            std::cin >> allnum;
            printf("please input total list key:\n");
            std::cin >> key;
            printf("You has chosen workingthrnum:%d, allnum:%lu, lpush list key:%s\n", workingthrnum, allnum, key.c_str());
            printf("now starting...\n");
            int64_t starttm = time(NULL);
            tester.test_lpush(key, workingthrnum, allnum);
            int64_t endtm = time(NULL);
            printf("workingthrnum:%d, allnum:%lu, cost:%ld(s), finished!\n", workingthrnum, allnum, endtm - starttm);
        }
        break;
        case '4':
        {
            int workingthrnum = 0;
            uint64_t allnum = 0;
            std::string key;

            for (;;)
            {
                printf("please input set working thread number which is less than or equal poolsize:\n");
                std::cin >> workingthrnum;
                if (workingthrnum > 0 && workingthrnum <= nPoolSize)
                {
                    break;
                }
                printf("error! workingthrnum is TOO much, max pool size:%d\n", nPoolSize);
            }

            printf("please input total number:\n");
            std::cin >> allnum;
            printf("please input total list key:\n");
            std::cin >> key;
            printf("You has chosen workingthrnum:%d, allnum:%lu, lpop list key:%s\n", workingthrnum, allnum, key.c_str());
            printf("now starting...\n");
            int64_t starttm = time(NULL);
            tester.test_lpop(key, workingthrnum, allnum);
            int64_t endtm = time(NULL);
            printf("workingthrnum:%d, allnum:%lu,  cost:%ld(s), finished!\n", workingthrnum, allnum, endtm - starttm);
        }
        break;
        case '5':
        {
            std::string mt_prefix = "mt_peter";
            int push_mt_thrnum = 8;
            uint64_t push_total_mt_num = 1000000;
            std::string exptablename = "expire_list_peter";
            int expiredseconds = 10;
            int scanreqms = 1000;
            int scanthr = 2;
            int batchsize = 1000;
            int scanliveseconds = 60 * 5;

            std::vector<std::string> values;
            do 
            {
                printf("------------------------------------------------------------------------------------------------------------------\n");
                printf("mt_prefix,push_mt_thrnum,push_total_mt_num,exptablename,expiredseconds,scanreqms,scanthr,batchsize,scanliveseconds\n");
                printf("examples:\nmt_peter_prefix,8,1000000,expire_list_peter,10,1000,2,1000,300\n");
                printf("mt_peter_prefix,0,0,expire_list_peter,10,1000,2,1000,300\n");
                printf("mt_peter_prefix,8,1000000,,0,0,0,0,0\n");
                printf("------------------------------------------------------------------------------------------------------------------\n");
                printf("\n");
                printf("please input parameters that use a comma to separate values:\n");
               
                std::string mtdata;
                std::cin >> mtdata;
                
                values.clear();
                split3<std::vector<std::string>>(mtdata, values, ",");
                if (values.size() == 9) 
                {
                    push_mt_thrnum = atoi(values[1].c_str()); 
                    scanthr = atoi(values[6].c_str());
                    if ((push_mt_thrnum + scanthr) > nPoolSize)
                    {
                        printf("***error***! push_mt_thrnum:%d and scanthr:%d is TOO much, which is greater than poolsize:%d\n\n", push_mt_thrnum, scanthr, poolsize);
                        continue;
                    }
                    else {
                        break;
                    }
                }
                printf("input format incorrectly!\n\n");
            } while (1);

            mt_prefix = values[0];
            push_mt_thrnum = atoi(values[1].c_str());
            push_total_mt_num = strtoull(values[2].c_str(), NULL, 10);
            exptablename = values[3];
            expiredseconds = atoi(values[4].c_str());
            scanreqms = atoi(values[5].c_str());
            scanthr = atoi(values[6].c_str());
            batchsize = atoi(values[7].c_str());
            scanliveseconds = atoi(values[8].c_str());

            printf("mt_prefix:%s, push_mt_thrnum:%d, push_total_mt_num:%lu, exptablename:%s, expiredseconds:%d, "
                "scanreqms:%d, scanthr:%d, batchsize:%d, scanliveseconds:%d, starting...\n",
                mt_prefix.c_str(), push_mt_thrnum, push_total_mt_num, exptablename.c_str(),
                expiredseconds, scanreqms, scanthr, batchsize, scanliveseconds);

            int64_t starttm = time(NULL);
            tester.test_mt(mt_prefix, push_mt_thrnum, push_total_mt_num, exptablename, expiredseconds, scanreqms, scanthr, batchsize, scanliveseconds);
            int64_t endtm = time(NULL);
            printf("mt_prefix:%s, push_mt_thrnum:%d, push_total_mt_num:%lu, exptablename:%s, expiredseconds:%d, "
                    "scanreqms:%d, scanthr:%d, batchsize:%d, scanliveseconds:%d, cost:%ld(s), finished!\n", 
                    mt_prefix.c_str(), push_mt_thrnum, push_total_mt_num, exptablename.c_str(),
                    expiredseconds, scanreqms, scanthr, batchsize, scanliveseconds, endtm - starttm);
        }
        break;
        case '6':
        {
            std::string list_topic = "";
            int sharednum = 0;
            int pushthreadnum = 0;
            uint64_t pushtotalnum = 0;
            int popthreadnum = 0;
            int batchsize = 0;
            uint64_t poptotalnum = 0;

            std::vector<std::string> values;
            do
            {
                printf("------------------------------------------------------------------------------------------------------------------\n");
                printf("list_topic,sharednum,pushthreadnum,pushtotalnum,popthreadnum,batchsize,poptotalnum\n");
                printf("examples:\nhi,6,8,1000000,4,100,2000000\n");
                printf("hi,6,1,20,1,3,20\n");
                printf("hi,6,0,20,1,3,20\n");
                printf("hi,6,12,1000000,0,100,0\n");
                printf("hi,6,0,1000000,12,100,1000000\n");
                printf("------------------------------------------------------------------------------------------------------------------\n");
                printf("\n");
                printf("please input parameters that use a comma to separate values:\n");

                std::string inputdata;
                std::getline(std::cin, inputdata);

                printf("inputdata:%s\n", inputdata.c_str());
                values.clear();
                split3<std::vector<std::string>>(inputdata, values, ",");
                if (values.size() == 7)
                {
                    pushthreadnum = atoi(values[2].c_str());
                    popthreadnum = atoi(values[4].c_str());
                    if ((pushthreadnum + popthreadnum) > nPoolSize)
                    {
                        printf("\n\n***error***! pushthreadnum:%d and popthreadnum:%d is TOO much, which is greater than poolsize:%d\n\n", pushthreadnum, popthreadnum, poolsize);
                        continue;
                    }
                    else {
                        printf("\n\nstarting...\n");
                        break;
                    }
                }
                printf("\n\n***error***!input format incorrectly!\n");
            } while (1);

            list_topic = values[0];
            sharednum = atoi(values[1].c_str());
            pushthreadnum = atoi(values[2].c_str());
            pushtotalnum = atol(values[3].c_str());
            popthreadnum = atoi(values[4].c_str());
            batchsize = atoi(values[5].c_str());
            poptotalnum = atol(values[6].c_str());

            printf("shared list topic:%s, sharednum:%d, pushthreadnum:%d, pushtotalnum:%ld, popthreadnum:%d, batchsize:%d, "
                "poptotalnum:%ld starting...\n",
                list_topic.c_str(), sharednum, pushthreadnum, pushtotalnum, popthreadnum, batchsize, poptotalnum);

            int64_t starttm = time(NULL);
            tester.test_slice_list(list_topic, sharednum, pushthreadnum, pushtotalnum, popthreadnum, batchsize, poptotalnum);
            int64_t endtm = time(NULL);

            printf("shared list topic:%s, sharednum:%d, pushthreadnum:%d, pushtotalnum:%ld, popthreadnum:%d, batchsize:%d, "
                "poptotalnum:%ld  cost:%ld(s), finished!\n",
                list_topic.c_str(), sharednum, pushthreadnum, pushtotalnum, popthreadnum, batchsize, poptotalnum,
                endtm - starttm);
        }
        break;
        case '0':
        {
            printf("exited\n");
            fshutdown = true;
        }
        break;
        default:
        break;
        }

        if (fshutdown) {
            break; //exit
        }
    }

    tester.DestoryRedisDB();
    return 0;
}


TestRedis::TestRedis()  { }

TestRedis::~TestRedis() { }

//初始化组件
int TestRedis::InitRedisDB(const std::string& strRedisCluster,
    const std::string& strRedisPwd,
    int  nConnTimeout,
    int  nRWTimeout,
    int  nRetryLimit,
    int  nRetryInterval,
    int  nRetrySleep,
    int  nPoolSize,
    bool bPreset,
    bool bStdoutOpen)
{
    std::shared_ptr<MWRedisClient> redisClient(MWRedisClientFactory::New(), [](MWRedisClient* e) {
        e->unintRedis();
        MWRedisClientFactory::Destroy(e);
    });
    assert(nullptr != redisClient);
    int ret = redisClient->initRedis(strRedisCluster,
        strRedisPwd,
        nConnTimeout,
        nRWTimeout,
        nRetryLimit,
        nRetryInterval,
        nRetrySleep,
        nPoolSize,
        bPreset,
        bStdoutOpen);

    if (0 == ret) {
        m_pRedisClient = redisClient;
    }
    return ret;
}


//反初始化
void TestRedis::DestoryRedisDB()
{
    assert(nullptr != m_pRedisClient);
    if (m_pRedisClient) {
        m_pRedisClient.reset();
        m_pRedisClient = nullptr;
    }
}


int TestRedis::test_set(std::string& prefix, int thrnum, uint64_t allnum)
{
    {
        std::vector<std::string> names = { "msgid", "createtm" };
        std::string item = "333333333";
        std::map<std::string, std::string> kvmap;
        std::string error;
        int ret = m_pRedisClient->hmget(item, names, kvmap, error);
        if (ret < 0) {
            fprintf(stderr, "failed to hmget(key:%s), error:%s\n", item.c_str(), error.c_str());
        }
    }
    std::atomic<int> thrActiveNum;
    thrActiveNum.store(thrnum);
    std::atomic<uint64_t> nallSetCnt;
    nallSetCnt.store(0);
    std::atomic<uint64_t> nallFailedcnt;
    nallFailedcnt.store(0);
    std::vector<std::shared_ptr<std::thread>> thrs;
    for (int index = 0; index < thrnum; index++)
    {
        std::shared_ptr<std::thread> p(new std::thread([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "thread id:" << std::this_thread::get_id() << std::endl;
            uint64_t i;
            std::string error;
            while ((i = nallSetCnt.fetch_add(1)) < allnum)
            {
                //存数据
                std::string pos = prefix + std::to_string(i);
                std::string v1 = pos;
                bool ret = m_pRedisClient->set(pos, v1, error);
                if (false == ret) {
                    fprintf(stderr, "failed to set(key:%s,value:%s), error:%s\n", pos.c_str(), v1.c_str(), error.c_str());
                    nallFailedcnt++;
                }
            }
            thrActiveNum--;
            fprintf(stdout, "thread exited!\n");
        }));
        thrs.push_back(p);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::atomic<uint64_t> last_nallSetCnt;
    last_nallSetCnt.store(0);
    while (thrActiveNum != 0) {
        printf("set speed:%lu/s, total error num:%lu\n",
            nallSetCnt - last_nallSetCnt,
            nallFailedcnt - 0);
        last_nallSetCnt.store(nallSetCnt);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    //等待线程平滑退出
    for (size_t i = 0; i < thrs.size(); i++) {
        if (thrs[i]->joinable())
            thrs[i]->join();
    }
    return 0;
}

int TestRedis::test_get(std::string& prefix, int thrnum, uint64_t allnum)
{
    std::atomic<int> thrActiveNum;
    thrActiveNum.store(thrnum);
    std::atomic<uint64_t> nallGetCnt;
    nallGetCnt.store(0);
    std::atomic<uint64_t> nallFailedcnt;
    nallFailedcnt.store(0);
    std::vector<std::shared_ptr<std::thread>> thrs;
    for (int index = 0; index < thrnum; index++)
    {
        std::shared_ptr<std::thread> p(new std::thread([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "thread id:" << std::this_thread::get_id() << std::endl;
            uint64_t i;
            std::string error;
            while ((i = nallGetCnt.fetch_add(1)) < allnum)
            {
                //取数据
                std::string pos = prefix + std::to_string(i);
                std::string v1;
                bool ret = m_pRedisClient->get(pos, v1, error);
                if (false == ret) {
                    fprintf(stderr, "failed to get, error:%s\n", error.c_str());
                    nallFailedcnt++;
                }
            }
            thrActiveNum--;
            fprintf(stdout, "thread exited!\n");
        }));
        thrs.push_back(p);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::atomic<uint64_t> last_nallGetCnt;
    last_nallGetCnt.store(0);
    while (thrActiveNum != 0) {
        printf("get speed:%lu/s, total error num:%lu\n",
            nallGetCnt - last_nallGetCnt,
            nallFailedcnt - 0);
        last_nallGetCnt.store(nallGetCnt);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    //等待线程平滑退出
    for (size_t i = 0; i < thrs.size(); i++) {
        if (thrs[i]->joinable())
            thrs[i]->join();
    }
    return 0;
}



int TestRedis::test_lpush(std::string& key, int thrnum, uint64_t allnum)
{
    std::atomic<int> thrActiveNum;
    thrActiveNum.store(thrnum);
    std::atomic<uint64_t> nallLPushCnt;
    nallLPushCnt.store(0);
    std::atomic<uint64_t> nallFailedcnt;
    nallFailedcnt.store(0);
    std::vector<std::shared_ptr<std::thread>> thrs;
    for (int index = 0; index < thrnum; index++)
    {
        std::shared_ptr<std::thread> p(new std::thread([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "thread id:" << std::this_thread::get_id() << std::endl;
            uint64_t i;
            std::string error;
            while ((i = nallLPushCnt.fetch_add(1)) < allnum)
            {
                //取数据
                std::vector<std::string> values = { std::to_string(i) };
                int ret = m_pRedisClient->lpush(key.c_str(), values, error);
                if (ret < 0) {
                    fprintf(stderr, "failed to lpush(key:%s), error:%s\n", key.c_str(), error.c_str());
                    nallFailedcnt++;
                }
            }
            thrActiveNum--;
            fprintf(stdout, "thread exited!\n");
        }));
        thrs.push_back(p);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::atomic<uint64_t> last_nallLPushCnt;
    last_nallLPushCnt.store(0);
    while (thrActiveNum != 0) {
        printf("list:%s, lpush speed:%lu/s, total error num:%lu\n",
            key.c_str(),
            nallLPushCnt - last_nallLPushCnt,
            nallFailedcnt - 0);
        last_nallLPushCnt.store(nallLPushCnt);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    //等待线程平滑退出
    for (size_t i = 0; i < thrs.size(); i++) {
        if (thrs[i]->joinable())
            thrs[i]->join();
    }
    return 0;
}

int TestRedis::test_lpop(std::string& key, int thrnum, uint64_t allnum)
{
    std::atomic<int> thrActiveNum;
    thrActiveNum.store(thrnum);
    std::atomic<uint64_t> nallLPushCnt;
    nallLPushCnt.store(0);
    std::atomic<uint64_t> nallFailedcnt;
    nallFailedcnt.store(0);
    std::vector<std::shared_ptr<std::thread>> thrs;
    for (int index = 0; index < thrnum; index++)
    {
        std::shared_ptr<std::thread> p(new std::thread([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "thread id:" << std::this_thread::get_id() << std::endl;
            uint64_t i;
            std::string error;
            while ((i = nallLPushCnt.fetch_add(1)) < allnum)
            {
                //取数据
                std::string value;
                int ret = m_pRedisClient->lpop(key.c_str(), value, error);
                if (ret < 0) {
                    fprintf(stderr, "failed to lpop(key:%s), error:%s\n", key.c_str(), error.c_str());
                    nallFailedcnt++;
                }
            }
            thrActiveNum--;
            fprintf(stdout, "thread exited!\n");
        }));
        thrs.push_back(p);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::atomic<uint64_t> last_nallLPushCnt;
    last_nallLPushCnt.store(0);
    while (thrActiveNum != 0) {
        printf("list:%s, lpop speed:%lu/s, total error num:%lu\n",
            key.c_str(),
            nallLPushCnt - last_nallLPushCnt,
            nallFailedcnt - 0);
        last_nallLPushCnt.store(nallLPushCnt);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    //等待线程平滑退出
    for (size_t i = 0; i < thrs.size(); i++) {
        if (thrs[i]->joinable())
            thrs[i]->join();
    }
    return 0;
}


//push mt thread num, total mt num, expired seconds, scan frequency, scan thread num, scan batch size
int TestRedis::test_mt(std::string mt_prefix, int push_mt_thrnum, uint64_t push_total_mt_num, std::string exptablename, int expiredseconds, int scanreqms, int scanthr, int batchsize, int scanliveseconds)
{
    std::atomic<int> thrActiveNum;
    thrActiveNum.store(push_mt_thrnum + scanthr);

    std::atomic<uint64_t> nallMtFailedcnt;
    nallMtFailedcnt.store(0);
    
    std::atomic<uint64_t> nallPushQFailedcnt;
    nallPushQFailedcnt.store(0);

    std::atomic<uint64_t> nallPopQFailedcnt;
    nallPopQFailedcnt.store(0);

    std::atomic<uint64_t> nallHGetFailedcnt;
    nallHGetFailedcnt.store(0);

    std::atomic<uint64_t> nallMtPushCnt;
    nallMtPushCnt.store(0);

    std::atomic<uint64_t> nallPopCnt;
    nallPopCnt.store(0);

    std::atomic<uint64_t> nallHGetCnt;
    nallHGetCnt.store(0);

    std::atomic<uint64_t> nallScanCnt;
    nallScanCnt.store(0);

    /*
    {
        char szbuf[64] = { 0 };
        szbuf[0] = 2;
        szbuf[1] = '1';
        szbuf[3] = '3';
        szbuf[4] = 4;
        szbuf[62] = 'a';
        szbuf[63] = 63;
        std::string buf(szbuf, 64);
        std::string key = mt_prefix + std::to_string(0);
        int64_t nowtm = time(NULL);
        std::map<std::string, std::string> kvmap = { { "message", buf }, { "createtm", std::to_string(nowtm) } };
        std::string error;
        m_pRedisClient->hmset(key.c_str(), kvmap, error);
        m_pRedisClient->hset(key.c_str(), "message2", buf, error);
       
        std::map<std::string, std::string> kvmap2;
        m_pRedisClient->hgetall(key.c_str(), kvmap2, error);
        
        std::vector<std::string> names3 = { "message", "message2", "createtm" };
        std::map<std::string, std::string> kvmap3;
        m_pRedisClient->hmget(key.c_str(), names3, kvmap3, error);

        std::string buf2;
        m_pRedisClient->hget(key.c_str(), "message2", buf2, error);

        m_pRedisClient->set("name", buf, error);
        std::string buf3;
        m_pRedisClient->get("name", buf3, error);
        {
            if (kvmap["message"] == kvmap2["message"] &&
                kvmap2["message2"] == kvmap3["message2"] &&
                kvmap["message"] == kvmap3["message"] &&
                buf2 == buf &&
                buf3 == buf)
            {
                printf("\n\nhmset buf(size:%ld) is equal to hgetall buf(size:%ld)\n", kvmap["message"].size(), kvmap2["message"].size());
            }
            else {
                fprintf(stderr, "\n\nhmset buf(size:%ld) is NOT equal to hgetall buf(size:%ld)\n", kvmap["message"].size(), kvmap2["message"].size());
            }
        }

        return -1;
    }
    */

    std::vector<std::shared_ptr<std::thread>> thrs;

    for (int index = 0; index < push_mt_thrnum; index++)
    {
        std::shared_ptr<std::thread> p(new std::thread([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "thread id:" << std::this_thread::get_id() << std::endl;
            uint64_t i;
            std::string error;
            while ((i = nallMtPushCnt.fetch_add(1)) < push_total_mt_num)
            {
                //保存MT详细内容
                int64_t nowtm = time(NULL);
                std::string key = mt_prefix + std::to_string(i);
                int64_t extm = nowtm + expiredseconds;
                std::string value = std::to_string(extm);
                std::map<std::string, std::string> kvmap = { { "msgid", value }, { "createtm", std::to_string(nowtm) } };
                int ret = m_pRedisClient->hmset(key.c_str(), kvmap, error);
                if (ret < 0) {
                    fprintf(stderr, "failed to push_queue(key:%s, tm:%ld), error:%s\n", value.c_str(), nowtm, error.c_str());
                    nallMtFailedcnt++;
                }

                //保存到过期队列
                ret = m_pRedisClient->zpush_queue(exptablename.c_str(), key, static_cast<double>(extm), error);
                if (ret < 0) {
                    fprintf(stderr, "failed to push_queue(key:%s, item:%s, score:%0.2f), error:%s\n", exptablename.c_str(), value.c_str(), static_cast<double>(extm), error.c_str());
                    nallPushQFailedcnt++;
                }
            }
            thrActiveNum--;
            fprintf(stdout, "push mt thread exited!\n");
        }));
        thrs.push_back(p);
    }

    int64_t starttm = time(NULL);
    for (int index = 0; index < scanthr; index++)
    {
        std::shared_ptr<std::thread> p(new std::thread([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "thread id:" << std::this_thread::get_id() << std::endl;
            std::string error;
            std::vector<std::string> outlist;
            while (1) 
            {
                //扫描过期队列
                int64_t nowtm = time(NULL);
                int ret = m_pRedisClient->zpop_queue(exptablename.c_str(), "-inf", std::to_string(nowtm), batchsize, outlist, error);
                nallScanCnt++;

                if (ret < 0) {
                    fprintf(stderr, "failed to push_queue(key:%s, tm:%ld, batchsize:%d), error:%s\n", exptablename.c_str(), nowtm, batchsize, error.c_str());
                    nallPopQFailedcnt++;
                    continue;
                }
                
                if (outlist.empty()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(scanreqms));
                    if (scanliveseconds > 0) {
                        if (nowtm - starttm >= scanliveseconds)
                            break; //exit thread
                    }
                    else {
                        if (nallMtPushCnt >= push_total_mt_num)
                            break; //exit thread
                    }
                    continue;
                }

                assert(0 == outlist.size() % 2);
                std::vector<std::string> rmsMsgidList;
                for (auto pos = outlist.begin(); pos != outlist.end();)
                {
                    rmsMsgidList.push_back(*pos);
                    pos += 2;
                }

                for (auto item : rmsMsgidList) {
                    std::vector<std::string> names = { "msgid", "createtm" };
                    std::map<std::string, std::string> kvmap;
                    ret = m_pRedisClient->hmget(item, names, kvmap, error);
                    if (ret < 0) {
                        fprintf(stderr, "failed to hmget(key:%s), error:%s\n", item.c_str(), error.c_str());
                        nallHGetFailedcnt++;
                    }
                    nallHGetCnt++;
                }
               
                /* crash
                std::thread t1([&](std::vector<std::string> _rmsMsgidList)
                {
                    size_t end = _rmsMsgidList.size() / 2;
                    for (size_t i = 0; i < end; i++) {
                        std::vector<std::string> names = { "msgid", "createtm" };
                        std::map<std::string, std::string> kvmap;
                        int reti = m_pRedisClient->hmget(rmsMsgidList[i], names, kvmap, error);
                        if (reti < 0) {
                            fprintf(stderr, "failed to hmget(key:%s), error:%s\n", _rmsMsgidList[i].c_str(), error.c_str());
                            nallHGetFailedcnt++;
                        }
                        nallHGetCnt++;
                    }
                }, rmsMsgidList);
        
                std::thread t2([&](std::vector<std::string> _rmsMsgidList)
                {
                    size_t start = _rmsMsgidList.size() / 2;
                    size_t end = _rmsMsgidList.size();
                    for (size_t i = start; i < end; i++) {
                        std::vector<std::string> names = { "msgid", "createtm" };
                        std::map<std::string, std::string> kvmap;
                        int reti = m_pRedisClient->hmget(_rmsMsgidList[i], names, kvmap, error);
                        if (reti < 0) {
                            fprintf(stderr, "failed to hmget(key:%s), error:%s\n", _rmsMsgidList[i].c_str(), error.c_str());
                            nallHGetFailedcnt++;
                        }
                        nallHGetCnt++;
                    }
                }, rmsMsgidList);
                t1.join();
                t2.join();
                */


                m_pRedisClient->del(rmsMsgidList, error);
                nallPopCnt.fetch_add(rmsMsgidList.size());
            }
            thrActiveNum--;
            fprintf(stdout, "scan thread exited!\n");
        }));
        thrs.push_back(p);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
   
    std::atomic<uint64_t> last_nallLMtPushCnt;
    last_nallLMtPushCnt.store(0);

    std::atomic<uint64_t> last_nallPopCnt;
    last_nallPopCnt.store(0);
   
    while (thrActiveNum != 0) {
        printf("mt[hset+push zset] speed:%lu/s, total mt error num:%lu, total push Q error num:%lu, total scan count:%lu, consume[pop zset(lua)+hget+del] speed:%lu/s, total pop error num:%lu\n",
            nallMtPushCnt - last_nallLMtPushCnt,
            nallMtFailedcnt - 0,
            nallPushQFailedcnt - 0,
            nallScanCnt - 0,
            nallPopCnt - last_nallPopCnt,
            nallPopQFailedcnt - 0
            );
        last_nallLMtPushCnt.store(nallMtPushCnt);
        last_nallPopCnt.store(nallPopCnt);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    //等待线程平滑退出
    for (size_t i = 0; i < thrs.size(); i++) {
        if (thrs[i]->joinable())
            thrs[i]->join();
    }
    return 0;
}


int TestRedis::test_slice_list(
    std::string list_topic,
    int shard_num,
    int pushthreadnum,
    uint64_t pushtotalnum,
    int popthreadnum,
    int batchsize,
    uint64_t poptotalnum)
{
    std::atomic<int> thrActiveNum;
    thrActiveNum.store(pushthreadnum + popthreadnum);

    std::atomic<uint64_t> nallPushQ;
    nallPushQ.store(0);

    std::atomic<uint64_t> nallPopCnt;
    nallPopCnt.store(0);

    std::atomic<uint64_t> nallRealPopCnt;
    nallRealPopCnt.store(0);

    std::atomic<uint64_t> nallPushQFailedcnt;
    nallPushQFailedcnt.store(0);

    std::atomic<uint64_t> nallPopQFailedcnt;
    nallPopQFailedcnt.store(0);


    MWRedisSliceList slist(m_pRedisClient);

    {
        std::string error;
        int ret = slist.listinit(list_topic, shard_num, error);
        if (0 != ret)
        {
            fprintf(stderr, "failed to listinit(list_topic:%s), ret:%d\n", list_topic.c_str(), ret);
            return -1;
        }

        ret = slist.is_available(error);
        if (ret < 0)
        {
            fprintf(stderr, "failed to is_available(list_topic:%s), ret:%d\n", list_topic.c_str(), ret);
            return -2;
        }

        /*
        {
            char szbuf[64] = { 0 };
            szbuf[1] = '1';
            szbuf[3] = '3';
            szbuf[4] = 4;
            szbuf[63] = 63;
            std::string buf(szbuf, 64);
            slist.listrpush(buf, nullptr, error);

            std::string popbuf;
            slist.listlpop(popbuf, nullptr, error);

            if (buf != popbuf) {
                fprintf(stderr, "\n\npush buf(size:%ld) is NOT equal to pop buf(size:%ld)\n", buf.size(), popbuf.size());
            }
            else {
                printf("\n\npush buf(size:%ld) is equal to pop buf(size:%ld)\n", buf.size(), popbuf.size());
            }
        }
        */
    }

    std::vector<std::shared_ptr<std::thread>> thrs;

    for (int index = 0; index < pushthreadnum; index++)
    {
            std::shared_ptr<std::thread> p(new std::thread([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::ostringstream os;
            os << std::this_thread::get_id();
            printf("thread id:%s\n", os.str().c_str());

            uint64_t i;
            std::string error;
            int ret = 0;
            while ((i = nallPushQ.fetch_add(1)) < pushtotalnum)
            {
                int64_t nowtm = time(NULL);
                std::string value = list_topic + std::to_string(i);   
                std::vector<std::string> values = { value };
                uint32_t slotindex;
                //std::this_thread::sleep_for(std::chrono::milliseconds(10000));
                //printf("enter listrpush:%ld\n", time(NULL));
                ret = slist.listrpush(values, &slotindex, error);
                //printf("leave listrpush:%ld\n", time(NULL));
                if (ret < 0) {
                    fprintf(stderr, "failed to listrpush(key:%s, tm:%ld), error:%s\n", value.c_str(), nowtm, error.c_str());
                    nallPopQFailedcnt++;
                }
                //printf("listrpush slotindex:%d\n", slotindex);
            }
            thrActiveNum--;
            fprintf(stdout, "push shared thread exited!\n");
        }));
        thrs.push_back(p);
    }

    for (int index = 0; index < popthreadnum; index++)
    {
        std::shared_ptr<std::thread> p(new std::thread([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::ostringstream os;
            os << std::this_thread::get_id();
            printf("thread id:%s\n", os.str().c_str());

            uint64_t i;
            std::string error;
            int ret = 0;
            //int curIndex = 0;
            while ((i = nallPopCnt.fetch_add(1)) < poptotalnum)
            {
                int64_t nowtm = time(NULL);
                std::string buf;
                uint32_t slotindex;
                ret = slist.listlpop(buf, &slotindex, error);
                std::vector<std::string> values;
                if (0 == ret) { //空,无数据
                    //fprintf(stderr, "list_rdrobin_range is Empty(key:%s, i:%ld, tm:%ld)\n", list_topic.c_str(), i, nowtm);
                }
                else if (ret < 0) { //出错了
                    fprintf(stderr, "failed to list_rdrobin_range(key:%s, i:%ld, tm:%ld), error:%s\n", list_topic.c_str(), i, nowtm, error.c_str());
                    nallPopQFailedcnt++;
                }
                else {
                    assert(!buf.empty());
                    if (!buf.empty()) {
                        nallRealPopCnt.fetch_add(1);
                    }
                }
                //printf("listrpop slotindex:%d, buf:%s\n", slotindex, buf.c_str());
            }
            thrActiveNum--;
            fprintf(stdout, "pop shared thread exited!\n");
        }));
        thrs.push_back(p);
    }

    std::atomic<uint64_t> last_nallPushQ;
    last_nallPushQ.store(0);

    std::atomic<uint64_t> last_nallPopCnt;
    last_nallPopCnt.store(0);


    std::atomic<uint64_t> last_nallRealPopCnt;
    last_nallRealPopCnt.store(0);

    while (thrActiveNum != 0) {
        printf("shard list(%s) push speed:%lu/s, total push error num:%lu, pop speed:%lu/s, total real pop number:%lu, total pop error num:%lu\n",
            list_topic.c_str(),
            nallPushQ - last_nallPushQ,
            nallPushQFailedcnt - 0,
            nallPopCnt - last_nallPopCnt,
            nallRealPopCnt - 0,
            nallPopQFailedcnt - 0
            );
        last_nallPushQ.store(nallPushQ);
        last_nallPopCnt.store(nallPopCnt);
        last_nallRealPopCnt.store(nallRealPopCnt);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    //等待线程平滑退出
    for (size_t i = 0; i < thrs.size(); i++) {
        if (thrs[i]->joinable())
            thrs[i]->join();
    }

    return 0;
}
