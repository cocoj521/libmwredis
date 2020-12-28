/*
************************************************************************
*
* Description : 操作REDIS业务接口 
*
* Created Date : 2019 / 7 / 13
*
* Author : Peter Hu
*
* Copyright(c) ShenZhen Montnets Technology, Inc.All rights reserved.
*
************************************************************************
*/

#ifndef __MREDISDB_H__
#define __MREDISDB_H__

#include <memory>
#include <vector>
#include <set>
#include "mwredislib.h"


//富信失败业务网关已补发短信状态
enum SupptedSmsState
{
    NotSupptedSms = 0,                  //未补发
    SupptedSmsForFailedNoticeRpt = 1,   //通知报告失败已补发
    SupptedSmsForFailedDlgRpt = 2,      //下载报告失败已补发
    SupptedSmsForExpiredNoticeRpt = 3,  //通知报告超时已补发
    SupptedSmsForExpiredDlgRpt = 4,     //下载报告超时已补发
    SupptedSmsMax
};

//富信下行消息缓存结构体
class mt_cache_rms
{
public:
    std::string tmpl_id;   //模板ID
    std::string msg_type;  /*
                           0: 短信
                           1: 闪信
                           6: RO-SMS-Y 短转富信（wap方式通知），短转运营富
                           7: RT-SMS R短信D（短转富，互联网通知下发)
                           8: RX-SMS 短信C（短转富，短信通知下发)
                           9: 拼接多拆分出来的信息（注：如果是该信息则rpt
                           10: RO-SMS 短转富信（wap方式通知），为协议层的默
                           11: RO-RMS 富信（wap方式通知），为协议层的默认方
                           12: RX-RMS 富信B（短信通知下发) （运营商的)
                           13: RT-RMS 富信C（互联网IM富信) (梦网的)
                           14: RO-RMS-Y 运营商富信通、第三方通道富信
                           15: RX-RCS 通过短信通知下发打开的富信B（梦网的)
                           */
    std::string tmpl_type; //1:动态模板 2:静态模板
    std::string sms_msgid; //补发短信的流水号
    std::string mt_exdata; //扩展字段, Base64编码
    std::string reserve;   //预留字段, Base64编码

    std::string chk_expflg; /*业务网关检测超时报告标识 :
                            0x0：不检测超时
                            0x1：检测通知报告超时；【暂未使用】
                            0x2：检测下载报告超时
                            组合示例：0x3 = (0x1 | 0x2) :
                            同时检测通知报告和下载报告超时
                            */

    std::string supp_privilige;     /*用户补发短信权限 : 通知 0x1, 下载0x2
                                    0x1：通知报告失败补发短信权限；
                                    0x2：下载报告失败补发短信权限；
                                    组合示例：0x3 = (0x1 | 0x2) :
                                    通知报告失败或下载报告失败时补发短信
                                    */

    std::string supp_sms_state;     /*已补发短信状态
                                    0：未补发
                                    1：通知报告失败已补发
                                    2：下载报告失败已补发
                                    3：通知报告超时已补发
                                    4：下载报告超时已补发
                                    */

    std::string send_notice_rpt;	/*
                                    0：未返回通知报告给用户
                                    1：已返回成功通知报告给用户
                                    2：已返回失败通知报告给用户
                                    */

    std::string send_dld_rpt;       /*	
                                    0：未返回下载报告给用户
                                    1：已返回成功下载报告给用户
                                    2：已返回失败下载报告给用户
                                    */

    std::string agent_login_uid;   //代理账号ID

    std::string send_suppsms_rpt;   /*
                                    0：未返回富补短报告给用户
                                    1：已返回成功富补短报告给用户
                                    2：已返回失败富补短报告给用户
                                    */

    std::string adsneedrpt;	        /*推送状态报告给广告平台内部标识
                                    0：不推状态报告给广告平台
                                    1：推状态报告给广告平台*/

    std::string adtailty;           /*广告贴尾类型
                                     0：未贴尾
                                     1：短贴短
                                     2：短贴富
                                     3：富贴富*/

    std::string supp_msg;           //补发短信内容，Base64编码
    std::string msg_src_ip_port;    //用户的ip和端口
};


