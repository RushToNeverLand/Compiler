﻿// compiler.c: 与预编译标头对应的源文件；编译成功所必需的

#include "compiler.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 一般情况下，忽略此文件，但如果你使用的是预编译标头，请保留它。

/*
 * 用数组实现集合的集合运算
 */
int inset(int e, bool* s) {
	return s[e];
}

int addset(bool* sr, bool* s1, bool* s2, int n) {
	int i;
	for (i = 0; i < n; i++) {
		sr[i] = s1[i] || s2[i];
	}
	return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n) {
	int i;
	for (i = 0; i < n; i++) {
		sr[i] = s1[i] && (!s2[i]);
	}
	return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n) {
	int i;
	for (i = 0; i < n; i++) {
		sr[i] = s1[i] && s2[i];
	}
	return 0;
}

/*
 *	出错处理，打印出错位置和错误编码
 */
void error(int n) {
	char space[81];
	memset(space, 32, 81);

	space[cc - 1] = 0; /* 出错时当前符号已经读完，所以cc-1 */


	printf("**%s^%d\n", space);
	printf("%s\n", errorInfo[n]);
	fprintf(foutput, "**%s^%d\n", space, n);
	fprintf(foutput, "%s\n", errorInfo[n]);

	err = err + 1;
	if (err > maxerr) {
		exit(1);
	}
}

/*
 * 过滤空格，读取一个字符
 * 每次读一行，存入line缓冲区，line被getsym取空后再读一行
 * 被函数getsym调用
 */
