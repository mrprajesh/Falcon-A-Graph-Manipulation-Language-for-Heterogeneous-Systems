#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include <symtabold.h>
//#include "falctypes.h"
#include "externs.h"
char forcondarr[200];
char threadsizestring[100];
extern intpow(int,int);
char parallel_sections[]={"#pragma omp parallel sections \n{\n"};
char parallel_section[]={"#pragma omp  section \n{\n"};
fpos_t mainfpos;
extern int FUN_EBLOCK_RET,CPUGPU;
extern char *union_string[2];
extern char *pdim_string[4];
extern char *morphalloc_string[3],*cpy[];
extern int utflag,PEER_ENABLE;
extern  char *utflagarr[2][3];
extern char *treenodetypestr;
int FOREACH_FLAG=0,EXTRA_CRBRK=0,MAINF=0,foreachhostflag,NESTED_FOREACH;
char updatecopies[5][100];
int updatecopiescnt;
statement *CURR_STMT;
//extern tree_typedecl *findtypedefsymbol(char *);
//prints expr tree inorder
char threadstmt[100]={"int id= blockIdx.x * blockDim.x + threadIdx.x+FALCX;\n"};
//#include "codepartition.h"
void findstructref(dir_decl *d1,tree_expr *expr){
if(expr->expr_type==STRUCTREF){
if(expr->lhs && expr->lhs->expr_type==VAR){
if(expr->lhs->lhs==d1){
strcpy(updatecopies[updatecopiescnt++],expr->rhs->name);
//fprintf(FP1,"%s.%s",expr->lhs->name,updatecopies[updatecopiescnt-1]);
}
}
return;
}
if(expr->lhs!=NULL)findstructref(d1,expr->lhs);
if(expr->rhs!=NULL)findstructref(d1,expr->rhs);
}
//inorder print of expr types in a tree root node.

 printexpr_type(tree_expr *expr,int d){
if(expr==NULL){
fprintf(stderr,"null ");
return;
}
printexpr_type(expr->lhs,++d);
d--;
fprintf(stderr," node %d %d\n",expr->expr_type,d);
printexpr_type(expr->rhs,++d);
d--;
}
statement *partforeach;
int unnicnt=0,utcnt=0,singlearr,reduxcnt=0,tempfun=0,partcodegen;
char curr_fun_name[100],curr_fun_name1[100];
char barr_after[200];
fpos_t position;
struct UNION_KER *head_position=NULL;
int CPU_COLL_FLAG,PROCESS_CNT,PUSHER_CNT;

void tree_expr::findextraproperties(tree_expr *expr){
if(expr->expr_type==FUNCALL){
assign_stmt *t1= expr->arglist;
while(t1!=NULL){
if(t1->rhs  && t1->rhs->expr_type==STRUCTREF && t1->rhs->lhs && t1->rhs->lhs->expr_type==STRUCTREF){
t1->rhs->FOR_findgpuvars(t1->rhs);

tree_expr *expr1=((dir_decl *)(t1->rhs->lhs->lhs));
if(expr1 && expr1->lhs) {
dir_decl *d1=expr1->lhs;
if( d1!=NULL && d1->gpu==1){
extra_ppts *ex=d1->ppts;
while(ex!=NULL){

if(!strcmp(ex->name,t1->rhs->rhs->name)&& strlen(ex->name)==strlen(t1->rhs->rhs->name)){
fprintf(FP1, "%s *temp%d=(%s *) malloc(sizeof(%s)*%s.npoints);\n",dtypenames[ex->t1->datatype],extemp+1,dtypenames[ex->t1->datatype],dtypenames[ex->t1->datatype],d1->name);
fprintf(FP1,"%s  temp%d;\n cudaMemcpy((void *)&temp%d,%s.extra,sizeof(%s ),cudaMemcpyDeviceToHost);\n",d1->extra_name,extemp+2,extemp+2,d1->name,d1->extra_name);
fprintf(FP1,"cudaMemcpy(temp%d, temp%d.%s,sizeof(%s)*%s.npoints,cudaMemcpyDeviceToHost);\n",extemp+1,extemp+2,ex->name,dtypenames[ex->t1->datatype],d1->name);
t1->rhs->cpu_name=malloc(sizeof(char)*100);
sprintf(t1->rhs->cpu_name,"temp%d[%s]",extemp+1,t1->rhs->lhs->rhs->earr_list->rhs->name);
extemp=extemp+2;
}
ex=ex->next;
}
}
}
}
t1=t1->next;
}
}
}

void findextrapropretry(statement *t1){
statement *temp=t1;
if(temp->sttype==ASSIGN_STMT){
if(temp->stassign->rhs!=NULL)temp->stassign->rhs->findextraproperties(temp->stassign->rhs);

}
}


void statement::codeGen1(){
for(int i=0;i<300;i++)gpcopy_string[i]='\0';
this->codeGen(FP1);
while(CURR_STMT)CURR_STMT->codeGen(FP1);

}
//prints code for foreach statement and its rhs if it is function call
void print_foreach(statement *t){
tree_expr *e2= t->expr2;
tree_expr *e1= t->expr1;
dir_decl *d2=e2->lhs;
dir_decl *d1=e1->lhs;
char arr[200];
for(int i=0;i<200;i++)arr[i]='\0';
tree_expr *e5= t->expr5;
if(e5!=NULL) e5->rhs->printcode1(e5->rhs,arr);
if(d2!=NULL && d2->forit==1){
int itr=t->itr;
dir_decl *d2p=d2->parent;
if(d2->libdtype==POINT_TYPE){
printf("HERE\n");
d1->extra_name1=malloc(sizeof(char)*100);
if(d2->gpu)strcpy(d2->extra_name1,"id");
if(d2->gpu==false)strcpy(d2->extra_name1,d2->name);
if(itr==4|| itr==2){
fprintf(FP1,"/*%dXX*/int falcft%d=%s.index[id+1]-%s.index[id];\nint falcft%d=%s.index[id];\n",d2->gpu,unnicnt,d2p->name,d2p->name,unnicnt+1,d2p->name);
}
if(itr==3){
fprintf(FP1,"int falcft%d=%s.index[id+1]-%s.index[id];\nint falcft%d=%s.index[id];\n",unnicnt,d2p->name,d2p->name,unnicnt+1,d2p->name);
}
if(itr==2||itr==3|| itr==4){
if(itr==4|| itr==2){
fprintf(FP1,"/***/for(int falcft%d=0;falcft%d<falcft%d;falcft%d++){\n",unnicnt+2,unnicnt+2,unnicnt,unnicnt+2);
fprintf(FP1,"int ut%d=%s*(falcft%d+falcft%d);\n int ut%d=%s.edges[ut%d%s].%s;\nint ut%d=%s.edges[ut%d%s].%s;\n",utcnt,utflagarr[utflag][0],unnicnt+1,unnicnt+2,utcnt+1,d2p->name,utcnt,utflagarr[utflag][1],union_string[union_type],utcnt+2,d2p->name,utcnt,utflagarr[utflag][2],union_string[union_type]);
}
if(itr==3){
fprintf(FP1,"/**/for(int falcft%d=0;falcft%d<falcft%d;falcft%d++){\n",unnicnt+2,unnicnt+2,unnicnt,unnicnt+2);
fprintf(FP1,"int ut%d=3*(%s.inedges[falcft%d+falcft%d]);\n int ut%d=%s.edges[ut%d].%s;\nint ut%d=%s.edges[ut%d+2].%s;\n",utcnt,d2p->name,unnicnt+1,unnicnt+2,utcnt+1,d2p->name,utcnt,union_string[union_type],utcnt+2,d2p->name,utcnt,union_string[union_type]);
}
d2->extra_name=malloc(sizeof(char)*100);
strcpy(d2->extra_name,d2p->extra_name);
if(GLOBAL_ID_FLAG==1){strcpy(d2->extra_name1,"id+graph.start_point");
GLOBAL_ID_FLAG=0;
}
d1->extra_name=malloc(sizeof(char)*100);
sprintf(d1->extra_name1,"ut%d",utcnt+1);
if(d2p!=NULL && d2p->extra_name!=NULL)strcpy(d1->extra_name,d2p->extra_name);
d1->parent=d2p;
FOREACH_FLAG++;
unnicnt=unnicnt+3;
utcnt=utcnt+3;
}
if(itr==2){
char temparr[400];
if(t->stassign!=NULL) {
for(int i=0;i<400;i++)temparr[i]='\0';
t->stassign->rhs->printcode1(t->stassign->rhs,temparr);fprintf(FP1,"%s;",temparr);
}
char arr[200];
for(int i=0;i<200;i++)arr[i]='\0';
tree_expr *e5= t->expr5;
if(e5!=NULL){ 
//e5->rhs->printcode1(e5->rhs,arr);
fprintf(FP1,"int falcftadv%d=%s;\n",unnicnt,e5->rhs->name);
}
fprintf(FP1,"int falcft%d=%s.index[id+1]-%s.index[id];\nint falcft%d=%s.index[id];\n",unnicnt,d2p->name,d2p->name,unnicnt+1,d2p->name);
if(e5!=NULL)fprintf(FP1,"for(int falcft%d=falcftadv%d;falcft%d<falcft%d;falcft%d++){\n",unnicnt+2,unnicnt,unnicnt+2,unnicnt,unnicnt+2);
else fprintf(FP1,"for(int falcft%d=0;falcft%d<falcft%d;falcft%d++){\n",unnicnt+2,unnicnt+2,unnicnt,unnicnt+2);
fprintf(FP1,"int ut%d=3*(%s.inedges[falcft%d+falcft%d]);\n int ut%d=%s.edges[ut%d].%s;\nint ut%d=%s.edges[ut%d+2].%s;\n",utcnt,d2p->name,unnicnt+1,unnicnt+2,utcnt+1,d2p->name,utcnt,union_string[union_type],utcnt+2,d2p->name,utcnt,union_string[union_type]);
d2->extra_name1=malloc(sizeof(char)*100);
d2->extra_name=malloc(sizeof(char)*100);
strcpy(d2->extra_name,d2p->extra_name);
strcpy(d2->extra_name1,"id");
d1->extra_name1=malloc(sizeof(char)*100);
d1->extra_name=malloc(sizeof(char)*100);
sprintf(d1->extra_name1,"ut%d",utcnt+1);
if(d2p!=NULL && d2p->extra_name!=NULL)strcpy(d1->extra_name,d2p->extra_name);
d1->parent=d2p;
unnicnt=unnicnt+3;
utcnt=utcnt+3;

}
}
}
printf("%s ", d2->name);
if(d2->libdtype==GRAPH_TYPE){
char *name=t->expr3->name;
if(t->itr>=6 && name!=NULL){
for (std::map<char *,int>::iterator it1=newitr.begin(); it1!=newitr.end(); ++it1){
if(!strcmp(it1->first,t->expr3->name)){

fprintf(FP1,"for(int falcft%d=0;falcft%d<(( %s *)(%s.extra))->n%s;falcft%d++){\n",unnicnt,unnicnt,d2->extra_name,d2->name,it1->first,unnicnt);
d1->extra_name1=malloc(sizeof(char)*100);
d1->extra_name=malloc(sizeof(char)*100);
d1->extra_name2=malloc(sizeof(char)*100);
d1->iflag=1;
strcpy(d1->extra_name2,it1->first);
fprintf(stderr,"\n\nEEEEEEE %s EEEEEEEEEEEE\n\n",it1->first);
sprintf(d1->extra_name1,"falcft%d",unnicnt);
sprintf(d1->name,"falcft%d",unnicnt);//foreach changing name of variable itself
if(d2!=NULL && d2->extra_name!=NULL)strcpy(d1->extra_name,d2->extra_name);
if(d1->extra_name==NULL)fprintf(FP1,"parent null");
if(d2->parent!=NULL)d1->parent=d2->parent;
else d1->parent=d2;
unnicnt++;
break;
}
}
}
}
if(d2->libdtype==EDGE_TYPE){






}
printf(" DONE\n");
}
//prints assignment type
void print_assign_type(FILE *FP1,enum ASSIGN_TYPE tp){
if(tp==AASSIGN)fprintf(FP1,"=");
if(tp==AMUL_ASSIGN)fprintf(FP1,"*=");
if(tp==ADIV_ASSIGN)fprintf(FP1,"/=");
if(tp==AMOD_ASSIGN)fprintf(FP1,"%=");
if(tp==AADD_ASSIGN)fprintf(FP1,"+=");
if(tp==ASUB_ASSIGN)fprintf(FP1,"-=");
if(tp==ALEFT_ASSIGN)fprintf(FP1,"<=");
if(tp==ARIGHT_ASSIGN)fprintf(FP1,">=");
if(tp==AAND_ASSIGN)fprintf(FP1,"&=");
if(tp==AXOR_ASSIGN)fprintf(FP1,"^=");
if(tp==AOR_ASSIGN)fprintf(FP1,"|=");
if(tp==EMPTY_ASSIGN)fprintf(FP1,"=");
if(tp==RRADD_ASSIGN)fprintf(FP1,"+R=");
}
//reduction code generation.REDUXMUL and REDUXSUM
codeGenredux1(dir_decl *d1, int val, char *string3,char *string1,char *string2,char *string,char *str4,char *str5){

fprintf(FP,"__device__ unsigned int reduxsum%d;\n  __global__ void RSUM%d (GGraph %s,int FALCX){\n  ",reduxcnt++,reduxcnt,d1->name);
fprintf(FP,"int id=blockIdx.x * blockDim.x + threadIdx.x+FALCX;\n __shared__ volatile unsigned int blockcount[1024];\n");
fprintf(FP,"if(id <%s.nedges){\n",d1->name);
if(val==1)fprintf(FP,"if(%s %s %s)blockcount[threadIdx.x]=%s; else blockcount[threadIdx.x]=0;}\n else blockcount[threadIdx.x]=0;\n __syncthreads();\n",string1,str5,str4,string);
if(val==2)fprintf(FP,"blockcount[threadIdx.x]=%s;\n}else blockcount[threadIdx.x]=0;\n ",string);
fprintf(FP,"for (int i=2;i<=1024;i=i*2){\nif(threadIdx.x%%i==0)blockcount[threadIdx.x]+=blockcount[threadIdx.x+i/2];\n__syncthreads();\n}\n__syncthreads();\nif(threadIdx.x==0)atomicAdd(&reduxsum%d,blockcount[0]);\n}\n",reduxcnt-1);
fprintf(FP1,"unsigned int hreduxsum%d=0;\ncudaMemcpyToSymbol(reduxsum%d,&hreduxsum%d,sizeof(unsigned int ),0,cudaMemcpyHostToDevice);\n",reduxcnt-1,reduxcnt-1,reduxcnt-1);
fprintf(FP1,"cudaSetDevice(%d);\nfor(int kk=0;kk<%s.nedges;kk+=FALCK32*1024)RSUM%d<<<FALCK32,1024>>>(%s,kk);\ncudaDeviceSynchronize();\ncudaSetDevice(0);\n",d1->dev_no,d1->name,reduxcnt-1,d1->name);
fprintf(FP1,"cudaMemcpyFromSymbol(&hreduxsum%d,reduxsum%d,sizeof(unsigned int ),0,cudaMemcpyDeviceToHost);\n",reduxcnt-1,reduxcnt-1);
fprintf(FP1,"%s=hreduxsum%d;\n",string2 ,reduxcnt-1);
}