//富补富下行消息缓存
class rms2rms_cache
{
public:
    std::string tmpl_id;   //模板ID
    std::string msg_type;  /*
                           0: 短信
                           1: 闪信
                           6: RO-SMS-Y 短转富信（wap方式通知），短转运营富
                           7: RT-SMS R短信D（短转富，互联网通知下发)
                           8: RX-SMS 短信C（短转富，短信通知下发)
                           9: 拼接多拆分出来的信息（注：如果是该信息则rpt
                           10: RO-SMS 短转富信（wap方式通知），为协议层的默
                           11: RO-RMS 富信（wap方式通知），为协议层的默认方
                           12: RX-RMS 富信B（短信通知下发) （运营商的)
                           13: RT-RMS 富信C（互联网IM富信) (梦网的)
                           14: RO-RMS-Y 运营商富信通、第三方通道富信
                           15: RX-RCS 通过短信通知下发打开的富信B（梦网的)
                           */
    std::string tmpl_type; //1:动态模板 2:静态模板
    std::string agent_login_uid;   //代理账号ID
    std::string content;   //富补富内容， Base64编码
    std::string supptime;  //富补富时效
    std::string suppcnt;   //富补富最大次数
    std::string usedcnt;   //富补富已补发次数

    std::string supp_sms_timeout; //富补短补发超时时间
    std::string supp_rms_timeout; //富补富补发超时时间
    std::string last_mt_msgid;  //富补富最近一次下行消息id
    std::string last_notice_rpt; //0：通知报告未收到 1：通知报告已收到
    std::string last_dld_rpt;  //0：下载报告未收到 1：下载报告已收到
    std::string simulate_type; //0：非模拟报告 1：模拟报告 
    std::string has_sent_ntc;  //0:通知报告没有推送给用户 1:通知报告已推送给用户
    std::string has_sent_dld; //0:下载报告没有推送给用户 1:下载报告已推送给用户
    std::string suppstop; //0：正常补发 1：停止补发
    std::string sms_supp_flag; //0：富补短没有触发 1：富补短已触发
    std::string last_delay_rpt; /* 0：无待补发的下载报告
                                   1：有待补发的下载报告
                                   2：通知报告流程已补发该下载报告
                                   3：下载报告流程已发该下载报告
                                */
    std::string state;  //发送短信的应答结果
    std::string createtm; //记录创建时间戳，单位：秒
};


class rpt_cache {
public:
    std::string msgid; //消息流水号
    std::string userid;//账号ID
    std::string custid;//自定义流水号
    std::string pknum;//当前条数
    std::string pktotal;//总条数
    std::string mobile; //用户手机号
    std::string spno; //通道号
    std::string exno; //扩展号
    std::string stime; //发送时间
    std::string rtime; //返回时间
    std::string type; //信息类型
    std::string status; //接收状态
    std::string errcode; //错误代码
    std::string errdesc; //错误代码描述
    std::string exdata; //自定义扩展数据
};


//操作redis业务接口
class MRedisDB
{
public:
    MRedisDB();
    ~MRedisDB();

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

    //ping redis 服务器
    bool PingRedisDB();
public:
    //////////////////////////////////////////////////////////////////////////
    /*富信下行消息REDIS缓存
      缓存类型:hash set
      持久化:yes
      主键:mtrms:{rms_msgid}
    */

    /************************************************************************
    *  功能:插入富信下行消息缓存
    *  参数:rmsMsgid：富信流水号 rmsDetail:待插入的富信下行消息缓存 error:错误描述
    *  @return 0:成功; 非0:错误码
    ************************************************************************/
    int InsertMtRms(const std::string& rmsMsgid, const mt_cache_rms& rmsDetail, std::string& error);
	int InsertMtRms(const std::string& rmsMsgid, const mt_cache_rms& rmsDetail, std::string& error, const int8_t n8Count);

    /************************************************************************
    *  功能:查询富信下行消息缓存
    *  参数:rmsMsgid：富信流水号 rmsDetail:返回的富信下行消息缓存
    *  @return 0:成功; -3:数据不存在 其它:错误码
    ************************************************************************/
    int QueryMtRms(const std::string& rmsMsgid, mt_cache_rms& rmsDetail, std::string& error);
	int QueryMtRms(const std::string& rmsMsgid, mt_cache_rms& rmsDetail, std::string& error,const int8_t n8Count);

  
    /************************************************************************
    *  功能:"缓存中富信失败业务网关已补发短信状态"与cmpSmsState相等则更新；否则不更新
    *  参数:rmsMsgid:富信流水号 cmpSmsState:待比较的状态 newSuppSmsState:设置的新状态 
    *  @return 0:未更新; 1:已更新; 其它:错误码
    ************************************************************************/
    int CheckAndSetSuppedSmsState(const std::string& rmsMsgid, const std::string& cmpSmsState, const std::string& newSuppSmsState, std::string& error);
	int CheckAndSetSuppedSmsState(const std::string& rmsMsgid, const std::string& cmpSmsState, const std::string& newSuppSmsState, std::string& error, const int8_t n8Count);


