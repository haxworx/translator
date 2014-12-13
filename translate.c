#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void Failure(char *fmt, ...)
{
	char buf[8192] = { 0 };
	va_list ap;

	va_start(ap, fmt);
	sprintf(buf, "ERR: %s\n", fmt);
	fprintf(stderr, buf);
	va_end(ap);

	exit(EXIT_FAILURE);
}

typedef struct flags_t flags_t;
struct flags_t {
	int end_instruction;
	int is_block;
	int top_block;
};

typedef struct token_t token_t;
struct token_t {
	char token[128];
	int type;
	flags_t flags;
	token_t *block;
	token_t *next;
};

#define TK_OP_ADD 1
#define TK_OP_DEL 2
#define TK_OP_MUL 3
#define TK_OP_DIV 4
#define TK_OP_ASG 5
#define TK_OP_STR 6

#define TK_VAR_VAL 7 
#define TK_VAR_NAM 8
#define TK_OP_PWR 9

int type_from_token(char *token)
{
	char *t = token;
	
	switch (*t) {
		case '+':
			return TK_OP_ADD;
			break;
		case '-':
			return TK_OP_DEL;
			break;
		case '*':
			return TK_OP_MUL;
			break;
		case '/':
			return TK_OP_DIV;
			break;
		case '=':
			return TK_OP_ASG;
			break;
		case '^':
			return TK_OP_PWR;
			break;
		
	}	

	if (*t == '"' && t[strlen(t) - 1] == '"')
		return TK_OP_STR;


	if (isdigit(*t))
		return TK_VAR_VAL;
	else
		return TK_VAR_NAM;

	return 0; 
}

token_t *AddToken(token_t *tokens, char *token, flags_t *flags)
{
	token_t *c = tokens;
	
	int type = type_from_token(token); 

	if (c == NULL) {
		c = calloc(1, sizeof(token_t));
		strcpy(c->token, token);
		c->type = type; //
		c->flags.end_instruction = flags->end_instruction;
		c->flags.is_block = flags->is_block;
		return c;
	}

	for (; c->next; c = c->next);

	if (c->next == NULL) {
		c->next = calloc(1, sizeof(token_t));
		c = c->next;
		strcpy(c->token, token);
		c->type = type; // 
		c->flags.end_instruction = flags->end_instruction;
		c->flags.is_block = flags->is_block;
		return c;
	}

	return NULL; // wtf?
}


void TokensList(token_t *tokens)
{	
	token_t *c = tokens->next;

	while (c) {
		if (c->flags.is_block)
		printf("\t\t");
		printf("%s %d %d %d %d\n", c->token, c->type, c->flags.end_instruction, c->flags.is_block, c->flags.top_block);
		token_t *b = c->block;
		while (b) {
			printf("\t%s %d %d %d\n", b->token, b->type, b->flags.end_instruction, b->flags.is_block, c->flags.top_block);
			b = b->next;
		}
		c = c->next;
	}
}

token_t *Tokenize(char *file, ssize_t length)
{
	char *map = calloc(1, 1 + length);

	token_t *tokens = calloc(1, sizeof(token_t));

	FILE *f = fopen(file, "r");		
	if (!f)
		Failure("input file issue");

	fread(map, length, 1, f);	

	fclose(f);

	char *m = map;
	char *end = m + length;
	printf("end is %p\n", end);
	while (*m) {
		char *start = m;
		int end_instruction = 0;

		flags_t flags;
		memset(&flags, 0, sizeof(flags));
	
		while (*m != ',' && *m != ' ' && *m != '\r') {
			if (*m == '\t') {
				m++;
				while (*m != '\n')
					++m;
			//	m++;
				flags.is_block = 1;
				break;
			} 
			if (*m == '\n') {
				end_instruction = 1;
				break;
			} else if (*m == '"') {
				m++; // onwards christian soldier!
				while (*m != '"') {
					++m;
				}
				m++;
			} else
				m++;
		}

		*m = '\0'; 
		m++;

		if (end_instruction) {
			flags.end_instruction = end_instruction;
			if (!strncmp(start, "sub", 3) || !strncmp(start, "if", 2) 
				|| !strncmp(start, "while", 5) || strncmp(start, "loop", 4))
				flags.is_block = 0;
		

		}
		token_t *here_i_am = AddToken(tokens, start, &flags);

		while (*m == ' ' ||  *m == '\r' || *m == '\n') {
			m++;
		}			
	}

	return tokens;
}

int main(void)
{
	token_t *tokens = calloc(1, sizeof(token_t)); // our tokens

	char *infile = "test.al";

	struct stat fstats;

	if (stat(infile, &fstats))
		Failure("input file issue");

	ssize_t length = fstats.st_size;

	tokens = Tokenize(infile, length);	

	TokensList(tokens);

	return 0;
}
