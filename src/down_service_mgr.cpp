#include "down_service_mgr.h"
#include "auto_lock.h"

int CDownServiceMgr::Register(const string& zk_path, CNodeInfo* node_info)
{
    CAutoMutexLock auto_lock(m_mutex);
    if (m_node_list.count(zk_path))
    {
        return -1;
    }
    m_node_list[zk_path] = node_info;
    //TODO 长连接等

    return 0;
}

int CDownServiceMgr::UnRegister(const string& zk_path)
{
    CAutoMutexLock auto_lock(m_mutex);
    if (m_node_list.count(zk_path) == 0)
    {
        return -1;
    }
    //TODO 长连接等

    m_node_list.erase(zk_path);
    return 0;
}