    /************************************************************************
    *  功能:标识已收到rdn下载报告
    *  参数:rmsMsgid:富信流水号
    *  @return 0:未更新; 1:已更新; 其它:错误码
    ************************************************************************/
    int MarkRecivedRdnDldRpt(const std::string& rmsMsgid, std::string& error);
	int MarkRecivedRdnDldRpt(const std::string& rmsMsgid, std::string& error, const int8_t n8Count);


    /************************************************************************
    *  功能:设置有效期
    *  参数:rmsMsgid:富信流水号 expireMilSec:有效期，单位：毫秒
    *  @return >0:成功; 0：该key不存在;  <0: 出错
    ************************************************************************/
    int ExpireMtRms(const std::string& rmsMsgid, const int& expireMilSec, std::string& error);
	int ExpireMtRms(const std::string& rmsMsgid, const int& expireMilSec, std::string& error, const int8_t n8Count);


    /************************************************************************
    *  功能:设置是否已返回通知报告给用户
    *  参数 noticeRptState: 
    *          "0"：未返回通知报告给用户
    *          "1"：已返回成功通知报告给用户
    *          "2"：已返回失败通知报告给用户
    *  @return 0:成功; 非0:错误码
    ************************************************************************/
    int MarkSendNoticeRptState(const std::string& rmsMsgid, const std::string& noticeRptState, std::string& error);


    /************************************************************************
    *  功能:设置是否已返回下载报告给用户
    *  参数 state:
    *            "0"：未返回下载报告给用户
    *            "1"：已返回成功下载报告给用户
    *            "2"：已返回失败下载报告给用户
    *  @return 0:成功; 非0:错误码
    ************************************************************************/
    int MarkSendDldRptState(const std::string& rmsMsgid, const std::string& dldRptState, std::string& error);

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
    int MarkSendRptState(const std::string& rmsMsgid, const std::string& noticeRptState, const std::string& dldRptState, std::string& error);

public:
    //////////////////////////////////////////////////////////////////////////
    /*富信下行消息缓存过期队列
      缓存类型:zset
      持久化:Yes
      主键:rms_expirelist
    */

    /************************************************************************
    *  功能:插入富信流水号到RDN下载超时队列
    *  参数:rmsMsgid:富信流水号
    *  @return
    *  0：表示一个也未添加，可能因为该成员已经存在于有序集中
    *  <0：表示出错或 key 对象非有序集对象
    *  >0：新添加的成员数量
    ************************************************************************/
    int InsertRmsExpireQueue(const std::string& rmsMsgid, const int timeoutSec, std::string& error);
	int InsertRmsExpireQueue(const std::string& rmsMsgid, const int timeoutSec, std::string& error, const int8_t n8Count);

    /************************************************************************
    *  功能:批量扫描RDN下载超时的富信流水号列表
    *  参数:nBatchSize:一次性获取的最大个数,1~1000 rmsMsgidList:返回的超时的富信流水号列表
    *  @return 0:成功; 非0:错误码
    ************************************************************************/
    int ScanRmsExpireQuque(const int nBatchSize, std::vector<std::string>& rmsMsgidList, std::string& error);


    /************************************************************************
    *  功能:从RDN下载超时队列批量移除富信流水号
    *  参数 rmsMsgidList:待移除的富信流水号列表
    *  @return <0 出错; 0 表示该有序集不存在或成员不存在; > 0 表示成功删除的成员数量
    ************************************************************************/
    int RemoveExpireQuque(const std::vector<std::string>& rmsMsgidList, std::string& error);
	int RemoveExpireQuque(const std::vector<std::string>& rmsMsgidList, std::string& error, const int8_t n8Count);


public:
    //////////////////////////////////////////////////////////////////////////
    //富补富
    
