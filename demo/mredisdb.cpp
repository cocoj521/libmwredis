//#include <StdAfx.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <chrono>
#include <thread>
#include <algorithm>
#include "mredisdb.h"



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

//在接收到的指针上调用delte[]的函数
void MRedisDBDeleter(MWRedisClient* e) {
    e->unintRedis();
    MWRedisClientFactory::Destroy(e);
}

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
    bool bStdoutOpen,
    std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient(MWRedisClientFactory::New(), MRedisDBDeleter);
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
              bStdoutOpen, error);

    if (0 == ret) {
        m_pRedisClient = redisClient;
    }
    return ret;
}


//反初始化
void MRedisDB::DestoryRedisDB()
{
    assert(nullptr != m_pRedisClient);
    if (m_pRedisClient) {
        m_pRedisClient.reset();
        m_pRedisClient = nullptr;
    }
}

bool MRedisDB::PingRedisDB()
{
    assert(nullptr != m_pRedisClient);
    if (nullptr == m_pRedisClient) {
        return false;
    }
    return m_pRedisClient->ping();
}


/************************************************************************
*  功能:插入富信下行消息缓存
*  参数:rmsMsgid：富信流水号 rmsDetail:待插入的富信下行消息缓存 error:错误描述
*  @return 0:成功; 非0:错误码
************************************************************************/
int MRedisDB::InsertMtRms(const std::string& rmsMsgid, const mt_cache_rms& rmsDetail, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::InsertMtRms(rmsMsgid, rmsDetail, error);
	if (n32Ret != 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::InsertMtRms(rmsMsgid, rmsDetail, error);
			if (0 == n32Ret)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}

int MRedisDB::InsertMtRms(const std::string& rmsMsgid, const mt_cache_rms& rmsDetail, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
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
    properties["agent_login_uid"] = rmsDetail.agent_login_uid;
    properties["send_suppsms_rpt"] = rmsDetail.send_suppsms_rpt;
    properties["adsneedrpt"] = rmsDetail.adsneedrpt;
    properties["adtailty"] = rmsDetail.adtailty;
    properties["supp_msg"] = rmsDetail.supp_msg;
    properties["msg_src_ip_port"] = rmsDetail.msg_src_ip_port;

    return redisClient->hmset(redisKey, properties, error);
}


/************************************************************************
*  功能:查询富信下行消息缓存
*  参数:rmsMsgid：富信流水号 rmsDetail:返回的富信下行消息缓存
*  @return 0:成功; -3:数据不存在 其它:错误码
************************************************************************/
int MRedisDB::QueryMtRms(const std::string& rmsMsgid, mt_cache_rms& rmsDetail, std::string& error, const int8_t n8Count)
{
	int32_t n32Ret = MRedisDB::QueryMtRms(rmsMsgid, rmsDetail, error);
	if (0 != n32Ret && -3 != n32Ret)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = QueryMtRms(rmsMsgid, rmsDetail, error);
			if (0 == n32Ret || -3 == n32Ret)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}

int MRedisDB::QueryMtRms(const std::string& rmsMsgid, mt_cache_rms& rmsDetail, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgid.empty());
    if (rmsMsgid.empty()) {
        error = "invalid parameter";
        return -2;
    }

  
    std::vector<std::string> names = { "tmpl_id", "msg_type", "tmpl_type", "sms_msgid", "chk_expflg",
        "supp_privilige", "supp_sms_state", "mt_exdata", "reserve", "send_notice_rpt", "send_dld_rpt",
        "agent_login_uid", "send_suppsms_rpt", "adsneedrpt", "adtailty", "supp_msg", "msg_src_ip_port"
    };

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_mtrms:{%s}", rmsMsgid.c_str());

    std::map<std::string, std::string> properties;
    int ret = redisClient->hmget(redisKey, names, properties, error);
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
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.msg_type = properties["msg_type"];
    }

    pos = properties.find("tmpl_type");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.tmpl_type = properties["tmpl_type"];
    }

    pos = properties.find("sms_msgid");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.sms_msgid = properties["sms_msgid"];
    }

    pos = properties.find("chk_expflg");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.chk_expflg = properties["chk_expflg"];
    }

    pos = properties.find("supp_privilige");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.supp_privilige = properties["supp_privilige"];
    }

    pos = properties.find("supp_sms_state");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.supp_sms_state = properties["supp_sms_state"];
    }

    pos = properties.find("mt_exdata");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.mt_exdata = properties["mt_exdata"];
    }

    pos = properties.find("reserve");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.reserve = properties["reserve"];
    }

    pos = properties.find("send_notice_rpt");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.send_notice_rpt = properties["send_notice_rpt"];
    }

    pos = properties.find("send_dld_rpt");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.send_dld_rpt = properties["send_dld_rpt"];
    }
    
    pos = properties.find("agent_login_uid");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.agent_login_uid = properties["agent_login_uid"];
    }

    pos = properties.find("send_suppsms_rpt");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.send_suppsms_rpt = properties["send_suppsms_rpt"];
    }

    pos = properties.find("adsneedrpt");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.adsneedrpt = properties["adsneedrpt"];
    }

    pos = properties.find("adtailty");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.adtailty = properties["adtailty"];
    }

    pos = properties.find("supp_msg");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.supp_msg = properties["supp_msg"];
    }

    pos = properties.find("msg_src_ip_port");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rmsDetail.msg_src_ip_port = properties["msg_src_ip_port"];
    }
 
    return 0;
}


