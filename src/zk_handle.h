#ifndef _ZK_HANDLE_H_
#define _ZK_HANDLE_H_

#include "base_class.h"

#include <pthread.h>

#include <zookeeper.h>
#include <zookeeper_log.h>
#include <zookeeper.jute.h>

class CZkHandle : public CUnCopyable
{
private:
    static pthread_mutex_t m_mutex;
    static CZkHandle* m_pins;
    CZkHandle();
public:
    static CZkHandle* GetInstance();

public:
    int ZkInit(const char* host_list, const int time_out);
    int ZkClose();
    int ZkExists(const char* path, struct Stat & stat);

    int ZkCreateNode(const char* path, const char* value, bool is_sequential);
    int ZkDeleteNode(const char* path, const int version = -1);

private:
    static void ZkInitWatchar(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx);

private:
    zhandle_t* m_zk_handle;
};

#endif