    /************************************************************************
    *  功能:插入富补富下行消息缓存
    *  参数:rmsMsgid：富信流水号 rms2rmsDetail:待插入富补富下行消息缓存 error:错误描述
    *  @return 0:成功; 非0:错误码
    ************************************************************************/
    int InsertRms2Rms(const std::string& rmsMsgid, const rms2rms_cache& rms2rmsDetail, std::string& error);
	int InsertRms2Rms(const std::string& rmsMsgid, const rms2rms_cache& rms2rmsDetail, std::string& error, const int8_t n8Count);
    int WriteRms2RmsInfo(const std::string& rmsMsgid,
        const std::map<std::string, std::string>& kvMap,
        int nRetry,
        int nRetrySleepMsec,
        std::string& error);

    /************************************************************************
    *  功能:查询富补富下行消息缓存
    *  参数:rmsMsgid：富信流水号 rms2rmsDetail:返回的富补富下行消息缓存
    *  @return 0:成功; -3:数据不存在 其它:错误码
    ************************************************************************/
    int QueryRms2Rms(const std::string& rmsMsgid, rms2rms_cache& rms2rmsDetail, std::string& error);
	int QueryRms2Rms(const std::string& rmsMsgid, rms2rms_cache& rms2rmsDetail, std::string& error, const int8_t n8Count);


    /************************************************************************
    *  功能:设置富补富下行消息缓存有效期
    *  参数:rmsMsgid:富信流水号 expireSec:有效期，单位：毫秒
    *  @return >0:成功; 0：该key不存在;  <0: 出错
    ************************************************************************/
	int ExpireRms2Rms(const std::string& rmsMsgid, const int& expireMilSec, std::string& error);
	int ExpireRms2Rms(const std::string& rmsMsgid, const int& expireMilSec, std::string& error, const int8_t n8Count);


    /************************************************************************
    *  功能:富补富已补发次数加1
    *  参数:rmsMsgid：富信流水号 error:错误描述
    *  @return 0:成功，<0:表明出错或该 key 对象非哈希对象或该域字段非整数类型
    ************************************************************************/
    int IncSuppRmsUsedCnt(const std::string& rmsMsgid, long long int* result, std::string& error);
	int IncSuppRmsUsedCnt(const std::string& rmsMsgid, long long int* result, std::string& error, const int8_t n8Count);


    /************************************************************************
    *  功能:新增一条富补富通道轨迹
    *  参数:rmsMsgid：富信流水号 error:错误描述
    *  @return >=0:新增的个数; <0:错误码
    ************************************************************************/
    int InsertSuppRmsTrack(const std::string& rmsMsgid, const std::string& gateBindId, std::string& error);
	int InsertSuppRmsTrack(const std::string& rmsMsgid, const std::string& gateBindId, std::string& error, const int8_t n8Count);


    /************************************************************************
    *  功能:设置富补富通道轨迹缓存有效期
    *  参数:rmsMsgid:富信流水号 expireSec:有效期，单位：毫秒
    *  @return >0:成功; 0：该key不存在;  <0: 出错
    ************************************************************************/
    int ExpireSuppRmsTrack(const std::string& rmsMsgid, const int& expireMilSec, std::string& error);
	int ExpireSuppRmsTrack(const std::string& rmsMsgid, const int& expireMilSec, std::string& error, const int8_t n8Count);

    /************************************************************************
    *  功能:查询富补富某条通道轨迹是否存在
    *  参数:rmsMsgid：富信流水号 error:错误描述
    *  @result:true:存在 false:不存在
    *  @return 0:成功; 非0:错误码
    ************************************************************************/
    int LookupSuppRmsTrack(const std::string& rmsMsgid, const std::string& gateBindId, bool& result, std::string& error);


    /************************************************************************
    *  功能:返回富补富所有通道轨迹列表
    *  参数:rmsMsgid：富信流水号 error:错误描述
    *  @return {int} 结果集数量，返回 <0 表示出错或有一个 key 非集合对象
    ************************************************************************/
	int QuerySuppRmsTrack(const std::string& rmsMsgid, std::set<std::string>& gateBindIdList, std::string& error);
	int QuerySuppRmsTrack(const std::string& rmsMsgid, std::set<std::string>& gateBindIdList, std::string& error, const int8_t n8Count);


    /************************************************************************
    *  功能:删除富补富下行消息和所有通道轨迹列表缓存
    *  参数:rmsMsgid：富信流水号 error:错误描述
    *  @return >=0:成功; 非0:错误码
    ************************************************************************/
    int DeleteAllRms2Rms(const std::string& rmsMsgid, std::string& error);
	int DeleteAllRms2Rms(const std::string& rmsMsgid, std::string& error, const int8_t n8Count);


