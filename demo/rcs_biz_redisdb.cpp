#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <stdarg.h>
#include <memory.h>
#include "kcfg.h"
#include "rcs_biz_redisdb.h"


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

std::string format3(const char* fmt, ...)
{
    va_list marker;
    va_start(marker, fmt);
    std::string strResult = "";
    size_t nLength = vsnprintf(NULL, 0, fmt, marker);
    va_start(marker, fmt);
    strResult.resize(nLength + 1);
    size_t nSize = vsnprintf((char*)strResult.data(), nLength + 1, fmt, marker);
    va_end(marker);
    strResult.resize(nSize);
    return strResult;
}


namespace rcs
{
    RcsBizRedisDB::RcsBizRedisDB()  { }

    RcsBizRedisDB::~RcsBizRedisDB() { }

    //初始化组件
    int RcsBizRedisDB::InitRedisDB(const std::string& strRedisCluster,
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
        std::shared_ptr<MWRedisClient> redisClient(MWRedisClientFactory::New(), [&](MWRedisClient* e){
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
                  bStdoutOpen,
                  error);

        if (0 == ret) {
            m_pRedisClient = redisClient;
        }
        return ret;
    }


    //反初始化
    void RcsBizRedisDB::DestoryRedisDB()
    {
        assert(nullptr != m_pRedisClient);
        if (m_pRedisClient) {
            m_pRedisClient.reset();
            m_pRedisClient = nullptr;
        }
    }


    std::string RcsBizRedisDB::makeBatchTaskKey(const std::string& userid,
        const std::string& taskid)
    {
        assert(!userid.empty());
        assert(!taskid.empty());

        if (userid.empty() ||
            taskid.empty()) {
            return "";
        }

        std::string redisKey = format3("rcs:5g:batchmsg:%s:%s", userid.c_str(), taskid.c_str());
        return redisKey;
    }

    int RcsBizRedisDB::WriteBatchTaskInfo(const std::string& strBatchTaskKey,
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

        if (strBatchTaskKey.empty() ||
            kvMap.empty())
        {
            error = "invalid parameter";
            return -2;
        }

        int ret = 0;
        do {
            ret = redisClient->hmset(strBatchTaskKey, kvMap, error);
            if (0 != ret) {
                std::this_thread::sleep_for(std::chrono::milliseconds(nRetrySleepMsec));
            }
        } while (0 != ret && nRetry-- > 0);
        return  ret;
    }


    int RcsBizRedisDB::ReadBatchTaskInfo(const std::string& strBatchTaskKey,
        int nRetry,
        int nRetrySleepMsec,
        std::map<std::string, std::string>& kvMap,
        std::string& error)
    {
        std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
        assert(nullptr != redisClient);
        if (!redisClient) {
            error = "MRedisDB is uninitialized";
            return -1;
        }

        if (strBatchTaskKey.empty() ||
            kvMap.empty()) {
            error = "invalid parameter";
            return -2;
        }

        std::vector<std::string> names;
        for (auto& items : kvMap) {
            names.emplace_back(items.first);
        }

        int ret = 0;
        do {
            ret = redisClient->hmget(strBatchTaskKey, names, kvMap, error);
            if (0 == ret) {
                break;
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(nRetrySleepMsec));
            }
        } while (0 != ret && nRetry-- > 0);
        return ret;
    }


    //////////////////////////////////////////////////////////////////////////
    std::string RcsBizRedisDB::makeMWCarrierNoTableKey(const std::string& carrier)
    {
        assert(!userid.empty());
        if (carrier.empty()) {
            return "";
        }

        std::string redisKey = format3("rcs:5g:mw_carrier_no:%s", carrier.c_str());
        return redisKey;
    }

    int RcsBizRedisDB::ReadMWCarrierNo(const std::string& strMWCarrierNoTableKey,
        std::string& carrier,
        std::string& bno,
        std::string& vno,
        std::string& createtm,
        std::string& error)
    {
        std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
        assert(nullptr != redisClient);
        if (!redisClient) {
            error = "MRedisDB is uninitialized";
            return -1;
        }

        if (strMWCarrierNoTableKey.empty()) {
            error = "invalid parameter";
            return -2;
        }

        std::vector<std::string> names = { "carrier", "bno", "vno", "createtm"};
        std::map<std::string, std::string> properties;
        int ret = redisClient->hmget(strMWCarrierNoTableKey, names, properties, error);
        if (0 != ret) {
            return ret;
        }

        if (properties.empty()) {
            error = "data is empty";
            return -3;
        }

        carrier = properties["carrier"];
        bno = properties["bno"];
        vno = properties["vno"];
        createtm = properties["createtm"];
        return 0;
    }


