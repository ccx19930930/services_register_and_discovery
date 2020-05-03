#ifndef _ZK_HANDLE_H_
#define _ZK_HANDLE_H_

#include "base_class.h"

#include <set>
#include <string>

#include <pthread.h>

#include <zookeeper.h>
#include <zookeeper_log.h>
#include <zookeeper.jute.h>

using namespace std;

const int kMaxBufferLen = 4096;

class CZkHandle : public CUnCopyable
{
private:
    static pthread_mutex_t m_mutex;
    static CZkHandle* m_pins;
    CZkHandle();
public:
    static CZkHandle* GetInstance();

public:
    int ZkInit(const string& host_list, const int time_out);
    int ZkClose();
    int ZkExists(const string& path, struct Stat & stat);

public:
    int ZkCreateNode(const string& path, const string& value, bool is_sequential);
    int ZkDeleteNode(const string& path, const int version = -1);

public:
    int ZkGetChildren(const string& path, set<string>& node_list);
    int ZkGetNodeInfo(const string& path, string & info);

    int ZkWgetChildren(const string& path, watcher_fn watcher, set<string>& node_list);
    int ZkWGetNodeInfo(const string& path, watcher_fn watcher, string& info);



private:
    static void ZkInitWatcher(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx);

private:
    zhandle_t* m_zk_handle;
};

#endif
