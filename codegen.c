/*
  $Id: codegen-skel.c,v 1.3 2009/03/27 09:36:54 gondow Exp gondow $
 */
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "xcc.h"
#include "misc.h"
#include "AST.h"
#include "type.h"
#include "symbol.h"
#include "codegen.h"

static void emit_code (struct AST *ast, char *fmt, ...);
static void codegen_begin_block (struct AST *ast);
static void codegen_end_block (void);
static void codegen_begin_function (struct AST *ast);
static void codegen_end_function (void);

static int frame_height;	/* スタックフレームの大きさ */
static char *funcname;		/* コンパイル中の関数名 */

static int label_num; /* 次に使用するラベル番号を管理 */

static void codegen_global (void);
static void codegen_global_func (struct Symbol *sym);
static void codegen_global_var (struct Symbol *sym);
static void visit_AST (struct AST *ast);
static void codegen_function_definition (struct AST *ast);
static void codegen_statement_exp (struct AST *ast);
static void codegen_compound_statement (struct AST *ast);
static void codegen_statement_if (struct AST *ast);
static void codegen_statement_ifelse (struct AST *ast);
static void codegen_statement_while (struct AST *ast);
static void codegen_statement_return (struct AST *ast);
static void codegen_expression_id (struct AST *ast);
static void codegen_expression_intchar (struct AST *ast);
static void codegen_expression_string (struct AST *ast);
static void codegen_expression_funcall (struct AST *ast);
static void codegen_expression_assign (struct AST *ast);
static void codegen_left_value(struct AST *ast);
static void codegen_expression_lor (struct AST *ast);
static void codegen_expression_land (struct AST *ast);
static void codegen_expression_eq (struct AST *ast);
static void codegen_expression_less (struct AST *ast);
static void codegen_expression_add (struct AST *ast);
static void codegen_expression_sub (struct AST *ast);
static void codegen_expression_mul (struct AST *ast);
static void codegen_expression_div (struct AST *ast);
static void codegen_expression_unary (struct AST *ast);
static void codegen_argument_expression_list_pair (struct AST *ast);

/* ---------------------------------------------------------------------- */

static void
emit_code (struct AST *ast, char *fmt, ...)
{
  va_list  argp;
  va_start (argp, fmt);
  vfprintf (xcc_out, fmt, argp);
  va_end   (argp);
  
  /* the argument 'ast' can be used for debug purpose */
}

static void
codegen_begin_block (struct AST *ast)
{
  assert (!strcmp (ast->ast_type, "AST_compound_statement"));
  sym_table.local [++sym_table.local_index] = ast->u.local;
}

static void
codegen_end_block (void)
{
  sym_table.local_index--;
}

static void
codegen_begin_function (struct AST *ast)
{
  assert(!strcmp (ast->ast_type, "AST_function_definition"));
  sym_table.local_index = -1;
  sym_table.global = ast->u.func.global;
  sym_table.arg    = ast->u.func.arg;
  sym_table.label  = ast->u.func.label;
  sym_table.string = ast->u.func.string;
}

static void
codegen_end_function (void)
{
  /* do nothing */
}

/* ---------------------------------------------------------------------- */
void
codegen (void)
{
  emit_code (ast_root, "\t.file   \"%s\"\n", filename);
  
  codegen_global ();
  
  emit_code (ast_root, "\t.ident  \"XCC skeleton ($Revision: 1.3 $)\"\n");
}
/* ---------------------------------------------------------------------- */

static void
codegen_global (void)
{
  struct Symbol *sym;

  sym = sym_table.global;
  while (sym != NULL) {
    if (sym->type->kind == TYPE_KIND_FUNCTION) {
      codegen_global_func (sym);
    }
		else {
			codegen_global_var (sym);
		}
    sym = sym->next;
  };
}

