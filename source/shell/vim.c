//vim.c文件的修改者：江俊广

// #include "types.h"
// #include "user.h"
// #include "fcntl.h"
// #include "stat.h"
// #include "fs.h"
//此处暂时这么处理！！！尚未解决的问题：如何调用另外一个文件中定义的函数，目前只能把那个文件include进来
#include "lexical_analysis.h"
#include "vim.h"
#include "color.h"

void 
initColorConfiguration()//初始化颜色配置
{
    //首先尝试从配置文件中读入颜色设置(该功能尚未实现)
    BACKGROUND_COLOR = BLACK;//背景色
    RESERVERED_WORD_COLOR = BLUE;//保留字的颜色
    PARAMETER_COLOR = DARKGREEN;//常量的颜色
    VARIABLE_COLOR = PINK;//变量的颜色
    OPERATOR_COLOR = WHITE;//运算符的颜色
    EMPHASIZE_WORD_COLOR = RED;//强调字体的颜色

    CONTROL_LINE_COLOR = YELLOW;//控制栏的颜色
    CURRENT_POSITION_COLOR = DARKPINK;//当前光标位置的颜色
    CURRENT_LINE_COLOR = DARKGRAY;//当前行的颜色
}

int 
isTextChar(char c)//判断是否为可显示字符
{
    if (c >= ' ' && c <= '~')
        return 1;
    return 0;
}

int isControlChar(char c){//判断是否为支持的控制字符
    switch(c){
        case BACKSPACE:
        case NEW_LINE:
        case CARRIAGE_RETURN:
        case HORIZONTAL_TAB:
            return 1;
        default:
            return 0;
    }
}

int
coor(int x, int y)//将二维坐标转换为一维的编号
{
    return x * CONSOLE_WIDTH + y;
}

void
init()//将显示区刷新为空白内容
{
    int i, j;
    for (i = 0; i < CONSOLE_HEIGHT; ++i)
        for (j = 0; j < CONSOLE_WIDTH; ++j)
            setconsole(coor(i, j), 0, WHITE_ON_BLACK, -1, 2);
    setconsole(-1, 0, 0, coor(cursorX, cursorY), 2);
}

void
quit()
{
    init();
    setconsole(-1, 0, 0, 0, 0);
    exit();
}

int
openFile(char* filename)//打开文件
{
    int fd = open(filename, O_RDONLY | O_CREATE);
    if (fd < 0)
        return fd;
    char buf[128];//读入字符缓冲区
    int n, i, p = 0;
    num_line = 0;
    while ((n = read(fd, buf, 128)) > 0){//从文件中不断读入字符到缓冲区
        for (i = 0; i < n; ++i){
            if (buf[i] == '\n'){//如果读入的是换行符，则增加行数
                textbuf[num_line++][p] = '\0';
                p = 0;
            }
            else{
                if (p < MAX_LENGTH)//否则读入当前行不超过MAX_LENGTH的字符串。
                    textbuf[num_line][p++] = buf[i];//Note:这样的实现导致较长行字符串的丢失。
            }
        }
    }
    close(fd);
    return 0;
}

int
saveFile(char* filename)//保存文件
{
    unlink(filename);
    int fd = open(filename, O_WRONLY | O_CREATE | O_OVER);
    if (fd < 0)
        return fd;
    int i;
    for (i = 0; i < num_line; ++i)
    {
        write(fd, textbuf[i], strlen(textbuf[i]));
        write(fd, "\n", 1);
    }
    close(fd);
    saved = 1;
    return 0;
}

char*
intToString(int k)//整数转换为字符串
{
    char* result = malloc(sizeof(char) * 10);
    if (k == 0)
    {
        result[0] = '0';
        result[1] = 0;
        return result;
    }
    int l = 0, p = k;
    while (p > 0){
        l++;
        p /= 10;
    }
    result[l--] = 0;
    while (k > 0){
        result[l--] = k % 10 + '0';
        k /= 10;
    }
    return result;
}

void 
showMessage(char* msg)//将控制命令输出在最后一行
{
    int line = 24;
    int i;
    int l = strlen(msg);
    int messageColor = combineColor(EMPHASIZE_WORD_COLOR, CONTROL_LINE_COLOR);//设置控制命令的颜色
    for (i = 0; i < 65; ++i)
        setconsole(coor(line, i), 0, messageColor, -1, 2);
    if (l > 60)
        l = 60;
    for (i = 0; i < l; ++i)
        setconsole(coor(line, i), msg[i], messageColor, -1, 2);
}

