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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "uuid4gen.h"


 /*
  * Generate an UUID version 4 and stores it into a string
  * Reference: http://www.ietf.org/rfc/rfc4122.txt
  * Returns EXIT_SUCCESS on success, or EXIT_FAILURE on... failure.
  */
static uint8_t uuid4init = 0;

uint8_t uuid4gen(char *myuuid)
{
	unsigned char r[16];
	uint8_t i;

	if (uuid4init == 0) {
		(void)srand((unsigned int)time(NULL));
		uuid4init = 1;
	}

	for (i = 0; i < 16; i++) {
		*(r + i) = (unsigned char)rand() % 255;
	}

	*(r + 6) = 0x40 | (*(r + 6) & 0xf);
	*(r + 8) = 0x80 | (*(r + 8) & 0x3f);

	sprintf(myuuid,
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		*(r + 0), *(r + 1), *(r + 2), *(r + 3), *(r + 4), *(r + 5), *(r + 6), *(r + 7), *(r + 8), *(r + 9), *(r + 10),
		*(r + 11), *(r + 12), *(r + 13), *(r + 14), *(r + 15));

	return EXIT_SUCCESS;
}
