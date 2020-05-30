#include "zk_handle.h"
#include "auto_lock.h"

#include <stdio.h>

#include <sys/prctl.h>
#include <unistd.h>
#include <pthread.h>

CZkHandle* CZkHandle::m_pins = nullptr;
pthread_mutex_t CZkHandle::m_mutex;

CZkHandle::CZkHandle()
: m_handle_check_thread_id(0)
, m_zk_handle(nullptr)
, m_is_running(false)
, m_host_list("")
, m_time_out(0)
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
    m_host_list = host_list;
    m_time_out = time_out;

    if (0 == m_handle_check_thread_id)
    {
        m_is_running = true;
        if (0 != pthread_create(&m_handle_check_thread_id, nullptr, CZkHandle::ZkHandleCheckThread, nullptr))
        {
            printf("CZkHandle::ZkInit create register check thread fail.\n");
            return -1;
        }
        printf("CZkHandle::ZkInit create register check thread succ.\n");
    }
    return 0;
}

int CZkHandle::ZkClose()
{
    if (m_zk_handle)
    {
        zookeeper_close(m_zk_handle);
        m_zk_handle = nullptr;
    }
    printf("CZkHandle::ZkClose: zk close.\n");
}

int CZkHandle::ZkExists(const string& path, struct Stat & stat)
{
    int ret_code = zoo_exists(m_zk_handle, path.c_str(), 0, &stat);
    printf("CZkHandle::ZkExists: [ret=%d]\n", ret_code);
    if(ZOK == ret_code)
    {
        printf("CZkHandle::ZkExists: [path=%s] [czxid=%ld] [mzxid=%ld] [version=%d] [cversion=%d] [child_num=%d]\n",
            path.c_str(), stat.czxid, stat.mzxid, stat.version, stat.cversion, stat.numChildren);
    }
    return ret_code;
}