    //////////////////////////////////////////////////////////////////////////
    std::string RcsBizRedisDB::makeMWPlatformTableKey()
    {
        return "rcs:5g:mw_platform";
    }

    //0：成功 非0：失败
    int RcsBizRedisDB::ReadMWPlatform(const std::string& strMWPlatformTableKey,
        std::vector<platform>& t,
        std::string& error)
    {
        std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
        assert(nullptr != redisClient);
        if (!redisClient) {
            error = "MRedisDB is uninitialized";
            return -1;
        }

        if (strMWPlatformTableKey.empty()) {
            error = "invalid parameter";
            return -2;
        }

        std::vector<std::pair<std::string, double>> itemlist;
        int ret = redisClient->zrangebyscore_with_scores(strMWPlatformTableKey.c_str(),
            "-inf", "+inf", itemlist, NULL, NULL, error);

        if (ret < 0) { //error
            return ret;
        }

        for (auto& item : itemlist) {
            std::string data_json = item.first;
            /*
            {
                 "ptcode": "J85",
                 "platname" : "J85平台",
                 "islocal" : "0",
                 "ptid" : "8899",
                 "isnova" : "0",
                 "createtm" : "1601358833013" //2020/9/29 13:53:53:013
            }
            */
            std::map<std::string, std::string> obj;
            bool bRet = kcfg::Json2Struct(data_json, obj);
            if (false == bRet) {
                assert(0 && "failed to parse json");
                continue;
            }

            platform p;
            auto pos = obj.find("ptcode");
            if (obj.end() == pos) { continue; }
            p.ptcode = pos->second;

            pos = obj.find("platname");
            if (obj.end() == pos) { continue;}
            p.platname = pos->second;

            pos = obj.find("islocal");
            if (obj.end() == pos) { continue; }
            p.islocal = pos->second;

            pos = obj.find("ptid");
            if (obj.end() == pos) { continue; }
            p.ptid = pos->second;

            pos = obj.find("isnova");
            if (obj.end() == pos) { continue; }
            p.isnova = pos->second;

            pos = obj.find("createtm");
            if (obj.end() == pos) { continue; }
            p.createtm = pos->second;

            //save it
            t.emplace_back(p);
            
        }
        return 0;
    }


    //////////////////////////////////////////////////////////////////////////
    std::string RcsBizRedisDB::makeChatBotTableKey(const std::string& chatbot_name_id)
    {
        assert(!chatbot_name_id.empty());
        if (chatbot_name_id.empty()) {
            return "";
        }

        std::string redisKey = format3("rcs:5g:chatbot:%s", chatbot_name_id.c_str());
        return redisKey;
    }


