// compiler.c: ��Ԥ�����ͷ��Ӧ��Դ�ļ�������ɹ��������

#include "compiler.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// һ������£����Դ��ļ����������ʹ�õ���Ԥ�����ͷ���뱣������

/*
 * ������ʵ�ּ��ϵļ�������
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
 *	��������ӡ����λ�úʹ������
 */
void error(int n) {
	char space[81];
	memset(space, 32, 81);

	space[cc - 1] = 0; /* ����ʱ��ǰ�����Ѿ����꣬����cc-1 */


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
 * ���˿ո񣬶�ȡһ���ַ�
 * ÿ�ζ�һ�У�����line��������line��getsymȡ�պ��ٶ�һ��
 * ������getsym����
 */
void getch() {
	if (cc == ll) /* �жϻ��������Ƿ����ַ��������ַ����������һ���ַ����������� */
	{
		if (feof(fin)) {
			sym = period;
			printf("Program is incomplete!\n");
			exit(1);
		}
		ll = 0;
		cc = 0;

		fprintf(foutput, "%d ", cx);
		ch = ' ';
		while (ch != 10) {
			if (EOF == fscanf(fin, "%c", &ch)) {
				line[ll] = 0;
				break;
			}
			fprintf(foutput, "%c", ch);
			line[ll] = ch;
			ll++;
		}
	}
	ch = line[cc];
	cc++;
}

/*
 * �ʷ���������ȡһ������
 */
void getsym()
{
	int i, j, k;

	while (ch == ' ' || ch == 10 || ch == 9)	/* ���˿ո񡢻��к��Ʊ�� */
	{
		getch();
	}
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) /* ��ǰ�ĵ����Ǳ�ʶ�����Ǳ����� */
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
		do {    /* ������ǰ�����Ƿ�Ϊ�����֣�ʹ�ö��ַ����� */
			k = (i + j) / 2;
			if (strcmp(id, word[k]) <= 0) {
				j = k - 1;
			}
			if (strcmp(id, word[k]) >= 0) {
				i = k + 1;
			}
		} while (i <= j);
		if (i - 1 > j)	/* ��ǰ�ĵ����Ǳ����� */
		{
			sym = wsym[k];
		}
		else			/* ��ǰ�ĵ����Ǳ�ʶ�� */
		{
			sym = ident;
		}
	}
	else
	{
		if (ch >= '0' && ch <= '9') /* ��ǰ�ĵ��������� */
		{
			k = 0;
			num = 0;
			sym = number;
			do {
				num = 10 * num + ch - '0';
				k++;
				getch();;
			} while (ch >= '0' && ch <= '9'); /* ��ȡ���ֵ�ֵ */
			k--;
			if (k > nmax) /* ����λ��̫�� */
			{
				error(14);
			}
		}
		else
		{
			if (ch == '=') {		/* ��⸳ֵ���� */
				getch();
				if (ch == '=') {
					sym = eql;
					getch();
				}
				else {
					sym = becomes;	/* = */
				}
			}
			else if (ch == '<')		/* ���С�ڻ�С�ڵ��ڷ��� */
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
			else if (ch == '>')		/* �����ڻ���ڵ��ڷ��� */
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
				if (ch == '/') {			 /* ��ע��*/
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
				sym = ssym[ch];		/* ���յ��ַ����Ŵ��� */
				if (sym != rbrace) {
					getch();
				}
			}
		}
	}
}

/*
 * �������������
 *
 * x: instruction.f;
 * y: instruction.l;
 * z: instruction.a;
 */
