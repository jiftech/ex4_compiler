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

static int frame_height;	/* �����å��ե�`��δ󤭤� */
static char *funcname;		/* ����ѥ����Ф��v���� */

static int label_num; /* �Τ�ʹ�ä����٥뷬�Ť���� */

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

/* ����������I��_�� */
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
  
  codegen_begin_function (ast);	/* ��ǰ������� */
  frame_height = 4;	/* ���ӳ����줿�Ȥ��ϡ�%eip�Τߥ����å��ˤ��뤿�ᡢ�󤭤���4 */
  if (sym_table.string != NULL) 
    codegen_string_def (ast, sym_table.string);
  emit_code (ast, "\t.text\n");
  emit_code (ast, "\t.globl\t_%s\n", funcname);
  emit_code (ast, "_%s:\n", funcname);
  emit_code (ast, "\tpushl\t%%ebp\t# �����å��ե�`�������\n");
  frame_height += 4;	/* �����å���%ebp���e�ޤ줿���ᡢ�󤭤���4���䤹 */
  emit_code (ast, "\tmovl\t%%esp, %%ebp\n");

  /* �����������I��_�� */
  emit_code (ast, "\tsubl\t%d, %%esp\t# �����������I���_��\n", ast.u.func.total_local_size);
  
  /* ����Υ���ѥ��� */
  for (i = 0; i < ast->num_child; i++) {
    visit_AST (ast->child [i]);
  };
  
  emit_code (ast, "L.XCC.RE.%s:\n", funcname);
  emit_code (ast, "\tmovl\t%%ebp, %%esp\t# �����å��ե�`����ȥ\n");
  emit_code (ast, "\tpopl\t%%ebp\n");
  emit_code (ast, "\tret\n");
  
  codegen_end_function (); /* ��ǰ������� */
}

static void
codegen_statement_exp (struct AST *ast)
{
  int i;
  
  assert (!strcmp (ast->ast_type, "AST_statement_exp"));

  /* ����Υ���ѥ��� */
  for (i = 0; i < ast->num_child; i++) {
    visit_AST (ast->child [i]);
  };
  if (!strcmp (ast->child[0]->ast_type, "AST_expression_opt_single")){
    emit_code (ast, "\taddl\t$%d, %%esp\t# ���ꂎ�����\n", 4);
    frame_height -= 4;	/* �����å��ȥåפ��������줿�Τǡ�4�p�餹��*/
  };
}

static void
codegen_compound_statement (struct AST *ast)
{
  int i;

  assert (!strcmp (ast->ast_type, "AST_compound_statement"));

  codegen_begin_block (ast);	/* ��ǰ������� */

  /* ����Υ���ѥ��� */
  for (i = 0; i < ast->num_child; i++) {
    visit_AST (ast->child [i]);
  };

  codegen_end_block ();	/* ��ǰ������� */
}

static void
codegen_statement_if (struct AST *ast)
{
  int label_end   = label_num++; /* if�ĤΥ��`�ɤνK���Υ�٥뷬��*/

  assert (!strcmp (ast->ast_type, "AST_statement_if"));

  /* ����ʽ�Υ���ѥ���*/
  visit_AST(ast->child[0]);

  emit_code (ast, "\tpopl\t%%eax\t#ʽ�΂���%%eax��\n");
  emit_code (ast, "\tcmpl\t$0, %%eax\t#ʽ�΂�=0?\n");
  emit_code (ast, "\tje\tL%d\t#0�ʤ�K���ޤǥ�����\n", label_end);
  frame_height -= 4;

  /* �g���ĤΥ���ѥ���*/
  visit_AST(ast->child[1]);

  emit_code (ast, "L%d:\n", label_end);
}

