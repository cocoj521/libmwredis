//#include <StdAfx.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include "mredisdb.h"


MRedisDB::MRedisDB()  { }

MRedisDB::~MRedisDB() { }

//初始化组件
int MRedisDB::InitRedisDB(const std::string& strRedisCluster,
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
    assert(nullptr == m_pRedisClient);
    m_pRedisClient = MWRedisClientFactory::New();
    assert(nullptr != m_pRedisClient);
    if (nullptr == m_pRedisClient) {
        return -1;
    }

    return m_pRedisClient->initRedis(strRedisCluster,
        strRedisPwd,
        nConnTimeout,
        nRWTimeout,
        nRetryLimit,
        nRetryInterval,
        nRetrySleep,
        nPoolSize,
        bPreset,
        bStdoutOpen);
}


//反初始化
void MRedisDB::DestoryRedisDB()
{
    assert(nullptr != m_pRedisClient);
    if (m_pRedisClient) {
        m_pRedisClient->unintRedis();
        MWRedisClientFactory::Destroy(m_pRedisClient);
        m_pRedisClient = nullptr;
    }
}


/************************************************************************
*  功能:插入富信下行消息缓存
*  参数:rmsMsgid：富信流水号 rmsDetail:待插入的富信下行消息缓存 error:错误描述
*  @return 0:成功; 非0:错误码
************************************************************************/
int MRedisDB::InsertMtRms(const std::string& rmsMsgid, const mt_cache_rms& rmsDetail, std::string& error)
{
    assert(nullptr != m_pRedisClient);
    if (!m_pRedisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgid.empty());
    if (rmsMsgid.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_mtrms:{%s}", rmsMsgid.c_str());

    std::map<std::string, std::string> properties;
    properties["tmpl_id"] = rmsDetail.tmpl_id;
    properties["msg_type"] = rmsDetail.msg_type;
    properties["tmpl_type"] = rmsDetail.tmpl_type;
    properties["sms_msgid"] = rmsDetail.sms_msgid;
    properties["mt_exdata"] = rmsDetail.mt_exdata;
    properties["reserve"] = rmsDetail.reserve;

    properties["chk_expflg"] = rmsDetail.chk_expflg;
    properties["supp_privilige"] = rmsDetail.supp_privilige;
    properties["supp_sms_state"] = rmsDetail.supp_sms_state;
    properties["send_notice_rpt"] = rmsDetail.send_notice_rpt;
    properties["send_dld_rpt"] = rmsDetail.send_dld_rpt;
    return m_pRedisClient->hmset(redisKey, properties, error);
}


/************************************************************************
*  功能:查询富信下行消息缓存
*  参数:rmsMsgid：富信流水号 rmsDetail:返回的富信下行消息缓存
*  @return 0:成功; -3:数据不存在 其它:错误码
************************************************************************/
int MRedisDB::QueryMtRms(const std::string& rmsMsgid, mt_cache_rms& rmsDetail, std::string& error)
{
    assert(nullptr != m_pRedisClient);
    if (!m_pRedisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgid.empty());
    if (rmsMsgid.empty()) {
        error = "invalid parameter";
        return -2;
    }

    std::vector<std::string> names = { "tmpl_id", "msg_type", "tmpl_type", "sms_msgid", "chk_expflg",
        "supp_privilige", "supp_sms_state", "mt_exdata", "reserve", "send_notice_rpt", "send_dld_rpt"
    };

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_mtrms:{%s}", rmsMsgid.c_str());

    std::map<std::string, std::string> properties;
    int ret = m_pRedisClient->hmget(redisKey, names, properties, error);
    if (0 != ret) {
        return ret;
    }

    if (properties.empty()) {
        error = "data is empty";
        return -3;
    }
    
    auto pos = properties.find("tmpl_id");
    assert(pos != properties.end());
    if (pos != properties.end()) { 
        rmsDetail.tmpl_id = properties["tmpl_id"]; 
    }

    pos = properties.find("msg_type");
    assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.msg_type = properties["msg_type"];
    }

    pos = properties.find("tmpl_type");
    assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.tmpl_type = properties["tmpl_type"];
    }

    pos = properties.find("sms_msgid");
    assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.sms_msgid = properties["sms_msgid"];
    }

    pos = properties.find("chk_expflg");
    assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.chk_expflg = properties["chk_expflg"];
    }

    pos = properties.find("supp_privilige");
    assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.supp_privilige = properties["supp_privilige"];
    }

    pos = properties.find("supp_sms_state");
    assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.supp_sms_state = properties["supp_sms_state"];
    }

    pos = properties.find("mt_exdata");
    assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.mt_exdata = properties["mt_exdata"];
    }

    pos = properties.find("reserve");
    assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.reserve = properties["reserve"];
    }

    pos = properties.find("send_notice_rpt");
    assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.send_notice_rpt = properties["send_notice_rpt"];
    }

    pos = properties.find("send_dld_rpt");
    assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.send_dld_rpt = properties["send_dld_rpt"];
    }

    return 0;
}