void gen(enum fct x, int y, int z) {
	if (cx >= cxmax) {
		printf("���ɵ����������������!\n");	/* ���ɵ���������������� */
		exit(1);
	}
	if (z >= amax) {
		printf("��ַƫ��Խ��!\n");	/* ��ַƫ��Խ�� */
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
}


/*
 * ���Ե�ǰ�����Ƿ�Ϸ�
 *
 * ���﷨�����������ںͳ��ڴ����ò��Ժ���test��
 * ��鵱ǰ���ʽ�����˳����﷨��λ�ĺϷ���
 *
 * s1:	��Ҫ�ĵ��ʼ���
 * s2:	���������Ҫ�ĵ��ʣ���ĳһ����״̬ʱ��
 *      �ɻָ��﷨�����������������Ĳ��䵥�ʷ��ż���
 * n:  	�����
 */
void test(bool* s1, bool* s2, int n)
{
	printf("%d %d\n", sym, s1[sym]);
	if (!inset(sym, s1)) {
		error(n);
		/* ����ⲻͨ��ʱ����ͣ��ȡ���ţ�ֱ����������Ҫ�ļ��ϻ򲹾ȵļ��� */
		while ((!inset(sym, s1)) && (!inset(sym, s2))) {
			getsym();
		}
	}
}

/*
 * ��ʼ��
 */
void init()
{
	int i;

	/* ���õ��ַ����� */
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

	/* ���ñ���������,������ĸ˳�򣬱��ڶ��ֲ��� */
	strcpy(&(word[0][0]), "and");
	strcpy(&(word[1][0]), "bool");
	strcpy(&(word[2][0]), "break");
	strcpy(&(word[3][0]), "call");
	strcpy(&(word[4][0]), "case");
	strcpy(&(word[5][0]), "char");
	strcpy(&(word[6][0]), "const");
	strcpy(&(word[7][0]), "continue");
	strcpy(&(word[8][0]), "do");
	strcpy(&(word[9][0]), "else");
	strcpy(&(word[10][0]), "exit");
	strcpy(&(word[11][0]), "for");
	strcpy(&(word[12][0]), "func");
	strcpy(&(word[13][0]), "if");
	strcpy(&(word[14][0]), "int");
	strcpy(&(word[15][0]), "main");
	strcpy(&(word[16][0]), "odd");
	strcpy(&(word[17][0]), "or");
	strcpy(&(word[18][0]), "read");
	strcpy(&(word[19][0]), "repeat");
	strcpy(&(word[20][0]), "switch");
	strcpy(&(word[21][0]), "until");
	strcpy(&(word[22][0]), "while");
	strcpy(&(word[23][0]), "write");

	/* ���ñ����ַ��� */
	wsym[0] = andsym;
	wsym[1] = boolsym;
	wsym[2] = breaksym;
	wsym[3] = callsym;
	wsym[4] = casesym;
	wsym[5] = charsym;
	wsym[6] = constsym;
	wsym[7] = constinuesym;
	wsym[8] = dosym;
	wsym[9] = elsesym;
	wsym[10] = exitsym;
	wsym[11] = forsym;
	wsym[12] = funcsym;
	wsym[13] = ifsym;
	wsym[14] = intsym;
	wsym[15] = mainsym;
	wsym[16] = oddsym;
	wsym[17] = orsym;
	wsym[18] = readsym;
	wsym[19] = repeatsym;
	wsym[20] = switchsym;
	wsym[21] = untilsym;
	wsym[22] = whilesym;
	wsym[23] = writesym;

	/* ����ָ������ */
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[ini][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");
	strcpy(&(mnemonic[jeq][0]), "jeq");

	/* ���÷��ż� */
	memset(declbegsys, false, sizeof(declbegsys));
	memset(statbegsys, false, sizeof(statbegsys));
	memset(facbegsys, false, sizeof(facbegsys));


	/* ����������ʼ���ż� */
	declbegsys[constsym] = true;
	declbegsys[intsym] = true;
	declbegsys[charsym] = true;
	declbegsys[boolsym] = true;
	declbegsys[funcsym] = true;
	declbegsys[mainsym] = true;

	/* ������俪ʼ���ż� */
	statbegsys[callsym] = true;
	statbegsys[ifsym] = true;
	statbegsys[whilesym] = true;
	statbegsys[readsym] = true;
	statbegsys[forsym] = true;
	statbegsys[repeatsym] = true;
	statbegsys[writesym] = true;
	statbegsys[ident] = true;

	/* �������ӿ�ʼ���ż� */
	facbegsys[ident] = true;
	facbegsys[number] = true;
	facbegsys[lparen] = true;


	strcpy(errorInfo[0], "�������ź���ȱ�ٱ�ʶ��");
	strcpy(errorInfo[1], "ȱ��';'");
	strcpy(errorInfo[2], "�����ߺ���������ʼ���Ŵ���");
	strcpy(errorInfo[3], "ȱ����俪ʼ����");
	strcpy(errorInfo[4], "ȱ������������");
	strcpy(errorInfo[5], "����δ���");
	strcpy(errorInfo[6], "��ʶ��δ����");
	strcpy(errorInfo[7], "��ֵ���Ⱥ���಻�Ǳ���");
	strcpy(errorInfo[8], "ȱ�ٸ�ֵ����");
	strcpy(errorInfo[9], "[]��ȱ������");
	strcpy(errorInfo[10], "���������Ŵ���");
	strcpy(errorInfo[11], "ȱ�ٹ�ϵ�����");
	strcpy(errorInfo[12], "��ʶ�������Ǻ���");
	strcpy(errorInfo[13], "���ӿ�ʼ���Ŵ���");
	strcpy(errorInfo[14], "����������λ��̫�����������");
	strcpy(errorInfo[15], "���ִ�С������Χ");
	strcpy(errorInfo[16], "ȱ��')'");
	strcpy(errorInfo[17], "ȱ��'('");
	strcpy(errorInfo[18], "����δ����");
	strcpy(errorInfo[19], "function����ȱ��'()'");
	strcpy(errorInfo[20], "ȱ��'{'");
	strcpy(errorInfo[21], "ȱ��'}'");
	strcpy(errorInfo[22], "ȱ��'in'");
	strcpy(errorInfo[23], "forѭ��ȱ����߽�");
	strcpy(errorInfo[24], "forѭ��ȱ���ұ߽�");
	strcpy(errorInfo[25], "forѭ��ȱ��'...'");
	strcpy(errorInfo[30], "����Ƕ�ײ�������");
	strcpy(errorInfo[31], "call��������ȱ�ٹ�����");
	strcpy(errorInfo[32], "���������е�=��Ӧ������");
	strcpy(errorInfo[33], "cannot find the single symbol");
	strcpy(errorInfo[34], "����ȱ��[");
	strcpy(errorInfo[35], "��������ȱ�����������С");
	strcpy(errorInfo[36], "����ȱ��]");
	strcpy(errorInfo[37], "loss the symbol main");

	err = 0;
	cc = ll = cx = 0;
	ch = ' ';
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
			nxtlev[semicolon] = true;
			nxtlev[rbrace] = true;
			block(0, 0, nxtlev);	/* ����ֳ��� */
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


	listall();			/* ������д��� */
	interpret();		/* ���ý���ִ�г��� */

	fclose(fcode);
	fclose(fresult);
	fclose(ftable);
	fclose(foutput);
	fclose(fin);
}

/*
 * �����������
 *
 * lev:    ��ǰ�ֳ������ڲ�
 * tx:     ���ű�ǰβָ��
 * fsys:   ��ǰģ���̷��ż���
 */
void block(int lev, int tx, bool* fsys)
{
	int i;

	int dx;                 /* ��¼���ݷ������Ե�ַ */
	int tx0;                /* ������ʼtx */
	int cx0;                /* ������ʼcx */
	bool nxtlev[symnum];    /* ���¼������Ĳ����У����ż��Ͼ�Ϊֵ�Σ�������ʹ������ʵ�֣�
							   ���ݽ�������ָ�룬Ϊ��ֹ�¼������ı��ϼ������ļ��ϣ������µĿռ�
							   ���ݸ��¼�����*/

	dx = 3;                 /* �����ռ����ڴ�ž�̬��SL����̬��DL�ͷ��ص�ַRA  */
	tx0 = tx;		        /* ��¼�����ʶ���ĳ�ʼλ�� */
	table[tx].adr = cx;	    /* ��¼��ǰ�����Ŀ�ʼλ�� */
	gen(jmp, 0, 0);         /* ������תָ���תλ��δ֪��ʱ��0 */

	if (lev > levmax)		/* Ƕ�ײ������� */
	{
		error(32);
	}

	if (sym == nul) {
		sym = period;
	}

	do {

		while (sym == constsym) {
			getsym();
			constdeclaration(&tx, lev, &dx);
			if (sym == semicolon) {
				getsym();
			}
			else {
				error(9); /* ©���˷ֺ� */
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
				intlistdeclaration(&tx, lev, &dx, num);
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
					error(4);			/* ȱ��]*/
				}
			}
			else {
				error(9);				/* ©���˷ֺ�*/
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
				charlistdeclaration(&tx, lev, &dx, num);
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
					error(4);			/* ȱ��]*/
				}
			}
			else {
				error(1);				/* ©���˷ֺ� */
			}
		}

		while (sym == funcsym) {
			getsym();

			if (sym == ident) {
				enter(procedure, &tx, lev, &dx);	/* ��д���ű� */
				getsym();
			}
			else {
				error(4);	/* procedure��ӦΪ��ʶ�� */
			}

			if (sym == lparen) getsym();
			else error(19);
			if (sym == rparen) getsym();
			else error(19);
			if (sym == lbrace) getsym();
			else error(20);

			memcpy(nxtlev, fsys, sizeof(bool) * symnum);
			block(lev + 1, tx, nxtlev); /* �ݹ���� */

			if (sym == rbrace) {
				getsym();
				//memcpy(nxtlev, statbegsys, sizeof(bool) * symnum);
				//nxtlev[funcsym] = true;
				//test(nxtlev, fsys, 6);
			}
			else {
				error(5);	/* ©����'}' */
			}
		}
	} while (inset(sym, declbegsys));	/* ֱ��û���������� */

	code[table[tx0].adr].a = cx;	/* ��ǰ�����ɵ���ת������תλ�øĳɵ�ǰλ�� */
	table[tx0].adr = cx;	        /* ��¼��ǰ���̴����ַ */
	table[tx0].size = dx;	        /* ����������ÿ����һ�����������dx����1�����������Ѿ�������dx���ǵ�ǰ�������ݵ�size */
	cx0 = cx;
	gen(ini, 0, dx);	            /* ����ָ���ָ��ִ��ʱ������ջ��Ϊ�����õĹ��̿���dx����Ԫ�������� */

	if (tableswitch)		/* ������ű� */
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
				fprintf(ftable, "val=%d\n", table[i].val);
				break;
			case character:
				fprintf(ftable, "    %d char %s ", i, table[i].name);
				fprintf(ftable, "lev=%d addr=%d\n", table[i].level, table[i].adr);
				break;
			}
		}
		fprintf(ftable, "\n");
	}


	statement(nxtlev, &tx, lev);
	gen(opr, 0, 0);								/* ÿ�����̳��ڶ�Ҫʹ�õ��ͷ����ݶ�ָ�� */
}

