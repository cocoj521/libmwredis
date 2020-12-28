/*
************************************************************************
*
* Description : acl-redis纯虚接口
*
* Created Date : 2019 / 7 / 26
*
* Modified Date: 2019 / 8 / 5
*
* Author: Peter Hu
*
* Copyright(c) ShenZhen Montnets Technology, Inc.All rights reserved.
*
************************************************************************
*/

#ifndef __MWREDISLIB_H__
#define __MWREDISLIB_H__

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <memory>

/************************************************************************/
/*  MWRedisClient 基类                                                  */
/************************************************************************/
class MWRedisClient : public std::enable_shared_from_this<MWRedisClient>
{
public:
    MWRedisClient() { }
    virtual ~MWRedisClient() { }

public:
    // 初始化redis组件
    virtual int initRedis(const std::string& strRedisCluster,
        const std::string& strRedisPwd,
        int  nConnTimeout,
        int  nRWTimeout,
        int  nRetryLimit,
        int  nRetryInterval,
        int  nRetrySleep,
        int  nPoolSize,
        bool bPreset,
        bool bStdoutOpen, 
        std::string& error) = 0;

    // 销毁redis组件
    virtual void unintRedis() = 0;

public:
    /**
    * 探测 redis 连接是否正常
    * PING command for testing if the connection is OK
    * @return {bool} 连接是否正常
    *  return true if success
    */
    virtual bool ping() = 0;

public:
    //////////////////////////////////////////////////////////////////////////
    //hashes
    /************************************************************************
    *  功能:hash set
    *  参数:key：key kvMap:设置的属性 error:错误描述
    *  @return 0:成功; <0:错误码
    ************************************************************************/
    virtual int hmset(const std::string& key, const std::map<std::string, std::string>& kvmap, std::string& error) = 0;


    /************************************************************************
    *  功能:hash set
    *  @return
    *  1:表示新添加的域字段添加成功
    *  0:表示更新已经存在的域字段成功
    *  <0表示出错或该 key 对象非哈希对象或从结点禁止修改
    ************************************************************************/
    virtual int hset(const std::string& key, const std::string& name, const std::string& value, std::string& error) = 0;


    /************************************************************************
    *  功能:hash std::map get
    *  参数:key：key names:待取的属性字段 kvMap:返回属性表 error:错误描述
    *  @return 0:成功; <0:错误码
    ************************************************************************/
    virtual int hmget(const std::string& key, const std::vector<std::string>& names, std::map<std::string, std::string>& kvmap, std::string& error) = 0;


    /**
    * 从 redis 哈希表中获取某个 key 对象的某个域的值
    * get the value assosiated with field in the hash stored at key
    * @param key key 键值
    *  the hash key
    * @param name key 对象的域字段名称
    *  the field's name
    * @param result 存储查询结果值(内部对该 string 进行内容追加)
    *  store the value result of the given field
    * @return {bool} 返回值含义：
    *  0  -- 操作成功，当result为空时表示 KEY 或字段域不存在
    *          get the value associated with field; if result is empty then
    *          the key or the name field doesn't exist
    *  <0 -- 域字段不存在或操作失败或该 key 对象非哈希对象
    *           the field not exists, or error happened,
    *           or the key isn't a hash key
    */
    virtual int hget(const std::string& key, const std::string& name, std::string& result, std::string& error) = 0;


    /************************************************************************
    *  功能:hash std::map get
    *  参数:key：key kvMap:返回所有属性表 error:错误描述
    *  @return 0:成功; <0:错误码
    ************************************************************************/
    virtual int hgetall(const std::string& key, std::map<std::string, std::string>& kvmap, std::string& error) = 0;


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
    virtual int hdel(const std::string& key, const std::string& first_name, std::string& error) = 0;


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
    virtual int hlen(const std::string& key, std::string& error) = 0;


