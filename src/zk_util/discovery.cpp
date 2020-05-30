#include "discovery.h"
#include "auto_lock.h"

#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <pthread.h>

CDiscovery* CDiscovery::m_pins = nullptr;
pthread_mutex_t CDiscovery::m_mutex;

CDiscovery::CDiscovery()
    : m_is_running(false)
    , m_down_check_thread_id(0)
{
}

CDiscovery* CDiscovery::GetInstance()
{
    if (m_pins == nullptr)
    {
        CAutoMutexLock auto_lock(m_mutex);
        if (m_pins == nullptr)
        {
            m_pins = new CDiscovery;
        }
    }
    return m_pins;
}

int CDiscovery::Init(const set<string>& down_path_list, const set<int>& down_service_list)
{
    CAutoMutexLock auto_lock(m_mutex);
    for (const auto & zk_path : down_path_list)
    {
        m_down_path_list[zk_path] = new CDownNode;
    }

    for (const auto& module_id : down_service_list)
    {
        m_down_service_list[module_id] = new CDownServiceMgr(module_id);
    }

    return 0;
}

int CDiscovery::StartCheck()
{
    if (0 == m_down_check_thread_id)
    {
        m_is_running = true;
        if (0 != pthread_create(&m_down_check_thread_id, nullptr, CDiscovery::DiscoveryCheckThread, nullptr))
        {
            printf("CDiscovery::StartCheck create discovery check thread fail.");
            return -1;
        }
        printf("CDiscovery::StartCheck create discovery check thread succ.");
    }
    return 0;
}

int CDiscovery::Stop()
{
    m_is_running = false;
}

void CDiscovery::OnZkHandleResetFunc()
{
    CDiscovery::GetInstance()->OnZkHandleReset();
}

void* CDiscovery::DiscoveryCheckThread(void* param)
{
    prctl(PR_SET_NAME, "zk_discovery_check");

    CZkHandle::GetInstance()->AddResetHandleFn("discovery", CDiscovery::OnZkHandleResetFunc);

    while (true == CDiscovery::GetInstance()->IsRunning())
    {
        CDiscovery::GetInstance()->DiscoveryCheck();
        usleep(kZkDiscoveryIntervalTime);
    }
    return nullptr;
}

int CDiscovery::DiscoveryCheck()
{
    DownPathCheck();
    InvalidNodeCheck();

#ifdef _DEBUG_
    DebugPrintAllNode();
#endif
}

int CDiscovery::DownPathCheck()
{
    printf("%s =======================================================\n", __func__);
    CAutoMutexLock auto_lock(m_mutex);
    for (const auto& down_path : m_down_path_list)
    {
        CDownNode* down_node = down_path.second;
        if (down_node->m_full_node)
        {
            continue;
        }
        set<string> node_list;
        if (ZOK == CZkHandle::GetInstance()->ZkWgetChildren(down_path.first, CDiscovery::ZkPathWatcher, node_list))
        {
            for (auto node_path : node_list)
            {
                down_node->m_invalid_node_path_list.insert(down_path.first + '/' + node_path);
            }
            for (const auto& node : down_node->m_node_list)
            {
                down_node->m_invalid_node_path_list.insert(node.first);
            }

            down_node->m_full_node = true;
        }
    }
    return 0;
}

int CDiscovery::InvalidNodeCheck()
{
    printf("%s =======================================================\n", __func__);
    CAutoMutexLock auto_lock(m_mutex);
    for (const auto& down_path : m_down_path_list)
    {
        CDownNode* down_node = down_path.second;
        for (auto it_node_path = down_node->m_invalid_node_path_list.begin(); it_node_path != down_node->m_invalid_node_path_list.end();)
        {
            struct Stat stat;
            string zk_node_info;
            int ret_code = CZkHandle::GetInstance()->ZkWGetNodeInfo(*it_node_path, CDiscovery::ZkNodeWatcher, zk_node_info, stat);
            if (ZOK == ret_code)
            {
                CNodeInfo node_info;
                node_info.FromString(zk_node_info);
                down_node->m_node_list[*it_node_path] = node_info;
                it_node_path = down_node->m_invalid_node_path_list.erase(it_node_path);
                m_down_service_list[atoi(node_info.m_module_id.c_str())]->Register(*it_node_path, &node_info);
                m_down_path_2_dir[*it_node_path] = down_path.first;
            }
            else if(ZNONODE == ret_code)
            {
                if (down_node->m_node_list.count(*it_node_path))
                {
                    CNodeInfo& node_info = down_node->m_node_list[*it_node_path];
                    m_down_service_list[atoi(node_info.m_module_id.c_str())]->UnRegister(*it_node_path);
                    down_node->m_node_list.erase(*it_node_path);
                }
                if (m_down_path_2_dir.count(*it_node_path))
                {
                    m_down_path_2_dir.erase(*it_node_path);
                }
                it_node_path = down_node->m_invalid_node_path_list.erase(it_node_path);
            }
            else
            {
                ++it_node_path;
            }
        }
    }
    return 0;
}

int CDiscovery::DebugPrintAllNode()
{
    printf("%s =======================================================\n", __func__);
    CAutoMutexLock auto_lock(m_mutex);
    for (const auto& down_path : m_down_path_list)
    {
        printf("%s down_path=%s is_full_node=%d --------------------------------------------\n", __func__, down_path.first.c_str(), down_path.second->m_full_node);
        printf("%s node_list: \n", __func__);
        for (auto& down_node : down_path.second->m_node_list)
        {
            printf("%s node=%s \n", __func__, down_node.first.c_str());
            printf("%s info=%s \n", __func__, down_node.second.ToString().c_str());
        }

        printf("%s invalid_node_list: \n", __func__);
        for (const auto& invalid_node : down_path.second->m_invalid_node_path_list)
        {
            printf("%s invalid_node:%s \n", __func__, invalid_node.c_str());
        }
    }
    return 0;
}

bool CDiscovery::IsRunning()
{
    return m_is_running;
}

void CDiscovery::ZkPathWatcher(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx)
{
    if (ZOO_CHILD_EVENT == type)
    {
        CDiscovery::GetInstance()->OnPathChange(path);
    }
}

void CDiscovery::ZkNodeWatcher(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx)
{
    if (ZOO_CHANGED_EVENT == type)
    {
        CDiscovery::GetInstance()->OnNodeChange(path);
    }
    else if (ZOO_DELETED_EVENT == type)
    {
        CDiscovery::GetInstance()->OnNodeChange(path);
    }
}

int CDiscovery::OnPathChange(string path)
{
    printf("%s path=%s  =======================================================\n", __func__, path.c_str());
    CAutoMutexLock auto_lock(m_mutex);
    if (m_down_path_list.count(path))
    {
        m_down_path_list[path]->m_full_node = false;
    }
    return 0;
}

int CDiscovery::OnNodeChange(string node)
{
    printf("%s node=%s  =======================================================\n", __func__, node.c_str());
    CAutoMutexLock auto_lock(m_mutex);
    string path;
    if (m_down_path_2_dir.count(node))
    {
        path = m_down_path_2_dir[node];
    }
    if (m_down_path_list.count(path))
    {
        m_down_path_list[path]->m_invalid_node_path_list.insert(node);
    }
    return 0;
}

void CDiscovery::OnZkHandleReset()
{
    printf("%s =======================================================\n", __func__);
    CAutoMutexLock auto_lock(m_mutex);
    for (const auto& down_path : m_down_path_list)
    {
        down_path.second->m_full_node = false;
    }
}

