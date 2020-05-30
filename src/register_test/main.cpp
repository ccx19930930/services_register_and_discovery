#include "../zk_util/zk_handle.h"
#include "../zk_util/register.h"

#include <json.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

//伪分布式部署 host list最好以配置文件形式，此处为测试程序，暂时写死
const char* host_list = "47.113.122.217:12181,47.113.122.217:12182,47.113.122.217:12183";
const int time_out = 3000;
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "./" << argv[0] << " <conf.json>" << std::endl;
        return -1;
    }

    CZkHandle::GetInstance()->ZkInit(host_list, time_out);
    CNodeInfo node_info;

    std::ifstream ifs; 
    ifs.open(argv[1]);
    if (!ifs.good())
    {
        return -1;
    }

    Json::Value jsn_conf;
    Json::Reader jsn_reader;
    if (jsn_reader.parse(ifs, jsn_conf) == false)
    {
        return -2;
    }

    node_info.FromJson(jsn_conf);

    CRegister::GetInstance()->Init(node_info);
    CRegister::GetInstance()->Register();

    sleep(60);

    return 0;
}