/*
 * �ڷ��ű��м���һ��
 *
 * k:      ��ʶ��������Ϊconst��var��procedure
 * ptx:    ���ű�βָ���ָ�룬Ϊ�˿��Ըı���ű�βָ���ֵ
 * lev:    ��ʶ�����ڵĲ��
 * pdx:    dxΪ��ǰӦ����ı�������Ե�ַ�������Ҫ����1
 *
 */
void enter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, id); /* ���ű��name���¼��ʶ�������� */
	table[(*ptx)].kind = k;
	switch (k)
	{
	case constant:	/* ���� */
		if (num > amax) {
			error(31);	/* ����Խ�� */
			num = 0;
		}
		table[(*ptx)].val = num; /* �Ǽǳ�����ֵ */
		break;
	case procedure:	/* ���� */
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
	}
}

/*������˵�һ�����û���֣��Ͳ��ᱻ�ҵ�*/
void listenter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, ""); /* ���ű��name���¼��ʶ���������ǿ� */
	table[(*ptx)].kind = k;
	switch (k)
	{
	case character:						/* �ַ�����*/
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case integer:						/* ���ͱ���*/
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	}
}

/*
 * ���ұ�ʶ���ڷ��ű��е�λ�ã���tx��ʼ������ұ�ʶ��
 * �ҵ��򷵻��ڷ��ű��е�λ�ã����򷵻�0
 *
 * id:    Ҫ���ҵ�����
 * tx:    ��ǰ���ű�βָ��
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
		isChar = 1;
	}
	else {
		isChar = 0;
	}
	return i;
}

/*
 * ������������
 */
