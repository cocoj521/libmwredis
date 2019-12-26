/*
************************************************************************
*
* Description : 分片list存储接口
*
* Created Date : 2019 / 10 / 15
*
* Author : Peter Hu
*
* Copyright(c) ShenZhen Montnets Technology, Inc.All rights reserved.
*
************************************************************************
*/

#ifndef __MWREDISSLICELIST_H__
#define __MWREDISSLICELIST_H__

#include <memory>
#include <vector>
#include <atomic>
#include "mwredislib.h"

class MWRedisSliceList
{
public:
    MWRedisSliceList(std::shared_ptr<MWRedisClient> pRedisClient);
    ~MWRedisSliceList();

public:
    /*初始化分片list列表，同一个分片内的数据是先进先出规则
    **topic:list的名称
    **shardnum:分片的个数，一般为redis集群总数量
    **@return
    ** <0: 表示出错
    ** 0:成功 
    */
    int listinit(const std::string& topic, const int slicenum, std::string& error);
    int getlisttopic(std::string& topic);
    int getlistslicenum(int& slicenum);


    /**反初始化
    **@return
    ** <0:表示出错
    ** 0:成功
    */
    int listunint();


    /*检查分片list列表状态
    **@return 
    ** <0: 表示出错，不可用
    ** 0:可用
    */
    int is_available(std::string& reason);


    /*获取分片list列表对象的元素总个数
    **@return
    ** <0: 表示出错，不可用
    ** >=0:所有分片list元素数量总和
    */
    int list_totallen(std::string& error);


public:
    /*将一个或多个值元素插入到分片list列表的表尾
    **slotindex:返回插入的分片索引,允许填null
    *@return
    ** >0:返回添加完后当前列表对象中的元素个数
    ** <0:表示出错
    */
    int listrpush(const std::string& buf, uint32_t* slotindex, std::string& error);
    int listrpush(const std::vector<std::string>& values, uint32_t* slotindex, std::string& error);

    //将一个或多个值元素插入到指定的分片list列表的表尾,slotindex索引从0开始,取值范围：[0,N)
    int list_rpush_by_index(const uint32_t& slotindex, const std::string& buf, std::string& error);
    int list_rpush_by_index(const uint32_t& slotindex, const std::vector<std::string>& values, std::string& error);

public:
    /*轮循从有数据的分片list列表对象中移除并返回头部元素-如果为空，尝试取下一个，最多重试N次
    **slotindex:返回弹出数据的分片索引,允许填null
    **@return
    ** >0:表示成功弹出一个元素且返回值表示元素的长度，
    ** <0:表示出错，或该对象非列表对象，或该对象已经为空
    */
    int listlpop(std::string& buf, uint32_t* slotindex, std::string& error);
    //轮循从分片list列表对象中移除并返回头部元素-重试0次
    int list_robin_lpop(std::string& buf, uint32_t* slotindex, std::string& error);
    //从指定索引分区移除并返回头部元素,slotindex索引从0开始,取值范围：[0,N)
    int list_lpop_by_index(const uint32_t& slotindex, std::string& buf, std::string& error);

private:
    std::atomic<uint64_t> m_last_push_index;
    std::atomic<uint64_t> m_last_pop_index;
    std::atomic<uint32_t> m_list_slicenum;
    std::weak_ptr<MWRedisClient> m_pRedisClient;
    std::string m_topic;
};


#endif // !__MWREDISSLICELIST_H__