static void
codegen_global_func (struct Symbol *sym)
{
  assert (!strcmp (sym->ast->ast_type, "AST_function_definition")
          || !strcmp (sym->ast->ast_type, "AST_declaration"));
  
  if (!strcmp (sym->ast->ast_type, "AST_function_definition")) {
    funcname = sym->name;
    codegen_function_definition (sym->ast);
  };
}

/* 大域変数の領域確保 */
static void
codegen_global_var (struct Symbol *sym)
{
	emit_code(sym->ast, "\t.comm\t_%s,4,2\n", sym->name); 
}
static void
visit_AST (struct AST *ast)
{
  int i;
  
  if (!strcmp (ast->ast_type, "AST_statement_exp")) {
    codegen_statement_exp (ast);
  } else if (!strcmp (ast->ast_type, "AST_compound_statement")) {
    codegen_compound_statement (ast);
  } else if (!strcmp (ast->ast_type, "AST_statement_if")) {
    codegen_statement_if (ast);
  } else if (!strcmp (ast->ast_type, "AST_statement_ifelse")) {
    codegen_statement_ifelse (ast);
  } else if (!strcmp (ast->ast_type, "AST_statement_while")) {
    codegen_statement_while (ast);
  } else if (!strcmp (ast->ast_type, "AST_statement_return")) {
    codegen_statement_return (ast);
  } else if (!strcmp (ast->ast_type, "AST_expression_int")
             ||!strcmp (ast->ast_type, "AST_expression_char")) {
    codegen_expression_intchar (ast);
  } else if (!strcmp (ast->ast_type, "AST_expression_string")) {
    codegen_expression_string (ast);
  } else if (!strcmp (ast->ast_type, "AST_expression_id")) {
    codegen_expression_id (ast);
  } else if (!strcmp (ast->ast_type, "AST_expression_funcall1")
             || !strcmp (ast->ast_type, "AST_expression_funcall2")) {
    codegen_expression_funcall (ast);
  } else if (!strcmp (ast->ast_type, "AST_expression_lor")){
    codegen_expression_lor (ast);
  } else if (!strcmp (ast->ast_type, "AST_expression_land")){
    codegen_expression_land (ast);
  } else if (!strcmp (ast->ast_type, "AST_expression_eq")){
    codegen_expression_eq (ast);
  } else if (!strcmp (ast->ast_type, "AST_expression_assign")) {
    codegen_expression_assign (ast);
  } else if (!strcmp (ast->ast_type, "AST_expression_less")) {
    codegen_expression_less (ast);
  } else if (!strcmp (ast->ast_type, "AST_expression_add")) {
    codegen_expression_add (ast);
  } else if (!strcmp (ast->ast_type, "AST_expression_sub")) {
    codegen_expression_sub (ast);
  } else if (!strcmp (ast->ast_type, "AST_expression_mul")) {
    codegen_expression_mul (ast);
  } else if (!strcmp (ast->ast_type, "AST_expression_div")) {
    codegen_expression_div (ast); 
  } else if (!strcmp (ast->ast_type, "AST_expression_unary")) {
    codegen_expression_unary (ast); 
  } else if (!strcmp (ast->ast_type, "AST_argument_expression_list_pair")) {
    codegen_argument_expression_list_pair (ast);
  } else {
    for (i = 0; i < ast->num_child; i++) {
      visit_AST (ast->child [i]);
    };
  };
}

static void
codegen_string_def (struct AST *ast, struct String *string)
{
  struct String *p;
  
  emit_code (ast, "\t.cstring\n");
  for (p = string; p != NULL; p = p->next) {
    emit_code (ast, "L%s:\n", p->label);
    emit_code (ast, "\t.ascii\t\"%s\\0\"\n", p->data);
  }
}