static void
codegen_statement_ifelse (struct AST *ast)
{
  int label_else = label_num++; /* if-else�Ĥ�else�Υ��`�ɤ�ʼ�ޤ�*/
  int label_end  = label_num++; /* if-else�ĤΥ��`�ɤνK���*/

  assert (!strcmp (ast->ast_type, "AST_statement_ifelse"));

  /* ����ʽ�Υ���ѥ���*/
  visit_AST(ast->child[0]);

  emit_code (ast, "\tpopl\t%%eax\t#ʽ�΂���%%eax��\n");
  emit_code (ast, "\tcmpl\t$0, %%eax\t#ʽ�΂�=0?\n");
  emit_code (ast, "\tje\tL%d\t#0�ʤ�else���˥�����\n", label_else);
  frame_height -= 4;

  /* ����ʽ��true�ΤȤ��g�Ф������(if��)�Υ���ѥ���*/
  visit_AST(ast->child[1]);
  emit_code (ast, "\tjmp\tL%d\t#else���򥹥��å�\n", label_end);

  emit_code (ast, "L%d:\n", label_else); /* else���Υ�٥�*/

  /* ����ʽ��false�ΤȤ��g�Ф������(else��)�Υ���ѥ���*/
  visit_AST(ast->child[2]);

  emit_code (ast, "L%d:\n", label_end);
}

static void
codegen_statement_while (struct AST *ast)
{
  int label_begin = label_num++; /* while�ĤΥ��`�ɤΤϤ���Υ�٥뷬�� */
  int label_end   = label_num++; /* while�ĤΥ��`�ɤΤ����Υ�٥뷬�� */

  assert (!strcmp (ast->ast_type, "AST_statement_while"));

  emit_code (ast, "L%d:\n", label_begin);

  /* ����ʽ�Υ���ѥ��� */
  visit_AST(ast->child[0]);

  emit_code (ast, "\tpopl\t%%eax\t#ʽ�΂���%%eax��\n");
  emit_code (ast, "\tcmpl\t$0, %%eax\t#ʽ�΂�=0?\n");
  emit_code (ast, "\tje\tL%d\t#0�ʤ��`�פ������\n", label_end);
  frame_height -= 4; /* �����ĤνY�����Ɨ����줿 */

  /* �g���ĤΥ���ѥ��� */
  visit_AST(ast->child[1]);

  emit_code (ast, "\tjmp\tL%d\t#�����ж��ؑ���\n", label_begin);

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
			frame_height += 4;	/* �����å��ˉ����΂����e�ޤ줿 */
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
  frame_height += 4;	/* �����å��˼������e�ޤ줿 */
}

static void
codegen_expression_string (struct AST *ast)
{
  struct String *string;

  assert (!strcmp (ast->ast_type, "AST_expression_string"));
  
  string = string_lookup(ast->u.id);
  emit_code (ast, "\tpushl\t$L%s\n", string->label);
  frame_height += 4;	/* �����å���string�Τ��륢�ɥ쥹���e�ޤ줿 */
}