/************************************************************************
*  功能:"缓存中富信失败业务网关已补发短信状态"与cmpSmsState相等则更新；否则不更新 
*  参数:rmsMsgid:富信流水号 cmpSmsState:待比较的状态 newSuppSmsState:设置的新状态
*  @return 0:未更新; 1:已更新; 其它:错误码
************************************************************************/
int MRedisDB::CheckAndSetSuppedSmsState(const std::string& rmsMsgid, const std::string& cmpSmsState, const std::string& newSuppSmsState, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::CheckAndSetSuppedSmsState(rmsMsgid, cmpSmsState, newSuppSmsState, error);
	if (0 != n32Ret && 1 != n32Ret)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::CheckAndSetSuppedSmsState(rmsMsgid, cmpSmsState, newSuppSmsState, error);
			if (0 == n32Ret && 1 == n32Ret)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}
int MRedisDB::CheckAndSetSuppedSmsState(const std::string& rmsMsgid, const std::string& cmpSmsState, const std::string& newSuppSmsState, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
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
    int ret = redisClient->eval_number(script.c_str(), keys, args, out, error);
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
int MRedisDB::MarkRecivedRdnDldRpt(const std::string& rmsMsgid, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::MarkRecivedRdnDldRpt(rmsMsgid, error);
	if (0 != n32Ret && 1 != n32Ret)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::MarkRecivedRdnDldRpt(rmsMsgid, error);
			if (0 == n32Ret || 1 == n32Ret)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}
int MRedisDB::MarkRecivedRdnDldRpt(const std::string& rmsMsgid, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
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
    int ret = redisClient->eval_number(script.c_str(), keys, args, out, error);
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
int MRedisDB::ExpireMtRms(const std::string& rmsMsgid, const int& expireMilSec, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::ExpireMtRms(rmsMsgid, expireMilSec, error);
	if (n32Ret <= 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::ExpireMtRms(rmsMsgid, expireMilSec, error);
			if (n32Ret > 0)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}

int MRedisDB::ExpireMtRms(const std::string& rmsMsgid, const int& expireMilSec, std::string& error)
{
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_mtrms:{%s}", rmsMsgid.c_str());
    return this->ExpireKey(redisKey, expireMilSec, error);
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
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
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
    return redisClient->hmset(redisKey, properties, error);
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
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
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
    return redisClient->hmset(redisKey, properties, error);
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
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
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
    return redisClient->hmset(redisKey, properties, error);
}




/************************************************************************
*  功能:插入富信流水号到RDN下载超时队列
*  参数:rmsMsgid:富信流水号
*  @return
*  0：表示一个也未添加，可能因为该成员已经存在于有序集中
*  <0：表示出错或 key 对象非有序集对象
*  >0：新添加的成员数量
*********************************************ble value***************************/
int MRedisDB::InsertRmsExpireQueue(const std::string& rmsMsgid, const int timeoutSec, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::InsertRmsExpireQueue(rmsMsgid, timeoutSec, error);
	if (n32Ret <= 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::InsertRmsExpireQueue(rmsMsgid, timeoutSec, error);
			if (n32Ret > 0)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}
int MRedisDB::InsertRmsExpireQueue(const std::string& rmsMsgid, const int timeoutSec, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
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
    return redisClient->zadd(redisKey, rmsMsgid, score, error);
}


/************************************************************************
*  功能:批量扫描RDN下载超时的富信流水号列表
*  参数:nBatchSize:一次性获取的最大个数,1~1000 rmsMsgidList:返回的超时的富信流水号列表
*  @return 0:成功; 非0:错误码
************************************************************************/
int MRedisDB::ScanRmsExpireQuque(const int nBatchSize, std::vector<std::string>& rmsMsgidList, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
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
    int ret = redisClient->eval_strings(script.c_str(), keys, args, out, error);
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
int MRedisDB::RemoveExpireQuque(const std::vector<std::string>& rmsMsgidList, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::RemoveExpireQuque(rmsMsgidList, error);
	if (n32Ret < 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::RemoveExpireQuque(rmsMsgidList, error);
			if (n32Ret >= 0)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}
int MRedisDB::RemoveExpireQuque(const std::vector<std::string>& rmsMsgidList, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgidList.empty());
    if (rmsMsgidList.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char* redisKey = "fj_rms_expirelist";
    return redisClient->zrem(redisKey, rmsMsgidList, error);
}

int MRedisDB::InsertRms2Rms(const std::string& rmsMsgid, const rms2rms_cache& rms2rmsDetail, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::InsertRms2Rms(rmsMsgid, rms2rmsDetail, error);
	if (0 != n32Ret)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::InsertRms2Rms(rmsMsgid, rms2rmsDetail, error);
			if (0 == n32Ret)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}
int MRedisDB::InsertRms2Rms(const std::string& rmsMsgid, const rms2rms_cache& rms2rmsDetail, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgid.empty());
    if (rmsMsgid.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_rms2rms:{%s}", rmsMsgid.c_str());

    std::map<std::string, std::string> properties;
    properties["tmpl_id"] = rms2rmsDetail.tmpl_id;
    properties["msg_type"] = rms2rmsDetail.msg_type;
    properties["tmpl_type"] = rms2rmsDetail.tmpl_type;
    properties["agent_login_uid"] = rms2rmsDetail.agent_login_uid;
    properties["content"] = rms2rmsDetail.content;
    properties["supptime"] = rms2rmsDetail.supptime;
    properties["suppcnt"] = rms2rmsDetail.suppcnt;
    properties["usedcnt"] = rms2rmsDetail.usedcnt;


    properties["supp_sms_timeout"] = rms2rmsDetail.supp_sms_timeout;
    properties["supp_rms_timeout"] = rms2rmsDetail.supp_rms_timeout;
    properties["last_mt_msgid"] = rms2rmsDetail.last_mt_msgid;
    properties["last_notice_rpt"] = rms2rmsDetail.last_notice_rpt;
    properties["last_dld_rpt"] = rms2rmsDetail.last_dld_rpt;
    properties["simulate_type"] = rms2rmsDetail.simulate_type;
    properties["has_sent_ntc"] = rms2rmsDetail.has_sent_ntc;
    properties["has_sent_dld"] = rms2rmsDetail.has_sent_dld;
    properties["suppstop"] = rms2rmsDetail.suppstop;
    properties["sms_supp_flag"] = rms2rmsDetail.sms_supp_flag;
    properties["last_delay_rpt"] = rms2rmsDetail.last_delay_rpt;
    properties["state"] = rms2rmsDetail.state;
    properties["createtm"] = rms2rmsDetail.createtm;
    return redisClient->hmset(redisKey, properties, error);
}


int MRedisDB::WriteRms2RmsInfo(const std::string& rmsMsgid,
    const std::map<std::string, std::string>& kvMap,
    int nRetry,
    int nRetrySleepMsec,
    std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }
 
    if (rmsMsgid.empty())
    {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_rms2rms:{%s}", rmsMsgid.c_str());

    int ret = 0;
    do {
        ret = redisClient->hmset(redisKey, kvMap, error);
        if (0 != ret) {
            std::this_thread::sleep_for(std::chrono::milliseconds(nRetrySleepMsec));
        }
    } while (0 != ret && nRetry-- > 0);
    return  ret;
}

int MRedisDB::QueryRms2Rms(const std::string& rmsMsgid, rms2rms_cache& rms2rmsDetail, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::QueryRms2Rms(rmsMsgid, rms2rmsDetail, error);
	if (0 != n32Ret && -3 != n32Ret)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::QueryRms2Rms(rmsMsgid, rms2rmsDetail, error);
			if (0 == n32Ret || -3 == n32Ret)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}
int MRedisDB::QueryRms2Rms(const std::string& rmsMsgid, rms2rms_cache& rms2rmsDetail, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgid.empty());
    if (rmsMsgid.empty()) {
        error = "invalid parameter";
        return -2;
    }

    std::vector<std::string> names = { "tmpl_id", "msg_type", "tmpl_type", "agent_login_uid", "content", "supptime",
        "suppcnt", "usedcnt", "supp_sms_timeout", "supp_rms_timeout", "last_mt_msgid", "last_notice_rpt", "last_dld_rpt",
        "simulate_type", "has_sent_ntc", "has_sent_dld", "suppstop", "sms_supp_flag", "last_delay_rpt", "state", "createtm"
    };

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_rms2rms:{%s}", rmsMsgid.c_str());

    std::map<std::string, std::string> properties;
    int ret = redisClient->hmget(redisKey, names, properties, error);
    if (0 != ret) {
        return ret;
    }

    if (properties.empty()) {
        error = "data is empty";
        return -3;
    }

    auto pos = properties.find("tmpl_id");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.tmpl_id = properties["tmpl_id"];
    }

    pos = properties.find("msg_type");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.msg_type = properties["msg_type"];
    }

    pos = properties.find("tmpl_type");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.tmpl_type = properties["tmpl_type"];
    }

    pos = properties.find("agent_login_uid");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.agent_login_uid = properties["agent_login_uid"];
    }

    pos = properties.find("content");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.content = properties["content"];
    }

    pos = properties.find("supptime");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.supptime = properties["supptime"];
    }

    pos = properties.find("suppcnt");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.suppcnt = properties["suppcnt"];
    }

    pos = properties.find("usedcnt");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.usedcnt = properties["usedcnt"];
    }

    pos = properties.find("supp_sms_timeout");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.supp_sms_timeout = properties["supp_sms_timeout"];
    }

    pos = properties.find("supp_rms_timeout");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.supp_rms_timeout = properties["supp_rms_timeout"];
    }

    pos = properties.find("last_mt_msgid");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.last_mt_msgid = properties["last_mt_msgid"];
    }

    pos = properties.find("last_notice_rpt");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.last_notice_rpt = properties["last_notice_rpt"];
    }

    pos = properties.find("last_dld_rpt");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.last_dld_rpt = properties["last_dld_rpt"];
    }

    pos = properties.find("simulate_type");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.simulate_type = properties["simulate_type"];
    }


    pos = properties.find("has_sent_ntc");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.has_sent_ntc = properties["has_sent_ntc"];
    }

    pos = properties.find("has_sent_dld");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.has_sent_dld = properties["has_sent_dld"];
    }

    pos = properties.find("suppstop");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.suppstop = properties["suppstop"];
    }

    pos = properties.find("sms_supp_flag");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.sms_supp_flag = properties["sms_supp_flag"];
    }

    pos = properties.find("last_delay_rpt");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.last_delay_rpt = properties["last_delay_rpt"];
    }

    pos = properties.find("state");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.state = properties["state"];
    }

    pos = properties.find("createtm");
    //assert(pos != properties.end());
    if (pos != properties.end()) {
        rms2rmsDetail.createtm = properties["createtm"];
    }
    return 0;
}

