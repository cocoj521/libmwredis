/*
************************************************************************
*
* Description : 5g消息业务服务(5g-biz-svc)操作REDIS业务接口 
*
* Created Date : 2020 / 10 / 15
*
* Author : Peter Hu
*
* Copyright(c) ShenZhen Montnets Technology, Inc.All rights reserved.
*
************************************************************************
*/

#ifndef __RCS_BIZ_REDISDB_H__
#define __RCS_BIZ_REDISDB_H__

#include <memory>
#include <vector>
#include <set>
#include <string>
#include "mwredislib.h"


//操作redis业务接口
namespace rcs
{
    class RcsBizRedisDB
    {
    public:
        RcsBizRedisDB();
        ~RcsBizRedisDB();

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
            bool bStdoutOpen,
            std::string& error);

        //反初始化
        void DestoryRedisDB();

    public:
        //////////////////////////////////////////////////////////////////////////
        //5g批量任务表 rcs:5g:batchmsg:{$userid}:{$taskid}
        //////////////////////////////////////////////////////////////////////////
        std::string makeBatchTaskKey(const std::string& userid,
            const std::string& taskid);

        //0：成功 非0：失败
        int WriteBatchTaskInfo(const std::string& strBatchTaskKey,
            const std::map<std::string, std::string>& kvMap,
            int nRetry,
            int nRetrySleepMsec,
            std::string& error);

        //0：成功 非0：失败
        int ReadBatchTaskInfo(const std::string& strBatchTaskKey,
            int nRetry,
            int nRetrySleepMsec,
            std::map<std::string, std::string>& kvMap,
            std::string& error);


    public:
        //////////////////////////////////////////////////////////////////////////
        //梦网平台运营商号段缓存表 rcs:5g:mw_carrier_no:{$carrier}
        //0-移动,1-联通,21-电信,5-国外
        //////////////////////////////////////////////////////////////////////////
        std::string makeMWCarrierNoTableKey(const std::string& carrier);

        //0：成功 非0：失败
        int RcsBizRedisDB::ReadMWCarrierNo(const std::string& strMWCarrierNoTableKey,
            std::string& carrier,
            std::string& bno,
            std::string& vno,
            std::string& createtm,
            std::string& error);

    public:
        //////////////////////////////////////////////////////////////////////////
        //梦网平台信息缓存表(rcs:5g:mw_platform)
        //////////////////////////////////////////////////////////////////////////
        std::string makeMWPlatformTableKey();

        class platform {
        public:
            std::string ptcode; //平台编码
            std::string platname; //平台编码
            std::string islocal; //平台位置:0:该平台不是当前平台  非0 : 当前平台
            std::string ptid; //平台ID
            std::string isnova; //是否NOVA平台
            std::string createtm; //写入记录的时间戳，精度：毫秒
        };

        //>=0：成功 <0：失败
        int ReadMWPlatform(const std::string& strMWPlatformTableKey,
            std::vector<platform>& t,
            std::string& error);

    public:
        //////////////////////////////////////////////////////////////////////////
        //梦网侧chatbot_name_id列表缓存表 (rcs:5g:chatbot_name_idzset)
        //////////////////////////////////////////////////////////////////////////
        std::string makeChatBotTableKey(const std::string& chatbot_name_id);

        class chatbot_table {
        public:
            class carrier_chatbotinfo {
            public:
                std::string maap_id;
                std::string maap_name;
                std::string carrier;
                std::string maap_spip;
                std::string maap_resip;
                std::string chatboth5_callback_url;
                std::string chatboth5_fallback_url;
                std::string maap_status;
                std::string chatbot_id;
                std::string chatbot_id_status;
                std::string app_id;
                std::string app_secret;
            };
            std::string ecid;  //企业ID
            std::string name; //CHATBOT名称
            std::string name_id;  //梦网侧CHATBOT的ID
            std::string name_id_status; /*梦网侧CHATBOTID的状态
                                        NORMAL-正常可用
                                        CHECKING-审核中
                                        REJECTED-审核不通过
                                        FORBIDDEN-已禁用
                                        */
            std::set<std::string> api_acct_list; //绑定的API账号列表
            std::vector<carrier_chatbotinfo> chatbot_list; //三大运营商绑定的chatbot信息
            
