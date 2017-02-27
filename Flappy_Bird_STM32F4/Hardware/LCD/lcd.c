#include "lcd.h"
#include "font.h" 
//////////////////////////////////////////////////////////////////////////////////	 

DMA_HandleTypeDef hdma2_fsmc;

//LCD的画笔颜色和背景色	   
uint16_t POINT_COLOR=0x0000;	//画笔颜色
uint16_t BACK_COLOR=0xFFFF;  //背景色 

//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;
	 
//写寄存器函数
//regval:寄存器值
void LCD_WR_REG(__IO uint16_t regval)
{   
	regval=regval;		//使用-O2优化的时候,必须插入的延时
	LCD->REG=regval;//写入要写的寄存器序号	 
}
//写LCD数据
//data:要写入的值
void LCD_WR_DATA(__IO uint16_t data)
{	 
	data=data;			//使用-O2优化的时候,必须插入的延时
	LCD->RAM=data;		 
}
//读LCD数据
//返回值:读到的值
uint16_t LCD_RD_DATA(void)
{		
	__IO uint16_t ram;			//防止被优化
	ram=LCD->RAM;
	return ram;		 
}					   
//写寄存器
//LCD_Reg:寄存器地址
//LCD_RegValue:要写入的数据
inline void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{	
	LCD->REG = LCD_Reg;		//写入要写的寄存器序号	 
	LCD->RAM = LCD_RegValue;//写入数据	    		 
}	   
//读寄存器
//LCD_Reg:寄存器地址
//返回值:读到的数据
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{										   
	LCD_WR_REG(LCD_Reg);		//写入要读的寄存器序号
	delay_us(5);		  
	return LCD_RD_DATA();		//返回读到的值
}   

//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值
uint16_t LCD_BGR2RGB(uint16_t c)
{
	uint16_t  r,g,b,rgb;   
	b=(c>>0)&0x1f;
	g=(c>>5)&0x3f;
	r=(c>>11)&0x1f;	 
	rgb=(b<<11)+(g<<5)+(r<<0);		 
	return(rgb);
} 
//当mdk -O1时间优化时需要设置
//延时i
void opt_delay(uint8_t i)
{
	while(i--);
}
//读取个某点的颜色值	 
//x,y:坐标
//返回值:此点的颜色
uint16_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
 	__IO uint16_t r=0,g=0,b=0;
	if(x>=lcddev.width||y>=lcddev.height) return 0;	//超过了范围,直接返回		   
	LCD_SetCursor(x,y);	    
	LCD_WR_REG(CMD_WRITE_RAM);      		 				//其他IC发送读GRAM指令  
 	LCD_RD_DATA();									//dummy Read	   
	opt_delay(2);	  
 	r=LCD_RD_DATA();  		  						//实际坐标颜色

	if (lcddev.id == 0X7783 || lcddev.id == 0X9325 || lcddev.id == 0X4535 || lcddev.id == 0X4531 || lcddev.id == 0XB505 || lcddev.id == 0XC505) return r;	//这几种IC直接返回颜色值
}			 
//LCD开启显示
void LCD_DisplayOn(void)
{					   
	LCD_WriteReg(R7,0x0173); 				 	//开启显示
}	 
//LCD关闭显示
void LCD_DisplayOff(void)
{	   
	LCD_WriteReg(R7,0x0);//关闭显示 
}   
//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{	 
	if(lcddev.dir==1)Xpos=lcddev.width-1-Xpos;//横屏其实就是调转x,y坐标
	LCD_WriteReg(lcddev.setxcmd, Xpos);
	LCD_WriteReg(lcddev.setycmd, Ypos);
} 		 
//设置LCD的自动扫描方向
//注意:其他函数可能会受到此函数设置的影响(尤其是9341/6804这两个奇葩),
//所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
//dir:0~7,代表8个方向(具体定义见lcd.h)
//9320/9325/9328/4531/4535/1505/b505/8989/5408/9341/5310/5510等IC已经实际测试	   	   
void LCD_Scan_Dir(uint8_t dir)
{
	uint16_t regval=0;
	uint16_t dirreg=0;
	uint16_t temp;  
	uint16_t xsize,ysize;

	switch(dir)
	{
		case L2R_U2D://从左到右,从上到下
			regval|=(1<<5)|(1<<4)|(0<<3); 
			break;
		case L2R_D2U://从左到右,从下到上
			regval|=(0<<5)|(1<<4)|(0<<3); 
			break;
		case R2L_U2D://从右到左,从上到下
			regval|=(1<<5)|(0<<4)|(0<<3);
			break;
		case R2L_D2U://从右到左,从下到上
			regval|=(0<<5)|(0<<4)|(0<<3); 
			break;	 
		case U2D_L2R://从上到下,从左到右
			regval|=(1<<5)|(1<<4)|(1<<3); 
			break;
		case U2D_R2L://从上到下,从右到左
			regval|=(1<<5)|(0<<4)|(1<<3); 
			break;
		case D2U_L2R://从下到上,从左到右
			regval|=(0<<5)|(1<<4)|(1<<3); 
			break;
		case D2U_R2L://从下到上,从右到左
			regval|=(0<<5)|(0<<4)|(1<<3); 
			break;	 
	}

	dirreg=0X03;
	regval|=1<<12;  
		
	LCD_WriteReg(dirreg,regval);
}     
//画点
//x,y:坐标
//POINT_COLOR:此点的颜色
void LCD_DrawPoint(uint16_t x,uint16_t y)
{
	LCD_SetCursor(x,y);		//设置光标位置 
	LCD->REG = CMD_WRITE_RAM;	//开始写入GRAM
	LCD->RAM=POINT_COLOR; 
}
//快速画点
//x,y:坐标
//color:颜色
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{	   
 	if(lcddev.dir==1) x=lcddev.width-1-x;//横屏其实就是调转x,y坐标
	LCD_WriteReg(lcddev.setxcmd,x);
	LCD_WriteReg(lcddev.setycmd,y);		 

	LCD->REG = CMD_WRITE_RAM;
	LCD->RAM = color; 
}	 


