/*
* PL/0 complier program implemented in C
*
* The program has been tested on Visual Studio 2010
*
* 使用方法：
* 运行后输入PL/0源程序文件名
* 回答是否输出虚拟机代码
* 回答是否输出符号表
* fcode.txt输出虚拟机代码
* foutput.txt输出源文件、出错示意（如有错）和各行对应的生成代码首地址（如无错）
* fresult.txt输出运行结果
* ftable.txt输出符号表
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define bool int
#define true 1
#define false 0

#define norw 8       /* 保留字个数 */
#define txmax 100     /* 符号表容量 */
#define nmax 14       /* 数字的最大位数 */
#define al 10         /* 标识符的最大长度 */
#define maxerr 30     /* 允许的最多错误数 */
#define amax 2048     /* 地址上界*/
#define levmax 3      /* 最大允许过程嵌套声明层数*/
#define cxmax 200     /* 最多的虚拟机代码数 */
#define stacksize 1000000 /* 运行时数据栈元素最多为500个 */

/* 符号 */
enum symbol {
	nul, ident, number, plus, minus,  //5
	multiply, divide, eql, neq, becomes,  //5
	lss, leq, gtr, geq, lparen, //5
	rparen, lparen2,
	rparen2, lparen3,
	rparen3,
	ifsym, whilesym, elsesym,
	writesym, readsym,
	charsym, intsym, mainsym, //13
	semicolon
};
#define symnum 29

/* 符号表中的类型 */
enum object {
	integer,
	//int_list,
	character,
	//char_list
};

/* 虚拟机代码指令 */
enum fct {
	lit, opr, lod,
	sto, cal, ini,
	jmp, jpc,
};
#define fctnum 8

/* 虚拟机代码结构 */
struct instruction
{
	enum fct f; /* 虚拟机代码指令 */
	int l;      /* 引用层与声明层的层次差 */
	int a;      /* 根据f的不同而不同 */
};

int linecount = 0;
int shift = 0;
int shouldbechar=0; /* 如果是char输出方式不同 */
bool listswitch;   /* 显示虚拟机代码与否 */
bool tableswitch;  /* 显示符号表与否 */
char ch;            /* 存放当前读取的字符，getch 使用 */
enum symbol sym;    /* 当前的符号 */
char id[al + 1];      /* 当前ident，多出的一个字节用于存放0 */
int num;            /* 当前number */
int cc, ll;         /* getch使用的计数器，cc表示当前字符(ch)的位置 */
int cx;             /* 虚拟机代码指针, 取值范围[0, cxmax-1]*/
char line[81];      /* 读取行缓冲区 */
char a[al + 1];       /* 临时符号，多出的一个字节用于存放0 */
struct instruction code[cxmax]; /* 存放虚拟机代码的数组 */
char word[norw][al];        /* 保留字 */
enum symbol wsym[norw];     /* 保留字对应的符号值 */
enum symbol ssym[256];      /* 单字符的符号值 */
char mnemonic[fctnum][5];   /* 虚拟机代码指令名称 */
bool declbegsys[symnum];    /* 表示声明开始的符号集合 */
bool statbegsys[symnum];    /* 表示语句开始的符号集合 */
bool facbegsys[symnum];     /* 表示因子开始的符号集合 */

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
FILE* ferr;  /* 输出错误 */
char fname[al];
int err;        /* 错误计数器 */

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
void simple_expr(bool* fsys, int* ptx, int lev);
void expression_stat(bool* fsys, int* ptx, int lev);
void expression(bool* fsys, int* ptx, int lev);
void statement_list(bool* fsys, int* ptx, int lev);
void statement(bool* fsys, int* ptx, int lev);
void listcode(int cx0);
void listall();
//void vardeclaration(int* ptx, int lev, int* pdx);
//void constdeclaration(int* ptx, int lev, int* pdx);
void intdeclaration(int* ptx, int lev, int* pdx);
void intlistdeclaration(int* ptx, int lev, int* pdx,int length);
void chardeclaration(int* ptx, int lev, int* pdx);
void charlistdeclaration(int* ptx, int lev, int* pdx, int length);
int position(char* idt, int tx);
void enter(enum object k, int* ptx, int lev, int* pdx);
int base(int l, int* s, int b);