    int RcsBizRedisDB::ReadCarrierChatbot(const std::string& str5gTplTableKey,
        rcs::RcsBizRedisDB::chatbot_table& t,
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

        if (str5gTplTableKey.empty()) {
            error = "invalid parameter";
            return -2;
        }

        static std::vector<std::string> names = { "ecid", "name", "name_id", "name_id_status",
            "api_acct_list", "chatbot_list", 
            "rms_acct", "rms_acct_pwd", "rms_sign", "rms_fallback_url",
            "createtm"
        };

        int ret = 0;
        do {
            std::map<std::string, std::string> properties;
            ret = redisClient->hmget(str5gTplTableKey, names, properties, error);
            if (0 == ret) {
                t.ecid = properties["ecid"];
                t.name = properties["name"];
                t.name_id = properties["name_id"];
                t.name_id_status = properties["name_id_status"];

                {
                    std::string data_json = properties["api_acct_list"];
                    bool ret = kcfg::Json2Struct(data_json, t.api_acct_list);
                    if (false == ret) {
                        error = "failed to parse json:" + data_json;
                        return -1000;
                    }
                }

                {
                    std::string data_json = properties["chatbot_list"];
                    std::vector<std::map<std::string, std::string>> chatbot_list;
                    bool ret = kcfg::Json2Struct(data_json, chatbot_list);
                    if (false == ret) {
                        error = "failed to parse json:" + data_json;
                        return -1000;
                    }

                    for (auto& item : chatbot_list) 
                    {    
                        chatbot_table::carrier_chatbotinfo e;
                        e.maap_id = item["maap_id"];
                        e.maap_name = item["maap_name"];
                        e.carrier = item["carrier"];
                        e.maap_spip = item["maap_spip"];
                        e.maap_resip = item["maap_resip"];
                        e.chatboth5_callback_url = item["chatboth5_callback_url"];
                        e.chatboth5_fallback_url = item["chatboth5_fallback_url"];
                        e.maap_status = item["maap_status"];
                        e.chatbot_id = item["chatbot_id"];
                        e.chatbot_id_status = item["chatbot_id_status"];
                        e.app_id = item["app_id"];
                        e.app_secret = item["app_secret"];

                        t.chatbot_list.emplace_back(e);
                    }
                }

                {
                    t.rms_acct = properties["rms_acct"];
                    t.rms_acct_pwd = properties["rms_acct_pwd"];
                    t.rms_sign = properties["rms_sign"];
                    t.rms_fallback_url = properties["rms_fallback_url"];
                }
                t.createtm = properties["createtm"];
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(nRetrySleepMsec));
            }
        } while (0 != ret && nRetry-- > 0);
        return ret;
    }


    //////////////////////////////////////////////////////////////////////////
    std::string RcsBizRedisDB::makeApiAcctTableKey(const std::string& api_acct)
    {
        assert(!chatbot_name_id.empty());
        if (api_acct.empty()) {
            return "";
        }

        std::string redisKey = format3("rcs:5g:userid:%s", api_acct.c_str());
        return redisKey;
    }

    int RcsBizRedisDB::ReadApiAcctInfo(const std::string& strApiAcctTableKey,
        rcs::RcsBizRedisDB::api_acct_table& t,
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

        if (strApiAcctTableKey.empty()) {
            error = "invalid parameter";
            return -2;
        }

        static std::vector<std::string> names = { "ecid", "corp_status", "charg_obj",
            "api_acct", "acct_pwd", "acct_status", "ec_status",
            "fee_flag", "send_level", "mo_get_mode", "tplsts_get_mode",
            "rpt_get_mode", "mo_url", "rpt_url", "tpl_sts_url",
            "link_num", "keep_alive",
            "chatbot_list",
            "ip_segment","ip_addr", "createtm"
        };

        int ret = 0;
        do {
            std::map<std::string, std::string> properties;
            ret = redisClient->hmget(strApiAcctTableKey, names, properties, error);
            if (0 == ret) {
                t.ecid = properties["ecid"];
                t.corp_status = properties["corp_status"];
                t.charg_obj = properties["charg_obj"];
                t.api_acct = properties["api_acct"];
                t.acct_pwd = properties["acct_pwd"];
                t.acct_status = properties["acct_status"];
                t.ec_status = properties["ec_status"];
                t.fee_flag = properties["fee_flag"];
                t.send_level = properties["send_level"];
                t.mo_get_mode = properties["mo_get_mode"];
                t.tplsts_get_mode = properties["tplsts_get_mode"];
                t.rpt_get_mode = properties["rpt_get_mode"];
                t.mo_url = properties["mo_url"];
                t.rpt_url = properties["rpt_url"];
                t.tpl_sts_url = properties["tpl_sts_url"];
                t.link_num = properties["link_num"];
                t.keep_alive = properties["keep_alive"];

                {
                    std::string data_json = properties["chatbot_list"];
                    if (!data_json.empty())
                    {
                        std::vector<std::map<std::string, std::string>> chatbot_list;
                        bool ret = kcfg::Json2Struct(data_json, chatbot_list);
                        if (false == ret) {
                            error = "failed to parse json:" + data_json;
                            return -1000;
                        }

                        for (auto& item : chatbot_list)
                        {
                            api_acct_table::carrier_chatbotid e;
                            e.bind_status = item["bind_status"];
                            e.name = item["name"];
                            e.name_id = item["name_id"];
                            e.name_id_status = item["name_id_status"];
                            t.chatbot_list.emplace_back(e);
                        }
                    }
                }

                t.ip_segment = properties["ip_segment"];
                t.ip_addr = properties["ip_addr"];
                t.createtm = properties["createtm"];
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(nRetrySleepMsec));
            }
        } while (0 != ret && nRetry-- > 0);
        return ret;
    }


    //////////////////////////////////////////////////////////////////////////
    std::string RcsBizRedisDB::make5gTplTableKey(const std::string& tplid)
    {
        assert(!tplid.empty());
        if (tplid.empty()) {
            return "";
        }

        std::string redisKey = format3("rcs:5g:tplid:%s", tplid.c_str());
        return redisKey;
    }

    int RcsBizRedisDB::Read5gTplInfo(const std::string& str5gTplTableKey,
        rcs::RcsBizRedisDB::tpltable& t,
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

        if (str5gTplTableKey.empty()) {
            error = "invalid parameter";
            return -2;
        }

        static std::vector<std::string> names = { "ecid", "tplid", "tpl_type", "tpl_name", "tpl_content_url",
            "chatbot_name", "chatbot_name_id", "tpl_status",
            "tpl_bind_list",
            "createtm"
        };

        int ret = 0;
        do {
            std::map<std::string, std::string> properties;
            ret = redisClient->hmget(str5gTplTableKey, names, properties, error);
            if (0 == ret) {
                t.ecid = properties["ecid"];
                t.tplid = properties["tplid"];
                t.tpl_type = properties["tpl_type"];
                t.tpl_name = properties["tpl_name"];
                t.tpl_content_url = properties["tpl_content_url"];

                {
                    std::string data_json = properties["fallback_chabot_h5"];
                    std::vector<std::map<std::string, std::string>> fallback_chabot_h5_list;
                    bool ret = kcfg::Json2Struct(data_json, fallback_chabot_h5_list);
                    if (false == ret) {
                        error = "failed to parse json:" + data_json;
                        return -1000;
                    }

                    for (auto& item : fallback_chabot_h5_list)
                    {
                        tpltable::fallback_chabot_h5_item e;
                        e.content = item["content"];
                        e.carrier = item["carrier"];
                     
                        t.fallback_chabot_h5.emplace_back(e);
                    }
                }

                {
                    std::string data_json = properties["fallback_rms"];
                    std::map<std::string, std::string> fallback_rms_kv;
                    bool ret = kcfg::Json2Struct(data_json, fallback_rms_kv);
                    if (false == ret) {
                        error = "failed to parse json:" + data_json;
                        return -1001;
                    }
  
                    t.fallback_rms.rms_tpl_id = fallback_rms_kv["rms_tpl_id"];
                    t.fallback_rms.rms_params = fallback_rms_kv["rms_params"];
                }

                t.chatbot_name = properties["chatbot_name"];
                t.chatbot_name_id = properties["chatbot_name_id"];
                t.tpl_status = properties["tpl_status"];
              
                {
                    std::string data_json = properties["tpl_bind_list"];
                    std::vector<std::map<std::string, std::string>> tpl_bind_list;
                    bool ret = kcfg::Json2Struct(data_json, tpl_bind_list);
                    if (false == ret) {
                        error = "failed to parse json:" + data_json;
                        return -1002;
                    }

                    for (auto& item : tpl_bind_list)
                    {
                        tpltable::carrier_tpl e;
                        e.maap_id = item["maap_id"];
                        e.carrier = item["carrier"];
                        e.up2x_content = item["up2x_content"];
                        e.up1x_content = item["up1x_content"];
                        e.rms_content = item["rms_content"];
                        e.sms_content = item["sms_content"];
                        e.audit_status = item["audit_status"];

                        t.tpl_bind_list.emplace_back(e);
                    }
                }

                t.createtm = properties["createtm"];
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(nRetrySleepMsec));
            }
        } while (0 != ret && nRetry-- > 0);
        return ret;
    }

    std::string RcsBizRedisDB::make5gApiMsgSendListTableKey()
    {
        std::string redisKey = "rcs:api:msgsendlist";
        return redisKey;
    }

    //////////////////////////////////////////////////////////////////////////
    std::string RcsBizRedisDB::make5gMtToSendTableKey(const std::string& maap_id)
    {
        assert(!maap_id.empty());
        if (maap_id.empty()) {
            return "";
        }

        std::string redisKey = format3("rcs:5g:msg:tosendzset:%s", maap_id.c_str());
        return redisKey;
    }


    int RcsBizRedisDB::Push5gMtToSendZset(const std::string& str5gToSendTableKey,
        std::string& msg, 
        double score,
        std::string& error)
    {
        std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
        assert(nullptr != redisClient);
        if (!redisClient) {
            error = "MRedisDB is uninitialized";
            return -1;
        }

        if (str5gToSendTableKey.empty() ||
            msg.empty()) {
            error = "invalid parameter";
            return -2;
        }

        return redisClient->zadd(str5gToSendTableKey, msg, score, error);
    }


    //////////////////////////////////////////////////////////////////////////
    std::string RcsBizRedisDB::makeMoZsetKey()
    {
        return "rcs:5g:msg:mozset";
    }



    int RcsBizRedisDB::PopMoZset(const std::string& strMoZsetKey,
        const std::string& minpos,
        const std::string& maxpos,
        const int& batchsize,
        std::vector<std::string>& outlist,
        std::string& error)
    {
        std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
        assert(nullptr != redisClient);
        if (!redisClient) {
            error = "MRedisDB is uninitialized";
            return -1;
        }

        if (strMoZsetKey.empty() ||
            batchsize <= 0)
        {
            error = "invalid parameter";
            return -2;
        }
        return redisClient->zpop_queue(strMoZsetKey, minpos, maxpos, batchsize, outlist, error);
    }



    //////////////////////////////////////////////////////////////////////////
    std::string RcsBizRedisDB::makeReplyKeywordTableKey() 
    {
        return "rcs:5g:rcs_reply_keyword";
    }


    int RcsBizRedisDB::ReadReplyKeyword(const std::string& strReplyKeywordTableKey,
        std::vector<reply_keyword_table>& t,
        std::string& error)
    {
        std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
        assert(nullptr != redisClient);
        if (!redisClient) {
            error = "MRedisDB is uninitialized";
            return -1;
        }

        if (strReplyKeywordTableKey.empty())
        {
            error = "invalid parameter";
            return -2;
        }

        std::vector<std::pair<std::string, double>> itemlist;
        int ret = redisClient->zrangebyscore_with_scores(strReplyKeywordTableKey.c_str(),
            "-inf", "+inf", itemlist, NULL, NULL, error);

        if (ret < 0) { //error
            return ret;
        }

        for (auto& item : itemlist) {
            std::string data_json = item.first;
            std::map<std::string, std::string> obj;
            bool bRet = kcfg::Json2Struct(data_json, obj);
            if (false == bRet) {
                assert(0 && "failed to parse json");
                continue;
            }

            reply_keyword_table e;
            e.ecid = obj["ecid"]; //企业ID
            e.chatbot_name = obj["chatbot_name"]; //梦网侧的chatbot的名称
            e.chatbot_name_id = obj["chatbot_name_id"]; //梦网侧的chatbotid
            e.keyword = obj["keyword"]; //关键词内容
            e.priority = obj["priority"];  //关键词优先级
            e.scene_id = obj["scene_id"];  //关键词所在场景ID
            e.tpl_id = obj["tpl_id"]; //触发关键词后回复的模板ID
            e.match_type = obj["match_type"]; //关键词匹配方式 "exact"：精确匹配 "vague"：模糊匹配
            e.status = obj["status"]; //关键词状态 0：启用 1：禁用
            e.createtm = obj["createtm"]; //写入记录的时间戳，精度：毫秒

            //save it
            t.emplace_back(e);
        }
        return 0;
    }


    //////////////////////////////////////////////////////////////////////////
    std::string RcsBizRedisDB::makeSpChatbotIdZSetKey()
    {
        return "rcs:5g:sp_chatbot_idzset";
    }


    bool RcsBizRedisDB::LookupSpChatbotId(const std::string& spchatbotid,
        std::string& error)
    {
        std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
        assert(nullptr != redisClient);
        if (!redisClient) {
            error = "MRedisDB is uninitialized";
            return false;
        }

        std::string redis_key = makeSpChatbotIdZSetKey();
        assert(!redis_key.empty());
        assert(!spchatbotid.empty());
        if (redis_key.empty() ||
            spchatbotid.empty()) {
            error = "invalid parameter";
            return false;
        }

        double result;
        int ret = redisClient->zscore(redis_key.c_str(), spchatbotid.c_str(), spchatbotid.size(), result, error);
        return 0 == ret? true:false;
    }

    //////////////////////////////////////////////////////////////////////////
    //运营商ChatBot查找API账号缓存表(rcs:5g:mochatbot:{$chatbotid}:carrier:{$carrier})
    //////////////////////////////////////////////////////////////////////////
    std::string RcsBizRedisDB::makeMoSpChatbotTableKey(const std::string& spchatbotid, const std::string& carrier)
    {
        assert(!spchatbotid.empty());
        assert(!carrier.empty());
        if (spchatbotid.empty() ||
            carrier.empty()) {
            return "";
        }

        std::string redisKey = format3("rcs:5g:mochatbot:%s:carrier:%s", spchatbotid.c_str(), carrier.c_str());
        return redisKey;
    }

    
    int RcsBizRedisDB::ReadMoSpChatbotApiacct(const std::string& strMoSpChatbotTableKey,
        std::vector<mo_spchatbot_table>& t,
        std::string& error)
    {
        
        std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
        assert(nullptr != redisClient);
        if (!redisClient) {
            error = "MRedisDB is uninitialized";
            return -1;
        }

        if (strMoSpChatbotTableKey.empty())
        {
            error = "invalid parameter";
            return -2;
        }

        std::string json_array_data;
        bool bret = redisClient->get(strMoSpChatbotTableKey.c_str(),
            json_array_data, error);

        if (false == bret) {
            return -3;
        }


        /*
        [
            {
            "api_acct": "FXS001",
            "chatbot_name_id": "梦网侧Chatbotid",
            "createtm": "1601358833013" //2020/9/29 13:53:53:013
            }
        ]
        */
        std::vector<std::map<std::string, std::string>> array_obj;
        bret = kcfg::Json2Struct(json_array_data, array_obj);
        if (false == bret) {
            assert(0 && "failed to parse json");
            error = "failed to parse json:" + json_array_data;
            return -4;
        }
        for (auto& item : array_obj) {
            mo_spchatbot_table e;
            e.api_acct = item["api_acct"]; //API账号
            e.chatbot_name_id = item["chatbot_name_id"]; //梦网侧的chatbotid
            e.createtm = item["createtm"]; //写入记录的时间戳，精度：毫秒

            //save it
            t.emplace_back(e);
        }
        return 0;
    }



    std::string RcsBizRedisDB::makeMaapIdZSetKey()
    {
        return "rcs:5g:maap_idzset";
    }


    int RcsBizRedisDB::ReadMaapIdZSet(const std::string& strMaapIdZSetKey,
        std::vector<std::pair<std::string, double>> & maapidList,
        std::string& error)
    {
        std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
        assert(nullptr != redisClient);
        if (!redisClient) {
            error = "MRedisDB is uninitialized";
            return -1;
        }

        if (strMaapIdZSetKey.empty())
        {
            error = "invalid parameter";
            return -2;
        }

        return redisClient->zrangebyscore_with_scores(strMaapIdZSetKey.c_str(),
            "-inf", "+inf", maapidList, NULL, NULL, error);
    }

    //////////////////////////////////////////////////////////////////////////
    int RcsBizRedisDB::Hincrby(const std::string& key,
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

    int RcsBizRedisDB::ZCard(const std::string& strZsetKey, std::string& error)
    {
        std::shared_ptr<MWRedisClient> redisClient = m_pRedisClient;
        assert(nullptr != redisClient);
        if (!redisClient) {
            error = "MRedisDB is uninitialized";
            return -1;
        }

        if (strZsetKey.empty())
        {
            error = "invalid parameter";
            return -2;
        }

        return redisClient->zcard(strZsetKey.c_str(), error);
    }


    int RcsBizRedisDB::SliceListRpush(const std::string& key, const std::string& buf, const int& slicenum, uint32_t* slotindex, std::string& error)
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

        return redisClient->slicelist_rpush(key, slicenum, buf, slotindex, error);
    }

    int RcsBizRedisDB::SliceListLpop(const std::string& key, std::string& buf, const int& slicenum, uint32_t* slotindex, std::string& error)
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

        return redisClient->slicelist_lpop(key, slicenum, buf, slotindex, error);
    }


}