/************************************************************************
*  功能:"缓存中富信失败业务网关已补发短信状态"与cmpSmsState相等则更新；否则不更新 
*  参数:rmsMsgid:富信流水号 cmpSmsState:待比较的状态 newSuppSmsState:设置的新状态
*  @return 0:未更新; 1:已更新; 其它:错误码
************************************************************************/
int MRedisDB::CheckAndSetSuppedSmsState(const std::string& rmsMsgid, const std::string& cmpSmsState, const std::string& newSuppSmsState, std::string& error)
{
    assert(nullptr != m_pRedisClient);
    if (!m_pRedisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }
    assert(!rmsMsgid.empty());
    assert(!cmpSmsState.empty());
    assert(!newSuppSmsState.empty());
    if (rmsMsgid.empty() || cmpSmsState.empty() || newSuppSmsState.empty()) {
        error = "invalid parameter";
        return -2;
    }

    //https://gist.github.com/klovadis/5170446
    /* test.lua
    $./redis-cli -c -h 192.169.7.135 -p 7001 --eval test.lua person, 0 1
    local e = redis.call('HGETALL', KEYS[1]) 
    local result = {} 
    local nextkey 
    for i, v in ipairs(e) do 
    if i % 2 == 1 then 
    nextkey = v 
    else 
    result[nextkey] = v 
    end 
    end 
    if result['supp_sms_state'] == ARGV[1] then 
    redis.call('HMSET', KEYS[1], 'supp_sms_state', ARGV[2]) 
    return 1 
    end 
    return 0
    */

    std::string script = "local e = redis.call('HGETALL', KEYS[1]) \
        local result = {} \
        local nextkey \
        for i, v in ipairs(e) do \
            if i % 2 == 1 then \
                nextkey = v \
            else \
            result[nextkey] = v \
            end \
        end \
        if result['supp_sms_state'] == ARGV[1] then \
            redis.call('HMSET', KEYS[1], 'supp_sms_state', ARGV[2]) \
            return 1 \
        end \
        return 0";

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_mtrms:{%s}", rmsMsgid.c_str());

    std::vector<std::string> keys = { redisKey };
    std::vector<std::string> args = { cmpSmsState, newSuppSmsState };
    int out = 0;
    int ret = m_pRedisClient->eval_number(script.c_str(), keys, args, out, error);
    if (0 == ret) { //执行成功
        ret = out; //lua脚本返回值 0:未更新 1:已更新
    }
    return ret;
}