    /************************************************************************
    *  功能:设置富补富最近一次下行消息id
    *  参数:rmsMsgid：富信流水号 error:错误描述
    *       lastMsgid: 最新msgid 映射关系：rmsMsgid-->lastMsgid
    *  @return >=0:成功; 非0:错误码
    ************************************************************************/
    int SetSuppRmsLastMsgId(const std::string& rmsMsgid, const std::string& lastMsgid, std::string& error);
	int SetSuppRmsLastMsgId(const std::string& rmsMsgid, const std::string& lastMsgid, std::string& error, const int8_t n8Count);

    /************************************************************************
    *  功能:获取富补富最近一次下行消息id
    *  参数:rmsMsgid：富信流水号 error:错误描述
    *       lastMsgid: 最新msgid, 映射关系：rmsMsgid-->lastMsgid
    *  @return >=0:成功; 非0:错误码
    ************************************************************************/
    int GetSuppRmsLastMsgId(const std::string& rmsMsgid, std::string& lastMsgid, std::string& error);
	int GetSuppRmsLastMsgId(const std::string& rmsMsgid, std::string& lastMsgid, std::string& error, const int8_t n8Count);
	

    /************************************************************************
    *  功能:设置富补富最近一次下行消息id有效期
    *  参数:rmsMsgid:富信流水号 expireSec:有效期，单位：毫秒
    *  @return >0:成功; 0：该key不存在;  <0: 出错
    ************************************************************************/
    int ExpireSuppRmsLastMsgId(const std::string& rmsMsgid, const int& expireMilSec, std::string& error);
	int ExpireSuppRmsLastMsgId(const std::string& rmsMsgid, const int& expireMilSec, std::string& error, const int8_t n8Count);


    /************************************************************************
    *  功能:删除富补富最近一次下行消息id
    *  参数:rmsMsgid：富信流水号 error:错误描述
    *  @return >=0:成功; 非0:错误码
    ************************************************************************/
    int DeleteSuppRmsLastMsgId(const std::string& rmsMsgid, std::string& error);
	int DeleteSuppRmsLastMsgId(const std::string& rmsMsgid, std::string& error, const int8_t n8Count);

public:
    /************************************************************************
    *  功能:网关短升富:网关M-Gate保存待处理的短升富消息到队列
    *  参数:
    *  buf:smstorms.proto.sms2rms_msg的PB二进制流
    *  slicenum:分片数量，一般设置为REDIS集群数量
    *  slotindex:返回数据保存在哪个分片
    *  error:错误描述
    *  @return >0:成功; <0:错误码
    ************************************************************************/
    int PushTodoSms2rms(const std::string& buf, const int& slicenum, uint32_t* slotindex, std::string& error);


    /************************************************************************
    *  功能:网关短升富:网关M-Gate保存待处理的短升富消息到队列的滞留量
    *  参数:
    *  slicenum:分片数量，一般设置为REDIS集群数量
    *  error:错误描述
    *  @return
    ** <0: 表示出错，不可用
    ** >=0:所有分片list元素数量总和
    ************************************************************************/
    int QueryTodoSms2rmsSize(const int& slicenum, std::string& error);


    /************************************************************************
    *  功能:网关短升富:网关M-Gate获取已处理过的短升富消息，最终发走
    *  参数:
    *  buf:smstorms.proto.sms2rms_msg的PB二进制流
    *  slicenum:分片数量，一般设置为REDIS集群数量
    *  slotindex:返回从哪个分片里读取的数据
    *  error:错误描述
    **@return
    ** >0:表示成功弹出一个元素且返回值表示元素的长度，
    ** <0:表示出错，或该对象非列表对象，或该对象已经为空
    ************************************************************************/
    int PopDoneSms2rms(std::string& buf, const int& slicenum, uint32_t* slotindex, std::string& error);