    /**
    * 当某个 key 对象中的某个域字段为整数时，对其进行加减操作
    * the hash key
    * @param name key 对象的域字段名称
    * @param inc {long long int} 增加的值，可以为负值
    * @param result {long long int*} 非 NULL 时存储结果值
    * @return 0:成功，<0:表明出错或该 key 对象非哈希对象或该域字段非整数类型
    */
    virtual int hincrby(const std::string& key, const std::string& name, const long long int& inc, long long int* result, std::string& error) = 0;

public:
    //////////////////////////////////////////////////////////////////////////
    //list
    /**
    * 将一个或多个值元素插入到列表对象 key 的表头
    * add one or more element(s) to the head of a list
    * @param key {const char*} 列表对象的 key
    *  the list key
    * @param first_value {const char*} 第一个非空字符串，该变参的列表的最后一个
    *  必须设为 NULL
    *  the first no-NULL element of the variable args, the last arg must
    *  be NULL indicating the end of the args.
    * @return {int} 返回添加完后当前列表对象中的元素个数，返回 -1 表示出错或该 key
    *  对象非列表对象，当该 key 不存在时会添加新的列表对象及对象中的元素
    *  return the number of elements in list. -1 if error happened,
    *  or the object specified by key is not a list.
    */
    virtual int lpush(const char* key, const std::vector<std::string>& values, std::string& error) = 0;


    /**
    * 将一个或多个值元素插入到列表对象 key 的表尾
    * append one or multiple values to a list
    * @param key {const char*} 列表对象的 key
    *  the key of a list
    * @param first_value {const char*} 第一个非空字符串，该变参的列表的最后一个
    *  必须设为 NULL
    *  the first element of a variable args must be not NULL, and the
    *  last arg must be NULL indicating the end of the args.
    * @return {int} 返回添加完后当前列表对象中的元素个数，返回 -1 表示出错或该 key
    *  对象非列表对象，当该 key 不存在时会添加新的列表对象及对象中的元素
    *  return the number of a list specified by a key. -1 if error
    *  happened, or the key's object isn't a list, if the list by the
    *  key doese not exist, a new list will be created with the key.
    */
    virtual int rpush(const char* key, const std::vector<std::string>& values, std::string& error) = 0;


    /**
    * 返回列表 key 中指定区间内（闭区间）的元素，区间以偏移量 start 和 end 指定；
    * 下标起始值从 0 开始，-1 表示最后一个下标值
    * @return {int} 返回值含义：
    * >=0：表示成功弹出元素的个数，
    * <0:错误
    */
    virtual int lrange(const char* key, int start, int end, std::vector<std::string>& result, std::string& error) = 0;


    /**
    * 从列表对象中移除并返回头部元素
    * remove and get the element in the list's head
    * @param key {const char*} 元素对象的 key
    *  the key of one list
    * @param buf {string&} 存储弹出的元素值
    *  store the element when successful.
    * @return {int} 返回值含义：>0 -- 表示成功弹出一个元素且返回值表示元素的长度，
    *  <0 -- 表示出错，或该对象非列表对象，或该对象已经为空
    *  return value as below:
    *   >0: get one element successfully and return the length of element
    *  <0: error happened, or the oject is not a list specified
    *      by the key, or the list specified by key is empty
    */
    virtual int lpop(const char* key, std::string& buf, std::string& error) = 0;

    /**
    * 从列表对象中移除并返回尾部元素
    * remove and get the last element of a list
    * @param key {const char*} 元素对象的 key
    *  the key of the list
    * @param buf {string&} 存储弹出的元素值
    *  store the element pop from list
    * @return {int} 返回值含义：>0 -- 表示成功弹出一个元素且返回值表示元素的长度，
    *  <0 -- 表示出错，或该对象非列表对象，或该对象已经为空
    *  return value as below:
    *  >0: get one element successfully and return the length of element
    *  <0: error happened, or the oject is not a list specified
    *      by the key, or the list specified by key is empty
    */
    virtual int rpop(const char* key, std::string& buf, std::string& error) = 0;


    /**
    * 返回指定列表对象的元素个数
    * get the number of elements in list specified the given key
    * @param key {const char*} 列表对象的 key
    *  the list's key
    * @return {int} 返回指定列表对象的长度（即元素个数）， -1 if error happened
    *  return the number of elements in list, -1 if error
    */
    virtual int llen(const std::string& key, std::string& error) = 0;

public:
    /*将一个或多个值元素插入到分片list列表的表尾
    **当topic不存在时，将自动初始化后插入
    **slotindex:返回插入的分片索引,允许填null
    *@return
    ** >0:返回添加完后当前列表对象中的元素个数
    ** <0:表示出错
    */
    virtual int slicelist_rpush(const std::string& topic, const int slicenum, const std::string& buf, uint32_t* slotindex, std::string& error) = 0;
    
