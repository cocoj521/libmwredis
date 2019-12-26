/**
 * Copyright (C) 2017-2018
 * All rights reserved.
 *
 * AUTHOR(S)
 *   Zheng Shuxin
 *   E-mail: zhengshuxin@qiyi.com
 * 
 * VERSION
 *   Tue 08 Aug 2017 02:11:05 PM CST
 */

#pragma once
#include "../acl_cpp_define.hpp"

struct iovec;

namespace acl
{

class socket_stream;

/**
 * tcp ipc ͨ�ŷ����࣬�ڲ��Զ����
 */
class ACL_CPP_API tcp_sender
{
public:
	tcp_sender(socket_stream& conn);
	~tcp_sender(void);

	/**
	 * ���ͷ���
	 * @param data {const void*} Ҫ���͵����ݰ���ַ
	 * @param len {unsigned int} ���ݰ�����
	 * @return {bool} �����Ƿ�ɹ�
	 */
	bool send(const void* data, unsigned int len);

	/**
	 * �������������
	 * @return {acl::socket_stream&}
	 */
	acl::socket_stream& get_conn(void) const
	{
		return *conn_;
	}

private:
	acl::socket_stream* conn_;
	struct iovec* v2_;
};

} // namespace acl