static void
codegen_function_definition (struct AST *ast)
{
  int i;
  int l_size = ast->u.func.total_local_size;

  assert (!strcmp (ast->ast_type, "AST_function_definition"));
  codegen_begin_function (ast);	/* 名前表の修正 */
  frame_height = 4;	/* 呼び出されたときは、%eipのみスタックにあるため、大きさは4 */
  if (sym_table.string != NULL) 
    codegen_string_def (ast, sym_table.string);
  emit_code (ast, "\t.text\n");
  emit_code (ast, "\t.globl\t_%s\n", funcname);
  emit_code (ast, "_%s:\n", funcname);
  emit_code (ast, "\tpushl\t%%ebp\t# スタックフレームを作成\n");
  frame_height += 4;	/* スタックに%ebpが積まれたため、大きさを4増やす */
  emit_code (ast, "\tmovl\t%%esp, %%ebp\n");

  /* 局所変数の領域確保 */
  if (l_size != 0){
    emit_code (ast, "\tsubl\t$%d, %%esp\t# 局所変数の領域を確保\n", l_size);
    frame_height += l_size;
  }
  
  /* 本体のコンパイル */
  for (i = 0; i < ast->num_child; i++) {
    visit_AST (ast->child [i]);
  };
  
  emit_code (ast, "L.XCC.RE.%s:\n", funcname);
  emit_code (ast, "\tmovl\t%%ebp, %%esp\t# スタックフレームを除去\n");
  emit_code (ast, "\tpopl\t%%ebp\n");
  emit_code (ast, "\tret\n");
  
  codegen_end_function (); /* 名前表の修正 */
}

static void
codegen_statement_exp (struct AST *ast)
{
  int i;
  
  assert (!strcmp (ast->ast_type, "AST_statement_exp"));

  /* 本体のコンパイル */
  for (i = 0; i < ast->num_child; i++) {
    visit_AST (ast->child [i]);
  };
  if (!strcmp (ast->child[0]->ast_type, "AST_expression_opt_single")){
    emit_code (ast, "\taddl\t$%d, %%esp\t# 返り値を廃棄\n", 4);
    frame_height -= 4;	/* スタックトップが廃棄されたので、4減らす。*/
  };
}

static void
codegen_compound_statement (struct AST *ast)
{
  int i;

  assert (!strcmp (ast->ast_type, "AST_compound_statement"));

  codegen_begin_block (ast);	/* 名前表の修正 */

  /* 本体のコンパイル */
  for (i = 0; i < ast->num_child; i++) {
    visit_AST (ast->child [i]);
  };

  codegen_end_block ();	/* 名前表の修正 */
}

static void
codegen_statement_if (struct AST *ast)
{
  int label_end   = label_num++; /* if文のコードの終わりのラベル番号*/

  assert (!strcmp (ast->ast_type, "AST_statement_if"));

  /* 条件式のコンパイル*/
  visit_AST(ast->child[0]);

  emit_code (ast, "\tpopl\t%%eax\t#式の値を%%eaxへ\n");
  emit_code (ast, "\tcmpl\t$0, %%eax\t#式の値=0?\n");
  emit_code (ast, "\tje\tL%d\t#0なら終わりまでジャンプ\n", label_end);
  frame_height -= 4;

  /* 実行文のコンパイル*/
  visit_AST(ast->child[1]);

  emit_code (ast, "L%d:\n", label_end);
}

static void
codegen_statement_ifelse (struct AST *ast)
{
  int label_else = label_num++; /* if-else文のelseのコードの始まり*/
  int label_end  = label_num++; /* if-else文のコードの終わり*/

  assert (!strcmp (ast->ast_type, "AST_statement_ifelse"));

  /* 条件式のコンパイル*/
  visit_AST(ast->child[0]);

  emit_code (ast, "\tpopl\t%%eax\t#式の値を%%eaxへ\n");
  emit_code (ast, "\tcmpl\t$0, %%eax\t#式の値=0?\n");
  emit_code (ast, "\tje\tL%d\t#0ならelse部にジャンプ\n", label_else);
  frame_height -= 4;

  /* 条件式がtrueのとき実行される文(if部)のコンパイル*/
  visit_AST(ast->child[1]);
  emit_code (ast, "\tjmp\tL%d\t#else部をスキップ\n", label_end);

  emit_code (ast, "L%d:\n", label_else); /* else部のラベル*/

  /* 条件式がfalseのとき実行される文(else部)のコンパイル*/
  visit_AST(ast->child[2]);

  emit_code (ast, "L%d:\n", label_end);
}

