#ifndef _BASE_CLASS_H_
#define _BASE_CLASS_H_

class CUnCopyable 
{
protected:
    CUnCopyable() {}
    ~CUnCopyable() {}
private:
    CUnCopyable(const CUnCopyable&);
    CUnCopyable& operator=(const CUnCopyable&);
};


#endif
