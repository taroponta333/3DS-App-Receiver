#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>

#include <psputility.h>
#include <psputility_netconf.h>
#include <psputility_modules.h>

#include <string.h>

#include "dialog.h"

int Dialog_ShowNetwork(void)
{
    int ret;
    pspUtilityNetconfData dialog;
    int timeout = 0;

    memset(&dialog, 0, sizeof(dialog));

    /* Initialize network modules */
    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
    if(ret < 0)
    {
        return -1;
    }

    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
    if(ret < 0)
    {
        sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
        return -1;
    }

    /* Setup dialog parameters - use minimal configuration */
    dialog.base.size = sizeof(dialog);
    dialog.base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;

    dialog.action = PSP_NETCONF_ACTION_CONNECTAP;

    /* Start the dialog */
    ret = sceUtilityNetconfInitStart(&dialog);
    if(ret < 0)
    {
        sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
        sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
        return -1;
    }

    /* Dialog loop with timeout protection */
    while(1)
    {
        int status = sceUtilityNetconfGetStatus();

        switch(status)
        {
            case PSP_UTILITY_DIALOG_NONE:
            case PSP_UTILITY_DIALOG_INIT:
            case PSP_UTILITY_DIALOG_VISIBLE:
                sceUtilityNetconfUpdate(1);
                timeout = 0;  /* Reset timeout on active status */
                break;

            case PSP_UTILITY_DIALOG_QUIT:
                sceUtilityNetconfShutdownStart();
                break;

            case PSP_UTILITY_DIALOG_FINISHED:
                sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
                sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
                return 0;

            default:
                /* Unknown status - cleanup and exit */
                sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
                sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
                return 0;
        }

        /* Timeout protection - prevent infinite loop */
        timeout++;
        if(timeout > 1000)
        {
            sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
            sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
            return -1;
        }

        sceDisplayWaitVblankStart();
    }

    return 0;
}
