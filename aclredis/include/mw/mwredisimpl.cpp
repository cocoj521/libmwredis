#include <assert.h>
#include <map>
#include "acl_stdafx.hpp"
#include "mwredisimpl.h"
#include "mwredisslicelist.h"


// 构造函数
MWRedisRedisImpl::MWRedisRedisImpl()  { }

// 析构函数
MWRedisRedisImpl::~MWRedisRedisImpl() { }


// 初始化redis组件
int MWRedisRedisImpl::initRedis(const std::string& strRedisCluster,
    const std::string& strRedisPwd,
    int nConnTimeout,
    int nRWTimeout,
    int nRetryLimit,
    int nRetryInterval,
    int nRetrySleep,
    int nPoolSize,
    bool bPreset,
    bool bStdoutOpen)
{
    if (m_bInited)
    {
        // 已经初始化过了，返回失败
        return -1;
    }

    //检验redis服务器地址
    if (strRedisCluster.empty())
    {
        return -2;
    }

    // 保存参数
    m_strRedisCluster = strRedisCluster;
    m_strRedisPwd = strRedisPwd;
    if (nConnTimeout >= 0)
    {
        m_nConnTimeout = nConnTimeout > 60 ? 60 : nConnTimeout;
    }

    if (nRWTimeout >= 0)
    {
        m_nRWTimeout = nRWTimeout > 60 ? 60 : nRWTimeout;
    }

    if (nRetryLimit >= 0)
    {
        m_nRetryLimit = nRetryLimit > 30 ? 30 : nRetryLimit;
    }

    m_nRetryInterval = m_nRetryInterval;
    m_nRetrySleep = nRetrySleep;

    
    /*
    if (nPoolSize > 0)
    {
        m_nPoolSize = nPoolSize > 32 ? 32: nPoolSize;
    }
    */
    m_nPoolSize = nPoolSize;

    m_preset = bPreset;

    // 初始acl组件
    acl::acl_cpp_init();
    acl::log::stdout_open(bStdoutOpen);

    m_cluster = std::make_shared<acl::redis_client_cluster>();
    if (nullptr == m_cluster) 
    {
        return -3;
    }

    // 当某个连接池结点出问题，设置探测该连接结点是否恢复的时间间隔(秒)，当该值
    // 为 0 时，则不检测
    m_cluster->set_retry_inter(m_nRetryInterval);

    // 设置重定向的最大阀值，若重定向次数超过此阀值则报错
    m_cluster->set_redirect_max(m_nRetryLimit);

    // 当重定向次数 >= 2 时每次再重定向此函数设置休息的时间(毫秒)
    m_cluster->set_redirect_sleep(m_nRetrySleep);

    // 初始化redis组件
    acl::string addrs(m_strRedisCluster.c_str()), cmd, passwd(m_strRedisPwd.c_str());
    m_cluster->init(NULL, addrs.c_str(), m_nPoolSize, m_nConnTimeout, m_nRWTimeout);

    // 设置连接 redis 集群的密码，第一个参数为一个 redis 服务节点的服务地址，
    // 当第一个参数值为 default 时，则设置了所有节点的统一连接密码
    if (!m_strRedisPwd.empty())
    {
        m_cluster->set_password("default", m_strRedisPwd.c_str());
    }

    // 是否需要将所有哈希槽的对应关系提前设置好，这样可以去掉运行时动态添加
    // 哈希槽的过程，从而可以提高运行时的效率
    if (m_preset)
    {
        const std::vector<acl::string>& token = addrs.split2(",; \t");
        m_cluster->set_all_slot(token[0], m_nPoolSize);
    }

    //检查有无连上redis服务器
    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    bool ret = redis.ping();
    if (false == ret)
    {
        //没连上redis服务器
        printf("failed to ping redis:%s\n", redis.result_error());
        m_cluster.reset();
        return -4;
    }

    m_bInited = true;
    return 0;
}


