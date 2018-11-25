#pragma once
// ������ʾ: 
//   1. ʹ�ý��������Դ�������������/�����ļ�
//   2. ʹ���Ŷ���Դ�������������ӵ�Դ�������
//   3. ʹ��������ڲ鿴���������������Ϣ
//   4. ʹ�ô����б��ڲ鿴����
//   5. ת������Ŀ��>���������Դ����µĴ����ļ�����ת������Ŀ��>�����������Խ����д����ļ���ӵ���Ŀ
//   6. ��������Ҫ�ٴδ򿪴���Ŀ����ת�����ļ���>���򿪡�>����Ŀ����ѡ�� .sln �ļ�

#ifndef COMPILER_H
#define COMPILER_H

// TODO: ���Ҫ�ڴ˴�Ԥ����ı�ͷ


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#define true 1
#define false 0

#define norw 24       /* �����ָ��� */
#define txmax 128     /* ���ű����� */
#define nmax 14       /* ���ֵ����λ�� */
#define al 10         /* ��ʶ������󳤶� */
#define maxerr 30     /* ������������� */
#define amax 2048     /* ��ַ�Ͻ�*/
#define levmax 1      /* ����������Ƕ����������*/
#define cxmax 200     /* ��������������� */
#define stacksize 500 /* ����ʱ����ջԪ�����Ϊ500�� */
#define symnum 48
#define fctnum 9	

/* ���� */
enum symbol {
	nul, ident, number, plus, minus,
	times, slash, oddsym, eql, neq,
	lss, leq, gtr, geq, lparen,
	rparen, comma, semicolon, period, becomes,
	mainsym, continsym, ifsym, breaksym, whilesym,
	writesym, readsym, dosym, callsym, constsym,
	elsesym, selfplus, selfminus, constinuesym, switchsym,
	repeatsym, forsym, mod, untilsym, xor,
	lbrace, rbrace, lrange, rrange, exitsym,
	casesym, andsym, boolsym, charsym,
	intsym, orsym, varsym, funcsym,
};

/* ���ű��е����� */
enum object {
	constant,
	variable,
	procedure,
	array,
	integer,
	character,
};

/* ���������ָ�� */
enum fct {
	lit, opr, lod,
	sto, cal, ini,
	jmp, jpc, jeq,
};

/* ���������ṹ */
struct instruction {
	enum fct f; /* ���������ָ�� */
	int l;      /* ���ò���������Ĳ�β� */
	int a;      /* ����f�Ĳ�ͬ����ͬ */
};


bool listswitch;   /* ��ʾ������������ */
bool tableswitch;  /* ��ʾ���ű���� */
char ch;            /* ��ŵ�ǰ��ȡ���ַ���getch ʹ�� */
enum symbol sym;    /* ��ǰ�ķ��� */
enum symbol nxtsym;	/* Ϊ�����ͻ�������һ������*/
enum symbol oldsym;	/* Ϊ�����ͻ�������һ������*/
char id[al + 1];      /* ��ǰident�������һ���ֽ����ڴ��0 */
int num;            /* ��ǰnumber */
int cc, ll;         /* getchʹ�õļ�������cc��ʾ��ǰ�ַ�(ch)��λ�� */
int cx;             /* ���������ָ��, ȡֵ��Χ[0, cxmax-1]*/
char line[81];		/* ��ȡ�л����� */
char a[al + 1];       /* ��ʱ���ţ������һ���ֽ����ڴ��0 */
struct instruction code[cxmax]; /* ����������������� */
char word[norw][al];        /* ������ */
enum symbol wsym[norw];     /* �����ֶ�Ӧ�ķ���ֵ */
enum symbol ssym[256];      /* ���ַ��ķ���ֵ */
char mnemonic[fctnum][5];   /* ���������ָ������ */
bool declbegsys[symnum];    /* ��ʾ������ʼ�ķ��ż��� */
bool statbegsys[symnum];    /* ��ʾ��俪ʼ�ķ��ż��� */
bool facbegsys[symnum];     /* ��ʾ���ӿ�ʼ�ķ��ż��� */
bool debug;
char errorInfo[100][100];	/* ���������Ϣ*/
int  shift;					/* ����ƫ����*/

/* ���ű�ṹ */
struct tablestruct
{
	char name[al];	    /* ���� */
	enum object kind;	/* ���ͣ�const��var��procedure */
	int val;            /* ��ֵ����constʹ�� */
	int level;          /* �����㣬��const��ʹ�� */
	int adr;            /* ��ַ����const��ʹ�� */
	int size;           /* ��Ҫ������������ռ�, ��procedureʹ�� */
};

struct tablestruct table[txmax]; /* ���ű� */

FILE* fin;      /* ����Դ�ļ� */
FILE* ftable;	/* ������ű� */
FILE* fcode;    /* ������������ */
FILE* foutput;  /* ����ļ�������ʾ�⣨���д������ж�Ӧ�����ɴ����׵�ַ�����޴� */
FILE* fresult;  /* ���ִ�н�� */
FILE* ferrors;	/* ���������Ϣ*/
FILE* fdebug;	/* ���������Ϣ*/
char fname[al];
int err;        /* ��������� */
int isChar;

void compile();
void error(int n);
void getsym();
void getch();
void init();
void gen(enum fct x, int y, int z);
void test(bool* s1, bool* s2, int n);
int inset(int e, bool* s);
int addset(bool* sr, bool* s1, bool* s2, int n);
int subset(bool* sr, bool* s1, bool* s2, int n);
int mulset(bool* sr, bool* s1, bool* s2, int n);
void block(int lev, int tx, bool* fsys);
void interpret();
void factor(bool* fsys, int* ptx, int lev);
void term(bool* fsys, int* ptx, int lev);
void condition(bool* fsys, int* ptx, int lev);
void expression(bool* fsys, int* ptx, int lev);
void expression_stat(bool* fsys, int* ptx, int lev);
void additive_expr(bool* fsys, int* ptx, int lev);
void simple_expr(bool* fsys, int* ptx, int lev);
void statement(bool* fsys, int* ptx, int lev);
void listall();
void intdeclaration(int* ptx, int lev, int* pdx);
void chardeclaration(int* ptx, int lev, int* pdx);
void intlistdeclaration(int * ptx, int lev, int * pdx, int length);
void charlistdeclaration(int * ptx, int lev, int * pdx, int length);
void constdeclaration(int* ptx, int lev, int* pdx);
int position(char* idt, int tx);
void enter(enum object k, int* ptx, int lev, int* pdx);
void listenter(enum object k, int* ptx, int lev, int* pdx);
int base(int l, int* s, int b);
void statement_list(bool* fsys, int* ptx, int lev);

#endif //COMPILER_H