/* 主程序开始 */
int main()
{
	bool nxtlev[symnum];
	printf("Input pl/0 file?   ");
	//scanf("%s", fname);		/* 输入文件名 */
	strcpy(fname, "1.txt");
	if ((fin = fopen(fname, "r")) == NULL)
	{
		printf("Can't open the input file!\n");
		exit(1);
	}

	ch = fgetc(fin);
	if (ch == EOF)
	{
		printf("The input file is empty!\n");
		fclose(fin);
		exit(1);
	}
	rewind(fin);

	if ((foutput = fopen("foutput.txt", "w")) == NULL)
	{
		printf("Can't open the output file!\n");
		exit(1);
	}

	if ((ftable = fopen("ftable.txt", "w")) == NULL)
	{
		printf("Can't open ftable.txt file!\n");
		exit(1);
	}
	if ((ferr = fopen("ferr.txt", "w")) == NULL)
	{
		printf("Can't open ferr.txt file!\n");
		exit(1);
	}
	printf("List object codes\n");	/* 是否输出虚拟机代码 */

	listswitch = 1;

	printf("List symbol table\n");	/* 是否输出符号表 */
	tableswitch = 1;

	init();		/* 初始化 */
	err = 0;
	cc = ll = cx = 0;
	ch = ' ';

	getsym();

	addset(nxtlev, declbegsys, statbegsys, symnum);
	block(0, 0, nxtlev);	/* 处理分程序 */

	if (sym != rparen3)
	{
		error(13);
	}

	if (err == 0)
	{
		printf("\n===Parsing success!===\n");
		fprintf(foutput, "\n===Parsing success!===\n");
		fprintf(ferr, "err 0 0 0\n");
		if ((fcode = fopen("fcode.txt", "w")) == NULL)
		{
			printf("Can't open fcode.txt file!\n");
			exit(1);
		}

		if ((fresult = fopen("fresult.txt", "w")) == NULL)
		{
			printf("Can't open fresult.txt file!\n");
			exit(1);
		}

		listall();	 /* 输出所有代码 */
		fclose(fcode);
		fclose(ferr);
		//interpret();	/* 调用解释执行程序 */
		fclose(fresult);
	}
	else
	{
		printf("\n%d errors in pl/0 program!\n", err);
		fprintf(foutput, "\n%d errors in pl/0 program!\n", err);
	}

	fclose(ftable);
	fclose(foutput);
	fclose(fin);

	return 0;
}

/*
* 初始化
*/
void init()
{
	int i;

	/* 设置单字符符号 */
	for (i = 0; i <= 255; i++)
	{
		ssym[i] = nul;
	}
	ssym['+'] = plus;
	ssym['-'] = minus;
	ssym['*'] = multiply;
	ssym['/'] = divide;
	ssym['('] = lparen;
	ssym[')'] = rparen;
	ssym['['] = lparen2;
	ssym[']'] = rparen2;
	ssym['{'] = lparen3;
	ssym['}'] = rparen3;
	ssym[';'] = semicolon;

	/* 设置保留字名字,按照字母顺序，便于二分查找 */
	strcpy(&(word[0][0]), "char");
	strcpy(&(word[1][0]), "else");
	strcpy(&(word[2][0]), "if");
	strcpy(&(word[3][0]), "int");
	strcpy(&(word[4][0]), "main");
	strcpy(&(word[5][0]), "read");
	strcpy(&(word[6][0]), "while");
	strcpy(&(word[7][0]), "write");


	/* 设置保留字符号 */
	wsym[0] = charsym;
	wsym[1] = elsesym;
	wsym[2] = ifsym;
	wsym[3] = intsym;
	wsym[4] = mainsym;
	wsym[5] = readsym;
	wsym[6] = whilesym;
	wsym[7] = writesym;


	/* 设置指令名称 */
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[ini][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");

	/* 设置符号集 */
	for (i = 0; i<symnum; i++)
	{
		declbegsys[i] = false;
		statbegsys[i] = false;
		facbegsys[i] = false;
	}

	/* 设置声明开始符号集 */
	declbegsys[intsym] = true;
	declbegsys[charsym] = true;
	//declbegsys[procsym] = true;

	/* 设置语句开始符号集 */
	//statbegsys[callsym] = true;
	statbegsys[ifsym] = true;
	statbegsys[whilesym] = true;
	statbegsys[readsym] = true;
	statbegsys[writesym] = true;
	statbegsys[lparen3] = true;
	statbegsys[ident] = true;
	statbegsys[semicolon] = true;

	/* 设置因子开始符号集 */
	facbegsys[ident] = true;
	facbegsys[number] = true;
	facbegsys[lparen] = true;
}

/*
* 用数组实现集合的集合运算
*/
int inset(int e, bool* s)
{
	return s[e];
}

int addset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i = 0; i<n; i++)
	{
		sr[i] = s1[i] || s2[i];
	}
	return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i = 0; i<n; i++)
	{
		sr[i] = s1[i] && (!s2[i]);
	}
	return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i = 0; i<n; i++)
	{
		sr[i] = s1[i] && s2[i];
	}
	return 0;
}

/*
*	出错处理，打印出错位置和错误编码
*/
void error(int n)
{
	char space[81];
	memset(space, 32, 81);

	space[cc - 1] = 0; /* 出错时当前符号已经读完，所以cc-1 */
	fprintf(ferr, "err %d %d %d\n", n,linecount, cc-1);
	printf("**%s^%d\n", space, n);
	fprintf(foutput, "**%s^%d\n", space, n);

	err = err + 1;
	if (err > maxerr)
	{
		exit(1);
	}
}

/*
* 过滤空格，读取一个字符
* 每次读一行，存入line缓冲区，line被getsym取空后再读一行
* 被函数getsym调用
*/
void getch()
{
	if (cc == ll) /* 判断缓冲区中是否有字符，若无字符，则读入下一行字符到缓冲区中 */
	{
		if (feof(fin))
		{
			printf("Program is incomplete!\n");
			exit(1);
		}
		ll = 0;
		cc = 0;
		printf("%d ", cx);
		fprintf(foutput, "%d ", cx);
		ch = ' ';
		while (ch != 10)
		{
			if (EOF == fscanf(fin, "%c", &ch))
			{
				line[ll] = 0;
				break;
			}
			if (ch == '\n')linecount++;
			printf("%c", ch);
			fprintf(foutput, "%c", ch);
			line[ll] = ch;
			ll++;
		}
	}
	ch = line[cc];
	cc++;
}