int MRedisDB::ExpireRms2Rms(const std::string& rmsMsgid, const int& expireMilSec, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::ExpireRms2Rms(rmsMsgid, expireMilSec, error);
	if (n32Ret <= 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::ExpireRms2Rms(rmsMsgid, expireMilSec, error);
			if (n32Ret > 0)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}

int MRedisDB::ExpireRms2Rms(const std::string& rmsMsgid, const int& expireMilSec, std::string& error)
{
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_rms2rms:{%s}", rmsMsgid.c_str());
    return this->ExpireKey(redisKey, expireMilSec, error);
}

int MRedisDB::IncSuppRmsUsedCnt(const std::string& rmsMsgid, long long int* result, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::IncSuppRmsUsedCnt(rmsMsgid, result, error);
	if (n32Ret != 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::IncSuppRmsUsedCnt(rmsMsgid, result, error);
			if (0 == n32Ret)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}

int MRedisDB::IncSuppRmsUsedCnt(const std::string& rmsMsgid, long long int* result, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgid.empty());
    if (rmsMsgid.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_rms2rms:{%s}", rmsMsgid.c_str());
    return redisClient->hincrby(redisKey, "usedcnt", 1, result, error);
}

int MRedisDB::InsertSuppRmsTrack(const std::string& rmsMsgid, const std::string& gateBindId, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::InsertSuppRmsTrack(rmsMsgid, gateBindId, error);
	if (n32Ret < 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::InsertSuppRmsTrack(rmsMsgid, gateBindId, error);
			if (n32Ret >= 0)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}
int MRedisDB::InsertSuppRmsTrack(const std::string& rmsMsgid, const std::string& gateBindId, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgid.empty());
    assert(!gateBindId.empty());
    if (rmsMsgid.empty() || gateBindId.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_supprmstrack:{%s}", rmsMsgid.c_str());

    std::vector<std::string> members = { gateBindId };
    return redisClient->sadd(redisKey, members, error);
}

int MRedisDB::ExpireSuppRmsTrack(const std::string& rmsMsgid, const int& expireMilSec, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::ExpireSuppRmsTrack(rmsMsgid, expireMilSec, error);
	if (n32Ret <= 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::ExpireSuppRmsTrack(rmsMsgid, expireMilSec, error);
			if (n32Ret > 0)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}

int MRedisDB::ExpireSuppRmsTrack(const std::string& rmsMsgid, const int& expireMilSec, std::string& error)
{
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_supprmstrack:{%s}", rmsMsgid.c_str());
    return this->ExpireKey(redisKey, expireMilSec, error);
}


int MRedisDB::LookupSuppRmsTrack(const std::string& rmsMsgid, const std::string& gateBindId, bool& result, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgid.empty());
    assert(!gateBindId.empty());
    if (rmsMsgid.empty() || gateBindId.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_supprmstrack:{%s}", rmsMsgid.c_str());
    int ret = redisClient->sismember(redisKey, gateBindId, error);
    result = (0 == ret ? true : false);
    return ret;
}
int MRedisDB::QuerySuppRmsTrack(const std::string& rmsMsgid, std::set<std::string>& gateBindIdList, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::QuerySuppRmsTrack(rmsMsgid, gateBindIdList, error);
	if (n32Ret < 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::QuerySuppRmsTrack(rmsMsgid, gateBindIdList, error);
			if (n32Ret >= 0)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}

int MRedisDB::QuerySuppRmsTrack(const std::string& rmsMsgid, std::set<std::string>& gateBindIdList, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgid.empty());
    if (rmsMsgid.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_supprmstrack:{%s}", rmsMsgid.c_str());

    std::vector<std::string> tmplist;
    int ret = redisClient->smembers(redisKey, tmplist, error);
    for (auto item : tmplist) {
		gateBindIdList.insert(item);
	}
    return ret;
}
int MRedisDB::DeleteAllRms2Rms(const std::string& rmsMsgid, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::DeleteAllRms2Rms(rmsMsgid, error);
	if (n32Ret < 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::DeleteAllRms2Rms(rmsMsgid, error);
			if (n32Ret >= 0)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}

int MRedisDB::DeleteAllRms2Rms(const std::string& rmsMsgid, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    std::vector<std::string> keys;

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_rms2rms:{%s}", rmsMsgid.c_str());
    keys.push_back(redisKey);

    _snprintf_s(redisKey, _countof(redisKey), "fj_supprmstrack:{%s}", rmsMsgid.c_str());
    keys.push_back(redisKey);

    return redisClient->del(keys, error);
}

int MRedisDB::SetSuppRmsLastMsgId(const std::string& rmsMsgid, const std::string& lastMsgid, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::SetSuppRmsLastMsgId(rmsMsgid, lastMsgid, error);
	if (n32Ret < 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::SetSuppRmsLastMsgId(rmsMsgid, lastMsgid, error);
			if (n32Ret >= 0)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}

int MRedisDB::SetSuppRmsLastMsgId(const std::string& rmsMsgid, const std::string& lastMsgid, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgid.empty());
    assert(!lastMsgid.empty());
    if (rmsMsgid.empty() || lastMsgid.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_supprmslastid:{%s}", rmsMsgid.c_str());
    return (redisClient->set(redisKey, lastMsgid, error) == true? 0 : -3);
}

int MRedisDB::GetSuppRmsLastMsgId(const std::string& rmsMsgid, std::string& lastMsgid, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::GetSuppRmsLastMsgId(rmsMsgid, lastMsgid, error);
	if (n32Ret < 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::GetSuppRmsLastMsgId(rmsMsgid, lastMsgid, error);
			if (n32Ret >= 0)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}

int MRedisDB::GetSuppRmsLastMsgId(const std::string& rmsMsgid, std::string& lastMsgid, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!rmsMsgid.empty());
    if (rmsMsgid.empty()) {
        error = "invalid parameter";
        return -2;
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_supprmslastid:{%s}", rmsMsgid.c_str());
    return (redisClient->get(redisKey, lastMsgid, error) == true ? 0 : -3);
}

int MRedisDB::ExpireSuppRmsLastMsgId(const std::string& rmsMsgid, const int& expireMilSec, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::ExpireSuppRmsLastMsgId(rmsMsgid, expireMilSec, error);
	if (n32Ret <= 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::ExpireSuppRmsLastMsgId(rmsMsgid, expireMilSec, error);
			if (n32Ret > 0)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}

int MRedisDB::ExpireSuppRmsLastMsgId(const std::string& rmsMsgid, const int& expireMilSec, std::string& error)
{
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_supprmslastid:{%s}", rmsMsgid.c_str());
    return this->ExpireKey(redisKey, expireMilSec, error);
}

int MRedisDB::DeleteSuppRmsLastMsgId(const std::string& rmsMsgid, std::string& error, const int8_t n8Count)
{
	int32_t  n32Ret = MRedisDB::DeleteSuppRmsLastMsgId(rmsMsgid, error);
	if (n32Ret < 0)
	{
		int8_t n8Num = 0;
		do
		{
			//暂停1秒
			//CAdapter::Sleep(1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			n32Ret = MRedisDB::DeleteSuppRmsLastMsgId(rmsMsgid, error);
			if (n32Ret >= 0)//成功
			{
				break;
			}
			n8Num++;
		} while (n8Num < n8Count);
	}
	return  n32Ret;
}


int MRedisDB::DeleteSuppRmsLastMsgId(const std::string& rmsMsgid, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    std::vector<std::string> keys;
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, _countof(redisKey), "fj_supprmslastid:{%s}", rmsMsgid.c_str());
    keys.push_back(redisKey);
    return redisClient->del(keys, error);
}


int MRedisDB::PushTodoSms2rms(const std::string& buf, const int& slicenum, uint32_t* slotindex, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }
    
    return redisClient->slicelist_rpush("fj:sms2rms:todolist", slicenum, buf, slotindex, error);
}

int MRedisDB::QueryTodoSms2rmsSize(const int& slicenum, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }
    assert(slicenum > 0);
    if (slicenum <= 0) {
        error = "invalid parameter";
        return -2;
    }
    return redisClient->slicelist_totallen("fj:sms2rms:todolist", slicenum, error);
}


int MRedisDB::PopDoneSms2rms(std::string& buf, const int& slicenum, uint32_t* slotindex, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    return redisClient->slicelist_lpop("fc:sms2rms:donelist", slicenum, buf, slotindex, error);
}

int MRedisDB::QueryDoneSms2rmsSize(const int& slicenum, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }
    assert(slicenum > 0);
    if (slicenum <= 0) {
        error = "invalid parameter";
        return -2;
    }
    return redisClient->slicelist_totallen("fc:sms2rms:donelist", slicenum, error);
}


std::string MRedisDB::makeSms2rmsRemainRptKey(const std::string& strSmsSpgateId)
{
    assert(!strSmsSpgateId.empty());
    if (strSmsSpgateId.empty()) {
        return "";
    }
    char redisKey[128] = { 0 };
#ifdef _WIN32
    _snprintf_s(redisKey, sizeof(redisKey) / sizeof(redisKey[0]),
        "fj:sms2rms:remainrpt:%s", strSmsSpgateId.c_str());
#else 
    snprintf(redisKey, sizeof(redisKey) / sizeof(redisKey[0]),
        "fj:sms2rms:remainrpt:%s", strSmsSpgateId.c_str());
#endif
    return redisKey;
}

int MRedisDB::PushRemainRptSms2rms(const std::string& strRemainRptKey,
    const std::string& buf,
    const int& slicenum,
    uint32_t* slotindex,
    std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    return redisClient->slicelist_rpush(strRemainRptKey, slicenum, buf, slotindex, error);
}


int MRedisDB::PopRemainRptSms2rms(const std::string& strRemainRptKey,
    std::string& buf,
    const int& slicenum,
    uint32_t* slotindex,
    std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    return redisClient->slicelist_lpop(strRemainRptKey, slicenum, buf, slotindex, error);
}


std::string MRedisDB::makeBatchMtToSendListKey(const int& msglevel)
{
    assert(msglevel >= 1 && msglevel <= 9);
    if (!(msglevel >= 1 && msglevel <= 9)) {
        return "";
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, sizeof(redisKey) / sizeof(redisKey[0]),
        "mgate:batchmsg:tosendlist:%d", msglevel);
    return redisKey;
}

int MRedisDB::PopFromSendList(const int& maxMsglevel,
    std::string& buf,
    const int& slicenum,
    uint32_t* slotindex,
    std::string& error)
{
    assert(maxMsglevel >= 1 && maxMsglevel <= 9);
    if (!(maxMsglevel >= 1 && maxMsglevel <= 9)) {
        error = "invalid parameter";
        return -1;
    }

    int ret = 0;
    for (int i = 1; i <= maxMsglevel; i++)
    {
        ret = this->PopFromSendList(makeBatchMtToSendListKey(i), buf, slicenum, slotindex, error);
        if (ret > 0 && !buf.empty()) {
            break; // got one
        }
    }
    return ret;
}


int MRedisDB::QueryBatchMtToSendListSize(const int& msglevel, const int& slicenum, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(msglevel >= 1 && msglevel <= 9);
    if (!(msglevel >= 1 && msglevel <= 9)) {
        error = "invalid parameter";
        return -1;
    }

    assert(slicenum > 0);
    if (slicenum <= 0) {
        error = "invalid parameter";
        return -2;
    }

    return redisClient->slicelist_totallen(makeBatchMtToSendListKey(msglevel), slicenum, error);
}


int MRedisDB::PopFromSendList(const std::string& strBatchMtToSendListKey,
    std::string& buf,
    const int& slicenum,
    uint32_t* slotindex,
    std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(slicenum > 0);
    if (slicenum <= 0) {
        error = "invalid parameter";
        return -2;
    }

    return redisClient->slicelist_lpop(strBatchMtToSendListKey, slicenum, buf, slotindex, error);
}

std::string MRedisDB::makeUseridFeeKey(const int& protype, const std::string& userid)
{
    assert(!userid.empty());
    if (userid.empty()) {
        return "";
    }
    // mgate:fee:userid:{$protype}:{$userid}
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, sizeof(redisKey) / sizeof(redisKey[0]),
        "mgate:fee:userid:%d:%s", protype, userid.c_str());
    return redisKey;
}

std::string MRedisDB::makeEcidFeeKey(const int& protype, const std::string& ecid)
{
    assert(!ecid.empty());
    if (ecid.empty()) {
        return "";
    }
    // mgate:fee:ecid:{$protype}:{$ecid}
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, sizeof(redisKey) / sizeof(redisKey[0]),
        "mgate:fee:ecid:%d:%s", protype, ecid.c_str());
    return redisKey;
}


int MRedisDB::WriteBalance(const std::string& strFeeKey,
    std::string& mgateid,
    std::string prenum,
    std::string totalnum,
    const std::string& inserttm,
    const std::string& invalidtm,
    int nRetry,
    int nRetrySleepMsec,
    std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!(prenum.empty() && totalnum.empty()));
    if ((prenum.empty() && totalnum.empty()) ||
        strFeeKey.empty()) {
        error = "invalid parameter";
        return -2;
    }

    std::map<std::string, std::string> kv;
    if (!prenum.empty()) {
        //pre:mgateid:{$待扣网关编号1} 
        char filedName[128] = { 0 };
        _snprintf_s(filedName, sizeof(filedName) / sizeof(filedName[0]),
            "pre:mgateid:%s", mgateid.c_str());

        //{$precharge_num}:{$inserttm}:{$invalidtm}
        char filedValue[128] = { 0 };
        _snprintf_s(filedValue, sizeof(filedValue) / sizeof(filedValue[0]),
            "%s:%s:%s", prenum.c_str(), inserttm.c_str(), invalidtm.c_str());
        kv.insert(std::make_pair(filedName, filedValue));
    }

    if (!totalnum.empty()) {
        //total:mgateid:{$余额总条数网关编号}
        char filedName[128] = { 0 };
        _snprintf_s(filedName, sizeof(filedName) / sizeof(filedName[0]),
            "total:mgateid:%s", mgateid.c_str());

        //{$total_num}:{$updatetm}:{$invalidtm}
        char filedValue[128] = { 0 };
        _snprintf_s(filedValue, sizeof(filedValue) / sizeof(filedValue[0]),
            "%s:%s:%s", totalnum.c_str(), inserttm.c_str(), invalidtm.c_str());
        kv.insert(std::make_pair(filedName, filedValue));
    }

    int ret = 0;
    do {
        ret = redisClient->hmset(strFeeKey, kv, error);
        if (0 != ret) {
            std::this_thread::sleep_for(std::chrono::milliseconds(nRetrySleepMsec));
        }
    } while (0 != ret && nRetry-- > 0);
    return  ret;
}


int MRedisDB::ReadBalance(const std::string& strFeeKey,
    const int64_t& nowtm,
    int64_t& amount,
    int nRetry,
    int nRetrySleepMsec,
    std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    if (strFeeKey.empty()) {
        error = "invalid parameter";
        return -2;
    }

    int ret = 0;
    do {
        std::map<std::string, std::string> properties;
        ret = redisClient->hgetall(strFeeKey, properties, error);
        if (0 == ret) {
            if (properties.size() < 2) {
                error = "no data in redis";
                return 208; //找不到用户计费信息，与计费库返回值保持一致
            }

            bool hasFeeInfo = false;
            int64_t total_num = 0;
            int64_t precharge_num = 0;
            std::vector<std::pair<int64_t, int64_t>> total_num_list; //inserttm:total_num
            for (auto item : properties)
            {
                std::string filedname = item.first;
                std::string filedvalue = item.second;
                if (filedname.empty() || filedvalue.empty()) {
                    continue;
                }

                //{$num}:{$inserttm}:{$invalidtm}
                std::vector<std::string> token;
                split3<std::vector<std::string>>(filedvalue, token, ":");
                if (token.size() < 3) {  //format error, just skip it. 
                    continue;
                }

                if (token[0].empty() ||
                    token[1].empty() ||
                    token[2].empty()) {  //format error, just skip it. 
                    continue;
                }

                int64_t num = std::atoll(token[0].c_str());
                int64_t inserttm = std::atoll(token[1].c_str());
                int64_t invalidtm = std::atoll(token[2].c_str());
                if (nowtm > invalidtm) {
                    continue; //本条记录超出有效期，作废
                }

                static const std::string PRECHARGE_PREFIX = "pre:mgateid:";
                static const std::string TOTALCHARGE_PREFIX = "total:mgateid:";
                //pre:mgateid:{$待扣网关编号N}
                if (0 == memcmp(filedname.c_str(), PRECHARGE_PREFIX.c_str(), PRECHARGE_PREFIX.length())) {
                    //{$precharge_num}:{$updatetm}
                    precharge_num += num;
                }
                //total:mgateid:{$余额总条数网关编号}
                else if (0 == memcmp(filedname.c_str(), TOTALCHARGE_PREFIX.c_str(), TOTALCHARGE_PREFIX.length())) 
                {
                    if (false == hasFeeInfo) {
                        total_num = num;
                        hasFeeInfo = true;
                    }
                    else if (num < total_num) {
                        total_num = num;
                    }
                }
            }

            if (false == hasFeeInfo) {
                error = "invalid fee information in redis";
                return -4; //找不到总余额费用信息
            }
            
            //计算规则
            /*
            富信实时剩余总条数策略：
            1、取某个节点富信实时剩余总条数的最小值。
            2、当前时间超过有效期，本条记录不参与计算。

            剩余总条数计算规则：
            实际剩余总条数=富信实时剩余总条数 – （网关编号1的总待扣富信条数 +网关编号2的总待扣富信条数 + … +网关编号N的总待扣富信条数）
            （注：当前时间超过有效期，本条记录不参与计算。）
            */
            amount = total_num - precharge_num;
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(nRetrySleepMsec));
        }
    } while (0 != ret && nRetry-- > 0);
    return ret;
}

std::string MRedisDB::makeBatchTaskKey(const std::string& userid,
    const std::string& taskid)
{
    assert(!userid.empty());
    assert(!taskid.empty());

    if (userid.empty() ||
        taskid.empty()) {
        return "";
    }

    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, sizeof(redisKey) / sizeof(redisKey[0]),
        "fep:batchmsg:%s:%s", userid.c_str(), taskid.c_str());
    return redisKey;
}


