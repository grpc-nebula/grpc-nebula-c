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
*    Author : heiden deng(dengjianquan@beyondcent.com)
*    2017/05/15
*    version 0.0.9
*    �����ļ���ȡ��ؽӿں���
*/

#ifndef ORIENTSEC_GRPC_PROPERTIES_TOOLS_H
#define ORIENTSEC_GRPC_PROPERTIES_TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

#define ORIENTSEC_GRPC_PATH_MAX_LEN 2048
#define ORIENTSEC_GRPC_BUF_LEN 2048

//�����ļ���key��󳤶�
#define ORIENTSEC_GRPC_PROPERTY_KEY_MAX_LEN 100

//�����ļ���value��󳤶�
#define ORIENTSEC_GRPC_PROPERTY_VALUE_MAX_LEN 1024

//�����ļ���������ԣ�key=value������
#define ORIENTSEC_GRPC_MAX_PERPERTIES_NUM 100

#define ORIENTSEC_GRPC_CONF_ENV "ORIENTSEC_GRPC_CONFIG"

//Ĭ�ϵ������ļ���
#define ORIENTSEC_GRPC_PROPERTIES_FILENAME "dfzq-grpc-config.properties"

//���������ļ�,�����ļ����ɺ�ORIENTSEC_GRPC_PROPERTIES_FILENAME����
//Ĭ�������ļ�����˳�򣬵�ǰִ��Ŀ¼./�� ./config/, ../configĿ¼��, /etc/Ŀ¼��,�򿪳ɹ�����0,
// ʾ�� 
// .bin/xxx.exe
// .conf/orientsec-grpc-c-config.conf
int orientsec_grpc_properties_init();

//�������ļ��ж�ȡָ��key������ֵ��֧��ǰ׺��ʽ��д,����ֵ0����ɹ�
//���� consumer.version <=> version,����prefix="consumer."
int orientsec_grpc_properties_get_value(const char *key, char *prefix, char *value);


#ifdef __cplusplus
}
#endif
#endif // !ORIENTSEC_GRPC_PROPERTIES_TOOLS_H