void getch() {
	if (cc == ll) /* 判断缓冲区中是否有字符，若无字符，则读入下一行字符到缓冲区中 */
	{
		if (feof(fin)) {
			printf("Program is incomplete!\n");
			exit(1);
		}
		ll = 0;
		cc = 0;
		printf("%d ", cx);
		fprintf(foutput, "%d ", cx);
		ch = ' ';
		while (ch != 10) {
			if (EOF == fscanf(fin, "%c", &ch)) {
				line[ll] = 0;
				break;
			}
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
			if (k < al) {
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
			if (strcmp(id, word[k]) <= 0) {
				j = k - 1;
			}
			if (strcmp(id, word[k]) >= 0) {
				i = k + 1;
			}
		} while (i <= j);
		if (i - 1 > j)	/* 当前的单词是保留字 */
		{
			sym = wsym[k];
		}
		else			/* 当前的单词是标识符 */
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
				error(14);
			}
		}
		else
		{
			if (ch == '=') {		/* 检测赋值符号 */
				getch();
				if (ch == '=') {
					sym = eql;
					getch();
				}
				else {
					sym = becomes;	/* = */
				}
			}
			else if (ch == '<')		/* 检测小于或小于等于符号 */
			{
				getch();
				if (ch == '=') {
					sym = leq;
					getch();
				}
				else {
					sym = lss;
				}
			}
			else if (ch == '>')		/* 检测大于或大于等于符号 */
			{
				getch();
				if (ch == '=') {
					sym = geq;
					getch();
				}
				else {
					sym = gtr;
				}
			}
			else if (ch == '!')
			{
				getch();
				if (ch == '=') {
					sym = neq;
					getch();
				}
				else {
					sym = nul;
				}
			}
			else if (ch == '+') {
				getch();
				if (ch == '+') {
					sym = selfplus;
					getch();
				}
				else {
					sym = plus;
				}
			}
			else if (ch == '-') {
				getch();
				if (ch == '-') {
					sym = selfminus;
					getch();
				}
				else {
					sym = minus;
				}
			}
			else if (ch == '/') {
				getch();
				if (ch == '/') {			 /* 行注释*/
					cc = ll;
					getch();
					getsym();
				}
				else {
					sym = slash;
					getch();
				}
			}
			else {
				sym = ssym[ch];		/* 按照单字符符号处理 */
				getch();
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
void gen(enum fct x, int y, int z) {
	if (cx >= cxmax) {
		printf("生成的虚拟机代码程序过长!\n");	/* 生成的虚拟机代码程序过长 */
		exit(1);
	}
	if (z >= amax) {
		printf("地址偏移越界!\n");	/* 地址偏移越界 */
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
	printf("%d %d\n", sym, s1[sym]);
	if (!inset(sym, s1)) {
		error(n);
		/* 当检测不通过时，不停获取符号，直到它属于需要的集合或补救的集合 */
		while ((!inset(sym, s1)) && (!inset(sym, s2))) {
			getsym();
		}
	}
}

/*
 * 初始化
 */
void init()
{
	int i;

	/* 设置单字符符号 */
	memset(ssym, 0, sizeof(ssym));

	ssym['+'] = plus;
	ssym['-'] = minus;
	ssym['*'] = times;
	ssym['/'] = slash;
	ssym['('] = lparen;
	ssym[')'] = rparen;
	ssym['{'] = lbrace;
	ssym['}'] = rbrace;
	ssym['['] = lrange;
	ssym[']'] = rrange;
	ssym['='] = eql;
	ssym[','] = comma;
	ssym[';'] = semicolon;
	ssym['%'] = mod;
	ssym['^'] = xorsym;
	ssym['&'] = andsym;
	ssym['|'] = orsym;
	ssym['~'] = notsym;
	ssym[':'] = colonsym;

	/* 设置保留字名字,按照字母顺序，便于二分查找 */
	strcpy(&(word[0][0]), "and");
	strcpy(&(word[1][0]), "bool");
	strcpy(&(word[2][0]), "break");
	strcpy(&(word[3][0]), "call");
	strcpy(&(word[4][0]), "case");
	strcpy(&(word[5][0]), "char");
	strcpy(&(word[6][0]), "const");
	strcpy(&(word[7][0]), "continue");
	strcpy(&(word[8][0]), "default");
	strcpy(&(word[9][0]), "do");
	strcpy(&(word[10][0]), "else");
	strcpy(&(word[11][0]), "exit");
	strcpy(&(word[12][0]), "for");
	strcpy(&(word[13][0]), "func");
	strcpy(&(word[14][0]), "if");
	strcpy(&(word[15][0]), "int");
	strcpy(&(word[16][0]), "main");
	strcpy(&(word[17][0]), "odd");
	strcpy(&(word[18][0]), "or");
	strcpy(&(word[19][0]), "read");
	strcpy(&(word[20][0]), "repeat");
	strcpy(&(word[21][0]), "switch");
	strcpy(&(word[22][0]), "until");
	strcpy(&(word[23][0]), "while");
	strcpy(&(word[24][0]), "write");

	/* 设置保留字符号 */
	wsym[0] = andsym;
	wsym[1] = boolsym;
	wsym[2] = breaksym;
	wsym[3] = callsym;
	wsym[4] = casesym;
	wsym[5] = charsym;
	wsym[6] = constsym;
	wsym[7] = continuesym;
	wsym[8] = defaultsym;
	wsym[9] = dosym;
	wsym[10] = elsesym;
	wsym[11] = exitsym;
	wsym[12] = forsym;
	wsym[13] = funcsym;
	wsym[14] = ifsym;
	wsym[15] = intsym;
	wsym[16] = mainsym;
	wsym[17] = oddsym;
	wsym[18] = orsym;
	wsym[19] = readsym;
	wsym[20] = repeatsym;
	wsym[21] = switchsym;
	wsym[22] = untilsym;
	wsym[23] = whilesym;
	wsym[24] = writesym;

	/* 设置指令名称 */
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[ini][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");
	strcpy(&(mnemonic[jeq][0]), "jeq");
	strcpy(&(mnemonic[stv][0]), "stv");
	strcpy(&(mnemonic[lov][0]), "lov");

	/* 设置符号集 */
	memset(declbegsys, false, sizeof(declbegsys));
	memset(statbegsys, false, sizeof(statbegsys));
	memset(facbegsys, false, sizeof(facbegsys));


	/* 设置声明开始符号集 */
	declbegsys[constsym] = true;
	declbegsys[intsym] = true;
	declbegsys[charsym] = true;
	declbegsys[boolsym] = true;
	declbegsys[funcsym] = true;
	declbegsys[mainsym] = true;

	/* 设置语句开始符号集 */
	statbegsys[callsym] = true;
	statbegsys[dosym] = true;
	statbegsys[ifsym] = true;
	statbegsys[whilesym] = true;
	statbegsys[readsym] = true;
	statbegsys[writesym] = true;
	statbegsys[forsym] = true;
	statbegsys[repeatsym] = true;
	statbegsys[switchsym] = true;
	statbegsys[ident] = true;
	statbegsys[lbrace] = true;
	statbegsys[notsym] = true;
	statbegsys[selfminus] = true;
	statbegsys[selfplus] = true;

	/* 设置因子开始符号集 */
	facbegsys[ident] = true;
	facbegsys[number] = true;
	facbegsys[lparen] = true;
	facbegsys[notsym] = true;
	facbegsys[selfminus] = true;
	facbegsys[selfplus] = true;


	strcpy(errorInfo[0], "声明符号后面缺少标识符");
	strcpy(errorInfo[1], "缺少';'");
	strcpy(errorInfo[2], "语句或者函数声明开始符号错误");
	strcpy(errorInfo[3], "缺少语句开始符号");
	strcpy(errorInfo[4], "缺少语句结束符号");
	strcpy(errorInfo[5], "编译未完成");
	strcpy(errorInfo[6], "标识符未声明");
	strcpy(errorInfo[7], "赋值语句等号左侧不是变量");
	strcpy(errorInfo[8], "缺少赋值符号");
	strcpy(errorInfo[9], "[]内缺少数字");
	strcpy(errorInfo[10], "语句结束符号错误");
	strcpy(errorInfo[11], "缺少关系运算符");
	strcpy(errorInfo[12], "标识符不能是函数");
	strcpy(errorInfo[13], "因子开始符号错误");
	strcpy(errorInfo[14], "程序中数字位数太长，溢出错误");
	strcpy(errorInfo[15], "数字大小超出范围");
	strcpy(errorInfo[16], "缺少')'");
	strcpy(errorInfo[17], "缺少'('");
	strcpy(errorInfo[18], "变量未声明");
	strcpy(errorInfo[19], "function后面缺少'()'");
	strcpy(errorInfo[20], "缺少'{'");
	strcpy(errorInfo[21], "缺少'}'");
	strcpy(errorInfo[22], "缺少'in'");
	strcpy(errorInfo[23], "for循环缺少左边界");
	strcpy(errorInfo[24], "for循环缺少右边界");
	strcpy(errorInfo[25], "for循环缺少'...'");
	strcpy(errorInfo[30], "函数嵌套层数过多");
	strcpy(errorInfo[31], "call声明后面缺少过程名");
	strcpy(errorInfo[32], "=后应是数字");
	strcpy(errorInfo[33], "cannot find the single symbol");
	strcpy(errorInfo[34], "数组缺少[");
	strcpy(errorInfo[35], "声明数组缺少声明数组大小");
	strcpy(errorInfo[36], "数组缺少]");
	strcpy(errorInfo[37], "缺少标识符main");
	strcpy(errorInfo[38], "缺少标识符until");
	strcpy(errorInfo[39], "缺少标识符while");
	strcpy(errorInfo[40], "not bool type");
	strcpy(errorInfo[41], "index of array list exceed");
	strcpy(errorInfo[42], "缺少'break'");
	strcpy(errorInfo[43], "bool赋值错误");
	strcpy(errorInfo[44], "not后应为bool类型");
	strcpy(errorInfo[45], "++或--运算符对象只能是var");

	err = 0;
	cc = ll = cx = 0;
	ch = ' ';
	isChar = 0;
	isBool = 0;
	conf_sym = -1;

	conf_i = 0;
	conf_j = 0;
	conf_shift = -1;
}

void compile()
{
	bool nxtlev[symnum];
	init();

	getsym();

	if (sym == mainsym) {
		getsym();
		if (sym == lbrace) {
			getsym();
			addset(nxtlev, declbegsys, statbegsys, symnum);
			block(0, 0, nxtlev);	/* 处理分程序 */
			if (sym == rbrace) {

			}
			else {
				error(21);
			}
		}
		else {
			error(20);
		}
	}
	else {
		error(37);
	}

	if (err == 0) {
		fprintf(foutput, "\n===Parsing success!===\n");
	}
	if ((fcode = fopen("fcode.txt", "w")) == NULL) {
		printf("Can't open fcode.txt file!\n");
		exit(1);
	}
	if ((fresult = fopen("fresult.txt", "w")) == NULL) {
		printf("Can't open fresult.txt file!\n");
		exit(1);
	}


	listall();			/* 输出所有代码 */
	interpret();		/* 调用解释执行程序 */

	fclose(fcode);
	fclose(fresult);
	fclose(ftable);
	fclose(foutput);
	fclose(fin);
}

/*
 * 编译程序主体
 *
 * lev:    当前分程序所在层
 * tx:     符号表当前尾指针
 * fsys:   当前模块后继符号集合
 */
void block(int lev, int tx, bool* fsys)
{
	int i, oldNum;

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
		error(32);
	}


	do {

		while (sym == constsym) {
			getsym();
			constdeclaration(&tx, lev, &dx);
			if (sym == semicolon) {
				getsym();
			}
			else {
				error(9); /* 漏掉了分号 */
			}
		}

		while (sym == intsym) {
			getsym();
			intdeclaration(&tx, lev, &dx);
			if (sym == semicolon) {
				getsym();
			}
			else if (sym == lrange) {
				getsym();
				if (sym == number) {
					oneSize[tx] = num;
					intlistdeclaration(&tx, lev, &dx, num);
				}
				else if (sym == ident) {
					i = position(id, tx);
					if (i == 0) error(18);
					if (table[i].kind != constant) error(6);
					num = table[i].val;
					oneSize[tx] = num;
					intlistdeclaration(&tx, lev, &dx, num);
				}
				else {
					error(6);
				}
				if (sym == rrange) {
					getsym();
					if (sym == semicolon) {
						getsym();
					}
					else {
						error(1);
					}
				}
				else {
					error(4);			/* 缺少]*/
				}

			}
			else {
				error(9);				/* 漏掉了分号*/
			}
		}

		while (sym == charsym) {
			getsym();
			chardeclaration(&tx, lev, &dx);
			if (sym == semicolon) {
				getsym();
			}
			else if (sym == lrange) {
				getsym();
				if (sym == number) {
					oneSize[tx] = num;
					charlistdeclaration(&tx, lev, &dx, num);
				}
				else if (sym == ident) {
					i = position(id, tx);
					if (i == 0) error(18);
					if (table[i].kind != constant) error(6);
					num = table[i].val;
					oneSize[tx] = num;
					charlistdeclaration(&tx, lev, &dx, num);
				}
				else {
					error(6);
				}
				if (sym == rrange) {
					getsym();
					if (sym == semicolon) {
						getsym();
					}
					else {
						error(1);
					}
				}
				else {
					error(4);			/* 缺少]*/
				}
			}
			else {
				error(1);				/* 漏掉了分号 */
			}
		}
		while (sym == boolsym) {
			getsym();
			booldeclaration(&tx, lev, &dx);
			if (sym == semicolon) {
				getsym();
			}
			else {
				error(1);				/* 漏掉了分号 */
			}
		}
		while (sym == funcsym) {
			getsym();

			if (sym == ident) {
				enter(procedure, &tx, lev, &dx);	/* 填写符号表 */
				getsym();
			}
			else {
				error(4);	/* procedure后应为标识符 */
			}

			if (sym == lparen) getsym();
			else error(19);
			if (sym == rparen) getsym();
			else error(19);
			if (sym == lbrace) getsym();
			else error(20);

			block(lev + 1, tx, nxtlev); /* 递归调用 */

			if (sym == rbrace) {
				getsym();
			}
			else {
				error(5);	/* 漏掉了'}' */
			}
		}
	} while (inset(sym, declbegsys));	/* 直到没有声明符号 */

	code[table[tx0].adr].a = cx;	/* 把前面生成的跳转语句的跳转位置改成当前位置 */
	table[tx0].adr = cx;	        /* 记录当前过程代码地址 */
	table[tx0].size = dx;	        /* 声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size */
	cx0 = cx;
	gen(ini, 0, dx);	            /* 生成指令，此指令执行时在数据栈中为被调用的过程开辟dx个单元的数据区 */

	if (tableswitch)		/* 输出符号表 */
	{
		for (i = 1; i <= tx; i++)
		{
			switch (table[i].kind)
			{
			case constant:
				fprintf(ftable, "    %d const %s ", i, table[i].name);
				fprintf(ftable, "val=%d\n", table[i].val);
				break;
			case variable:
				fprintf(ftable, "    %d var   %s ", i, table[i].name);
				fprintf(ftable, "lev=%d addr=%d\n", table[i].level, table[i].adr);
				break;
			case procedure:
				fprintf(ftable, "    %d proc  %s ", i, table[i].name);
				fprintf(ftable, "lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				break;
			case integer:
				fprintf(ftable, "    %d int %s ", i, table[i].name);
				fprintf(ftable, "lev=%d addr=%d\n", table[i].level, table[i].adr);
				break;
			case character:
				fprintf(ftable, "    %d char %s ", i, table[i].name);
				fprintf(ftable, "lev=%d addr=%d\n", table[i].level, table[i].adr);
				break;
			}
		}
		fprintf(ftable, "\n");
	}
	isDo = 0;
	statement_list(nxtlev, &tx, lev);
	gen(opr, 0, 0);								/* 每个过程出口都要使用的释放数据段指令 */
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
	case constant:	/* 常量 */
		if (num > amax) {
			error(31);	/* 常数越界 */
			num = 0;
		}
		table[(*ptx)].val = num; /* 登记常数的值 */
		break;
	case procedure:	/* 过程 */
		table[(*ptx)].level = lev;
		break;
	case integer:
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case character:
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case boolean:
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	}
}

/*数组除了第一个别的没名字，就不会被找到*/
void listenter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, ""); /* 符号表的name域记录标识符的名字是空 */
	table[(*ptx)].kind = k;
	switch (k)
	{
	case character:						/* 字符常量*/
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case integer:						/* 整型变量*/
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
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
	while (strcmp(table[i].name, id) != 0) {
		i--;
	}
	if (table[i].kind == character) {
		isBool = 0;
		isChar = 1;
	}
	else if (table[i].kind == boolean) {
		isBool = 1;
		isChar = 0;
	}
	else {
		isBool = 0;
		isChar = 0;
	}
	return i;
}

/*
 * 常量声明处理
 */
void constdeclaration(int* ptx, int lev, int* pdx) {
	if (sym == ident) {
		getsym();
		if (sym == eql || sym == becomes) {
			if (sym == eql) {
				error(1);	/* 把=写成了== */
			}
			getsym();
			if (sym == number) {
				enter(constant, ptx, lev, pdx);
				getsym();
			}
			else {
				error(2);	/* 常量声明中的=后应是数字 */
			}
		}
		else {
			error(3);	/* 常量声明中的标识符后应是= */
		}
	}
	else {
		error(4);	/* const后应是标识符 */
	}
}

/*
 * 变量声明处理
 */
void intdeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident) {
		enter(integer, ptx, lev, pdx);	// 填写符号表
		getsym();
	}
	else {
		error(4);	/* var后面应是标识符 */
	}
}

void chardeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident) {
		enter(character, ptx, lev, pdx);	// 填写符号表
		getsym();
	}
	else {
		error(4);	/* var后面应是标识符 */
	}
}

void booldeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident) {
		enter(boolean, ptx, lev, pdx);	// 填写符号表
		getsym();
	}
	else {
		error(4);	/* var后面应是标识符 */
	}
}

/*
* int数组声明处理
*/
void intlistdeclaration(int * ptx, int lev, int * pdx, int length) {
	if (sym == number || sym == ident) {
		int i = 0;
		for (i = 0; i < length - 1; ++i) {
			listenter(integer, ptx, lev, pdx);		/* 填写数组符号表*/
		}
		getsym();
	}
	else {
		error(6);									/* 数组方括号内应该是number*/
	}
}

/*
* char数组声明处理
*/
void charlistdeclaration(int * ptx, int lev, int * pdx, int length) {
	if (sym == number) {
		int i = 0;
		for (i = 0; i < length - 1; ++i) {
			listenter(character, ptx, lev, pdx);	/* 填写数组符号表*/
		}
		getsym();
	}
	else {
		error(6);									/* 数组方括号内应该是number*/
	}
}

/*
 * 输出所有目标代码
 */
void listall()
{
	int i;
	if (listswitch) {
		for (i = 0; i < cx; i++) {
			fprintf(fcode, "%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}


void statement_list(bool *fsys, int *ptx, int lev) {
	bool nxtlev[symnum];
	while (inset(sym, statbegsys)) {
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		statement(nxtlev, ptx, lev);
	}
}

/*
 * 语句处理
 */
void statement(bool* fsys, int* ptx, int lev)
{
	int i, j, cx1, cx2, oldNum, isArray, shift;
	bool nxtlev[symnum];
	if (sym == readsym) {
		getsym();

		if (sym == ident) {
			isArray = 0;
			shift = -1;
			i = position(id, *ptx);
			if (i == 0) {
				error(6);
			}
			else {
				if (table[i].kind != integer && table[i].kind != character) {
					error(7);
					i = 0;
				}
				else {
					getsym();
					if (sym == lrange) {
						isArray = 1;
						getsym();
						if (sym == number) {
							shift = num;
							getsym();
							if (shift >= oneSize[i]) {
								error(41);
							}
						}
						else if (sym == constant) {
							j = position(id, *ptx);
							if (j == 0) error(6);
							if (table[j].kind != constant) {
								error(7);
								j = 0;
							}
							shift = table[j].val;
							getsym();
							if (shift >= oneSize[i]) {
								error(41);
							}
						}
						else if (sym == ident) {
							j = position(id, *ptx);
							if (j == 0) error(6);
							if (table[j].kind != integer && table[j].kind != character) {
								error(7);
								j = 0;
							}
							getsym();
						}

						if (sym == rrange) {
							getsym();
						}
						else {
							error(36);
						}
					}
				}
			}
		}
		if (sym == semicolon) {
			getsym();
			if (i != 0) {
				gen(opr, 0, 16);
				if (isArray) {
					if (shift != 0) {
						gen(sto, lev - table[i].level, table[i].adr + shift);
					}
					else {
						gen(lod, lev - table[j].level, table[j].adr);
						gen(stv, lev - table[i].level, table[i].adr);
					}
					isArray = 0;
				}
				else {
					gen(sto, lev - table[i].level, table[i].adr);	/* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
				}
			}
		}
		else {
			error(16);
		}
	}
	else {
		if (sym == writesym) {
			getsym();

			isWrite = 1;
			isBool = 0;
			isChar = 0;
			expression(nxtlev, ptx, lev);
			isWrite = 0;

			if (isChar) {
				gen(opr, 0, 17);
				isChar = 0;
			}
			else if (isBool) {
				gen(opr, 0, 23);
				isBool = 0;
			}
			else {
				gen(opr, 0, 14);
			}

			gen(opr, 0, 15);
			if (sym != semicolon) {
				error(16);
			}
			else {
				getsym();
			}
		}
		else {
			if (sym == callsym) {			/* 准备按照call语句处理 */
				getsym();
				if (sym != ident) {
					error(0);				/* call后应为标识符 */
				}
				else {
					i = position(id, *ptx);
					if (i == 0) {
						error(6);			/* 过程名未找到 */
					}
					else {
						if (table[i].kind == procedure) {
							gen(cal, lev - table[i].level, table[i].adr);	/* 生成call指令 */
						}
						else {
							error(31);		/* call后标识符类型应为过程 */
						}
					}
					getsym();
					if (sym != semicolon) {
						error(16);
					}
					else {
						getsym();
					}
				}
			}
			else {
				if (sym == ifsym) {	/* 准备按照if语句处理 */
					getsym();
					if (sym == lparen) {
						getsym();
					}
					else {
						error(17);
					}
					expression(nxtlev, ptx, lev); /* 调用条件处理 */
					if (sym == rparen) {
						getsym();
					}
					else {
						error(16);
					}
					cx1 = cx;	/* 保存当前指令地址 */
					gen(jpc, 0, 0);	/* 生成条件跳转指令，跳转地址未知，暂时写0 */
					statement(fsys, ptx, lev);	/* 处理then后的语句 */

					cx2 = cx;
					gen(jmp, 0, 0);
					code[cx1].a = cx;	/* 经statement处理后，cx为then后语句执行完的位置，它正是前面未定的跳转地址，此时进行回填 */
					if (sym == elsesym) {
						getsym();
						statement(nxtlev, ptx, lev);
					}
					code[cx2].a = cx;
				}
				else {
					if (sym == whilesym) {				/* 准备按照while语句处理 */
						if (isDo) {
							/* do statement_list while(expression) */
							getsym();
							if (sym != lparen) error(16);
							else getsym();
							expression(nxtlev, ptx, lev);
							if (sym != rparen) error(17);
							else getsym();
							gen(jeq, 0, doCX);
							isDo = 0;
						}
						else {
							/* while(expression) statement_list */
							cx1 = cx;						/* 保存判断条件操作的位置 */
							getsym();
							if (sym == lparen) {
								getsym();
							}
							else {
								error(17);					/* 缺少(*/
							}
							expression(nxtlev, ptx, lev);	/* 调用条件处理 */
							cx2 = cx;						/* 保存循环体的结束的下一个位置 */
							gen(jpc, 0, 0);					/* 生成条件跳转，但跳出循环的地址未知，标记为0等待回填 */
							if (sym == rparen) {
								getsym();
							}
							else {
								error(16);	/* 缺少)*/
							}
							statement(fsys, ptx, lev);	/* 循环体 */
							gen(jmp, 0, cx1);	/* 生成条件跳转指令，跳转到前面判断条件操作的位置 */
							code[cx2].a = cx;	/* 回填跳出循环的地址 */
						}
					}
					else if (sym == forsym) {
						getsym();
						if (sym == lparen) {
							getsym();
							expression(nxtlev, ptx, lev);
							if (sym == semicolon) getsym();
							else error(22);

							int cx1 = cx;
							expression(nxtlev, ptx, lev);
							int cx2 = cx;
							gen(jpc, 0, 0);
							int cx3 = cx;
							gen(jmp, 0, 0);
							if (sym == semicolon) getsym();
							else error(22);

							int cx4 = cx;
							expression(nxtlev, ptx, lev);
							gen(jmp, 0, cx1);
							if (sym == rparen) getsym();
							else error(17);

							code[cx3].a = cx;
							statement(nxtlev, ptx, lev);
							gen(jmp, 0, cx4);

							code[cx2].a = cx;
						}
						else {
							error(16);
						}
					}
					else if (sym == lbrace) {
						getsym();
						statement_list(fsys, ptx, lev);
						if (sym == rbrace) {
							getsym();
						}
						else {
							error(21);
						}
					}
					else if (sym == repeatsym) {
						getsym();
						int cx1 = cx;
						statement_list(nxtlev, ptx, lev);
						if (sym != untilsym) error(38);
						else getsym();
						if (sym != lparen) error(16);
						else getsym();
						expression(nxtlev, ptx, lev);
						if (sym != rparen) error(17);
						else getsym();
						gen(jeq, 0, cx1);
					}
					else if (sym == dosym) {
						isDo = 1;
						getsym();
						doCX = cx;
						statement_list(nxtlev, ptx, lev);
					}
					else if (sym == exitsym) {
						getsym();
						if (sym == lparen) getsym();
						else error(16);
						if (sym == rparen) getsym();
						else error(17);
						if (sym == semicolon) getsym();
						else error(16);
					}
					else if (sym == switchsym) {
						int breakList[100];
						int currBreak = 0;
						getsym();
						if (sym == lparen)	getsym();
						else error(16);
						expression(nxtlev, ptx, lev);
						if (sym == rparen) getsym();
						else error(17);

						if (sym == lbrace) {
							getsym();
							int cx1 = -1, cx2 = -1;
							while (sym == casesym) {
								if (cx1 != -1) {
									code[cx1].a = cx;
								}
								getsym();
								if (sym == number) {
									gen(lod, lev - table[oldI].level, table[oldI].adr);
									gen(lit, 0, num);
									gen(opr, 0, 8);
									cx1 = cx;
									gen(jpc, 0, 0);
									getsym();

									if (sym == colonsym) {
										getsym();
										if (cx2 != -1) {
											code[cx2].a = cx;
										}
										statement_list(nxtlev, ptx, lev);
										if (sym == breaksym) {
											getsym();
											breakList[currBreak++] = cx;
											if (sym == semicolon) {
												getsym();
											}
											else {
												error(21);
											}
										}
										else {
											error(42);
										}
										cx2 = cx;
										gen(jmp, 0, 0);
									}
									else {
										error(21);
									}
								}
							}
							if (cx != -1) {
								code[cx1].a = cx;
							}
							if (cx2 != -1) {
								code[cx2].a = cx;
							}
							if (sym == defaultsym) {
								getsym();
								if (sym == colonsym) {
									getsym();
									statement_list(nxtlev, ptx, lev);
									if (sym == breaksym) {
										getsym();
										breakList[currBreak++] = cx;
										if (sym == semicolon) {
											getsym();
										}
										else {
											error(21);
										}
									}
									else {
										error(42);
									}
									gen(jmp, 0, 0);
								}
								else {
									error(1);
								}
							}
						}
						else {
							error(20);
						}
						if (sym == rbrace) getsym();
						else error(21);

						for (int k = 0; k < currBreak; k++) {
							int pos = breakList[k];
							code[pos].a = cx;
						}
					}
					else {
						expression_stat(nxtlev, ptx, lev);
					}
				}
			}
		}
	}

}

/* 递归表达式处理*/
void expression_stat(bool* fsys, int* ptx, int lev)
{
	bool nxtlev[symnum];
	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	if (sym == semicolon) {
		getsym();
	}
	else {
		expression(nxtlev, ptx, lev);
		if (isWrite) {
			isWrite = 0;
		}
		else {
			if (sym != semicolon) {
				error(16);
			}
			else {
				getsym();
			}
		}
	}
}

/* 表达式处理*/
void expression(bool *fsys, int *ptx, int lev) {
	int i, j, isArray, shift;
	enum symbol oldSym;
	int oldPosition;
	bool nxtlev[symnum];
	while (inset(sym, facbegsys)) {
		if (sym == ident) {
			isArray = 0;
			conf_array = 0;
			shift = -1;
			conf_shift = -1;
			i = position(id, *ptx);
			strcpy(oldId, id);
			oldI = i;
			conf_i = i;
			isChar = 0;
			isBool = 0;
			if (i == 0) {
				error(6);
			}
			else {
				if (table[i].kind != integer && table[i].kind != character && table[i].kind != boolean) {
					error(7);
					i = 0;
				}
				else {
					getsym();
					if (table[i].kind == character) {
						isChar = 1;
					}
					else if (table[i].kind == boolean) {
						isBool = 1;
					}
					if (sym == lrange) {
						isArray = 1;
						conf_array = 1;
						getsym();
						if (sym == number) {
							shift = num;
							conf_shift = num;
							if (shift >= oneSize[i]) error(41);
							getsym();
						}
						else if (sym == constant) {
							j = position(id, *ptx);
							if (j == 0) error(6);
							if (table[j].kind != constant) {
								error(7);
								j = 0;
							}
							shift = table[j].val;
							conf_shift = table[j].val;
							if (shift >= oneSize[i]) error(41);
							getsym();
						}
						else if (sym == ident) {
							j = position(id, *ptx);
							conf_j = j;
							if (j == 0) error(6);
							if (table[j].kind != integer && table[j].kind != character) {
								error(7);
								j = 0;
							}
							getsym();
						}
						if (sym == rrange) {
							getsym();
						}
						else {
							error(36);
						}
					}
					if (sym == becomes) { /* = */
						getsym();
						simple_expr(nxtlev, ptx, lev);	/* 处理赋值符号右侧表达式 */
						if (i != 0) {
							if (isArray) {
								if (shift != -1) {
									gen(sto, lev - table[i].level, table[i].adr + shift);
								}
								else {
									gen(lod, lev - table[j].level, table[j].adr);
									gen(stv, lev - table[i].level, table[i].adr);
								}
								isArray = 0;
							}
							else {
								gen(sto, lev - table[i].level, table[i].adr);	/* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
							}
						}
					}
					else if (sym == semicolon) {
						if (i != 0) {
							if (isArray) {
								if (shift != -1) {
									gen(lod, lev - table[i].level, table[i].adr + shift);
								}
								else {
									gen(lod, lev - table[j].level, table[j].adr);
									gen(lov, lev - table[i].level, table[i].adr);
								}
								isArray = 0;
							}
							else {
								gen(lod, lev - table[i].level, table[i].adr);	/* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
							}
						}
					}
					else {
						/* conflict */
						conf_sym = sym;

						sym = ident;
						strcpy(id, oldId);
						simple_expr(nxtlev, ptx, lev);

						conf_sym = -1;
					}
				}
			}
		}
		else {
			simple_expr(nxtlev, ptx, lev);
		}
	}
}

void simple_expr(bool* fsys, int* ptx, int lev) {
	enum symbol relop;
	bool nxtlev[symnum];

	if (sym == oddsym)					/* 准备按照odd运算处理 */
	{
		getsym();
		additive_expr(fsys, ptx, lev);
		gen(opr, 0, 6);					/* 生成odd指令 */
	}
	else
	{
		/* 逻辑表达式处理 */
		additive_expr(nxtlev, ptx, lev);
		if (sym == semicolon || sym == rparen) {
			return;
		}
		if (sym == eql || sym == neq || sym == lss || sym == leq || sym == gtr || sym == geq || sym == mod || sym == xorsym || sym == andsym || sym == orsym || sym == notsym) {
			relop = sym;
			getsym();
			additive_expr(fsys, ptx, lev);
			switch (relop)
			{
			case eql:						/* == */
				gen(opr, 0, 8);
				break;
			case neq:						/* != */
				gen(opr, 0, 9);
				break;
			case lss:						/* < */
				gen(opr, 0, 10);
				break;
			case geq:						/* >= */
				gen(opr, 0, 11);
				break;
			case gtr:						/* > */
				gen(opr, 0, 12);
				break;
			case leq:						/* <= */
				gen(opr, 0, 13);
				break;
			case mod:						/* % */
				gen(opr, 0, 18);
				break;
			case xorsym:					/* ^ */
				gen(opr, 0, 19);
				break;
			case andsym:					/* & */
				gen(opr, 0, 20);
				break;
			case orsym:						/* | */
				gen(opr, 0, 21);
				break;
			case notsym:					/* ~ */
				gen(opr, 0, 22);
				break;
			}
		}
		else {
			error(20); /* 应该为关系运算符 */
		}
	}
}
/*
 * 表达式处理
 */
void additive_expr(bool* fsys, int* ptx, int lev)
{
	enum symbol addop;	/* 用于保存正负号 */
	bool nxtlev[symnum];

	if (sym == plus || sym == minus)	/* 表达式开头有正负号，此时当前表达式被看作一个正的或负的项 */
	{
		addop = sym;	/* 保存开头的正负号 */
		getsym();
		term(nxtlev, ptx, lev);	/* 处理项 */
		if (addop == minus)
		{
			gen(opr, 0, 1);	/* 如果开头为负号生成取负指令 */
		}
	}
	else	/* 此时表达式被看作项的加减 */
	{
		term(nxtlev, ptx, lev);	/* 处理项 */
	}
	while (sym == plus || sym == minus) {
		addop = sym;
		getsym();
		term(nxtlev, ptx, lev);	/* 处理项 */
		if (addop == plus) {
			gen(opr, 0, 2);	/* 生成加法指令 */
		}
		else {
			gen(opr, 0, 3);	/* 生成减法指令 */
		}
	}
}

/*
 * 项处理
 */
void term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;	/* 用于保存乘除法符号 */
	bool nxtlev[symnum];
	factor(nxtlev, ptx, lev);	/* 处理因子 */
	while (sym == times || sym == slash) {
		mulop = sym;
		getsym();
		factor(nxtlev, ptx, lev);
		if (mulop == times) {
			gen(opr, 0, 4);	/* 生成乘法指令 */
		}
		else if (mulop == xorsym) {
			gen(opr, 0, 19);
		}
		else if (mulop == slash) {
			gen(opr, 0, 5);	/* 生成除法指令 */
		}
		else {
			gen(opr, 0, 18);
		}
	}
}

/*
 * 因子处理
 */
void factor(bool* fsys, int* ptx, int lev)
{
	int i, j, shift, isArray;
	enum symbol minusOrPlus = -1;
	enum symbol isNot;
	bool nxtlev[symnum];
	while (inset(sym, facbegsys)) 	/* 循环处理因子 */
	{
		if (conf_sym != -1) {
			sym = conf_sym;

			if (conf_array) {
				if (conf_shift != -1) {
					gen(lod, lev - table[conf_i].level, table[conf_i].adr + conf_shift);
				}
				else {
					gen(lod, lev - table[conf_j].level, table[conf_j].adr);
					gen(lov, lev - table[conf_i].level, table[conf_i].adr);
				}
			}
			else {
				gen(lod, lev - table[conf_i].level, table[conf_i].adr);
			}
			if (table[conf_i].kind == integer || table[conf_i].kind == character) {
				if (sym == selfminus || sym == selfplus) {
					gen(lit, 0, 1);
					if (sym == selfminus) gen(opr, 0, 3);
					else gen(opr, 0, 2);
					if (conf_array) {
						if (conf_shift != -1) {
							gen(sto, lev - table[conf_i].level, table[conf_i].adr + conf_shift);
						}
						else {
							gen(lod, lev - table[conf_j].level, table[conf_j].adr);
							gen(stv, lev - table[conf_i].level, table[conf_i].adr);
						}
					}
					else {
						gen(sto, lev - table[conf_i].level, table[conf_i].adr);
					}
					getsym();
				}
			}
			conf_sym = -1;
			conf_i = 0;
			conf_j = 0;
			conf_shift = -1;
			return;
		}
		if (sym == selfminus || sym == selfplus) {
			minusOrPlus = sym;
			getsym();
		}
		if (sym == notsym) {
			isNot = sym;
			getsym();
			if (sym == ident) {
				i = position(id, *ptx);
				if (i == 0) {
					error(11);
				}
				if (table[i].kind != boolean) {
					error(44);
				}
				gen(lod, lev - table[i].level, table[i].adr);
				gen(opr, 0, 22);
				getsym();
			}
		}
		if (sym == ident)	/* 因子为常量或变量 */
		{
			isArray = 0;
			shift = -1;
			if (strcmp(id, "true") == 0 || strcmp(id, "True") == 0) {
				gen(lit, 0, 1);
				getsym();
				return;
			}
			else if (strcmp(id, "false") == 0 || strcmp(id, "False") == 0) {
				gen(lit, 0, 0);
				getsym();
				return;
			}
			else {

			}
			i = position(id, *ptx);	/* 查找标识符在符号表中的位置 */
			if (i == 0) {
				error(11);	/* 标识符未声明 */
			}
			else {
				getsym();
				if (sym == lrange) {
					isArray = 1;
					getsym();
					if (sym == number) {
						shift = num;
						if (shift >= oneSize[i]) {
							error(41);
						}
					}
					else if (sym == ident) {
						j = position(id, *ptx);
						if (j == 0) error(6);
						if (table[j].kind != constant) {
							error(7);
							j = 0;
						}
					}
					else if (sym == constant) {
						j = position(id, *ptx);
						if (j == 0) error(6);
						if (table[j].kind != constant) {
							error(7);
							j = 0;
						}
						shift = table[j].val;
						if (shift >= oneSize[i]) {
							error(41);
						}
					}
					getsym();
					if (sym == rrange) {
						getsym();
					}
					else {
						error(36);
					}
				}
				if (table[i].kind == integer || table[i].kind == character || table[i].kind == boolean) {
					if (isArray) {
						if (shift > -1) {
							gen(lod, lev - table[i].level, table[i].adr + shift);
						}
						else {
							gen(lod, lev - table[j].level, table[j].adr);
							gen(lov, lev - table[i].level, table[i].adr);
						}
					}
					else {
						gen(lod, lev - table[i].level, table[i].adr);
					}
				}
				else if (table[i].kind == constant) {
					gen(lit, 0, table[i].val);
				}
				else {

				}
			}
			if (minusOrPlus == selfminus || minusOrPlus == selfplus) {
				if (table[i].kind != character && table[i].kind != integer) {
					error(45);
				}
				gen(lit, 0, 1);
				if (minusOrPlus == selfminus) {
					gen(opr, 0, 3);
				}
				else {
					gen(opr, 0, 2);
				}
				if (isArray) {
					if (shift > -1) {
						gen(sto, lev - table[i].level, table[i].adr + shift);
					}
					else {
						gen(lod, lev - table[j].level, table[j].adr);
						gen(stv, lev - table[i].level, table[i].adr);
					}
				}
				else {
					gen(sto, lev - table[i].level, table[i].adr);
				}
			}
		}
		else
		{
			if (sym == number)	/* 因子为数 */
			{
				if (num > amax)
				{
					error(31); /* 数越界 */
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
					expression(nxtlev, ptx, lev);
					if (sym == rparen)
					{
						getsym();
					}
					else
					{
						error(22);	/* 缺少右括号 */
					}
				}
			}
		}
	}
}

/*
 * 解释程序
 */
int s[stacksize];	/* 栈 */
void interpret()
{
	int p = 0; /* 指令指针 */
	int b = 1; /* 指令基址 */
	int t = 0; /* 栈顶指针 */
	struct instruction i;	/* 存放当前指令 */


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
				//printf("%d %d\n", s[t], s[t + 1]);
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
				printf("\nInput:");
				fprintf(fresult, "\nInput:");
				scanf("%d", &(s[t]));
				fprintf(fresult, "%d\n", s[t]);
				break;
			case 17:/* 栈顶值char类型输出 */
				printf("%c", s[t]);
				fprintf(fresult, "%c", s[t]);
				t = t - 1;
				break;
			case 18:/* 求余 */
				t = t - 1;
				s[t] = s[t] % s[t + 1];
				break;
			case 19:/* 异或 */
				t = t - 1;
				s[t] = s[t] ^ s[t + 1];
				break;
			case 20:/* & */
				t = t - 1;
				s[t] = s[t] & s[t - 1];
				break;
			case 21:/* | */
				t = t - 1;
				s[t] = s[t] | s[t - 1];
				break;
			case 22:/* not */
				s[t] = !s[t];
				break;
			case 23:/* 栈顶值bool类型输出 */
				if (s[t] == 1) {
					printf("true");
					fprintf(fresult, "true");
				}
				else {
					printf("false");
					fprintf(fresult, "false");
				}
				break;
			}
			break;
		case lod:	/* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
			t = t + 1;
			s[t] = s[base(i.l, s, b) + i.a];
			break;
		case sto:	/* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
			s[base(i.l, s, b) + i.a] = s[t];
			t = t - 1;
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
		case jeq:
			if (s[t] != 0)
				p = i.a;
			t = t - 1;
			break;
		case stv:
			s[base(i.l, s, b) + i.a + s[t]] = s[t - 1];
			t = t - 2;
			break;
		case lov:
			s[t] = s[base(i.l, s, b) + i.a + s[t]];
			break;
		}
	} while (p != 0);
	printf("\nEnd X0\n");
	fprintf(fresult, "\nExecute Successfully\n");
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