/*
* 词法分析，获取一个符号
*/
void getsym()
{
	int i, j, k;

	while (ch == ' ' || ch == 10 || ch == 9)	/* 过滤空格、换行和制表符 */
	{
		getch();
	}
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) /* 当前的单词是标识符或是保留字 */
	{
		k = 0;
		do {
			if (k < al)
			{
				a[k] = ch;
				k++;
			}
			getch();
		} while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'));
		a[k] = 0;
		strcpy(id, a);
		i = 0;
		j = norw - 1;
		do {    /* 搜索当前单词是否为保留字，使用二分法查找 */
			k = (i + j) / 2;
			if (strcmp(id, word[k]) <= 0)
			{
				j = k - 1;
			}
			if (strcmp(id, word[k]) >= 0)
			{
				i = k + 1;
			}
		} while (i <= j);
		if (i - 1 > j) /* 当前的单词是保留字 */
		{
			sym = wsym[k];
		}
		else /* 当前的单词是标识符 */
		{
			sym = ident;
		}
	}
	else
	{
		if (ch >= '0' && ch <= '9') /* 当前的单词是数字 */
		{
			k = 0;
			num = 0;
			sym = number;
			do {
				num = 10 * num + ch - '0';
				k++;
				getch();;
			} while (ch >= '0' && ch <= '9'); /* 获取数字的值 */
			k--;
			if (k > nmax) /* 数字位数太多 */
			{
				error(1);
			}
		}
		else
		{
			if (ch == '=')		/* 检测等于符号 */
			{
				getch();
				if (ch == '=')
				{
					sym = eql;
					getch();
				}
				else
				{
					sym = becomes;	/* = */
				}
			}
			else
			{
				if (ch == '<')		/* 检测小于或小于等于符号 */
				{
					getch();
					if (ch == '=')
					{
						sym = leq;
						getch();
					}
					else
					{
						sym = lss;
					}
				}
				else
				{
					if (ch == '>')		/* 检测大于或大于等于符号 */
					{
						getch();
						if (ch == '=')
						{
							sym = geq;
							getch();
						}
						else
						{
							sym = gtr;
						}
					}
					else
						if (ch == '!') {
							getch();
							if (ch == '=') { 
								sym = neq; 
								getch(); 
							}
						}
						else
					{
						sym = ssym[ch];		/* 当符号不满足上述条件时，全部按照单字符符号处理 */
							getch();

					}
				}
			}
		}
	}
}

/*
* 生成虚拟机代码
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
void gen(enum fct x, int y, int z)
{
	if (cx >= cxmax)
	{
		printf("Program is too long!\n");	/* 生成的虚拟机代码程序过长 */
		exit(1);
	}
	if (z >= amax)
	{
		printf("Displacement address is too big!\n");	/* 地址偏移越界 */
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
}


/*
* 测试当前符号是否合法
*
* 在语法分析程序的入口和出口处调用测试函数test，
* 检查当前单词进入和退出该语法单位的合法性
*
* s1:	需要的单词集合
* s2:	如果不是需要的单词，在某一出错状态时，
*      可恢复语法分析继续正常工作的补充单词符号集合
* n:  	错误号
*/
void test(bool* s1, bool* s2, int n)
{
	if (!inset(sym, s1))
	{
		error(n);
		/* 当检测不通过时，不停获取符号，直到它属于需要的集合或补救的集合 */
		//while ((!inset(sym, s1)) && (!inset(sym, s2)))
		//{
		//	getsym();
		//}
	}
}