//设置LCD显示方向
//dir:0,竖屏；1,横屏
void LCD_Display_Dir(uint8_t dir)
{
	if(dir==0)			//竖屏
	{
		lcddev.dir=0;	//竖屏
		lcddev.width=240;
		lcddev.height=320;

	 	lcddev.setxcmd=R32;
		lcddev.setycmd=R33;  

	}else 				//横屏
	{	  				
		lcddev.dir=1;	//横屏
		lcddev.width=320;
		lcddev.height=240;

	 	lcddev.setxcmd=R33;
		lcddev.setycmd=R32;  
	} 
	LCD_Scan_Dir(DFT_SCAN_DIR);	//默认扫描方向
}	 
//设置窗口,并自动设置画点坐标到窗口左上角(sx,sy).
//sx,sy:窗口起始坐标(左上角)
//width,height:窗口宽度和高度,必须大于0!!
//窗体大小:width*height.
//68042,横屏时不支持窗口设置!! 
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{   
	uint16_t hsaval,heaval,vsaval,veaval; 
	width=sx+width-1;
	height=sy+height-1;
	
	if (lcddev.dir == 1)//横屏
	{
		//窗口值
		hsaval = sy;
		heaval = height;
		vsaval = lcddev.width - width - 1;
		veaval = lcddev.width - sx - 1;
	}
	else
	{
		hsaval = sx;
		heaval = width;
		vsaval = sy;
		veaval = height;
	}

	LCD_WriteReg(0x50, hsaval);
	LCD_WriteReg(0x51, heaval);
	LCD_WriteReg(0x52, vsaval);
	LCD_WriteReg(0x53, veaval);
	LCD_SetCursor(sx, sy);	//设置光标位置

} 
//初始化lcd
//该初始化函数可以初始化各种ILI93XX液晶,但是其他函数是基于ILI9320的!!!
//在其他型号的驱动芯片上没有测试! 
void LCD_Init(void)
{ 	
	__IO uint32_t i=0;

	GPIO_InitTypeDef GPIO_InitStruct;
	
	__GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();//使能IO时钟  
	__HAL_RCC_FSMC_CLK_ENABLE();//使能FSMC时钟  
	
	GPIO_InitStruct.Pin = GPIO_PIN_1;			//PB1 推挽输出,控制背光
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;	//普通输出模式
	GPIO_InitStruct.Pull = GPIO_NOPULL;			//上拉
	GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;	//中速
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);		//初始化

	
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_8
						 |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;//PD0,1,4,5,8,9,10,14,15
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;	//复用推挽输出
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;//高速
	GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;//FSMC复用
	GPIO_InitStruct.Pull = GPIO_PULLUP;		//上拉
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);	//初始化

	GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8| GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | 
						  GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;//PE7~15
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);	//初始化

	SRAM_HandleTypeDef LCD_FSMC_Handler;

	LCD_FSMC_Handler.Instance = FSMC_NORSRAM_DEVICE;
	LCD_FSMC_Handler.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
	LCD_FSMC_Handler.Init.NSBank = FSMC_NORSRAM_BANK1;				//这里我们使用NE1 
	LCD_FSMC_Handler.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;	//不复用数据地址
	LCD_FSMC_Handler.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;			//FSMC_MemoryType_SRAM;  //SRAM   
	LCD_FSMC_Handler.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;	//存储器数据宽度为16bit   
	LCD_FSMC_Handler.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;	//突发访问模式关; 
	LCD_FSMC_Handler.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
	LCD_FSMC_Handler.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
	LCD_FSMC_Handler.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
	LCD_FSMC_Handler.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
	LCD_FSMC_Handler.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;	//存储器写使能
	LCD_FSMC_Handler.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
	LCD_FSMC_Handler.Init.ExtendedMode = FSMC_EXTENDED_MODE_ENABLE;		//读写使用不同的时序
	LCD_FSMC_Handler.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
	LCD_FSMC_Handler.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
	LCD_FSMC_Handler.Init.PageSize = FSMC_PAGE_SIZE_NONE;
	LCD_FSMC_Handler.Init.ContinuousClock = FSMC_CONTINUOUS_CLOCK_SYNC_ASYNC;

	FMC_NORSRAM_TimingTypeDef ReadWrite_Timing;
	FMC_NORSRAM_TimingTypeDef Write_Timing;

	ReadWrite_Timing.AddressSetupTime = 0XF;	 //地址建立时间（ADDSET）为16个HCLK 1/168M=6ns*16=96ns	
	ReadWrite_Timing.AddressHoldTime = 0x00;	 //地址保持时间（ADDHLD）模式A未用到	
	ReadWrite_Timing.DataSetupTime = 0x0F;			//数据保存时间为60个HCLK	=6*60=360ns
	ReadWrite_Timing.BusTurnAroundDuration = 0;
	ReadWrite_Timing.CLKDivision = 0x00;
	ReadWrite_Timing.DataLatency = 0x00;
	ReadWrite_Timing.AccessMode = FSMC_ACCESS_MODE_A;	 //模式A 
    
	Write_Timing.AddressSetupTime =9;	      //地址建立时间（ADDSET）为9个HCLK =54ns 
	Write_Timing.AddressHoldTime = 0x00;	 //地址保持时间（A		
	Write_Timing.DataSetupTime = 8;		 //数据保存时间为6ns*9个HCLK=54ns
	Write_Timing.BusTurnAroundDuration = 0x00;
	Write_Timing.CLKDivision = 0x00;
	Write_Timing.DataLatency = 0x00;
	Write_Timing.AccessMode = FSMC_ACCESS_MODE_A;	 //模式A 

	HAL_SRAM_Init(&LCD_FSMC_Handler, &ReadWrite_Timing, &Write_Timing);


	__HAL_RCC_DMA2_CLK_ENABLE();
	HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

	hdma2_fsmc.Instance = DMA2_Stream1;
	hdma2_fsmc.Init.Direction = DMA_MEMORY_TO_MEMORY;
	hdma2_fsmc.Init.Mode = DMA_NORMAL;
	hdma2_fsmc.Init.MemInc = DMA_MINC_DISABLE;
	hdma2_fsmc.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	hdma2_fsmc.Init.PeriphInc = DMA_PINC_ENABLE;
	hdma2_fsmc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	hdma2_fsmc.Init.Priority = DMA_PRIORITY_HIGH;
	hdma2_fsmc.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	hdma2_fsmc.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	hdma2_fsmc.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma2_fsmc.Init.PeriphBurst = DMA_PBURST_SINGLE;

	HAL_DMA_DeInit(&hdma2_fsmc);
	HAL_DMA_Init(&hdma2_fsmc);

	HAL_Delay(50); // delay 50 ms 
 	LCD_WriteReg(0x0000,0x0001);
	HAL_Delay(50); // delay 50 ms 

	lcddev.id = LCD_ReadReg(0x0000);   
