#include <assert.h>
#include "acl_stdafx.hpp"
#include "mwredislib.h"
#include "mwredisslicelist.h"

MWRedisSliceList::MWRedisSliceList(std::shared_ptr<MWRedisClient> pRedisClient) {
    assert(nullptr != pRedisClient);
    m_pRedisClient = pRedisClient;
    m_list_slicenum.store(1);
    m_last_pop_index.store(0);
    m_last_push_index.store(0);
}

MWRedisSliceList::~MWRedisSliceList() { }


int MWRedisSliceList::listinit(const std::string& topic, const int slicenum, std::string& error)
{
    assert(!topic.empty());
    assert(slicenum > 0);
    if (topic.empty() || slicenum <= 0) {
        error = "invalid parameter";
        return -1;
    }

    auto pRedisClient = m_pRedisClient.lock();
    if (nullptr == pRedisClient)  {
        error = "m_pRedisClient is nullptr";
        return -2;
    }

    m_last_push_index.store(0);
    m_last_pop_index.store(0);
    m_list_slicenum.store(slicenum);
    m_topic = topic;
    return 0;
}


int MWRedisSliceList::getlisttopic(std::string& topic)
{
    topic = m_topic;
    return 0;
}

int MWRedisSliceList::getlistslicenum(int& slicenum)
{
    slicenum = m_list_slicenum;
    return 0;
}

int MWRedisSliceList::listunint()
{
    //nothing to do here...
    return 0;
}

int MWRedisSliceList::is_available(std::string& reason)
{
    auto pRedisClient = m_pRedisClient.lock();
    assert(nullptr != pRedisClient);
    if (nullptr == pRedisClient) {
        reason = "m_pRedisClient is nullptr";
        return -1;
    }

    if (false == pRedisClient->ping()) {
        reason = "failed to ping";
        return -2;
    }
    return 0;
}

int MWRedisSliceList::list_totallen(std::string& error)
{
    auto pRedisClient = m_pRedisClient.lock();
    assert(nullptr != pRedisClient);
    if (nullptr == pRedisClient) {
        error = "m_pRedisClient is nullptr";
        return -1;
    }

    //获取所有分片数量总和
    uint32_t slicenum = m_list_slicenum;
    int len = 0;
    int totallen = 0;
    for (uint32_t index = 0; index < slicenum; index++) {
        std::string key = m_topic + ":" + std::to_string(index);
        len = pRedisClient->llen(key, error);
        if (len < 0) {
            return len;
        }
        totallen += len;
    }
    return totallen;
}

int MWRedisSliceList::list_len(const uint32_t& slotindex, std::string& error)
{
    auto pRedisClient = m_pRedisClient.lock();
    assert(nullptr != pRedisClient);
    if (nullptr == pRedisClient) {
        error = "m_pRedisClient is nullptr";
        return -1;
    }

    if (slotindex >= m_list_slicenum) {
        return -2;
    }

    std::string key = m_topic + ":" + std::to_string(slotindex);
    return pRedisClient->llen(key, error);
}

int MWRedisSliceList::listrpush(const std::string& buf, uint32_t* slotindex, std::string& error)
{
    assert(!m_topic.empty());
    assert(!buf.empty());
    if (m_topic.empty() || buf.empty()) {
        error = "invalid parameter";
        return -1;
    }

    auto pRedisClient = m_pRedisClient.lock();
    assert(nullptr != pRedisClient);
    if (nullptr == pRedisClient) {
        error = "m_pRedisClient is nullptr";
        return -2;
    }

    //轮循
    uint64_t index = m_last_push_index++ % m_list_slicenum;
    std::string key = m_topic + ":" + std::to_string(index);
    if (nullptr != slotindex) {
        *slotindex = static_cast<uint32_t>(index);
    }

    std::vector<std::string> values = { buf };
    return pRedisClient->rpush(key.c_str(), values, error);
}


