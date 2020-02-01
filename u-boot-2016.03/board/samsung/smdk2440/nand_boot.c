
/* NAND FLASH*/
#define NFCONF (*((volatile unsigned long *)0x4E000000))
#define NFCONT (*((volatile unsigned long *)0x4E000004))
#define NFCMMD (*((volatile unsigned char *)0x4E000008))
#define NFADDR (*((volatile unsigned char *)0x4E00000C))
#define NFDATA (*((volatile unsigned char *)0x4E000010))
#define NFSTAT (*((volatile unsigned char *)0x4E000020))

static int boot_isBootFromNorFlash(void)
{
	volatile int *p = (volatile int *)0;
	int val;

	val = *p;
	*p = 0x12345678;
	if (*p == 0x12345678)
	{
		*p = val;
		return 0;
	}
	else
	{
		return 1;
	}
}

void boot_nand_init(void)
{
#define TACLS   0
#define TWRPH0  4
#define TWRPH1  2

	NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
	NFCONT = (1<<4)|(1<<1)|(1<<0);
}

static void boot_nand_select(void)
{
	NFCONT &= ~(1<<1);	
}

static void boot_nand_deselect(void)
{
	NFCONT |= (1<<1);	
}

static void boot_nand_cmd(unsigned char cmd)
{
	volatile int i;
	NFCMMD = cmd;
	for (i = 0; i < 10; i++);
}

static void boot_nand_addr(unsigned int addr)
{
	unsigned int col  = addr % 2048;
	unsigned int page = addr / 2048;
	volatile int i;

	NFADDR = col & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	
	NFADDR  = page & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 16) & 0xff;
	for (i = 0; i < 10; i++);	
}

static void boot_nand_wait_ready(void)
{
	while (!(NFSTAT & 1));
}

static void boot_nand_read(unsigned int addr, unsigned char *buf, unsigned int len)
{
	int col = addr % 2048;
	int i = 0;

	/*1. chip enable*/
	boot_nand_select();

	while (i < len)
	{
		/*2. send command 0x00*/
		boot_nand_cmd(0x00);

		/*3. send address*/
		boot_nand_addr(addr);

		/*4. send command 0x30*/
		boot_nand_cmd(0x30);

		/*5. wait status*/
		boot_nand_wait_ready();

		/*6. read data*/
		for (; (col < 2048) && (i < len); col++)
		{
			buf[i] = NFDATA;
			i++;
			addr++;
		}
		
		col = 0;
	}

	/*7. chip disable*/	
	boot_nand_deselect();
}

void copy_code_nandtosdram(unsigned char *src, unsigned char *dest, unsigned int len)
{	
	boot_nand_init();
	boot_nand_read((unsigned int)src, dest, len);
#if 0
	int i = 0;
	
	if (boot_isBootFromNorFlash())
	{
		while (i < len)
		{
			dest[i] = src[i];
			i++;
		}
	}
	else
	{
		boot_nand_init();
		boot_nand_read((unsigned int)src, dest, len);
	}
#endif	
}

void boot_clear_bss(void)
{
	extern int __bss_start, __bss_end;
	int *p = &__bss_start;
	
	for (; p < &__bss_end; p++)
		*p = 0;
}

/*uart debug code*/
/*liup: add for test printf*/
#define GPFCON_TEST      (*(volatile unsigned long *)0x56000050)
#define GPFDAT_TEST      (*(volatile unsigned long *)0x56000054)
#define TEST_LED_DELAY 0x20000

#define GPHCON_TEST              (*(volatile unsigned long *)0x56000070)
#define GPHDAT_TEST              (*(volatile unsigned long *)0x56000074)
#define GPHUP_TEST               (*(volatile unsigned long *)0x56000078)

/*UART registers*/
#define ULCON0_TEST              (*(volatile unsigned long *)0x50000000)
#define UCON0_TEST               (*(volatile unsigned long *)0x50000004)
#define UFCON0_TEST              (*(volatile unsigned long *)0x50000008)
#define UMCON0_TEST              (*(volatile unsigned long *)0x5000000c)
#define UTRSTAT0_TEST            (*(volatile unsigned long *)0x50000010)
#define UTXH0_TEST               (*(volatile unsigned char *)0x50000020)
#define URXH0_TEST               (*(volatile unsigned char *)0x50000024)
#define UBRDIV0_TEST             (*(volatile unsigned long *)0x50000028)

#define	LOCKTIME_TEST	(*(volatile unsigned long *)0x4c000000)
#define	MPLLCON_TEST	(*(volatile unsigned long *)0x4c000004)
#define	UPLLCON_TEST	(*(volatile unsigned long *)0x4c000008)
#define	CLKCON_TEST		(*(volatile unsigned long *)0x4c00000c)
#define	CLKSLOW_TEST	(*(volatile unsigned long *)0x4c000010)
#define	CLKDIVN_TEST		(*(volatile unsigned long *)0x4c000014)