static void
codegen_statement_while (struct AST *ast)
{
  int label_begin = label_num++; /* while文のコードのはじめのラベル番号 */
  int label_end   = label_num++; /* while文のコードのおわりのラベル番号 */

  assert (!strcmp (ast->ast_type, "AST_statement_while"));

  emit_code (ast, "L%d:\n", label_begin);

  /* 条件式のコンパイル */
  visit_AST(ast->child[0]);

  emit_code (ast, "\tpopl\t%%eax\t#式の値を%%eaxへ\n");
  emit_code (ast, "\tcmpl\t$0, %%eax\t#式の値=0?\n");
  emit_code (ast, "\tje\tL%d\t#0ならループから出る\n", label_end);
  frame_height -= 4; /* 条件文の結果が破棄された */

  /* 実行文のコンパイル */
  visit_AST(ast->child[1]);

  emit_code (ast, "\tjmp\tL%d\t#条件判定へ戻る\n", label_begin);

  emit_code (ast, "L%d:\n", label_end);
}

static void
codegen_statement_return (struct AST *ast)
{
  assert (!strcmp (ast->ast_type, "AST_statement_return"));

  /* 返り値の式のコンパイル */
  visit_AST(ast->child[0]);

  /* 返り値がある場合は返り値を%eaxへ */
  if(!strcmp (ast->child[0]->ast_type, "AST_expression_opt_single")){
    emit_code (ast, "\tpopl\t%%eax\t#返り値を%%eaxへ\n");
    frame_height -= 4;
  }
  emit_code (ast, "\tjmp\tL.XCC.RE.%s\n", funcname);
}

static void
codegen_expression_id (struct AST *ast)
{
  char *id;
  struct Symbol *symbol;

  assert (!strcmp (ast->ast_type, "AST_expression_id"));

  id = ast->child[0]->u.id;
  symbol = sym_lookup (id);

  switch (symbol->name_space) {
  case NS_LOCAL:
    switch (symbol->type->kind) {
    case TYPE_KIND_PRIM:
      emit_code (ast, "\tpushl\t-%d(%%ebp)\n", symbol->offset + 4);
      frame_height += 4;	/* スタックに変数の値が積まれた */
      break;
    case TYPE_KIND_POINTER:
      emit_code (ast, "\tpushl\t-%d(%%ebp)\n", symbol->offset + 4);
      frame_height += 4;  /* スタックにポインタアドレス値が積まれた*/
      break;
    default:
      assert(0);
      break;
    }
    break;
  case NS_ARG:
    switch (symbol->type->kind) {
    case TYPE_KIND_PRIM:
      emit_code (ast, "\tpushl\t%d(%%ebp)\n", symbol->offset + 8);
      frame_height += 4;	/* スタックに変数の値が積まれた */
      break;
    case TYPE_KIND_POINTER:
      emit_code (ast, "\tpushl\t%d(%%ebp)\n", symbol->offset + 8);
      frame_height += 4;  /* スタックにポインタアドレス値が積まれた*/
      break;
    default:
      assert(0);
      break;
    }
    break;
  case NS_GLOBAL:
    switch (symbol->type->kind) {
    case TYPE_KIND_PRIM:
      emit_code (ast, "\tpushl\t_%s\n", id);
      frame_height += 4;	/* スタックに変数の値が積まれた */
      break;
    case TYPE_KIND_POINTER:
      emit_code (ast, "\tpushl\t_%s\n", id);
      frame_height += 4;  /* スタックに変数の値が積まれた */
      break;
    case TYPE_KIND_FUNCTION:
      emit_code (ast, "\tcall\t_%s\n", id);
      break;
    default:
      assert (0);
      break;
    };
    break;
  case NS_LABEL:
    break;
  default:
    assert (0);
    break;
  };
}

