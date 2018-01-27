#ifndef SESSIONINFO_H
#define SESSIONINFO_H

#include <netinet/in.h>

// Connection session info struct.

struct SessionInfo
{	
    const long int SocketHandle;
    
    const struct sockaddr_in SocketAddressInfo;
    
    SessionInfo(const long int socketHandle, const struct sockaddr_in socketAddressInfo): SocketHandle(socketHandle), SocketAddressInfo(socketAddressInfo) {}
};

#endif