std::string MRedisDB::makeRptDetailKey(const std::string& userid,
    const std::string& msgid,
    const int& rpttype
    )
{
    assert(!userid.empty());
    assert(!msgid.empty());
    if (userid.empty() ||
        msgid.empty()) {
        return "";
    }

    // biz:rpt:msgid: {$userid}:{$msgid}:{$rpttype}
    // (rpttype：1-通知报告2-下载报告 3-富补短报告）
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, sizeof(redisKey) / sizeof(redisKey[0]),
        "biz:rpt:msgid:%s:%s:%d", userid.c_str(), msgid.c_str(), rpttype);
    return redisKey;
}

std::string MRedisDB::makeRptKeyByMobile(const std::string& userid,
    const std::string& mobile
    )
{
    assert(!userid.empty());
    assert(!mobile.empty());
    if (userid.empty() ||
        mobile.empty()) {
        return "";
    }

    // biz:rpt:mobile:{$userid}:{$mobile}
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, sizeof(redisKey) / sizeof(redisKey[0]),
        "biz:rpt:mobile:%s:%s", userid.c_str(), mobile.c_str());
    return redisKey;
}

std::string MRedisDB::makeRptKeyByCustid(const std::string& userid,
    const std::string& custid
    )
{
    assert(!userid.empty());
    assert(!custid.empty());
    if (userid.empty() ||
        custid.empty()) {
        return "";
    }

    // biz:rpt:custid:{$userid}:{$custid}
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, sizeof(redisKey) / sizeof(redisKey[0]),
        "biz:rpt:custid:%s:%s", userid.c_str(), custid.c_str());
    return redisKey;
}