            std::string rms_acct; //回落的富信API账号
            std::string rms_acct_pwd; //回落的富信API账号密码
            std::string rms_sign; //富信签名内容，2-18个字，可包含中英文、数字和常用符号，包含【】
            std::string rms_fallback_url; //用于下发富信下行回落地址

            std::string createtm; //写入记录的时间戳，精度：毫秒
        };

        //0：成功 非0：失败
        int ReadCarrierChatbot(const std::string& str5gTplTableKey,
            rcs::RcsBizRedisDB::chatbot_table& t,
            int nRetry,
            int nRetrySleepMsec,
            std::string& error);

   public:
       //////////////////////////////////////////////////////////////////////////
       //API账号信息缓存表(rcs:5g:userid:{$api_acct})
       //api_acct就是userid
       //////////////////////////////////////////////////////////////////////////
       std::string makeApiAcctTableKey(const std::string& api_acct);

       class api_acct_table {
       public:
           class carrier_chatbotid {
           public:
               std::string bind_status; //API账号绑定chatbot_name_id的状态 0：启用 1：禁用
               std::string name; //CHATBOT名称
               std::string name_id; //梦网侧CHATBOT的ID
               std::string name_id_status; /*梦网侧CHATBOTID的状态
                                             NORMAL - 正常可用
                                             CHECKING - 审核中
                                             REJECTED - 审核不通过
                                             FORBIDDEN - 已禁用
                                            */
           };
       public:
           std::string ecid;  //企业ID
           std::string corp_status; //企业状态 0：正常可用 非0：不可用
           std::string charg_obj;  //计费对象 0:按账号计费 1 : 按企业计费
           std::string api_acct; //API账号
           std::string acct_pwd; //API密码
           std::string acct_status; //API账号状态 0:正常，1 : 禁用
           std::string ec_status; //企业侧账号状态 0: 上线  1 : 下线
           std::string fee_flag; //1：预付费 2：后付费
           std::string send_level; //消息级别 1~9之间取值  越小级越高
           std::string mo_get_mode; //PULL-拉取    PUSH-推送
           std::string tplsts_get_mode; //PULL-拉取    PUSH-推送
           std::string rpt_get_mode; //PULL-拉取    PUSH-推送
           std::string mo_url; //格式：http(s)://ip/域名:port/uri
           std::string rpt_url; //格式：http(s)://ip/域名:port/uri
           std::string tpl_sts_url; //格式：http(s)://ip/域名:port/uri
           std::string link_num; //<=0：不限制并发连接数 其他：限制到具体的连接数
           std::string keep_alive; //0：不允许，全部当短链接处理 1：允许，按HTTP标准协议处理
           std::vector<carrier_chatbotid> chatbot_list;
           std::string ip_segment; //ip范围段，多个范围段地址之间用英文逗号隔开，例如："0.0.0.0-1.1.1.1,2.2.2.2-3.3.3.3"
           std::string ip_addr; //固定ip地址，多个地址之间用英文逗号隔开，例如："21.10.10.34,89.188.234.32"
           std::string createtm; //写入记录的时间戳，精度：毫秒
       };

       //0：成功 非0：失败
       int ReadApiAcctInfo(const std::string& strApiAcctTableKey,
           rcs::RcsBizRedisDB::api_acct_table& t,
           int nRetry,
           int nRetrySleepMsec,
           std::string& error);

  public:
      //////////////////////////////////////////////////////////////////////////
      //5g模板信息缓存表 rcs:5g:tplid:{$tplid}
      //////////////////////////////////////////////////////////////////////////
      std::string make5gTplTableKey(const std::string& tplid);

      class tpltable {
      public:
          class carrier_tpl {
          public:
              std::string maap_id;
              std::string carrier;
              std::string up2x_content;
              std::string up1x_content;
              std::string rms_content;
              std::string sms_content;
              std::string audit_status;
          };

          class fallback_chabot_h5_item {
          public:
              std::string content; /*回落5G 生成的短息内容，
                                   例如 :查看消息
                                   https://f.10086.cn/5g/v/chatbotH5/#/chat?chabotId=12520020000066&sendMsg=%E4%BD%A0%E5%A5%BD
                                   */
              std::string carrier; //CHATBOT H5 所属运营商 0-移动,1-联通,21-电信,5-国外
          };
          
