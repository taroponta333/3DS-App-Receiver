#ifndef NETWORK_H
#define NETWORK_H

/* ネットワーク初期化 */
int Network_Init(void);

/* ネットワーク終了 */
void Network_Shutdown(void);

/* 接続状態 */
int Network_IsConnected(void);

/* 接続待機 */
int Network_WaitConnection(void);

/* IPアドレス取得 */
int Network_GetIP(char *ip);

/* IPアドレス表示 */
void Network_PrintIP(void);

#endif
