/*
* PL/0 complier program implemented in C
*
* The program has been tested on Visual Studio 2010
*
* ʹ�÷�����
* ���к�����PL/0Դ�����ļ���
* �ش��Ƿ�������������
* �ش��Ƿ�������ű�
* fcode.txt������������
* foutput.txt���Դ�ļ�������ʾ�⣨���д��͸��ж�Ӧ�����ɴ����׵�ַ�����޴�
* fresult.txt������н��
* ftable.txt������ű�
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define bool int
#define true 1
#define false 0

#define norw 8       /* �����ָ��� */
#define txmax 100     /* ���ű����� */
#define nmax 14       /* ���ֵ����λ�� */
#define al 10         /* ��ʶ������󳤶� */
#define maxerr 30     /* ������������� */
#define amax 2048     /* ��ַ�Ͻ�*/
#define levmax 3      /* ����������Ƕ����������*/
#define cxmax 200     /* ��������������� */
#define stacksize 1000000 /* ����ʱ����ջԪ�����Ϊ500�� */

/* ���� */
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

/* ���ű��е����� */
enum object {
	integer,
	//int_list,
	character,
	//char_list
};

/* ���������ָ�� */
enum fct {
	lit, opr, lod,
	sto, cal, ini,
	jmp, jpc,
};
#define fctnum 8

/* ���������ṹ */
struct instruction
{
	enum fct f; /* ���������ָ�� */
	int l;      /* ���ò���������Ĳ�β� */
	int a;      /* ����f�Ĳ�ͬ����ͬ */
};

int linecount = 0;
int shift = 0;
int shouldbechar=0; /* �����char�����ʽ��ͬ */
bool listswitch;   /* ��ʾ������������ */
bool tableswitch;  /* ��ʾ���ű���� */
char ch;            /* ��ŵ�ǰ��ȡ���ַ���getch ʹ�� */
enum symbol sym;    /* ��ǰ�ķ��� */
char id[al + 1];      /* ��ǰident�������һ���ֽ����ڴ��0 */
int num;            /* ��ǰnumber */
int cc, ll;         /* getchʹ�õļ�������cc��ʾ��ǰ�ַ�(ch)��λ�� */
int cx;             /* ���������ָ��, ȡֵ��Χ[0, cxmax-1]*/
char line[81];      /* ��ȡ�л����� */
char a[al + 1];       /* ��ʱ���ţ������һ���ֽ����ڴ��0 */
struct instruction code[cxmax]; /* ����������������� */
char word[norw][al];        /* ������ */
enum symbol wsym[norw];     /* �����ֶ�Ӧ�ķ���ֵ */
enum symbol ssym[256];      /* ���ַ��ķ���ֵ */
char mnemonic[fctnum][5];   /* ���������ָ������ */
bool declbegsys[symnum];    /* ��ʾ������ʼ�ķ��ż��� */
bool statbegsys[symnum];    /* ��ʾ��俪ʼ�ķ��ż��� */
bool facbegsys[symnum];     /* ��ʾ���ӿ�ʼ�ķ��ż��� */

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
FILE* ferr;  /* ������� */
char fname[al];
int err;        /* ��������� */

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


