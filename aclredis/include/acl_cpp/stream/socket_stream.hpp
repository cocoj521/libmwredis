#pragma once
#include "../acl_cpp_define.hpp"
#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#endif
#include "istream.hpp"
#include "ostream.hpp"

struct ACL_VSTREAM;

namespace acl {

class ACL_CPP_API socket_stream
	: public istream
	, public ostream
{
public:
	socket_stream(void);
	virtual ~socket_stream(void);

	/**
	 * �����׽��ִ򿪵�һ��������
	 * @param fd �׽���
	 * @param udp_mode {bool} �Ƿ��� UDP ��ʽ
	 * @return {bool} �����Ƿ�ɹ�
	 */
#if defined(_WIN32) || defined(_WIN64)
	bool open(SOCKET fd, bool udp_mode = false);
#else
	bool open(int fd, bool udp_mode = false);
#endif

	/**
	 * ���� ACL_VSTREAM ����������
	 * @param vstream {ACL_VSTREAM*}
	 * @param udp_mode {bool} �Ƿ��� UDP ��ʽ
	 * @return {bool} �����Ƿ�ɹ�
	 */
	bool open(ACL_VSTREAM* vstream, bool udp_mode = false);

	/**
	 * ����Զ�̷�������������������
	 * @param addr {const char*} ��������ַ, ���������׽ӿڷ�����(��UNIXƽ̨),
	 *  ���׽ӵ�ַ��/tmp/test.sock; �������һ��TCP�����������ַ��ʽΪ:
	 *  [${local_ip}@]${remote_addr}, ��: 60.28.250.199@www.sina.com:80,
	 *  ��˼�ǰ󶨱���������ַΪ: 60.28.250.199, Զ������ www.sina.com �� 80,
	 *  �����OS�Զ��󶨱��� IP ��ַ�������дΪ��www.sina.com:80
	 * @param conn_timeout {int} ���ӳ�ʱʱ��(��)
	 * @param rw_timeout {int} ��д��ʱʱ��(��)
	 * @return {bool} �����Ƿ�ɹ�
	 */
	bool open(const char* addr, int conn_timeout, int rw_timeout);

	/**
	 * �󶨱��� UDP ��ַ������ UDP ����������
	 * @param addr {const char*} ������ַ����ʽ��ip:port
	 * @param rw_timeout {int} ��д��ʱʱ��(��)
	 * @param flag {unsigned}
	 * @return {bool} ���Ƿ�ɹ�
	 */
	bool bind_udp(const char* addr, int rw_timeout = 0, unsigned flag = 0);

	/**
	 * �ر��׽ӿڶ�����
	 * @return {bool}
	 */
	bool shutdown_read(void);

	/**
	 * �ر��׽ӿ�д����
	 * @return {bool}
	 */
	bool shutdown_write(void);

	/**
	 * �ر��׽ӿڶ�д����
	 * @return {bool}
	 */
	bool shutdown_readwrite(void);

	/**
	 * ����������������׽������Ӿ��
	 * @return {ACL_SOCKET} �������򷵻� - 1(UNIX ƽ̨)
	 *  �� INVALID_SOCKET(win32ƽ̨)
	 */
#if defined(_WIN32) || defined(_WIN64)
	SOCKET sock_handle(void) const;
#else
	int   sock_handle(void) const;
#endif

	/**
	 * ����׽�����������İ󶨹�ϵ��ͬʱ���׽��ַ��ظ��û�����
	 * �����׽��ֵĹ���Ȩ�����û��������������ͷ�ʱ����رո���
	 * ���֣����û��ӹܸ��׽��ֺ��������뽫��ر�
	 * ������� close/open �ĵ����������⣬�����ĵ���(��������
	 * ���д����)��������
	 * @return {ACL_SOCKET} ���� ACL_SOCKET_INVALID ��ʾ��������
	 *  �Ѿ����׽��ֽ��
	 */
#if defined(_WIN32) || defined(_WIN64)
	SOCKET unbind_sock(void);
#else
	int    unbind_sock(void);
#endif

	/**
	 * ��� socket ������
	 * @return {int} ����ֵ�У�AF_INET, AF_INT6, AF_UNIX������ʱ���� -1
	 */
	int sock_type(void) const;

	/**
	 * ���Զ�����ӵĵ�ַ
	 * @param full {bool} �Ƿ���������ַ������IP:PORT������ò���
	 *  Ϊ false��������� IP�����򷵻� IP:PORT
	 * @return {const char*} Զ�����ӵ�ַ��������ֵ == '\0' ���ʾ
	 *  �޷����Զ�����ӵ�ַ
	 */
	const char* get_peer(bool full = false) const;

	/**
	 * ���Զ�����ӵ� IP ��ַ
	 * @return {const char*} Զ�����ӵ�ַ��������ֵ == '\0' ���ʾ
	 *  �޷����Զ�����ӵ�ַ
	 */
	const char* get_peer_ip(void) const;

	/**
	 * ����Զ�����Ӷ���ĵ�ַ������ TCP ���䷽ʽ������Ҫ��ʾ���ô˺���
	 * ����Զ�̶����ַ��UDP ���䷽ʽʱ��Ҫ���ô˺�������Զ�̵�ַ��Ȼ��
	 * �ſ�����Զ������д����
	 * @param addr {const char*} Զ�����Ӷ���ĵ�ַ����ʽ��ip:port
	 * @return {bool} ��������δ��ʱ���� false
	 */
	bool set_peer(const char* addr);

	/**
	 * ������ӵı��ص�ַ
	 * @param full {bool} �Ƿ���������ַ������IP:PORT������ò���
	 *  Ϊ false��������� IP�����򷵻� IP:PORT
	 * @return {const char*} �����ӵı��ص�ַ��������ֵ == "" ���ʾ
	 *  �޷���ñ��ص�ַ
	 */
	const char* get_local(bool full = false) const;

	/**
	 * ������ӵı��� IP ��ַ
	 * @return {const char*} �����ӵı��ص�ַ��������ֵ == "" ���ʾ
	 *  �޷���ñ��ص�ַ
	 */
	const char* get_local_ip(void) const;

	/**
	 * ���ñ��ص�ַ
	 * @param addr {const char*} ��ַ����ʽ��ip:port
	 * @return {bool} ��������δ��ʱ���� false
	 */
	bool set_local(const char* addr);

	/**
	 * ����׽ӿ����ӵĴ��״̬(�ڲ�ʹ���˷��������ķ�ʽ����̽��)
	 * @return {bool} ����������δ�򿪻��Ѿ��ر�ʱ�ú������� false�����
	 *  ���������򷵻� true
	 */
	bool alive(void) const;

	/**
	 * ���� TCP �׽��ֵ� nodelay ����
	 * @param on {bool} true ��ʾ�򿪣�false ��ʾ�ر�
	 * @return {socket_stream&}
	 */
	socket_stream& set_tcp_nodelay(bool on);

	/**
	 * ���� TCP �׽��ֵ� SO_LINGER ѡ��
	 * @param on {bool} �Ƿ����� SO_LINGER ѡ��
	 * @param linger {int} ��SO_LINGER��ʱ��ȡ�� timed_wait ��ʱ�䣬��λΪ��
	 * @return {socket_stream&}
	 */
	socket_stream& set_tcp_solinger(bool on, int linger);

	/**
	 * ���� TCP �׽��ֵ�д��������С
	 * @param size {int} ���������ô�С
	 * @return {socket_stream&}
	 */
	socket_stream& set_tcp_sendbuf(int size);

	/**
	 * ���� TCP �׽��ֵĶ���������С
	 * @param size {int} ���������ô�С
	 * @return {socket_stream&}
	 */
	socket_stream& set_tcp_recvbuf(int size);

	/**
	 * ���� TCP �׽��ֵķ�����״̬
	 * @param on {bool} �Ƿ�����Ϊ������״̬����Ϊ true ʱ��
	 *  ����׽��ֱ���Ϊ������״̬������Ϊ����״̬
	 * @return {socket_stream&}
	 */
	socket_stream& set_tcp_non_blocking(bool on);

	/**
	 * ��� TCP �׽����Ƿ������� nodelay ѡ��
	 * @return {bool} true ��ʾ�򿪣�false ��ʾ�ر�
	 */
	bool get_tcp_nodelay(void);

	/**
	 * ��� TCP �׽��ֵ� linger ֵ
	 * @param fd {ACL_SOCKET} �׽���
	 * @return {int} ���� -1 ��ʾδ���� linger ѡ����ڲ�����>= 0
	 *  ��ʾ������ linger ѡ���Ҹ�ֵ��ʾ�׽��ֹرպ�� TCP �������ں���
	 *  ά�� TIME_WAIT ״̬�Ķ���ʱ��(��)
	 */
	int get_tcp_solinger(void);

	/**
	 * ��ȡ TCP �׽��ֵ�д��������С
	 * @param fd {ACL_SOCKET} �׽���
	 * @return {int} ��������С
	 */
	int get_tcp_sendbuf(void);

	/**
	 * ��ȡ TCP �׽��ֵĶ���������С
	 * @param fd {ACL_SOCKET} �׽���
	 * @return {int} ��������С
	 */
	int get_tcp_recvbuf(void);

	/**
	 * �жϵ�ǰ�׽����Ƿ������˷�����ģʽ
	 * @return {bool}
	 * ע���÷���Ŀǰ��֧�� UNIX ƽ̨
	 */
	bool get_tcp_non_blocking(void);

private:
	char  dummy_[1];
	char  peer_ip_[256];
	char  local_ip_[256];
	const char* get_ip(const char* addr, char* buf, size_t size);
};

} // namespace acl