/************************************************************************
*  功能:标识已收到rdn下载报告
*  参数:rmsMsgid:富信流水号
*  @return 0:未更新; 1:已更新; 其它:错误码
************************************************************************/
int MRedisDB::MarkRecivedRdnDldRpt(const std::string& rmsMsgid, std::string& error)
{
    assert(nullptr != m_pRedisClient);
    if (!m_pRedisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }
    assert(!rmsMsgid.empty());
    if (rmsMsgid.empty()) {
        error = "invalid parameter";
        return -2;
    }


    //0x1:通知报告 0x2:下载报告
    //已收到rdn下载报告，去除0x2标志位，即：chk_expflg &=~0x2
    //$./redis-cli -c -h 192.169.7.135 -p 7001 --eval test.lua person , chk_expflg
    /*
    local e = redis.call('HMGET',KEYS[1],ARGV[1])
    if e[1]==nil then
        return 0
    end
    local a = tonumber(e[1])
    local b = bit.band(a,2)
    if b ~= 0 then
        local c = bit.bnot(2)
        local d = tostring(bit.band(a,c))
        redis.call('HMSET',KEYS[1],ARGV[1],d)
        return 1
    end
    return 0
    */

    std::string script = "local e=redis.call('HMGET',KEYS[1],ARGV[1]) if e[1]==nil then return 0 end local a=tonumber(e[1]); local b = bit.band(a,2);  if b~=0 then local c = bit.bnot(2) local d = tostring(bit.band(a, c)) redis.call('HMSET',KEYS[1],ARGV[1],d) return 1 end return 0";
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_mtrms:{%s}", rmsMsgid.c_str());

    std::vector<std::string> keys = { redisKey };
    std::vector<std::string> args = { "chk_expflg" };
    int out = 0;
    int ret = m_pRedisClient->eval_number(script.c_str(), keys, args, out, error);
    if (0 == ret) { //执行成功
        ret = out; //lua脚本返回值 0:未更新 1:已更新
    }
    return ret;
}


/************************************************************************
*  功能:设置有效期
*  参数:rmsMsgid:富信流水号 expireSec:有效期，单位：秒
*  @return >0:成功; 0：该key不存在;  <0: 出错
************************************************************************/
int MRedisDB::ExpireMtRms(const std::string& rmsMsgid, const int& expireSec, std::string& error)
{
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_mtrms:{%s}", rmsMsgid.c_str());
    return this->ExpireKey(redisKey, expireSec, error);
}