    /*轮循从有数据的分片list列表对象中移除并返回头部元素-如果为空，尝试取下一个，最多重试N次
    **当topic不存在时，将自动初始化后消费
    **slotindex:返回弹出数据的分片索引,允许填null
    **@return
    ** >0:表示成功弹出一个元素且返回值表示元素的长度，
    ** <0:表示出错，或该对象非列表对象，或该对象已经为空
    */
    virtual int slicelist_lpop(const std::string& topic, const int slicenum, std::string& buf, uint32_t* slotindex, std::string& error) = 0;

    /*轮循从有数据的分片list列表对象中移除并返回头部元素-如果为空，尝试取下一个，最多重试N次
    **当topic不存在时，将自动初始化后消费
    **batchsize:一次性获取的最大个数,填1~1000
    **slotindex:返回弹出数据的分片索引,允许填null
    **@return
    ** 0:成功
    ** <0:表示出错，或该对象非列表对象，或该对象已经为空
    */
    virtual int slicelist_batchlpop(const std::string& topic, const int slicenum, int batchsize, std::vector<std::string>& buflist, uint32_t* slotindex, std::string& error) = 0;


    /*获取分片list列表对象的元素总个数
    **当topic不存在时，将自动初始化后插入
    **@return
    ** <0: 表示出错，不可用
    ** >=0:所有分片list元素数量总和
    */
    virtual int slicelist_totallen(const std::string& topic, const int slicenum, std::string& error) = 0;

    //获取指定分片的元素个数，索引从0开始，区间：[0, slicenum)
    virtual int slicelist_len(const std::string& topic, const int slicenum, const uint32_t& slotindex, std::string& error) = 0;

    //依次查找[0, slicenum)分区,直到该区间[start, end]内有数据返回
    virtual int slicelist_lrange(
        const std::string& topic,
        const int slicenum,
        const uint32_t& start,
        const uint32_t& end,
        std::vector<std::string>& result,
        std::string& error) = 0;
public:
    //////////////////////////////////////////////////////////////////////////
    //sets

    /**
    * 将一个或多个 member 元素加入到集合 key 当中，已经存在于集合的 member 元素
    * 将被忽略;
    * 1) 假如 key 不存在，则创建一个只包含 member 元素作成员的集合
    * 2) 当 key 不是集合类型时，返回一个错误
    * add one or more members to a set stored at a key
    * 1) if the key doesn't exist, a new set by the key will be created,
    *    and add the members to the set
    * 2) if the key exists and not a set's key, then error happened
    * @param key {const char*} 集合对象的键
    *  the key of a set
    * @param first_member {const char*} 第一个非 NULL 的成员
    *  the first member of a variable args which isn't NULL, the last
    *  arg of the args must be NULL indicating the end of args
    * @return {int} 被添加到集合中的新元素的数量，不包括被忽略的元素
    *  the number of elements that were added to the set, not including
    *  all the elements already present into the set. -1 if error
    *  happened or it isn't a set stored by the key.
    */
    virtual int sadd(const std::string& key, const std::vector<std::string>& members, std::string& error) = 0;
    
    /**
    * 从集合对象中随机移除并返回某个成员
    * remove and get one member from the set
    * @param key {const char*} 集合对象的键
    *  the key of the set
    * @param buf {string&} 存储被移除的成员
    *  store the member removed from the set
    * @return 失败 -1
      成功:0
    */
    virtual int spop(const std::string& key, std::string& member, std::string& error) = 0;

    /*
    * 返回集合 key 中的所有成员
    * get all the members in a set stored at a key
    * @param key {const char*} 集合对象的键值
    *  the key of the set
    * @param members {std::vector<std::string>*} 非空时存储结果集
    *  if not NULL, it will store the members.
    * @return {int} 结果集数量，返回 <0 表示出错或有一个 key 非集合对象
    */
    virtual int smembers(const std::string& key, std::vector<std::string>& result, std::string& error) = 0;