void
showCoor()//显示行列号
{
    int line = 24;
    int i, p = 0, l;
    int coorColor = combineColor(EMPHASIZE_WORD_COLOR, CONTROL_LINE_COLOR);//设置行列号的颜色
    for (i = 65; i < CONSOLE_WIDTH; ++i)
        setconsole(coor(line, i), 0, coorColor, -1, 2);//先将原先的位置上的字符清空
    char tmp[20];//存储当前光标位置的行、列的字符串
    char* digit;
    digit = intToString(cursorX + 1 + startline);
    for (i = 0; i < strlen(digit); ++i)
        tmp[p++] = digit[i];
    tmp[p++] = 'R';
    tmp[p++] = ',';
    tmp[p++] = ' ';
    digit = intToString(cursorY + 1 - left);
    for (i = 0; i < strlen(digit); ++i)
        tmp[p++] = digit[i];
    tmp[p++] = 'C';
    tmp[p++] = 0;
    l = strlen(tmp);
    for (i = 0; i < l; ++i)
        setconsole(coor(line, i + 65), tmp[i], coorColor, -1, 2);//然后输出当前光标位置的行、列号
}

void 
updateCharColorByLine(int line){//更新文档中第line行的所有位置上的字符颜色
    int syn = -1, p = 0;
    char token[20] = {0};
    int l, r, i = 0;//每个单词的左右界
    memset(charColor, 0, sizeof(charColor));//清空原先的颜色
    while(syn != 0 && i < 80){
        l = p;
        Scanner(&syn, textbuf[line], token, &p);
        r = p;
        int c = VARIABLE_COLOR;//默认为变量的颜色
        if(isReserveWord(syn)){
            c = RESERVERED_WORD_COLOR;
        }else if(isIDentifier(syn)){
            c = VARIABLE_COLOR;
        }else if(isParameter(syn)){
            c = PARAMETER_COLOR;
        }else if(isOperatorOrDelimiter(syn)){
            c = OPERATOR_COLOR;
        }
        for(i = l; i < r; i++) charColor[i] = c;//设置单词token每个位置的颜色
    }
}

void showTextRange(int up, int down){//更新[up,down)行的内容
    int i, j, l;
    if(up < top) up = 0;
    if(down > bottom) down = bottom; 
    for (i = up; i < down; ++i)
        for (j = 0; j < CONSOLE_WIDTH; ++j)
            if( i != cursorX)//如果当前行不是焦点行
                setconsole(coor(i, j), 0, GRAY, -1, 2);
            else
                setconsole(coor(i,j), 0, combineColor(BLACK, CURRENT_LINE_COLOR), -1, 2);
    for (i = startline + up; i < startline + down; ++i){
        l = strlen(textbuf[i]);
        updateCharColorByLine(i);//更新文档第i行各个位置的字符颜色
        int backgroundColor = BACKGROUND_COLOR;
        if((i-startline) == cursorX) backgroundColor = CURRENT_LINE_COLOR;//如果是焦点行 
        for (j = 0; j < l; ++j){
            if( (i-startline) != cursorX)//如果不是焦点行
                backgroundColor = BACKGROUND_COLOR;
            else if( (j-left) != cursorY)//如果是焦点行
                backgroundColor = CURRENT_LINE_COLOR;
            else//如果是焦点位置
                backgroundColor = CURRENT_POSITION_COLOR;
            setconsole(coor(i - startline, j + left), textbuf[i][j], combineColor(charColor[j], backgroundColor), -1, 2);
        }
    }
    showCoor();
}

void showTextLine(int line){//更新一行的内容
    if(top <= line && line < bottom){
        showTextRange(line, line+1);
    }
}

void
showText()//更新整个显示区的内容
{
    int n = num_line - startline;
    if (n > bottom)
        n = bottom;
    showTextRange(top, n);
}

char*
getFileInfo()//返回文件的信息，包括文件名和文件的总行数
{
    char* result = malloc(sizeof(char) * 60);
    char* digit = intToString(num_line);

    int fl = strlen(filename);
    int dl = strlen(digit);

    int i, p = 1;
    result[0] = '\"';
    for (i = 0; i < fl; ++i)
        result[p++] = filename[i];
    result[p++] = '\"';
    for (i = 0; i < 3; ++i)
        result[p++] = ' ';
    for (i = 0; i < dl; ++i)
        result[p++] = digit[i];
    result[p++] = ' ';
    result[p++] = 'l';
    result[p++] = 'i';
    result[p++] = 'n';
    result[p++] = 'e';
    if (num_line != 1)
        result[p++] = 's';
    result[p++] = 0;
    return result;
}

