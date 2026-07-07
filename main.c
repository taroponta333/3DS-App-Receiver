#include <pspkernel.h>
#include <pspdebug.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psputility.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pspctrl.h>
#include <pspdisplay.h>

PSP_MODULE_INFO("3DS_Receiver", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define RECV_PORT 8080
#define PACKET_SIZE 1448
#define SAVE_PATH "ms0:/received_file.pbp"

int exit_request = 0;

int exit_callback(int arg1, int arg2, void *common) {
    exit_request = 1;
    return 0;
}
int callback_thread(SceSize args, void *argp) {
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}
void setup_callbacks(void) {
    int thid = sceKernelCreateThread("update_thread", callback_thread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0) sceKernelStartThread(thid, 0, 0);
}

int show_exit_dialog(void) {
    pspUtilityMsgDialogParams dialog;
    memset(&dialog, 0, sizeof(dialog));
    
    dialog.base.size = sizeof(dialog);
    dialog.base.language = 1;
    dialog.base.buttonSwap = 0;
    dialog.base.graphicsThread = 17;
    dialog.base.accessThread = 19;
    dialog.base.fontThread = 18;
    dialog.base.soundThread = 16;
    
    dialog.mode = PSP_UTILITY_MSGDIALOG_MODE_TEXT;
    dialog.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT; 
    snprintf(dialog.message, 512, "Do you want to quit the application?");

    sceUtilityMsgDialogInitStart(&dialog);

    while (1) {
        int status = sceUtilityMsgDialogGetStatus();
        
        if (status == PSP_UTILITY_DIALOG_VISIBLE) { 
            sceUtilityMsgDialogUpdate(1);
        } else if (status == PSP_UTILITY_DIALOG_FINISHED) {
            if (dialog.buttonPressed == PSP_UTILITY_MSGDIALOG_RESULT_YES) {
                sceUtilityMsgDialogShutdownStart();
                return 1;
            }
            sceUtilityMsgDialogShutdownStart();
            return 0;
        }
        sceDisplayWaitVblankStart(); 
    }
}

int init_network(void) {
    sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
    sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
    sceNetInit(128 * 1024, 42, 4 * 1024, 42, 4 * 1024);
    sceNetInetInit();
    sceNetResolverInit();
    sceNetApctlInit(0x1400, 42);
    return 0;
}

void shutdown_network(void) {
    sceNetApctlTerm();
    sceNetResolverTerm();
    sceNetInetTerm();
    sceNetTerm();
    sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
    sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
}

// 画面の特定の行をクリアして再描画するための補助関数
void print_status_line(int y, const char *msg) {
    pspDebugScreenSetXY(0, y);
    // 既存の文字を上書き消去するために空白で埋めてから描画
    printf("                                                                ");
    pspDebugScreenSetXY(0, y);
    printf("%s", msg);
}

int main(void) {
    pspDebugScreenInit();
    setup_callbacks();
    
    printf("==========================================\n");
    printf("       PSP Wireless File Receiver         \n");
    printf("==========================================\n\n");
    
    printf("[SYS] Initializing network...\n");
    if (init_network() < 0) {
        printf("[ERR] Network initialization failed!\n");
        sceKernelDelayThread(2000000);
        sceKernelExitGame();
        return 0;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(RECV_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    struct timeval timeout = {0, 100000}; // タイムアウトを0.1秒にしてボタン反応を向上
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    char buffer[PACKET_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    FILE *file = NULL;
    
    int is_transferring = 0;
    unsigned int total_file_size = 0;   // 3DSから通知される総ファイルサイズ
    unsigned int received_bytes_sum = 0; // これまでに受信した総バイト数

    SceCtrlData pad;

    // メッセージ描画位置の固定用の行番号
    const int STATUS_LINE_Y = 6;
    const int PROGRESS_LINE_Y = 8;

    print_status_line(STATUS_LINE_Y, "[STATUS] Ready. Waiting for 3DS broadcast...");
    printf("\n\n(Press TRIANGLE to quit)\n");

    while (!exit_request) {
        sceCtrlPeekBufferPositive(&pad, 1);
        if (pad.Buttons & PSP_CTRL_TRIANGLE) { 
            if (show_exit_dialog()) {
                break; 
            } else {
                // ダイアログから戻ったらデバッグ画面をクリアして再描画
                pspDebugScreenClear();
                printf("==========================================\n");
                printf("       PSP Wireless File Receiver         \n");
                printf("==========================================\n\n");
                if (is_transferring) {
                    print_status_line(STATUS_LINE_Y, "[STATUS] Downloading data from 3DS...");
                } else {
                    print_status_line(STATUS_LINE_Y, "[STATUS] Ready. Waiting for 3DS broadcast...");
                }
                pspDebugScreenSetXY(0, STATUS_LINE_Y + 2);
                printf("(Press TRIANGLE to quit)\n");
            }
        }

        int bytes_received = recvfrom(sock, buffer, PACKET_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (bytes_received > 0) {
            if (!is_transferring) {
                file = fopen(SAVE_PATH, "wb");
                is_transferring = 1;
                print_status_line(STATUS_LINE_Y, "[STATUS] Connection established! Receiving data...");
                
                // 💡 【進捗処理】最初のパケットの先頭4バイトを総ファイルサイズとして読み取る場合
                // ※3DS側の送信コードの先頭に「ファイルサイズ」を仕込んでおく必要があります
                if (bytes_received >= sizeof(unsigned int)) {
                    memcpy(&total_file_size, buffer, sizeof(unsigned int));
                    
                    // 最初のパケットの残りのデータをファイルに書き込む
                    int data_size = bytes_received - sizeof(unsigned int);
                    if (data_size > 0 && file) {
                        fwrite(buffer + sizeof(unsigned int), 1, data_size, file);
                        received_bytes_sum += data_size;
                    }
                }
            } else {
                // 2パケット目以降の通常の書き込み
                if (file) {
                    fwrite(buffer, 1, bytes_received, file);
                    received_bytes_sum += bytes_received;
                }
            }

            // 📊 画面に進捗状況をリアルタイム描画
            char progress_msg[128];
            if (total_file_size > 0) {
                int percent = (int)(((long long)received_bytes_sum * 100) / total_file_size);
                if (percent > 100) percent = 100; // 念のための上限ガード
                snprintf(progress_msg, sizeof(progress_msg), "Progress: %d%% (%u / %u bytes)", percent, received_bytes_sum, total_file_size);
            } else {
                // ファイルサイズが不明な場合は、受信サイズのみカウント表示
                snprintf(progress_msg, sizeof(progress_msg), "Progress: ---%% (%u bytes received)", received_bytes_sum);
            }
            print_status_line(PROGRESS_LINE_Y, progress_msg);

        } else if (is_transferring) {
            // タイムアウト（データが送られてこなくなった）＝転送完了とみなす
            if (file) fclose(file);
            file = NULL;
            print_status_line(STATUS_LINE_Y, "[STATUS] Transfer complete! File saved successfully.");
            sceKernelDelayThread(3000000); // 3秒間画面を保持
            break;
        }
    }

    if (file) fclose(file);
    close(sock);
    shutdown_network();
    sceKernelExitGame();
    return 0;
}