static void
codegen_expression_funcall (struct AST *ast)
{
  int i;
  int fh;
  int padding; /* ���ӳ����r%esp��16�Х��Ⱦ���Τ��ᡢ���뤹��padding�δ󤭤� */

  assert (!strcmp (ast->ast_type, "AST_expression_funcall1")
          || !strcmp (ast->ast_type, "AST_expression_funcall2"));

  /*  ���뤹��padding�δ󤭤���Ӌ�㤷��padding��e�� */
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
    emit_code (ast, "\tsubl\t$%d, %%esp\t# padding��e��\n", padding);
    frame_height += padding;
  };
  
  /* �����΂���e��Ǥ��顢�v�����ӳ��� */
  for (i = ast->num_child - 1; i >= 0 ; i--) {
    visit_AST (ast->child [i]);
  };
  
  /* ������padding��ΤƤ� */
  if (!strcmp (ast->ast_type, "AST_expression_funcall1") 
      && ast->child[1]->u.arg_size != 0) {
    emit_code (ast, "\taddl\t$%d, %%esp\t# ������ΤƤ�\n", ast->child[1]->u.arg_size); 
    frame_height -= ast->child[1]->u.arg_size;
  };
  if (padding != 0) {
    emit_code (ast, "\taddl\t$%d, %%esp\t# padding�����\n", padding); 
    frame_height -= padding;
  };
  
  /* ���ꂎ�򥹥��å��ˁ\���� */
  emit_code (ast, "\tpushl\t%%eax\t# ���ꂎ��e��\n");
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
  
  /* ���x���򥹥��å��˷e�� */
  visit_AST (ast->child[1]);
  
  /* ���x���Υ��ɥ쥹�򥹥��å��˷e�� */
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
  
  /* ���� �B�A����ʽ�Τ�������x���ϥ����å��ȥåפ˲Ф� */
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
		emit_code (ast, "\ttestl\t%%eax, %%eax\t#�Է������AND��Ȥ�\n");
		emit_code (ast, "\tjne\tL%d\t#0�Ǥʤ���нY����1(true)\n", label1);
	}
	emit_code (ast, "\tpushl\t$0\t#�ɤ����0�ʤ�Y����0(false)\n");
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
		emit_code (ast, "\ttestl\t%%eax, %%eax\t#�Է������AND��Ȥ�\n");
		emit_code (ast, "\tje\tL%d\t#0�ʤ�Y����0(false)\n", label1);
	}
	emit_code (ast, "\tpushl\t$1\t#�ɤ����0�Ǥʤ���нY����1(true)\n");
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
  emit_code (ast, "\tcmpl\t%%ecx, %%eax\t#���^\n");
  emit_code (ast, "\tsete\t%%al\t#���^�Y����%%al�˴���\n");
  emit_code (ast, "\tmovzbl\t%%al, %%eax\t#%%el�򥼥품������%%eax��\n");
  emit_code (ast, "\tpushl\t%%eax\n");

  frame_height -= 4; /* 2��pop(-8)��,�Y����e��(+4) */
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
  emit_code (ast, "\tcmpl\t%%ecx, %%eax\t#���^\n");
  emit_code (ast, "\tjl\tL%d\n", label1);
  emit_code (ast, "\tpushl\t$0\t#�ΤΈ���\n");
  emit_code (ast, "\tjmp\tL%d\n", label2);
  emit_code (ast, "L%d:\n" ,label1);
  emit_code (ast, "\tpushl\t$1\t#��Έ���\n");
  emit_code (ast, "L%d:\n", label2);

  frame_height -= 4; /* 2��pop(-8)��,�Y����e��(+4) */
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
  emit_code (ast, "\taddl\t%%ecx, %%eax\t#����\n");
  emit_code (ast, "\tpushl\t%%eax\t#�Y���򥹥��å��˷e��\n");

  frame_height -= 4; /* 2��pop(-8)��,�Y����e��(+4) */
  
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
  emit_code (ast, "\tsubl\t%%ecx, %%eax\t#�p��\n");
  emit_code (ast, "\tpushl\t%%eax\t#�Y���򥹥��å��˷e��\n");

  frame_height -= 4; /* 2��pop(-8)��,�Y����e��(+4) */
  
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
  emit_code (ast, "\timull\t%%ecx, %%eax\t#�\��\n");
  emit_code (ast, "\tpushl\t%%eax\t#�Y���򥹥��å��˷e��\n");

  frame_height -= 4; /* 2��pop(-8)��,�Y����e��(+4) */
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
  emit_code (ast, "\tcltd\t#���Œ���\n");
  emit_code (ast, "\tidivl\t%%ecx\t#����\n");
  emit_code (ast, "\tpushl\t%%eax\t#�Y���򥹥��å��˷e��\n");

  frame_height -= 4; /* 2��pop(-8)��,�Y����e��(+4) */
}

static void
codegen_argument_expression_list_pair (struct AST *ast)
{
  int i;
  
  assert (!strcmp (ast->ast_type, "AST_argument_expression_list_pair"));
  
  /* �����΂�����혤˥����å��˷e�ि�ᡢ����ӹ��򤿤ɤ� */
  for (i = ast->num_child - 1; i >= 0 ; i--) {
    visit_AST (ast->child [i]);
  };
}
