#ifndef LEXICAL_ANALYSIS_H
#define LEXICAL_ANALYSIS_H

//词法分析程序头文件
//修改者：江俊广
//参考来源：https://www.cnblogs.com/zyrblog/p/6885922.html

/*
第一类：标识符   letter(letter | digit)*  无穷集
第二类：常数    (digit)+  无穷集
第三类：保留字(32)
auto       break    case     char        const      continue
default    do       double   else        enum       extern
float      for      goto     if          int        long
register   return   short    signed      sizeof     static
struct     switch   typedef  union       unsigned   void
volatile    while

第四类：界符  ‘/_*’、‘//’、 () { } [ ] " "  '
第五类：运算符 <、<=、>、>=、=、+、-、*、/、^、

对所有可数符号进行编码：
<$,0>
<auto,1>
...
<while,32>
<+，33>
<-,34>
<*,35>
</,36>
<<,37>
<<=,38>
<>,39>
<>=,40>
<=,41>
<==,42>
<!=,43>
<;,44>
<(,45>
<),46>
<^,47>
<,,48>
<",49>
<',50>
<#,51>
<&,52>
<&&,53>
<|,54>
<||,55>
<%,56>
<~,57>
<<<,58>左移
<>>,59>右移
<[,60>
<],61>
<{,62>
<},63>
<\,64>
<.,65>
<?,66>
<:,67>
<!,68>
"[","]","{","}"
<常数99  ,数值>
<标识符100 ，标识符指针>
*/

//分词函数对外提供的接口

//词法分析子程序
//*synptr：返回词语的类型
//s: 输入的字符串
//token: 返回的词语
//*pptr: 当前词法分析子程序在s中扫描到的位置
void Scanner(int* synptr, char s[], char token[], int*pptr);

#define isReserveWord(syn) (syn >= 1 && syn <= 32) //判断是否为保留字
#define isIDentifier(syn) (syn == 100) //判断是否为标识符
#define isParameter(syn) (syn == 99) //判断是否为常数
#define isOperatorOrDelimiter(syn) (syn>=33 && syn<=68)//判断是否为运算符或者定界符

#include "lexical_analysis.c"

#endif