static void
codegen_expression_intchar (struct AST *ast)
{
  assert (!strcmp (ast->ast_type, "AST_expression_int")
          || !strcmp (ast->ast_type, "AST_expression_char"));
  
  emit_code (ast, "\tpushl\t$%d\n", ast->u.int_val);
  frame_height += 4;	/* スタックに即値が積まれた */
}

static void
codegen_expression_string (struct AST *ast)
{
  struct String *string;

  assert (!strcmp (ast->ast_type, "AST_expression_string"));
  
  string = string_lookup(ast->u.id);
  emit_code (ast, "\tpushl\t$L%s\n", string->label);
  frame_height += 4;	/* スタックにstringのあるアドレスが積まれた */
}

static void
codegen_expression_funcall (struct AST *ast)
{
  int i;
  int fh;
  int padding; /* 呼び出し時%espが16バイト境界のため、挿入するpaddingの大きさ */

  assert (!strcmp (ast->ast_type, "AST_expression_funcall1")
          || !strcmp (ast->ast_type, "AST_expression_funcall2"));

  /*  挿入するpaddingの大きさの計算し、paddingを積む */
  if (!strcmp (ast->ast_type, "AST_expression_funcall1")) {
    fh = (frame_height + ast->child[1]->u.arg_size) % 16;
  } else {
    fh = frame_height % 16;
  };
  if (fh == 0) {
    padding = 0;
  } else {
    padding = 16 - fh;
  };
  if (padding != 0) {
    emit_code (ast, "\tsubl\t$%d, %%esp\t# paddingを積む\n", padding);
    frame_height += padding;
  };
  
  /* 引数の値を積んでから、関数呼び出し */
  for (i = ast->num_child - 1; i >= 0 ; i--) {
    visit_AST (ast->child [i]);
  };
  
  /* 引数とpaddingを捨てる */
  if (!strcmp (ast->ast_type, "AST_expression_funcall1") 
      && ast->child[1]->u.arg_size != 0) {
    emit_code (ast, "\taddl\t$%d, %%esp\t# 引数を捨てる\n", ast->child[1]->u.arg_size); 
    frame_height -= ast->child[1]->u.arg_size;
  };
  if (padding != 0) {
    emit_code (ast, "\taddl\t$%d, %%esp\t# paddingを廃棄\n", padding); 
    frame_height -= padding;
  };
  
  /* 返り値をスタックに乗せる */
  emit_code (ast, "\tpushl\t%%eax\t# 返り値を積む\n");
  frame_height += 4;
}

static void
codegen_expression_assign (struct AST *ast)
{
  assert(!strcmp (ast->ast_type, "AST_expression_assign"));

  /* 右辺値をスタックに積む */
  visit_AST (ast->child[1]);
  
  /* 左辺値のアドレスをスタックに積む */
  if(!strcmp(ast->child[0]->ast_type, "AST_expression_id")){ /* 左辺が変数 */
    codegen_left_value(ast->child[0]);
  } 
  else{ /* 左辺が変数でない: *(アドレス) */
    /* &ptrはコンパイルエラーに */
    assert(strcmp(ast->child[0]->child[0]->ast_type, "AST_unary_operator_address"));
    visit_AST(ast->child[0]->child[1]);
    frame_height -= 4;
  }
  /* 代入 連続代入式のために右辺値はスタックトップに残す */
  emit_code (ast, "\tpopl\t%%eax\n");
  emit_code (ast, "\tmovl\t0(%%esp),%%ecx\n");
  emit_code (ast, "\tmovl\t%%ecx,0(%%eax)\n");
}

