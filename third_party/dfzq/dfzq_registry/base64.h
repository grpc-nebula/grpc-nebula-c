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
