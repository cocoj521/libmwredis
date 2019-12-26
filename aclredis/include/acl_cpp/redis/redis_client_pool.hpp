#pragma once
#include "../acl_cpp_define.hpp"
#include "../connpool/connect_pool.hpp"

namespace acl
{

/**
 * redis ���ӳ��࣬����̳��� connect_pool���� connect_pool ������ͨ�õ��й�
 * TCP ���ӳص�ͨ�÷�����
 * redis connection pool inherting from connect_pool, which includes
 * TCP connection pool methods.
 */
class ACL_CPP_API redis_client_pool : public connect_pool
{
public:
	/**
	 * ���캯��
	 * constructor
	 * @param addr {const char*} ����˵�ַ����ʽ��ip:port
	 *  the redis-server's listening address, format: ip:port
	 * @param count {size_t} ���ӳص�������������ƣ������ֵΪ 0�������ӳ�
	 *  û���������ơ�
	 *  the max connections for each connection pool. there is
	 *  no connections limit of the pool when the count is 0.
	 * @param idx {size_t} �����ӳض����ڼ����е��±�λ��(�� 0 ��ʼ)
	 *  the subscript of the connection pool in the connection cluster
	 */
	redis_client_pool(const char* addr, size_t count, size_t idx = 0);

	virtual ~redis_client_pool(void);

	/**
	 * �������� redis ����������������
	 * @param pass {const char*} ��������
	 * @return {redis_client_pool&}
	 */
	redis_client_pool& set_password(const char* pass);

	/**
	 * �ڷǼ�Ⱥģʽ�£������������������ӽ�������ѡ���db
	 * in no-cluster mode, the method is used to select the db after
	 * the connection is created
	 * @param dbnum {int}
	 * @return {redis_client_pool&}
	 */
	redis_client_pool& set_db(int dbnum);

	/**
	 * ��ñ����ӳ�����Ӧ��db
	 * get the current db of the connections pool
	 * @return {int}
	 */
	int get_db(void) const
	{
		return dbnum_;
	}

protected:
	/**
	 * ���ി�麯��: ���ô˺�����������һ���µ�����
	 * virtual function in class connect_pool to create a new connection
	 * @return {connect_client*}
	 */
	connect_client* create_connect(void);

private:
	char* pass_;
	int   dbnum_;
};

} // namespace acl
