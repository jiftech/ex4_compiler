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

static int frame_height;	/* スタックフレ�`ムの寄きさ */
static char *funcname;		/* コンパイル嶄の�v方兆 */

static int label_num; /* 肝に聞喘するラベル桑催を砿尖 */

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
static void codegen_expression_id (struct AST *ast);
static void codegen_expression_intchar (struct AST *ast);
static void codegen_expression_string (struct AST *ast);
static void codegen_expression_funcall (struct AST *ast);
static void codegen_expression_assign (struct AST *ast);
static void codegen_expression_lor (struct AST *ast);
static void codegen_expression_land (struct AST *ast);
static void codegen_expression_eq (struct AST *ast);
static void codegen_expression_less (struct AST *ast);
static void codegen_expression_add (struct AST *ast);
static void codegen_expression_sub (struct AST *ast);
static void codegen_expression_mul (struct AST *ast);
static void codegen_expression_div (struct AST *ast);
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
		else if (sym->type->kind == TYPE_KIND_PRIM) {
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

/* 寄囃�篳�の�I囃�_隠 */
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

  assert (!strcmp (ast->ast_type, "AST_function_definition"));
  
  codegen_begin_function (ast);	/* 兆念燕の俐屎 */
  frame_height = 4;	/* 柵び竃されたときは、%eipのみスタックにあるため、寄きさは4 */
  if (sym_table.string != NULL) 
    codegen_string_def (ast, sym_table.string);
  emit_code (ast, "\t.text\n");
  emit_code (ast, "\t.globl\t_%s\n", funcname);
  emit_code (ast, "_%s:\n", funcname);
  emit_code (ast, "\tpushl\t%%ebp\t# スタックフレ�`ムを恬撹\n");
  frame_height += 4;	/* スタックに%ebpが�eまれたため、寄きさを4��やす */
  emit_code (ast, "\tmovl\t%%esp, %%ebp\n");

  /* 蕉侭�篳�の�I囃�_隠 */
  emit_code (ast, "\tsubl\t%d, %%esp\t# 蕉侭�篳�の�I囃を�_隠\n", ast.u.func.total_local_size);
  
  /* 云悶のコンパイル */
  for (i = 0; i < ast->num_child; i++) {
    visit_AST (ast->child [i]);
  };
  
  emit_code (ast, "L.XCC.RE.%s:\n", funcname);
  emit_code (ast, "\tmovl\t%%ebp, %%esp\t# スタックフレ�`ムを茅肇\n");
  emit_code (ast, "\tpopl\t%%ebp\n");
  emit_code (ast, "\tret\n");
  
  codegen_end_function (); /* 兆念燕の俐屎 */
}

static void
codegen_statement_exp (struct AST *ast)
{
  int i;
  
  assert (!strcmp (ast->ast_type, "AST_statement_exp"));

  /* 云悶のコンパイル */
  for (i = 0; i < ast->num_child; i++) {
    visit_AST (ast->child [i]);
  };
  if (!strcmp (ast->child[0]->ast_type, "AST_expression_opt_single")){
    emit_code (ast, "\taddl\t$%d, %%esp\t# 卦り�､�����\n", 4);
    frame_height -= 4;	/* スタックトップが����されたので、4�pらす。*/
  };
}

static void
codegen_compound_statement (struct AST *ast)
{
  int i;

  assert (!strcmp (ast->ast_type, "AST_compound_statement"));

  codegen_begin_block (ast);	/* 兆念燕の俐屎 */

  /* 云悶のコンパイル */
  for (i = 0; i < ast->num_child; i++) {
    visit_AST (ast->child [i]);
  };

  codegen_end_block ();	/* 兆念燕の俐屎 */
}

