#include "zk_handle.h"

#include <unistd.h>

//α�ֲ�ʽ���� host list����������ļ���ʽ���˴�Ϊ���Գ�����ʱд��
const char* host_list = "127.0.0.1:12181,127.0.0.1:12182,127.0.0.1:12183";
const int time_out = 50000;
int main()
{
    CZkHandle* zk_handle = CZkHandle::GetInstance();
    struct Stat stat;

    zk_handle->ZkInit(host_list, time_out);
    zk_handle->ZkExists("/", stat);

    zk_handle->ZkCreateNode("/test_1", "1", true);
    zk_handle->ZkCreateNode("/test_1", "1", true);
    zk_handle->ZkCreateNode("/test_2", "2", false);
    zk_handle->ZkCreateNode("/test_2", "2", false);

    sleep(10);
    zk_handle->ZkDeleteNode("/test_2");

    sleep(10);

    zk_handle->ZkClose();

    return 0;
}
