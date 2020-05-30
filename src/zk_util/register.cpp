#include "register.h"
#include "auto_lock.h"
#include "zk_handle.h"
#include "time_utils.h"

#include <stdio.h>

#include <sys/prctl.h>
#include <unistd.h>
#include <pthread.h>

CRegister* CRegister::m_pins = nullptr;
pthread_mutex_t CRegister::m_mutex;

CRegister::CRegister() 
    : m_is_running(false)
    , m_reg_check_thread_id(0)
    , m_status(EN_ZK_REGISTER_STATUS_UNREGISTER)
    , m_last_check_time(0)
{

}

CRegister* CRegister::GetInstance()
{
    if (m_pins == nullptr)
    {
        CAutoMutexLock auto_lock(m_mutex);
        if (m_pins == nullptr)
        {
            m_pins = new CRegister;
        }
    }
    return m_pins;
}

int CRegister::Init(const CNodeInfo& node_info)
{
    m_self_info = node_info;
    return 0;
}

int CRegister::Register()
{
    if (0 == m_reg_check_thread_id)
    {
        m_is_running = true;
        if (0 != pthread_create(&m_reg_check_thread_id, nullptr, CRegister::RegisterCheckThread, nullptr))
        {
            printf("CRegister::Register create register check thread fail.\n");
            return -1;
        }
        printf("CRegister::Register create register check thread succ.\n");
    }
    return 0;
}

int CRegister::UnRegister()
{
    m_is_running = false;
    CAutoMutexLock auto_lock(m_mutex);
    TryUnregisterNode();
}

void* CRegister::RegisterCheckThread(void* param)
{
    prctl(PR_SET_NAME, "zk_register_check");
    while (true == CRegister::GetInstance()->IsRunning())
    {
        CRegister::GetInstance()->RegisterCheck();
        usleep(kZkRegisterIntervalTime);
    }
    return nullptr;
}

int CRegister::RegisterCheck()
{
    CAutoMutexLock auto_lock(m_mutex);

    //Î´×¢²á£¬³¢ÊÔ×¢²á
    if (EN_ZK_REGISTER_STATUS_UNREGISTER == m_status)
    {
        TryRegisterNode();
    }

    //ÒÑ×¢²á£¬checkÁ¬½Ó
    else if (EN_ZK_REGISTER_STATUS_REGISTER == m_status)
    {
        TryCheckNode();
    }

    //×´Ì¬Î´Öª£¬checkÁ¬½Ó
    else if (EN_ZK_REGISTER_STATUS_UNKNOWN == m_status)
    {
        TryCheckNode();
    }
    return 0;
}

bool CRegister::IsRunning()
{
    return m_is_running;
}

int CRegister::TryCheckNode()
{
    printf("%s =======================================================\n", __func__);
    if (EN_ZK_REGISTER_STATUS_UNREGISTER == m_status)
    {
        printf("CRegister::TryCheckNode status is un register. don't need check\n");
        return 0;
    }

    struct Stat stat;
    string info;
    int ret_code = CZkHandle::GetInstance()->ZkGetNodeInfo(m_raw_zk_path, info, stat);
    if (ZNONODE == ret_code)
    {
        printf("CRegister::TryCheckNode node don't exists. raw_path=%s\n.", m_raw_zk_path.c_str());
        m_status = EN_ZK_REGISTER_STATUS_UNREGISTER;
    }
    else if (ZOK != ret_code)
    {
        printf("CRegister::TryCheckNode something wrong. raw_path=%s ret_code=%d\n.", m_raw_zk_path.c_str(), ret_code);
        m_status = EN_ZK_REGISTER_STATUS_UNKNOWN;
    }
    else if (m_self_info.ToString() != info)
    {
        m_status = EN_ZK_REGISTER_STATUS_UNREGISTER;
        printf("CRegister::TryCheckNode get node succ.but belong to others!\n.");
    }
    else
    {
        printf("CRegister::TryCheckNode get node succ and check succ!\n.");
    }
}

