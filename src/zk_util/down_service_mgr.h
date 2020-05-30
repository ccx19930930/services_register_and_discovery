#ifndef _DOWN_SERVICE_MGR_H_
#define _DOWN_SERVICE_MGR_H_

#include "base_class.h"

class CDownServiceMgr : CUnCopyable
{
public:
    CDownServiceMgr(int module_id) : m_module_id(module_id) {}
    ~CDownServiceMgr() {}
private:
    CDownServiceMgr() {}

public:
    int Register(const string& zk_path, CNodeInfo* node_info);
    int UnRegister(const string& zk_path);

private:
    int m_module_id;
    map<string, CNodeInfo *> m_node_list;
     pthread_mutex_t m_mutex;
};

#endif
