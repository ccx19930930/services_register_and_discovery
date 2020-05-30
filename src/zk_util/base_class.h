#ifndef _BASE_CLASS_H_
#define _BASE_CLASS_H_

#include "zk_define.h"

#include <string>
#include <sstream>

#include <json.h>

using namespace std;

class CUnCopyable 
{
protected:
    CUnCopyable() {}
    ~CUnCopyable() {}
private:
    CUnCopyable(const CUnCopyable&);
    CUnCopyable& operator=(const CUnCopyable&);
};

class CNodeInfo
{
public:
    CNodeInfo() { Reset(); }
    ~CNodeInfo() {}
public:
    void Reset()
    {
        m_ip = "";
        m_port = "";
        m_zk_path = "";
        m_module_name = "";
        m_module_id = "";
        m_module_idx = "";
    }

    string ToString()
    {
        stringstream oss;
        oss.str("");
        auto set_val = [&oss](const string& key, const string& value)
        {
            oss << key << "[" << value << "] ";
        };

        set_val("IP", m_ip);
        set_val("PORT", m_port);
        set_val("ZK_PATH", m_zk_path);
        set_val("MODULE_NAME", m_module_name);
        set_val("MODULE_ID", m_module_id);
        set_val("MODULE_IDX", m_module_idx);

        return oss.str();
    }

    void FromString(const string& info)
    {
        auto get_val = [info](string key)->string
        {
            int idx;
            string key1 = key + "[";
            string key2 = "]";
            idx = info.find(key1) + key1.size();
            return info.substr(idx, info.find_first_of(key2, idx) - idx);
        };

        m_ip = get_val("IP");
        m_port = get_val("PORT");
        m_zk_path = get_val("ZK_PATH");
        m_module_name = get_val("MODULE_NAME");
        m_module_id = get_val("MODULE_ID");
        m_module_idx = get_val("MODULE_IDX");
    }

    void FromJson(const Json::Value& info)
    {
        m_ip = info["ip"].asString();
        m_port = info["port"].asString();
        m_zk_path = info["zk_path"].asString();
        m_module_name = info["module_name"].asString();
        m_module_id = info["module_id"].asString();
        m_module_idx = info["module_idx"].asString();
        m_reg_type = info["reg_type"].asInt();
    }

public:
    string m_ip;
    string m_port;
    string m_zk_path;
    string m_module_name;
    string m_module_id;
    string m_module_idx;

public:
    int m_reg_type;
};

#endif
