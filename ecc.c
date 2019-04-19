#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN 255

// Token type
enum {
  TK_NUM = 256, // Integer token
  TK_EOF,       // End of token
};

//Token struct
typedef struct {
  int ty;      // Token type
  int val;     // Token value (integer)
  char *input; // Token character (for error)
} Token;

// Array for tokenized token
Token tokens[MAX_TOKEN];

// Separate p(char) with space and set to tokens
void tokenize(char *p) {
  int i = 0;
  while (*p) {
    // Skip space
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    fprintf(stderr, "Failed to tokenize: %s\n", p);
    exit(1);
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

// Output error and exit program
void error(int i) {
  fprintf(stderr, "Invalid token: %s\n",
          tokens[i].input);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid args number\n");
    return 1;
  }

  tokenize(argv[1]);

  // Output first half assembly
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // First character must be digit
  if (tokens[0].ty != TK_NUM) error(0);
  printf("  mov rax, %d\n", tokens[0].val);

  // Token must be `+ <digit>` or `- <digit>`
  // Output assembly
  int i = 1;
  while (tokens[i].ty != TK_EOF) {
    if (tokens[i].ty == '+') {
      i++;
      if (tokens[i].ty != TK_NUM)
        error(i);
      printf("  add rax, %d\n", tokens[i].val);
      i++;
      continue;
    }

    if (tokens[i].ty == '-') {
      i++;
      if (tokens[i].ty != TK_NUM)
        error(i);
      printf("  sub rax, %d\n", tokens[i].val);
      i++;
      continue;
    }

    error(i);
  }

  printf("  ret\n");
  return 0;
}