void constdeclaration(int* ptx, int lev, int* pdx) {
	if (sym == ident) {
		getsym();
		if (sym == eql || sym == becomes) {
			if (sym == eql) {
				error(1);	/* ��=д����== */
			}
			getsym();
			if (sym == number) {
				enter(constant, ptx, lev, pdx);
				getsym();
			}
			else {
				error(2);	/* ���������е�=��Ӧ������ */
			}
		}
		else {
			error(3);	/* ���������еı�ʶ����Ӧ��= */
		}
	}
	else {
		error(4);	/* const��Ӧ�Ǳ�ʶ�� */
	}
}

/*
 * ������������
 */
void intdeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident) {
		enter(integer, ptx, lev, pdx);	// ��д���ű�
		getsym();
	}
	else {
		error(4);	/* var����Ӧ�Ǳ�ʶ�� */
	}
}

void chardeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident) {
		enter(character, ptx, lev, pdx);	// ��д���ű�
		getsym();
	}
	else {
		error(4);	/* var����Ӧ�Ǳ�ʶ�� */
	}
}

/*
* int������������
*/
void intlistdeclaration(int * ptx, int lev, int * pdx, int length) {
	if (sym == number) {
		int i = 0;
		for (i = 0; i < length - 1; ++i) {
			listenter(integer, ptx, lev, pdx);		/* ��д������ű�*/
		}
		getsym();
	}
	else {
		error(6);									/* ���鷽������Ӧ����number*/
	}
}

/*
* char������������
*/
void charlistdeclaration(int * ptx, int lev, int * pdx, int length) {
	if (sym == number) {
		int i = 0;
		for (i = 0; i < length - 1; ++i) {
			listenter(character, ptx, lev, pdx);	/* ��д������ű�*/
		}
		getsym();
	}
	else {
		error(6);									/* ���鷽������Ӧ����number*/
	}
}

/*
 * �������Ŀ�����
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
 * ��䴦��
 */
