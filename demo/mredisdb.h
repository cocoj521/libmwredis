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
#include "mw/mwredislib.h"


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

    std::string send_dld_rpt;       /*	0：未返回下载报告给用户
                                    1：已返回成功下载报告给用户
                                    2：已返回失败下载报告给用户
                                    */
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
        bool bStdoutOpen);

    //反初始化
    void DestoryRedisDB();

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

    /************************************************************************
    *  功能:查询富信下行消息缓存
    *  参数:rmsMsgid：富信流水号 rmsDetail:返回的富信下行消息缓存
    *  @return 0:成功; -3:数据不存在 其它:错误码
    ************************************************************************/
    int QueryMtRms(const std::string& rmsMsgid, mt_cache_rms& rmsDetail, std::string& error);

  
    /************************************************************************
    *  功能:"缓存中富信失败业务网关已补发短信状态"与cmpSmsState相等则更新；否则不更新
    *  参数:rmsMsgid:富信流水号 cmpSmsState:待比较的状态 newSuppSmsState:设置的新状态 
    *  @return 0:未更新; 1:已更新; 其它:错误码
    ************************************************************************/
    int CheckAndSetSuppedSmsState(const std::string& rmsMsgid, const std::string& cmpSmsState, const std::string& newSuppSmsState, std::string& error);


    /************************************************************************
    *  功能:标识已收到rdn下载报告
    *  参数:rmsMsgid:富信流水号
    *  @return 0:未更新; 1:已更新; 其它:错误码
    ************************************************************************/
    int MarkRecivedRdnDldRpt(const std::string& rmsMsgid, std::string& error);


    /************************************************************************
    *  功能:设置有效期
    *  参数:rmsMsgid:富信流水号 expireSec:有效期，单位：毫秒
    *  @return >0:成功; 0：该key不存在;  <0: 出错
    ************************************************************************/
    int ExpireMtRms(const std::string& rmsMsgid, const int& expireSec, std::string& error);


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
    *  @return 0:成功; 非0:错误码
    ************************************************************************/
    int InsertRmsExpireQueue(const std::string& rmsMsgid, const int timeoutSec, std::string& error);

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

public:
    //////////////////////////////////////////////////////////////////////////
    /*富信失败补发短信映射关系表
      缓存类型:hash set
      持久化:yes
      主键:sms2rms:{sms_msgid}
    */

    /************************************************************************
    *  功能:保存补发的短信流水号对应的富信流水号
    *  参数:smsMsgid:短信流水号 rmsMsgid:富信流水号
    *  @return 0:成功; 非0:错误码
    ************************************************************************/
    int InsertSms2Rms(const std::string& smsMsgid, const std::string& rmsMsgid, std::string& error);


    /************************************************************************
    *  功能:查询补发的短信流水号对应的富信流水号
    *  参数:smsMsgid:短信流水号 rmsMsgid:返回的富信流水号
    *  @return 0:成功; -3:数据不存在 其它:错误码
    ************************************************************************/
    int QuerySms2Rms(const std::string& smsMsgid, std::string& rmsMsgid, std::string& error);


    /************************************************************************
    *  功能:设置有效期
    *  参数:smsMsgid:短信流水号 expireSec:有效期，单位：毫秒
    *  @return >0:成功; 0：该key不存在;  <0: 出错
    ************************************************************************/
    int ExpireSms2Rms(const std::string& rmsMsgid, const int& expireSec, std::string& error);

private:
    /************************************************************************
    *  功能:设置有效期
    *  参数:key：redis中的key值 expireSec:有效期，单位：毫秒
    *  @return >0:成功; 0：该key不存在;  <0: 出错
    ************************************************************************/
    int ExpireKey(const std::string& key, const int& expireSec, std::string& error);

private:
    MWRedisClient* m_pRedisClient = nullptr;
};



#endif // !__MREDISDB_H__