/* ������ʼ */
int main()
{
	bool nxtlev[symnum];
	printf("Input pl/0 file?   ");
	//scanf("%s", fname);		/* �����ļ��� */
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
	printf("List object codes\n");	/* �Ƿ������������� */

	listswitch = 1;

	printf("List symbol table\n");	/* �Ƿ�������ű� */
	tableswitch = 1;

	init();		/* ��ʼ�� */
	err = 0;
	cc = ll = cx = 0;
	ch = ' ';

	getsym();

	addset(nxtlev, declbegsys, statbegsys, symnum);
	block(0, 0, nxtlev);	/* ����ֳ��� */

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

		listall();	 /* ������д��� */
		fclose(fcode);
		fclose(ferr);
		//interpret();	/* ���ý���ִ�г��� */
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
* ��ʼ��
*/
void init()
{
	int i;

	/* ���õ��ַ����� */
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

	/* ���ñ���������,������ĸ˳�򣬱��ڶ��ֲ��� */
	strcpy(&(word[0][0]), "char");
	strcpy(&(word[1][0]), "else");
	strcpy(&(word[2][0]), "if");
	strcpy(&(word[3][0]), "int");
	strcpy(&(word[4][0]), "main");
	strcpy(&(word[5][0]), "read");
	strcpy(&(word[6][0]), "while");
	strcpy(&(word[7][0]), "write");


	/* ���ñ����ַ��� */
	wsym[0] = charsym;
	wsym[1] = elsesym;
	wsym[2] = ifsym;
	wsym[3] = intsym;
	wsym[4] = mainsym;
	wsym[5] = readsym;
	wsym[6] = whilesym;
	wsym[7] = writesym;


	/* ����ָ������ */
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[ini][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");

	/* ���÷��ż� */
	for (i = 0; i<symnum; i++)
	{
		declbegsys[i] = false;
		statbegsys[i] = false;
		facbegsys[i] = false;
	}

	/* ����������ʼ���ż� */
	declbegsys[intsym] = true;
	declbegsys[charsym] = true;
	//declbegsys[procsym] = true;

	/* ������俪ʼ���ż� */
	//statbegsys[callsym] = true;
	statbegsys[ifsym] = true;
	statbegsys[whilesym] = true;
	statbegsys[readsym] = true;
	statbegsys[writesym] = true;
	statbegsys[lparen3] = true;
	statbegsys[ident] = true;
	statbegsys[semicolon] = true;

	/* �������ӿ�ʼ���ż� */
	facbegsys[ident] = true;
	facbegsys[number] = true;
	facbegsys[lparen] = true;
}

/*
* ������ʵ�ּ��ϵļ�������
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
*	��������ӡ����λ�úʹ������
*/
void error(int n)
{
	char space[81];
	memset(space, 32, 81);

	space[cc - 1] = 0; /* ����ʱ��ǰ�����Ѿ����꣬����cc-1 */
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
* ���˿ո񣬶�ȡһ���ַ�
* ÿ�ζ�һ�У�����line��������line��getsymȡ�պ��ٶ�һ��
* ������getsym����
*/
void getch()
{
	if (cc == ll) /* �жϻ��������Ƿ����ַ��������ַ����������һ���ַ����������� */
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
		do {    /* ������ǰ�����Ƿ�Ϊ�����֣�ʹ�ö��ַ����� */
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
		if (i - 1 > j) /* ��ǰ�ĵ����Ǳ����� */
		{
			sym = wsym[k];
		}
		else /* ��ǰ�ĵ����Ǳ�ʶ�� */
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
				error(1);
			}
		}
		else
		{
			if (ch == '=')		/* �����ڷ��� */
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
				if (ch == '<')		/* ���С�ڻ�С�ڵ��ڷ��� */
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
					if (ch == '>')		/* �����ڻ���ڵ��ڷ��� */
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
						sym = ssym[ch];		/* �����Ų�������������ʱ��ȫ�����յ��ַ����Ŵ��� */
							getch();

					}
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
void gen(enum fct x, int y, int z)
{
	if (cx >= cxmax)
	{
		printf("Program is too long!\n");	/* ���ɵ���������������� */
		exit(1);
	}
	if (z >= amax)
	{
		printf("Displacement address is too big!\n");	/* ��ַƫ��Խ�� */
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
	if (!inset(sym, s1))
	{
		error(n);
		/* ����ⲻͨ��ʱ����ͣ��ȡ���ţ�ֱ����������Ҫ�ļ��ϻ򲹾ȵļ��� */
		//while ((!inset(sym, s1)) && (!inset(sym, s2)))
		//{
		//	getsym();
		//}
	}
}

/*
* ����������� block��ʵ��Ӧprogram
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
		error(2);
	}
	if (sym == mainsym) { getsym(); }
	if (sym == lparen3) { getsym(); }
	do {


		if (sym == intsym)		/* ���������������ţ���ʼ����������� */
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
					//num; ������ά��
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
						error(4);/*ȱ�١�*/
					}
				}
				else
				{
					error(3);
				} /* ©���˷ֺ� */
			}
		}
		if (sym == charsym)		/* ���������������ţ���ʼ����������� */
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
					//num; ������ά��
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
						error(4);/*ȱ�١�*/
					}
				}
				else
				{
					error(3);
				} /* ©���˷ֺ� */
			}
		}

		//memcpy(nxtlev, statbegsys, sizeof(bool) * symnum);
		//nxtlev[semicolon] = true;
		//test(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));	/* ֱ��û���������� */

	code[table[tx0].adr].a = cx;	/* ��ǰ�����ɵ���ת������תλ�øĳɵ�ǰλ�� */
	table[tx0].adr = cx;	        /* ��¼��ǰ���̴����ַ */
	table[tx0].size = dx;	        /* ����������ÿ����һ�����������dx����1�����������Ѿ�������dx���ǵ�ǰ�������ݵ�size */
	cx0 = cx;
	gen(ini, 0, dx);	            /* ����ָ���ָ��ִ��ʱ������ջ��Ϊ�����õĹ��̿���dx����Ԫ�������� */


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


	/* ����̷���Ϊ�ֺŻ�end */
	memcpy(nxtlev, fsys, sizeof(bool) * symnum);	/* ÿ����̷��ż��϶������ϲ��̷��ż��ϣ��Ա㲹�� */
	nxtlev[semicolon] = true;
	statement_list(nxtlev, &tx, lev);
	gen(opr, 0, 0);	                    /* ÿ�����̳��ڶ�Ҫʹ�õ��ͷ����ݶ�ָ�� */
	memset(nxtlev, 0, sizeof(bool) * symnum);	/* �ֳ���û�в��ȼ��� */
	//test(fsys, nxtlev, 8);            	/* ����̷�����ȷ�� */
	listcode(cx0);                      /* ������ֳ������ɵĴ��� */
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
	case character:	/* ���� */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case integer:	/* ���� */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	//case procedure:	/* ���� */
	//	table[(*ptx)].level = lev;
	//	break;
	}
}

