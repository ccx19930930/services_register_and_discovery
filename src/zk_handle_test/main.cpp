#include "zk_handle.h"

#include <unistd.h>

//伪分布式部署 host list最好以配置文件形式，此处为测试程序，暂时写死
const char* host_list = "47.113.122.217:12181,47.113.122.217:12182,47.113.122.217:12183";
const int time_out = 50000;
int main()
{
    CZkHandle* zk_handle = CZkHandle::GetInstance();
    struct Stat stat;
    set<string> node_list;
    string node_info;
    string raw_name;

    zk_handle->ZkInit(host_list, time_out);
    sleep(1);
    zk_handle->ZkExists("/", stat);

    zk_handle->ZkCreateNode("/test_1", "1", true, raw_name);
    zk_handle->ZkCreateNode("/test_1", "1", true, raw_name);
    zk_handle->ZkCreateNode("/test_2", "2", false, raw_name);
    zk_handle->ZkCreateNode("/test_2", "2", false, raw_name);

    zk_handle->ZkGetNodeInfo("/test_2", node_info, stat);
    zk_handle->ZkSetNodeInfo("/test_2", "3");
    zk_handle->ZkGetNodeInfo("/test_2", node_info, stat);


    node_list.clear();
    zk_handle->ZkGetChildren("/", node_list);

    zk_handle->ZkDeleteNode("/test_2");
    node_list.clear();
    zk_handle->ZkGetChildren("/", node_list);

    sleep(60);

    zk_handle->ZkClose();
    return 0;
}
