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
 *    base64 encrypt algo c definition
 */

#ifndef BASE64_H
#define BASE64_H

// move to cpp file
//const char cbase[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

/* Base64 ±àÂë */
char* base64_encode(const char* data, int data_len);

/* Base64 ½âÂë */
char *base64_decode(const char* data, int data_len);

#endif
