#include <ctype.h>
#include <stdarg.h>
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

//Token position
int pos = 0;

enum {
  ND_NUM = 256,     // Type of digit nodes
};

typedef struct Node {
  int ty;           // Operator or ND_NUM
  struct Node *lhs; // Left-hand side
  struct Node *rhs; // Right-hand side
  int val;          // Use if ty is ND_NUM
} Node;

Node *term();
Node *mul();
Node *add();
Node *unary();
void error(char*, ...);
void gen(Node *node);

// Create new node
Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// Create new node (digit)
Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

int consume(int ty) {
  if (tokens[pos].ty != ty)
    return 0; //FALSE
  pos++;
  return 1; //TRUE
}

// term is "(" add ")" or num
Node *term() {
  // If next token is '(', "(" add ")" is expected
  if (consume('(')) {
    Node *node = add();
    if (!consume(')'))
      error("Second parenthesis is required: %s",
            tokens[pos].input);
    return node;
  }

  // If next token is not '(', it should be num
  if (tokens[pos].ty == TK_NUM)
    return new_node_num(tokens[pos++].val);

  error("Token is neither num nor parenthesis: %s",
        tokens[pos].input);
}

// Left-hand operator "*" or "/"
Node *mul() {
  Node *node = unary();

  while (1) {
    if (consume('*'))
      node = new_node('*', node, unary());
    else if (consume('/'))
      node = new_node('/', node, unary());
    else
      return node;
  }
}

// Left-hand operator "+" or "-"
Node *add() {
  Node *node = mul();

  while (1) {
    if (consume('+'))
      node = new_node('+', node, mul());
    else if (consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

Node *unary() {
  if (consume('+'))
    return term();
  if (consume('-'))
    return new_node('-', new_node_num(0), term());
  return term();
}

// Separate p(char) with space and set to tokens
void tokenize(char *p) {
  int i = 0;
  while (*p) {
    // Skip space
    if (isspace(*p)) {
      p++;
      continue;
    }

//    if (*p == '+' || *p == '-') {
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' ) {
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
// Same args as "printf"
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
// void error(int i) {
//   fprintf(stderr, "Invalid token: %s\n",
//           tokens[i].input);
//   exit(1);
// }

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  mul rdi\n");
    break;
  case '/':
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
  }

  printf("  push rax\n");
}

//main function
int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid args number\n");
    return 1;
  }

  //Tokenize
  tokenize(argv[1]);
  
  //Parse
  Node *node = add();

  // Output first half assembly
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // Generate code going down abstract syntax tree
  gen(node);

/*
  // First character must be digit
  if (tokens[0].ty != TK_NUM) error("First character must be digit");
  printf("  mov rax, %d\n", tokens[0].val);

  // Token must be `+ <digit>` or `- <digit>`
  // Output assembly
  int i = 1;
  while (tokens[i].ty != TK_EOF) {
    if (tokens[i].ty == '+') {
      i++;
      if (tokens[i].ty != TK_NUM) error("Unexpected token: %s", tokens[i].input);
      printf("  add rax, %d\n", tokens[i].val);
      i++;
      continue;
    }

    if (tokens[i].ty == '-') {
      i++;
      if (tokens[i].ty != TK_NUM) error("Unexpected token: %s", tokens[i].input);
      printf("  sub rax, %d\n", tokens[i].val);
      i++;
      continue;
    }

    error("Unexpected token: %s", tokens[i].input);
  }
*/
  
  // Load the top of stack (it should be the result) to RAX
  // And it return from function
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}