int MWRedisSliceList::listrpush(const std::vector<std::string>& values, uint32_t* slotindex, std::string& error)
{
    assert(!m_topic.empty());
    assert(!values.empty());
    if (m_topic.empty() || values.empty()) {
        error = "invalid parameter";
        return -1;
    }

    auto pRedisClient = m_pRedisClient.lock();
    assert(nullptr != pRedisClient);
    if (nullptr == pRedisClient) {
        error = "m_pRedisClient is nullptr";
        return -2;
    }

    //轮循
    uint64_t index = m_last_push_index++ % m_list_slicenum;
    std::string key = m_topic + ":" + std::to_string(index);
    if (nullptr != slotindex) {
        *slotindex = static_cast<uint32_t>(index);
    }
    return pRedisClient->rpush(key.c_str(), values, error);
}

int MWRedisSliceList::list_rpush_by_index(const uint32_t& slotindex, const std::string& buf, std::string& error)
{
    assert(!m_topic.empty());
    assert(!buf.empty());
    if (m_topic.empty() || buf.empty()) {
        error = "invalid parameter";
        return -1;
    }

    auto pRedisClient = m_pRedisClient.lock();
    assert(nullptr != pRedisClient);
    if (nullptr == pRedisClient) {
        error = "m_pRedisClient is nullptr";
        return -2;
    }

    if (slotindex >= m_list_slicenum) {
        error = "index is out of range";
        return -3;
    }

    std::string key = m_topic + ":" + std::to_string(slotindex);
    std::vector<std::string> values = { buf };
    return pRedisClient->rpush(key.c_str(), values, error);
}


int MWRedisSliceList::list_rpush_by_index(const uint32_t& slotindex, const std::vector<std::string>& values, std::string& error)
{
    assert(!m_topic.empty());
    assert(!values.empty());
    if (m_topic.empty() || values.empty()) {
        error = "invalid parameter";
        return -1;
    }

    auto pRedisClient = m_pRedisClient.lock();
    assert(nullptr != pRedisClient);
    if (nullptr == pRedisClient) {
        error = "m_pRedisClient is nullptr";
        return -2;
    }

    if (slotindex >= m_list_slicenum) {
        error = "index is out of range";
        return -3;
    }

    std::string key = m_topic + ":" + std::to_string(slotindex);
    return pRedisClient->rpush(key.c_str(), values, error);
}


int MWRedisSliceList::listlpop(std::string& buf, uint32_t* slotindex, std::string& error)
{
    assert(!m_topic.empty());
    if (m_topic.empty()) {
        error = "invalid parameter";
        return -1;
    }

    auto pRedisClient = m_pRedisClient.lock();
    assert(nullptr != pRedisClient);
    if (nullptr == pRedisClient) {
        error = "m_pRedisClient is nullptr";
        return -2;
    }

    int ret = 0;
    uint64_t start_index = (m_last_pop_index++) % m_list_slicenum;
    uint64_t index = start_index;
    do
    {
        //轮循查找
		index = (index + 1) % m_list_slicenum;
		std::string key = m_topic + ":" + std::to_string(index);
        ret = pRedisClient->lpop(key.c_str(), buf, error);
        if (ret > 0) { //取到了数据
            if (nullptr != slotindex){
				*slotindex = static_cast<uint32_t>(index);
            }
            break; 
        }
        else if (ret < 0) {
            break;
        }

		if (index == start_index) { //所有list都取不到数据
            ret = 0;
            break;
        }
    } while (1);
    
    return ret;
}


