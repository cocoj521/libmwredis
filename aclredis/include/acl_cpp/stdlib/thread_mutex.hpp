/**
 * Copyright (C) 2017-2018 IQIYI
 * All rights reserved.
 *
 * AUTHOR(S)
 *   Zheng Shuxin
 *   E-mail: zhengshuxin@qiyi.com
 * 
 * VERSION
 *   Tue 22 Aug 2017 11:09:55 AM CST
 */

#pragma once
#include "../acl_cpp_define.hpp"

#if !defined(_WIN32) && !defined(_WIN64)
# include <pthread.h>
# ifndef	acl_pthread_mutex_t
#  define	acl_pthread_mutex_t	pthread_mutex_t
# endif
#else
struct acl_pthread_mutex_t;
#endif

namespace acl {

/**
 * �̻߳�����
 */
class ACL_CPP_API thread_mutex
{
public:
	/**
	 * ���췽��
	 * @param recursive {bool} �Ƿ����õݹ�����ʽ
	 */
	thread_mutex(bool recursive = true);
	~thread_mutex(void);

	/**
	 * ���߳������м�����һֱ�������ɹ����ڲ�ʧ��(һ�㲻��ʧ�ܣ�������ϵͳ����)
	 * @return {bool} ���� false ˵���߳���������
	 */
	bool lock(void);

	/**
	 * �����Լ��������۳ɹ���񶼻���������
	 * @return {bool} ���� true ��ʾ�����ɹ������� false ��ʾ����ʧ��
	 */
	bool try_lock(void);

	/**
	 * ���߳���
	 * @return {bool} ���� false ��ʾ����ʧ�ܣ��п���֮ǰ��δ�����ɹ�����
	 */
	bool unlock(void);

	/**
	 * ��� acl �� C �汾��ϵͳ���͵��߳���
	 * @return {acl_pthread_mutex_t*}
	 */
	acl_pthread_mutex_t* get_mutex(void) const;

private:
	acl_pthread_mutex_t* mutex_;
#if !defined(_WIN32) && !defined(_WIN64)
	pthread_mutexattr_t  mutex_attr_;
#endif
};

class ACL_CPP_API thread_mutex_guard
{
public:
	thread_mutex_guard(thread_mutex& mutex);
	~thread_mutex_guard(void);

private:
	thread_mutex& mutex_;
};

} // namespace acl