    /**
    * 移除集合 key 中的一个或多个 member 元素，不存在的 member 元素会被忽略
    * Remove the specified members from the set stored at key. if the
    * member doesn't exist, it will be ignored.
    * @param key {const char*} 集合对象的键值
    *  the key of the set
    * @param first_member {const char*} 需要被移除的成员列表的第一个非 NULL成员，
    *  在变参的输入中需要将最后一个变参写 NULL
    *  the first non-NULL member to be removed in a variable member list,
    *  and the last one must be NULL indicating the end of the list.
    * @retur {int} 被移除的成员元素的个数，当出错或非集合对象时返回 <0；当 key 不
    *  存在或成员不存在时返回 0
    *  the number of members be removed, 0 if the set is empty or the
    *  key doesn't exist, -1 if error happened or it's not a set by key
    */
    virtual int srem(const char* key, const std::vector<std::string>& members, std::string& error) = 0;

    /**
    * 判断 member 元素是否集合 key 的成员
    * determine if a given value is a member of a set
    * @return {int} 返回 0 表示是，否则可能是因为不是或出错或该 key 对象
    *  非集合对象
    */
    virtual int sismember(const std::string& key, const std::string& member, std::string& error) = 0;


    /**
    * 获得集合对象中成员的数量
    * get the number of members in a set stored at the key
    * @param key {const char*} 集合对象的键
    *  the key of the set
    * @return {int} 返回该集合对象中成员数量，含义如下：
    *  return int value as below:
    *  <0：出错或非集合对象
    *      error or it's not a set by the key
    *   0：成员数量为空或该 key 不存在
    *      the set is empty or the key doesn't exist
    *  >0：成员数量非空
    *      the number of members in the set
    */
    virtual int scard(const char* key, std::string& error) = 0;

public:
    //////////////////////////////////////////////////////////////////////////
    //zset(sorted sets)

    /**
    * 添加对应 key 的有序集
    * add one or more members to a sorted set, or update its score if
    * it already exists
    * @param key {const char*} 有序集键值
    *  the key of a sorted set
    * @param members "分值-成员"集合
    *  the set storing values and stores
    * @return {int} 新成功添加的 "分值-成员" 对的数量
    *  the number of elements added to the sorted set, not including
    *  elements already existing for which the score was updated
    *  0：表示一个也未添加，可能因为该成员已经存在于有序集中
    *     nothing was added to the sorted set
    * <0：表示出错或 key 对象非有序集对象
    *     error or it was not a sorted set by the key
    * >0：新添加的成员数量
    *     the number of elements added
    */
    virtual int zadd(const std::string& key, const std::string& item, double score, std::string& error) = 0;
    virtual int zadd(const std::string& key, const std::map<std::string, double>& items, std::string& error) = 0;

    /**
    * 从有序集中删除某个成员
    * @param key {const char*} 有序集键值
    * @param members 要删除的成员列表
    * @return {int} 成功删除的成员的数量，-1 表示出错或该 key 非有序集对象，
    *  0 表示该有序集不存在或成员不存在，> 0 表示成功删除的成员数量
    */
    virtual int zrem(const char* key, const std::vector<std::string>& members, std::string& error) = 0;
    virtual int zremrangebyscore(const char* key, const char* min, const char* max, std::string& error) = 0;

    /**
    * 获得相应键的有序集的成员数量
    * get the number of elements in a sorted set
    * @param key {const char*} 有序集键值
    *  the key of a a sorted set
    * @return {int} 一个键的有序集的成员数量
    *  the number of elements of the sorted set
    *   0：该键不存在
    *      the key doesn't exist
    *  <0：出错或该键的数据对象不是有效的有序集对象
    *      error or it wasn't a sorted set by the key
    *  >0：当前键值对应的数据对象中的成员个数
    *      the number of elements in the sorted set
    */
    virtual int zcard(const char* key, std::string& error) = 0;

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
    virtual int zcount(const char* key, double min, double max, std::string& error) = 0;


    /**
    * 从 key 的有序集中获得指定位置区间的成员名列表，成员按分值递增方式排序
    * get the specified range memebers of a sorted set sotred at key
    * @param key {const char*} 有序集键值
    *  the key of a sorted set
    * @param start {int} 起始下标位置
    *  the begin index of the sorted set
    * @param stop {int} 结束下标位置（结果集同时含该位置）
    *  the end index of the sorted set
    * @param result {std::vector<string>*} 非空时存储结果集，内部先调用
    *  result.clear() 清除其中的元素
    *  if not NULL, it will store the memebers result
    * @return {int} 结果集中成员的数量
    *  the number of memebers
    *  0: 表示结果集为空或 key 不存在
    *     the result is empty or the key doesn't exist
    * <0: 表示出错或 key 对象非有序集对象
    *     error or it's not a sorted set by the key
    * >0: 结果集的数量
    **/
    virtual int zrange(const std::string& key, int start, int stop,
        std::vector<std::string>& result, std::string& error) = 0;

