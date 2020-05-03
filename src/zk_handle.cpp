#include "zk_handle.h"
#include "auto_lock.h"

#include <stdio.h>

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

int CZkHandle::ZkInit(const string& host_list, const int time_out)
{
    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
    m_zk_handle = zookeeper_init(host_list.c_str(), &ZkInitWatchar, time_out, 0, nullptr, 0);
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

int CZkHandle::ZkExists(const string& path, struct Stat & stat)
{
    printf("CZkHandle::ZkExists: ==========BEGIN============\n");
    int ret_code = zoo_exists(m_zk_handle, path.c_str(), 0, &stat);
    printf("CZkHandle::ZkExists: [ret=%d]\n", ret_code);
    if(ZOK == ret_code)
    {
        printf("CZkHandle::ZkExists: [path=%s] [czxid=%ld] [mzxid=%ld] [version=%d] [cversion=%d] [child_num=%d]\n",
            path.c_str(), stat.czxid, stat.mzxid, stat.version, stat.cversion, stat.numChildren);
    }
    printf("CZkHandle::ZkExists: ==========END============\n");
    return ret_code;
}

int CZkHandle::ZkCreateNode(const string& path, const string& value, bool is_sequential)
{
    int ret_code = 0;
    char tmp_name[1024];
    int flag = ZOO_EPHEMERAL;
    if (is_sequential)
    {
        flag |= ZOO_SEQUENCE;
    }
    printf("CZkHandle::ZkCreateNode create node [path=%s] [value=%s]\n", path.c_str(), value.c_str());
    
    ret_code = zoo_create(m_zk_handle, path.c_str(), value.c_str(), value.size(), &ZOO_OPEN_ACL_UNSAFE, flag, tmp_name, 1024);
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

int CZkHandle::ZkDeleteNode(const string& path, const int version /*= -1*/)
{
    int ret_code = zoo_delete(m_zk_handle, path.c_str(), version);
    printf("CZkHandle::ZkDeleteNode delete node path=%s version=%d ret=%d\n", path.c_str(), version, ret_code);
    return ret_code;
}

int CZkHandle::ZkGetChildren(const string& path, set<string>& node_list)
{
    int ret_code = 0;
    struct String_vector children_list;

    printf("CZkHandle::ZkGetChildren get children for path=%s\n", path.c_str());

    ret_code = zoo_get_children(m_zk_handle, path.c_str(), 0, &children_list);

    if (ZOK != ret_code)
    {
        printf("CZkHandle::ZkGetChildren get children fail.\nj");
        return ret_code;
    }
    
    printf("CZkHandle::ZkGetChildren get children succ. children_num=%d\n", children_list.count);

    for (unsigned int children_idx = 0; children_idx < children_list.count; ++children_idx)
    {
        printf("CZkHandle::ZkGetChildren children_idx=%u, children_name=%s\n", children_idx, children_list.data[children_idx]);
        node_list.insert(children_list.data[children_idx]);
    }

    deallocate_String_vector(&children_list);
    return ret_code;
}

void CZkHandle::ZkInitWatchar(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx)
{
    printf("CZkHandle::ZkInitWatchar: [type=%d] [state=%d] [path=%s] [watcher_ctx=%p]\n", type, state, path, watcherCtx);
}