int MRedisDB::WriteRpt(const std::string& strRptKey,
    const rpt_cache& rpt, 
    long long int& updatedcnt,
    int nRetry,
    int nRetrySleepMsec,
    std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!strRptKey.empty());
    if (strRptKey.empty()) {
        error = "invalid parameter";
        return -2;
    }

    std::map<std::string, std::string> properties;
    properties["msgid"] = rpt.msgid;
    properties["userid"] = rpt.userid;
    properties["custid"] = rpt.custid;
    properties["pknum"] = rpt.pknum;
    properties["pktotal"] = rpt.pktotal;
    properties["mobile"] = rpt.mobile;
    properties["spno"] = rpt.spno;
    properties["exno"] = rpt.exno;
    properties["stime"] = rpt.stime;
    properties["rtime"] = rpt.rtime;
    properties["type"] = rpt.type;
    properties["status"] = rpt.status;
    properties["errcode"] = rpt.errcode;
    properties["errdesc"] = rpt.errdesc;
    properties["exdata"] = rpt.exdata;

    int ret = 0;
    do{
        ret = redisClient->hmset(strRptKey, properties, error);
        if (0 != ret) {
            std::this_thread::sleep_for(std::chrono::milliseconds(nRetrySleepMsec));
        }
    } while (ret !=0 && nRetry-- > 0);
   
    if (0 == ret) {
        long long int result = 0;
        ret = redisClient->hincrby(strRptKey, "updatedcnt", 1, &result, error);
        if (0 != ret) {
            return ret;
        }
        updatedcnt = result;
    }
    return ret;
}