static void
codegen_statement_if (struct AST *ast)
{
  int label_end   = label_num++; /* if猟のコ�`ドの�Kわりのラベル桑催*/

  assert (!strcmp (ast->ast_type, "AST_statement_if"));

  /* 訳周塀のコンパイル*/
  visit_AST(ast->child[0]);

  emit_code (ast, "\tpopl\t%%eax\t#塀の�､�%%eaxへ\n");
  emit_code (ast, "\tcmpl\t$0, %%eax\t#塀の��=0?\n");
  emit_code (ast, "\tje\tL%d\t#0なら�Kわりまでジャンプ\n", label_end);
  frame_height -= 4;

  /* �g佩猟のコンパイル*/
  visit_AST(ast->child[1]);

  emit_code (ast, "L%d:\n", label_end);
}

static void
codegen_statement_ifelse (struct AST *ast)
{
  int label_else = label_num++; /* if-else猟のelseのコ�`ドの兵まり*/
  int label_end  = label_num++; /* if-else猟のコ�`ドの�Kわり*/

  assert (!strcmp (ast->ast_type, "AST_statement_ifelse"));

  /* 訳周塀のコンパイル*/
  visit_AST(ast->child[0]);

  emit_code (ast, "\tpopl\t%%eax\t#塀の�､�%%eaxへ\n");
  emit_code (ast, "\tcmpl\t$0, %%eax\t#塀の��=0?\n");
  emit_code (ast, "\tje\tL%d\t#0ならelse何にジャンプ\n", label_else);
  frame_height -= 4;

  /* 訳周塀がtrueのとき�g佩される猟(if何)のコンパイル*/
  visit_AST(ast->child[1]);
  emit_code (ast, "\tjmp\tL%d\t#else何をスキップ\n", label_end);

  emit_code (ast, "L%d:\n", label_else); /* else何のラベル*/

  /* 訳周塀がfalseのとき�g佩される猟(else何)のコンパイル*/
  visit_AST(ast->child[2]);

  emit_code (ast, "L%d:\n", label_end);
}

static void
codegen_statement_while (struct AST *ast)
{
  int label_begin = label_num++; /* while猟のコ�`ドのはじめのラベル桑催 */
  int label_end   = label_num++; /* while猟のコ�`ドのおわりのラベル桑催 */

  assert (!strcmp (ast->ast_type, "AST_statement_while"));

  emit_code (ast, "L%d:\n", label_begin);

  /* 訳周塀のコンパイル */
  visit_AST(ast->child[0]);

  emit_code (ast, "\tpopl\t%%eax\t#塀の�､�%%eaxへ\n");
  emit_code (ast, "\tcmpl\t$0, %%eax\t#塀の��=0?\n");
  emit_code (ast, "\tje\tL%d\t#0ならル�`プから竃る\n", label_end);
  frame_height -= 4; /* 訳周猟の�Y惚が篤��された */

  /* �g佩猟のコンパイル */
  visit_AST(ast->child[1]);

  emit_code (ast, "\tjmp\tL%d\t#訳周登協へ��る\n", label_begin);

  emit_code (ast, "L%d:\n", label_end);
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
    case TYPE_KIND_POINTER:
      break;
    case default:
      assert(0);
      break;
    }
  case NS_ARG:
    switch (symbol->type->kind) {
    case TYPE_KIND_PRIM:
      emit_code (ast, "\tpushl\t%d(%%ebp)\n", symbol->offset);
      break;
    case TYPE_KIND_POINTER:
      break;
    case default:
      assert(0);
      break;
    }
  case NS_GLOBAL:
    switch (symbol->type->kind) {
    case TYPE_KIND_PRIM:
      emit_code (ast, "\tpushl\t_%s\n", id);
			frame_height += 4;	/* スタックに�篳�の�､��eまれた */
      break;
    case TYPE_KIND_POINTER:
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
  frame_height += 4;	/* スタックに軸�､��eまれた */
}

static void
codegen_expression_string (struct AST *ast)
{
  struct String *string;

  assert (!strcmp (ast->ast_type, "AST_expression_string"));
  
  string = string_lookup(ast->u.id);
  emit_code (ast, "\tpushl\t$L%s\n", string->label);
  frame_height += 4;	/* スタックにstringのあるアドレスが�eまれた */
}

