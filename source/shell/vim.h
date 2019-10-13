//定义了编辑栏的大小
#define CONSOLE_HEIGHT 25
#define CONSOLE_WIDTH 80
#define MAX_LINE 100
#define MAX_LENGTH 80

//按键命令
#define KEY_HOME        0xE0
#define KEY_END         0xE1
#define KEY_UP          0xE2
#define KEY_DN          0xE3
#define KEY_LF          0xE4
#define KEY_RT          0xE5
#define KEY_PGUP        0xE6
#define KEY_PGDN        0xE7
#define KEY_INS         0xE8
#define KEY_DEL         0xE9
#define KEY_ESC         0x1B

//输入的控制符
#define BACKSPACE 		8//退格符
#define NEW_LINE		10//换行符
#define CARRIAGE_RETURN 13//回车键
#define HORIZONTAL_TAB  9//水平制表符

int top = 0, bottom = 24, left = 0, right = 80;
int startline = 0;//标记视图中的第一行在实际文本中的行数
int cursorX, cursorY;//光标在视图中的行和列
int num_line = 0; //总行数
int mode = 0; //控制模式
//0,Control 
//1,Insert 
//2,Replace

char filename[100]; //文件名
char textbuf[MAX_LINE][MAX_LENGTH + 20];//文档中的字符内容
int saved = 1;//是否已经保存

char controlbuf[65];//控制字符的缓冲区
int controlp = 0;

//颜色配置
int BACKGROUND_COLOR;//背景色
int RESERVERED_WORD_COLOR;//保留字的颜色
int PARAMETER_COLOR;//常量的颜色
int VARIABLE_COLOR;//变量的颜色
int CONTROL_LINE_COLOR;//控制栏的颜色
int CURRENT_POSITION_COLOR;//当前光标位置的颜色
int CURRENT_LINE_COLOR;//当前行的颜色
int EMPHASIZE_WORD_COLOR;//强调字的颜色
int OPERATOR_COLOR;//运算符的颜色

int charColor[80];//标记某一行各个位置的单词的颜色
