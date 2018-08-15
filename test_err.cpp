#include <stdio.h>
#include <conio.h>
#include <windows.h>

#define STRMAX      100  // ���ᨬ��쭠� ����� ��ப�
#define SYMMAX      100  // ������ ⠡���� ᨬ����� (��)
#define EOS         '\0'
#define NUM         256
#define ID          257
#define DONE        258
#define COMMENT     '$'
#define COUNT_TERM  11

typedef FILE* TextFile;

struct Entry // ������ � ��
{
	char token[STRMAX];
	char flag_def;      // ���� ��।������� ��६����� (��ꥪ�)
};

struct TStack // ������� �⥪� �ନ�����
{
	char a;
	TStack *last, *next;
};

TextFile in_f, relat;

TStack *Current = NULL;

Entry symtable[SYMMAX];
int lastentry = -1; // ��᫥���� �ᯮ�짮������ ������ � ��
int tokenval;       // ����� ⥪�饩 ����� � ��

void add_in_stack(char b) // ����饭�� b � �⥪
{
	Current -> next = new TStack;
	Current -> next -> last = Current;
	Current = Current -> next;
	Current -> a = b;
}

char remove_from_stack() // ����� � �⥪� �����
{
	char res = Current -> a;
	Current = Current -> last;
	delete Current -> next;
	return res;
}

void free_resources() // �᢮������ ������
{
	if(Current != NULL)
	{
		while(Current -> last != NULL)remove_from_stack();
		delete Current;
	}
	fclose(in_f);
	fclose(relat);
	putchar('\n');
	getch();
}

void init_stack() // ���樠������ �⥪�
{
	Current = new TStack;
	Current -> last = NULL;
	Current -> a = 10;
}

void my_exit(int c)
{
	free_resources();
	exit(c);
}

char pos_in_symtable(char s[]) // ������ s � ��
{
	for(int p =	0; p <= lastentry; p++)
		if(!strcmp(symtable[p].token, s))return p;
	return 0;
}

int lineno = 1;

void insert(char s[]) // ������塞 ���� ⮪�� � ��
{
	if(lastentry++ == SYMMAX)
	{
		printf("�訡�� �������樨 (%d): ��९������� ⠡���� ᨬ�����.", lineno);
		my_exit(1);
	}
	strcpy(symtable[lastentry].token, s);
	symtable[lastentry].flag_def = 0; // ��ꥪ� �� ��।����
}

char flag_separ = 0, op_3par = 0, nNUM = 0, is_id = 0;

int lexan() // �����᪨� ���������
{
	int t;
	while(1)
	{
		if(flag_separ)
		{
			flag_separ = 0;
			return '$';
		}
		t = fgetc(in_f);
		if(t == ' ' || t == '\t');
		else if(t == '\n')lineno++;
		else if(isdigit(t)) // t - ���
		{
			char was_point = 1;
			while(isdigit(t))
			{
				t = fgetc(in_f);
				if(t == '.' && was_point)
				{
					was_point = 0;
					t = fgetc(in_f);
					if(!isdigit(t))
					{
						ungetc(t, in_f);
						ungetc('.', in_f);
						return NUM;
					}
				}
			}
			ungetc(t, in_f);
			return NUM;
		}
		else if(isalpha(t)) // t - �㪢�
		{
			char lexbuf[STRMAX];
			int b = 0;
			while(isalnum(t)) // t - �㪢� ��� ���
			{
				lexbuf[b] = toupper(t);
				if (b++ == STRMAX)
				{
					printf("�訡�� �������樨 (%d): ��९������� ����.", lineno);
					my_exit(2);
				}
				t = fgetc(in_f);
			}
			lexbuf[b] = EOS;
			if(t != EOF)ungetc(t, in_f);
			if(!pos_in_symtable(lexbuf))insert(lexbuf);
			return ID;
		}
		else if(t == COMMENT) // ��ࠡ��뢠�� �������਩
			if(fgetc(in_f) == COMMENT)
				while(1)
				{
					t = fgetc(in_f);
					if(t == '\n')
					{
						lineno++;
						break;
					}
					if(t == EOF)break;
				}
			else
			{
				printf("�訡�� (%d): ���ࠢ���� �������਩.", lineno);
				my_exit(3);
			}
		else if(t == ';')
		{
			flag_separ = 1;
			return t;
		}
		else if(t == '=')
		{
			if(is_id)symtable[tokenval].flag_def = 1;
			return t;
		}
		else if(t == '/' || t == ',')return t;
		else if(t == EOF)return DONE;
		else
		{
			printf("�訡�� (%d): �������⨬� ᨬ��� - '%c'.", lineno, t);
			my_exit(4);
		}
	}
}

