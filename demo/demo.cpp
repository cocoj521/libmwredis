// demo.cpp : 定义控制台应用程序的入口点。
//

#if defined(_WIN32) || defined(_WIN64)
#include "stdafx.h"
#endif

#include <assert.h>

extern int do_test(int argc, char* argv[]);
extern int test_mredisdb(int argc, char* argv[]);
extern int test_set2getdb(int argc, char* argv[]);


/*
**官网：https://github.com/acl-dev/acl/
**ACL-REDIS示例：https://github.com/acl-dev/acl/tree/master/lib_acl_cpp/samples/redis
**官方示例:https://zsxxsz.iteye.com/blog/2184744
**依赖库：acllib.a、aclredis.a
**demo author：Peter Hu
*/

int main(int argc, char* argv[])
{
    //return test_set2getdb(argc, argv);
    //return test_mredisdb(argc, argv);
    return do_test(argc, argv);

    /*
    do 
    {
        //////////////////////////////////////////////////////////////////////////
        //初始化redis连接池
//        std::string redis_cluster = "192.169.0.163:6379"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
//        std::string redis_pwd = "nking"; //redis访问密码，默认为空

        std::string redis_cluster = "192.169.0.60:7000"; //redis集群地址，多地址之间用英文逗号隔开。如:192.169.1.22:6379,192.169.1.23:6380
        std::string redis_pwd = ""; //redis访问密码，默认为空
        int nConnTimeout = 30; //连接超时时间，默认为30秒,最大值60
        int nRWTimeout = 30;  //读写超时时间，默认为30秒,最大值60
        int nRetryLimit = 15; //设置重定向的最大阀值，若重定向次数超过此阀值则报错,最大值30
        int nRetryInterval = 1; //当某个连接池结点出问题，设置探测该连接结点是否恢复的时间间隔(秒)，当该值为 0 时，则不检测
        int nRetrySleep = 500; //当重定向次数 >= 2 时每次再重定向此函数设置休息的时间(毫秒)，默认为500毫秒
        int nPoolSize = 4; // 最大线程数，默认为4


        RedisHelper redisClient;
        int ret = redisClient.initRedis(redis_cluster, redis_pwd,
            nConnTimeout, nRWTimeout,
            nRetryLimit, nRetryInterval,
            nRetrySleep, nPoolSize, true, false);
        if (0 != ret)
        {
            fprintf(stderr, "failed to init redis, ret:%d\n", ret);
            break;
        }

        //存数据
        std::string v1 = "peter";
        ret = redisClient.set("name", v1);
        if (0 != ret)
        {
            fprintf(stderr, "failed to set, ret:%d\n", ret);
            break;
        }

        //取数据
        std::string v2;
        ret = redisClient.get("name", v2);
        if (0 != ret)
        {
            fprintf(stderr, "failed to get, ret:%d\n", ret);
            break;
        }

        //验证相等
        assert(v1 == v2);
        printf("v1:%s, v2:%s, result:%s", v1.c_str(), v2.c_str(), v1==v2? "Equal":"Not equal");
    } while (0);
    */

    //getchar();
	return 0;
}

//void* xmalloc(int n) {
//    return new char[n];
//}
//
//void xfree(void* p) {
//    delete[] p;
//}
//
//#include <map>
//template< typename T > struct myallocator : public std::allocator<T>
//{
//    typedef std::allocator<T> base;
//    typedef typename base::size_type size_type;
//    typedef typename base::difference_type  difference_type;
//    typedef typename base::pointer pointer;
//    typedef typename base::const_pointer const_pointer;
//    typedef typename base::reference reference;
//    typedef typename base::const_reference const_reference;
//    typedef typename base::value_type value_type;
//    myallocator() throw() {}
//    myallocator(const myallocator& a) throw() : base(a) {}
//    template <typename X> myallocator(
//        const myallocator<X>&) throw() {}
//    ~myallocator() throw() {}
//    template <typename X> struct rebind
//    {
//        typedef myallocator<X> other;
//    };
//    pointer allocate(size_type sz, const void* hint = 0)
//    {
//        printf("allocate sz:%u\n", sz);
//        // record alloc request eg. ++num_allocs ;
//        return base::allocate(sz, hint);
//        //return  (pointer)::operator new(sizeof(pointer));
//        //return (pointer)malloc(sizeof(pointer));
//        //return reinterpret_cast<T*> (xmalloc(sz*sizeof(T)));
//    }
//    void deallocate(pointer p, size_type n)
//    {
//        // record dealloc request eg. --num_allocs ;
//        printf("deallocate n:%u\n", n);
//        return base::deallocate(p, n);
//        //delete[] p;
//        //free((void*)p);
//        //xfree(p);
//    }
//};
//template<typename T> inline bool operator==(
//    const myallocator<T>&, const myallocator<T>&)
//{
//    return true;
//}
//template<typename T> inline bool operator!=(
//    const myallocator<T>&, const myallocator<T>&)
//{
//    return false;
//}
//int main2()
//{
//    std::map< int, float, std::less<int>,
//        myallocator<int> > testmap;
//    for (int i = 0; i < 10000; i++)
//    {
//        testmap[i] = (float)i;
//    }
//
//    for (int i = 10000; i < 20000; i++)
//    {
//        testmap[i] = (float)i;
//    }
//    testmap.clear();
//    getchar();
//    return 0;
//}