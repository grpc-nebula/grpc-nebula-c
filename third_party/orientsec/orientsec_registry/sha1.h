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
 *    2019/02/18
 *    version 0.9
 *    SHA1 encrypt algorthm c definition
 */

#ifndef SHA1_H
#define SHA1_H

/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain
*/

#include "stdint.h"

typedef struct
{
	uint32_t state[5];
	uint32_t count[2];
	unsigned char buffer[64];
} SHA1_CTX;

void SHA1Transform(uint32_t state[5],const unsigned char buffer[64]);

void SHA1Init(SHA1_CTX * context);

void SHA1Update(SHA1_CTX * context,const unsigned char *data,uint32_t len);

void SHA1Final(	unsigned char digest[20],SHA1_CTX * context);

void SHA1(char *hash_out,const char *str,int len);

#endif /* SHA1_H */
