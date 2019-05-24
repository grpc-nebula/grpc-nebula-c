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
/**********************************
 * function: consistent hash
 * author: Yang Jianbin
 * date: 2017.12.19
 * viersion: 0.9
**********************************/

#ifndef CONSISTENT_HASH_H  
#define CONSISTENT_HASH_H  

#include "md5.h"
#include "stdlib.h"

/*ʵ����*/
class cnode_s
{
public:
	/*���캯��*/
	cnode_s();
	cnode_s(char * p_iden, int p_vnode_count, void * p_data);

	/*��ȡ����ʾ*/
	const char * get_iden();

	/*��ȡʵ���������������*/
	int get_vnode_count();

	/*����ʵ��������ֵ*/
	void set_data(void * data);

	/*��ȡʵ��������ֵ*/
	void * get_data();
private:
	void set_cnode_s(char * p_iden, int p_vnode_count, void * p_data);
	char iden[100];/*����ʾ��*/
	int vnode_count; /*��������Ŀ*/
	void * data;/*���ݽ��*/
};

/*������*/
class cvirtual_node_s
{
public:
	/*���캯��*/
	cvirtual_node_s();
	cvirtual_node_s(cnode_s * pnode);

	/*������������ָ���ʵ����*/
	void set_node_s(cnode_s * pnode);

	/*��ȡ��������ָ���ʵ����*/
	cnode_s * get_node_s();

	/*����������hashֵ*/
	void set_hash(long phash);

	/*��ȡ������hashֵ*/
	long get_hash();
private:
	long hash; /*hashֵ*/
	cnode_s * node; /*��������ָ���ʵ����*/
};

class cmd5_hash_fun 
{
public:
	virtual long get_hash_val(const char *);
};


typedef unsigned int u_int;
typedef unsigned char u_char;
typedef long util_long;

typedef struct util_rbtree_s util_rbtree_t;
typedef struct util_rbtree_node_s util_rbtree_node_t;

struct util_rbtree_node_s
{
	long key;
	util_rbtree_node_t *parent;
	util_rbtree_node_t *right;
	util_rbtree_node_t *left;
	int color;
	void *data;
};

struct util_rbtree_s
{
	util_rbtree_node_t *root;
	util_rbtree_node_t  null;
	u_int size;
};

#define util_rbt_black(rbnode)   ((rbnode)->color = 1)
#define util_rbt_red(rbnode)     ((rbnode)->color = 0)
#define util_rbt_isblack(rbnode) ((rbnode)->color == 1)
#define util_rbt_isred(rbnode)   ((rbnode)->color == 0)

/* clear a node's link */
#define rbt_clear_node(node) do{ \
	node->left = NULL;  \
	node->right = NULL; \
	node->parent = NULL; \
	}while(0)

/* is the tree empty */
#define util_rbtree_isempty(rbtree) ((rbtree)->root == &(rbtree)->null)

/*
* find the min node of tree
* return NULL is tree is empty
*/
#define util_rbtree_min(rbtree) util_rbsubtree_min((rbtree)->root, &(rbtree)->null)

/*
* find the max node of tree
* return NULL is tree is empty
*/
#define util_rbtree_max(rbtree) util_rbsubtree_max((rbtree)->root, &(rbtree)->null)

void util_rbtree_init(util_rbtree_t *rbtree);
void util_rbtree_insert(util_rbtree_t *rbtree, util_rbtree_node_t *node);
void util_rbtree_delete(util_rbtree_t *rbtree, util_rbtree_node_t *node);

/*
* search node with key = @key in the tree
* if no such node exist, return NULL
*/
util_rbtree_node_t* util_rbtree_search(util_rbtree_t *rbtree, long key);

/*
* look node in the tree
* return the first node with key >= @key;
* if @key > all the key values in the tree, return the node with minimum key
* return NULL if tree is empty
*/
util_rbtree_node_t* util_rbtree_lookup(util_rbtree_t *rbtree, long key);

/*
* find the min node of subtree
* @rbnode: root of the subtree
* @sentinel : the sentinel node
* return NULL if subtree is empty
*/
util_rbtree_node_t* util_rbsubtree_min(util_rbtree_node_t *node, util_rbtree_node_t *sentinel);

/*
* find the max node of subtree
* @rbnode: root of the subtree
* @sentinel : the sentinel node
* return NULL if subtree is empty
*/
util_rbtree_node_t* util_rbsubtree_max(util_rbtree_node_t *node, util_rbtree_node_t *sentinel);

/*
* check whether a tree is a rb tree, the null node is n't checked
* return 0: yes
* return 1: root isn't black
* return 2: node is in other color than black and red
* return 3: tree's black height isn't unique
* return 4: a red node with parent in red exists
* return 5: volatile binary search properties
*
* when return !0, @blackheight & @maxdepth is uselsess
* when return 0, @blackheight contains the tree's black height
*
* @maxdepth contains the max length of all simple roads from root to it's leaf nodes
*/
int util_rbtree_check(const util_rbtree_t *rbtree, int *blackheight, int *maxdepth);

/*
* travel through a rb tree in sequence: left-root-right
* you CAN NOT do any operations that will break the RB properties
*/
void util_rbtree_mid_travel(util_rbtree_t *rbtree, void(*opera)(util_rbtree_node_t *, void *), void *data);



class ccon_hash
{
public:
	/*���캯��*/
	ccon_hash(cmd5_hash_fun * pfunc);

	/*����hash����*/
	void set_func(cmd5_hash_fun * pfunc);

	/*����ʵ���� , 0����ɹ� , -1����ʧ��*/
	int add_node_s(cnode_s * pnode);

	/*ɾ��ʵ���� , 0����ɹ� , -1����ʧ��*/
	int del_node_s(cnode_s * pnode);

	/*����ʵ����*/
	cnode_s * lookup_node_s(const char * object);

	/*��ȡһ����hash�ṹ����������������*/
	int get_vnodes();
private:
	/*Hash����*/
	cmd5_hash_fun * func;
	/*�������ܸ���*/
	int vnodes;
	/*�洢������ĺ����*/
	util_rbtree_t * vnode_tree;
};
/*����������������ת��Ϊ��������*/
util_rbtree_node_t * vnode_2rb_node(cvirtual_node_s * vnode);

#endif   
