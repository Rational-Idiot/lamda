#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *xstrdup(const char *s) {
  size_t n = strlen(s) + 1;
  char *p = malloc(n);
  if (!p) {
    perror("malloc");
    exit(1);
  }
  memcpy(p, s, n);
  return p;
}

typedef enum { TY_BOOL, TY_NAT, TY_ARROW } TypeKind;

typedef struct Type {
  TypeKind kind;
  struct Type *from;
  struct Type *to;
} Type;

Type *type_bool(void) {
  Type *t = malloc(sizeof(Type));
  t->kind = TY_BOOL;
  t->from = t->to = NULL;
  return t;
}

Type *type_nat(void) {
  Type *t = malloc(sizeof(Type));
  t->kind = TY_NAT;
  t->from = t->to = NULL;
  return t;
}

Type *type_arrow(Type *from, Type *to) {
  Type *t = malloc(sizeof(Type));
  t->kind = TY_ARROW;
  t->from = from;
  t->to = to;
  return t;
}

Type *type_clone(Type *t) {
  if (!t)
    return NULL;
  if (t->kind == TY_BOOL)
    return type_bool();
  if (t->kind == TY_NAT)
    return type_nat();
  return type_arrow(type_clone(t->from), type_clone(t->to));
}

int type_equal(Type *a, Type *b) {
  if (a == b)
    return 1;
  if (!a || !b || a->kind != b->kind)
    return 0;
  if (a->kind == TY_ARROW)
    return type_equal(a->from, b->from) && type_equal(a->to, b->to);
  return 1;
}

void print_type(Type *t) {
  if (!t)
    return;
  switch (t->kind) {
  case TY_BOOL:
    printf("Bool");
    break;
  case TY_NAT:
    printf("Nat");
    break;
  case TY_ARROW:
    printf("(");
    print_type(t->from);
    printf(" -> ");
    print_type(t->to);
    printf(")");
    break;
  }
}

typedef enum { VAR, LAMBDA, APP } NodeType;

typedef struct Node {
  NodeType type;
  char *var;
  Type *param_type;
  struct Node *left;
  struct Node *right;
} Node;

Node *new_var(const char *name) {
  Node *n = malloc(sizeof(Node));
  n->type = VAR;
  n->var = xstrdup(name);
  n->param_type = NULL;
  n->left = n->right = NULL;
  return n;
}

Node *new_lambda(const char *param, Type *param_type, Node *body) {
  Node *n = malloc(sizeof(Node));
  n->type = LAMBDA;
  n->var = xstrdup(param);
  n->param_type = type_clone(param_type);
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
  n->param_type = NULL;
  return n;
}

typedef enum {
  T_VAR,
  T_LAMBDA,
  T_DOT,
  T_COLON,
  T_LPAREN,
  T_RPAREN,
  T_ARROW,
  T_BOOL,
  T_NAT,
  T_EOF
} TokenType;

typedef struct {
  TokenType type;
  char text[64];
} Token;

char *src;
Token current;