    /************************************************************************
    *  功能:网关短升富:网关M-Gate待处理的短升富消息队列的滞留量
    *  参数:
    *  slicenum:分片数量，一般设置为REDIS集群数量
    *  error:错误描述
    *  @return
    ** <0: 表示出错，不可用
    ** >=0:所有分片list元素数量总和
    ************************************************************************/
    int QueryDoneSms2rmsSize(const int& slicenum, std::string& error);

public:
    //生成网关M-Gate保存滞留的短升富消息KEY
    std::string makeSms2rmsRemainRptKey(const std::string& strSmsSpgateId);
    /************************************************************************
    *  功能:网关短升富:网关M-Gate保存滞留的短升富消息的RPT到队列
    *  参数:
    *  buf:smstorms.proto.mgate_remain_rpt的PB二进制流
    *  slicenum:分片数量，一般设置为REDIS集群数量
    *  slotindex:返回数据保存在哪个分片
    *  error:错误描述
    *  @return >0:成功; <0:错误码
    ************************************************************************/
    int PushRemainRptSms2rms(const std::string& strRemainRptKey, 
        const std::string& buf, 
        const int& slicenum, 
        uint32_t* slotindex,
        std::string& error);

    /************************************************************************
    *  功能:网关短升富:网关M-Gate获取滞留的短升富消息的RPT到队列
    *  参数:
    *  buf:smstorms.proto.mgate_remain_rpt的PB二进制流
    *  slicenum:分片数量，一般设置为REDIS集群数量
    *  slotindex:返回从哪个分片里读取的数据
    *  error:错误描述
    ** @return
    ** >0:表示成功弹出一个元素且返回值表示元素的长度，
    ** <0:表示出错，或该对象非列表对象，或该对象已经为空
    ************************************************************************/
    int PopRemainRptSms2rms(const std::string& strRemainRptKey, 
        std::string& buf, 
        const int& slicenum, 
        uint32_t* slotindex, 
        std::string& error);

public:
    //////////////////////////////////////////////////////////////////////////
    //批量任务：待发送的MT下行消息队列表
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    /* mgate:batchmsg:tosendlist:{$msglevel}
    * 参数:
    * msglevel:优先级，1~9
    */
    std::string makeBatchMtToSendListKey(const int& msglevel);

    /************************************************************************
    *  功能:网关从REDIS取走待发送的批量任务的消息
    每次都重新从级别1到级别9循环拿，拿到一个就返回
    *  参数:
    *  maxMsglevel:填消息优先级最大数字，一般为9
    *  buf:rmsmsg.proto的PB二进制流
    *  slicenum:分片数量，一般设置为REDIS集群数量
    *  slotindex:返回从哪个分片里读取的数据
    *  error:错误描述
    **@return
    ** >0:表示成功弹出一个元素且返回值表示元素的长度，
    ** <0:表示出错，或该对象非列表对象，或该对象已经为空
    ************************************************************************/
    int PopFromSendList(const int& maxMsglevel,
        std::string& buf,
        const int& slicenum,
        uint32_t* slotindex,
        std::string& error);


    /************************************************************************
    *  功能:批量任务:指定取某个消息优先级的滞留数量
    *  参数:
    *  msglevel:指定的消息优先级，1~9
    *  slicenum:分片数量，一般设置为REDIS集群数量
    *  error:错误描述
    *  @return
    ** <0: 表示出错，不可用
    ** >=0:所有分片list元素数量总和
    ************************************************************************/
    int QueryBatchMtToSendListSize(const int& msglevel, const int& slicenum, std::string& error);

private:
    int PopFromSendList(const std::string& strBatchMtToSendListKey,
        std::string& buf,
        const int& slicenum,
        uint32_t* slotindex,
        std::string& error);

public:
       
    //////////////////////////////////////////////////////////////////////////
    //企业和账号余额总条数功能
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    /* 账号余额总条数==>mgate:fee:userid:{$protype}:{$userid}
     * 企业余额总条数==>mgate:fee:ecid:{$protype}:{$ecid}
    * 参数:
    */
    std::string makeUseridFeeKey(const int& protype, const std::string& userid);
    std::string makeEcidFeeKey(const int& protype, const std::string& ecid);


    /* 功能：更新总待扣富信条数和富信实时剩余总条数
    * 参数:
    **@mgateid：待扣网关编号
    **@prenum：实时总待扣富信条数，填空本字段不更新
    **@totalnum：富信实时剩余总条数，填空本字段不更新
    **@inserttm：当前插入记录的时间戳，单位：秒
    **@invalidtm：本条记录的截止有效期的时间戳，单位：秒
    ** @return 0：成功 非0：失败
    */
    int WriteBalance(const std::string& strFeeKey,
        std::string& mgateid,
        std::string prenum,
        std::string totalnum,
        const std::string& inserttm,
        const std::string& invalidtm,
        int nRetry,
        int nRetrySleepMsec,
        std::string& error);