void codegenRedux(statement *t){
char arr3[300];
for(int i=0;i<300;i++)arr3[i]='\0';
char arr2[100];
for(int i=0;i<100;i++)arr2[i]='\0';
char arr1[100];
for(int i=0;i<100;i++)arr1[i]='\0';
char arr[100];
for(int i=0;i<100;i++)arr[i]='\0';
char arr4[100];
for(int i=0;i<100;i++)arr4[i]='\0';
char arr5[100];
for(int i=0;i<100;i++)arr5[i]='\0';
if(t->sttype==IF_STMT){
if(t->expr1!=NULL){
if(t->expr1->lhs->lhs!=NULL &&   t->expr1->lhs->lhs->lhs && t->expr1->lhs->lhs->lhs->libdtype==GRAPH_TYPE){
dir_decl * d1=  t->expr1->lhs->lhs->lhs->lhs;
t->f1->stassign->lhs->printcode1(t->f1->stassign->lhs,arr2);
enum EXPR_TYPE t1= t->expr1->expr_type;
if(t1==EQ)sprintf(arr5,"==");
t->expr1->rhs->printcode1(t->expr1->rhs,arr4);
sprintf(arr1,"((%s *)(%s.extra))->%s[id]",d1->extra_name,d1->name,t->expr1->lhs->rhs->name);
if(t->expr1->lhs->lhs->rhs!=NULL && !strcmp(t->expr1->lhs->lhs->rhs->name,"edges") && strlen(t->expr1->lhs->lhs->rhs->name)==strlen("edges") ){
if(!strcmp(t->f1->stassign->rhs->rhs->name,"weight") && strlen(t->f1->stassign->rhs->rhs->name)==strlen("weight")){

sprintf(arr,"%s.edges[3*id+2].ipe",d1->name,t->expr1->lhs->rhs->name);

}
sprintf(arr3,"if(%s ) %s+=  %s;\n",arr1,arr2,arr);
codeGenredux1(d1, 1,arr3,arr1,arr2,arr,arr4,arr5);
}
}
}
return;
}
if(t->stassign->asstype==RRADD_ASSIGN){
tree_expr *expr1=t->stassign->rhs;
fprintf(stderr,"//here \n");
if(expr1->lhs->lhs!=NULL && expr1->lhs->lhs->libdtype==GRAPH_TYPE){
fprintf(stderr,"//here \n");
dir_decl * d1=  expr1->lhs->lhs;
t->stassign->lhs->printcode1(t->stassign->lhs,arr2);
if(d1==NULL)return;
if(!strcmp(t->stassign->rhs->rhs->name,"weight") && strlen(t->stassign->rhs->rhs->name)==strlen("weight")){

sprintf(arr,"%s.edges[%s*id%s].ipe",d1->name,utflagarr[utflag][0],utflagarr[utflag][2]);
fprintf(stderr,"//here \n");
codeGenredux1(d1, 2,arr3,arr1,arr2,arr,arr4,arr5);
fprintf(stderr,"//here \n");
}
}
}
}
codeGenreduxMul1(dir_decl *d1, int val, char *string3,char *string1,char *string2,char *string,char *str4,char *str5){

fprintf(FP,"__device__ unsigned int reduxmul%d;\n  __global__ void RMUL%d (GGraph %s ){\n  ",reduxcnt++,reduxcnt,d1->name);
fprintf(FP,"int id=blockIdx.x * blockDim.x + threadIdx.x;\n __shared__ volatile unsigned int blockcount[1024];\n");
fprintf(FP,"if(id <%s.nedges){\n",d1->name);
if(val==1)fprintf(FP,"if(%s %s %s)blockcount[threadIdx.x]=%s; else blockcount[threadIdx.x]=0;}\n else blockcount[threadIdx.x]=0;\n __syncthreads();\n",string1,str5,str4,string);
if(val==2)fprintf(FP,"blockcount[threadIdx.x]=%s;\n}else blockcount[threadIdx.x]=0;\n ",string);
fprintf(FP,"for (int i=2;i<=1024;i=i*2){\nif(threadIdx.x%%i==0)blockcount[threadIdx.x]*=blockcount[threadIdx.x+i/2];\n__syncthreads();\n}\n__syncthreads();\nif(threadIdx.x==0)atomicAdd(&reduxmul%d,blockcount[0]);\n}\n",reduxcnt-1);
fprintf(FP1,"unsigned int hreduxmul%d=0;\ncudaMemcpyToSymbol(reduxmul%d,&hreduxmul%d,sizeof(unsigned int ),0,cudaMemcpyHostToDevice);\n",reduxcnt-1,reduxcnt-1,reduxcnt-1);
fprintf(FP1,"cudaSetDevice(%d);\nRMUL%d<<<graph.nedges/1024+1,1024>>>(%s);\ncudaDeviceSynchronize();\ncudaSetDevice(0)",d1->dev_no,reduxcnt-1,d1->name);
fprintf(FP1,"cudaMemcpyFromSymbol(&hreduxmul%d,reduxmul%d,sizeof(unsigned int ),0,cudaMemcpyDeviceToHost);\n",reduxcnt-1,reduxcnt-1);
fprintf(FP1,"%s=hreduxmul%d;\n",string2 ,reduxcnt-1);
}

