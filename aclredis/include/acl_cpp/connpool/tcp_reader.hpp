/**
 * Copyright (C) 2017-2018
 * All rights reserved.
 *
 * AUTHOR(S)
 *   Zheng Shuxin
 *   E-mail: zhengshuxin@qiyi.com
 * 
 * VERSION
 *   Tue 08 Aug 2017 01:57:39 PM CST
 */

#pragma once

namespace acl
{

class socket_stream;
class string;

/**
 * tcp ipc ͨ�Ž����࣬�ڲ����Զ���ȡ���µ����ݰ�
 */
class ACL_CPP_API tcp_reader
{
public:
	tcp_reader(socket_stream& conn);
	~tcp_reader(void) {}

	/**
	 * �ӶԶ˶�ȡ���ݣ�ÿ��ֻ��һ�����ݰ�
	 * @param out {string&} �洢���ݰ����ڲ�����׷�ӷ�ʽ�� out �������
	 */
	bool read(string& out);

	/**
	 * �������������
	 * @return {acl::socket_stream&}
	 */
	acl::socket_stream& get_conn(void) const
	{
		return *conn_;
	}

private:
	socket_stream* conn_;
};

} // namespace acl
