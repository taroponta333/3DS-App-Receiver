#include <pspkernel.h>
#include <pspdebug.h>

#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>

#include <stdio.h>
#include <string.h>

#include "network.h"

/*=========================================================
    Network Initialization
=========================================================*/

int Network_Init(void)
{
    int ret;

    /* Initialize network with standard parameters for user mode */
    ret = sceNetInit(128 * 1024, 42, 4 * 1024, 42, 4 * 1024);

    if(ret < 0)
    {
        return ret;
    }

    /* Initialize inet module */
    ret = sceNetInetInit();

    if(ret < 0)
    {
        sceNetTerm();
        return ret;
    }

    /* Initialize resolver */
    ret = sceNetResolverInit();

    if(ret < 0)
    {
        sceNetInetTerm();
        sceNetTerm();
        return ret;
    }

    /* Initialize access control - adjust stack size for user mode */
    ret = sceNetApctlInit(0x8000, 48);

    if(ret < 0)
    {
        sceNetResolverTerm();
        sceNetInetTerm();
        sceNetTerm();
        return ret;
    }

    return 0;
}

/*=========================================================
    Network Shutdown
=========================================================*/

void Network_Shutdown(void)
{
    sceNetApctlTerm();
    sceNetResolverTerm();
    sceNetInetTerm();
    sceNetTerm();
}

/*=========================================================
    Connection Check
=========================================================*/

int Network_IsConnected(void)
{
    int state;

    if(sceNetApctlGetState(&state) < 0)
        return 0;

    return (state == 4);
}

/*=========================================================
    Wait for Connection
=========================================================*/

int Network_WaitConnection(void)
{
    while(!Network_IsConnected())
    {
        sceKernelDelayThread(100000);
    }

    return 0;
}

/*=========================================================
    Get IP Address
=========================================================*/

int Network_GetIP(char *ip)
{
    union SceNetApctlInfo info;

    memset(&info, 0, sizeof(info));

    if(sceNetApctlGetInfo(PSP_NET_APCTL_INFO_IP, &info) < 0)
    {
        return -1;
    }

    strncpy(ip, info.ip, 15);
    ip[15] = '\0';

    return 0;
}

/*=========================================================
    Print IP Address
=========================================================*/

void Network_PrintIP(void)
{
    char ip[16];

    if(Network_GetIP(ip) < 0)
    {
        pspDebugScreenPrintf("IP : Unknown\n");
        return;
    }

    pspDebugScreenPrintf("IP : %s\n", ip);
}
