// demo.cpp : 定义控制台应用程序的入口点。
//

#if defined(_WIN32) || defined(_WIN64)
#include "stdafx.h"
#endif

#include <assert.h>
#include "redistest.h"

extern int do_test(int argc, char* argv[]);
extern int test_mredisdb(int argc, char* argv[]);
extern int test_wbsredisdb(int argc, char* argv[]);
extern int test_set2getdb(int argc, char* argv[]);
extern int test_combizredisdb(int argc, char* argv[]);


/*
**官网：https://github.com/acl-dev/acl/
**ACL-REDIS示例：https://github.com/acl-dev/acl/tree/master/lib_acl_cpp/samples/redis
**官方示例:https://zsxxsz.iteye.com/blog/2184744
**依赖库：acllib.a、aclredis.a
**demo author：Peter Hu
*/

int main(int argc, char* argv[])
{
    std::map<std::string, std::string> testMap;
    char buf[10] = { 0 };
    buf[0] = 'A';
    buf[1] = 0;
    buf[2] = 'B';

    char buf2[10] = { 0 };
    buf2[0] = 'A';
    buf2[1] = 0;
    buf2[2] = 'B';

    testMap.insert(std::make_pair("content1", (char*)buf));
    testMap.insert(std::make_pair("content2", std::string((char*)buf2, 3)));
    
    std::string s1 = testMap["content1"];
    std::string s2 = testMap["content2"];
    if (s1 == s2) {
        printf("It's equal");
    }
    else { printf("It's not equal"); }
   

    /*
    Foo f;
    size_t len1 = sizeof(f);

    std::string s;
    size_t lens = sizeof(s);

    Foo2 f2;
    size_t len2 = sizeof(f2);

    Foo f3;
    f3.s = "11111111111111111111111111111111111111111111111111111abccdfdafdfdfdfdffdasfadfadsfdasfadsfdafdasfkdajfdasjfidfdjfaidjfiafj1";
    size_t len3 = sizeof(f3);

    Foo* p = &f;
    Foo* p3 = &f3;
    */

    //return test_combizredisdb(argc, argv);

    //int ret = doTestRedis(argc, argv);

    //return ret;
    
    //return test_wbsredisdb(argc, argv);
    //return do_test(argc, argv);
    return test_mredisdb(argc, argv);
    //return test_sharedlist(argc, argv);
#ifdef _WIN32
    getchar();
#endif
}