/* 変数の左辺値を求める関数(代入式と&[変数]のコンパイルに用いる) */
static void
codegen_left_value(struct AST *ast)
{
  assert(!strcmp(ast->ast_type, "AST_expression_id"));

  char *id = ast->child[0]->u.id;
  struct Symbol *symbol = sym_lookup(id);
  switch(symbol->name_space){
    case NS_LOCAL:
      emit_code (ast, "\tleal\t-%d(%%ebp), %%eax\n", symbol->offset + 4);
      emit_code (ast, "\tpushl\t%%eax\n");
      break;
    case NS_ARG:
      emit_code (ast, "\tleal\t%d(%%ebp), %%eax\n", symbol->offset + 4);
      emit_code (ast, "\tpushl\t%%eax\n");
      break;
    case NS_GLOBAL:
      emit_code (ast, "\tpushl\t$_%s\n", symbol->name);
      break;
    default:
      assert(0);
      break;
  }
}

static void
codegen_expression_lor (struct AST *ast)
{
  int i;
  int label1 = label_num++;
  int label2 = label_num++;
  
  assert (!strcmp (ast->ast_type, "AST_expression_lor"));
  
  for(i = 0; i < ast->num_child; i++){
    visit_AST (ast->child[i]);
    emit_code (ast, "\tpopl\t%%eax\n");
    frame_height -= 4;
    emit_code (ast, "\ttestl\t%%eax, %%eax\t#自分自身とANDをとる\n");
    emit_code (ast, "\tjne\tL%d\t#0でなければ結果は1(true)\n", label1);
  }
  emit_code (ast, "\tpushl\t$0\t#どちらも0なら結果は0(false)\n");
  emit_code (ast, "\tjmp\tL%d\n", label2);
  emit_code (ast, "L%d:\n", label1);
  emit_code (ast, "\tpushl\t$1\n");
  emit_code (ast, "L%d:\n", label2);
  
  frame_height += 4;
}

static void codegen_expression_land (struct AST *ast)
{
  int i;
  int label1 = label_num++;
  int label2 = label_num++;
  
  assert (!strcmp (ast->ast_type, "AST_expression_land"));
  
  for(i = 0; i < ast->num_child; i++){
    visit_AST (ast->child[i]);
    emit_code (ast, "\tpopl\t%%eax\n");
    frame_height -= 4;
    emit_code (ast, "\ttestl\t%%eax, %%eax\t#自分自身とANDをとる\n");
    emit_code (ast, "\tje\tL%d\t#0なら結果は0(false)\n", label1);
  }
  emit_code (ast, "\tpushl\t$1\t#どちらも0でなければ結果は1(true)\n");
  emit_code (ast, "\tjmp\tL%d\n", label2);
  emit_code (ast, "L%d:\n", label1);
  emit_code (ast, "\tpushl\t$0\n");
  emit_code (ast, "L%d:\n", label2);
  
  frame_height += 4;
}

static void codegen_expression_eq (struct AST *ast)
{
  int i;

  assert (!strcmp (ast->ast_type, "AST_expression_eq"));

  for(i = 0; i < ast->num_child; i++){
    visit_AST (ast->child[i]);
  }

  emit_code (ast, "\tpopl\t%%ecx\n");
  emit_code (ast, "\tpopl\t%%eax\n");
  emit_code (ast, "\tcmpl\t%%ecx, %%eax\t#比較\n");
  emit_code (ast, "\tsete\t%%al\t#比較結果を%%alに代入\n");
  emit_code (ast, "\tmovzbl\t%%al, %%eax\t#%%elをゼロ拡張して%%eaxへ\n");
  emit_code (ast, "\tpushl\t%%eax\n");

  frame_height -= 4; /* 2回pop(-8)後,結果を積む(+4) */
}