/************************************************************************
*  功能:设置是否已返回通知报告给用户
*  参数 noticeRptState:
*          "0"：未返回通知报告给用户
*          "1"：已返回成功通知报告给用户
*          "2"：已返回失败通知报告给用户
*  @return 0:成功; 非0:错误码
************************************************************************/
int MRedisDB::MarkSendNoticeRptState(const std::string& rmsMsgid, const std::string& noticeRptState, std::string& error)
{
    assert(nullptr != m_pRedisClient);
    if (!m_pRedisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }
    assert(!rmsMsgid.empty());
    assert(!noticeRptState.empty());
    if (rmsMsgid.empty() || noticeRptState.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_mtrms:{%s}", rmsMsgid.c_str());

    std::map<std::string, std::string> properties;
    properties["send_notice_rpt"] = noticeRptState;
    return m_pRedisClient->hmset(redisKey, properties, error);
}



/************************************************************************
*  功能:设置是否已返回下载报告给用户
*  参数 state:
*            "0"：未返回下载报告给用户
*            "1"：已返回成功下载报告给用户
*            "2"：已返回失败下载报告给用户
*  @return 0:成功; 非0:错误码
************************************************************************/
int MRedisDB::MarkSendDldRptState(const std::string& rmsMsgid, const std::string& dldRptState, std::string& error)
{
    assert(nullptr != m_pRedisClient);
    if (!m_pRedisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }
    assert(!rmsMsgid.empty());
    assert(!dldRptState.empty());
    if (rmsMsgid.empty() || dldRptState.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_mtrms:{%s}", rmsMsgid.c_str());

    std::map<std::string, std::string> properties;
    properties["send_dld_rpt"] = dldRptState;
    return m_pRedisClient->hmset(redisKey, properties, error);
}




/************************************************************************
*  功能:设置是否已返回通知报告和下载报告给用户
*  参数
*  noticeRptState:
*                "0"：未返回通知报告给用户
*                "1"：已返回成功通知报告给用户
*                "2"：已返回失败通知报告给用户
*  ,
*  dldRptState:
*              "0"：未返回通知报告给用户
*              "1"：已返回成功下载报告给用户
*              "2"：已返回失败下载报告给用户
*  @return 0:成功; 非0:错误码
************************************************************************/
int MRedisDB::MarkSendRptState(const std::string& rmsMsgid, const std::string& noticeRptState, const std::string& dldRptState, std::string& error)
{
    assert(nullptr != m_pRedisClient);
    if (!m_pRedisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgid.empty());
    assert(!noticeRptState.empty());
    assert(!dldRptState.empty());
    if (rmsMsgid.empty() || noticeRptState.empty() || dldRptState.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_mtrms:{%s}", rmsMsgid.c_str());

    std::map<std::string, std::string> properties;
    properties["send_notice_rpt"] = noticeRptState;
    properties["send_dld_rpt"] = dldRptState;
    return m_pRedisClient->hmset(redisKey, properties, error);
}




/************************************************************************
*  功能:插入富信流水号到RDN下载超时队列
*  参数:rmsMsgid:富信流水号
*  @return 0:成功; 非0:错误码
*********************************************ble value***************************/
int MRedisDB::InsertRmsExpireQueue(const std::string& rmsMsgid, const int timeoutSec, std::string& error)
{
    assert(nullptr != m_pRedisClient);
    if (!m_pRedisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }
    assert(!rmsMsgid.empty());
    assert(timeoutSec > 0);
    if (rmsMsgid.empty() || timeoutSec <=0) {
        error = "invalid parameter";
        return -2;
    }

    double score = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>
        (std::chrono::system_clock::now().time_since_epoch()).count() + timeoutSec);
    char* redisKey = "fj_rms_expirelist";
    return m_pRedisClient->zadd(redisKey, rmsMsgid, score, error);
}


/************************************************************************
*  功能:批量扫描RDN下载超时的富信流水号列表
*  参数:nBatchSize:一次性获取的最大个数,1~1000 rmsMsgidList:返回的超时的富信流水号列表
*  @return 0:成功; 非0:错误码
************************************************************************/
int MRedisDB::ScanRmsExpireQuque(const int nBatchSize, std::vector<std::string>& rmsMsgidList, std::string& error)
{
    assert(nullptr != m_pRedisClient);
    if (!m_pRedisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(nBatchSize > 0);
    if (nBatchSize <= 0) {
        error = "invalid parameter";
        return -2;
    }

    std::string exprirStart = "-inf";
    //std::string exprirEnd = "+inf";
    std::string exprirEnd = "+" + std::to_string(std::chrono::duration_cast<std::chrono::seconds>
        (std::chrono::system_clock::now().time_since_epoch()).count());

    //./redis-cli -c -h 192.169.7.135 -p 7001 
    //EVAL "local e=redis.call('ZRANGEBYSCORE', 'keyQ7', '-inf', '+inf','withscores', 'limit', 0, 2) if e[1] ~= nil then for k,v in ipairs(e) do if k%2==1 then redis.call('ZREM', KEYS[1], v) end end end return e" 1 keyQ7 0
    std::string script = "local e = redis.call('ZRANGEBYSCORE', KEYS[1], ARGV[1], ARGV[2], 'withscores', 'limit', 0, ARGV[3]) if e[1] ~= nil then for k,v in ipairs(e) do if k%2==1 then redis.call('ZREM', KEYS[1], v) end end end return e";    
    std::vector<std::string> keys = { "fj_rms_expirelist" };
    std::vector<std::string> args = { exprirStart, exprirEnd, std::to_string(nBatchSize) };
    std::vector<std::string> out;
    int ret = m_pRedisClient->eval_strings(script.c_str(), keys, args, out, error);    
    if (0 == ret) 
    {
        //rmsmsgid1,score1,rmsmsgid2,score2,rmsmsgidN,scoreN
        assert(0 == out.size() % 2 );
        rmsMsgidList.clear();
        for (auto pos = out.begin(); pos != out.end(); ) 
        {
            rmsMsgidList.push_back(*pos);
            pos += 2;
        }
        ret = 0;
    }
    return ret;
}


/************************************************************************
*  功能:从RDN下载超时队列批量移除富信流水号
*  参数 rmsMsgidList:待移除的富信流水号列表
*  @return <0 出错; 0 表示该有序集不存在或成员不存在; > 0 表示成功删除的成员数量
************************************************************************/
int MRedisDB::RemoveExpireQuque(const std::vector<std::string>& rmsMsgidList, std::string& error)
{
    assert(nullptr != m_pRedisClient);
    if (!m_pRedisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgidList.empty());
    if (rmsMsgidList.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char* redisKey = "fj_rms_expirelist";
    return m_pRedisClient->zrem(redisKey, rmsMsgidList, error);
}


/************************************************************************
*  功能:保存补发的短信流水号对应的富信流水号
*  参数:smsMsgid:短信流水号 rmsMsgid:富信流水号
*  @return 0:成功; 非0:错误码
************************************************************************/
int MRedisDB::InsertSms2Rms(const std::string& smsMsgid, const std::string& rmsMsgid, std::string& error)
{
    assert(nullptr != m_pRedisClient);
    if (!m_pRedisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!smsMsgid.empty());
    assert(!rmsMsgid.empty());
    if (smsMsgid.empty() || rmsMsgid.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_sms2rms:{%s}", smsMsgid.c_str());

    std::map<std::string, std::string> properties;
    properties["rms_msgid"] = rmsMsgid;
    return m_pRedisClient->hmset(redisKey, properties, error);
}


/************************************************************************
*  功能:查询补发的短信流水号对应的富信流水号
*  参数:smsMsgid:短信流水号 rmsMsgid:返回的富信流水号
*  @return 0:成功; -3:数据不存在 其它:错误码
************************************************************************/
int MRedisDB::QuerySms2Rms(const std::string& smsMsgid, std::string& rmsMsgid, std::string& error)
{
    assert(nullptr != m_pRedisClient);
    if (!m_pRedisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!smsMsgid.empty());
    if (smsMsgid.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_sms2rms:{%s}", smsMsgid.c_str());

    std::vector<std::string> names = { "rms_msgid" };
    std::map<std::string, std::string> properties;
    int ret = m_pRedisClient->hmget(redisKey, names, properties, error);
    if (0 != ret) {
        return ret;
    }

    if (properties.empty()) {
        error = "data is empty";
        return -3;
    }

    auto pos = properties.find("rms_msgid");
    assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsMsgid = properties["rms_msgid"];
    }

    return 0;
}


/************************************************************************
*  功能:设置有效期
*  参数:smsMsgid:短信流水号 expireSec:有效期，单位：毫秒
*  @return >0:成功; 0：该key不存在;  <0: 出错
************************************************************************/
int MRedisDB::ExpireSms2Rms(const std::string& smsMsgid, const int& expireSec, std::string& error)
{
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_sms2rms:{%s}", smsMsgid.c_str());
    return this->ExpireKey(redisKey, expireSec, error);
}


/************************************************************************
*  功能:设置有效期
*  参数:key：redis中的key值 expireSec:有效期，单位：毫秒
*  @return >0:成功; 0：该key不存在;  <0: 出错
************************************************************************/
int MRedisDB::ExpireKey(const std::string& key, const int& expireSec, std::string& error)
{
    assert(nullptr != m_pRedisClient);
    if (!m_pRedisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    if (expireSec < 0) {
        error = "invalid parameter";
        return -2;
    }

    return m_pRedisClient->pexpire(key.c_str(), expireSec, error);
}