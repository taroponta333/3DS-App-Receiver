#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>

#include "network.h"
#include "receiver.h"

PSP_MODULE_INFO("3DS App Receiver", 0, 5, 4);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

/*=========================================================
    HOMEボタン対応
=========================================================*/

int exit_callback(int arg1, int arg2, void *common)
{
    sceKernelExitGame();
    return 0;
}

int CallbackThread(SceSize args, void *argp)
{
    int cbid;

    cbid = sceKernelCreateCallback(
        "Exit Callback",
        exit_callback,
        NULL);

    sceKernelRegisterExitCallback(cbid);

    sceKernelSleepThreadCB();

    return 0;
}

int SetupCallbacks(void)
{
    int thid;

    thid = sceKernelCreateThread(
        "update_thread",
        CallbackThread,
        0x11,
        0xFA0,
        0,
        NULL);

    if (thid >= 0)
        sceKernelStartThread(thid, 0, NULL);

    return thid;
}

/*=========================================================
    Main
=========================================================*/

int main(void)
{
    int ret;

    SetupCallbacks();

    pspDebugScreenInit();

    pspDebugScreenPrintf("=================================\n");
    pspDebugScreenPrintf("      3DS App Receiver\n");
    pspDebugScreenPrintf("            v0.5.4\n");
    pspDebugScreenPrintf("=================================\n\n");

    /*-----------------------------
        ネットワーク初期化
    -----------------------------*/

    pspDebugScreenPrintf("Initializing Network...\n");

    ret = Network_Init();

    if (ret < 0)
    {
        pspDebugScreenPrintf("\n");
        pspDebugScreenPrintf("Network Init Failed!\n");
        pspDebugScreenPrintf("Error : 0x%08X\n", ret);

        while (1)
            sceDisplayWaitVblankStart();
    }

    pspDebugScreenPrintf("[OK] Network Initialized\n");

    /* 接続状態表示 */

    Network_PrintConnectionState();

    /* Receiver初期化 */

    Receiver_Init();

    pspDebugScreenPrintf("\n");
    pspDebugScreenPrintf("---------------------------------\n");
    pspDebugScreenPrintf("Waiting for 3DS...\n");
    pspDebugScreenPrintf("---------------------------------\n\n");

    pspDebugScreenPrintf("HOME : Exit\n");

    /*=====================================================
        メインループ
    =====================================================*/

    while (1)
    {
        Receiver_Update();

        sceDisplayWaitVblankStart();
    }

    /* 通常ここには来ない */

    Receiver_Shutdown();

    Network_Shutdown();

    return 0;
}