/* S3C2440: Mpll = (2*m * Fin) / (p * 2^s), UPLL = (m * Fin) / (p * 2^s)
 * m = M (the value for divider M)+ 8, p = P (the value for divider P) + 2
 */
 #define S3C2440_MPLL_400MHZ_TEST     ((0x5c<<12)|(0x01<<4)|(0x01))
#define S3C2440_MPLL_100MHZ_TEST     ((0x5c<<12)|(0x01<<4)|(0x03))
#define S3C2440_UPLL_48MHZ_TEST      ((0x38<<12)|(0x02<<4)|(0x02))
#define S3C2440_CLKDIV_TEST          (0x05) // | (1<<3))    /* FCLK:HCLK:PCLK = 1:4:8, UCLK = UPLL/2 */


#define TXD0READY_TEST   (1<<2)
#define RXD0READY_TEST   (1)

#define PCLK_TEST            50000000    // init.c?Dµ?clock_inito¯êyéè??PCLK?a50MHz
#define UART_CLK_TEST        PCLK_TEST        //  UART0µ?ê±?ó?'éè?aPCLK
#define UART_BAUD_RATE_TEST  115200      // 2¨ì??ê
#define UART_BRD_TEST        ((UART_CLK_TEST  / (UART_BAUD_RATE_TEST * 16)) - 1)

static inline void boot_delay (unsigned long loops)
{
    __asm__ volatile ("1:\n"
      "subs %0, %1, #1\n"
      "bne 1b":"=r" (loops):"0" (loops));
}

void boot_clock_init(void)
{    
	/* FCLK:HCLK:PCLK = 1:4:8 */    
	CLKDIVN_TEST = S3C2440_CLKDIV_TEST;    
	/* change to asynchronous bus mod */    
	__asm__(    "mrc    p15, 0, r1, c1, c0, 0\n"    /* read ctrl register   */              
				"orr    r1, r1, #0xc0000000\n"      /* Asynchronous         */              
				"mcr    p15, 0, r1, c1, c0, 0\n"    /* write ctrl register  */              
				:::"r1"            );    /* to reduce PLL lock time, adjust the LOCKTIME register */    
	LOCKTIME_TEST = 0xFFFFFFFF;    /* configure UPLL */    
	UPLLCON_TEST = S3C2440_UPLL_48MHZ_TEST;    /* some delay between MPLL and UPLL */    
	boot_delay (4000);    /* configure MPLL */    
	MPLLCON_TEST = S3C2440_MPLL_400MHZ_TEST;    /* some delay between MPLL and UPLL */    
	boot_delay (8000);
}

void boot_uart0_init(void)
{    
	GPHCON_TEST  |= 0xa0;    // GPH2,GPH3ó?×÷TXD0,RXD0    
	GPHUP_TEST   = 0x0c;     // GPH2,GPH3?ú2?é?à-    
	ULCON0_TEST  = 0x03;     // 8N1(8??êy?Y?????T???é??1??í??1??)    
	UCON0_TEST   = 0x05;     // 2é?¯·?ê???UARTê±?ó?'?aPCLK    
	UFCON0_TEST  = 0x00;     // 2?ê1ó?FIFO    
	UMCON0_TEST  = 0x00;     // 2?ê1ó?á÷??    
	UBRDIV0_TEST = UART_BRD_TEST; // 2¨ì??ê?a115200
}

void boot_putc(unsigned char c)
{
    while (!(UTRSTAT0_TEST & TXD0READY_TEST));
    UTXH0_TEST = c;
    while (!(UTRSTAT0_TEST & TXD0READY_TEST));
}

int boot_print_string(unsigned char *s)
{
    while( *s != 0)
    {
        boot_putc(*s);
        s++;
    }
    
    return(0);
}

void boot_print_hex(unsigned int in_U32)
{
    unsigned char buffer[8];
    unsigned char ch;
    int i = 0;

    while(in_U32 != 0)
    {
        ch = in_U32 & 0x0F;
        in_U32 >>= 4;

        if( ch<10 )
        {
            buffer[i++] = ch+'0';
        }
        else
        {
            buffer[i++] = ch-10+'a';
        }        
    }

    if(i == 0)
    {
        boot_putc('0');
    }
    else
    {
        while(i > 0)
        {
            boot_putc(buffer[i - 1]);
            i --;
        }
    }
    boot_putc('h');
}

void boot_uart_init(void)
{
	boot_clock_init();
	//boot_uart0_init();
	//boot_print_hex(0x1234);
	//boot_putc('A');
	//boot_putc('\n');
	//boot_print_string("boot_uart_init\n");
}