int CZkHandle::ZkCreateNode(const string& path, const string& value, bool is_sequential, string& raw_node_name)
{
    int ret_code = 0;
    char tmp_name[kMaxBufferLen];
    int flag = ZOO_EPHEMERAL;
    if (is_sequential)
    {
        flag |= ZOO_SEQUENCE;
    }
    printf("CZkHandle::ZkCreateNode create node [path=%s] [value=%s]\n", path.c_str(), value.c_str());
    
    ret_code = zoo_create(m_zk_handle, path.c_str(), value.c_str(), value.size(), &ZOO_OPEN_ACL_UNSAFE, flag, tmp_name, kMaxBufferLen);
    if (ZOK != ret_code)
    {
        printf("CZkHandle::ZkCreateNode create node fail. ret=%d\n", ret_code);
    }
    else
    {
        printf("CZkHandle::ZkCreateNode create node succ! path=%s\n", tmp_name);
    }

    raw_node_name = tmp_name;
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
        printf("CZkHandle::ZkGetChildren get children fail. ret=%d\n", ret_code);
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

int CZkHandle::ZkGetNodeInfo(const string& path, string& info, struct Stat& stat)
{
    int ret_code = 0;

    char buffer[kMaxBufferLen];
    int buffer_len = kMaxBufferLen;

    printf("CZkHandle::ZkGetNodeInfo get node info for path=%s\n", path.c_str());
    ret_code = zoo_get(m_zk_handle, path.c_str(), 0, buffer, &buffer_len, &stat);

    if (ZOK != ret_code)
    {
        printf("CZkHandle::ZkGetNodeInfo get node info for path=%s fail. ret=%d\n", path.c_str(), ret_code);
        return ret_code;
    }

    buffer[buffer_len] = 0;
    printf("CZkHandle::ZkGetNodeInfo get node info for path=%s succ. buffer=%s\n", path.c_str(), buffer);
    printf("CZkHandle::ZkGetNodeInfo: [path=%s] [czxid=%ld] [mzxid=%ld] [version=%d] [cversion=%d] [child_num=%d]\n",
        path.c_str(), stat.czxid, stat.mzxid, stat.version, stat.cversion, stat.numChildren);

    info = buffer;

    return ret_code;
}

int CZkHandle::ZkWgetChildren(const string& path, watcher_fn watcher, set<string>& node_list)
{
    int ret_code = 0;
    struct String_vector children_list;

    printf("CZkHandle::ZkWgetChildren get children for path=%s\n", path.c_str());

    ret_code = zoo_wget_children(m_zk_handle, path.c_str(), watcher, NULL, &children_list);

    if (ZOK != ret_code)
    {
        printf("CZkHandle::ZkWgetChildren get children fail. ret=%d\n", ret_code);
        return ret_code;
    }

    printf("CZkHandle::ZkWgetChildren get children succ. children_num=%d\n", children_list.count);

    for (unsigned int children_idx = 0; children_idx < children_list.count; ++children_idx)
    {
        printf("CZkHandle::ZkWgetChildren children_idx=%u, children_name=%s\n", children_idx, children_list.data[children_idx]);
        node_list.insert(children_list.data[children_idx]);
    }

    deallocate_String_vector(&children_list);
    return ret_code;
}

int CZkHandle::ZkWGetNodeInfo(const string& path, watcher_fn watcher, string& info, struct Stat& stat)
{
    int ret_code = 0;

    char buffer[kMaxBufferLen];
    int buffer_len = kMaxBufferLen;

    printf("CZkHandle::ZkWGetNodeInfo get node info for path=%s\n", path.c_str());
    ret_code = zoo_wget(m_zk_handle, path.c_str(), watcher, NULL, buffer, &buffer_len, &stat);

    if (ZOK != ret_code)
    {
        printf("CZkHandle::ZkWGetNodeInfo get node info for path=%s fail. ret=%d\n", path.c_str(), ret_code);
        return ret_code;
    }

    buffer[buffer_len] = 0;
    printf("CZkHandle::ZkWGetNodeInfo get node info for path=%s succ. buffer=%s\n", path.c_str(), buffer);
    printf("CZkHandle::ZkWGetNodeInfo: [path=%s] [czxid=%ld] [mzxid=%ld] [version=%d] [cversion=%d] [child_num=%d]\n",
        path.c_str(), stat.czxid, stat.mzxid, stat.version, stat.cversion, stat.numChildren);

    info = buffer;

    return ret_code;
}

int CZkHandle::ZkSetNodeInfo(const string& path, const string& value)
{
    printf("CZkHandle::ZkSetNodeInfo set node info. path=%s value=%s\n", path.c_str(), value.c_str());
    int ret_code = zoo_set(m_zk_handle, path.c_str(), value.c_str(), value.size(), -1);
    if (ZOK != ret_code)
    {
        printf("CZkHandle::ZkSetNodeInfo set node fail.");
    }
    return ret_code;
}

int CZkHandle::AddResetHandleFn(string type, OnResetHandle_Fn func)
{
    CAutoMutexLock auto_lock(m_mutex);
    m_on_reset_handle_fn_list[type] = func;
    return 0;
}

int CZkHandle::DelResetHandleFn(string type)
{
    CAutoMutexLock auto_lock(m_mutex);
    if (m_on_reset_handle_fn_list.count(type))
    {
        m_on_reset_handle_fn_list.erase(type);
    }
    return 0;
}

void CZkHandle::ZkInitWatcher(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx)
{
    printf("CZkHandle::ZkInitWatchar: [type=%d] [state=%d] [path=%s] [watcher_ctx=%p]\n", type, state, path, watcherCtx);
}

int CZkHandle::ResetZkHandle()
{
    CAutoMutexLock auto_lock(m_mutex);
    zhandle_t* new_zk_handle = zookeeper_init(m_host_list.c_str(), &ZkInitWatcher, m_time_out, 0, nullptr, 0);
    if (nullptr == new_zk_handle)
    {
        printf("CZkHandle::ResetZkHandle: connect to zk fail.\n");
        return -1;
    }
    printf("CZkHandle::ResetZkHandle: connect to zk succ.\n");

    zhandle_t* old_zk_handle = m_zk_handle;
    m_zk_handle = new_zk_handle;

    for (const auto & func : m_on_reset_handle_fn_list)
    {
        func.second();
    }

    if (old_zk_handle)
    {
        zookeeper_close(old_zk_handle);
    }
    return 0;
}

void* CZkHandle::ZkHandleCheckThread(void* param)
{
    prctl(PR_SET_NAME, "zk_handle_check");
    while (true == CZkHandle::GetInstance()->IsRunning())
    {
        CZkHandle::GetInstance()->ZkHandleCheck();
        usleep(kZkHandleIntervalTime);
    }
    return nullptr;
}

int CZkHandle::ZkHandleCheck()
{
    struct Stat stat;
    if (m_zk_handle == nullptr)
    {
        ResetZkHandle();
    }
    else if(ZOK != ZkExists("/", stat))
    {
        ResetZkHandle();
    }
}

bool CZkHandle::IsRunning()
{
    return m_is_running;
}