// 销毁redis组件
void MWRedisRedisImpl::unintRedis()
{
    if (m_bInited)
    {
        // nothing to do here
        m_bInited = false;
        m_cluster.reset();
    }
}


bool MWRedisRedisImpl::ping()
{
    if (!m_bInited)
    {
        return false;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    return redis.ping();
}

/************************************************************************
*  功能:hash std::map set
*  参数:key：key cacheMap:属性表
*  @return 0:成功; 非0:错误码
************************************************************************/
int MWRedisRedisImpl::hmset(const std::string& key, const std::map<std::string, std::string>& kvmap, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    std::map<acl::string, acl::string> attrs;
    for (auto item = kvmap.begin(); item != kvmap.end(); item++)
    {
        acl::string k(item->first.c_str(), item->first.size());
        acl::string v(item->second.c_str(), item->second.size());
        attrs[k] = v;
    }

    if (redis.hmset(key.c_str(), attrs) == false)
    {
        error = redis.result_error();
        return -2;
    }

    return 0;
}


/************************************************************************
*  功能:hash set
*  @return 
*  1:表示新添加的域字段添加成功
*  0:表示更新已经存在的域字段成功
*  <0表示出错或该 key 对象非哈希对象或从结点禁止修改
************************************************************************/
int MWRedisRedisImpl::hset(const std::string& key, const std::string& name, const std::string& value, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    int ret = redis.hset(key.c_str(), name.c_str(), name.size(), value.c_str(), value.size());
    if (ret < 0)
    {
        error = redis.result_error();
        return -2;
    }

    return ret;
}



/************************************************************************
*  功能:hash std::map get
*  参数:key：key names:待取的属性字段 cacheMap:返回属性表
*  @return 0:成功; 非0:错误码
************************************************************************/
int MWRedisRedisImpl::hmget(const std::string& key, const std::vector<std::string>& names, std::map<std::string, std::string>& kvmap, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    assert(!key.empty());
    assert(names.size()>0);

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    std::vector<acl::string> tempName;
    for (auto i = names.begin(); i != names.end(); i++)
    {
        tempName.emplace_back(acl::string(i->c_str(), i->size()));
    }
    std::vector<acl::string> result;
    if (redis.hmget(key.c_str(), tempName, &result) == false)
    {
        error = redis.result_error();
        return -2;
    }

    size_t size = redis.result_size();
    for (size_t j = 0; j < size; j++)
    {
        size_t len = 0;
        const char* val = redis.result_value(j, &len);
        if (nullptr != val && len > 0) {
            kvmap[names[j]] = std::string(val, len);
        }
        else {
            kvmap[names[j]] = "";
        }
    }

    return 0;
}



int MWRedisRedisImpl::hget(const std::string& key, const std::string& name, std::string& result, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    assert(!key.empty());
    assert(!name.empty());

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    acl::string tmpresult;
    if (redis.hget(key.c_str(), name.c_str(), name.length(), tmpresult) == false)
    {
        error = redis.result_error();
        return -2;
    }

    std::string tmp(tmpresult.c_str(), tmpresult.size());
    result = std::move(tmp);
    return 0;
}


int MWRedisRedisImpl::hgetall(const std::string& key, std::map<std::string, std::string>& kvmap, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    assert(!key.empty());
    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    std::map<acl::string, acl::string> result;
    if (redis.hgetall(key.c_str(), result) == false)
    {
        error = redis.result_error();
        return -2;
    }

    for (auto& item : result) {
        kvmap.emplace(std::make_pair(std::string(item.first.c_str(), item.first.size()), 
            std::string(item.second.c_str(), item.second.size())));
    }

    return 0;
}




/**
* 从 redis 哈希表中删除某个 key 对象的某些域字段
* remove one or more fields from hash stored at key
* @param key {const char*} key 键值
*  the hash key
* @param first_name {const char*} 第一个域字段名，最后一个字段必须是 NULL
*  the first field of the fields list, the last field must be NULL
*  indicating the end of vary parameters
* @return {int} 成功删除的域字段个数，返回 -1 表示出错或该 key 对象非哈希对象
*  return the number of fields be removed successfully, or -1 when
*  error happened or operating on a no hash key
*/
int MWRedisRedisImpl::hdel(const std::string& key, const std::string& first_name, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    std::vector<acl::string> tmplist;
    tmplist.emplace_back(acl::string(first_name.c_str(), first_name.size()));
    int ret = redis.hdel(key.c_str(), tmplist);
    if (ret < 0)
    {
        error = redis.result_error();
    }
    return ret;
}



/**
* 获得某个 key 对象中所有域字段的数量
* get the count of fields in hash stored at key
* @param key {const char*} key 键值
*  the hash key
* @return {int} 返回值含义：
*  return int value as below:
*  <0 -- 出错或该 key 对象非哈希对象
*        error or not a hash key
*  >0 -- 域字段数量
*        the count of fields
*   0 -- 该 key 不存在或域字段数量为 0
*        key not exists or no fields in hash stored at key
*/
int MWRedisRedisImpl::hlen(const std::string& key, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    int ret = redis.hlen(key.c_str());
    if (ret < 0)
    {
        error = redis.result_error();
    }
    return ret;
}


int MWRedisRedisImpl::hincrby(const std::string& key, const std::string& name, const long long int& inc, long long int* result, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    if (false == redis.hincrby(key.c_str(), name.c_str(), inc, result))
    {
        error = redis.result_error();
        return -2;
    }
    return 0;
}


int MWRedisRedisImpl::lpush(const char* key, const std::vector<std::string>& values, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    assert(nullptr != key && !values.empty());
    if (nullptr == key || values.empty())
    {
        error = "invalid parameter";
        return -2;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    std::vector<acl::string> members;
    for (auto& item : values)
    {
        acl::string tmp(item.c_str(), item.size());
        members.emplace_back(tmp);
    }

    int ret = redis.lpush(key, members);
    if (ret < 0)
    {
        error = redis.result_error();
    }
    return ret;
}


int MWRedisRedisImpl::rpush(const char* key, const std::vector<std::string>& values, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    assert(nullptr != key && !values.empty());
    if (nullptr == key || values.empty())
    {
        error = "invalid parameter";
        return -2;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    std::vector<acl::string> members;
    for (auto& item : values)
    {
        acl::string tmp(item.c_str(), item.size());
        members.emplace_back(std::move(tmp));
    }

    int ret = redis.rpush(key, members);
    if (ret < 0)
    {
        error = redis.result_error();
    }
    return ret;
}


int MWRedisRedisImpl::lpop(const char* key, std::string& buf, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    assert(nullptr != key);
    if (nullptr == key)
    {
        error = "invalid parameter";
        return -2;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    acl::string tempout;
    int ret = redis.lpop(key, tempout);
    if (ret < 0)
    {
        //fprintf(stderr,"sadd key: %s error", key.c_str());
        error = redis.result_error();
    }
    else {
        std::string tmp(tempout.c_str(), tempout.size());
        buf = std::move(tmp);
    }
    return ret;
}


int MWRedisRedisImpl::rpop(const char* key, std::string& buf, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    assert(nullptr != key);
    if (nullptr == key)
    {
        error = "invalid parameter";
        return -2;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    acl::string tempout;
    int ret = redis.rpop(key, tempout);
    if (ret < 0)
    {
        //fprintf(stderr,"sadd key: %s error", key.c_str());
        error = redis.result_error();
    }
    else {
        std::string tmp(tempout.c_str(), tempout.size());
        buf = std::move(tmp);
    }
    return ret;
}


int MWRedisRedisImpl::lrange(const char* key, int start, int end, std::vector<std::string>& result, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    assert(nullptr != key);
    if (nullptr == key)
    {
        error = "invalid parameter";
        return -2;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    std::vector<acl::string> res;
    bool bret = redis.lrange(key, start, end, &res);
    if (false == bret)
    {
        error = redis.result_error();
        return -3;
    }

    for (auto pos = res.begin(); pos != res.end(); ++pos)
    {
        result.emplace_back(std::string(pos->c_str(), pos->size()));
    }

    return static_cast<int>(result.size());
}


int MWRedisRedisImpl::llen(const std::string& key, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    int ret = redis.llen(key.c_str());
    if (ret < 0)
    {
        error = redis.result_error();
    }
    return ret;
}


int MWRedisRedisImpl::slicelist_init(const std::string& topic, const int slicenum, std::string& error)
{
    if (topic.empty() || slicenum <= 0) {
        error = "invalid parameter";
        return -1;
    }

    auto pos = m_sliceTopicMap.find(topic);
    if (pos != m_sliceTopicMap.end())
    {
        error = "topic is already initialized";
        return 1;
    }

    {
        std::lock_guard<std::mutex> guard(m_sliceMutex);
        auto it = m_sliceTopicMap.insert(std::make_pair(topic, std::make_shared<MWRedisSliceList>(shared_from_this())));
        pos = it.first;

        assert(pos != m_sliceTopicMap.end());
        if (pos == m_sliceTopicMap.end())
        {
            error = "failed to insert slice topic map";
            return -2;
        }

        return pos->second->listinit(topic, slicenum, error);
    }
}


int MWRedisRedisImpl::slicelist_rpush(const std::string& topic, const int slicenum, const std::string& buf, uint32_t* slotindex, std::string& error)
{
    if (topic.empty() || slicenum <= 0) {
        error = "invalid parameter";
        return -1;
    }

    auto pos = m_sliceTopicMap.find(topic);
    if (pos == m_sliceTopicMap.end())
    {
        std::lock_guard<std::mutex> guard(m_sliceMutex);
        auto it = m_sliceTopicMap.insert(std::make_pair(topic, std::make_shared<MWRedisSliceList>(shared_from_this())));
        pos = it.first;
        if (true == it.second)
        {
            int ret = pos->second->listinit(topic, slicenum, error);
            assert(0 == ret);
            if (0 != ret) {
                error = "listinit:" + error;
                return ret;
            }
        }
    }
    return pos->second->listrpush(buf, slotindex, error);
}

int MWRedisRedisImpl::slicelist_lpop(const std::string& topic, const int slicenum, std::string& buf, uint32_t* slotindex, std::string& error)
{
    if (topic.empty() || slicenum <= 0) {
        error = "invalid parameter";
        return -1;
    }

    auto pos = m_sliceTopicMap.find(topic);
    if (pos == m_sliceTopicMap.end())
    {
        std::lock_guard<std::mutex> guard(m_sliceMutex);
        auto it = m_sliceTopicMap.insert(std::make_pair(topic, std::make_shared<MWRedisSliceList>(shared_from_this())));
        pos = it.first;
        if (true == it.second)
        {
            int ret = pos->second->listinit(topic, slicenum, error);
            assert(0 == ret);
            if (0 != ret) {
                error = "listinit:" + error;
                return ret;
            }
        }
    }

    return pos->second->listlpop(buf, slotindex, error);
}

int MWRedisRedisImpl::slicelist_totallen(const std::string& topic, const int slicenum, std::string& error)
{
    if (topic.empty() || slicenum <= 0) {
        error = "invalid parameter";
        return -1;
    }

    auto pos = m_sliceTopicMap.find(topic);
    if (pos == m_sliceTopicMap.end())
    {
        std::lock_guard<std::mutex> guard(m_sliceMutex);
        auto it = m_sliceTopicMap.insert(std::make_pair(topic, std::make_shared<MWRedisSliceList>(shared_from_this())));
        pos = it.first;
        if (true == it.second)
        {
            int ret = pos->second->listinit(topic, slicenum, error);
            assert(0 == ret);
            if (0 != ret) {
                error = "listinit:" + error;
                return ret;
            }
        }
    }

    return pos->second->list_totallen(error);
}



int MWRedisRedisImpl::sadd(const std::string& key, const std::vector<std::string>& members, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    std::vector<acl::string> tempmembers;
    for (auto& item : members)
    {
        tempmembers.emplace_back(acl::string(item.c_str(), item.size()));
    }
    int ret = redis.sadd(key.c_str(), tempmembers);
    if (ret < 0)
    {
        error = redis.result_error();
        return ret;
    }
    return 0;
}


int  MWRedisRedisImpl::zrange(const std::string& key, int start, int stop,
    std::vector<std::string>& result, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    std::vector<acl::string> res;
    int ret = redis.zrange(key.c_str(), start, stop, &res);
    if (ret < 0)
    {
        error = redis.result_error();
        return ret;
    }

    for (auto pos = res.begin(); pos != res.end(); ++pos)
    {
        result.emplace_back(std::string(pos->c_str(), pos->size()));
    }
    return ret;
}


int MWRedisRedisImpl::smembers(const std::string& key, std::vector<std::string>& result, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    std::vector<acl::string> res;
    int ret = redis.smembers(key.c_str(), &res);
    if (ret < 0)
    {
        error = redis.result_error();
        return ret;
    }

    for (auto pos = res.begin(); pos != res.end(); ++pos)
    {
        result.emplace_back(std::string(pos->c_str(), pos->size()));
    }
    return ret;
}


int MWRedisRedisImpl::srem(const char* key, const std::vector<std::string>& members, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    std::vector<acl::string> vMem;
    for (auto pos = members.begin(); pos != members.end(); ++pos)
    {
        vMem.emplace_back(acl::string(pos->c_str(), pos->size()));
    }

    int ret = redis.srem(key, vMem);
    if (ret < 0)
    {
        error = redis.result_error();
    }
    return ret;
}


int MWRedisRedisImpl::sismember(const std::string& key, const std::string& member, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    std::vector<acl::string> res;
    bool bret = redis.sismember(key.c_str(), member.c_str(), member.length());
    if (false == bret)
    {
        error = redis.result_error();
        return -2;
    }
    return 0;
}


int MWRedisRedisImpl::scard(const char* key, std::string& error)
{
    if (!m_bInited)
    {
        //fprintf(stderr,"redis is uninitialized!");
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    int ret = redis.scard(key);
    if (ret < 0)
    {
        error = redis.result_error();
    }
    return ret;
}


int MWRedisRedisImpl::zadd(const std::string& key, const std::string& item, double score, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    std::map<acl::string, double> members;
    acl::string member(item.c_str(), item.size());
    members[member] = score;

    int ret = redis.zadd(key.c_str(), members);
    if (ret < 0)
    {
        error = redis.result_error();
    }
    return ret;
}


int MWRedisRedisImpl::zrem(const char* key, const std::vector<std::string>& members, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    std::vector<acl::string> vMem;
    for (auto pos = members.begin(); pos != members.end(); ++pos)
    {
        vMem.emplace_back(acl::string(pos->c_str(), pos->size()));
    }

    int ret = redis.zrem(key, vMem);
    if (ret < 0)
    {
        error = redis.result_error();
    }
    return ret;
}



int MWRedisRedisImpl::zcard(const char* key, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    int ret = redis.zcard(key);
    if (ret <= 0)
    {
        error = redis.result_error();
    }

    return ret;
}



/**
* 获得 key 的有序集中指定分值区间的成员个数
* get the number of elements in a sorted set with scores within
* the given values
* @param key {const char*} 有序集键值
*  the key of a sorted set
* @param min {double} 最小分值
*  the min score specified
* @param max {double} 最大分值
*  the max socre specified
* @return {int} 符合条件的成员个数
*  the number of elements in specified score range
*  0：该键对应的有序集不存在或该 KEY 有序集的对应分值区间成员为空
*     nothing in the specified score range, or the key doesn't exist
*  <0: 出错或该键的数据对象不是有效的有序集对象
*     error or it is not a sorted set by the key
*/
int MWRedisRedisImpl::zcount(const char* key, double min, double max, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    int ret = redis.zcount(key, min, max);
    if (ret < 0)
    {
        error = redis.result_error();
    }
    return ret;
}


int MWRedisRedisImpl::zrangebyscore_with_scores(const char* key, double min, double max,
    std::vector<std::pair<std::string, double>>& out, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    std::vector<std::pair<acl::string, double>> tmpout;
    int ret = redis.zrangebyscore_with_scores(key, min, max, tmpout, NULL, NULL);
    if (ret < 0)
    {
        error = redis.result_error();
    }
    else 
    {
        for (auto& item : tmpout)
        {
            out.emplace_back(std::make_pair(std::string(item.first.c_str(), item.first.size()), item.second));
        }
    }
    return ret;
}


int MWRedisRedisImpl::zrangebyscore_with_scores(const char* key, char* min, char* max,
    std::vector<std::pair<std::string, double> >& out, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    std::vector<std::pair<acl::string, double>> tmpout;
    int ret = redis.zrangebyscore_with_scores(key, min, max, tmpout, NULL, NULL);
    if (ret < 0)
    {
        error = redis.result_error();
    }
    else
    {
        for (auto& item : tmpout)
        {
            out.emplace_back(std::make_pair(std::string(item.first.c_str(), item.first.size()), item.second));
        }
    }
    return ret;
}


bool MWRedisRedisImpl::set(const std::string& key, const std::string& value, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return false;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    bool ret = redis.set(key.c_str(), key.size(), value.c_str(), value.size());
    if (false == ret)
    {
        error = redis.result_error();
    }
    return ret;
}


/************************************************************************
*  功能:get
*  参数:key：key value:返回的value
*  @return 0:成功; 非0:错误码
************************************************************************/
bool MWRedisRedisImpl::get(const std::string& key, std::string& value, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return false;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    acl::string v;
    if (redis.get(key.c_str(), key.size(), v) == false)
    {
        error = redis.result_error();
        return false;
    }

    std::string tmp(v.c_str(), v.size());
    value = std::move(tmp);
    return true;
}

/************************************************************************
* 将 key 改名为 newkey
* rename a key
* @return {bool}
*  true on success, or error happened
************************************************************************/
bool MWRedisRedisImpl::rename_key(const char* key, const char* newkey, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return false;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    bool bret = redis.rename_key(key, newkey);
    if (!bret)
    {
        error = redis.result_error();
    }
    return bret;
}


int MWRedisRedisImpl::pexpire(const char* key, int n, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    int ret = redis.pexpire(key, n);
    if (ret <= 0)
    {
        error = redis.result_error();
    }
    return ret;
}


int MWRedisRedisImpl::expire(const char* key, int n, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    int ret = redis.expire(key, n);
    if (ret <= 0)
    {
        error = redis.result_error();
    }
    return ret;
}


/************************************************************************
* 判断 KEY 是否存在
* check if the key exists in redis
* @param key {const char*} KEY 值
*  the key
* @return {bool} 返回 true 表示存在，否则表示出错或不存在
*  true returned if key existing, false if error or not existing
************************************************************************/
bool MWRedisRedisImpl::exists(const char* key, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return false;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    bool ret = redis.exists(key);
    if (false == ret){
        error = redis.result_error();
    }
    return ret;
}


int MWRedisRedisImpl::del(const std::vector<std::string>& keys, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);
    std::vector<acl::string> vKey;
    for (auto pos = keys.begin(); pos != keys.end(); ++pos)
    {
        vKey.emplace_back(acl::string(pos->c_str(), pos->size()));
    }

    int ret = redis.del(vKey);
    if (ret < 0)
    {
        error = redis.result_error();
    }
    return ret;
}



bool MWRedisRedisImpl::incrby(const std::string& key, const long long int& inc, long long int* result, std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return false;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    bool ret = redis.incrby(key.c_str(), inc, result);
    if (false == ret)
    {
        error = redis.result_error();
    }
    return ret;
}



int MWRedisRedisImpl::zpush_queue(const std::string& key,
    const std::string& item,
    double score,
    std::string& error)
{
    return this->zadd(key, item, score, error);
}



int MWRedisRedisImpl::zpop_queue(const std::string& key,
    const std::string& minpos,
    const std::string& maxpos,
    const int& batchsize,
    std::vector<std::string>& outlist,
    std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    assert(batchsize > 0);
    if (batchsize <= 0) {
        error = "invalid parameter";
        return -2;
    }

    //./redis-cli -c -h 192.169.7.135 -p 7001 
    //EVAL "local e=redis.call('ZRANGEBYSCORE', 'keyQ7', '-inf', '+inf','withscores', 'limit', 0, 2) if e[1] ~= nil then for k,v in ipairs(e) do if k%2==1 then redis.call('ZREM', KEYS[1], v) end end end return e" 1 keyQ7 0
    std::string script = "local e = redis.call('ZRANGEBYSCORE', KEYS[1], ARGV[1], ARGV[2], 'withscores', 'limit', 0, ARGV[3]) if e[1] ~= nil then for k,v in ipairs(e) do if k%2==1 then redis.call('ZREM', KEYS[1], v) end end end return e";
    std::vector<std::string> keys = { key };
    std::vector<std::string> args = { minpos, maxpos, std::to_string(batchsize) };
    outlist.clear();

    //item1,score1,item2,score2,itemN,scoreN
    int ret = this->eval_strings(script.c_str(), keys, args, outlist, error);
    return ret;
}


int MWRedisRedisImpl::eval_number(const char* script,
    const std::vector<std::string>& keys,
    const std::vector<std::string>& args,
    int& out,
    std::string& error)
{
    if (!m_bInited)
    {
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    std::vector<acl::string> tempkeys;
    for (auto item : keys) {
        tempkeys.emplace_back(acl::string(item.c_str(), item.size()));
    }

    std::vector<acl::string> tempargs;
    for (auto item : args) {
        tempargs.emplace_back(acl::string(item.c_str(), item.size()));
    }

    bool bret = redis.eval_number(script, tempkeys, tempargs, out);
    if (false == bret) {
        error = redis.result_error();
        return -2;
    }
    return 0;
}

int MWRedisRedisImpl::eval_strings(const char* script,
    const std::vector<std::string>& keys,
    const std::vector<std::string>& args,
    std::vector<std::string>& out,
    std::string& error)
{
    if (!m_bInited)
    {
        //fprintf(stderr,"redis is uninitialized!");
        error = "redis is uninitialized";
        return -1;
    }

    acl::redis redis;
    redis.set_cluster(m_cluster.get(), m_nPoolSize);

    std::vector<acl::string> tempkeys;  
    for (auto item : keys) {
        tempkeys.emplace_back(acl::string(item.c_str(), item.size()));
    }

    std::vector<acl::string> tempargs;
    for (auto item : args) {
        tempargs.emplace_back(acl::string(item.c_str(), item.size()));
    }

    std::vector<acl::string> tempout;
    int ret = redis.eval_strings(script, tempkeys, tempargs, tempout);
    if (ret < 0) {
        error = redis.result_error();
        return -2;
    }

    for (auto item : tempout) {
        out.emplace_back(std::string(item.c_str(), item.size()));
    }
    return 0;
}



/************************************************************************/
/* 产生MWRedisRedisImpl对象											    */
/************************************************************************/
MWRedisClient* MWRedisClientFactory::New()
{
    return new MWRedisRedisImpl();
}

/************************************************************************/
/* free                                                                 */
/************************************************************************/
void MWRedisClientFactory::Destroy(MWRedisClient* pBase)
{
    delete pBase;
}