    /* 功能：读取企业或账号余额总条数
    *  参数:
    ** @amount：返回的余额总条数
    ** @nowtm:当前时间戳，单位：秒
    ** @nRetry:重试次数
    ** @nRetrySleepMsec:间隔时长，单位:毫秒
    **@return:
      0：成功 
      208：找不到用户计费信息 
      <0：出错
    */
    int ReadBalance(const std::string& strFeeKey,
        const int64_t& nowtm,
        int64_t& amount,
        int nRetry,
        int nRetrySleepMsec,
        std::string& error);

public:
    //////////////////////////////////////////////////////////////////////////
    //批量任务表 fep:batchmsg:{$userid}:{$taskid}
    //////////////////////////////////////////////////////////////////////////
    std::string makeBatchTaskKey(const std::string& userid,
        const std::string& taskid);

public:
    //////////////////////////////////////////////////////////////////////////
    //RPT状态报告缓存
    //////////////////////////////////////////////////////////////////////////
    std::string makeRptDetailKey(const std::string& userid,
        const std::string& msgid,
        const int& rpttype
        );

    std::string makeRptKeyByMobile(const std::string& userid,
        const std::string& mobile
        );

    std::string makeRptKeyByCustid(const std::string& userid,
        const std::string& custid
        );

    /************************************************************************
    *  功能:保存rpt
    *  参数:
       @updatedcnt:已更新过的次数 1：表示首次插入
    *  @return 0:成功, 非0:错误码
    ************************************************************************/
    int WriteRpt(const std::string& strRptKey, 
        const rpt_cache& rpt, 
        long long int& updatedcnt, 
        int nRetry,
        int nRetrySleepMsec,
        std::string& error);

    /************************************************************************
    *  功能:ZSET队列新增元素
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
    int PushToZSet(const std::string& key,
        const std::string& value,
        const double score,
        int nRetry,
        int nRetrySleepMsec,
        std::string& error);

public:
    //////////////////////////////////////////////////////////////////////////
    //通道级路由：通道当月发送明细表
    //////////////////////////////////////////////////////////////////////////
    //biz:sproute:cnt:{$YYYYMM}:{$area}:{$gateid}
    std::string makeSPRouteKey(const std::string& yyyymm,
        const std::string& area,
        const std::string& gateid
        );


    /************************************************************************
    *  功能:新增当日计数和当月总计数
    *  参数:@day：几号（取值范围：1~31)
            @inc：当日增加的值，可以为负值
            @daycnt：返回累积后当日计数存储结果值
            @totalcnt：返回累积后当月计数存储结果值
    * @return 0:成功，<0:表明出错或该 key 对象非哈希对象或该域字段非整数类型
    ************************************************************************/
    int IncSPRouteCnt(const std::string& strSPRouteKey,
        const int& dayofmonth,
        const long long int& inc,
        long long int* daycnt,
        long long int* totalcnt,
        std::string& error);

    /************************************************************************
    *  功能:读取当日计数和当月总计数
    *  参数:@day：几号（取值范围：1~31)
            @daycnt：返回当日计数存储结果值
            @totalcnt：返回当月计数存储结果值
    * @return 0：成功 非0：失败
    ************************************************************************/
    int ReadSPRouteCnt(const std::string& strSPRouteKey,
        const int& dayofmonth,
        long long& daycnt,
        long long& totalcnt,
        int nRetry,
        int nRetrySleepMsec,
        std::string& error);

public:
    /************************************************************************
    *  功能:设置有效期
    *  参数:key：redis中的key值 expireMilSec:有效期，单位：毫秒
    *  @return >0:成功; 0：该key不存在;  <0: 出错
    ************************************************************************/
    int ExpireKey(const std::string& key, const int& expireMilSec, std::string& error);
    //expireSec:秒
    int ExpireKeyBySec(const std::string& key, const int& expireSec, std::string& error);


    /************************************************************************
    *  功能:删除一组KEY
    *  @return:
    *  0: 未删除任何 KEY
    *  -1: 出错
    *  >0: 真正删除的 KEY 的个数，该值有可能少于输入的 KEY 的个数
    ************************************************************************/
    int DeleteKey(const std::vector<std::string>& keys, std::string& error);


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

private:
    std::shared_ptr<MWRedisClient> m_pRedisClient = nullptr;
};



#endif // !__MREDISDB_H__
