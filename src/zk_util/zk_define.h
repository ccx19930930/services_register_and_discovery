#ifndef _ZK_DEFINE_H_ 
#define _ZK_DEFINE_H_

const int kMaxBufferLen = 4096;
const int kZkHandleIntervalTime = 1000000;
const int kZkRegisterIntervalTime = 1000000;
const int kZkDiscoveryIntervalTime = 1000000;

enum EZkRegisterStatus
{
    EN_ZK_REGISTER_STATUS_UNREGISTER = 0,
    EN_ZK_REGISTER_STATUS_REGISTER,
    EN_ZK_REGISTER_STATUS_TIMEOUT,
    EN_ZK_REGISTER_STATUS_UNKNOWN,
};

enum EZkRegisterType
{
    EN_ZK_REGISTER_TYPE_NORMAL,
    EN_ZK_REGISTER_TYPE_LOCK,
};

#endif