// ��१�ࢨ஢���� ���祢� ᫮��
char *keywords[] = {"CIRCLE", "LINE", "GOTO", "GOFWD"};

// ��ॢ�� arg � �।�⠢����� ��� ᨭ⠪��᪮�� ���������
char convert(int arg)
{
	switch(arg)
	{
		case ID:
			char p;
			for(p=0; p<4; p++)
				if(!strcmp(keywords[p], symtable[lastentry].token))
				{
					lastentry--;
					op_3par = p < 3;
					return p+6;
				}
			tokenval = pos_in_symtable(symtable[lastentry].token);
			is_id = 1;
			return 0;
		case NUM:
			nNUM++;
			return 1;
		case ';':
			return 2;
		case '=':
			return 3;
		case '/':
			return 4;
		case ',':
			return 5;
	}
	return 10;
}

void open_file(TextFile &tf, char file_name[] = "relations.txt", char num_call = 1)
{
	if((tf = fopen(file_name, "rt")) == NULL)
	{
		printf("�� 㤠���� ������ 䠩� %s\n", file_name);
		if(num_call)fclose(in_f);
		getch();
		exit(5);
	}
}

// �����頥� �⭮����� ����� �ନ������ a (�� ���設� �⥪�; ����� ��ப�)
// � b (ᨬ���, �� ����� 㪠�뢠�� ip; ����� �⮫��)
char get_relation(char a, char b)
{
	fseek(relat, a*COUNT_TERM+b, SEEK_SET);
	char t = fgetc(relat);
	if(t == ' ' || t == '<' || t == '>' || t == '=')return t;
	printf("�訡�� � 䠩�� relations.txt.");
	my_exit(6);
}

void syntax_error() // �뢮� ᮮ�饭�� �� �訡��
{
	printf("���⠪��᪠� �訡�� � ��ப� ����� %d", lineno);
	my_exit(8);
}

void parser() // ���⠪��᪨� ���������
{
	int lex = lexan();
	while(1)
	{
		char b = convert(lex);
		if(b == 10 && lex != DONE)
		{
			if(op_3par)
				if(nNUM != 3)syntax_error();
			op_3par = nNUM = 0;
			if(is_id)
				if(!symtable[tokenval].flag_def)
				{
					printf("�訡�� (%d): ��ꥪ� %s �� ��।����.", lineno, symtable[tokenval].token);
					my_exit(9);
				}
			is_id = 0;
			lex = lexan();
		}
		char r = get_relation(Current -> a, b);
		if(b == 10 && Current -> a == 10)break;
		if(r == '<' || r == '=')
		{
			add_in_stack(b);
			lex = lexan();
			if(lex == DONE)break;
		}
		else if(r == '>')
		{
			char elem, was_pair = 0, one_id = Current -> a == 0, iter =0;
			do
			{
				elem = remove_from_stack();
				if(Current -> a == 8 && was_pair)syntax_error();
				if(!was_pair)was_pair = Current -> a == 4 && !elem;
				iter++;
			}while(get_relation(Current -> a, elem) != '<');
			if(iter == 1 && one_id)syntax_error();
		}
		else syntax_error();
	}
	if(Current -> a != 10)syntax_error();
}

void main(int argc, char *argv[])
{
	if(GetVersion() & 0x80000000)
		SetConsoleTitle("������� ������������ �������������� ������ � ���������� ������� � ���");
	else
		SetConsoleTitle("���⥬� ���஢���� ᨭ⠪��᪨� �訡�� � �ணࠬ��� �⠭��� � ���");
	if(argc == 1)
	{
		printf("Usage: test_err <chpu_prog_file>.\n");
		getch();
		exit(7);
	}
	open_file(in_f, argv[1], 0);
	open_file(relat);
	init_stack();
	parser();
	printf("�ணࠬ�� �� ᮤ�ন� �訡��.");
	free_resources();
}