int MRedisDB::PushToZSet(const std::string& key,
    const std::string& value,
    const double score,
    int nRetry,
    int nRetrySleepMsec,
    std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    if (key.empty() ||
        value.empty())
    {
        error = "invalid parameter";
        return -2;
    }
    int ret = 0;
    do
    {
        ret = redisClient->zpush_queue(key, value, score, error);
        if (ret <0) {     
            std::this_thread::sleep_for(std::chrono::milliseconds(nRetrySleepMsec));
        }
    } while (ret<0 && nRetry-- > 0);
    return ret;
}


std::string MRedisDB::makeSPRouteKey(const std::string& yyyymm,
    const std::string& area,
    const std::string& gateid
    )
{
    assert(!yyyymm.empty());
    assert(!area.empty());
    assert(!gateid.empty());
    if (yyyymm.empty() ||
        area.empty()   ||
        gateid.empty()) {
        return "";
    }

    //biz:sproute:cnt:{$YYYYMM}:{$area}:{$gateid}
    char redisKey[128] = { 0 };
    _snprintf_s(redisKey, sizeof(redisKey) / sizeof(redisKey[0]),
        "biz:sproute:cnt:%s:%s:%s", yyyymm.c_str(), area.c_str(), gateid.c_str());
    return redisKey;
}