static void
codegen_expression_less (struct AST *ast)
{
  int i;
  int label1 = label_num++;
  int label2 = label_num++;

  assert (!strcmp (ast->ast_type, "AST_expression_less"));
  
  for(i = 0; i < ast->num_child; i++){
    visit_AST (ast->child[i]);
  }

  emit_code (ast, "\tpopl\t%%ecx\n");
  emit_code (ast, "\tpopl\t%%eax\n");
  emit_code (ast, "\tcmpl\t%%ecx, %%eax\t#比較\n");
  emit_code (ast, "\tjl\tL%d\n", label1);
  emit_code (ast, "\tpushl\t$0\t#偽の場合\n");
  emit_code (ast, "\tjmp\tL%d\n", label2);
  emit_code (ast, "L%d:\n" ,label1);
  emit_code (ast, "\tpushl\t$1\t#真の場合\n");
  emit_code (ast, "L%d:\n", label2);

  frame_height -= 4; /* 2回pop(-8)後,結果を積む(+4) */
}

static void
codegen_expression_add (struct AST *ast)
{
  int i;
  int left_is_ptr  = ast->child[0]->type->kind == TYPE_KIND_POINTER;
  int right_is_ptr = ast->child[1]->type->kind == TYPE_KIND_POINTER;

  assert (!strcmp (ast->ast_type, "AST_expression_add"));

  for(i = 0; i < ast->num_child; i++){
    visit_AST (ast->child[i]);
  }

  if(left_is_ptr){
    if(right_is_ptr){
      assert(0);  /* コンパイルエラー */
    }
    else{
      emit_code (ast, "\tpopl\t%%ecx\n");
      emit_code (ast, "\tsall\t$2, %%ecx\t#4倍\n");
      emit_code (ast, "\tpopl\t%%eax\n");
    }
  }
  else{ /* left is not ptr */
    if(right_is_ptr){
      emit_code (ast, "\tpopl\t%%ecx\n");
      emit_code (ast, "\tpopl\t%%eax\n");
      emit_code (ast, "\tsall\t$2, %%eax\t#4倍\n");
    }
    else{
      emit_code (ast, "\tpopl\t%%ecx\n");
      emit_code (ast, "\tpopl\t%%eax\n");
    }
  }
  emit_code (ast, "\taddl\t%%ecx, %%eax\t#加算\n");
  emit_code (ast, "\tpushl\t%%eax\t#結果をスタックに積む\n");
  frame_height -= 4; /* 2回pop(-8)後,結果を積む(+4) */
}

static void
codegen_expression_sub (struct AST *ast)
{
  int i;
  int left_is_ptr  = ast->child[0]->type->kind == TYPE_KIND_POINTER;
  int right_is_ptr = ast->child[1]->type->kind == TYPE_KIND_POINTER;

  assert (!strcmp (ast->ast_type, "AST_expression_sub"));

  for(i = 0; i < ast->num_child; i++){
    visit_AST (ast->child[i]);
  }

  if(left_is_ptr){
    if(right_is_ptr){
      /* ptr - ptr */
      emit_code (ast, "\tpopl\t%%ecx\n");
      emit_code (ast, "\tpopl\t%%eax\n");
      emit_code (ast, "\tsubl\t%%ecx, %%eax\t#減算\n");
      emit_code (ast, "\tsarl\t$2, %%eax\t#4で割る\n");
    }
    else{
      /* ptr - prim */
      emit_code (ast, "\tpopl\t%%ecx\n");
      emit_code (ast, "\tsall\t$2, %%ecx\t#4倍\n");
      emit_code (ast, "\tpopl\t%%eax\n");
      emit_code (ast, "\tsubl\t%%ecx, %%eax\t#減算\n");
    }
  }
  else{ /* left is not ptr */
    if(right_is_ptr){
      /* prim - ptr */
      assert(0); /* コンパイルエラー */
    }
    else{ 
      /* prim - prim */
    emit_code (ast, "\tpopl\t%%ecx\n");
    emit_code (ast, "\tpopl\t%%eax\n");
    emit_code (ast, "\tsubl\t%%ecx, %%eax\t#減算\n");
    }
  }
  emit_code (ast, "\tpushl\t%%eax\t#結果をスタックに積む\n");
  frame_height -= 4; /* 2回pop(-8)後,結果を積む(+4) */
}

