/*
************************************************************************
*
* Description : redis测试类
*
* Created Date : 2019 / 8 / 29
*
* Author : Peter Hu
*
* Copyright(c) ShenZhen Montnets Technology, Inc.All rights reserved.
*
************************************************************************
*/

#ifndef __REDISTEST_H__
#define __REDISTEST_H__

#include <memory>
#include <vector>
#include <atomic>
#include "mw/mwredisslicelist.h"
#include "mw/mwredislib.h"

int doTestRedis(int argc, char* argv[]);


class TestRedis
{
public:
    TestRedis();
    ~TestRedis();

public:
    //初始化连接redis服务器
    int InitRedisDB(const std::string& strRedisCluster, //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
        const std::string& strRedisPwd, //redis访问密码，默认为空
        int  nConnTimeout, //连接超时时间，默认为30秒,最大值60
        int  nRWTimeout, //读写超时时间，默认为30秒,最大值60
        int  nRetryLimit, //设置重定向的最大阀值，若重定向次数超过此阀值则报错,最大值30，默认为15
        int  nRetryInterval, //当某个连接池结点出问题，设置探测该连接结点是否恢复的时间间隔(秒)，当该值为 0 时，则不检测,默认为1
        int  nRetrySleep, //当重定向次数 >= 2 时每次再重定向此函数设置休息的时间(毫秒)，默认为500毫秒
        int  nPoolSize, //最大线程数，默认为4
        bool bPreset,  //是否需要将所有哈希槽的对应关系提前设置好，这样可以去掉运行时动态添加哈希槽的过程，从而可以提高运行时的效率, 默认true
        bool bStdoutOpen);

    //反初始化
    void DestoryRedisDB();

public:
    int test_set(std::string& prefix, int thrnum, uint64_t allnum);
    int test_get(std::string& prefix, int thrnum, uint64_t allnum);
    int test_lpush(std::string& key, int thrnum, uint64_t allnum);
    int test_lpop(std::string& key, int thrnum, uint64_t allnum);

public:
    //push mt thread num, total mt num, expired seconds, scan frequency, scan thread num, scan batch size
    int test_mt(std::string mt_prefix, int push_mt_thrnum, uint64_t push_total_mt_num, std::string exptablename, int expiredseconds, int scanreqms, int scanthr, int batchsize, int scanliveseconds);

public:
    int test_slice_list(
    std::string list_topic,
    int shard_num,
    int pushthreadnum,
    uint64_t pushtotalnum,
    int popthreadnum,
    int batchsize,
    uint64_t poptotalnum);

public:
    int test_exception(std::string hgetall_key_name, int threadnum, uint64_t totalnum);
public:
    int test_push_queue();
    int test_pop_queue();
    int test_insert_hashmap();
    int test_get_hashmap();
    int test_modify_hashmap_filed();
private:
    std::shared_ptr<MWRedisClient> m_pRedisClient = nullptr;
};

#endif // !__REDISTEST_H__
