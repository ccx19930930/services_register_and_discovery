#ifndef _REGISTER_H_ 
#define _REGISTER_H_

#include "base_class.h"

#include <zookeeper.jute.h>

class CRegister : public CUnCopyable
{
private:
    static pthread_mutex_t m_mutex;
    static CRegister* m_pins;
    CRegister();
public:
    static CRegister* GetInstance();
    int Init(const CNodeInfo & node_info);
    
public:
    int Register();
    int UnRegister();

private:
    static void* RegisterCheckThread(void * param);
    int RegisterCheck();
    bool IsRunning();

private:
    int TryCheckNode();
    int TryRegisterNode();
    int TryUnregisterNode();

private:
    pthread_t m_reg_check_thread_id;
    bool m_is_running;
    CNodeInfo m_self_info;
    struct Stat m_self_stat;
    EZkRegisterStatus m_status;
    long m_last_check_time;

private:
    string m_raw_zk_path;
};

#endif