          class fallback_rms_item {
          public:
              std::string rms_tpl_id; //回落的富信模版ID
              std::string rms_params; //富信模板参数
          };

          std::string ecid;  //企业ID
          std::string tplid; //模板ID
          std::string tpl_type;  /*模板类型
                                 "text":文本
                                 "image" : 图片
                                 "audio" : 音频
                                 "video" : 视频
                                 "card" : 卡片
                                 "mixed" : 组合
                                 */
          std::string tpl_name; //模板名称
          std::string tpl_content_url;//这里填写的是存放在OSS上的模板内容JSON串http地址。格式：http(s) ://oss.xx.xx:port/xxx.json
          
          std::vector<fallback_chabot_h5_item> fallback_chabot_h5; //回落Chatbot H5
          fallback_rms_item fallback_rms; //回落富信，包含模板ID和动参

          std::string chatbot_name; //CHATBOT名称
          std::string chatbot_name_id; //梦网侧CHATBOT的ID
          std::string tpl_status; //梦网侧的模板最终状态 0 - 正常可用 1 - 不可用
          std::vector<carrier_tpl> tpl_bind_list; //5G模板在运营商侧审核信息列表
          std::string createtm; //写入记录的时间戳，精度：毫秒
      };


      //0：成功 非0：失败
      int Read5gTplInfo(const std::string& str5gTplTableKey,
          rcs::RcsBizRedisDB::tpltable& t,
          int nRetry,
          int nRetrySleepMsec,
          std::string& error);

  //////////////////////////////////////////////////////////////////////////
  //MT下行相关
  public:
      //////////////////////////////////////////////////////////////////////////
      //fep->mtbiz通信队列(rcs:api:msgsendlist)，存取数据见下：分片接口 
      //////////////////////////////////////////////////////////////////////////
      std::string make5gApiMsgSendListTableKey();


      //////////////////////////////////////////////////////////////////////////
      //待发送的MT下行消息队列表(rcs:5g:msg:tosendzset:{$maap_id})
      //////////////////////////////////////////////////////////////////////////
      std::string make5gMtToSendTableKey(const std::string& maap_id);

      /*
       * score:下行消息的时间戳，精度：毫秒
       * 0：表示一个也未添加，可能因为该成员已经存在于有序集中
       *     nothing was added to the sorted set
       * < 0：表示出错或 key 对象非有序集对象
       *     error or it was not a sorted set by the key
       * >0：新添加的成员数量
       *     the number of elements added
       */
      int Push5gMtToSendZset(const std::string& str5gMtToSendTableKey,
          std::string& msg, //JSON字符串
          double score,
          std::string& error);

    //////////////////////////////////////////////////////////////////////////
    //MO上行相关
    public:
        //////////////////////////////////////////////////////////////////////////
        //MO上行消息队列表(rcs:5g:msg:mozset)
        //////////////////////////////////////////////////////////////////////////
        std::string makeMoZsetKey();


        /************************************************************************
        *  功能:批量排序队列出栈
        *  参数:@key:队列的名称
        *  范围:[@minpos, @maxpos],[-inf, +inf]表示无穷小(无限制),无穷大(无限制)
        *  @batchsize:一次性获取的最大个数,填1~1000
        *  @outlist 返回的数据，格式为：item1,score1,item2,score2,itemN,scoreN...
        *  即:outlist[0]=item, outlist[1]=score
        *  @return >=0:成功; <0:错误码
        ************************************************************************/
        int PopMoZset(const std::string& strMoZsetKey,
            const std::string& minpos,
            const std::string& maxpos,
            const int& batchsize,
            std::vector<std::string>& outlist, //JSON数组
            std::string& error);

    //////////////////////////////////////////////////////////////////////////
    //RPT状态报告相关
    public:
        //////////////////////////////////////////////////////////////////////////
        //

    public:
        //////////////////////////////////////////////////////////////////////////
        //5G消息关键字缓存表(rcs:5g:rcs_reply_keyword)
        //////////////////////////////////////////////////////////////////////////
        std::string makeReplyKeywordTableKey();