/*������˵�һ�����û���֣��Ͳ��ᱻ�ҵ�*/
void enter2(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, ""); /* ���ű��name���¼��ʶ���������ǿ� */
	table[(*ptx)].kind = k;
	switch (k)
	{
	case character:	/* ���� */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case integer:	/* ���� */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
		//case procedure:	/* ���� */
		//	table[(*ptx)].level = lev;
		//	break;
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
	while (strcmp(table[i].name, id) != 0)
	{
		i--;
	}
	if (table[i].kind == character)shouldbechar=1;
	else shouldbechar = 0;
	return i;
}



/*
* ������������
*/
void intdeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)
	{
		enter(integer, ptx, lev, pdx);	// ��д���ű�
		getsym();
	}
	else
	{
		error(5);	/* var����Ӧ�Ǳ�ʶ�� */
	}
}
/*
* int������������
*/
void intlistdeclaration(int * ptx, int lev, int * pdx, int length)
{
	if (sym == number)
	{
		int i = 0;
		for (i = 0; i < length - 1; ++i) {
			enter2(integer, ptx, lev, pdx);// ��д���ű�
		}	
		getsym();
	}
	else
	{
		error(6);	/* ���鷽������Ӧ���� */
	}
}


void chardeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)
	{
		enter(character, ptx, lev, pdx);	// ��д���ű�
		getsym();
	}
	else
	{
		error(5);	/* var����Ӧ�Ǳ�ʶ�� */
	}
}
/*
* char������������
*/
void charlistdeclaration(int * ptx, int lev, int * pdx, int length)
{
	if (sym == number)
	{
		int i = 0;
		for (i = 0; i < length - 1; ++i) {
			enter2(character, ptx, lev, pdx);// ��д���ű�
		}
		getsym();
	}
	else
	{
		error(6);	/* ���鷽������Ӧ���� */
	}
}

