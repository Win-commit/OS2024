#include "x86.h"
#include "device.h"

extern int displayRow;
extern int displayCol;

extern uint32_t keyBuffer[MAX_KEYBUFFER_SIZE];
extern int bufferHead;
extern int bufferTail;

int tail=0;

void GProtectFaultHandle(struct TrapFrame *tf);

void KeyboardHandle(struct TrapFrame *tf);

void syscallHandle(struct TrapFrame *tf);
void syscallWrite(struct TrapFrame *tf);
void syscallPrint(struct TrapFrame *tf);
void syscallRead(struct TrapFrame *tf);
void syscallGetChar(struct TrapFrame *tf);
void syscallGetStr(struct TrapFrame *tf);


void irqHandle(struct TrapFrame *tf) { // pointer tf = esp
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds"::"a"(KSEL(SEG_KDATA)));

	switch(tf->irq) {
		// TODO: 填好中断处理程序的调用

		case(0xd):GProtectFaultHandle(tf);break;
		case(0x21):KeyboardHandle(tf);break;
		case(0x80):syscallHandle(tf);break;
		case(-1):break;


		default:assert(0);
	}
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}


void KeyboardHandle(struct TrapFrame *tf){
	uint32_t code = getKeyCode();

	if(code == 0xe){ // 退格符
		//要求只能退格用户键盘输入的字符串，且最多退到当行行首
		if(displayCol>0&&displayCol>tail){
			displayCol--;
			uint16_t data = 0 | (0x0c << 8);
			int pos = (80*displayRow+displayCol)*2;
			asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
		}
	}else if(code == 0x1c){ // 回车符
		//处理回车情况
		keyBuffer[bufferTail++]='\n';
		displayRow++;
		displayCol=0;
		tail=0;
		if(displayRow==25){
			scrollScreen();
			displayRow=24;
			displayCol=0;
		}
	}else if(code < 0x81){ 
		// TODO: 处理正常的字符
		char displayChar = getChar(code); 
    		if (displayChar) { // 确保字符可显示
        		keyBuffer[bufferTail++] = displayChar; // 将字符存储到 keyBuffer

        		
        		uint16_t data = (uint16_t)displayChar | (0x0c << 8); // 字符与颜色数据合并
        		int pos = (80 * displayRow + displayCol) * 2; // 计算位置
        		asm volatile("movw %0, (%1)"::"r"(data), "r"(pos + 0xb8000)); // 写入显存

        		displayCol++; // 更新列位置
        		if (displayCol >= 80) { // 行末，自动换行
            		displayRow++;
            		displayCol = 0;
            		if (displayRow == 25) { // 屏幕底部，滚动屏幕
                		scrollScreen();
                		displayRow = 24;
            }
        }

	}
	updateCursor(displayRow, displayCol);
	}
}


void syscallHandle(struct TrapFrame *tf) {
	switch(tf->eax) { // syscall number
		case 0:
			syscallWrite(tf);
			break; // for SYS_WRITE
		case 1:
			syscallRead(tf);
			break; // for SYS_READ
		default:break;
	}
}

void syscallWrite(struct TrapFrame *tf) {
	switch(tf->ecx) { // file descriptor
		case 0:
			syscallPrint(tf);
			break; // for STD_OUT
		default:break;
	}
}

void syscallPrint(struct TrapFrame *tf) {
	int sel =  USEL(SEG_UDATA);
	char *str = (char*)tf->edx;
	int size = tf->ebx;
	int i = 0;
	int pos = 0;
	char character = 0;
	uint16_t data = 0;
	asm volatile("movw %0, %%es"::"m"(sel));
	for (i = 0; i < size; i++) {
		asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(str+i));
		// TODO: 完成光标的维护和打印到显存

		if (character == '\n') { // 处理换行符
        displayRow++;
        displayCol = 0;
        if (displayRow >= 25) { 
            scrollScreen();
            displayRow = 24; 
        }
    	} else if (character == '\r') { // 处理回车符
        displayCol = 0;
    	} else { // 处理普通字符
        data = (uint16_t)character | (0x0c << 8); // 设定字符颜色属性
        pos = (80 * displayRow + displayCol) * 2; // 计算字符在显存中的位置
        asm volatile("movw %0, (%1)"::"r"(data), "r"(pos + 0xb8000)); // 写入显存
        displayCol++; // 移动到下一列
        if (displayCol >= 80) { // 行末，自动换行
            displayRow++;
            displayCol = 0;
            if (displayRow >= 25) { // 超过屏幕底部，需要滚动
                scrollScreen();
                displayRow = 24; // 保持在屏幕最底部
            }
        }
    }

	}
	tail=displayCol;
	updateCursor(displayRow, displayCol);
}

void syscallRead(struct TrapFrame *tf){
	switch(tf->ecx){ //file descriptor
		case 0:
			syscallGetChar(tf);
			break; // for STD_IN
		case 1:
			syscallGetStr(tf);
			break; // for STD_STR
		default:break;
	}
}

void syscallGetChar(struct TrapFrame *tf){
	// TODO: 自由实现
	if (bufferHead == bufferTail) {
        // 缓冲区为空，没有字符可读
        tf->eax = 0; 
    } else {
        // 读取缓冲区中的字符
        char ch = keyBuffer[bufferHead++];
        if (bufferHead >= MAX_KEYBUFFER_SIZE) {
            bufferHead = 0; // 循环缓冲区
        }

        tf->eax = ch; // 将字符返回给用户程序，存储在 eax 寄存器中
    }
}

void syscallGetStr(struct TrapFrame *tf){
	// TODO: 自由实现

	// 设置段寄存器
	int sel = USEL(SEG_UDATA);
    asm volatile("movw %0, %%es"::"m"(sel));

    // 获取用户空间的指针和请求的字符数量
    char *userBuffer = (char *)tf->edx;
    int requestedLength = tf->ebx;
    int availableLength = bufferTail - bufferHead;
    
    // 如果缓冲区中没有数据或请求长度为0，直接返回
    if (availableLength <= 0 || requestedLength <= 0) {
        tf->eax = 0;  // 返回已读取的字符数量，这里是0
        return;
    }

    // 计算实际可以复制的字符数量，避免越界
    int copyLength = (availableLength < requestedLength) ? availableLength : requestedLength;
    int i;
	int temp=(bufferTail-1 >0) ? bufferTail-1:MAX_KEYBUFFER_SIZE-1;
	// 检查缓冲区是否已经读完
	if(keyBuffer[temp]!='\n'){tf->eax = 0;return;}
    
	// 复制字符到用户空间
    for (i = 0; i < copyLength; i++) {
        char character = keyBuffer[bufferHead++];
        asm volatile("movb %1, %%es:(%0)"::"r"(userBuffer + i), "r"(character));
        if (bufferHead >= MAX_KEYBUFFER_SIZE) {
            bufferHead = 0; // 循环缓冲区
        }
        if (character == '\n') { // 遇到换行符，停止读取
			char ch='\0';
			asm volatile("movb %1, %%es:(%0)"::"r"(userBuffer + i), "r"(ch));
            i++;
            break;
        }
    }


    // 更新 bufferHead 按照实际读取的字符数
    if (bufferHead == bufferTail) {
        bufferHead = bufferTail = 0; // 重置缓冲区索引，如果已经读完
    }

    tf->eax = i; // 返回读取的字符数

}
