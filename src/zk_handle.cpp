#include "zk_handle.h"
#include "auto_lock.h"

#include <stdio.h>
#include <string.h>

using namespace std;

CZkHandle* CZkHandle::m_pins = nullptr;
pthread_mutex_t CZkHandle::m_mutex;

CZkHandle::CZkHandle()
: m_zk_handle(nullptr)
{
}

CZkHandle* CZkHandle::GetInstance()
{
    if (m_pins == nullptr)
    {
        CAutoMutexLock auto_lock(m_mutex);
        if (m_pins == nullptr)
        {
            m_pins = new CZkHandle;
        }
    }
    return m_pins;
}

int CZkHandle::ZkInit(const char* host_list, const int time_out)
{
    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
    m_zk_handle = zookeeper_init(host_list, &ZkInitWatchar, time_out, 0, nullptr, 0);
    if (nullptr == m_zk_handle)
    {
        printf("CZkHandle::ZkInit: connect to zk fail.\n");
        return -1;
    }
    printf("CZkHandle::ZkInit: connect to zk succ.\n");
    return 0;
}

int CZkHandle::ZkClose()
{
    zookeeper_close(m_zk_handle);
    printf("CZkHandle::ZkClose: zk close.\n");
}

int CZkHandle::ZkExists(const char* path, struct Stat & stat)
{
    printf("CZkHandle::ZkExists: ==========BEGIN============\n");
    int ret_code = zoo_exists(m_zk_handle, path, 0, &stat);
    printf("CZkHandle::ZkExists: [ret=%d]\n", ret_code);
    if(ZOK == ret_code)
    {
        printf("CZkHandle::ZkExists: [path=%s] [czxid=%ld] [mzxid=%ld] [version=%d] [cversion=%d] [child_num=%d]\n",
            path, stat.czxid, stat.mzxid, stat.version, stat.cversion, stat.numChildren);
    }
    printf("CZkHandle::ZkExists: ==========END============\n");
    return ret_code;
}

int CZkHandle::ZkCreateNode(const char* path, const char* value, bool is_sequential)
{
    int ret_code = 0;
    char tmp_name[1024];
    int flag = ZOO_EPHEMERAL;
    if (is_sequential)
    {
        flag |= ZOO_SEQUENCE;
    }
    printf("CZkHandle::ZkCreateNode create node [path=%s] [value=%s]\n", path, value);
    
    ret_code = zoo_create(m_zk_handle, path, value, strlen(value), &ZOO_OPEN_ACL_UNSAFE, flag, tmp_name, 1024);
    if (ZOK != ret_code)
    {
        printf("CZkHandle::ZkCreateNode create node fail. ret=%d\n", ret_code);
    }
    else
    {
        printf("CZkHandle::ZkCreateNode create node succ! path=%s\n", tmp_name);
    }
    return ret_code;
}

int CZkHandle::ZkDeleteNode(const char* path, const int version /*= -1*/)
{
    int ret_code = zoo_delete(m_zk_handle, path, version);
    printf("CZkHandle::ZkDeleteNode delete node path=%s version=%d ret=%d\n", path, version, ret_code);
    return ret_code;
}

void CZkHandle::ZkInitWatchar(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx)
{
    printf("CZkHandle::ZkInitWatchar: [type=%d] [state=%d] [path=%s] [watcher_ctx=%p]\n", type, state, path, watcherCtx);
}
