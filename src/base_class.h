#ifndef _BASE_CLASS_H_
#define _BASE_CLASS_H_

#include <string>
#include <sstream>

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
    CNodeInfo() {}
    ~CNodeInfo() {}
public:
    string ToString()
    {
        stringstream oss;
        oss.str("");
        oss << "IP[" << m_ip << "] "
            << "PORT[" << m_port << "] ";
        return oss.str();
    }

    void FromString(const string & info)
    {
        auto get_val = [info](string key1, string key2)->string
        {
            int idx;
            idx = info.find(key1) + key1.size();
            return info.substr(idx, info.find_first_of(key2, idx) - idx);
        };

        m_ip = get_val("IP[", "]");
        m_port = get_val("PORT[", "]");
    }

public:
    string m_ip;
    string m_port;
};

#endif