void statement(bool* fsys, int* ptx, int lev)
{
	int i, cx1, cx2;
	bool nxtlev[symnum];
	//if (sym == ident) {
	//	i = position(id, *ptx);
	//	if (i == 0) {
	//		error(6);
	//	} 
	//	else {
	//		if (table[i].kind != integer && table[i].kind!=character) {
	//			error(7);
	//			i = 0;
	//		}
	//		else {
	//			getsym();
	//			if (sym == plus) {
	//				gen(lod, lev - table[i].level, table[i].adr);
	//				gen(lit, 0, 1);
	//				gen(opr, 0, 2);
	//				gen(sto, lev - table[i].level, table[i].adr);
	//				getsym();
	//			}
	//			else if (sym == minus) {
	//				gen(lod, lev - table[i].level, table[i].adr);
	//				gen(lit, 0, 1);
	//				gen(opr, 0, 3);
	//				gen(sto, lev - table[i].level, table[i].adr);
	//				getsym();
	//			}
	//			else if (sym == becomes) {
	//				getsym();
	//				memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	//				additive_expr(nxtlev, ptx, lev);	/* ����ֵ�����Ҳ���ʽ */
	//				if (i != 0) {
	//					/* expression��ִ��һϵ��ָ������ս�����ᱣ����ջ����ִ��sto������ɸ�ֵ */
	//					gen(sto, lev - table[i].level, table[i].adr);
	//				}
	//			}
	//		}
	//	}
	//	if (sym == semicolon) {
	//		getsym();
	//	}
	//	else {
	//		error(1);
	//	}
	//}
	//else {
	if (sym == readsym) {
		getsym();
		if (sym == ident) {
			i = position(id, *ptx);
		}
		else {
			i = 0;
		}
		if (i == 0) {
			error(18);
		}
		else {
			getsym();
			if (sym == semicolon) {
				gen(opr, 0, 16);
				gen(sto, lev - table[i].level, table[i].adr);
				getsym();
			}
			else if (sym == lrange) {
				getsym();
				if (sym == number) {
					gen(opr, 0, 16);
					gen(sto, lev - table[i].level, table[i].adr + num);
					getsym();
					if (sym == rrange) {
						getsym();
						if (sym == semicolon) {
							getsym();
						}
						else {
							error(1);				/* ������ȱ��;*/
						}
					}
					else {
						error(36);					/* ��������Ԫ��ȱ��]*/
					}
				}
				else {
					error(9);						/* []��Ӧ���������ʵ�����*/
				}
			}
			else {
				error(1);							/* ������ȱ��;*/
			}
		}
	}
	else {
		if (sym == writesym) {
			getsym();
			expression(nxtlev, ptx, lev);

			if (isChar) {
				gen(opr, 0, 14);
			}
			else {
				gen(opr, 0, 17);
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
			if (sym == callsym) {			/* ׼������call��䴦�� */
				getsym();
				if (sym != ident) {
					error(0);				/* call��ӦΪ��ʶ�� */
				}
				else {
					i = position(id, *ptx);
					if (i == 0) {
						error(6);			/* ������δ�ҵ� */
					}
					else {
						if (table[i].kind == procedure) {
							gen(cal, lev - table[i].level, table[i].adr);	/* ����callָ�� */
						}
						else {
							error(31);		/* call���ʶ������ӦΪ���� */
						}
					}
					getsym();
					if (sym == semicolon) {
						getsym();
					}
					else {
						error(1);
					}
				}
			}
			else {
				if (sym == ifsym) {	/* ׼������if��䴦�� */
					getsym();
					if (sym == lparen) {
						getsym();
					}
					else {
						error(17);
					}
					expression(nxtlev, ptx, lev); /* ������������ */
					if (sym == rparen) {
						getsym();
					}
					else {
						error(16);
					}
					cx1 = cx;	/* ���浱ǰָ���ַ */
					gen(jpc, 0, 0);	/* ����������תָ���ת��ַδ֪����ʱд0 */
					statement(fsys, ptx, lev);	/* ����then������ */

					cx2 = cx;
					gen(jmp, 0, 0);
					code[cx1].a = cx;	/* ��statement�����cxΪthen�����ִ�����λ�ã�������ǰ��δ������ת��ַ����ʱ���л��� */
					if (sym == elsesym) {
						getsym();
						statement(nxtlev, ptx, lev);
					}
					code[cx2].a = cx;
				}
				else {
					if (sym == whilesym) {				/* ׼������while��䴦�� */
						cx1 = cx;						/* �����ж�����������λ�� */
						getsym();
						if (sym == lparen) {
							getsym();
						}
						else {
							error(17);					/* ȱ��(*/
						}
						memcpy(nxtlev, fsys, sizeof(bool) * symnum);
						nxtlev[lbrace] = true;			/* ��̷���Ϊdo */
						expression(nxtlev, ptx, lev);	/* ������������ */
						cx2 = cx;						/* ����ѭ����Ľ�������һ��λ�� */
						gen(jpc, 0, 0);					/* ����������ת��������ѭ���ĵ�ַδ֪�����Ϊ0�ȴ����� */
						if (sym == rparen) {
							getsym();
						}
						else {
							error(16);	/* ȱ��)*/
						}
						memcpy(nxtlev, fsys, sizeof nxtlev);
						nxtlev[rbrace] = true;
						statement(fsys, ptx, lev);	/* ѭ���� */
						gen(jmp, 0, cx1);	/* ����������תָ���ת��ǰ���ж�����������λ�� */
						code[cx2].a = cx;	/* ��������ѭ���ĵ�ַ */
					}
					else if (sym == forsym) {
						getsym();
						if (sym == ident) {
							i = position(id, *ptx);
							if (i == 0) error(6);
							else {
								if (table[i].kind != variable) {
									error(7);
									i = 0;
								}
								else {
									getsym();
									if (sym == becomes) getsym();
									else error(22);

									if (sym != semicolon) error(1);	/* ���˷ֺ�*/
									else getsym();
									memcpy(nxtlev, fsys, sizeof nxtlev);
									//nxtlev[range] = true;
									additive_expr(nxtlev, ptx, lev);
									gen(sto, lev - table[i].level, table[i].adr);
									//if (sym != range) error(25);
									//else getsym();
									cx1 = cx;
									gen(lod, lev - table[i].level, table[i].adr);
									memcpy(nxtlev, fsys, sizeof nxtlev);
									nxtlev[lbrace] = true;
									additive_expr(nxtlev, ptx, lev);
									gen(opr, 0, 13);
									cx2 = cx;
									gen(jpc, 0, 0);
									if (sym == lbrace)
										getsym();
									else error(20);
									memcpy(nxtlev, fsys, sizeof nxtlev);
									nxtlev[rbrace] = true;
									statement(nxtlev, ptx, lev);
									if (sym == rbrace)
										getsym();
									else error(21);
									gen(lod, lev - table[i].level, table[i].adr);
									gen(lit, 0, 1);
									gen(opr, 0, 2);
									gen(sto, lev - table[i].level, table[i].adr);
									gen(jmp, 0, cx1);
									code[cx2].a = cx;
								}
							}
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
					else {
						expression_stat(nxtlev, ptx, lev);
					}
				}
			}
		}
	}

}

/* �ݹ���ʽ����*/
void expression_stat(bool* fsys, int* ptx, int lev)
{
	bool nxtlev[symnum];
	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	if (sym == semicolon) {
		getsym();
	}
	else {
		expression(nxtlev, ptx, lev);
		if (sym == semicolon) {
			getsym();
		}
		else {
			error(3);/*û�зֺ�*/
		}
	}
}

/* ���ʽ����*/
void expression(bool *fsys, int *ptx, int lev) {
	int i;
	enum symbol oldSym;
	int oldPosition;
	bool nxtlev[symnum];
	if (inset(sym, facbegsys)) {
		if (sym == ident) {
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
					shift = 0;
					getsym();
					int shift2 = shift;
					if (sym == lrange) { /* �ж��Ƿ���array�е�Ԫ��*/
						getsym();
						if (sym == number) {
							getsym();
							int tempNumber = num;
							if (sym == rrange) {
								getsym();
							}
							else {
								error(36);
							}
						}
						else {
							error(9);
						}
					}
					if (sym == becomes) { /* = */
						getsym();
						additive_expr(nxtlev, ptx, lev);	/* ����ֵ�����Ҳ���ʽ */
						if (i != 0) {
							gen(sto, lev - table[i].level, table[i].adr);	/* expression��ִ��һϵ��ָ������ս�����ᱣ����ջ����ִ��sto������ɸ�ֵ */
						}

					}
					else if (sym == neq) { /* != */
						additive_expr(nxtlev, ptx, lev);
					}
					else if (sym == eql) { /* == */
						additive_expr(nxtlev, ptx, lev);
					}
					else if (sym == lss) { /* < */
						additive_expr(nxtlev, ptx, lev);
					}
					else if (sym == leq) { /* <= */
						additive_expr(nxtlev, ptx, lev);
					}
					else if (sym == gtr) { /* > */
						additive_expr(nxtlev, ptx, lev);
					}
					else if (sym == geq) { /* >= */
						additive_expr(nxtlev, ptx, lev);
					}
					else {
						error(6);
					}
				}
			}
			if (sym == semicolon) {
				getsym();
			}
			else {
				error(1);
			}
		}
		else if (sym == number) {
			if (num > amax) {
				error(31);
				num = 0;
			}
			gen(lit, 0, num);
			getsym();
		}
		else if (sym == lparen) {
			getsym();
			expression(nxtlev, ptx, lev);
			if (sym == rparen) {
				getsym();
			}
			else {
				error(22);
			}
		}
		else {
			error(6);
		}
	}
}

void simple_expr(bool* fsys, int* ptx, int lev) {
	enum symbol relop;
	bool nxtlev[symnum];

	if (sym == oddsym)	/* ׼������odd���㴦�� */
	{
		getsym();
		additive_expr(fsys, ptx, lev);
		gen(opr, 0, 6);	/* ����oddָ�� */
	}
	else
	{
		/* �߼����ʽ���� */
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
			error(20); /* Ӧ��Ϊ��ϵ����� */
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
 * ���ʽ����
 */
void additive_expr(bool* fsys, int* ptx, int lev)
{
	enum symbol addop;	/* ���ڱ��������� */
	bool nxtlev[symnum];

	if (sym == plus || sym == minus)	/* ���ʽ��ͷ�������ţ���ʱ��ǰ���ʽ������һ�����Ļ򸺵��� */
	{
		addop = sym;	/* ���濪ͷ�������� */
		getsym();
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);	/* ������ */
		if (addop == minus)
		{
			gen(opr, 0, 1);	/* �����ͷΪ��������ȡ��ָ�� */
		}
	}
	else	/* ��ʱ���ʽ��������ļӼ� */
	{
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);	/* ������ */
	}
	while (sym == plus || sym == minus) {
		addop = sym;
		getsym();
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);	/* ������ */
		if (addop == plus) {
			gen(opr, 0, 2);	/* ���ɼӷ�ָ�� */
		}
		else {
			gen(opr, 0, 3);	/* ���ɼ���ָ�� */
		}
	}
}

