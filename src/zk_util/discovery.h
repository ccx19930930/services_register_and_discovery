#ifndef _DISCOVERY_H_
#define _DISCOVERY_H_

#include "base_class.h"
#include "zk_handle.h"
#include "down_service_mgr.h"

#include <zookeeper.jute.h>

class CDownNode
{
public:
    CDownNode() { Reset(); }
    ~CDownNode() {}

    void Reset()
    {
        m_full_node = false;
        m_node_info.Reset();
        m_node_list.clear();
        m_invalid_node_path_list.clear();
    }

public:
    bool m_full_node;
    CNodeInfo m_node_info;

    map<string, CNodeInfo> m_node_list;
    set<string> m_invalid_node_path_list;

};

class CDiscovery : public CUnCopyable
{
private:
    static pthread_mutex_t m_mutex;
    static CDiscovery* m_pins;
    CDiscovery();
public:
    static CDiscovery* GetInstance();
    int Init(const set<string> & down_path_list, const set<int> & down_service_list);


public:
    int StartCheck();
    int Stop();

private:
    static void* DiscoveryCheckThread(void * param);
    int DiscoveryCheck();
    int DownPathCheck();
    int InvalidNodeCheck();
    bool IsRunning();

private:
    static void ZkPathWatcher(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx);
    static void ZkNodeWatcher(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx);

private:
    int OnPathChange(string path);
    int OnNodeChange(string node);

private:
    pthread_t m_down_check_thread_id;
    bool m_is_running;
    map<string, CDownNode*> m_down_path_list; // <zk_path, down_node_info>
    map<string, string> m_down_path_2_dir;    // <zk_node, zk_node>
    map<int, CDownServiceMgr* > m_down_service_list;
};

#endif