void codegenReduxMul(statement *t){
char arr3[300];
for(int i=0;i<300;i++)arr3[i]='\0';
char arr2[100];
for(int i=0;i<100;i++)arr2[i]='\0';
char arr1[100];
for(int i=0;i<100;i++)arr1[i]='\0';
char arr[100];
for(int i=0;i<100;i++)arr[i]='\0';
char arr4[100];
for(int i=0;i<100;i++)arr4[i]='\0';
char arr5[100];
for(int i=0;i<100;i++)arr5[i]='\0';
if(t->sttype==IF_STMT){
if(t->expr1!=NULL){
if(t->expr1->lhs->lhs!=NULL &&   t->expr1->lhs->lhs->lhs && t->expr1->lhs->lhs->lhs->libdtype==GRAPH_TYPE){
dir_decl * d1=  t->expr1->lhs->lhs->lhs->lhs;
t->f1->stassign->lhs->printcode1(t->f1->stassign->lhs,arr2);
enum EXPR_TYPE t1= t->expr1->expr_type;
if(t1==EQ)sprintf(arr5,"==");
t->expr1->rhs->printcode1(t->expr1->rhs,arr4);
sprintf(arr1,"((%s *)(%s.extra))->%s[id]",d1->extra_name,d1->name,t->expr1->lhs->rhs->name);
if(t->expr1->lhs->lhs->rhs!=NULL && !strcmp(t->expr1->lhs->lhs->rhs->name,"edges") && strlen(t->expr1->lhs->lhs->rhs->name)==strlen("edges") ){
if(!strcmp(t->f1->stassign->rhs->rhs->name,"weight") && strlen(t->f1->stassign->rhs->rhs->name)==strlen("weight")){

sprintf(arr,"%s.edges[%s*id%s].ipe",d1->name,utflagarr[utflag][0],utflagarr[utflag][2],t->expr1->lhs->rhs->name);

}
sprintf(arr3,"if(%s ) %s*=  %s;\n",arr1,arr2,arr);
codeGenreduxMul1(d1, 1,arr3,arr1,arr2,arr,arr4,arr5);
}
}
}
return;
}
if(t->stassign->asstype==RRMUL_ASSIGN){
tree_expr *expr1=t->stassign->rhs;
fprintf(stderr,"//here \n");
if(expr1->lhs->lhs!=NULL && expr1->lhs->lhs->libdtype==GRAPH_TYPE){
fprintf(stderr,"//here \n");
dir_decl * d1=  expr1->lhs->lhs;
t->stassign->lhs->printcode1(t->stassign->lhs,arr2);
if(d1==NULL)return;
if(!strcmp(t->stassign->rhs->rhs->name,"weight") && strlen(t->stassign->rhs->rhs->name)==strlen("weight")){

sprintf(arr,"%s.edges[3*id+2].ipe",d1->name);
codeGenreduxMul1(d1, 2,arr3,arr1,arr2,arr,arr4,arr5);
fprintf(stderr,"return\n");
}
}
}
}
void statement::codeGen(FILE *FP1){
static int n;
int xu,XX;
if(this->sttype==FUNCTION_EBLOCK_STMT ){
NESTED_FOREACH=0;
fprintf(FP1,"}//end fun %d \n",partcodegen);
if(EXTRA_CRBRK){
fprintf(FP1,"\n}\n");
EXTRA_CRBRK=0;
}
//FUN_EBLOCK_RET==1 when partitioned execution happens.
if(FUN_EBLOCK_RET==1)return;
//below segment happens only in partitioned execution.
if(partcodegen==1&&partforeach!=NULL){
partcodegen=2;
fprintf(FP1,"//} //end fun partition");
fprintf(stderr,"//}%d //end fun partition",this->lineno);
fprintf(FP1,"\n");
strcat(partforeach->stdecl->dirrhs->name,"gpu");
tree_decl_stmt *df1=partforeach->stdecl->dirrhs->params;
while(df1!=NULL){
if(df1->dirrhs->libdtype==GRAPH_TYPE || df1->dirrhs->libdtype==POINT_TYPE){
df1->dirrhs->gpu=1;
if(df1->dirrhs->libdtype==POINT_TYPE){
strcpy(df1->dirrhs->name,"id+graph.start_point");
df1->dirrhs->forit=1;
}
}
df1=df1->next;
}

if(partforeach!=NULL){
GLOBAL_ID_FLAG=1;
partforeach->ker=1;
partforeach->codeGen(FP1);
CPUGPU=0;
partforeach=NULL;
partcodegen=0;
GLOBAL_ID_FLAG=0;
return;
}
partcodegen=0;
}
CURR_STMT=this->next;
return;
}
fprintf(stderr,"CODEGEN %d\n",this->sttype);
if(this->barrier==1)fprintf(FP1,"//BARRIER %d %d\n",this->sttype,this->lineno);
printf(" \nstmt line=%d  ",this->lineno);
if(this->sttype==SBLOCK_STMT){//"{"
if( this->prev && this->prev->sttype!=FOREACH_STMT )fprintf(FP1,"{");
if(this->prev==NULL)fprintf(FP1,"NULL");
if(MAINF==1){
if(CPU_COLL_FLAG==1){ 
fprintf(FP1,"Galois::StatManager statManager;\n LonestarStart(argc,argv,name,desc,url);\n");
}else {

for(int ii=0;ii<TOT_GPU_GRAPH;ii++)fprintf(FP1,"\ncudaGetDeviceProperties(&prop%d,%d);",ii,ii%4);
fprintf(FP1," \nalloc_sync_array();//%d\n",TOT_GPU_GRAPH);
}
MAINF=0;
}
}
if(this->sttype==EBLOCK_STMT ){ //"}"
fprintf(FP1,"}//end");
}
if(this->sttype==FOR_EBLOCK_STMT ){//"}"
fprintf(FP1,"}//endfor");
}
if(this->sttype==FOREACH_EBLOCK_STMT){ //"}"
if(FOREACH_FLAG>0)FOREACH_FLAG--;
if(foreachhostflag>0)foreachhostflag--;
fprintf(FP1,"}//foreach");
}
if(this->sttype==RETURN_STMT){
fprintf(FP1,"return ");
if(this->expr1!=NULL)this->expr1->printcode(this->expr1);
fprintf(FP1,";");
}
if(this->sttype==GOTO_STMT){
fprintf(FP1,"goto %s ;",this->name);
}
if(this->sttype==CONTINUE_STMT){
fprintf(FP1,"continue;");
}
if(this->sttype==EMPTY_STMT){
fprintf(FP1,";");
}
if(this->sttype==SWITCH_STMT){
fprintf(FP1,"switch (");
this->expr1->printcode(this->expr1);
fprintf(FP1,")");
if(this->f1!=NULL)this->f1->codeGen(FP1);
}
if(this->sttype==CASE_STMT){
fprintf(FP1,"case ");
this->expr1->printcode(this->expr1);
fprintf(FP1," :");
}
if(this->sttype==DEFAULT_STMT){
fprintf(FP1,"default");
this->expr1->printcode(this->expr1);
fprintf(FP1," :");
}
if(this->sttype==BREAK_STMT){
fprintf(FP1,"break;");
}
if(this->sttype==MACRO_STMT){
fprintf(FP1,"\n#define ");
this->expr1->printcode(this->expr1);
fprintf (FP1, " ");

if(this->f1!=NULL)this->f1->codeGen(FP1);
fprintf(FP1,"\n");
}
if(this->sttype==SECTIONS_STMT){
fprintf(FP1,"#pragma omp parallel sections\n{\n");
if(this->f1!=NULL){
this->f1->print();
}
}
if(this->sttype==SECTION_STMT){
fprintf(FP1,"#pragma omp section \n{\n");
if(this->f1!=NULL)this->f1->print();
}
if(this->sttype==SINGLE_STMT){
char arr[100];
tree_expr *t1=this->expr1;
while(t1 && t1->lhs!=NULL && t1->expr_type!=VAR && t1->lhs->expr_type!=VAR)t1=t1->lhs;
if(t1&& t1->lhs &&  t1->lhs->libdtype==COLLECTION_TYPE){
tree_typedecl *ttp=((dir_decl *)(t1->lhs))->tp1;
dir_decl *dd1=ttp->d1;
fprintf(FP1,"}\nbool backoff=false;\nfor(int i=0;i<%s.size;i++)\n \t ((%s *)(%s.extra))->owner[%s.D_Vec[i]]=id;\n",t1->name,dd1->extra_name,dd1->name,t1->name);
if(t1->lhs->gpu==1)fprintf(FP1,"__gpu_sync(++goal,arrayin,arrayout);\n");
else fprintf(FP1,"#pragma omp barrier\n");
fprintf(FP1,"for(int i=0;i<%s.size;i++){\n \t if(( (%s *   )(%s.extra))->owner[%s.D_Vec[i]]<id){backoff=true;break;}\n \t else if(((%s *)(%s.extra))->owner[%s.D_Vec[i]]>id)((%s *)(%s.extra))->owner[%s.D_Vec[i]]=id; \n}\n",t1->lhs->name,dd1->extra_name,dd1->name,t1->lhs->name,dd1->extra_name,dd1->name,t1->lhs->name,dd1->extra_name,dd1->name,t1->lhs->name);
if(t1->lhs->gpu==1)fprintf(FP1,"__gpu_sync(++goal,arrayin,arrayout);\n");
else fprintf(FP1,"#pragma omp barrier\n");
fprintf(FP1,"if(backoff==0){\n \t for(int i=0;i<%s.size;i++){\n \t \tif(( (%s *   )(%s.extra))->owner[%s.D_Vec[i]]!=id){backoff=true;break;\n \t \t}\n \t}\n}\n",t1->lhs->name,dd1->extra_name,dd1->name,t1->lhs->name);
fprintf(FP1,"%s\nif(backoff==0)  ",barr_after);
}
else {
if(this->f3){statement *tt=this->f3;int elecnt=0; 
while(tt){elecnt++;tt=tt->next;}
fprintf(FP1,"int iiscarr%d[%d];\n",singlearr,elecnt);
tt=this->f3;
elecnt=0;
while(tt){ tree_expr *ttexpr=tt->stassign->rhs; while(ttexpr->lhs && ttexpr->expr_type!=VAR)ttexpr=ttexpr->lhs;
fprintf(FP1,"iiscarr%d[%d]=%s;\n",singlearr,elecnt++,ttexpr->name);
  tt=tt->next;
}
fprintf(FP1,"thrust::sort(iiscarr%d,iiscarr%d+%d);\n",singlearr,singlearr,elecnt);
statement *tt1=this->f3;
for(int i=0;i<elecnt;i++){
fprintf(FP1,"if(atomicCAS( &(");
if(tt1){
tree_expr *ttexpr=tt1->stassign->rhs;
while(ttexpr && ttexpr->lhs && ttexpr->lhs->expr_type!=VAR)ttexpr=ttexpr->lhs;
if(ttexpr && ttexpr->lhs && ttexpr->lhs->expr_type==VAR&& ttexpr->lhs->lhs && ttexpr->lhs->lhs->extra_name)fprintf(FP1,"((%s *)(%s.extra))->%s",ttexpr->lhs->lhs->extra_name,ttexpr->lhs->lhs->parent->name,ttexpr->rhs->name);
tree_expr *texp1= tt1->stassign->rhs;
while(texp1->lhs!=ttexpr)texp1=texp1->lhs;
if(texp1!=NULL)
fprintf(FP1,"[iiscarr%d[%d]].%s),0,1)==0)\n",singlearr,i,texp1->rhs->name);
else fprintf(FP1,"[iiscarr%d[%d]]),0,1)==0)\n",singlearr,i);
}
tt1=tt1->next;
}
singlearr++;
}
else {
for(int i=0;i<100;i++)arr[i]='\0';
if(Gkernel)fprintf(FP1,"if(atomicCAS( ");//CUDA CAS
else fprintf(FP1,"if(__sync_val_compare_and_swap( ");//GCC CAS
this->expr1->printcode1(this->expr1,arr);
fprintf(FP1,"&(%s),0,1)==0)",arr);
}
}
if(this->f1!=NULL){this->f1->codeGen(FP1);
}
if(this->f2!=NULL){
fprintf(FP1,"else ");
this->f2->codeGen(FP1);
}
}
if(this->sttype==IF_STMT){
if(this->f1 && this->f1->stassign && this->f1->stassign->asstype==RRADD_ASSIGN){
codegenRedux(this);
this->f1->next->codeGen(FP1);
return;
}
if(this->f1 && this->f1->stassign && this->f1->stassign->asstype==RRMUL_ASSIGN){
codegenReduxMul(this);
this->f1->next->codeGen(FP1);
return;
}
else {
int ff;
if(Gkernel==0){
int val=check_gpu_expr(this->expr1->lhs);//check for gpu variable in CPU code , kernel==0
val=val+check_gpu_expr(this->expr1->rhs);
if(val>=1 && Gkernel==0){
if(val!=5){print_gpu_if_stmt(this,val);
//this->f1->next->codeGen();
return;
}
else{//val==5 for partitioned execution
//print_partitioned_condition(this->expr1);
}
}
if(val!=5){this->expr1->findgpuvars(this->expr1);//condition should be !(val>=1). okay for present.

fprintf(FP1,"if( ");
char arr[100];
for(int i=0;i<100;i++)arr[i]='\0';
this->expr1->printcode1(this->expr1,arr);
fprintf(FP1,"%s",arr);
fprintf(FP1," )");
}
}
if(Gkernel>0){//CUDA CODE. no check for gpu vars needed.
fprintf(FP1,"if( ");
char arr[100];
for(int i=0;i<100;i++)arr[i]='\0';
this->expr1->printcode1(this->expr1,arr);
fprintf(FP1,"%s",arr);
fprintf(FP1," )");
}


if(this->f1!=NULL)this->f1->codeGen(FP1);
if(this->f2!=NULL){
fprintf(FP1,"\nelse ");
if(this->f2->nodetype==TREE_IF){
fprintf(FP1,"if ");
//return;
}
this->f2->codeGen(FP1);
printf("END\n");
}
}
}
if(this->sttype==ASSIGN_STMT){
if(this->stassign->lhs && this->stassign->rhs)fprintf(stderr,"XXX %d  %d \n",this->stassign->lhs->expr_type,this->stassign->rhs->expr_type);
if(this->stassign->lhs && this->stassign->rhs &&(this->stassign->asstype==RRADD_ASSIGN|| this->stassign->rhs &&this->stassign->asstype==RRMUL_ASSIGN)){
if(this->stassign->lhs && this->stassign->rhs &&this->stassign->asstype==RRADD_ASSIGN){
fprintf(stderr,"\n//REDUX OPERATION\n");
codegenRedux(this);
}
if(this->stassign->lhs && this->stassign->rhs &&this->stassign->asstype==RRMUL_ASSIGN){
codegenReduxMul(this);
} }
else {
int te=-1;
if(Gkernel==0) {te=check_gpu(this);//check for gpu variables in assignment statement. kernel==0
if(this->stassign->lhs!=NULL&& this->stassign->lhs->expr_type!=VAR)this->stassign->lhs->findgpuvars(this->stassign->lhs);
if(this->stassign->rhs!=NULL && this->stassign->rhs->expr_type!=VAR)this->stassign->rhs->findgpuvars(this->stassign->rhs);
}
if(te>=1 && this->stassign->lhs!=NULL){
int type1=this->stassign->lhs->expr_type;
int type=this->stassign->rhs->expr_type;
  
	if( type==LT || type==GT|| type==LE || type==GE|| type==LE || type==EQ || type==NEQ|| type==ADD &&type!=STRUCTREF &&	type1!=STRUCTREF && type1!=VAR){

char arr1[200],arr2[200];
for(int i=0;i<200;i++)arr1[i]=arr2[i]='\0';
if(this->stassign->lhs!=NULL)this->stassign->lhs->printcode1(this->stassign->lhs,arr1);
fprintf(FP1,"%s",arr1);
print_assign_type(FP1,this->stassign->asstype);
if(this->stassign->rhs!=NULL)this->stassign->rhs->printcode1(this->stassign->rhs,arr2);
fprintf(FP1,"%s;\n",arr2);
}
if(type1==ARRREF && this->stassign->lhs->lhs->libdtype==COLLECTION_TYPE){
char arr1[200],arr2[200];
for(int i=0;i<200;i++)arr1[i]=arr2[i]='\0';
if(this->stassign->lhs!=NULL)this->stassign->lhs->printcode1(this->stassign->lhs,arr1);
fprintf(FP1,"%s",arr1);
print_assign_type(FP1,this->stassign->asstype);
if(this->stassign->rhs!=NULL)this->stassign->rhs->printcode1(this->stassign->rhs,arr2);
fprintf(FP1,"%s;\n",arr2);

}


else{ print_gpu_assign_stmt(this,te);
}
}  
else {

char arr1[200],arr2[200];
for(int i=0;i<200;i++)arr1[i]=arr2[i]='\0';
if(this->stassign->rhs==NULL&&this->sttype==ASSIGN_STMT){if(this->stassign->lhs!=NULL/* && this->stassign->lhs->name!=NULL*/){ this->stassign->lhs->printcode1(this->stassign->lhs,arr1);fprintf(FP1,"%s//lhs null",arr1);
}}
if(this->stassign->asstype==DOWHILE_EXPR){
fprintf(FP1,"while ( "); 
this->stassign->rhs->printcode1(this->stassign->rhs,arr2);
fprintf(FP1,"%s",arr2);
fprintf(FP1,");");
}
if(this->stassign->asstype!=WHILE_EXPR && this->stassign->asstype!=-1){
//if(this->stassign->rhs==NULL || this->stassign->rhs->expr_type==EXPR){this->stassign->print1(1);}
}
fprintf(stderr,"PRINT1");
if(this->stassign->rhs!=NULL && this->stassign->rhs->expr_type!=FUNCALL){
int repli=0;
tree_expr *Tu1;
fprintf(stderr,"XXX\n");
if(this->stassign->lhs!=NULL/* && this->stassign->lhs->name!=NULL*/){ 
fprintf(stderr,"xx%dxx ",this->lineno);
this->stassign->lhs->printcode1(this->stassign->lhs,arr1);
fprintf(stderr,"xx%dxx ",this->lineno);
Tu1=this->stassign->lhs;
if(Tu1!=NULL && Tu1->expr_type!=ARRREF){ printf("XX XX\n ");if(Tu1->lhs==NULL)printf(" LHS NULL");while(Tu1 && Tu1->expr_type!=VAR){ printf("looXX%d ",this->lineno);Tu1=Tu1->lhs;}printf("outxx %d ",this->lineno);
if(Tu1->expr_type==VAR)Tu1=Tu1->lhs;
 if((Tu1!=NULL) && (Tu1->parent==NULL) &&Tu1->libdtype!=-1   && this->stassign->lhs->expr_type!=STRUCTREF)repli=1;
 if((Tu1!=NULL) && (Tu1->parent!=NULL) &&Tu1->dtype==STRUCT_TYPE   && this->stassign->lhs->expr_type!=STRUCTREF && this->stassign->rhs->expr_type==STRUCTREF){
fprintf(FP1,"// this is a access where rhs needs to b converted to int \n");
dir_decl *dpar=Tu1->parent;
struct extra_ppts *epp =dpar->ppts;
while(epp!=NULL){ if(!strcmp(epp->name,this->stassign->rhs->rhs->name)&& strlen(epp->name)==strlen(this->stassign->rhs->rhs->name))break; epp=epp->next;}
if(epp!=NULL) {
fprintf(FP1,"%s=",Tu1->name); assign_stmt *ass= this->stassign->rhs->rhs->earr_list; if(ass!=NULL && ass->rhs!=NULL)fprintf(FP1,"%s;\n",ass->rhs->name);
//fprintf(FP1,"%s",Tu1->name);print_assign_type(FP1,this->stassign->asstype);assign_stmt *ass= this->stassign->rhs->rhs->earr_list; if(ass!=NULL && ass->rhs!=NULL)fprintf(FP1,"%s;\n",ass->rhs->name);
repli=2;
}
}
}
if(repli==0){fprintf(FP1,"%s",arr1);
//fprintf(FP1,"=");
print_assign_type(FP1,this->stassign->asstype);

printf("lhs=%s",arr1);
}
if(repli==1 && Tu1->libdtype==COLLECTION_TYPE&&Tu1->gpu==0){fprintf(FP1,"%s=",arr1);}
//this->stassign->printcode1(arr1,1);
//fprintf(FP1,"%s",arr1);
}
if(this->stassign->rhs->expr_type==STRUCTREF && !strcmp(this->stassign->rhs->rhs->name,"OrderByIntValue")){
dir_decl *d1=this->stassign->rhs->lhs->lhs;
symtable *symt=d1->stable;
tree_typedecl *tt1=d1->tp1;
tree_expr *expr=this->stassign->rhs->rhs->arglist->rhs;
tree_expr *expr1=this->stassign->rhs->rhs->arglist->next->rhs;
if(fsetpos(FP1,&mainfpos)==-1)fprintf(FP1,"ERROR SET FPOS\n");
char *temp=malloc(sizeof(char)*3500);
char *old=malloc(sizeof(char)*9000);
for(int i=0;i<9000;i++)old[i]='\0';
fread(old,9000,1,FP1);
if(fsetpos(FP1,&mainfpos)==-1)fprintf(FP1,"ERROR SET FPOS\n");
char *temp1=malloc(sizeof(char)*3500);
for(int i=0;i<3500;i++)temp[i]='\0';
for(int i=0;i<3500;i++)temp1[i]='\0';
int deltapow=intpow(2,expr1->ival);
sprintf(temp1,"%sIndexer:public std::unary_function<%s,unsigned int> {\n unsigned int operator()(const %s &val)const {\n unsigned int t=val.%s/%d;\n return t;\n}\n};\n",tt1->name,tt1->name,tt1->name,expr->name,deltapow);
strcat(temp,temp1);
for(int i=0;i<3500;i++)temp1[i]='\0';
//this print should inside current function
sprintf(temp1,"using namespace Galois::WorkList;\n typedef dChunkedFIFO<64> Chunk;\n typedef OrderedByIntegerMetric<%sIndexer,Chunk,10> OBIM;\n",tt1->name);
strcat(temp,temp1);
fprintf(FP1," %s  \n",temp);
fgetpos(FP1,&mainfpos);
fprintf(FP1," %s\n ",old);
//mainfpos=mainfpos+strlen(temp);
for(int i=0;i<3500;i++)temp1[i]='\0';
for(int i=0;i<3500;i++)temp[i]='\0';
}
if(this->stassign->rhs->pflag==0){
if(this->stassign->rhs->expr_type==STRUCTREF &&!(!strcmp(this->stassign->rhs->rhs->name,"addPointProperty") || !strcmp(this->stassign->rhs->rhs->name,"addEdgeProperty")||!strcmp(this->stassign->rhs->rhs->name,"addProperty")||(  !strcmp(this->stassign->rhs->rhs->name,"OrderByIntValue") )))  {
char arr[200];
for(int i=0;i<200;i++)arr[i]='\0';
this->stassign->rhs->printcode1(this->stassign->rhs,arr);
dir_decl *d1=this->stassign->rhs;
if(d1->parent!=NULL && repli==1){
fprintf(FP1,"int repli%d=%s;\n",replicnt,arr);
 if((Tu1!=NULL) && (Tu1->parent==NULL) &&Tu1->libdtype==POINT_TYPE ){
repli=1;
fprintf(FP1,"%s.x=%s.points[%srepli%d].%s;\n",arr1,d1->parent->name,pdim_string[pdim_size],replicnt,union_string[union_type]);
if(pdim_size>=2)fprintf(FP1,"%s.y=%s.points[%srepli%d+1].%s;\n",arr1,d1->parent->name,pdim_string[pdim_size],replicnt,union_string[union_type]);
if(pdim_size>=3)fprintf(FP1,"%s.z=%s.points[%srepli%d+2].%s;\n",arr1,d1->parent->name,pdim_string[pdim_size],replicnt,union_string[union_type]);
replicnt++;
}

}else {
if(repli==0){fprintf(FP1,"%s",arr2);
fprintf(FP1,"%s; ",arr);
}
}
if(extra_array[0]!='\0'){

fprintf(FP1,"%s\n",extra_array);
for(int i=0;i<100;i++)extra_array[i]='\0';
}
}
if(this->stassign->rhs->expr_type!=STRUCTREF){this->stassign->rhs->printcode1(this->stassign->rhs,arr2);
fprintf(FP1,"%s",arr2);
if((this->stassign->rhs->expr_type!=GET_TYPE) &&( this->stassign->rhs->expr_type!=EXPR)&& this->stassign->semi==1)fprintf(FP1,"; ");
}
}

}
if(this->stassign->rhs!=NULL && this->stassign->rhs->expr_type==CAST_EXPR){
//char arr[200];
//for(int i=0;i<200;i++)arr[i]='\0';
//if(this->stassign->lhs!=NULL){this->stassign->lhs->printcode1(this->stassign->lhs,arr);
//fprintf(FP1,"%s=",arr);
//for(int i=0;i<200;i++)arr[i]='\0';
//}
//char tparr[100];
//for(int i=0;i<100;i++)tparr[i]='\0';
//if(this->stassign->rhs->tpname!=NULL)this->stassign->rhs->tpname->printcode(tparr);
//this->stassign->rhs->printcode1(this->stassign->rhs,arr);
//fprintf(FP1,"(%s)%s;\n",tparr,arr);


}
if(this->stassign->rhs!=NULL && this->stassign->rhs->expr_type==FUNCALL){
//tree_expr *f1=this->stassign->rhs;
char arr[200];
for(int i=0;i<200;i++)arr[i]='\0';
if(this->stassign->lhs!=NULL){this->stassign->lhs->printcode1(this->stassign->lhs,arr);
fprintf(FP1,"%s",arr);
print_assign_type(FP1,this->stassign->asstype);
for(int i=0;i<200;i++)arr[i]='\0';
}
char tparr[100];
for(int i=0;i<100;i++)tparr[i]='\0';
if(this->stassign->rhs->tpname!=NULL)this->stassign->rhs->tpname->printcode(tparr);
//kerdec15=Gkernel;
this->stassign->rhs->printcode1(this->stassign->rhs,arr);
fprintf(FP1,"%s%s;//rhs not null\n",tparr,arr);
//fprintf(FP1,"; ");
}
if(this->stassign && this->stassign->lhs==NULL &&this->stassign->rhs->expr_type==GET_TYPE&& this->stassign->rhs->rhs!=NULL){
int gf,hf;
//FILE *fp1=fopen("global.h","a+");
fp1=FPG;
dir_decl *t1=this->stassign->rhs->rhs->nextv;
dir_decl *t11=this->stassign->rhs->lhs;
//if(t11!=NULL)printf(" GET TYPE WITH CLONE OF %s\n",t11->name); 
while(t1!=NULL){if (t1->gpu==1)gf=1;if(t1->gpu==0)hf=1; if(hf&&gf) break; t1=t1->nextv;}
if(gf==1){ fprintf(FP1,"GGraph ");
int cnt=0;
t1=this->stassign->rhs->rhs->nextv;
while(t1!=NULL){if (t1->gpu==1){if(cnt>0)fprintf(FP1,","); fprintf(FP1,"%s",t1->name);cnt++; }t1=t1->nextv;
//++TOT_GPU_GRAPH;
}
fprintf(FP1,";\n");
}
if(hf==1){ fprintf(FP1,"HGraph ");
int cnt=0;
t1=this->stassign->rhs->rhs->nextv;
while(t1!=NULL){if (t1->gpu==0){if(cnt>0)fprintf(FP1,","); fprintf(FP1,"%s",t1->name);cnt++; }t1=t1->nextv;}
fprintf(FP1,";");
}
t1=this->stassign->rhs->rhs->nextv;
while(t1!=NULL){if (t1->ppts!=NULL){
if(t1->gpu==0 ){
fprintf(FP,"void alloc_extra_%s(HGraph &%s,int npoints) {\n ",t1->name,t1->name);
t1->extra_fun=(char *)malloc(sizeof(char)*100);
sprintf(t1->extra_fun,"alloc_extra_%s(%s,%s.npoints);",t1->name,t1->name,t1->name);
}
if(t1->gpu==1 ){
struct extra_ppts *ep=t1->ppts;
fprintf(FP,"void copy%scurrentsize(GGraph &graph){\n cudaMemcpyToSymbol(falc%spoint,&(graph.npoints),0,sizeof(int),cudaMemcpyHostToDevice);\ncudaMemcpyToSymbol(falc%sedge,&(graph.nedges),0,sizeof(int),cudaMemcpyHostToDevice);\n",t1->name,t1->name,t1->name);
while(ep!=NULL){
if(ep->t1->libdatatype==G_P_TYPE){
fprintf(FP,"%s temp;\n",t1->extra_name);
fprintf(FP,"cudaMemcpy(&temp,graph.extra,sizeof(%s ),cudaMemcpyDeviceToHost);\n",t1->extra_name,t1->name,t1->extra_name);
fprintf(FP,"cudaMemcpyToSymbol(&falc%s%s,&(temp.n%s),0,sizeof(int),cudaMemcpyHostToDevice);\n",t1->name,ep->name,ep->name);
}
ep=ep->next;
}
fprintf(FP,"}\n");

fprintf(fp1,"__device__ int falc%spoint,falc%sedge;\n",t1->name,t1->name);
fprintf(FP,"__device__ int falc%saddpointgraph(){\n int t1=atomicAdd(&falc%spoint,1);\n return t1;\n}\n",t1->name,t1->name);
fprintf(FP,"__device__ int falc%saddedgegraph(){\n int t1=atomicAdd(&falc%sedge,1);\n return t1;\n}\n",t1->name,t1->name);
fprintf(FP,"void alloc_extra_%s(GGraph &%s,int flag,int npoints) {/*\ncudaSetDevice(%d);*/\n ",t1->name,t1->name,t1->dev_no);
t1->extra_fun=(char *)malloc(sizeof(char)*100);
sprintf(t1->extra_fun,"alloc_extra_%s(%s,%sflag,%s.npoints);",t1->name,t1->name,t1->name,t1->name);
}
struct extra_ppts *ep=t1->ppts;
if(t1->gpu==0 && t1->ppts!=NULL){
fprintf(FP,"%s.extra)=(%s *)malloc(sizeof(%s)) ;\n",t1->name,t1->extra_name,t1->extra_name);
}
if(t1->gpu==1 && t1->ppts!=NULL){
fprintf(FP,"%s temp;\n",t1->extra_name);
fprintf(FP,"if(flag==0)cudaMalloc((void **)&(%s.extra),sizeof(%s));\n cudaMemcpy(&temp,((%s *)(%s.extra)),sizeof(%s),cudaMemcpyDeviceToHost);\n",t1->name,t1->extra_name,t1->extra_name,t1->name,t1->extra_name);
}
while(ep!=NULL){
if(t1->gpu==0){
if(ep->t1->libdatatype==E_P_TYPE ){
fprintf(FP,"((%s *)%s.extra)->%s=(%s *)malloc(sizeof(%s) * %s.nedges%s) ;\n",t1->extra_name,t1->name,ep->name,ep->t1->name,ep->t1->name,t1->name,morphalloc_string[morph_size]);
}
if(ep->t1->libdatatype==P_P_TYPE){
fprintf(FP,"/**/((%s *)%s.extra)->%s=(%s *)malloc(sizeof(%s) * %s.npoints%s) ;\n",t1->extra_name,t1->name,ep->name,ep->t1->name,ep->t1->name,t1->name,morphalloc_string[morph_size]);
}
if(ep->t1->libdatatype==G_P_TYPE){
if(ep->val2!=-1){
fprintf(FP,"((%s *)%s.extra)->%s=(%s *)malloc(sizeof(%s) * ((%s *)(%s.extra))->n%s%s) ;\n",t1->extra_name,t1->name,ep->name,ep->t1->name,ep->t1->name,t1->extra_name,t1->name,ep->name,morphalloc_string[morph_size]);
}
else {
fprintf(FP,"((%s *)%s.extra)->%s=(%s *)malloc(sizeof(%s) * ((%s *)(%s.extra))->n%s%s) ;\n",t1->extra_name,t1->name,ep->name,ep->t1->name,ep->t1->name,t1->extra_name,t1->name,ep->name,morphalloc_string[morph_size]);

}
}
if(ep->t1->libdatatype==G_P_TYPE){

fprintf(fp1," int falc%s%s;\n  int pn%s%s;\n  int max%s%s;\n",t1->name,ep->name,t1->name,ep->name,t1->name,ep->name);

}
}
else {

if(ep->t1->libdatatype==E_P_TYPE ){
fprintf(FP,"/**/cudaMalloc((void **)&( temp.%s),sizeof(%s)* %s.nedges%s); \n",ep->name,ep->t1->name,t1->name,morphalloc_string[morph_size]);
}
if(ep->t1->libdatatype==P_P_TYPE){
fprintf(FP,"cudaMalloc( (void **)&(temp.%s),sizeof(%s)* %s.npoints%s); \n",ep->name,ep->t1->name,t1->name,morphalloc_string[morph_size]);
}
if(ep->t1->libdatatype==G_P_TYPE){
if(ep->t1->name!=NULL)fprintf(FP,"cudaMalloc((void **)&( temp.%s),sizeof(%s)* temp.n%s%s); \n",ep->name,ep->t1->name,ep->name,morphalloc_string[morph_size]);
if(ep->t1->name!=NULL)fprintf(FP,"cudaMalloc((void **)&( temp.owner),sizeof(int)* temp.n%s%s); \n",ep->name,morphalloc_string[morph_size]);

fprintf(fp1,"__device__ int falc%s%s;\n __device__ int pn%s%s;\n __device__ int max%s%s;\n",t1->name,ep->name,t1->name,ep->name,t1->name,ep->name);

}
}
ep=ep->next;}
if(t1->gpu==1 && t1->ppts!=NULL){
fprintf(FP,"%scudaMemcpy(%s.extra,&temp,sizeof(%s),cudaMemcpyHostToDevice)%s%d%s\n",cpy[0],t1->name,t1->extra_name,cpy[1],errcnt++,cpy[2]);
}
fprintf(FP,"cudaSetDevice(0);\n}\n");/*realloc_graph1(t1);lock_graph1(t1);*/}
 t1=t1->nextv;}

//fclose(fp1);
}
}
}
}
if(this->sttype==WHILE_STMT){
fprintf(FP1,"while(");
char temp[100];
for(int i=0;i<100;i++)temp[i]='\0';
this->expr1->printcode1(this->expr1,temp);
fprintf(FP1,"%s) ",temp);
if(this->stassign!=NULL){this->stassign->rhs->printcode(this->stassign->rhs);fprintf(FP1,";");}
else fprintf(FP1," {");
fprintf(FP1," ");
}
if(this->sttype==DOWHILE_STMT){
fprintf(FP1,"do {\n");
}
if(this->sttype==DOWHILEEXPR_STMT){
fprintf(FP1,"while ( ");
this->expr1->printcode(this->expr1);
fprintf(FP1,");");
}
if(this->sttype==FOREACH_STMT ){
int it=this->itr;
dir_decl *d2=this->expr2->lhs;
fprintf(stderr,"FOREACHSTATEMENT\n");
if(Gkernel>=1 ){
//print_foreach(this);
if(this->stassign!=NULL) {char temp[200];
for(int i=0;i<200;i++)temp[i]='\0';
this->stassign->rhs->printcode1(this->stassign->rhs,temp);fprintf(FP1,"%s;///foreach\n",temp);}
}
else
 {
if(d2==NULL)fprintf(FP1,"NULL");
if(d2!=NULL &&d2->libdtype==GRAPH_TYPE){
fprintf(stderr,"XX");
partcodegen=1;
this->expr2->kernel=0;
d2->gpu=0;
this->expr1->lhs->libdtype=POINT_TYPE;
}
if(it==0){
this->expr1->lhs->libdtype=POINT_TYPE;

if(1) {
if(d2->gpu==1 ){
if(this->feb==0){
if(this->stassign!=NULL && this->stassign->rhs->expr_type==FUNCALL){
dir_decl *setvar=NULL;
fprintf(FP1,"cudaSetDevice(%d);\nfor(int kk=0;kk<%s.npoints;kk+=%spointkernelblocks*TPB%d){ \n%s<<<%spointkernelblocks,TPB%d>>>(",d2->dev_no,d2->name,d2->name,d2->dev_no,this->stassign->rhs->name,d2->name,d2->dev_no);

assign_stmt *ta=this->stassign->rhs->arglist;
ta=ta->next;
int cnt=0;
while(ta!=NULL && ta->next!=NULL){
fprintf(FP1,"%s,",ta->rhs->name);
if(ta->rhs->lhs->libdtype==SET_TYPE && ta->rhs->lhs->gpu==1)setvar=ta->rhs->lhs;
ta=ta->next;
}
if(ta!=NULL){
if(ta->rhs->lhs==d2)
 fprintf(FP1,"%s,kk);\n",ta->rhs->name);
if(ta->rhs->lhs->libdtype==SET_TYPE )setvar=ta->rhs->lhs;
}
fprintf(FP1,"}\ncudaDeviceSynchronize();\ncudaSetDevice(0);\n");
struct UNION_KER *tempu=head_position;
while(tempu!=NULL){
if(!strcmp(tempu->name,this->stassign->rhs->name) && strlen(tempu->name)==strlen(this->stassign->rhs->name)) {

//}
if(setvar!=NULL){fprintf(FP1,"cudaSetDevice(%d);\nfor(int kk=0;ii<%s.tot_size;kk+=%spointkernelblocksize*TPB%d){\nfindset<<<%spointkernelblcoksize,TPB%d>>>( %s,kk);\n",d2->dev_no,d2->name,d2->name,d2->dev_no,d2->name,d2->dev_no,setvar->name);
fprintf(FP1,"}\ncudaDeviceSynchronize();cudaSetDevice(0);\n");
break;
}
}
tempu=tempu->next;

}
}
if(this->stassign!=NULL &&this->stassign->lhs!=NULL && this->stassign->lhs->expr_type==STRUCTREF){
fsetpos(FP1, &position);
fpos_t position1=position;
char tempar[10000];
for(int i=0;i<10000;i++)tempar[i]='\0';
int fpos=0;
char fc;
while(fscanf(FP1,"%c",&fc)!=EOF)tempar[fpos++]=fc;
tempar[fpos]='\0';
fsetpos(FP1, &position1);
char temparr[20000],temparr1[2000],temparr2[2000];
for(int i=0;i<20000;i++)temparr[i]='\0';
for(int i=0;i<2000;i++)temparr1[i]=temparr2[i]='\0';
sprintf(temparr,"__global__ void falctfun%d(GGraph %s,int FALCX){\n %s if (id < %s.npoints) {\n ",tempfun,d2->name,threadstmt,d2->name);
dir_decl *dt1;
tree_expr *tex1=this->stassign->lhs;
while(tex1->lhs!=NULL)tex1=tex1->lhs;
dt1=(dir_decl *)tex1;
dt1->extra_name1=malloc(sizeof(char)*100);
strcpy(dt1->extra_name1,"id");

this->stassign->lhs->printcode1(this->stassign->lhs,temparr1);
strcat(temparr,temparr1);
fprintf(FP1,"%s",temparr);
this->stassign->rhs->printcode1(this->stassign->rhs,temparr2);
fprintf(FP1,"=%s;\n}\n}\n",temparr2);
fgetpos(FP1,&position);
fprintf(FP1,"%s",tempar);
fprintf(FP1,"cudaSetDevice(%d);\nfor(int kk=0;kk<%s.npoints;kk+=%spointkernelblocksize){\n falctfun%d<<<%spointkernelblocksize,TPB%d>>>(%s,kk);\n}",d2->dev_no,d2->name,d2->name,tempfun++,d2->name,d2->dev_no,d2->name);
fprintf(FP1,"\ncudaDeviceSynchronize();\ncudaSetDevice(0);\n");
}
}
}
if(d2->gpu==0) {
foreachhostflag++;


if(this->feb==0){
dir_decl *setvar=NULL;
if(this->stassign!=NULL && this->stassign->rhs->expr_type==FUNCALL){
fprintf(FP1,"#pragma omp parallel for   num_threads(12)\nfor(int i=0;i<%s.npoints;i++)%s(i,",d2->name,this->stassign->rhs->name);
assign_stmt *ta=this->stassign->rhs->arglist;
ta=ta->next;
if(d2->gpugpu==0){
while(ta!=NULL && ta->next!=NULL){
char temparr[200];
for(int i=0;i<200;i++)temparr[i]='\0';
if(ta->rhs->lhs!=d2)ta->rhs->printcode1(ta->rhs,temparr);
fprintf(FP1,"%s,",temparr);
if(ta->rhs->lhs->libdtype==SET_TYPE && ta->rhs->lhs->gpu!=1)setvar=ta->rhs->lhs;
ta=ta->next;
}
if(ta!=NULL){

char temparr[200];
for(int i=0;i<200;i++)temparr[i]='\0';
if(ta->rhs->lhs!=d2)ta->rhs->printcode1(ta->rhs,temparr);
fprintf(FP1,"%s);/xx",temparr);
if(ta->rhs->lhs && ta->rhs->lhs->libdtype==SET_TYPE )setvar=ta->rhs->lhs;
}

}
}
struct UNION_KER *tempu=head_position;
if(tempu==NULL)printf(" ***********NO UNION KERNEL\n");
while(tempu!=NULL){
printf("not null-------- %s",tempu->name);
if(!strcmp(tempu->name,this->stassign->rhs->name) && strlen(tempu->name)==strlen(this->stassign->rhs->name)) {
if(setvar!=NULL){fprintf(FP1,"\n#pragma omp parallel for num_threads(12) \n for(int i=0;i<%s.tot_size;i++)hfindset(i, %s);\n",setvar->name,setvar->name);
break;
}
}
tempu=tempu->next;
}
if(this->stassign!=NULL &&this->stassign->lhs!=NULL && this->stassign->lhs->expr_type==STRUCTREF){
fsetpos(FP1, &position);
fpos_t position1=position;
char tempar[1000];
for(int i=0;i<1000;i++)tempar[i]='\0';
int fpos=0;
char fc;
while(fscanf(FP1,"%c",&fc)!=EOF)tempar[fpos++]=fc;
tempar[fpos]='\0';
fsetpos(FP1, &position1);
char temparr[2000],temparr1[200],temparr2[200];
for(int i=0;i<2000;i++)temparr[i]='\0';
for(int i=0;i<200;i++)temparr1[i]=temparr2[i]='\0';
sprintf(temparr,"void falctfun%d(int &id,HGraph &%s){\n  if (id < %s.npoints) {\n ",tempfun,d2->name,d2->name);
dir_decl *dt1;
tree_expr *tex1=this->stassign->lhs;
while(tex1->lhs!=NULL)tex1=tex1->lhs;
dt1=(dir_decl *)tex1;
dt1->extra_name1=malloc(sizeof(char)*100);
strcpy(dt1->extra_name1,"id");

this->stassign->lhs->printcode1(this->stassign->lhs,temparr1);
strcat(temparr,temparr1);
fprintf(FP1,"%s",temparr);
this->stassign->rhs->printcode1(this->stassign->rhs,temparr2);
fprintf(FP1,"=%s;\n}\n}\n",temparr2);
fgetpos(FP1,&position);
fprintf(FP1,"%s",tempar);
fprintf(FP1,"#pragma omp parallel for   num_threads(12)\nfor(int id=0;id<%s.npoints;id++)falctfun%d( id,%s);",d2->name,tempfun++,d2->name);
}

}
else {
fprintf(FP1,"#pragma omp parallel for   num_threads(12)\n for(int i=0;i<%s.npoints;i++){",d2->name);
dir_decl *d3=this->expr1->lhs;
d3->extra_name1=malloc(sizeof(char)*100);
strcpy(d3->extra_name1,"i");
d3->extra_name=malloc(sizeof(char)*100);
strcpy(d3->extra_name,d2->extra_name);
d3->parent=d2;
}


}

}
}

else if(it==1){
this->expr1->lhs->libdtype=EDGE_TYPE;
if(d2->gpu==1){
if(this->feb==0){
if(this->stassign!=NULL && this->stassign->rhs->expr_type==FUNCALL){
fprintf(FP1,"cudaSetDevice(%d);\nfor(int kk=0;kk<%s.edges;kk+=%sedgekernelblocks*TPB%d){ \n%s<<<%sedgekernelblocks,TPB%d>>>(",d2->dev_no,d2->name,d2->name,d2->dev_no,this->stassign->rhs->name,d2->name,d2->dev_no);
assign_stmt *ta=this->stassign->rhs->arglist;
ta=ta->next;
while(ta!=NULL && ta->next!=NULL){
fprintf(FP1,"%s,",ta->rhs->name);
ta=ta->next;
}
if(ta!=NULL)fprintf(FP1,"%s,kk);",ta->rhs->name);
fprintf(FP1,"}\ncudaDeviceSynchronize();\ncudaSetDevice(0);\n");
}
if(this->stassign!=NULL &&this->stassign->lhs!=NULL && this->stassign->lhs->expr_type==STRUCTREF){
fsetpos(FP1, &position);
fpos_t position1=position;
char tempar[1000];
for(int i=0;i<1000;i++)tempar[i]='\0';
int fpos=0;
char fc;
while(fscanf(FP1,"%c",&fc)!=EOF)tempar[fpos++]=fc;
tempar[fpos]='\0';
fsetpos(FP1, &position1);
char temparr[2000],temparr1[200],temparr2[200];
for(int i=0;i<2000;i++)temparr[i]='\0';
for(int i=0;i<200;i++)temparr1[i]=temparr2[i]='\0';
sprintf(temparr,"__global__ void falctfun%d(GGraph %s,int FALCX){\n %s if (id < %s.nedges) {\n ",tempfun,d2->name,threadstmt,d2->name);
dir_decl *dt1;
tree_expr *tex1=this->stassign->lhs;
while(tex1->lhs!=NULL)tex1=tex1->lhs;
dt1=(dir_decl *)tex1;
dt1->extra_name1=malloc(sizeof(char)*100);
strcpy(dt1->extra_name1,"id");

this->stassign->lhs->printcode1(this->stassign->lhs,temparr1);
strcat(temparr,temparr1);
fprintf(FP1,"%s",temparr);
this->stassign->rhs->printcode1(this->stassign->rhs,temparr2);
fprintf(FP1,"=%s;\n}\n}\n",temparr2);
fgetpos(FP1,&position);
fprintf(FP1,"%s",tempar);
fprintf(FP1,"cudaSetDevice(%d);\nfor(int kk=0;kk<%s.nedges;kk+=%sedgekernelblocksize){\n falctfun%d<<<%sedgekernelblocksize,TPB%d>>>(%s,kk);\n}",d2->dev_no,d2->name,d2->name,tempfun++,d2->dev_no,d2->name,d2->name);
fprintf(FP1,"\ncudaDeviceSynchronize();\ncudaSetDevice(0);\n");
}
}
}

else {

foreachhostflag++;

if(this->feb==0){
if(this->stassign!=NULL && this->stassign->rhs->expr_type==FUNCALL){
fprintf(FP1,"#pragma omp parallel for num_threads(12)\n for(int i=0;i<%s.nedges;i++)%s(i,",d2->name,this->stassign->rhs->name);
assign_stmt *ta=this->stassign->rhs->arglist;
ta=ta->next;
while(ta!=NULL && ta->next!=NULL){
fprintf(FP1,"%s,",ta->rhs->name);
ta=ta->next;
}
if(ta!=NULL)fprintf(FP1,"%s);",ta->rhs->name);
}
if(this->stassign!=NULL &&this->stassign->lhs!=NULL && this->stassign->lhs->expr_type==STRUCTREF){
fprintf(FP1,"FEB 0L\n");
fsetpos(FP1, &position);
fpos_t position1=position;
char tempar[1000];
for(int i=0;i<1000;i++)tempar[i]='\0';
int fpos=0;
char fc;
while(fscanf(FP1,"%c",&fc)!=EOF)tempar[fpos++]=fc;
tempar[fpos]='\0';
fsetpos(FP1, &position1);
char temparr[2000],temparr1[200],temparr2[200];
for(int i=0;i<2000;i++)temparr[i]='\0';
for(int i=0;i<200;i++)temparr1[i]=temparr2[i]='\0';
sprintf(temparr,"void falctfun%d(int &id,HGraph &%s){\n  if (id < %s.npoints) {\n ",tempfun,d2->name,d2->name);
dir_decl *dt1;
tree_expr *tex1=this->stassign->lhs;
while(tex1->lhs!=NULL)tex1=tex1->lhs;
dt1=(dir_decl *)tex1;
dt1->extra_name1=malloc(sizeof(char)*100);
strcpy(dt1->extra_name1,"id");

this->stassign->lhs->printcode1(this->stassign->lhs,temparr1);
strcat(temparr,temparr1);
fprintf(FP1,"%s",temparr);
this->stassign->rhs->printcode1(this->stassign->rhs,temparr2);
fprintf(FP1,"=%s;\n}\n}\n",temparr2);
fgetpos(FP1,&position);
fprintf(FP1,"%s",tempar);
fprintf(FP1,"#pragma omp parallel for num_threads(12) \nfor(int id=0;id<%s.nedegs;id++)falctfun%d( id,%s);",d2->name,tempfun++,d2->name);
}

}
else {
fprintf(FP1,"#pragma omp parallel for num_threads(12)\n for(int i=0;i<%s.nedges;i++){ ",d2->name);
dir_decl *d3=this->expr1->lhs;
fprintf(FP1,"int falcft%d=%s*i;\n",unnicnt,utflagarr[utflag]);
d3->it=1;
d3->extra_name=malloc(sizeof(char)*100);
sprintf(d3->extra_name,"falcft%d",unnicnt);
d3->parent=d2;
}





}


}
else if(it<6)fprintf(FP1,"//invalid iterator for Graph type\n");

else {
int flag=0,xu;
if(this->stassign!=NULL && this->stassign->rhs->expr_type==FUNCALL){
for (std::map<char *,int>::iterator it1=newitr.begin(); it1!=newitr.end(); ++it1){

if(!strcmp(it1->first,this->expr3->name)){
flag=1;
extra_ppts *ep=d2->ppts;
while(ep!=NULL){if(!strcmp(ep->name,it1->first)){flag=2;break;}ep=ep->next;}
if(flag==2){
if(d2->gpu==1){
if(this->feb==0){
if(this->stassign!=NULL && this->stassign->rhs->expr_type==FUNCALL){
extemp++;
fprintf(FP1,"%s temp%d;\n",d2->extra_name,extemp);
fprintf(FP1,"%scudaMemcpy(&temp%d,%s.extra,sizeof(%s),cudaMemcpyDeviceToHost)%s%d%s\n",cpy[0],extemp,d2->name,d2->extra_name,cpy[1],errcnt++,cpy[2]);
fprintf(FP1,"int temp%d;\n",extemp+1);
fprintf(FP1,"temp%d=temp%d.n%s;\n",extemp+1,extemp,ep->name);
if(this->barrier==1)fprintf(FP1,"%s_realloc(%s);\n",d2->name,d2->name);
if(this->barrier==1)fprintf(FP1,"findthreadandblocksize(prop,nblocks,size);\n",extemp+1); 



if(this->barrier==0)fprintf(FP1,"%s<<<temp%d/prop.maxThreadsPerBlock+1,prop.maxThreadsPerBlock>>>(",this->stassign->rhs->name,extemp+1);
else { fprintf(FP1,"reset%s%s<<<temp%d/prop.maxThreadsPerBlock+1,prop.maxThreadsPerBlock>>>(%s  );\ncudaDeviceSynchronize();\n",d2->name,ep->name,extemp+1,d2->name);
fprintf(FP1,"%s<<<nblocks,size/4>>>(",this->stassign->rhs->name);

}
dir_decl *graphvar=NULL;
extemp=extemp+2;
assign_stmt *ta=this->stassign->rhs->arglist;
ta=ta->next;
while(ta!=NULL && ta->next!=NULL){
fprintf(FP1,"%s,",ta->rhs->name);
if(this->barrier==1&& ta->rhs->lhs->libdtype==GRAPH_TYPE)graphvar=ta->rhs->lhs;
ta=ta->next;
}
if(ta!=NULL)fprintf(FP1,"%s);//access gpu value in kernel blocksize\n",ta->rhs->name);
fprintf(FP1,"cudaDeviceSynchronize();\n");
if(this->barrier==1 && graphvar!=NULL){
fprintf(FP1,"int temp%d;\n",extemp);
fprintf(FP1,"temp%d=%s.npoints;\n",extemp,graphvar->name);
fprintf(FP1,"%scudaMemcpy(&(%s.npoints),%s.pnpoints,sizeof(int),cudaMemcpyDeviceToHost)%s%d%s\n",cpy[0],graphvar->name,graphvar->name,cpy[1],errcnt++,cpy[2]);
fprintf(FP1,"%scudaMemcpy(%s.pnpoints,&temp%d,sizeof(int),cudaMemcpyHostToDevice)%s%d%s\n",cpy[0],graphvar->name,extemp,cpy[1],errcnt++,cpy[2]);
fprintf(FP1,"temp%d=%s.nedges;\n",extemp,graphvar->name);
fprintf(FP1,"%scudaMemcpy(&(%s.nedges),%s.pnedges,sizeof(int),cudaMemcpyDeviceToHost)%s%d%s\n",cpy[0],graphvar->name,graphvar->name,cpy[1],errcnt++,cpy[2]);
fprintf(FP1,"%scudaMemcpy(%s.pnedges,&temp%d,sizeof(int),cudaMemcpyHostToDevice)%s%d%s\n",cpy[0],graphvar->name,extemp,cpy[1],errcnt++,cpy[2]);
fprintf(FP1,"goal=goal+1000;\n");
struct extra_ppts *tt=graphvar->ppts;
int flag=0;
while(tt!=NULL){
if(tt->libdtype==G_P_TYPE){
if(flag==0){
fprintf(FP1,"%s temp%d;\n",graphvar->extra_name,++extemp);
fprintf(FP1,"%scudaMemcpy(&temp%d,((%s *)(%s.extra)),sizeof(%s),cudaMemcpyDeviceToHost)%s%d%s\n",cpy[0],extemp,graphvar->extra_name,graphvar->name,graphvar->extra_name,cpy[1],errcnt++,cpy[2]);
flag=1;
}
fprintf(FP1,"temp%d=temp%d.n%s;\n",extemp-1,extemp,tt->name);
fprintf(FP1,"%scudaMemcpyFromSymbol(&(temp%d.n%s),falc%s%s,sizeof(int),0,cudaMemcpyDeviceToHost)%s%d%s\n",cpy[0],extemp,tt->name,graphvar->name,tt->name,cpy[1],errcnt++,cpy[2]);
fprintf(FP1,"temp%d.pn%s=temp%d;\n",extemp,tt->name,extemp-1);

}
tt=tt->next;
}
if(flag==1)fprintf(FP1,"%scudaMemcpy(((%s *)(%s.extra)),&temp%d,sizeof(%s),cudaMemcpyHostToDevice)%s%d%s\n",cpy[0],graphvar->extra_name,graphvar->name,extemp,graphvar->extra_name,cpy[1],errcnt++,cpy[2]);
}
}

}
}
if(d2->gpu==0){
if(this->feb==0){
fprintf(FP1," MORPH CALL HERE %d\n",flag);
if(this->stassign!=NULL && this->stassign->rhs->expr_type==FUNCALL){
extemp++;
fprintf(FP1,"int temp%d;\n",extemp);
fprintf(FP1,"temp%d=((%s *)%s)->n%s;\n",extemp,d2->extra_name,d2->name,ep->name);
if(this->barrier==1)fprintf(FP1,"%s_realloc(%s);\n",d2->name,d2->name);


if(this->barrier==0)fprintf(FP1,"#pragma omp parallel\n for(int i=0;i<temp%d;i++)%s(i,",extemp,this->stassign->rhs->name);
else { 
fprintf(FP1,"#pragma omp parallel  \n for(int i=0;i<temp%d;i++)%s(i,",extemp,this->stassign->rhs->name);

}
dir_decl *graphvar=NULL;
extemp=extemp+2;
assign_stmt *ta=this->stassign->rhs->arglist;
ta=ta->next;
while(ta!=NULL && ta->next!=NULL){
fprintf(FP1,"%s,",ta->rhs->lhs->name);
if(this->barrier==1&& ta->rhs->lhs->libdtype==GRAPH_TYPE)graphvar=ta->rhs->lhs;
ta=ta->next;
}

if(ta!=NULL)fprintf(FP1,"%s);//access gpu value in kernel blocksize\n",ta->rhs->name);


if(this->barrier==1 && graphvar!=NULL){
fprintf(FP1,"goal=goal+1000;\n");
struct extra_ppts *tt=graphvar->ppts;
int flag=0;
while(tt!=NULL){
if(tt->libdtype==G_P_TYPE){
if(flag==0){
fprintf(FP1,"%s temp%d;\n",graphvar->extra_name,++extemp);
fprintf(FP1,"%scudaMemcpy(&temp%d,((%s *)(%s.extra)),sizeof(%s),cudaMemcpyDeviceToHost)%s%d%s\n",cpy[0],extemp,graphvar->extra_name,graphvar->name,graphvar->extra_name,cpy[1],errcnt++,cpy[2]);
flag=1;
}
fprintf(FP1,"((%s *)%s)->n%s=devn%s;\n",d2->extra_name,d2->name,ep->name,ep->name);

}
tt=tt->next;
}
}
}

}
}
}
}
}
}
}
}
}
fprintf(stderr,"//FOREACH ON POINT %d\n",it);
if(d2->libdtype==POINT_TYPE &&(it==3 || it==2 || it==4)){//4=outnbrs,3=innbrs,2=outnbrs+innbrs
fprintf(stderr,"//foreach on point %d\n",it);
NESTED_FOREACH=1;
if(d2->gpu==0)foreachhostflag++;
if(it==2)this->expr1->lhs->libdtype=POINT_TYPE;
else if(it==3)this->expr1->lhs->libdtype=POINT_TYPE;
else if(it==4)this->expr1->lhs->libdtype=POINT_TYPE;
d2->extra_name1=malloc(sizeof(char)*100);
if(d2->gpu)strcpy(d2->extra_name1,"id");
else strcpy(d2->extra_name1,d2->name);
if(d2->libdtype==POINT_TYPE/* && it==4*/){
dir_decl *d2p=d2->parent;

if(d2p!=NULL){
char arr[200];
for(int i=0;i<200;i++)arr[i]='\0';
tree_expr *e5= this->expr5;
if(e5!=NULL){ 
e5->rhs->printcode1(e5->rhs,arr);
fprintf(FP1,"int falcftadv%d=%s;\n",unnicnt,arr);
}
printf("HERE\n");

if(GLOBAL_ID_FLAG==1){
//fprintf(FP1,"//global id one \n");
sprintf(this->expr2->lhs->name,"id+%s.start_point",d2p->name);
GLOBAL_ID_FLAG=0;
}
if(/*itr==2||itr==3||*/ itr==4){//outnbrs
printf("HERE\n");
fprintf(FP1,"int falcft%d=%s.index[id+1]-%s.index[id];\nint falcft%d=%s.index[id];\n",unnicnt,d2p->name,d2p->name,unnicnt+1,d2p->name);
int union_type=0;
if(e5!=NULL)fprintf(FP1,"for(int falcft%d=falcftadv%d;falcft%d<falcft%d;falcft%d++){\n",unnicnt+2,unnicnt,unnicnt+2,unnicnt,unnicnt+2);
else fprintf(FP1,"/*uuXX*/for(int falcft%d=0;falcft%d<falcft%d;falcft%d++){\n",unnicnt+2,unnicnt+2,unnicnt,unnicnt+2);
fprintf(FP1,"int ut%d=%s*(falcft%d+falcft%d);\n int ut%d=%s.edges[ut%d%s].%s;\nint ut%d=%s.edges[ut%d%s].%s;\n",utcnt,utflagarr[utflag][0],unnicnt+1,unnicnt+2,utcnt+1,d2p->name,utcnt,utflagarr[utflag][1],union_string[union_type],utcnt+2,d2p->name,utcnt,utflagarr[utflag][2],union_string[union_type]);
d2->extra_name1=malloc(sizeof(char)*100);
d2->extra_name=malloc(sizeof(char)*100);
if(d2p->extra_name!=NULL)strcpy(d2->extra_name,d2p->extra_name);
//strcpy(d2->extra_name,d2p->extra_name);
strcpy(d2->extra_name1,this->expr2->lhs->name);
dir_decl *d1=this->expr1->lhs;
d1->extra_name1=malloc(sizeof(char)*100);
d1->extra_name=malloc(sizeof(char)*100);
sprintf(d1->extra_name1,"ut%d",utcnt+1);
if(d2p!=NULL && d2p->extra_name!=NULL)strcpy(d1->extra_name,d2p->extra_name);
if(this->stassign!=NULL) {char temp[200];
for(int i=0;i<200;i++)temp[i]='\0';
FOREACH_FLAG++;
this->stassign->rhs->printcode1(this->stassign->rhs,temp);fprintf(FP1,"%s;///foreach\n",temp);}
d1->parent=d2p;
unnicnt=unnicnt+3;
utcnt=utcnt+3;
}
if(itr==3){//innbrs
int union_type=0;
fprintf(stderr,"%d \n",utflag);
fprintf(FP1,"int falcft%d=%s.inindex[id+1]-%s.inindex[id];\nint falcft%d=%s.index[id];\n",unnicnt,d2p->name,d2p->name,unnicnt+1,d2p->name);
 fprintf(FP1,"/*XX*/for(int falcft%d=0;falcft%d<falcft%d;falcft%d++){\n",unnicnt+2,unnicnt+2,unnicnt,unnicnt+2);
fprintf(FP1,"int ut%d=%s*(falcft%d+falcft%d);\n int ut%d=%s.inedges[ut%d%s].%s;\nint ut%d=%s.inedges[ut%d%s].%s;\n",utcnt,utflagarr[utflag][0],unnicnt+1,unnicnt+2,utcnt+1,d2p->name,utcnt,utflagarr[utflag][1],union_string[union_type],utcnt+2,d2p->name,utcnt,utflagarr[utflag][2],union_string[union_type]);

d2->extra_name1=malloc(sizeof(char)*100);
d2->extra_name=malloc(sizeof(char)*100);
if(d2p->extra_name!=NULL)strcpy(d2->extra_name,d2p->extra_name);
//strcpy(d2->extra_name,d2p->extra_name);
strcpy(d2->extra_name1,this->expr2->lhs->name);
dir_decl *d1=this->expr1->lhs;
d1->extra_name1=malloc(sizeof(char)*100);
d1->extra_name=malloc(sizeof(char)*100);
sprintf(d1->extra_name1,"ut%d",utcnt+1);
if(d2p!=NULL && d2p->extra_name!=NULL)strcpy(d1->extra_name,d2p->extra_name);
if(this->stassign!=NULL) {char temp[200];
for(int i=0;i<200;i++)temp[i]='\0';
FOREACH_FLAG++;
this->stassign->rhs->printcode1(this->stassign->rhs,temp);fprintf(FP1,"%s;///foreach\n",temp);}
d1->parent=d2p;
unnicnt=unnicnt+3;
utcnt=utcnt+3;

}
}
}


 fprintf(stderr,"invalid iterator for Point datatype");
}
fprintf(stderr,"FOREACH ON COLLECTION \n");
if(d2->libdtype==COLLECTION_TYPE){ 
fprintf(stderr,"FOREACH ON COLLECTION \n");
if(d2->gpu==0){
fprintf(stderr,"FOREACH ON COLLECTION \n");
//fpos_t temppos;
//fgetpos(FP1,&temppos);
if(fsetpos(FP1,&mainfpos)==-1)fprintf(FP1,"ERROR SET FPOS\n");
char *temp=malloc(sizeof(char)*1500);
char *old=malloc(sizeof(char)*4500);
fread(old,4500,1,FP1);
if(fsetpos(FP1,&mainfpos)==-1)fprintf(FP1,"ERROR SET FPOS\n");
char *temp1=malloc(sizeof(char)*1500);
for(int i=0;i<1500;i++)temp[i]='\0';
for(int i=0;i<1500;i++)temp1[i]='\0';
++PROCESS_CNT;
if(d2->ordered==1)sprintf(temp,"struct Process%d {\n Process%d(){}\ntemplate <typename Pusher > \nvoid operator()(%s &req,Pusher &%s ){\n  ",PROCESS_CNT,PROCESS_CNT,d2->tp1->name,d2->name);
else sprintf(temp,"struct Process%d {\n Process%d(){}\n void operator()(%s &req){\n  ",PROCESS_CNT,PROCESS_CNT,d2->tp1->name,d2->name);
if(this->stassign!=NULL && this->stassign->rhs->expr_type==FUNCALL){
sprintf(temp1,"%s(req,",this->stassign->rhs->name);
strcat(temp,temp1);
assign_stmt *ta=this->stassign->rhs->arglist;
ta=ta->next;
while(ta!=NULL && ta->next!=NULL){
for(int i=0;i<1500;i++)temp1[i]='\0';
//ta->printcode1(temp1,0);
if(ta->rhs->name!=NULL)sprintf(temp1,"%s",ta->rhs->name);
else ta->rhs->printcode1(ta->rhs,temp1);
strcat(temp1,",");
strcat(temp,temp1);
ta=ta->next;
}
//if(ta!=NULL)fprintf(FP1,"%s,",ta->rhs->name);
for(int i=0;i<1500;i++)temp1[i]='\0';
if(ta!=NULL){
//ta->printcode1(temp1,0);
if(ta->rhs->name!=NULL)sprintf(temp1,"%s",ta->rhs->name);
else ta->rhs->printcode1(ta->rhs,temp1);
strcat(temp,temp1);
}
//fprintf(FP1,"%s);\n",d2->name);
for(int i=0;i<1500;i++)temp1[i]='\0';
sprintf(temp1,");\n");
strcat(temp,temp1);
}
for(int i=0;i<1500;i++)temp1[i]='\0';
sprintf(temp1,"}\n};\n");
strcat(temp,temp1);
fprintf(FP1,"%s\n",temp);
fpos_t temppos1;
fgetpos(FP1,&temppos1);
fgetpos(FP1,&mainfpos);
fprintf(FP1,"%s\n ",old);
char ch=' ';
if(this->expr5!=NULL && this->expr5->ptrflag)ch='*';
if(d2->ordered==1)fprintf(FP1,"Galois::for_each_local(%s,Process%d(),Galois::wl<OBIM>());\n",d2->name,PROCESS_CNT,d2->tp1->name);
else fprintf(FP1,"Galois::do_all_local(%c%s,Process%d());\n",ch,d2->name,PROCESS_CNT);
}

if(d2->gpu==1){
fprintf(FP1,"int falcvt%d;\n",++Temp);
fprintf(FP1,"cudaMemcpy(&falcvt%d,&(%s.size[0]),sizeof(int),cudaMemcpyDeviceToHost);\n",Temp,d2->name);
fprintf(FP1,"%s<<<(falcvt%d)/prop%d.maxThreadsPerBlock+1,prop%d.maxThreadsPerBlock>>>(",this->stassign->rhs->name,Temp,d2->dev_no,d2->dev_no);
assign_stmt *ta=this->stassign->rhs->arglist;
 fprintf(stderr, "foreach on gpu collection object\n");
ta=ta->next;
while(ta!=NULL && ta->next!=NULL){
fprintf(FP1,"%s,",ta->rhs->name);
fprintf(stderr,"%s,",ta->rhs->name);
ta=ta->next;
}
if(ta!=NULL)fprintf(FP1,"%s,0);",ta->rhs->name);
if(ta!=NULL)fprintf(stderr,"%s);",ta->rhs->name);
fprintf(FP1,"\ncudaDeviceSynchronize();\n");
}
}
}
if(this->sttype==FOR_STMT){
if(this->f2->stassign!=NULL ){
if(this->f2->stassign->lhs!=NULL &&this->f2->stassign->lhs->expr_type!=NULL)fprintf(FP1,"/*lhs*%d/",this->f2->stassign->lhs->expr_type);
//if(this->f2->stassign->lhs!=NULL && kernel==0/*&& this->stassign->lhs->expr_type==VAR*/)this->stassign->lhs->findgpuvars(this->f2->stassign->lhs);
if(this->f2->stassign->rhs!=NULL && Gkernel==0 /* && this->stassign->rhs->expr_type!=VAR*/)this->f2->stassign->rhs->FOR_findgpuvars(this->f2->stassign->rhs);
if(this->next!=NULL) findextrapropretry(this->next);
}
char arr[200];
fprintf(FP1,"for (");
if(this->f1->stdecl!=NULL){
fprintf(FP1,"%s ", dtypenames[this->f1->stdecl->lhs->datatype]);
if(this->f1->stdecl->dirrhs!=NULL) this->f1->stdecl->dirrhs->printcode(1);
fprintf(FP1,";");
}
if(this->f1->stassign!=NULL){

for(int i=0;i<200;i++)arr[i]='\0';
this->f1->stassign->printcode1(arr,0);
fprintf(FP1,"%s;",arr);

}
if(this->f2->stassign!=NULL ){
for(int i=0;i<200;i++)arr[i]='\0';
this->f2->stassign->printcode1(arr,0);
fprintf(FP1,"%s;",arr);
printf(";");
}
if(this->f3!=NULL&& this->f3->stassign!=NULL){
for(int i=0;i<200;i++)arr[i]='\0';
this->f3->stassign->printcode1(arr,0);
fprintf(FP1,"%s",arr);
}
if(this->stassign!=NULL){
fprintf(FP1,")");
for(int i=0;i<200;i++)arr[i]='\0';
this->stassign->printcode1(arr,0);
fprintf(FP1,"%s;",arr);
}
else fprintf(FP1,") {");
}
//for(int i=0;i<200;i++)forcondarr[i]='\0';
if(this->sttype==DECL_STMT){
if(this->stdecl && this->stdecl->dirrhs && this->stdecl->dirrhs->procd==1 && this->noprint==true){
statement *st=this;
tree_decl_stmt *df1=st->stdecl->dirrhs->params;
dir_decl *orig=df1->dirrhs;
df1=df1->next;
dir_decl *incom=df1->dirrhs;
int cnt=0;
while(st!=NULL &&st->sttype!=FUNCTION_EBLOCK_STMT){
fprintf(stderr,"xx %d\n",st->sttype);
st=st->next;
if(st->sttype==ASSIGN_STMT){
//fprintf(FP1,"%d ",++cnt);
findstructref(incom,st->stassign->rhs);
findstructref(incom,st->stassign->lhs);
}
if(st->sttype==IF_STMT &&st->f1!=NULL){
findstructref(incom,st->expr1);
st=st->f1;
if(st->sttype==ASSIGN_STMT){
//fprintf(FP1,"%d ",++cnt);

}

}
if(st->sttype==IF_STMT &&st->f2!=NULL){
findstructref(incom,st->expr1);
st=st->f2;
if(st->sttype==ASSIGN_STMT){
//fprintf(FP1,"%d ",++cnt);
}
}
}
fprintf(FP1,"\n");


if(st && st->next)st->next->codeGen(FP1);
return;
}

//if(this->gpugpu>1 )this->ker=1;
if(this->ker==1){
//fprintf(FP1,"//THIS FUNCTION %s IS A GPU KERNEL\n ", this->stdecl->dirrhs->name);
Gkernel=1;}
if(this->stdecl!=NULL && this->stdecl->dirrhs!=NULL &&this->stdecl->dirrhs->procd==1 && this->ker>=1){
for(int i=0;i<100;i++)curr_fun_name[i]='\0';
//sprintf(curr_fun_name1,"%s %s ",dtypenames[this->stdecl->lhs->datatype],this->stdecl->dirrhs->name);
strcpy(curr_fun_name,this->stdecl->dirrhs->name);
if(strcmp(this->stdecl->dirrhs->name,"main")==0 &&strlen(this->stdecl->dirrhs->name)==strlen("main")){MAINF=1;fgetpos(FP1,&mainfpos);}
if(EXTRA_CRBRK==1){
fprintf(FP1,"}//xx\n");
EXTRA_CRBRK=0;
}
if(EXTRA_CRBRK==2){
fprintf(FP1,"}\n}/xxxx\n");
EXTRA_CRBRK=0;
}
printf("GETPOS");
fgetpos(FP1, &position);
printf("GETPOS");
if(this->ker==1)fprintf(FP1,"__global__ ");
if(this->ker==2)fprintf(FP1,"__device__ ");
//this->print();
if(this->ker==1|| Gkernel==0)Gkernel=1;
if(this->ker==2)Gkernel=3;
//return;
}
if(this->ker==0 && this->stdecl && this->stdecl->dirrhs!=NULL && this->stdecl->dirrhs->procd==1){
for(int i=0;i<100;i++)curr_fun_name1[i]='\0';

if(EXTRA_CRBRK==1){
fprintf(FP1,"}//hereyy\n");
EXTRA_CRBRK=0;
}
}

if(this->stdecl!=NULL && this->stdecl->dirrhs!=NULL &&this->stdecl->dirrhs->procd==1 && this->ker==0){
if(strcmp(this->stdecl->dirrhs->name,"main")==0 &&strlen(this->stdecl->dirrhs->name)==strlen("main")){MAINF=1;
fgetpos(FP1,&mainfpos);
}
Gkernel=0;
fprintf(FP1,"/*X %dX*/",Gkernel);

if(EXTRA_CRBRK==1){
fprintf(FP1,"}//zz\n");
EXTRA_CRBRK=0;
}
if(EXTRA_CRBRK==2){
fprintf(FP1,"}\n}//zzzz\n");
EXTRA_CRBRK=0;
}
 fgetpos(FP1, &position);
//fprintf(FP1,"%s ",dtypenames[this->stdecl->lhs->datatype]);
}
tree_typedecl *x1=this->stdecl->lhs;
//printf("HERE\n");
if(x1->libdatatype!=-1 && !(this->stdecl!=NULL && this->stdecl->dirrhs!=NULL &&this->stdecl->dirrhs->procd==1)){

if(x1->libdatatype==GRAPH_TYPE){
int gf=0,hf=0;
dir_decl *t1=this->stdecl->dirrhs;
while(t1!=NULL){if (t1->gpu==1)gf=1;if(t1->gpu==0)hf=1; if(hf&&gf) break; t1=t1->nextv;}
if(gf==1){ //fprintf(FP1,"GGraph ");
int cnt=0;
t1=this->stdecl->dirrhs;
while(t1!=NULL){if (t1->gpu==1){if(cnt>0)fprintf(FP1,","); t1->printcode(0);cnt++; }t1=t1->nextv;}
fprintf(FP1,";\n");
}
if(hf==1){ //fprintf(FP1,"HGraph ");
int cnt=0;
char arr1[100];
t1=this->stdecl->dirrhs;
while(t1!=NULL){if (t1->gpu==0){if(cnt>0)fprintf(FP1,","); t1->printcode(0);cnt++; if(t1->read==0){fprintf(FP1,";\nread_and_allocate_%s(%s);\n",t1->name,t1->name);cnt=0; if(t1->nextv!=NULL)fprintf(FP1,"HGraph ");}}t1=t1->nextv;}
fprintf(FP1,";");
}
}
if(x1->libdatatype==SET_TYPE  ){
int gf=0,hf=0;
dir_decl *t1=this->stdecl->dirrhs;
while(t1!=NULL){

if(t1->tp1 && t1->tp1->d1 && t1->tp1->d1->gpu==1)gf=1;
if(t1->tp1 && t1->tp1->d1 && t1->tp1->d1->gpu==0)hf=1;
if(gf && hf)break;
t1=t1->nextv;
}
if(gf==1){
t1=this->stdecl->dirrhs;
fprintf(FP1,"GSet ");
int cnt=0;
while(t1!=NULL){
if(t1->tp1 && t1->tp1->d1 && t1->tp1->d1->gpu==1&& t1->tp1->libdatatype!=-1){
if(cnt>0)fprintf(FP1,",");
fprintf(FP1,"%s",t1->name);
cnt++;
}
t1=t1->nextv;
}
fprintf(FP1,";\n");
t1=this->stdecl->dirrhs;
while(t1!=NULL){
if(t1->tp1->next->libdatatype==POINT_TYPE && t1->tp1->d1!=NULL&& t1->tp1->d1->libdtype==GRAPH_TYPE&&t1->tp1->d1->gpu==1)fprintf(FP1,"%s.allocate(%s.npoints);",t1->name,t1->tp1->d1->name); 
if(t1->tp1->next->libdatatype==EDGE_TYPE && t1->tp1->d1!=NULL&& t1->tp1->d1->libdtype==GRAPH_TYPE&& t1->tp1->d1->gpu==1)fprintf(FP1,"%s.allocate(%s.nedges);",t1->name,t1->tp1->d1->name); 
t1=t1->nextv;}
}
if(hf==1){
t1=this->stdecl->dirrhs;
fprintf(FP1,"HSet ");
int cnt=0;
while(t1!=NULL){
if(t1->tp1 && t1->tp1->d1 && t1->tp1->d1->gpu==0&& t1->tp1->libdatatype!=-1){
if(cnt>0)fprintf(FP1,",");
fprintf(FP1,"%s",t1->name);
cnt++;
}
t1=t1->nextv;

}
fprintf(FP1,";\n");
t1=this->stdecl->dirrhs;
while(t1!=NULL){
if(t1->tp1->next->libdatatype==POINT_TYPE && t1->tp1->d1!=NULL&& t1->tp1->d1->libdtype==GRAPH_TYPE&&t1->tp1->d1->gpu==0)fprintf(FP1,"%s.allocate(%s.npoints);",t1->name,t1->tp1->d1->name); 
if(t1->tp1->next->libdatatype==EDGE_TYPE && t1->tp1->d1!=NULL&& t1->tp1->d1->libdtype==GRAPH_TYPE&& t1->tp1->d1->gpu==0)fprintf(FP1,"%s.allocate(%s.nedges);",t1->name,t1->tp1->d1->name); 
t1=t1->nextv;}

}
}
if(x1->libdatatype==COLLECTION_TYPE){
int cnt=0;
printf("HERE CNT 0");
if(Gkernel>=1){
char initarr[100];
for(int i=0;i<100;i++)initarr[0]='\0';
fprintf(FP1,"struct dev_vec ");
dir_decl *t1=this->stdecl->dirrhs;
while(t1!=NULL){if(cnt>0)fprintf(FP1,","); fprintf(FP1,"%s",t1->name);cnt++;
if(t1->rhs!=NULL){
t1->rhs->printcode1(t1->rhs,initarr);fprintf(FP1,"=%s",initarr); 
for(int i=0;i<100;i++)initarr[0]='\0';
}
if(t1!=NULL && t1->parent!=NULL){
t1->extra_name1=malloc(sizeof(char)*100);strcpy(t1->extra_name1,t1->name);
t1->extra_name=malloc(sizeof(char)*100);strcpy(t1->extra_name,t1->parent->extra_name);
}
t1=t1->nextv;
}
fprintf(FP1,";\n");
t1=this->stdecl->dirrhs;
while(t1!=NULL){fprintf(FP1,"%s= create_coll_device(100);\n",t1->name);t1=t1->nextv;} 

}
if(Gkernel<=0){
dir_decl *t1=this->stdecl->dirrhs;
while(t1!=NULL){
if(t1->gpu==1){
if(t1->ptrcnt>0 && (t1->tp1!=NULL && t1->tp1->next!=NULL&& t1->tp1->next->libdatatype==POINT_TYPE) ||(t1->tp1!=NULL && t1->tp1->next!=NULL&& t1->tp1->next->libdatatype==EDGE_TYPE)){
fprintf(FP1,"int *%s;",t1->name);
t1=t1->nextv;
continue;
}
if( t1->tp1!=NULL && t1->tp1!=NULL && t1->tp1->libdatatype==POINT_TYPE && t1->tp1->d1!=NULL&& t1->tp1->d1->libdtype==GRAPH_TYPE&&t1->tp1->d1->gpu==1){
//fprintf(FP1,"%s.allocate(%s.npoints);",t1->name,t1->tp1->d1->name); 
fprintf(FP1,"thrust::device_vector<int> %stemp(%s.npoints);\n",t1->name,t1->tp1->d1->name);
fprintf(FP1,"GColl %s;\n  %s.ptr=thrust::raw_pointer_cast(&%stemp[0]);\n",t1->name,t1->name,t1->name);
//fprintf(FP,"__device__ int %szie=0;\n",t1->name);
fprintf(FP1,"alloc%ssize(%s);\n",t1->name,t1->name);
fprintf(FP,"\nvoid alloc%ssize(GColl &%s){ cudaMalloc((void **)&(%s.size),sizeof(int));\nint temp=0;\ncudaMemcpy(&(%s.size[0]),&temp,sizeof(int),cudaMemcpyHostToDevice);\n}\n",t1->name,t1->name,t1->name,t1->name);

t1->gpu=1;
}
}
else {
char temp[200];
if(t1->isparam==true)fprintf(FP1,"Galois::InsertBag<%s> ",t1->tp1->name);
if(t1->isparam==false)fprintf(FP1,"struct host_vec");
t1->printcode(0);
fprintf(FP1,"; \n");
//break;
}
t1=t1->nextv;
}
}
}
if(x1->libdatatype==POINT_TYPE || x1->libdatatype==EDGE_TYPE){
int gf=0,hf=0;
dir_decl *t1=this->stdecl->dirrhs;

char initarr[100];
for(int i=0;i<100;i++)initarr[0]='\0';
if((x1->libdatatype==POINT_TYPE || x1->libdatatype==EDGE_TYPE)&& t1->parent!=NULL) fprintf(FP1,"int ");
if(x1->libdatatype==POINT_TYPE && t1->parent==NULL)fprintf(FP1,"struct Point ");
if(x1->libdatatype==EDGE_TYPE && t1->parent==NULL)fprintf(FP1,"struct Edge ");
int cnt=0;
t1=this->stdecl->dirrhs;
while(t1!=NULL){if(cnt>0)fprintf(FP1,","); 
//fprintf(FP1,"%s",t1->name);cnt++; 
char arhs[100];
for(int i=0;i<100;i++)arhs[i]='\0';
t1->printcodearray(0,arhs);cnt++; 
fprintf(FP1,"%s",arhs);
if(t1->rhs!=NULL){
//t1->rhs->printcode1(t1->rhs,initarr);fprintf(FP1,"=%s",initarr); 
for(int i=0;i<100;i++)initarr[0]='\0';
}
if(t1!=NULL && t1->parent!=NULL){
fprintf(stderr,"BUG HERE\n");
t1->extra_name1=malloc(sizeof(char)*100);strcpy(t1->extra_name1,t1->name);
if(t1->parent->extra_name!=NULL){t1->extra_name=malloc(sizeof(char)*100);strcpy(t1->extra_name,t1->parent->extra_name);}
}
t1=t1->nextv;
}

fprintf(FP1,";");
}
dir_decl *t1=this->stdecl->dirrhs;
while(t1!=NULL){if(t1->ppts!=NULL){
struct extra_ppts *ep=t1->ppts;
while(ep!=NULL){
//printf("%d  %d ",ep->t1->datatype,ep->libdtype);
if(ep->var1!=NULL)ep->var1->print();
if(ep->var2!=NULL)ep->var2->print();
ep=ep->next;
//fprintf(FP1,"\n");
}
}
t1=t1->next;
}
}
if(x1->datatype==STRUCT_TYPE && x1->libdatatype!=-1 &&  !(this->stdecl!=NULL && this->stdecl->dirrhs!=NULL &&this->stdecl->dirrhs->procd==1)){
this->stdecl->printcode(1);
}
if(x1->libdatatype==-1){

printf("%d Unni",x1->datatype);
while(x1  && (x1->datatype==-1 || x1->datatype==TYPEDEF_TYPE)){fprintf(FP1,"%s",x1->name);x1=x1->next;}
if((x1->datatype==STRUCT_TYPE||x1->datatype==ENUM_TYPE||x1->datatype==UNION_TYPE )&&  (!(this->stdecl!=NULL && this->stdecl->dirrhs!=NULL &&this->stdecl->dirrhs->procd==1))){
this->stdecl->printcode(1);
}
else {
if(this->stdecl->dirrhs!=NULL &&this->stdecl->dirrhs->procd!=1){
int gf=0,hf=0;
dir_decl *t1=this->stdecl->dirrhs;
while(t1!=NULL){if (t1->gpu==1 )gf=1;
if(t1->gpu && (t1->ctype==CARR_TYPE && t1->next && t1->next->assign!=NULL)&&gf==0)gf=1;
if(t1->gpu && !(t1->ctype==CARR_TYPE && t1->next && t1->next->assign!=NULL))gf=2;
if(t1->gpu==0)hf=1; if(hf&&gf) break; t1=t1->nextv;}
if(gf==2){ if(Gkernel==0)fprintf(FP1,"__device__ ");else fprintf(FP1,"");
int cnt=0;
tree_typedecl *tt1=this->stdecl->lhs;
while(tt1 && (tt1->datatype==TYPEDEF_TYPE||tt1->datatype==-1) && tt1->libdatatype==-1){tt1=tt1->next;}
printf("    Unni");
tree_typedecl *ttt1=tt1;
while(tt1!=NULL ){if(tt1->name!=NULL){fprintf(FP1,"%s ", tt1->name);}tt1=tt1->next;}
printf("XXXXXXXXX");
t1=this->stdecl->dirrhs;
while(t1!=NULL){if (t1->gpu==1){if(cnt>0)fprintf(FP1,","); if(!(t1->ctype==CARR_TYPE &&t1->next&& t1->next->assign!=NULL))t1->printcode(0);cnt++; }t1=t1->nextv;}
fprintf(FP1,";\n");
}
if(gf>=1){
t1=this->stdecl->dirrhs;
tree_typedecl *tt1=this->stdecl->lhs;
while(tt1 && (tt1->datatype==TYPEDEF_TYPE||tt1->datatype==-1) && tt1->libdatatype==-1){tt1=tt1->next;}
printf("    Unni");
tree_typedecl *ttt1=tt1;
while(t1!=NULL){if (t1->gpu==1){ if((t1->ctype==CARR_TYPE &&t1->next&& t1->next->assign!=NULL)){


tree_typedecl *Ttt1=ttt1;
while(Ttt1!=NULL ){if(Ttt1->name!=NULL){fprintf(FP1,"%s ", Ttt1->name);}Ttt1=Ttt1->next;}
fprintf(FP1,"* %s; ",t1->name);
fprintf(FP1,"cudaMalloc((void **)&%s,sizeof(%s)*",t1->name,ttt1->name);
t1->next->assign->printcode(t1->next->assign);
fprintf(FP1,");\n");


}

 }t1=t1->nextv;}
fprintf(FP1,";\n");
}
if(hf==1){ fprintf(FP1," ");
int cnt=0;
tree_typedecl *tt1=this->stdecl->lhs;
while(tt1 && (tt1->datatype==TYPEDEF_TYPE||tt1->datatype==-1) && tt1->libdatatype==-1){tt1=tt1->next;}
while(tt1!=NULL ){if(tt1->name!=NULL){fprintf(FP1,"%s ", tt1->name);}tt1=tt1->next;}
t1=this->stdecl->dirrhs;
while(t1!=NULL){if (t1->gpu==0){if(cnt>0)fprintf(FP1,","); 
t1->printcode(0);
cnt++; }t1=t1->nextv;}
fprintf(FP1,";\n");
}
}
}
printf("XXXXXXXXX");

if(this->stdecl->dirrhs!=NULL)printf("VAR THERE");
if(this->stdecl->dirrhs!=NULL &&this->stdecl->dirrhs->procd==1){//prints function declaration 
//foreachcond=this->expr4;
if(this->expr4!=NULL){
//fprintf(FP1,"//FUNCTion with conditional \n");
for(int i=0;i<200;i++)forcondarr[i]='\0';
tree_expr *tt1=this->expr4;
tt1->printcode1(tt1,forcondarr);
//fprintf(FP1, " && %s[id] ",forcondarr);
}
//printf("\n");
if(this->stdecl->dirrhs!=NULL) {
dir_decl *dd1=this->stdecl->dirrhs;
if(dd1->params!=NULL){
tree_decl_stmt *tt2=dd1->params;
PUSHER_CNT=0;
while(tt2!=NULL){
if(tt2->lhs->libdatatype==COLLECTION_TYPE && this->ker==0){
dir_decl *ddd1=tt2->dirrhs;
if(ddd1 && ddd1->tp1 && ddd1->gpu==0){
if(PUSHER_CNT==0)fprintf(FP1,"template<typename Pusher%d ",PUSHER_CNT++);
else {
fprintf(FP1,",typename Pusher%d",PUSHER_CNT++);
}
}
CPU_COLL_FLAG=1;
}
tt2=tt2->next;

}
}
if(PUSHER_CNT>0)fprintf(FP1, ">\n");
PUSHER_CNT=0;
}
if(this->stdecl->lhs!=NULL){
fprintf(FP1,"%s ", dtypenames[this->stdecl->lhs->datatype]);
if(this->stdecl->lhs->vname!=NULL )fprintf(FP1,"%s ",this->stdecl->lhs->vname);
}
if(this->ker>=1){
dir_decl *dd=this->stdecl->dirrhs;
if(dd->params!=NULL){
tree_decl_stmt *df1=dd->params;
while(df1!=NULL){
df1->dirrhs->gpu=1;
df1=df1->next;
}
this->stdecl->dirrhs->printcode(0);
}
}
if(this->ker==0)this->stdecl->dirrhs->printcode(1);
if(this->ker==1){
}
}
}
if(this->stdecl->dirrhs!=NULL && this->stdecl->dirrhs->tp1==NULL && this->stdecl->lhs->libdatatype==SET_TYPE)fprintf(FP1,"NULL HERE print statement");
}
if(this->next&& this->next->lineno>this->lineno)fprintf(FP1,"\n");
if(this->next && (this->sttype==DECL_STMT ||this->sttype==SBLOCK_STMT) && (this->next->sttype!=DECL_STMT||this->next->sttype!=SBLOCK_STMT) &&Gkernel==2){
//tree_expr *foreachcond=this->expr4;
if(this->prev->itr>=0 && this->prev->itr!=5)fprintf(FP1,"/* %d xx*/%s",this->prev->itr,threadstmt);
if(this->prev->itr>=0 && this->prev->itr==5)fprintf(FP1,"int id;\n if(threadIdx.x+blockDim.x*blockIdx.x>=%s.size)return; \nid=%s.ptr[threadIdx.x+blockDim.x*blockIdx.x];\n",this->prev->expr2->name,this->prev->expr2->name);
//if(this->prev->itr>=0 && this->prev->itr==5)fprintf(FP1,"int id=[threadIdx.x+blockDim.x*blockIdx.x];\n");
//fprintf(FP1,"/*int id=%d;*/\n",this->prev->itr);//modified here
statement *t1=this->prev;
while(t1){ if(t1->sttype==DECL_STMT && t1->stdecl->dirrhs!=NULL && t1->stdecl->dirrhs->procd==1)break;t1=t1->prev;}
dir_decl *d1=t1->stdecl->dirrhs;
tree_decl_stmt  *decl=d1->params;
dir_decl *d2= decl->dirrhs;
if(d2!=NULL/* && this->barrier==0 && this->prev->barrier==0*/){
if(d2->parent!=NULL){ 
if(this->barrier==1 || (this->prev &&this->prev->barrier==1)){
statement *t1=this;
t1=t1->next;
while(t1 && t1->sttype==DECL_STMT && t1->next->sttype!=IF_STMT)t1=t1->next; if(t1!=NULL ){t1->barrier=2;fprintf(FP1,"\n//barrier==2 %d \n",t1->lineno);}
for(int i=0;i<200;i++){barr_after[i]='\0';}
sprintf(barr_after,"if( /* */id < ");
}

if(d2->libdtype>=0)fprintf(FP1,"if( id < ");
}
//fprintf(FP1,"/*int%d d2it=%d;*/\n",d2->libdtype,d2->it);//modified here
if(d2->libdtype==POINT_TYPE && (d2->gpugpu || d2->cpugpu)){fprintf(FP1,"falcnpoints");}
else { if(d2->libdtype==POINT_TYPE &&d2->parent->libdtype!=COLLECTION_TYPE)fprintf(FP1,"%s.npoints",d2->parent->name);}
if(d2->libdtype==EDGE_TYPE){fprintf(FP1,"%s.nedges",d2->parent->name);}
if(d2->parent->libdtype==COLLECTION_TYPE){fprintf(FP1,"%s.size",d2->parent->name);}
if(d2->libdtype<0 && d2->it>=6){
	for (std::map<char *,int>::iterator it1=newitr.begin(); it1!=newitr.end(); ++it1){
	if(it1->second==d2->it){

if(d2->parent && !(this->barrier==1||this->prev->barrier==1)) {
      fprintf(FP1,"if(id <(( %s *)(%s.extra))->n%s",d2->parent->extra_name,d2->parent->name,it1->first);}
if(this->barrier==1 || (this->prev &&this->prev->barrier==1)){
if(d2->parent){
sprintf(threadsizestring,"((%s *)(%s.extra))->n%s",d2->parent->extra_name,d2->parent->name,it1->first); 
 strcat(barr_after,"(( "); strcat(barr_after,d2->parent->extra_name);strcat(barr_after," *)(");strcat(barr_after,d2->parent->name);strcat(barr_after,".extra))->n");strcat(barr_after,it1->first);}
 strcat(barr_after,"){");
}      
        d2->extra_name2=malloc(sizeof(char)*100);
d2->extra_name1=malloc(sizeof(char)*100);strcpy(d2->extra_name1,"id");
if(GLOBAL_ID_FLAG==1){d2->extra_name1=malloc(sizeof(char)*100);strcpy(d2->extra_name1,"id+graph.start_point");
}
d2->iflag=1;
d2->extra_name=malloc(sizeof(char)*100);strcpy(d2->extra_name,d2->parent->extra_name);
       strcpy(d2->extra_name2,it1->first);
strcpy(d2->name,"id");
if(GLOBAL_ID_FLAG==1){d2->name=malloc(sizeof(char)*100);strcpy(d2->name,"id+graph.start_point");
GLOBAL_ID_FLAG=0;
}
       break;
     
}
       }
      }

if(forcondarr[0]!='\0'){
fprintf(FP1, " && %s[id] ",forcondarr);
for(int i=0;i<200;i++)forcondarr[i]='\0';
}
if(!(this->barrier==1||this->prev->barrier==1)){
     fprintf(FP1,"){\n");
EXTRA_CRBRK=1;
}
if(this->barrier==1 || (this->prev &&this->prev->barrier==1))
EXTRA_CRBRK=2;
}
Gkernel=3;
}
//kernel=1;
if(this->next && this->sttype==DECL_STMT && this->next->sttype!=DECL_STMT &&Gkernel==1){Gkernel=2;}
//if(t->next &&t->next->comma==1)printf(" , ");
if(this->comma==0 && this->next &&this->next->comma==0)fprintf(FP1,"\n ");
if(errflag==1)fprintf(stderr,"lineno=%d\n",this->lineno);
//errflag=0;
if(this->barrier==2){
fprintf(FP1,"unsigned nthreads=blockDim.x*gridDim.x;\n unsigned wpt=(%s+nthreads)/nthreads;\nunsigned start=id*wpt;\nunsigned end=start+wpt;\n for(unsigned id=start;id<end;id++){\n%s\n",threadsizestring,barr_after);
}
if(this->next!=NULL)this->next->codeGen(FP1);
}