    /**
    * 返回有序集 key 中，所有 score 值介于 min 和 max 之间(包括等于 min 或 max )
    * 的成员。有序集成员按 score 值递增(从小到大)次序排列
    * @param key {const char*} 有序集键值
    * @param min {double} 最小分值
    * @param max {double} 最大分值
    * @param out {std::vector<string>*} 非空时存储“成员名”结果集
    * @return {int} 结果集中成员的数量
    *  0: 表示结果集为空或 key 不存在
    * <0: 表示出错或 key 对象非有序集对象
    * >0: 结果集的数量
    **/
    virtual int zrangebyscore_with_scores(const char* key, double min, double max,
        std::vector<std::pair<std::string, double> >& out, std::string& error) = 0;

    virtual int zrangebyscore_with_scores(const char* key, const char* min, const char* max,
        std::vector<std::pair<std::string, double> >& out, std::string& error) = 0;


    /**
    * 获得有序集 key 中，成员 member 的 score 值
    * @param key {const char*} 有序集键值
    * @param member {const char*} 成员名
    * @param len {size_t} member 的长度
    * @param result {double&} 存储分值结果
    * @return 当不存在或出错时<0，成功返回 0
    */
    virtual int zscore(const char* key, const char* member, size_t len,
        double& result, std::string& error) = 0;

public:
    /************************************************************************
    *  功能:set/setex
    *  参数:key：key value:value error:错误描述
    *  timeout:过期值，单位为秒
    *  @return true:成功; false:失败
    ************************************************************************/
    virtual bool set(const std::string& key, const std::string& value, std::string& error) = 0;
    virtual bool setex(const std::string& key, const std::string& value, const int& timeout, std::string& error) = 0;

    /************************************************************************
    *  功能:get
    *  参数:key：key value:返回的value
    *  @return true:成功; false:失败
    ************************************************************************/
    virtual bool get(const std::string& key, std::string& value, std::string& error) = 0;

public:
    /************************************************************************
    * 将 key 改名为 newkey
    * rename a key
    * @return {bool}
    *  true on success, or error happened
    ************************************************************************/
    virtual bool rename_key(const char* key, const char* newkey, std::string& error) = 0;

    /************************************************************************
    * 设置 KEY 的生存周期，单位（毫秒）
    * set a key's time to live in milliseconds
    * @param key {const char*} 键值
    *  the key
    * @param n {int} 生存周期（毫秒）
    *  time to live in milliseconds
    * @return {int} 返回值含义如下：
    *  value returned as below:
    *  > 0: 成功设置了生存周期
    *       set successfully
    *    0：该 key 不存在
    *       the key doesn't exist
    *  < 0: 出错
    *       error happened
    ************************************************************************/
    virtual int pexpire(const char* key, int n, std::string& error) = 0;


    /**
    * 设置 KEY 的生存周期，单位（秒）
    * set a key's time to live in seconds
    * @param key {const char*} 键值
    *  the key
    * @param n {int} 生存周期（秒）
    *  lief cycle in seconds
    * @return {int} 返回值含义如下：
    *  return value as below:
    *  > 0: 成功设置了生存周期
    *       set successfully
    *  0：该 key 不存在
    *    the key doesn't exist
    *  < 0: 出错
    *       error happened
    */
    virtual int expire(const char* key, int n, std::string& error) = 0;

    /************************************************************************
    * 判断 KEY 是否存在
    * check if the key exists in redis
    * @param key {const char*} KEY 值
    *  the key
    * @return {bool} 返回 true 表示存在，否则表示出错或不存在
    *  true returned if key existing, false if error or not existing
    ************************************************************************/
    virtual bool exists(const char* key, std::string& error) = 0;