/*
* ���Ŀ������嵥
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
* �������Ŀ�����
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
* ��䴦��2
*/
void statement(bool* fsys, int* ptx, int lev)
{
	int i, cx1, cx2;
	bool nxtlev[symnum];

	//if (sym == ident)	/* ׼�����ո�ֵ��䴦��  expression_stat -> var=expression; */
	//{
	//	i = position(id, *ptx);/* ���ұ�ʶ���ڷ��ű��е�λ�� */
	//	if (i == 0)
	//	{
	//		error(11);	/* ��ʶ��δ���� */
	//	}
	//	else
	//	{
	//		if (table[i].kind != character && table[i].kind != integer)
	//		{
	//			error(12);	/* ��ֵ����У���ֵ���󲿱�ʶ��Ӧ���Ǳ��� */
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
	//				error(13);	/* û�м�⵽��ֵ���� */
	//			}
	//			memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	//			expression(nxtlev, ptx, lev);	/* ����ֵ�����Ҳ���ʽ */
	//			if (i != 0)
	//			{
	//				/* expression��ִ��һϵ��ָ������ս�����ᱣ����ջ����ִ��sto������ɸ�ֵ */
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

	if (sym == readsym)	/* ׼������read��䴦�� */
	{
		getsym();

		if (sym == ident)
		{
			i = position(id, *ptx);	/* ����Ҫ���ı��� */
		}
		else
		{
			i = 0;
		}

		if (i == 0)
		{
			error(7);	/* read��������еı�ʶ��Ӧ�����������ı��� */
		}
		else
		{
			getsym();
			if (sym == semicolon) {
				getsym();
				gen(opr, 0, 16);	/* ��������ָ���ȡֵ��ջ�� */
				gen(sto, lev - table[i].level, table[i].adr);	/* ��ջ���������������Ԫ�� */
			}
			else if (sym == lparen2) {
				getsym();
				if (sym == number) {
					gen(opr, 0, 16);	/* ��������ָ���ȡֵ��ջ�� */
					gen(sto, lev - table[i].level, table[i].adr + num);	/* ��ջ���������������Ԫ�� */
					getsym();
					if (sym == rparen2) {
						getsym();
						if (sym == semicolon)
							getsym();
						else
							error(3);//û�ֺ�
					}
					else { error(4);/*����û�С�*/ }
				}
				else { error(6);/*���顾���ڱ���������*/ }
			}
			else { error(3); }//û�зֺ�
		}

	}
	else
	{
		if (sym == writesym)	/* ׼������write��䴦�� */
		{
			getsym();
			memcpy(nxtlev, fsys, sizeof(bool) * symnum);
			nxtlev[rparen] = true;
			expression(nxtlev, ptx, lev);	/* ���ñ��ʽ���� */
			if (!shouldbechar) {
				gen(opr, 0, 14);	/* �������ָ����ջ����ֵ */
				gen(opr, 0, 15);	/* ���ɻ���ָ�� */
			}
			else {
				gen(opr, 0, 17);	/* �������ָ����ջ����ֵ */
				gen(opr, 0, 15);	/* ���ɻ���ָ�� */
			}
			if (sym == semicolon)
			{
				getsym();
			}
			else 
			{
				error(3);
			}//û�зֺ�
		}
		//************************************************************************************************************
		else
		{
			if (false) // sym == callsym)	/* ׼������call��䴦�� */
			{
				getsym();
				if (sym != ident)
				{
					error(8);	/* call��ӦΪ��ʶ�� */
				}
				else
				{
					i = position(id, *ptx);
					if (i == 0)
					{
						error(9);	/* ������δ�ҵ� */
					}
					else
					{
						//if (table[i].kind == procedure)
						{
							gen(cal, lev - table[i].level, table[i].adr);	/* ����callָ�� */
						}
						//else
						{
							error(8);	/* call���ʶ������ӦΪ���� */
						}
					}
					getsym();
				}
			}
			//************************************************************************************************************
			else
			{

				if (sym == ifsym)	/* ׼������if��䴦�� */
				{
					getsym();
					if (sym == lparen)getsym();
					else error(10);
					memcpy(nxtlev, fsys, sizeof(bool) * symnum);
					nxtlev[rparen] = true;	/* ��̷���Ϊ) */
					//condition(nxtlev, ptx, lev); /* ������������ */
					expression(nxtlev, ptx, lev); /* ������������ */
					if (sym == rparen)
					{
						getsym();
					}
					else
					{
						error(11);	/* ȱ��) */
					}
					cx1 = cx;	/* ���浱ǰָ���ַ */
					gen(jpc, 0, 0);	/* ����������תָ���ת��ַδ֪����ʱд0 */
					statement(fsys, ptx, lev);	/* ����then������ */
					
					cx2 = cx;
					gen(jmp, 0, 0);
					code[cx1].a = cx;	/* ��statement�����cxΪthen�����ִ�����λ�ã�������ǰ��δ������ת��ַ����ʱ���л��� */
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
					if (sym == whilesym)	/* ׼������while��䴦�� */
					{
						cx1 = cx;	/* �����ж�����������λ�� */
						getsym();
						memcpy(nxtlev, fsys, sizeof(bool) * symnum);
						if (sym == lparen)
						{
							getsym();
						}
						else
						{
							error(12);	/* ȱ��( */
						}
						expression(nxtlev, ptx, lev);	/* ������������ */
						cx2 = cx;	/* ����ѭ����Ľ�������һ��λ�� */
						gen(jpc, 0, 0);	/* ����������ת��������ѭ���ĵ�ַδ֪�����Ϊ0�ȴ����� */
						if (sym == rparen)
						{
							getsym();
						}
						else
						{
							error(11);	/* ȱ��) */
						}
						statement(fsys, ptx, lev);	/* ѭ���� */

						gen(jmp, 0, cx1);	/* ����������תָ���ת��ǰ���ж�����������λ�� */
						code[cx2].a = cx;	/* ��������ѭ���ĵ�ַ */
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
								else { error(13); } /*û���Ҵ�����*/
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


	//memset(nxtlev, 0, sizeof(bool) * symnum);	/* �������޲��ȼ��� */
	//test(fsys, nxtlev, 19);	/* �������������ȷ�� */
}
void expression_stat(bool* fsys, int* ptx, int lev)
{
	printf("expression_stat\n");
	bool nxtlev[symnum];
	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	if (sym == semicolon) { getsym(); }
	else {
		expression(nxtlev, ptx, lev);
		if (sym != semicolon) { error(3);/*û�зֺ�*/ }
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
		if (sym == ident)	/* ׼�����ո�ֵ��䴦��  expression_stat -> var=expression; */
		{
			i = position(id, *ptx);/* ���ұ�ʶ���ڷ��ű��е�λ�� */
			if (i == 0)
			{
				error(14);	/* ��ʶ��δ���� */
			}
			else
			{
				if (table[i].kind != character && table[i].kind != integer)
				{
					error(15);	/* ��ֵ����У���ֵ���󲿱�ʶ��Ӧ���Ǳ��� */
					i = 0;
				}
				else
				{
					//getsym();
					shift = 0;	
					simple_expr(nxtlev, ptx, lev); //���ʧ����sym����=����ζ��Ӧ����var=expression
					int shift2 = shift;
					if (sym == becomes)
					{
						getsym();
						memcpy(nxtlev, fsys, sizeof(bool) * symnum);
						expression(nxtlev, ptx, lev);	/* ����ֵ�����Ҳ���ʽ */
						if (i != 0)
						{
							/* expression��ִ��һϵ��ָ������ս�����ᱣ����ջ����ִ��sto������ɸ�ֵ */
							gen(sto, lev - table[i].level, table[i].adr+shift2);
						}
					}
					else if (sym == lparen2) { //����·��������
						printf("�߽��˲������ߵ�·\n");
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
									expression(nxtlev, ptx, lev);	/* ����ֵ�����Ҳ���ʽ */
									if (i != 0)
									{
										/* expression��ִ��һϵ��ָ������ս�����ᱣ����ջ����ִ��sto������ɸ�ֵ */
										gen(sto, lev - table[i].level, table[i].adr + xx);
									}
								}
								else { error(16);/*δ֪����*/ }
							}
							else { error(4);/*û�С�*/ }
						}
						else { error(6);/*����û����*/ }
					}
					else
					{
						printf("��simple expression �ɹ��ˣ�\n");
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
* ���ʽ���� ��additive_expr ����simple_exprһ����
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
	while (sym == plus || sym == minus)
	{
		addop = sym;
		getsym();
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);	/* ������ */
		if (addop == plus)
		{
			gen(opr, 0, 2);	/* ���ɼӷ�ָ�� */
		}
		else
		{
			gen(opr, 0, 3);	/* ���ɼ���ָ�� */
		}
	}
}

/*
* ��� ��pascalһ��
*/
void term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;	/* ���ڱ���˳������� */
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[multiply] = true;
	nxtlev[divide] = true;
	factor(nxtlev, ptx, lev);	/* �������� */
	while (sym == multiply || sym == divide)
	{
		mulop = sym;
		getsym();
		factor(nxtlev, ptx, lev);
		if (mulop == multiply)
		{
			gen(opr, 0, 4);	/* ���ɳ˷�ָ�� */
		}
		else
		{
			gen(opr, 0, 5);	/* ���ɳ���ָ�� */
		}
	}
}

/*
* ���Ӵ���   ��pascalһ��
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
				error(14);	/* ��ʶ��δ���� */
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
							case integer:	/* ��ʶ��Ϊ���� */
								shift = xx;
								gen(lod, lev - table[i].level, table[i].adr+xx);	/* �ҵ�������ַ������ֵ��ջ */
								break;
							case character:	/* ��ʶ��Ϊ���� */
								shift = xx;
								gen(lod, lev - table[i].level, table[i].adr+xx);	/* �ҵ�������ַ������ֵ��ջ */
								break;
								//case procedure:	/* ��ʶ��Ϊ���� */
								//	error(21);	/* ����Ϊ���� */
								//	break;
							}
						}
						else { error(4);/*û�С�*/ }
					}
					else { error(6);/*����û������*/ }
				}
				else {
					switch (table[i].kind)
					{
					case integer:	/* ��ʶ��Ϊ���� */
						gen(lod, lev - table[i].level, table[i].adr);	/* �ҵ�������ַ������ֵ��ջ */
						break;
					case character:	/* ��ʶ��Ϊ���� */
						gen(lod, lev - table[i].level, table[i].adr);	/* �ҵ�������ַ������ֵ��ջ */
						break;
						//case procedure:	/* ��ʶ��Ϊ���� */
						//	error(21);	/* ����Ϊ���� */
						//	break;
					}
				}
			}
		}
		else
		{
			if (sym == number)	/* ����Ϊ�� */
			{
				if (num > amax)
				{
					error(17); /* ��Խ�� */
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
					expression(nxtlev, ptx, lev);
					if (sym == rparen)
					{
						getsym();
					}
					else
					{
						error(11);	/* ȱ�������� */
					}
				}
			}
		}
		//memset(nxtlev, 0, sizeof(bool) * symnum);
		//nxtlev[lparen] = true;
		//test(fsys, nxtlev, 4); /* һ�����Ӵ�����ϣ������ĵ���Ӧ��fsys������ */
	}
}

