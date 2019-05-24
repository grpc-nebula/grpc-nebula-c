/*
 * Copyright 2019 Orient Securities Co., Ltd.
 * Copyright 2019 BoCloud Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 *    Author : Jianbin Yang(yangjianbin@beyondcent.com)
 *    2019/02/20
 *    version 0.9
 *    DES encrypt algorthm c implement
 */

#ifndef DES_H
#define DES_H
#include <string.h>
#include "memory.h"
#include "stdio.h"
//#include "stdlib.h"


#define PLAIN_FILE_OPEN_ERROR -1
#define KEY_FILE_OPEN_ERROR -2
#define CIPHER_FILE_OPEN_ERROR -3
#define OK 1

typedef char ElemType;
typedef unsigned char  byte;

// Function Definition
int ByteToBit(ElemType ch, ElemType bit[8]);
int BitToByte(ElemType bit[8], ElemType* ch);
int Char8ToBit64(ElemType ch[8], ElemType bit[64]);
int Bit64ToChar8(ElemType bit[64], ElemType ch[8]);
int DES_MakeSubKeys(ElemType key[64], ElemType subKeys[16][48]);
int DES_PC1_Transform(ElemType key[64], ElemType tempbts[56]);
int DES_PC2_Transform(ElemType key[56], ElemType tempbts[48]);
int DES_ROL(ElemType data[56], int time);
int DES_IP_Transform(ElemType data[64]);
int DES_IP_1_Transform(ElemType data[64]);
int DES_E_Transform(ElemType data[48]);
int DES_P_Transform(ElemType data[32]);
int DES_SBOX(ElemType data[48]);
int DES_XOR(ElemType R[48], ElemType L[48], int count);
int DES_Swap(ElemType left[32], ElemType right[32]);
int DES_EncryptBlock(ElemType plainBlock[8], ElemType subKeys[16][48],
                     ElemType cipherBlock[8]);
int DES_DecryptBlock(ElemType cipherBlock[8], ElemType subKeys[16][48],
                     ElemType plainBlock[8]);
int DES_EncryptFile(char* plainFile, char* keyStr, char* cipherFile);
int DES_DecryptFile(char* cipherFile, char* keyStr, char* plainFile);
char* DES_Encrypt(char* sourceData, int sourceSize, char* keyStr,
                  int* resultSize);
char* DES_Decrypt(char* sourceData, int sourceSize, char* keyStr,int* resultSize);

void HexToStr(byte* pszDest, byte* pbSrc, int nLen);
void StrToHex(byte* pbDest, char* pszSrc, int nLen);

#endif