static void codegen_expression_mul (struct AST *ast)
{
  int i;

  assert (!strcmp (ast->ast_type, "AST_expression_mul"));

  for(i = 0; i < ast->num_child; i++){
    visit_AST (ast->child[i]);
  }

  emit_code (ast, "\tpopl\t%%ecx\n");
  emit_code (ast, "\tpopl\t%%eax\n");
  emit_code (ast, "\timull\t%%ecx, %%eax\t#乗算\n");
  emit_code (ast, "\tpushl\t%%eax\t#結果をスタックに積む\n");

  frame_height -= 4; /* 2回pop(-8)後,結果を積む(+4) */
}
static void codegen_expression_div (struct AST *ast)
{
  int i;

  assert (!strcmp (ast->ast_type, "AST_expression_div"));

  for(i = 0; i < ast->num_child; i++){
    visit_AST (ast->child[i]);
  }

  emit_code (ast, "\tpopl\t%%ecx\n");
  emit_code (ast, "\tpopl\t%%eax\n");
  emit_code (ast, "\tcltd\t#符号拡張\n");
  emit_code (ast, "\tidivl\t%%ecx\t#除算\n");
  emit_code (ast, "\tpushl\t%%eax\t#結果をスタックに積む\n");

  frame_height -= 4; /* 2回pop(-8)後,結果を積む(+4) */
}
static void
codegen_expression_unary (struct AST *ast)
{
  assert (!strcmp(ast->ast_type, "AST_expression_unary"));

  char *un_ope = ast->child[0]->ast_type;
  int label1;
  int label2;

  /* &[変数]: 変数のアドレスを求める */
  if(!strcmp(un_ope, "AST_unary_operator_address")){
    codegen_left_value(ast->child[1]);
  }
  /* その他の単項演算子の場合 */
  else{
    // 式のコンパイル
    visit_AST(ast->child[1]);
  
    // 演算子の種類によって式の結果を操作
    if(!strcmp(un_ope, "AST_unary_operator_deref")){
      /* *(式): ポインタの先を参照 */
      emit_code(ast, "\tpopl\t%%eax\t#ポインタのアドレス値を%%eaxへ\n");
      emit_code(ast, "\tmovl\t0(%%eax), %%eax\t#ポインタの先の値を%%eaxへ\n");
      emit_code(ast, "\tpushl\t%%eax\t#ポインタの先の値をスタックに積む\n");
    } 
    else if(!strcmp(un_ope, "AST_unary_operator_plus")){
      /* +(式): 符号そのまま(何もしない) */
    }
    else if(!strcmp(un_ope, "AST_unary_operator_minus")){
      /* -(式): 符号反転 */
      emit_code(ast, "\tnegl\t0(%%esp)\t#符号反転\n");
    } 
    else if(!strcmp(un_ope, "AST_unary_operator_negative")){
      /* !(式): 式の値が0なら1,0以外なら1にする */
      label1 = label_num++;
      label2 = label_num++;
  
      emit_code (ast, "\tpopl\t%%eax\n");
      emit_code (ast, "\ttestl\t%%eax, %%eax\t#自分自身とANDをとる\n");
      emit_code (ast, "\tje\tL%d\t#0ならジャンプ\n", label1);
      emit_code (ast, "\tpushl\t$0\t#1(0以外)なら0に\n");
      emit_code (ast, "\tjmp\tL%d\n", label2);
      emit_code (ast, "L%d:\n", label1);
      emit_code (ast, "\tpushl\t$1\t#0なら1に\n");
      emit_code (ast, "L%d:\n", label2);
    }
  }
}
static void
codegen_argument_expression_list_pair (struct AST *ast)
{
  int i;
  
  assert (!strcmp (ast->ast_type, "AST_argument_expression_list_pair"));
  
  /* 引数の値を逆順にスタックに積むため、逆に子供をたどる */
  for (i = ast->num_child - 1; i >= 0 ; i--) {
    visit_AST (ast->child [i]);
  };
}
