#include "../zk_util/zk_handle.h"
#include "../zk_util/discovery.h"

#include <unistd.h>

//伪分布式部署 host list最好以配置文件形式，此处为测试程序，暂时写死
const char* host_list = "47.113.122.217:12181,47.113.122.217:12182,47.113.122.217:12183";
const int time_out = 3000;
int main()
{
    CZkHandle::GetInstance()->ZkInit(host_list, time_out);

    set<string> down_path_list;
    down_path_list.insert("/zk_test1");
    down_path_list.insert("/zk_test2");

    set<int> down_service_list;
    down_service_list.insert(1);
    down_service_list.insert(2);

    CDiscovery::GetInstance()->Init(down_path_list, down_service_list);
    CDiscovery::GetInstance()->StartCheck();

    sleep(60);

    return 0;
}