void
moveCursor(int dx, int dy)//在视图坐标系下，移动光标位置
{
    cursorX += dx;
    cursorY += dy;
    if (cursorX < top){//如果光标超出了视图的顶部，
        if (startline > 0){//并且视图的第一行不是文档的第一行，
            startline--;//则将文档显示内容向上移动1行。Note:此处采用的策略，在快速移动多行时，会出错！！！
            showText();//并重现显示文本
        }
        cursorX = top;//然后将光标定位在视图的顶部。
    }
    if (cursorX > bottom - 1){//如果光标超出了视图的底部，
        if (num_line - startline > 24){//并且视图的最后一行不是文档的最后一行，
            startline++;//则将文档显示内容向下移动1行。Note:此处采用的策略，在快速移动多行时，会出错！！！
            showText();//并重现显示文本
        }
        cursorX = bottom - 1;//然后将光标定位在视图的底部。
    }
    if (startline + cursorX > num_line - 1)
        cursorX = num_line - startline - 1;//保证光标cursorX移动不能超过文档的最底部
    if (cursorY < left)
        cursorY = left;//cursorY不能超过视图的左界，
    int l = strlen(textbuf[startline + cursorX]);
    if (cursorY > l)
        cursorY = l;//同时也不能超过字符串的右端
    setconsole(-1, 0, 0, coor(cursorX, cursorY), 2);
    showCoor();//重新显示当前光标的位置
}

int
runCursorCtrl(char c)//光标的控制
{
    int t = c;
    t &= 0xff;
    switch (t){
        case KEY_UP:
            moveCursor(-1, 0);
            showTextRange(cursorX, cursorX+2);
            return 1;
        case KEY_DN:
            moveCursor(1, 0);
            showTextRange(cursorX-1, cursorX+1);
            return 1;
        case KEY_LF:
            moveCursor(0, -1);
            showTextLine(cursorX);
            return 1;
        case KEY_RT:
            moveCursor(0, 1);
            showTextLine(cursorX);
            return 1;
        case KEY_HOME:
            moveCursor(0, -10000);
            showTextLine(cursorX);
            return 1;
        case KEY_END:
            moveCursor(0, 10000);
            showTextLine(cursorX);
            return 1;
        case KEY_INS:
            mode = 1;
            showMessage("-- INSERT --");
            return 1;
    }
    return 0;
}

void
insertline()//在当前光标所在位置的下一行插入新的一行
{
    int i = 0, j = 0, l, textX = cursorX + startline + 1;//插入行在文档中的行数为textX
    num_line++;//总行数增加
    for (i = num_line - 1; i > textX; --i){//将textX以下的所有内容向下平移1行
        l = strlen(textbuf[i - 1]);
        for (j = 0; j < l; ++j)
            textbuf[i][j] = textbuf[i - 1][j];
        textbuf[i][j] = 0;
    }
    textbuf[num_line][j] = 0;//最后一行内容为空
    textbuf[textX][0] = 0;//插入行的内容为空
}

void
deleteline(int offset)//删除一行内容，该行相对于当前的光标位置的偏移量为offset
{
    int i, j, l, textX = cursorX + startline + offset;//删除行在文档的行数为textX
    num_line--;//总行数减少
    for (i = textX; i < num_line; ++i){//将textX以下的所有内容向上平移1行
        l = strlen(textbuf[i + 1]);
        for (j = 0; j < l; ++j)
            textbuf[i][j] = textbuf[i + 1][j];
        textbuf[i][j] = 0;
    }
    textbuf[num_line][0] = 0;//最后一行内容为空。
    if (num_line == 0)
        insertline();//保证文档中至少有一行内容
}

void
delete()//删除1个字符
{
    int i, j, l1, l2, textX = cursorX + startline, textY = cursorY + left;//光标在文档中的行与列
    if (cursorY == left){//如果光标位置在当前行的最左端
        if (textX == 0)//同时还是第一行
            return ;//则无法删除字符，直接返回
        l1 = strlen(textbuf[textX - 1]);
        l2 = strlen(textbuf[textX]);
        for (i = 0, j = l1; i < l2 && j < MAX_LENGTH; ++i, ++j)//将下一行的内容复制到上一行
            textbuf[textX - 1][j] = textbuf[textX][i];//Note:此处的复制策略比较简单粗暴，存在着一个潜在的bug，当下一行较长时，可能会丢失后半段的字符
        textbuf[textX - 1][j] = 0;
        deleteline(0);//然后删除下一行，
        moveCursor(-1, l1+1);//将光标移动到上一行的末尾
        showTextRange(cursorX, bottom);//下一行以下全部刷新
        return;
    }
	//如果光标位置不是在当前行的最左端
    l1 = strlen(textbuf[textX]);
    for (i = textY - 1; i < l1; i++){//将textY右边的字符整体左移一格
        textbuf[textX][i] = textbuf[textX][i + 1];
    }
}