        class reply_keyword_table {
        public:
            std::string ecid; //企业ID
            std::string chatbot_name; //梦网侧的chatbot的名称
            std::string chatbot_name_id; //梦网侧的chatbotid
            std::string keyword; //关键词内容,base64编码的
            std::string priority;  //关键词优先级
            std::string scene_id;  //关键词所在场景ID
            std::string tpl_id; //触发关键词后回复的模板ID
            std::string match_type; //关键词匹配方式 "exact"：精确匹配 "vague"：模糊匹配
            std::string status; //关键词状态 0：启用 1：禁用
            std::string createtm; //写入记录的时间戳，精度：毫秒
        };
        
        //>=0：成功 <0：失败
        int ReadReplyKeyword(const std::string& strReplyKeywordTableKey,
            std::vector<reply_keyword_table>& t, 
            std::string& error);

    public:
        //////////////////////////////////////////////////////////////////////////
        //运营商侧chatbot_id列表缓存表(rcs:5g:sp_chatbot_idzset)
        //////////////////////////////////////////////////////////////////////////
        std::string makeSpChatbotIdZSetKey();

        /**
        * 获得有序集 key 中，成员 member 的 score 值
        * @return {bool} 当不存在或出错时返回 false，否则返回 true
        */
        bool LookupSpChatbotId(const std::string& spchatbotid,
            std::string& error);

        //////////////////////////////////////////////////////////////////////////
        //运营商ChatBot查找API账号缓存表(rcs:5g:mochatbot:{$chatbotid}:carrier:{$carrier})
        //////////////////////////////////////////////////////////////////////////
        std::string makeMoSpChatbotTableKey(const std::string& spchatbotid, const std::string& carrier);

        class mo_spchatbot_table {
        public:
            std::string api_acct; //API账号
            std::string chatbot_name_id; //梦网侧的chatbotid
            std::string createtm; //写入记录的时间戳，精度：毫秒
        };

        //0:成功 <0:出错
        int ReadMoSpChatbotApiacct(const std::string& strMoSpChatbotTableKey,
            std::vector<mo_spchatbot_table>& t,
            std::string& error);

    public:
        //////////////////////////////////////////////////////////////////////////
        //运营商Maap平台id列表缓存表(rcs:5g:maap_idzset)
        //////////////////////////////////////////////////////////////////////////
        std::string makeMaapIdZSetKey();

        //>=0：成功 <0：失败
        int ReadMaapIdZSet(const std::string& strMaapIdZSetKey,
            std::vector<std::pair<std::string, double>>& maapidList,
            std::string& error);
    public:
        /**
        * 当某个 key 对象中的某个域字段为整数时，对其进行加减操作
        * the hash key
        * @param name key 对象的域字段名称
        * @param inc {long long int} 增加的值，可以为负值
        * @param result {long long int*} 非 NULL 时存储结果值
        * @return 0:成功，<0:表明出错或该 key 对象非哈希对象或该域字段非整数类型
        */
        int Hincrby(const std::string& key,
            const std::string& name,
            const long long int& inc,
            long long int* result,
            std::string& error);

        /************************************************************************
        *  功能:获取ZSet队列元素大小
        *：@return 该键不存在
        *      the key doesn't exist
        * < 0：出错或该键的数据对象不是有效的有序集对象
        *      error or it wasn't a sorted set by the key
        *  >0：当前键值对应的数据对象中的成员个数
        *      the number of elements in the sorted set
        */
        int ZCard(const std::string& strZsetKey, std::string& error);

    //分片接口
    public:
        /*将一个或多个值元素插入到分片list列表的表尾
        **当key不存在时，将自动初始化后插入
        **slotindex:返回插入的分片索引,允许填null
        *@return
        ** >0:返回添加完后当前分片列表对象中的元素个数
        ** <0:表示出错
        */
        int SliceListRpush(const std::string& key, const std::string& buf, const int& slicenum, uint32_t* slotindex, std::string& error);
        
        /*轮循从有数据的分片list列表对象中移除并返回头部元素-如果为空，尝试取下一个，最多重试N次
        **当key不存在时，将自动初始化后消费
        **slotindex:返回弹出数据的分片索引,允许填null
        **@return
        ** >0:表示成功弹出一个元素且返回值表示元素的长度，
        ** <0:表示出错，或该对象非列表对象，或该对象已经为空
        */
        int SliceListLpop(const std::string& key, std::string& buf, const int& slicenum, uint32_t* slotindex, std::string& error);

    private:
        std::shared_ptr<MWRedisClient> m_pRedisClient = nullptr;
    };

}

#endif // !__RCS_BIZ_REDISDB_H__