static void
codegen_expression_funcall (struct AST *ast)
{
  int i;
  int fh;
  int padding; /* 柵び竃し�r%espが16バイト廠順のため、�携襪垢�paddingの寄きさ */

  assert (!strcmp (ast->ast_type, "AST_expression_funcall1")
          || !strcmp (ast->ast_type, "AST_expression_funcall2"));

  /*  �携襪垢�paddingの寄きさの��麻し、paddingを�eむ */
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
    emit_code (ast, "\tsubl\t$%d, %%esp\t# paddingを�eむ\n", padding);
    frame_height += padding;
  };
  
  /* 哈方の�､魴eんでから、�v方柵び竃し */
  for (i = ast->num_child - 1; i >= 0 ; i--) {
    visit_AST (ast->child [i]);
  };
  
  /* 哈方とpaddingを�里討� */
  if (!strcmp (ast->ast_type, "AST_expression_funcall1") 
      && ast->child[1]->u.arg_size != 0) {
    emit_code (ast, "\taddl\t$%d, %%esp\t# 哈方を�里討�\n", ast->child[1]->u.arg_size); 
    frame_height -= ast->child[1]->u.arg_size;
  };
  if (padding != 0) {
    emit_code (ast, "\taddl\t$%d, %%esp\t# paddingを����\n", padding); 
    frame_height -= padding;
  };
  
  /* 卦り�､鬟好織奪�に�\せる */
  emit_code (ast, "\tpushl\t%%eax\t# 卦り�､魴eむ\n");
  frame_height += 4;
}

static void
codegen_expression_assign (struct AST *ast)
{
  char *id;
  struct Symbol *symbol;
  
  assert(!strcmp (ast->ast_type, "AST_expression_assign"));
  
  id = ast->child[0]->child[0]->u.id;
  symbol = sym_lookup (id);
  
  /* 嘔�x�､鬟好織奪�に�eむ */
  visit_AST (ast->child[1]);
  
  /* 恣�x�､離▲疋譽垢鬟好織奪�に�eむ */
  switch(symbol->name_space){
    case NS_LOCAL:
      emit_code (ast, "\tleal\t-%d(%%ebp), %%eax\n", symbol->offset + 4);
      emit_code (ast, "\tpushl\t%%eax\n");
      break;
    case NS_ARG:
      emit_code (ast, "\tleal\t%d(%%ebp), %%eax\n", symbol->offset);
      emit_code (ast, "\tpushl\t%%eax\n");
      break;
    case NS_GLOBAL:
      emit_code (ast, "\tpushl\t$_%s\n", symbol->name);
      break;
    case default:
      assert(0);
      break;
  }
  
  /* 旗秘 �B�A旗秘塀のために嘔�x�､魯好織奪�トップに火す */
  emit_code (ast, "\tpopl\t%%eax\n");
  emit_code (ast, "\tmovl\t0(%%esp),%%ecx\n");
  emit_code (ast, "\tmovl\t%%ecx,0(%%eax)\n");
}

