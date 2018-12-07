// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

#ifndef COMPILER_H
#define COMPILER_H

// TODO: 添加要在此处预编译的标头


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#define true 1
#define false 0

#define norw 25       /* 保留字个数 */
#define txmax 512     /* 符号表容量 */
#define nmax 14       /* 数字的最大位数 */
#define al 10         /* 标识符的最大长度 */
#define maxerr 30     /* 允许的最多错误数 */
#define amax 2048     /* 地址上界*/
#define levmax 1      /* 最大允许过程嵌套声明层数*/
#define cxmax 200     /* 最多的虚拟机代码数 */
#define stacksize 10000 /* 运行时数据栈元素最多为1000个 */
#define symnum 57
#define fctnum 11

/* 符号 */
enum symbol {
	nul, ident, number, plus, minus,
	times, slash, oddsym, eql, neq,
	lss, leq, gtr, geq, lparen,
	rparen, comma, semicolon, period, becomes,
	mainsym, continsym, ifsym, breaksym, whilesym,
	writesym, readsym, dosym, callsym, constsym,
	elsesym, selfplus, selfminus, continuesym, switchsym,
	repeatsym, forsym, mod, untilsym, xorsym,
	lbrace, rbrace, lrange, rrange, exitsym,
	casesym, andsym, boolsym, charsym,
	intsym, orsym, varsym, funcsym, notsym,
	defaultsym, colonsym,
};

/* 符号表中的类型 */
enum object {
	constant,
	variable,
	boolean,
	procedure,
	array,
	integer,
	character,
};

/* 虚拟机代码指令 */
enum fct {
	lit, opr, lod,
	sto, cal, ini,
	jmp, jpc, jeq,
	stv, lov,
};

/* 虚拟机代码结构 */
struct instruction {
	enum fct f; /* 虚拟机代码指令 */
	int l;      /* 引用层与声明层的层次差 */
	int a;      /* 根据f的不同而不同 */
};


bool listswitch;   /* 显示虚拟机代码与否 */
bool tableswitch;  /* 显示符号表与否 */
char ch;            /* 存放当前读取的字符，getch 使用 */
enum symbol sym;    /* 当前的符号 */
enum symbol nxtsym;	/* 为解决冲突引入的下一个符号*/
enum symbol oldsym;	/* 为解决冲突引入的上一个符号*/
char id[al + 1];      /* 当前ident，多出的一个字节用于存放0 */
int num;            /* 当前number */
int cc, ll;         /* getch使用的计数器，cc表示当前字符(ch)的位置 */
int cx;             /* 虚拟机代码指针, 取值范围[0, cxmax-1]*/
char line[81];		/* 读取行缓冲区 */
char a[al + 1];       /* 临时符号，多出的一个字节用于存放0 */
struct instruction code[cxmax]; /* 存放虚拟机代码的数组 */
char word[norw][al];        /* 保留字 */
enum symbol wsym[norw];     /* 保留字对应的符号值 */
enum symbol ssym[256];      /* 单字符的符号值 */
char mnemonic[fctnum][5];   /* 虚拟机代码指令名称 */
bool declbegsys[symnum];    /* 表示声明开始的符号集合 */
bool statbegsys[symnum];    /* 表示语句开始的符号集合 */
bool facbegsys[symnum];     /* 表示因子开始的符号集合 */
bool debug;
char errorInfo[100][100];	/* 保存错误信息*/
int oneSize[100];
int oldI;
int isWrite;
enum symbol conf_sym;
char oldId[al + 1];
int conf_i, conf_j, conf_shift, conf_array;
int conf_cons;

/* 符号表结构 */
struct tablestruct
{
	char name[al];	    /* 名字 */
	enum object kind;	/* 类型：const，var或procedure */
	int val;            /* 数值，仅const使用 */
	int level;          /* 所处层，仅const不使用 */
	int adr;            /* 地址，仅const不使用 */
	int size;           /* 需要分配的数据区空间, 仅procedure使用 */
};

struct tablestruct table[txmax]; /* 符号表 */

FILE* fin;      /* 输入源文件 */
FILE* ftable;	/* 输出符号表 */
FILE* fcode;    /* 输出虚拟机代码 */
FILE* foutput;  /* 输出文件及出错示意（如有错）、各行对应的生成代码首地址（如无错） */
FILE* fresult;  /* 输出执行结果 */
FILE* ferrors;	/* 输出错误信息*/
FILE* fdebug;	/* 输出调试信息*/
char fname[al];
int err;        /* 错误计数器 */
int isChar;
int isDo;
int doCX;
int isBool;

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
void expression(bool* fsys, int* ptx, int lev);
void expression_stat(bool* fsys, int* ptx, int lev);
void additive_expr(bool* fsys, int* ptx, int lev);
void simple_expr(bool* fsys, int* ptx, int lev);
void statement(bool* fsys, int* ptx, int lev);
void listall();
void intdeclaration(int* ptx, int lev, int* pdx);
void chardeclaration(int* ptx, int lev, int* pdx);
void booldeclaration(int* ptx, int lev, int* pdx);
void intlistdeclaration(int * ptx, int lev, int * pdx, int length);
void charlistdeclaration(int * ptx, int lev, int * pdx, int length);
void constdeclaration(int* ptx, int lev, int* pdx);
int position(char* idt, int tx);
void enter(enum object k, int* ptx, int lev, int* pdx);
void listenter(enum object k, int* ptx, int lev, int* pdx);
int base(int l, int* s, int b);
void statement_list(bool* fsys, int* ptx, int lev);

#endif //COMPILER_H