/*
* 编译程序主体 block其实对应program
*
* lev:    当前分程序所在层
* tx:     符号表当前尾指针
* fsys:   当前模块后继符号集合
*/
void block(int lev, int tx, bool* fsys)
{
	int i;

	int dx;                 /* 记录数据分配的相对地址 */
	int tx0;                /* 保留初始tx */
	int cx0;                /* 保留初始cx */
	bool nxtlev[symnum];    /* 在下级函数的参数中，符号集合均为值参，但由于使用数组实现，
							传递进来的是指针，为防止下级函数改变上级函数的集合，开辟新的空间
							传递给下级函数*/

	dx = 3;                 /* 三个空间用于存放静态链SL、动态链DL和返回地址RA  */
	tx0 = tx;		        /* 记录本层标识符的初始位置 */
	table[tx].adr = cx;	    /* 记录当前层代码的开始位置 */
	gen(jmp, 0, 0);         /* 产生跳转指令，跳转位置未知暂时填0 */

	if (lev > levmax)		/* 嵌套层数过多 */
	{
		error(2);
	}
	if (sym == mainsym) { getsym(); }
	if (sym == lparen3) { getsym(); }
	do {


		if (sym == intsym)		/* 遇到变量声明符号，开始处理变量声明 */
		{
			getsym();
			intdeclaration(&tx, lev, &dx);

			if (sym == semicolon)
			{
				getsym();
			}
			else
			{
				if (sym == lparen2)
				{
					getsym();
					//num; 即数组维度
					intlistdeclaration(&tx, lev, &dx,num);
					if (sym == rparen2) 
					{
						getsym(); 
						if (sym == semicolon)
						{
							getsym();
						}
						else {
							error(3);
						}
					}
					else 
					{
						error(4);/*缺少】*/
					}
				}
				else
				{
					error(3);
				} /* 漏掉了分号 */
			}
		}
		if (sym == charsym)		/* 遇到变量声明符号，开始处理变量声明 */
		{
			getsym();
			chardeclaration(&tx, lev, &dx);

			if (sym == semicolon)
			{
				getsym();
			}
			else
			{
				if (sym == lparen2)
				{
					getsym();
					//num; 即数组维度
					charlistdeclaration(&tx, lev, &dx, num);
					if (sym == rparen2)
					{
						getsym();
						if (sym == semicolon)
						{
							getsym();
						}
						else {
							error(3);
						}
					}
					else
					{
						error(4);/*缺少】*/
					}
				}
				else
				{
					error(3);
				} /* 漏掉了分号 */
			}
		}

		//memcpy(nxtlev, statbegsys, sizeof(bool) * symnum);
		//nxtlev[semicolon] = true;
		//test(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));	/* 直到没有声明符号 */

	code[table[tx0].adr].a = cx;	/* 把前面生成的跳转语句的跳转位置改成当前位置 */
	table[tx0].adr = cx;	        /* 记录当前过程代码地址 */
	table[tx0].size = dx;	        /* 声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size */
	cx0 = cx;
	gen(ini, 0, dx);	            /* 生成指令，此指令执行时在数据栈中为被调用的过程开辟dx个单元的数据区 */


	for (i = 1; i <= tx; i++)
	{
		switch (table[i].kind)
		{
		case integer:
			fprintf(ftable, "    %d int %s ", i, table[i].name);
			fprintf(ftable, "lev=%d addr=%d\n", table[i].level, table[i].adr);
			break;
		case character:

			fprintf(ftable, "    %d char %s ", i, table[i].name);
			fprintf(ftable, "lev=%d addr=%d\n", table[i].level, table[i].adr);
			break;
			/*case procedure:

				fprintf(ftable, "    %d proc  %s ", i, table[i].name);
				fprintf(ftable, "lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				break;*/
		}
	}
	printf("\n");
	fprintf(ftable, "\n");


	/* 语句后继符号为分号或end */
	memcpy(nxtlev, fsys, sizeof(bool) * symnum);	/* 每个后继符号集合都包含上层后继符号集合，以便补救 */
	nxtlev[semicolon] = true;
	statement_list(nxtlev, &tx, lev);
	gen(opr, 0, 0);	                    /* 每个过程出口都要使用的释放数据段指令 */
	memset(nxtlev, 0, sizeof(bool) * symnum);	/* 分程序没有补救集合 */
	//test(fsys, nxtlev, 8);            	/* 检测后继符号正确性 */
	listcode(cx0);                      /* 输出本分程序生成的代码 */
}

/*
* 在符号表中加入一项
*
* k:      标识符的种类为const，var或procedure
* ptx:    符号表尾指针的指针，为了可以改变符号表尾指针的值
* lev:    标识符所在的层次
* pdx:    dx为当前应分配的变量的相对地址，分配后要增加1
*
*/
void enter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, id); /* 符号表的name域记录标识符的名字 */
	table[(*ptx)].kind = k;
	switch (k)
	{
	case character:	/* 常量 */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case integer:	/* 变量 */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	//case procedure:	/* 过程 */
	//	table[(*ptx)].level = lev;
	//	break;
	}
}

/*数组除了第一个别的没名字，就不会被找到*/
void enter2(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, ""); /* 符号表的name域记录标识符的名字是空 */
	table[(*ptx)].kind = k;
	switch (k)
	{
	case character:	/* 常量 */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case integer:	/* 变量 */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
		//case procedure:	/* 过程 */
		//	table[(*ptx)].level = lev;
		//	break;
	}
}

/*
* 查找标识符在符号表中的位置，从tx开始倒序查找标识符
* 找到则返回在符号表中的位置，否则返回0
*
* id:    要查找的名字
* tx:    当前符号表尾指针
*/
int position(char* id, int tx)
{
	int i;
	strcpy(table[0].name, id);
	i = tx;
	while (strcmp(table[i].name, id) != 0)
	{
		i--;
	}
	if (table[i].kind == character)shouldbechar=1;
	else shouldbechar = 0;
	return i;
}



/*
* 变量声明处理
*/
void intdeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)
	{
		enter(integer, ptx, lev, pdx);	// 填写符号表
		getsym();
	}
	else
	{
		error(5);	/* var后面应是标识符 */
	}
}
/*
* int数组声明处理
*/
void intlistdeclaration(int * ptx, int lev, int * pdx, int length)
{
	if (sym == number)
	{
		int i = 0;
		for (i = 0; i < length - 1; ++i) {
			enter2(integer, ptx, lev, pdx);// 填写符号表
		}	
		getsym();
	}
	else
	{
		error(6);	/* 数组方括号内应该是 */
	}
}


void chardeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)
	{
		enter(character, ptx, lev, pdx);	// 填写符号表
		getsym();
	}
	else
	{
		error(5);	/* var后面应是标识符 */
	}
}
/*
* char数组声明处理
*/
void charlistdeclaration(int * ptx, int lev, int * pdx, int length)
{
	if (sym == number)
	{
		int i = 0;
		for (i = 0; i < length - 1; ++i) {
			enter2(character, ptx, lev, pdx);// 填写符号表
		}
		getsym();
	}
	else
	{
		error(6);	/* 数组方括号内应该是 */
	}
}