/*
* ��������  simple_exprһ����  ���޸�
*/
void simple_expr(bool* fsys, int* ptx, int lev)
{
	enum symbol relop;
	bool nxtlev[symnum];

	//if (sym == oddsym)	/* ׼������odd���㴦�� */
	//{
	//	getsym();
	//	expression(fsys, ptx, lev);
	//	gen(opr, 0, 6);	/* ����oddָ�� */
	//}
	//else
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
			/* Ӧ���Ǵ�additive_expr ʲô������ */
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
int s[stacksize];	/* ջ ����� �����Ǿֲ����� */
void interpret()
{
	int p = 0; /* ָ��ָ�� */
	int b = 1; /* ָ���ַ */
	int t = 0; /* ջ��ָ�� */
	struct instruction i;	/* ��ŵ�ǰָ�� */
	

	printf("Start pl0\n");
	fprintf(fresult, "----��ʼִ��---\n");
	s[0] = 0; /* s[0]���� */
	s[1] = 0; /* �������������ϵ��Ԫ����Ϊ0 */
	s[2] = 0;
	s[3] = 0;
	do {
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
				printf("?");
				fprintf(fresult, "?");
				scanf("%d", &(s[t]));
				fprintf(fresult, "%d\n", s[t]);
				break;
			case 17:/* ջ��ֵ��� ����char */
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
			//t = t - 1;  //�����޷�Ƕ�׸�ֵ
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
	printf("End pl0\n");
	fprintf(fresult, "----ִ�гɹ�---\n");
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