    /************************************************************************
    * 删除一组 KEY，对于变参的接口，则要求最后一个参数必须以 NULL 结束
    * delete one or some keys from redis, for deleting a variable
    * number of keys, the last key must be NULL indicating the end
    * of the variable args
    * @return {int} 返回所删除的 KEY 的个数，如下：
    *  0: 未删除任何 KEY
    *  -1: 出错
    *  >0: 真正删除的 KEY 的个数，该值有可能少于输入的 KEY 的个数
    *  return the number of keys been deleted, return value as below:
    *  0: none key be deleted
    * -1: error happened
    *  >0: the number of keys been deleted
    *
    ************************************************************************/
    virtual int del(const std::vector<std::string>& keys, std::string& error) = 0;

    /**
    * 将 key 所储存的值加上增量 increment
    * 1）如果 key 不存在，那么 key 的值会先被初始化为 0 ，然后再执行 INCRBY 命令
    * 2）如果值包含错误的类型，或字符串类型的值不能表示为数字，那么返回一个错误
    * 3）本操作的值限制在 64 位(bit)有符号数字表示之内
    * increment the integer value of a key by a given amount
    * 1) if key not exists, the key's value will be set 0 and INCRBY
    * 2) if key's value is not a number an error will be returned
    * 3) the number is a 64 signed integer
    * @param key {const char*} 字符串对象的 key
    *  the given key
    * @param inc {long long int} 增量值
    *  the given amount
    * @param result {long long int*} 非空时存储操作结果
    *  store the result after INCR if it isn't NULL
    * @return {bool} 操作是否成功
    *  if the INCRBY was executed correctly
    */
    virtual bool incrby(const std::string& key, const long long int& inc, long long int* result, std::string& error) = 0;

public:
    /************************************************************************
    *  功能:批量排序队列入栈
    *  参数:@key:队列的名称
    *       @item:新增加项
    *       @score:分值
    *  @return
    *  0：表示一个也未添加，可能因为该成员已经存在于有序集中
    *  nothing was added to the sorted set
    *  <0：表示出错或 key 对象非有序集对象
    *     error or it was not a sorted set by the key
    *  >0：新添加的成员数量
    *     the number of elements added
    ************************************************************************/
    virtual int zpush_queue(const std::string& key,
        const std::string& item,
        double score,
        std::string& error) = 0;


    /************************************************************************
    *  功能:批量排序队列出栈
    *  参数:@key:队列的名称
    *  范围:[@minpos, @maxpos],[-inf, +inf]表示无穷小(无限制),无穷大(无限制)
    *  @batchsize:一次性获取的最大个数,填1~1000
    *  @outlist 返回的数据，格式为：item1,score1,item2,score2,itemN,scoreN...
    *  即:outlist[0]=item, outlist[1]=score
    *  @return >=0:成功; <0:错误码
    ************************************************************************/
    virtual int zpop_queue(const std::string& key,
        const std::string& minpos,
        const std::string& maxpos,
        const int& batchsize,
        std::vector<std::string>& outlist,
        std::string& error) = 0;


public:
    /************************************************************************
    *  功能:批量从list队列出栈
    *  参数:@key:队列的名称
    *  @batchsize:一次性获取的最大个数,填1~1000
    *  @outlist 返回的数据列表
    *  @return 0:成功; <0:错误码
    ************************************************************************/
    virtual int zlpop_list(const std::string& key, const int& batchsize, std::vector<std::string>& outlist, std::string& error) = 0;

public:
    
    /************************************************************************
    *  功能:执行lua脚本
    *  参数:@script:lua脚本
    *       @keys:操作的redis key项
    *       @args:参数列表
    *       @out:lua脚本返回值
    *  @return >=0:成功; <0:错误码
    ************************************************************************/
    virtual int eval_number(const char* script,
        const std::vector<std::string>& keys,
        const std::vector<std::string>& args,
        int& out,
        std::string& error) = 0;

    virtual int eval_strings(const char* script,
        const std::vector<std::string>& keys,
        const std::vector<std::string>& args,
        std::vector<std::string>& out,
        std::string& error) = 0;
};



/************************************************************************/
/*  MWRedisClient 类工厂                                                */
/************************************************************************/
class MWRedisClientFactory
{
public:
    static MWRedisClient* New();
    static void Destroy(MWRedisClient* pBase);
};

#endif // __MWREDISLIB_H__