static void codegen_expression_lor (struct AST *ast)
{
  int i;
	int label1 = label_num++;
	int label2 = label_num++;

	assert (!strcmp (ast->ast_type, "AST_expression_lor"));

	for(i = 0; i < ast->num_child; i++){
		visit_AST (ast->child[i]);
		emit_code (ast, "\tpopl\t%%eax\n");
		frame_height -= 4;
		emit_code (ast, "\ttestl\t%%eax, %%eax\t#徭蛍徭附とANDをとる\n");
		emit_code (ast, "\tjne\tL%d\t#0でなければ�Y惚は1(true)\n", label1);
	}
	emit_code (ast, "\tpushl\t$0\t#どちらも0なら�Y惚は0(false)\n");
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
		emit_code (ast, "\ttestl\t%%eax, %%eax\t#徭蛍徭附とANDをとる\n");
		emit_code (ast, "\tje\tL%d\t#0なら�Y惚は0(false)\n", label1);
	}
	emit_code (ast, "\tpushl\t$1\t#どちらも0でなければ�Y惚は1(true)\n");
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
  emit_code (ast, "\tcmpl\t%%ecx, %%eax\t#曳�^\n");
  emit_code (ast, "\tsete\t%%al\t#曳�^�Y惚を%%alに旗秘\n");
  emit_code (ast, "\tmovzbl\t%%al, %%eax\t#%%elをゼロ����して%%eaxへ\n");
  emit_code (ast, "\tpushl\t%%eax\n");

  frame_height -= 4; /* 2指pop(-8)瘁,�Y惚を�eむ(+4) */
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
  emit_code (ast, "\tcmpl\t%%ecx, %%eax\t#曳�^\n");
  emit_code (ast, "\tjl\tL%d\n", label1);
  emit_code (ast, "\tpushl\t$0\t#�里���栽\n");
  emit_code (ast, "\tjmp\tL%d\n", label2);
  emit_code (ast, "L%d:\n" ,label1);
  emit_code (ast, "\tpushl\t$1\t#寔の��栽\n");
  emit_code (ast, "L%d:\n", label2);

  frame_height -= 4; /* 2指pop(-8)瘁,�Y惚を�eむ(+4) */
}

static void
codegen_expression_add (struct AST *ast)
{
  int i;

  assert (!strcmp (ast->ast_type, "AST_expression_add"));

  for(i = 0; i < ast->num_child; i++){
    visit_AST (ast->child[i]);
  }

  emit_code (ast, "\tpopl\t%%ecx\n");
  emit_code (ast, "\tpopl\t%%eax\n");
  emit_code (ast, "\taddl\t%%ecx, %%eax\t#紗麻\n");
  emit_code (ast, "\tpushl\t%%eax\t#�Y惚をスタックに�eむ\n");

  frame_height -= 4; /* 2指pop(-8)瘁,�Y惚を�eむ(+4) */
  
}

static void
codegen_expression_sub (struct AST *ast)
{
  int i;

  assert (!strcmp (ast->ast_type, "AST_expression_sub"));

  for(i = 0; i < ast->num_child; i++){
    visit_AST (ast->child[i]);
  }

  emit_code (ast, "\tpopl\t%%ecx\n");
  emit_code (ast, "\tpopl\t%%eax\n");
  emit_code (ast, "\tsubl\t%%ecx, %%eax\t#�p麻\n");
  emit_code (ast, "\tpushl\t%%eax\t#�Y惚をスタックに�eむ\n");

  frame_height -= 4; /* 2指pop(-8)瘁,�Y惚を�eむ(+4) */
  
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
  emit_code (ast, "\timull\t%%ecx, %%eax\t#�\麻\n");
  emit_code (ast, "\tpushl\t%%eax\t#�Y惚をスタックに�eむ\n");

  frame_height -= 4; /* 2指pop(-8)瘁,�Y惚を�eむ(+4) */
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
  emit_code (ast, "\tcltd\t#憲催����\n");
  emit_code (ast, "\tidivl\t%%ecx\t#茅麻\n");
  emit_code (ast, "\tpushl\t%%eax\t#�Y惚をスタックに�eむ\n");

  frame_height -= 4; /* 2指pop(-8)瘁,�Y惚を�eむ(+4) */
}

static void
codegen_argument_expression_list_pair (struct AST *ast)
{
  int i;
  
  assert (!strcmp (ast->ast_type, "AST_argument_expression_list_pair"));
  
  /* 哈方の�､鯆肭�にスタックに�eむため、剃に徨工をたどる */
  for (i = ast->num_child - 1; i >= 0 ; i--) {
    visit_AST (ast->child [i]);
  };
}