void die(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

void skip_ws(void) {
  while (*src && isspace((unsigned char)*src))
    src++;
}

void next_token(void) {
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

  if (*src == ':') {
    current.type = T_COLON;
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

  if (*src == '-' && src[1] == '>') {
    current.type = T_ARROW;
    src += 2;
    return;
  }

  if (isalpha((unsigned char)*src)) {
    int i = 0;
    while (isalnum((unsigned char)*src) || *src == '_') {
      if (i < 63)
        current.text[i++] = *src;
      src++;
    }
    current.text[i] = '\0';

    if (strcmp(current.text, "Bool") == 0) {
      current.type = T_BOOL;
      return;
    }
    if (strcmp(current.text, "Nat") == 0) {
      current.type = T_NAT;
      return;
    }

    current.type = T_VAR;
    return;
  }

  fprintf(stderr, "Unexpected char: %c\n", *src);
  exit(1);
}

Type *parse_type(void);
Node *parse_expr(void);

Type *parse_type_atom(void) {
  if (current.type == T_BOOL) {
    next_token();
    return type_bool();
  }

  if (current.type == T_NAT) {
    next_token();
    return type_nat();
  }

  if (current.type == T_LPAREN) {
    next_token();
    Type *t = parse_type();
    if (current.type != T_RPAREN)
      die("Expected ) in type");
    next_token();
    return t;
  }

  die("Expected a type");
  return NULL;
}

Type *parse_type(void) {
  Type *left = parse_type_atom();
  if (current.type == T_ARROW) {
    next_token();
    Type *right = parse_type();
    return type_arrow(left, right);
  }
  return left;
}

Node *parse_atom(void) {
  if (current.type == T_VAR) {
    Node *v = new_var(current.text);
    next_token();
    return v;
  }

  if (current.type == T_LAMBDA) {
    next_token();

    if (current.type != T_VAR)
      die("Expected variable after lambda");

    char param[64];
    strcpy(param, current.text);
    next_token();

    if (current.type != T_COLON)
      die("Expected : after parameter");
    next_token();

    Type *param_type = parse_type();

    if (current.type != T_DOT)
      die("Expected . after annotated parameter");
    next_token();

    Node *body = parse_expr();
    return new_lambda(param, param_type, body);
  }

  if (current.type == T_LPAREN) {
    next_token();
    Node *e = parse_expr();
    if (current.type != T_RPAREN)
      die("Expected )");
    next_token();
    return e;
  }

  die("Parse error");
  return NULL;
}

Node *parse_application(void) {
  Node *left = parse_atom();

  while (current.type == T_VAR || current.type == T_LAMBDA ||
         current.type == T_LPAREN) {
    Node *right = parse_atom();
    left = new_app(left, right);
  }

  return left;
}

Node *parse_expr(void) { return parse_application(); }

void print_term(Node *n) {
  if (!n)
    return;

  switch (n->type) {
  case VAR:
    printf("%s", n->var);
    break;
  case LAMBDA:
    printf("(\\%s:", n->var);
    print_type(n->param_type);
    printf(".");
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
    return new_lambda(n->var, n->param_type, copy(n->left));

  return new_app(copy(n->left), copy(n->right));
}

int gensym_id = 0;

char *fresh_var(void) {
  char buf[64];
  snprintf(buf, sizeof(buf), "v%d", gensym_id++);
  return xstrdup(buf);
}

int occurs_free(Node *t, const char *x) {
  if (!t)
    return 0;

  if (t->type == VAR)
    return strcmp(t->var, x) == 0;

  if (t->type == APP)
    return occurs_free(t->left, x) || occurs_free(t->right, x);

  if (strcmp(t->var, x) == 0)
    return 0;

  return occurs_free(t->left, x);
}

void rename_bound(Node *t, const char *oldname, const char *newname) {
  if (!t)
    return;

  if (t->type == VAR) {
    if (strcmp(t->var, oldname) == 0) {
      free(t->var);
      t->var = xstrdup(newname);
    }
    return;
  }

  if (t->type == APP) {
    rename_bound(t->left, oldname, newname);
    rename_bound(t->right, oldname, newname);
    return;
  }

  if (strcmp(t->var, oldname) == 0)
    return; // shadowing binder

  rename_bound(t->left, oldname, newname);
}

Node *substitute(Node *t, const char *x, Node *v) {
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

  if (strcmp(t->var, x) == 0)
    return t;

  if (occurs_free(v, t->var)) {
    char *fresh = fresh_var();
    rename_bound(t->left, t->var, fresh);
    free(t->var);
    t->var = fresh;
  }

  t->left = substitute(t->left, x, v);
  return t;
}

typedef struct Env {
  char *name;
  Type *type;
  struct Env *next;
} Env;

Env *env_push(Env *env, const char *name, Type *type) {
  Env *e = malloc(sizeof(Env));
  e->name = xstrdup(name);
  e->type = type;
  e->next = env;
  return e;
}

Type *env_lookup(Env *env, const char *name) {
  for (; env; env = env->next) {
    if (strcmp(env->name, name) == 0)
      return env->type;
  }
  return NULL;
}

Type *type_check(Node *t, Env *env) {
  if (t->type == VAR) {
    if (strcmp(t->var, "true") == 0 || strcmp(t->var, "false") == 0)
      return type_bool();

    Type *ty = env_lookup(env, t->var);
    if (!ty) {
      fprintf(stderr, "Unbound variable: %s\n", t->var);
      exit(1);
    }
    return type_clone(ty);
  }

  if (t->type == LAMBDA) {
    Env *env2 = env_push(env, t->var, t->param_type);
    Type *body_ty = type_check(t->left, env2);
    return type_arrow(type_clone(t->param_type), body_ty);
  }

  Type *fun_ty = type_check(t->left, env);
  Type *arg_ty = type_check(t->right, env);

  if (fun_ty->kind != TY_ARROW) {
    fprintf(stderr, "Attempted to apply a non-function term\n");
    exit(1);
  }

  if (!type_equal(fun_ty->from, arg_ty)) {
    fprintf(stderr, "Type mismatch in application\n");
    printf("  function expects: ");
    print_type(fun_ty->from);
    printf("\n  but got:          ");
    print_type(arg_ty);
    printf("\n");
    exit(1);
  }

  return type_clone(fun_ty->to);
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
    if (*changed)
      return t;

    t->right = reduce(t->right, changed);
    return t;
  }

  if (t->type == LAMBDA) {
    t->left = reduce(t->left, changed);
    return t;
  }

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

int main(void) {
  char input[2048];

  printf("STLC> ");
  if (!fgets(input, sizeof(input), stdin))
    return 0;

  src = input;
  next_token();

  Node *term = parse_expr();

  if (current.type != T_EOF) {
    fprintf(stderr, "Unexpected trailing input\n");
    return 1;
  }

  printf("\nParsed: ");
  print_term(term);
  printf("\n");

  Type *ty = type_check(term, NULL);
  printf("Type: ");
  print_type(ty);
  printf("\n\nReduction:\n");

  Node *result = normalize(term);

  printf("\nNormal form: ");
  print_term(result);
  printf("\nType preserved: ");
  print_type(ty);
  printf("\n");

  return 0;
}