int MRedisDB::IncSPRouteCnt(const std::string& strSPRouteKey,
    const int& dayofmonth,
    const long long int& inc,
    long long int* daycnt,
    long long int* totalcnt, 
    std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!strSPRouteKey.empty());
    if (strSPRouteKey.empty() ||
        !(dayofmonth>=1 && dayofmonth <=31)) {
        error = "invalid parameter";
        return -2;
    }

    std::string dayfield = "day" + std::to_string(dayofmonth);
    int r1 = redisClient->hincrby(strSPRouteKey, dayfield, inc, daycnt, error);
    if (0 != r1) {
        return r1;
    }
    return redisClient->hincrby(strSPRouteKey, "total", inc, totalcnt, error);
}


int MRedisDB::ReadSPRouteCnt(const std::string& strSPRouteKey,
    const int& dayofmonth,
    long long& daycnt,
    long long& totalcnt,
    int nRetry,
    int nRetrySleepMsec,
    std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    assert(!strSPRouteKey.empty());
    if (strSPRouteKey.empty() ||
        !(dayofmonth >= 1 && dayofmonth <= 31)) {
        error = "invalid parameter";
        return -2;
    }

    std::vector<std::string> names;
    names.emplace_back("total");
    std::string dayfield = "day" + std::to_string(dayofmonth);
    names.emplace_back(dayfield);

    std::map<std::string, std::string> kvmap;
    int ret = 0;
    do {
        ret = redisClient->hmget(strSPRouteKey, names, kvmap, error);
        if (0 == ret) {
            break;
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(nRetrySleepMsec));
        }
    } while (0 != ret && nRetry-- > 0);

    if (0 != ret) {
        return ret;
    }

    totalcnt = kvmap["total"].empty() == true ? 0 : (std::atoll(kvmap["total"].c_str()));
    daycnt = kvmap[dayfield].empty() == true ? 0 : (std::atoll(kvmap[dayfield].c_str()));
    return ret;
}

int MRedisDB::ExpireKey(const std::string& key, const int& expireMilSec, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    if (expireMilSec < 0) {
        error = "invalid parameter";
        return -2;
    }

    return redisClient->pexpire(key.c_str(), expireMilSec, error);
}


int MRedisDB::ExpireKeyBySec(const std::string& key, const int& expireSec, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }

    if (expireSec < 0) {
        error = "invalid parameter";
        return -2;
    }

    return redisClient->expire(key.c_str(), expireSec, error);
}

int MRedisDB::DeleteKey(const std::vector<std::string>& keys, std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }
    return redisClient->del(keys, error);
}


int MRedisDB::Hincrby(const std::string& key,
    const std::string& name,
    const long long int& inc,
    long long int* result,
    std::string& error)
{
    std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
    assert(nullptr != redisClient);
    if (!redisClient) {
        error = "MRedisDB is uninitialized";
        return -1;
    }
    return redisClient->hincrby(key, name, inc, result, error);
}