void
insert(char c)//插入一个字符
{
    int i, l, textX = cursorX + startline, textY = cursorY + left;
    l = strlen(textbuf[textX]);//当前行的长度
    if (mode == 1){//如果处于插入模式，
        if (l >= MAX_LENGTH)//如果当前行超过了最长长度，则直接返回
            return ;//Note：这样的实现较为简洁，但是不太合理，比较合理的实现方式应该是将字符插入到下一行？
        for (i = l; i > textY; --i)//将当前行textY以后的字符串向右平移一个单位
            textbuf[textX][i] = textbuf[textX][i - 1];
        textbuf[textX][l + 1] = 0;
        textbuf[textX][textY] = c;
    }
    if (mode == 0 || mode == 2){//如果处于控制或者替换模式
        textbuf[textX][textY] = c;//直接将当前位置的字符替换
        if (textY == l)//如果替换的字符原先是‘0’，
            textbuf[textX][l + 1] = 0;//还需要在当前行末尾补一个‘0’
    }
}

void
runTextInput(char c)//对输入的字符进行处理
{
    int i, j, textX = cursorX + startline, textY = cursorY + left;
    int l = strlen(textbuf[textX]);
    switch(c){
        case NEW_LINE: // 输入Enter时，
            insertline();//首先在当前位置的下一行插入新的一行
            for (i = textY, j = 0; i < l; ++i, ++j){//将当前textY右边的字符串移动到下一行
                textbuf[textX + 1][j] = textbuf[textX][i];
                textbuf[textX][i] = 0;
            }
            moveCursor(1, -10000);//将光标移动到下一行的开头            
            showTextRange(cursorX-1, bottom);
            break;
        case BACKSPACE: // 输入Backspace时
            delete();//则删除一个字符
            moveCursor(0,-1);
            showTextLine(cursorX);
            break;
        case HORIZONTAL_TAB:
            do{//移动到cursorY为4的倍数的位置
                insert(' ');
                moveCursor(0,1);
            }while((cursorY % 4) != 0);
            showTextLine(cursorX);
            break;
        default://其余情况
            insert(c);//直接插入字符
            moveCursor(0,1);
            showTextLine(cursorX);
            break;
    }
}

/*
command list
i Insert at cursor position
o Open a new line
a Insert at next position
A Insert at end of line
r Replace character
R Replace mode
x Delete character
h move left
j move down
k move up
l move right
d delete a line
:q quit
:w save
:wq save & quit
:q! quit without save
*/
void
runCommand()
{
    if (controlbuf[0] != ':')
        return ;
    if (controlbuf[1] == 'w' && (controlbuf[2] == ' ' || controlbuf[2] == 0 || controlbuf[2] == 'q')){
        if (controlbuf[2] == ' '){//如果输入为":w_filename",则将当前内容存到filename中
            int i, l = strlen(controlbuf);
            for (i = 3; i < l; ++i)
                filename[i - 3] = controlbuf[i];
            filename[i - 3] = 0;
        }
        if (saveFile(filename) != 0){//如果输入为“：w”，则将当前内容存到原先打开的文件中
            showMessage("Save failed!");
            return ;
        }
        showMessage("Save succeed!");
        if (controlbuf[2] == 'q')//如果输入为“：wq”，则在保存后，退出程序
            quit();
        return ;
    }
    if (controlbuf[1] == 'q'){
        if (controlbuf[2] == '!' && controlbuf[3] == 0)//如果输入为“：q!",则强制退出
            quit();
        if (controlbuf[2] == 0){//如果输入是":q"
            if (saved == 0){//如果当前内容尚未保存，
                showMessage("File isn't saved. Force quit by \'q!\'");//则退出失败
                return;
            }
            else//否则
                quit();//可以退出
        }
    }
    showMessage("Invalid command");//其余情况，则显示命令非负
}

void
clearControl()//清除命令缓冲区
{
    controlp = 0;
    controlbuf[0] = 0;
}