//	printf(" LCD ID:%x\r\n", lcddev.id);
	
	if (lcddev.id == 0x7783)
	{
		LCD_WriteReg(0x0001, 0x0100);
		LCD_WriteReg(0x0002, 0x0700);
		LCD_WriteReg(0x0003, 0x1030);
		LCD_WriteReg(0x0007, 0x0121);
		LCD_WriteReg(0x0008, 0x0508);
		LCD_WriteReg(0x0009, 0x0200);
		LCD_WriteReg(0x000A, 0x0000);
		//***********************************
		LCD_WriteReg(0x0010, 0x0790);
		LCD_WriteReg(0x0011, 0x0005);
		LCD_WriteReg(0x0012, 0x0000);
		LCD_WriteReg(0x0013, 0x0000);
		HAL_Delay(90);
		//***********************************
		LCD_WriteReg(0x0010, 0x14B0);
		HAL_Delay(50);
		LCD_WriteReg(0x0011, 0x0007);
		HAL_Delay(50);
		LCD_WriteReg(0x0012, 0x008C);
		LCD_WriteReg(0x0013, 0x1A00);
		LCD_WriteReg(0x0029, 0x0017);
		HAL_Delay(50);
		//***********************************
		LCD_WriteReg(0x0030, 0x0000);
		LCD_WriteReg(0x0031, 0x0507);
		LCD_WriteReg(0x0032, 0x0203);
		LCD_WriteReg(0x0035, 0x0006);
		LCD_WriteReg(0x0036, 0x0102);
		LCD_WriteReg(0x0037, 0x0000);
		LCD_WriteReg(0x0038, 0x0406);
		LCD_WriteReg(0x0039, 0x0304);
		LCD_WriteReg(0x003C, 0x0007);
		LCD_WriteReg(0x003D, 0x0101);
		//***********************************
		LCD_WriteReg(0x0050, 0x0000);
		LCD_WriteReg(0x0051, 0x00EF);
		LCD_WriteReg(0x0052, 0x0000);
		LCD_WriteReg(0x0053, 0x013F);
		//***********************************
		LCD_WriteReg(0x0060, 0xA700);
		LCD_WriteReg(0x0061, 0x0001);
		LCD_WriteReg(0x0090, 0x0029);
		//***********************************
		LCD_WriteReg(0x0009, 0x0000);
		LCD_WriteReg(0x0007, 0x0133);
		HAL_Delay(50);
	}
	LCD_Display_Dir(0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);					//点亮背光
	LCD_Clear(LIGHTBLUE);
	BACK_COLOR = LIGHTBLUE;
}  
//清屏函数
//color:要清屏的填充色
void LCD_Clear(uint16_t Color)
{    
	LCD_Set_Window(0, 0, 240, 320);
	LCD->REG = CMD_WRITE_RAM;     		//开始写入GRAM	 	  
	for(uint32_t i = 0; i < lcddev.width * lcddev.height; i++)
	{
		LCD->RAM = Color;	
	}
}  