int CRegister::TryRegisterNode()
{
    printf("%s =======================================================\n", __func__);
    if (EN_ZK_REGISTER_STATUS_UNREGISTER != m_status)
    {
        printf("CRegister::TryCheckNode status is not unregister.\n");
        return 0;
    }

    string reg_zk_path = m_self_info.m_zk_path + "/" + m_self_info.m_module_name + "_" + m_self_info.m_module_idx;
    string raw_node_name;
    bool is_sequential = m_self_info.m_reg_type == EN_ZK_REGISTER_TYPE_NORMAL;
    int ret_code = CZkHandle::GetInstance()->ZkCreateNode(reg_zk_path, m_self_info.ToString(), is_sequential, raw_node_name);
    if (ZNODEEXISTS == ret_code)
    {
        printf("CRegister::TryRegisterNode register fail. someone has already register\n");
        return ret_code;
    }
    else if(ZOK != ret_code)
    {
        printf("CRegister::TryRegisterNode register fail. ret=%d\n", ret_code);
        return ret_code;
    }

    m_raw_zk_path = raw_node_name;

    struct Stat stat;
    string info;
    ret_code = CZkHandle::GetInstance()->ZkGetNodeInfo(m_raw_zk_path, info, stat);
    if (ZNONODE == ret_code)
    {
        printf("CRegister::TryRegisterNode node don't exists. raw_path=%s\n.", m_raw_zk_path.c_str());
        return ret_code;
    }
    else if (ZOK != ret_code)
    {
        printf("CRegister::TryRegisterNode something wrong. raw_path=%s ret_code=%d\n.", m_raw_zk_path.c_str(), ret_code);
        return ret_code;
    }

    if (m_self_info.ToString() == info)
    {
        m_self_stat = stat;
        m_status = EN_ZK_REGISTER_STATUS_REGISTER;
        m_last_check_time = CTimeUtils::GetCurTimeUs();
        printf("CRegister::TryRegisterNode register succ!\n.");
    }
    else
    {
        printf("CRegister::TryRegisterNode register fail!\n.");
    }

    return ret_code;
}

int CRegister::TryUnregisterNode()
{
    printf("%s =======================================================\n", __func__);
    if (EN_ZK_REGISTER_STATUS_UNREGISTER == m_status)
    {
        printf("CRegister::TryUnregisterNode status is un register. don't need unregister\n");
        return 0;
    }

    struct Stat stat;
    string info;
    int ret_code = CZkHandle::GetInstance()->ZkGetNodeInfo(m_raw_zk_path, info, stat);
    if (ZNONODE == ret_code)
    {
        printf("CRegister::TryUnregisterNode node don't exists. raw_path=%s\n.", m_raw_zk_path.c_str());
        m_status = EN_ZK_REGISTER_STATUS_UNREGISTER;
        return 0;
    }
    else if (ZOK != ret_code)
    {
        printf("CRegister::TryUnregisterNode something wrong. raw_path=%s ret_code=%d\n.", m_raw_zk_path.c_str(), ret_code);
        m_status = EN_ZK_REGISTER_STATUS_UNKNOWN;
        return 0;
    }
    else if (m_self_info.ToString() != info)
    {
        m_status = EN_ZK_REGISTER_STATUS_UNREGISTER;
        printf("CRegister::TryUnregisterNode get node succ.but belong to others!\n.");
        return 0;
    }

    ret_code = CZkHandle::GetInstance()->ZkDeleteNode(m_raw_zk_path);

    if (ZOK != ret_code && ZNONODE != ret_code)
    {
        printf("CRegister::TryUnregisterNode unregister fail");
    }
    else
    {
        printf("CRegister::TryUnregisterNode unregister succ");
        m_status = EN_ZK_REGISTER_STATUS_UNREGISTER;
    }

    return ret_code;
}