void
runControl()
{
    switch (controlbuf[0]){
        case 'i'://在当前光标处开始插入字符
            mode = 1;
            showMessage("-- INSERT --");
            break;
        case 'o'://新建一行并开始插入字符
            mode = 1;
            insertline();
            moveCursor(1, 0);
            showText(cursorX-1, bottom);
            showMessage("-- INSERT --");
            break;
        case 'a'://在当前光标下一个位置开始插入插入字符
            mode = 1;
            moveCursor(0, 1);
            showMessage("-- INSERT --");
            break;
        case 'A'://在行尾开始插入字符
            mode = 1;
            moveCursor(0, 10000);
            showMessage("-- INSERT --");
            break;
        case 'r'://输入"ra"时
            if (isTextChar(controlbuf[1])){//若a是一个显示字符
                insert(controlbuf[1]);//将其插入光标所在位置
                // moveCursor(0, -1);//光标重新移到a左边的位置
                showTextLine(cursorX);
                break;
            }
            if (controlbuf[1] != 0)
                break;
            return ;
        case 'R'://进入替换模式
            mode = 2;
            showMessage("-- REPLACE --");
            break;
        case 'x'://删除光标出的字符
            moveCursor(0, 1);
            delete();
            showTextLine(cursorX);
            saved = 0;
            break;
        case 'h'://光标向左移动一个字符
            if (cursorY == 0){
                moveCursor(-1, 10000);
                showTextRange(cursorX,cursorX+2);
            }
            else{
                moveCursor(0, -1);
                showTextLine(cursorX);
            }
            break;
        case 'j'://光标向下移动一个字符
            moveCursor(1, 0);
            showTextRange(cursorX-1, cursorX+1);
            break;
        case 'k'://光标向上移动一个字符
            moveCursor(-1, 0);
            showTextRange(cursorX, cursorX+2);
            break;
        case 'l'://向右移动一个字符
            if (cursorY >= strlen(textbuf[cursorX + startline]) && 
            cursorX + startline != num_line - 1){
                moveCursor(1, -10000);
                showTextRange(cursorX-1, cursorX+1);
            }
            else{
                moveCursor(0, 1);
                showTextLine(cursorX);
            }   
            break;
        case 'd'://删除光标所在行
            deleteline(0);
            moveCursor(0, -10000);
            showText(cursorX-1, bottom);
            showMessage("1 line deleted");
            saved = 0;
            break;
        case ':'://当输入':'时，
            showMessage(controlbuf);//需要显示控制缓冲区内容
            return ;
        default:
            clearControl();
            break;
    }
    clearControl();
}

void
parseInput(char c)//综合上述各个处理函数对输入字符c进行处理
{
    if (c == KEY_ESC){//如果输入的是ESC
        mode = 0;//则进入控制模式
        clearControl();
        showMessage("");
        return ;
    }
    if (runCursorCtrl(c) != 0){//首先判断是否为光标的移动键
        clearControl();
        return ;
    }
    if (!isTextChar(c) && !isControlChar(c))
        return ;//确保字符c是一个显示字符或者控制字符中的一种
    if (mode == 0){//当前处于控制模式
        showMessage("");
        if (controlp >= 60)//控制字符串的长度不能超过60
            return ;
        if (c == ':'){//每当输入“：”时，
            clearControl();//都需要清空控制字符串
        }
        if (c == NEW_LINE){//当输入换行键时
            runCommand();//运行保存/退出等命令
            clearControl();//清空控制字符串
            return ;
        }
        if (c == BACKSPACE)//当输入退格键时
            controlp--;//光标向左移
        else//其余情况
            controlbuf[controlp++] = c;//输入字符c
        controlbuf[controlp] = 0;
        runControl();//并且尝试执行控制命令
    }
    else{//如果处于非控制模式
        saved = 0;//只要输入字符，当前文件就尚未保存
        runTextInput(c);//并按照文本内容处理字符c
    }
}

int
main(int argc, char *argv[])
{
    char buf[1];
    int i;

    if (argc > 1){
        if (openFile(argv[1]) < 0)
        {
            printf(1, "vim: cannot open %s\n", argv[1]);
            exit();
        }
        for (i = 0; i < strlen(argv[1]); ++i)
            filename[i] = argv[1][i];//将命令行的第1个参数作为文件名filename
        filename[i] = 0;
        if (num_line == 0)//如果文件中原先没有内容，则
            insertline();//插入一行内容
        moveCursor(-100,-100);//光标移到左上方
    }
    else{
        printf(1, "vim: please input filename\n");
        exit();
    }
    initColorConfiguration();//初始化颜色配置
    cursorX = cursorY = 0;
    init();//首先清空显示区的内容
    showText();//将文档内容显示在显示区
    showMessage(getFileInfo());//在最后一行显示当前文件的文件名和行数
    int n;
    while ((n = read(0, buf, sizeof(buf))) > 0){//不断从键盘读入字符
        if (buf[0] != 0)
            parseInput(buf[0]);//并进行处理
    }
    
  return 0;
}

int combineColor(int textColor, int backgroundColor)//设置字体的颜色和背景的颜色
{
    return ( textColor | ( backgroundColor << 4 ) );
}