/*
 * ���
 */
void term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;	/* ���ڱ���˳������� */
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[times] = true;
	nxtlev[slash] = true;
	factor(nxtlev, ptx, lev);	/* �������� */
	while (sym == times || sym == slash || sym == mod) {
		mulop = sym;
		getsym();
		factor(nxtlev, ptx, lev);
		if (mulop == times) {
			gen(opr, 0, 4);	/* ���ɳ˷�ָ�� */
		}
		else if (mulop == slash) {
			gen(opr, 0, 5);	/* ���ɳ���ָ�� */
		}
		else {
			gen(opr, 0, 17);
		}
	}
}

/*
 * ���Ӵ���
 */
void factor(bool* fsys, int* ptx, int lev)
{
	int i;
	bool nxtlev[symnum];
	//test(facbegsys, fsys, 24);	/* ������ӵĿ�ʼ���� */
	while (inset(sym, facbegsys)) 	/* ѭ���������� */
	{
		if (sym == ident)	/* ����Ϊ��������� */
		{
			i = position(id, *ptx);	/* ���ұ�ʶ���ڷ��ű��е�λ�� */
			if (i == 0)
			{
				error(11);	/* ��ʶ��δ���� */
			}
			else
			{
				switch (table[i].kind)
				{
				case constant:	/* ��ʶ��Ϊ���� */
					gen(lit, 0, table[i].val);	/* ֱ�Ӱѳ�����ֵ��ջ */
					break;
				case variable:	/* ��ʶ��Ϊ���� */
					gen(lod, lev - table[i].level, table[i].adr);	/* �ҵ�������ַ������ֵ��ջ */
					break;
				case procedure:	/* ��ʶ��Ϊ���� */
					error(21);	/* ����Ϊ���� */
					break;
				}
			}
			getsym();
		}
		else
		{
			if (sym == number)	/* ����Ϊ�� */
			{
				if (num > amax)
				{
					error(31); /* ��Խ�� */
					num = 0;
				}
				gen(lit, 0, num);
				getsym();
			}
			else
			{
				if (sym == lparen)	/* ����Ϊ���ʽ */
				{
					getsym();
					memcpy(nxtlev, fsys, sizeof(bool) * symnum);
					nxtlev[rparen] = true;
					additive_expr(nxtlev, ptx, lev);
					if (sym == rparen)
					{
						getsym();
					}
					else
					{
						error(22);	/* ȱ�������� */
					}
				}
			}
		}
	}
}

