#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { VAR, LAMBDA, APP } NodeType;

typedef struct Node {
  NodeType type;
  char *var;
  struct Node *left;
  struct Node *right;
} Node;

Node *new_var(char *name) {
  Node *n = malloc(sizeof(Node));
  n->type = VAR;
  n->var = strdup(name);
  n->left = n->right = NULL;
  return n;
}

Node *new_lambda(char *param, Node *body) {
  Node *n = malloc(sizeof(Node));
  n->type = LAMBDA;
  n->var = strdup(param);
  n->left = body;
  n->right = NULL;
  return n;
}

Node *new_app(Node *f, Node *a) {
  Node *n = malloc(sizeof(Node));
  n->type = APP;
  n->left = f;
  n->right = a;
  n->var = NULL;
  return n;
}

typedef enum { T_VAR, T_LAMBDA, T_DOT, T_LPAREN, T_RPAREN, T_EOF } TokenType;

typedef struct {
  TokenType type;
  char text[64];
} Token;

char *src;
Token current;

void skip_ws() {
  while (*src && isspace(*src))
    src++;
}

void next_token() {
  skip_ws();

  if (*src == '\0') {
    current.type = T_EOF;
    return;
  }

  if (*src == '\\') {
    current.type = T_LAMBDA;
    src++;
    return;
  }

  if (*src == '.') {
    current.type = T_DOT;
    src++;
    return;
  }

  if (*src == '(') {
    current.type = T_LPAREN;
    src++;
    return;
  }

  if (*src == ')') {
    current.type = T_RPAREN;
    src++;
    return;
  }

  if (isalpha(*src)) {
    int i = 0;
    while (isalnum(*src)) {
      current.text[i++] = *src++;
    }
    current.text[i] = '\0';
    current.type = T_VAR;
    return;
  }

  printf("Unexpected char: %c\n", *src);
  exit(1);
}

Node *parse_expr();

Node *parse_atom() {

  if (current.type == T_VAR) {
    Node *v = new_var(current.text);
    next_token();
    return v;
  }

  if (current.type == T_LAMBDA) {

    next_token();

    if (current.type != T_VAR) {
      printf("Expected variable after lambda\n");
      exit(1);
    }

    char param[64];
    strcpy(param, current.text);
    next_token();

    if (current.type != T_DOT) {
      printf("Expected .\n");
      exit(1);
    }

    next_token();

    Node *body = parse_expr();
    return new_lambda(param, body);
  }

  if (current.type == T_LPAREN) {

    next_token();
    Node *e = parse_expr();

    if (current.type != T_RPAREN) {
      printf("Expected )\n");
      exit(1);
    }

    next_token();
    return e;
  }

  printf("Parse error\n");
  exit(1);
}

Node *parse_application() {

  Node *left = parse_atom();

  while (current.type == T_VAR || current.type == T_LAMBDA ||
         current.type == T_LPAREN) {

    Node *right = parse_atom();
    left = new_app(left, right);
  }

  return left;
}

Node *parse_expr() { return parse_application(); }

void print_term(Node *n) {

  if (!n)
    return;

  switch (n->type) {

  case VAR:
    printf("%s", n->var);
    break;

  case LAMBDA:
    printf("(\\%s.", n->var);
    print_term(n->left);
    printf(")");
    break;

  case APP:
    printf("(");
    print_term(n->left);
    printf(" ");
    print_term(n->right);
    printf(")");
    break;
  }
}

Node *copy(Node *n) {

  if (!n)
    return NULL;

  if (n->type == VAR)
    return new_var(n->var);

  if (n->type == LAMBDA)
    return new_lambda(n->var, copy(n->left));

  return new_app(copy(n->left), copy(n->right));
}

int gensym_id = 0;

char *fresh_var() {
  static char buf[64];
  sprintf(buf, "v%d", gensym_id++);
  return strdup(buf);
}

Node *substitute(Node *t, char *x, Node *v) {

  if (t->type == VAR) {
    if (strcmp(t->var, x) == 0)
      return copy(v);
    return t;
  }

  if (t->type == APP) {
    t->left = substitute(t->left, x, v);
    t->right = substitute(t->right, x, v);
    return t;
  }

  if (t->type == LAMBDA) {

    if (strcmp(t->var, x) == 0)
      return t;

    t->left = substitute(t->left, x, v);
    return t;
  }

  return t;
}

Node *reduce(Node *t, int *changed) {

  if (t->type == APP) {

    if (t->left->type == LAMBDA) {

      Node *body = t->left->left;
      char *param = t->left->var;
      Node *arg = t->right;

      *changed = 1;
      return substitute(copy(body), param, arg);
    }

    t->left = reduce(t->left, changed);
    t->right = reduce(t->right, changed);
  }

  if (t->type == LAMBDA)
    t->left = reduce(t->left, changed);

  return t;
}

Node *normalize(Node *t) {

  while (1) {

    int changed = 0;

    t = reduce(t, &changed);

    if (!changed)
      break;

    printf(" -> ");
    print_term(t);
    printf("\n");
  }

  return t;
}

int main() {

  char input[2048];

  printf("Lambda> ");
  fgets(input, sizeof(input), stdin);

  src = input;
  next_token();

  Node *term = parse_expr();

  printf("\nParsed: ");
  print_term(term);
  printf("\n\nReduction:\n");

  Node *result = normalize(term);

  printf("\nNormal form: ");
  print_term(result);
  printf("\n");

  return 0;
}