/*
* 输出目标代码清单
*/
void listcode(int cx0)
{
	int i;
	if (listswitch)
	{
		printf("\n");
		for (i = cx0; i < cx; i++)
		{
			printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}

/*
* 输出所有目标代码
*/
void listall()
{
	int i;
	if (listswitch)
	{
		for (i = 0; i < cx; i++)
		{
			printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
			fprintf(fcode, "%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}


void statement_list(bool* fsys, int* ptx, int lev)
{
	bool nxtlev[symnum];
	while (inset(sym, statbegsys)) {
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		statement(nxtlev, ptx, lev);
	}
}
/*
* 语句处理2
*/
void statement(bool* fsys, int* ptx, int lev)
{
	int i, cx1, cx2;
	bool nxtlev[symnum];

	//if (sym == ident)	/* 准备按照赋值语句处理  expression_stat -> var=expression; */
	//{
	//	i = position(id, *ptx);/* 查找标识符在符号表中的位置 */
	//	if (i == 0)
	//	{
	//		error(11);	/* 标识符未声明 */
	//	}
	//	else
	//	{
	//		if (table[i].kind != character && table[i].kind != integer)
	//		{
	//			error(12);	/* 赋值语句中，赋值号左部标识符应该是变量 */
	//			i = 0;
	//		}
	//		else
	//		{
	//			getsym();
	//			if (sym == becomes)
	//			{
	//				getsym();
	//			}
	//			else
	//			{
	//				error(13);	/* 没有检测到赋值符号 */
	//			}
	//			memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	//			expression(nxtlev, ptx, lev);	/* 处理赋值符号右侧表达式 */
	//			if (i != 0)
	//			{
	//				/* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
	//				gen(sto, lev - table[i].level, table[i].adr);
	//			}
	//			if (sym == semicolon) {
	//				getsym(); 
	//			}
	//			else {
	//				error(123); 
	//			}
	//		}
	//	}
	//}
	//************************************************************************************************************

	if (sym == readsym)	/* 准备按照read语句处理 */
	{
		getsym();

		if (sym == ident)
		{
			i = position(id, *ptx);	/* 查找要读的变量 */
		}
		else
		{
			i = 0;
		}

		if (i == 0)
		{
			error(7);	/* read语句括号中的标识符应该是声明过的变量 */
		}
		else
		{
			getsym();
			if (sym == semicolon) {
				getsym();
				gen(opr, 0, 16);	/* 生成输入指令，读取值到栈顶 */
				gen(sto, lev - table[i].level, table[i].adr);	/* 将栈顶内容送入变量单元中 */
			}
			else if (sym == lparen2) {
				getsym();
				if (sym == number) {
					gen(opr, 0, 16);	/* 生成输入指令，读取值到栈顶 */
					gen(sto, lev - table[i].level, table[i].adr + num);	/* 将栈顶内容送入变量单元中 */
					getsym();
					if (sym == rparen2) {
						getsym();
						if (sym == semicolon)
							getsym();
						else
							error(3);//没分号
					}
					else { error(4);/*数组没有】*/ }
				}
				else { error(6);/*数组【】内必须是数字*/ }
			}
			else { error(3); }//没有分号
		}

	}
	else
	{
		if (sym == writesym)	/* 准备按照write语句处理 */
		{
			getsym();
			memcpy(nxtlev, fsys, sizeof(bool) * symnum);
			nxtlev[rparen] = true;
			expression(nxtlev, ptx, lev);	/* 调用表达式处理 */
			if (!shouldbechar) {
				gen(opr, 0, 14);	/* 生成输出指令，输出栈顶的值 */
				gen(opr, 0, 15);	/* 生成换行指令 */
			}
			else {
				gen(opr, 0, 17);	/* 生成输出指令，输出栈顶的值 */
				gen(opr, 0, 15);	/* 生成换行指令 */
			}
			if (sym == semicolon)
			{
				getsym();
			}
			else 
			{
				error(3);
			}//没有分号
		}
		//************************************************************************************************************
		else
		{
			if (false) // sym == callsym)	/* 准备按照call语句处理 */
			{
				getsym();
				if (sym != ident)
				{
					error(8);	/* call后应为标识符 */
				}
				else
				{
					i = position(id, *ptx);
					if (i == 0)
					{
						error(9);	/* 过程名未找到 */
					}
					else
					{
						//if (table[i].kind == procedure)
						{
							gen(cal, lev - table[i].level, table[i].adr);	/* 生成call指令 */
						}
						//else
						{
							error(8);	/* call后标识符类型应为过程 */
						}
					}
					getsym();
				}
			}
			//************************************************************************************************************
			else
			{

				if (sym == ifsym)	/* 准备按照if语句处理 */
				{
					getsym();
					if (sym == lparen)getsym();
					else error(10);
					memcpy(nxtlev, fsys, sizeof(bool) * symnum);
					nxtlev[rparen] = true;	/* 后继符号为) */
					//condition(nxtlev, ptx, lev); /* 调用条件处理 */
					expression(nxtlev, ptx, lev); /* 调用条件处理 */
					if (sym == rparen)
					{
						getsym();
					}
					else
					{
						error(11);	/* 缺少) */
					}
					cx1 = cx;	/* 保存当前指令地址 */
					gen(jpc, 0, 0);	/* 生成条件跳转指令，跳转地址未知，暂时写0 */
					statement(fsys, ptx, lev);	/* 处理then后的语句 */
					
					cx2 = cx;
					gen(jmp, 0, 0);
					code[cx1].a = cx;	/* 经statement处理后，cx为then后语句执行完的位置，它正是前面未定的跳转地址，此时进行回填 */
					if (sym == elsesym) {
						getsym();
						memcpy(nxtlev, fsys, sizeof nxtlev);
						statement(nxtlev, ptx, lev);
					}
					code[cx2].a = cx;
				}
				//************************************************************************************************************
				else
				{
					if (sym == whilesym)	/* 准备按照while语句处理 */
					{
						cx1 = cx;	/* 保存判断条件操作的位置 */
						getsym();
						memcpy(nxtlev, fsys, sizeof(bool) * symnum);
						if (sym == lparen)
						{
							getsym();
						}
						else
						{
							error(12);	/* 缺少( */
						}
						expression(nxtlev, ptx, lev);	/* 调用条件处理 */
						cx2 = cx;	/* 保存循环体的结束的下一个位置 */
						gen(jpc, 0, 0);	/* 生成条件跳转，但跳出循环的地址未知，标记为0等待回填 */
						if (sym == rparen)
						{
							getsym();
						}
						else
						{
							error(11);	/* 缺少) */
						}
						statement(fsys, ptx, lev);	/* 循环体 */

						gen(jmp, 0, cx1);	/* 生成条件跳转指令，跳转到前面判断条件操作的位置 */
						code[cx2].a = cx;	/* 回填跳出循环的地址 */
					}
					//************************************************************************************************************
					else
					{
						if (sym == semicolon) /* expression_stat -> ; */
						{
							getsym();
						}
						else
						{
							if (sym == lparen3)/* compound_stat */
							{
								getsym();
								statement_list(fsys, ptx, lev);
								if (sym == rparen3)
								{
									getsym();
									printf("compund end\n");
								}
								else { error(13); } /*没有右大括号*/
							}
							else
								//***********************************************************
							{
								memcpy(nxtlev, fsys, sizeof(bool) * symnum);
								expression_stat(nxtlev, ptx, lev);
							}
						}
					}
				}
			}
		}
	}


	//memset(nxtlev, 0, sizeof(bool) * symnum);	/* 语句结束无补救集合 */
	//test(fsys, nxtlev, 19);	/* 检测语句结束的正确性 */
}
void expression_stat(bool* fsys, int* ptx, int lev)
{
	printf("expression_stat\n");
	bool nxtlev[symnum];
	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	if (sym == semicolon) { getsym(); }
	else {
		expression(nxtlev, ptx, lev);
		if (sym != semicolon) { error(3);/*没有分号*/ }
		else { getsym(); }
	}
}
/*
* expression
*/
void expression(bool* fsys, int* ptx, int lev)
{
	int i;
	bool nxtlev[symnum];
	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	if (inset(sym, facbegsys))
	{
		//************
		if (sym == ident)	/* 准备按照赋值语句处理  expression_stat -> var=expression; */
		{
			i = position(id, *ptx);/* 查找标识符在符号表中的位置 */
			if (i == 0)
			{
				error(14);	/* 标识符未声明 */
			}
			else
			{
				if (table[i].kind != character && table[i].kind != integer)
				{
					error(15);	/* 赋值语句中，赋值号左部标识符应该是变量 */
					i = 0;
				}
				else
				{
					//getsym();
					shift = 0;	
					simple_expr(nxtlev, ptx, lev); //如果失败了sym就是=，意味着应该走var=expression
					int shift2 = shift;
					if (sym == becomes)
					{
						getsym();
						memcpy(nxtlev, fsys, sizeof(bool) * symnum);
						expression(nxtlev, ptx, lev);	/* 处理赋值符号右侧表达式 */
						if (i != 0)
						{
							/* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
							gen(sto, lev - table[i].level, table[i].adr+shift2);
						}
					}
					else if (sym == lparen2) { //这条路不可能走
						printf("走进了不可能走的路\n");
						int xx = 0;
						getsym();
						if (sym == number)
						{
							xx = num; 
							getsym();
							if (sym == rparen2) {
								getsym(); if (sym == becomes) {
									getsym();
									memcpy(nxtlev, fsys, sizeof(bool) * symnum);
									expression(nxtlev, ptx, lev);	/* 处理赋值符号右侧表达式 */
									if (i != 0)
									{
										/* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
										gen(sto, lev - table[i].level, table[i].adr + xx);
									}
								}
								else { error(16);/*未知错误*/ }
							}
							else { error(4);/*没有】*/ }
						}
						else { error(6);/*数组没数字*/ }
					}
					else
					{
						printf("走simple expression 成功了！\n");
					}
					

				}
			}
		}
		else { simple_expr(nxtlev, ptx, lev); }
		//**********
	//	getsym();
	//	if (sym == becomes) //var=expression
	//	{
	//
	//	}
	//	else
	//	{
	//		simple_expr(fsys, ptx, lev);
	//	}
	}
}

/*
* 表达式处理 即additive_expr ，是simple_expr一部分
*/
void additive_expr(bool* fsys, int* ptx, int lev)
{
	enum symbol addop;	/* 用于保存正负号 */
	bool nxtlev[symnum];

	if (sym == plus || sym == minus)	/* 表达式开头有正负号，此时当前表达式被看作一个正的或负的项 */
	{
		addop = sym;	/* 保存开头的正负号 */
		getsym();
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);	/* 处理项 */
		if (addop == minus)
		{
			gen(opr, 0, 1);	/* 如果开头为负号生成取负指令 */
		}
	}
	else	/* 此时表达式被看作项的加减 */
	{
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);	/* 处理项 */
	}
	while (sym == plus || sym == minus)
	{
		addop = sym;
		getsym();
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);	/* 处理项 */
		if (addop == plus)
		{
			gen(opr, 0, 2);	/* 生成加法指令 */
		}
		else
		{
			gen(opr, 0, 3);	/* 生成减法指令 */
		}
	}
}

/*
* 项处理 和pascal一样
*/
void term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;	/* 用于保存乘除法符号 */
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[multiply] = true;
	nxtlev[divide] = true;
	factor(nxtlev, ptx, lev);	/* 处理因子 */
	while (sym == multiply || sym == divide)
	{
		mulop = sym;
		getsym();
		factor(nxtlev, ptx, lev);
		if (mulop == multiply)
		{
			gen(opr, 0, 4);	/* 生成乘法指令 */
		}
		else
		{
			gen(opr, 0, 5);	/* 生成除法指令 */
		}
	}
}

/*
* 因子处理   和pascal一样
*/
void factor(bool* fsys, int* ptx, int lev)
{
	int i;
	bool nxtlev[symnum];
	//test(facbegsys, fsys, 24);	/* 检测因子的开始符号 */
	while (inset(sym, facbegsys)) 	/* 循环处理因子 */
	{
		if (sym == ident)	/* 因子为常量或变量 */
		{
			i = position(id, *ptx);	/* 查找标识符在符号表中的位置 */
			if (i == 0)
			{
				error(14);	/* 标识符未声明 */
				getsym();
			}
			else
			{
				getsym();
				if (sym == lparen2) {
					int xx = 0;
					getsym();
					if (sym == number) {
						xx = num; getsym();
						if (sym == rparen2) {
							getsym();
							switch (table[i].kind)
							{
							case integer:	/* 标识符为变量 */
								shift = xx;
								gen(lod, lev - table[i].level, table[i].adr+xx);	/* 找到变量地址并将其值入栈 */
								break;
							case character:	/* 标识符为变量 */
								shift = xx;
								gen(lod, lev - table[i].level, table[i].adr+xx);	/* 找到变量地址并将其值入栈 */
								break;
								//case procedure:	/* 标识符为过程 */
								//	error(21);	/* 不能为过程 */
								//	break;
							}
						}
						else { error(4);/*没有】*/ }
					}
					else { error(6);/*数组没有数字*/ }
				}
				else {
					switch (table[i].kind)
					{
					case integer:	/* 标识符为变量 */
						gen(lod, lev - table[i].level, table[i].adr);	/* 找到变量地址并将其值入栈 */
						break;
					case character:	/* 标识符为变量 */
						gen(lod, lev - table[i].level, table[i].adr);	/* 找到变量地址并将其值入栈 */
						break;
						//case procedure:	/* 标识符为过程 */
						//	error(21);	/* 不能为过程 */
						//	break;
					}
				}
			}
		}
		else
		{
			if (sym == number)	/* 因子为数 */
			{
				if (num > amax)
				{
					error(17); /* 数越界 */
					num = 0;
				}
				gen(lit, 0, num);
				getsym();
			}
			else
			{
				if (sym == lparen)	/* 因子为表达式 */
				{
					getsym();
					memcpy(nxtlev, fsys, sizeof(bool) * symnum);
					nxtlev[rparen] = true;
					expression(nxtlev, ptx, lev);
					if (sym == rparen)
					{
						getsym();
					}
					else
					{
						error(11);	/* 缺少右括号 */
					}
				}
			}
		}
		//memset(nxtlev, 0, sizeof(bool) * symnum);
		//nxtlev[lparen] = true;
		//test(fsys, nxtlev, 4); /* 一个因子处理完毕，遇到的单词应在fsys集合中 */
	}
}

/*
* 条件处理  simple_expr一部分  已修改
*/
void simple_expr(bool* fsys, int* ptx, int lev)
{
	enum symbol relop;
	bool nxtlev[symnum];

	//if (sym == oddsym)	/* 准备按照odd运算处理 */
	//{
	//	getsym();
	//	expression(fsys, ptx, lev);
	//	gen(opr, 0, 6);	/* 生成odd指令 */
	//}
	//else
	{
		/* 逻辑表达式处理 */
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[eql] = true;
		nxtlev[neq] = true;
		nxtlev[lss] = true;
		nxtlev[leq] = true;
		nxtlev[gtr] = true;
		nxtlev[geq] = true;
		additive_expr(nxtlev, ptx, lev);
		if (sym != eql && sym != neq && sym != lss && sym != leq && sym != gtr && sym != geq)
		{
			/* 应该是纯additive_expr 什么都不做 */
		}
		else
		{
			relop = sym;
			getsym();
			additive_expr(fsys, ptx, lev);
			switch (relop)
			{
			case eql:
				gen(opr, 0, 8);
				break;
			case neq:
				gen(opr, 0, 9);
				break;
			case lss:
				gen(opr, 0, 10);
				break;
			case geq:
				gen(opr, 0, 11);
				break;
			case gtr:
				gen(opr, 0, 12);
				break;
			case leq:
				gen(opr, 0, 13);
				break;
			}
		}
	}
}

/*
* 解释程序
*/
int s[stacksize];	/* 栈 需求大 不能是局部变量 */
void interpret()
{
	int p = 0; /* 指令指针 */
	int b = 1; /* 指令基址 */
	int t = 0; /* 栈顶指针 */
	struct instruction i;	/* 存放当前指令 */
	

	printf("Start pl0\n");
	fprintf(fresult, "----开始执行---\n");
	s[0] = 0; /* s[0]不用 */
	s[1] = 0; /* 主程序的三个联系单元均置为0 */
	s[2] = 0;
	s[3] = 0;
	do {
		i = code[p];	/* 读当前指令 */
		p = p + 1;
		switch (i.f)
		{
		case lit:	/* 将常量a的值取到栈顶 */
			t = t + 1;
			s[t] = i.a;
			break;
		case opr:	/* 数学、逻辑运算 */
			switch (i.a)
			{
			case 0:  /* 函数调用结束后返回 */
				t = b - 1;
				p = s[t + 3];
				b = s[t + 2];
				break;
			case 1: /* 栈顶元素取反 */
				s[t] = -s[t];
				break;
			case 2: /* 次栈顶项加上栈顶项，退两个栈元素，相加值进栈 */
				t = t - 1;
				s[t] = s[t] + s[t + 1];
				break;
			case 3:/* 次栈顶项减去栈顶项 */
				t = t - 1;
				s[t] = s[t] - s[t + 1];
				break;
			case 4:/* 次栈顶项乘以栈顶项 */
				t = t - 1;
				s[t] = s[t] * s[t + 1];
				break;
			case 5:/* 次栈顶项除以栈顶项 */
				t = t - 1;
				s[t] = s[t] / s[t + 1];
				break;
			case 6:/* 栈顶元素的奇偶判断 */
				s[t] = s[t] % 2;
				break;
			case 8:/* 次栈顶项与栈顶项是否相等 */
				t = t - 1;
				s[t] = (s[t] == s[t + 1]);
				break;
			case 9:/* 次栈顶项与栈顶项是否不等 */
				t = t - 1;
				s[t] = (s[t] != s[t + 1]);
				break;
			case 10:/* 次栈顶项是否小于栈顶项 */
				t = t - 1;
				s[t] = (s[t] < s[t + 1]);
				break;
			case 11:/* 次栈顶项是否大于等于栈顶项 */
				t = t - 1;
				s[t] = (s[t] >= s[t + 1]);
				break;
			case 12:/* 次栈顶项是否大于栈顶项 */
				t = t - 1;
				s[t] = (s[t] > s[t + 1]);
				break;
			case 13: /* 次栈顶项是否小于等于栈顶项 */
				t = t - 1;
				s[t] = (s[t] <= s[t + 1]);
				break;
			case 14:/* 栈顶值输出 */
				printf("%d", s[t]);
				fprintf(fresult, "%d", s[t]);
				t = t - 1;
				break;
			case 15:/* 输出换行符 */
				printf("\n");
				fprintf(fresult, "\n");
				break;
			case 16:/* 读入一个输入置于栈顶 */
				t = t + 1;
				printf("?");
				fprintf(fresult, "?");
				scanf("%d", &(s[t]));
				fprintf(fresult, "%d\n", s[t]);
				break;
			case 17:/* 栈顶值输出 当作char */
				printf("%c", s[t]);
				fprintf(fresult, "%c", s[t]);
				t = t - 1;
				break;
			}
			break;
		case lod:	/* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
			t = t + 1;
			s[t] = s[base(i.l, s, b) + i.a];
			break;
		case sto:	/* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
			s[base(i.l, s, b) + i.a] = s[t];
			//t = t - 1;  //这样无法嵌套赋值
			break;
		case cal:	/* 调用子过程 */
			s[t + 1] = base(i.l, s, b);	/* 将父过程基地址入栈，即建立静态链 */
			s[t + 2] = b;	/* 将本过程基地址入栈，即建立动态链 */
			s[t + 3] = p;	/* 将当前指令指针入栈，即保存返回地址 */
			b = t + 1;	/* 改变基地址指针值为新过程的基地址 */
			p = i.a;	/* 跳转 */
			break;
		case ini:	/* 在数据栈中为被调用的过程开辟a个单元的数据区 */
			t = t + i.a;
			break;
		case jmp:	/* 直接跳转 */
			p = i.a;
			break;
		case jpc:	/* 条件跳转 */
			if (s[t] == 0)
				p = i.a;
			t = t - 1;
			break;
		}
	} while (p != 0);
	printf("End pl0\n");
	fprintf(fresult, "----执行成功---\n");
}

/* 通过过程基址求上l层过程的基址 */
int base(int l, int* s, int b)
{
	int b1;
	b1 = b;
	while (l > 0)
	{
		b1 = s[b1];
		l--;
	}
	return b1;
}