void LCD_ColorFill(uint16_t PosX,uint16_t PosY,uint16_t Width,uint16_t Height,uint16_t Color)
{          
	LCD_Set_Window(PosX, PosY, Width, Height);
	LCD->REG = CMD_WRITE_RAM;
	for (uint32_t i = 0; i < Width * Height; i++)
	{
		LCD->RAM = Color;
	}
		 
}  

void LCD_DrawPicture(uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height,uint16_t *Color_Buffer)
{  
	LCD_Set_Window(PosX, PosY, Width, Height);
	LCD->REG = CMD_WRITE_RAM;

	HAL_DMA_Start(&hdma2_fsmc, Color_Buffer, &LCD->RAM, Width * Height); // I LOVE DMA LOL :D
	HAL_DMA_PollForTransfer(&hdma2_fsmc, HAL_DMA_FULL_TRANSFER, 30);
}  
//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		LCD_DrawPoint(uRow,uCol);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}    
//画矩形	  
//(x1,y1),(x2,y2):矩形的对角坐标
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		LCD_DrawPoint(x0+a,y0-b);             //5
 		LCD_DrawPoint(x0+b,y0-a);             //0           
		LCD_DrawPoint(x0+b,y0+a);             //4               
		LCD_DrawPoint(x0+a,y0+b);             //6 
		LCD_DrawPoint(x0-a,y0+b);             //1       
 		LCD_DrawPoint(x0-b,y0+a);             
		LCD_DrawPoint(x0-a,y0-b);             //2             
  		LCD_DrawPoint(x0-b,y0-a);             //7     	         
		a++;
		//使用Bresenham算法画圆     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
} 									  
//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16/24
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode)
{  							  
    uint8_t temp,t1,t;
	uint16_t y0=y;
	uint8_t csize=(size/8+((size%8)?1:0))*(size/2);		//得到字体一个字符对应点阵集所占的字节数	
	//设置窗口		   
	num=num-' ';//得到偏移后的值
	for(t=0;t<csize;t++)
	{   
		if(size==12)temp=asc2_1206[num][t]; 	 	//调用1206字体
		else if(size==16)temp=asc2_1608[num][t];	//调用1608字体
		else if(size==24)temp=asc2_2412[num][t];	//调用2412字体
		else return;								//没有的字库
		for(t1=0;t1<8;t1++)
		{			    
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
			else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR);
			temp<<=1;
			y++;
			if(y>=lcddev.height)return;		//超区域了
			if((y-y0)==size)
			{
				y=y0;
				x++;
				if(x>=lcddev.width)return;	//超区域了
				break;
			}
		}  	 
	}  	    	   	 	  
}   
//m^n函数
//返回值:m^n次方.
uint32_t LCD_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}			 
//显示数字,高位为0,则不显示
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//color:颜色 
//num:数值(0~4294967295);	 
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0); 
	}
} 
//显示数字,高位为0,还是显示
//x,y:起点坐标
//num:数值(0~999999999);	 
//len:长度(即要显示的位数)
//size:字体大小
//mode:
//[7]:0,不填充;1,填充0.
//[6:1]:保留
//[0]:0,非叠加显示;1,叠加显示.
void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
{  
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				if(mode&0X80)LCD_ShowChar(x+(size/2)*t,y,'0',size,mode&0X01);  
				else LCD_ShowChar(x+(size/2)*t,y,' ',size,mode&0X01);  
 				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,mode&0X01); 
	}
} 
//显示字符串
//x,y:起点坐标
//width,height:区域大小  
//size:字体大小
//*p:字符串起始地址		  
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint8_t *p)
{         
	uint8_t x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//退出
        LCD_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }  
}

