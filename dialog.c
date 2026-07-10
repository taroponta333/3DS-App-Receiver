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

    /* Setup dialog parameters */
    dialog.base.size = sizeof(dialog);
    dialog.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
    dialog.base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;
    dialog.base.graphicsThread = 17;
    dialog.base.accessThread = 19;
    dialog.base.fontThread = 18;
    dialog.base.soundThread = 16;

    dialog.action = PSP_NETCONF_ACTION_CONNECTAP;
    dialog.hotspot = 0;
    dialog.hotspot_connected = 0;
    dialog.wifisp = 0;

    /* Start the dialog */
    ret = sceUtilityNetconfInitStart(&dialog);
    if(ret < 0)
    {
        sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
        sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
        return -1;
    }

    /* Dialog loop */
    while(1)
    {
        int status = sceUtilityNetconfGetStatus();

        switch(status)
        {
            case PSP_UTILITY_DIALOG_NONE:
            case PSP_UTILITY_DIALOG_INIT:
            case PSP_UTILITY_DIALOG_VISIBLE:
                sceUtilityNetconfUpdate(1);
                break;

            case PSP_UTILITY_DIALOG_QUIT:
                sceUtilityNetconfShutdownStart();
                break;

            case PSP_UTILITY_DIALOG_FINISHED:
                sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
                sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
                return 0;

            default:
                sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
                sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
                return 0;
        }

        sceDisplayWaitVblankStart();
    }

    return 0;
}