/*
 * ��������
 */
void condition(bool* fsys, int* ptx, int lev)
{
	enum symbol relop;
	bool nxtlev[symnum];

	if (sym == oddsym)	/* ׼������odd���㴦�� */
	{
		getsym();
		additive_expr(fsys, ptx, lev);
		gen(opr, 0, 6);	/* ����oddָ�� */
	}
	else
	{
		/* �߼����ʽ���� */
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
			error(20); /* Ӧ��Ϊ��ϵ����� */
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
 * ���ͳ���
 */
void interpret()
{
	int p = 0; /* ָ��ָ�� */
	int b = 1; /* ָ���ַ */
	int t = 0; /* ջ��ָ�� */
	struct instruction i;	/* ��ŵ�ǰָ�� */
	int s[stacksize];	/* ջ */

	s[0] = 0; /* s[0]���� */
	s[1] = 0; /* �������������ϵ��Ԫ����Ϊ0 */
	s[2] = 0;
	s[3] = 0;
	do {
		//if (cx == 0) break;
		i = code[p];	/* ����ǰָ�� */
		p = p + 1;
		switch (i.f)
		{
		case lit:	/* ������a��ֵȡ��ջ�� */
			t = t + 1;
			s[t] = i.a;
			break;
		case opr:	/* ��ѧ���߼����� */
			switch (i.a)
			{
			case 0:  /* �������ý����󷵻� */
				t = b - 1;
				p = s[t + 3];
				b = s[t + 2];
				break;
			case 1: /* ջ��Ԫ��ȡ�� */
				s[t] = -s[t];
				break;
			case 2: /* ��ջ�������ջ���������ջԪ�أ����ֵ��ջ */
				t = t - 1;
				s[t] = s[t] + s[t + 1];
				break;
			case 3:/* ��ջ�����ȥջ���� */
				t = t - 1;
				s[t] = s[t] - s[t + 1];
				break;
			case 4:/* ��ջ�������ջ���� */
				t = t - 1;
				s[t] = s[t] * s[t + 1];
				break;
			case 5:/* ��ջ�������ջ���� */
				t = t - 1;
				s[t] = s[t] / s[t + 1];
				break;
			case 6:/* ջ��Ԫ�ص���ż�ж� */
				s[t] = s[t] % 2;
				break;
			case 8:/* ��ջ������ջ�����Ƿ���� */
				t = t - 1;
				s[t] = (s[t] == s[t + 1]);
				break;
			case 9:/* ��ջ������ջ�����Ƿ񲻵� */
				t = t - 1;
				s[t] = (s[t] != s[t + 1]);
				break;
			case 10:/* ��ջ�����Ƿ�С��ջ���� */
				t = t - 1;
				s[t] = (s[t] < s[t + 1]);
				break;
			case 11:/* ��ջ�����Ƿ���ڵ���ջ���� */
				t = t - 1;
				s[t] = (s[t] >= s[t + 1]);
				break;
			case 12:/* ��ջ�����Ƿ����ջ���� */
				t = t - 1;
				s[t] = (s[t] > s[t + 1]);
				break;
			case 13: /* ��ջ�����Ƿ�С�ڵ���ջ���� */
				t = t - 1;
				s[t] = (s[t] <= s[t + 1]);
				break;
			case 14:/* ջ��ֵ��� */
				printf("%d", s[t]);
				fprintf(fresult, "%d", s[t]);
				t = t - 1;
				break;
			case 15:/* ������з� */
				printf("\n");
				fprintf(fresult, "\n");
				break;
			case 16:/* ����һ����������ջ�� */
				t = t + 1;
				printf("Input:");
				fprintf(fresult, "Input:");
				scanf("%d", &(s[t]));
				fprintf(fresult, "%d\n", s[t]);
				break;
			case 17:/* ջ��ֵchar������� */
				printf("%c", s[t]);
				fprintf(fresult, "%c", s[t]);
				t = t - 1;
				break;
			}
			break;
		case lod:	/* ȡ��Ե�ǰ���̵����ݻ���ַΪa���ڴ��ֵ��ջ�� */
			t = t + 1;
			s[t] = s[base(i.l, s, b) + i.a];
			break;
		case sto:	/* ջ����ֵ�浽��Ե�ǰ���̵����ݻ���ַΪa���ڴ� */
			s[base(i.l, s, b) + i.a] = s[t];
			t = t - 1;
			break;
		case cal:	/* �����ӹ��� */
			s[t + 1] = base(i.l, s, b);	/* �������̻���ַ��ջ����������̬�� */
			s[t + 2] = b;	/* �������̻���ַ��ջ����������̬�� */
			s[t + 3] = p;	/* ����ǰָ��ָ����ջ�������淵�ص�ַ */
			b = t + 1;	/* �ı����ַָ��ֵΪ�¹��̵Ļ���ַ */
			p = i.a;	/* ��ת */
			break;
		case ini:	/* ������ջ��Ϊ�����õĹ��̿���a����Ԫ�������� */
			t = t + i.a;
			break;
		case jmp:	/* ֱ����ת */
			p = i.a;
			break;
		case jpc:	/* ������ת */
			if (s[t] == 0)
				p = i.a;
			t = t - 1;
			break;
		}
	} while (p != 0);
	printf("End X0\n");
	fprintf(fresult, "Execute Successfully\n");
}

/* ͨ�����̻�ַ����l����̵Ļ�ַ */
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