/*
void LCD_Show_Chinese_Char(uint16_t x, uint16_t y, uint8_t index, uint8_t mode)
{
	uint8_t temp, i, j;
	uint16_t y0 = y;

	for (i = 0; i < 72; i++)
	{
		temp = CHN_2412[index][i];			//得到点阵数据                          
		for (j = 0; j < 8; j++)
		{
			if (temp & 0x80)LCD_Fast_DrawPoint(x, y, POINT_COLOR);
			else if (!mode)LCD_Fast_DrawPoint(x, y, BACK_COLOR);
			temp <<= 1;
			y++;
			if ((y - y0) == 24)
			{
				y = y0;
				x++;
				break;
			}
		}
	}
}

void LCD_Show_Chinese_Str(uint16_t x, uint16_t y, uint8_t* str, uint8_t colorful)
{
	while (*str)
	{
		if (colorful) POINT_COLOR = rand() * 16;//make a big news

		LCD_Show_Chinese_Char(x, y, *str, 0);
		x += 24;

		if (x > 240) return;
		str++;
	}
}
*/
void delay_us(uint32_t nus)
{
	uint32_t i, j;
	__IO uint32_t k;
	for (i = 0; i<nus; i++)
	{
		for (j = 0; j<50; j++)
		{
			k++;
		}
	}
}






