int MWRedisSliceList::listbatchlpop(int batchsize, std::vector<std::string>& buflist, uint32_t* slotindex, std::string& error)
{
    assert(!m_topic.empty());
    if (m_topic.empty() || batchsize < 1) {
        error = "invalid parameter";
        return -1;
    }

    auto pRedisClient = m_pRedisClient.lock();
    assert(nullptr != pRedisClient);
    if (nullptr == pRedisClient) {
        error = "m_pRedisClient is nullptr";
        return -2;
    }

    int ret = 0;
    uint64_t start_index = (m_last_pop_index++) % m_list_slicenum;
    uint64_t index = start_index;
    do
    {
        //轮循查找
        index = (index + 1) % m_list_slicenum;
        std::string key = m_topic + ":" + std::to_string(index);
        ret = pRedisClient->zlpop_list(key.c_str(), batchsize, buflist, error);
        if (ret < 0) {
            break;
        }
        else if (0 == ret) { //执行成功

            if (!buflist.empty()) //取到了数据
            {
                if (nullptr != slotindex){
                    *slotindex = static_cast<uint32_t>(index);
                }
                break;  
            }
        }

        if (index == start_index) { //所有list都取不到数据
            ret = 0;
            break;
        }
    } while (1);

    return ret;
}


int MWRedisSliceList::list_robin_lpop(std::string& buf, uint32_t* slotindex, std::string& error)
{
    assert(!m_topic.empty());
    if (m_topic.empty()) {
        error = "invalid parameter";
        return -1;
    }

    auto pRedisClient = m_pRedisClient.lock();
    assert(nullptr != pRedisClient);
    if (nullptr == pRedisClient) {
        error = "m_pRedisClient is nullptr";
        return -2;
    }

    //轮循
    uint64_t index = (m_last_pop_index++) % m_list_slicenum;
    std::string key = m_topic + ":" + std::to_string(index);
    if (nullptr != slotindex){
        *slotindex = static_cast<uint32_t>(index);
    }
    return pRedisClient->lpop(key.c_str(), buf, error);
}


int MWRedisSliceList::list_lpop_by_index(const uint32_t& slotindex, std::string& buf, std::string& error)
{
    assert(!m_topic.empty());
    if (m_topic.empty()) {
        error = "invalid parameter";
        return -1;
    }

    auto pRedisClient = m_pRedisClient.lock();
    assert(nullptr != pRedisClient);
    if (nullptr == pRedisClient) {
        error = "m_pRedisClient is nullptr";
        return -2;
    }

    if (slotindex >= m_list_slicenum) {
        error = "index is out of range";
        return -3;
    }

    std::string key = m_topic + ":" + std::to_string(slotindex);
    return pRedisClient->lpop(key.c_str(), buf, error);
}

int MWRedisSliceList::list_lrange_by_index(
    const uint32_t& slotindex,
    const uint32_t& start,
    const uint32_t& end,
    std::vector<std::string>& result,
    std::string& error)
{
    assert(!m_topic.empty());
    if (m_topic.empty() || 
        start <= end) {
        error = "invalid parameter";
        return -1;
    }

    auto pRedisClient = m_pRedisClient.lock();
    assert(nullptr != pRedisClient);
    if (nullptr == pRedisClient) {
        error = "m_pRedisClient is nullptr";
        return -2;
    }

    if (slotindex >= m_list_slicenum) {
        error = "index is out of range";
        return -3;
    }

    std::string key = m_topic + ":" + std::to_string(slotindex);
    return pRedisClient->lrange(key.c_str(), 
        start,
        end,
        result,
        error);
}


int MWRedisSliceList::list_lrange(
    const uint32_t& start,
    const uint32_t& end,
    std::vector<std::string>& result,
    std::string& error)
{
    assert(!m_topic.empty());
    if (m_topic.empty() ||
        start <= end) {
        error = "invalid parameter";
        return -1;
    }

    auto pRedisClient = m_pRedisClient.lock();
    assert(nullptr != pRedisClient);
    if (nullptr == pRedisClient) {
        error = "m_pRedisClient is nullptr";
        return -2;
    }

    int ret = 0;
    for (uint32_t slotindex = 0; slotindex < m_list_slicenum; slotindex++) 
    {
        std::string key = m_topic + ":" + std::to_string(slotindex);
        ret = pRedisClient->lrange(key.c_str(),
            start,
            end,
            result,
            error);

        if (ret < 0) { //error
            return ret;
        }

        if (ret >= 0 && !result.empty()) {
            break;
        }
    }
    return ret;
}

