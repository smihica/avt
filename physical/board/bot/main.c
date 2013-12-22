//----------------------------------------------------------------------------
// C main line
//----------------------------------------------------------------------------
#include <m8c.h>        // part specific constants and macros
#include <stdlib.h>
#include "delay.h"
#include "PSoCAPI.h"    // PSoC API definitions for all User Modules

#define WAIT_FOR_SEND_1() { while( !( UART_1_bReadTxStatus() & UART_1_TX_BUFFER_EMPTY ) ); }
#define WAIT_FOR_SEND_2() { while( !( UART_2_bReadTxStatus() & UART_2_TX_BUFFER_EMPTY ) ); }
#define OFF(port, bit)  PRT##port##DR = (PRT##port##DR & (~bit))
#define ON(port, bit)   PRT##port##DR = (PRT##port##DR | bit)
#define BATTSW0  (1 << 6)
#define BATTSW1  (1 << 4)
#define LATCH    (1 << 2)
#define SCK      (1 << 0)
#define DATA     (1 << 6)
#define RX_SIZE  95

#define ARGLEN 6
unsigned char data[ARGLEN]; // dir, pan, acc, battsw0, battsw1,

void init_aq(void)
{
	unsigned char i;
	while (1) {
		UART_2_CPutString("?");
		WAIT_FOR_SEND_2();
		for(i = 0; i < 200; i++) {
			Delay50uTimes(100); // delay 5ms
			if (UART_2_bCmdCheck()){
				UART_2_CmdReset();
				return;
			}
		}
	}
}

void init(void)     //IO Initialize
{
	//UART setup
	UART_1_CmdReset(); 						//Initialize receiver/cmd buf.
	UART_1_IntCntl(UART_1_ENABLE_RX_INT);   //Enable RX interrupts
	UART_1_Start(UART_1_PARITY_NONE);		//Enable UART
	
	UART_2_CmdReset(); 						//Initialize receiver/cmd buf.
	UART_2_IntCntl(UART_2_ENABLE_RX_INT);   //Enable RX interrupts
	UART_2_Start(UART_2_PARITY_NONE);		//Enable UART

	//CPU ALL Interrupt Enable
	M8C_EnableGInt;							//CPU ALL Interrupt Enable

	PWM16_1_Start();
	PWM16_2_Start();
	PWM16_3_Start();
	PWM16_4_Start();
	// DAC8_2_Start(DAC8_2_HIGHPOWER);			// power up the DAC
	// DAC8_2_WriteStall(0);
	
	ON(0, LATCH);
	ON(0, SCK);
	OFF(0, BATTSW0);
	OFF(0, BATTSW1);
	
	data[0] = 127; // accel
	data[1] = 127; // direction
	data[2] = 127; // pan_h
	data[3] = 127; // pan_v
	data[4] = 0;   // batt1
	data[5] = 0;   // batt2
	
	// setup aquestalk
	init_aq();
}

void serial_print(const char * s, unsigned int l, unsigned int crlf)
{
	unsigned int i;
	for(i=0; i < l; i++){
		UART_1_SendData( *(s + i) );
		WAIT_FOR_SEND_1();
	}
	if (crlf != 0) UART_1_PutCRLF();
}

void serial_print_2(char * s, unsigned int l, unsigned int crlf)
{
	unsigned int i;
	for(i=0; i < l; i++){
		UART_2_SendData( *(s + i) );
		WAIT_FOR_SEND_2();
	}
	if (crlf != 0) UART_2_PutCRLF();
}

void send_test(void) //send test
{
	char *send_buf; //RS232C
	unsigned int c;
	char data[50]="ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	UART_1_CPutString("\r\nPSoC UART RS-232C program \r\n");
	WAIT_FOR_SEND_1();

	send_buf = &data[0];
	for(c=0; c<26; c++){
		UART_1_SendData( *(send_buf + c) );
		WAIT_FOR_SEND_1();
	}
	UART_1_PutCRLF();
}

void command_w(void)
{
	char *recvd;
	char buff[4];
	char send_buff[RX_SIZE];
	unsigned char c, i, j;
	char d;

	recvd = UART_1_szGetRestOfParams();
	for ( c = 0, i = 0, j = 0; c < RX_SIZE; c++ ) {
		d = *(recvd + c);
		if (d == ' ' || d == '\r' || d == '\n' || d == '\0') {
			buff[i] = '\0';
			data[j] = (unsigned char)atoi(buff);
			j++;
			i = 0;
			continue;
		}
		buff[i] = d;
		i++;
	}
	send_buff[0] = 'p';
	send_buff[1] = ' ';
	for(c = 2, j = 0; c < RX_SIZE; c++, j++){
		itoa(send_buff + c,(int)(data[j]), 10);
		if ((ARGLEN - 1) <= j) break;
		while (send_buff[c] != '\0') c++;
		send_buff[c] = ' ';
	}
	for(c = 0; c < RX_SIZE; c++){
		d = *(send_buff + c);
		if ( d == '\0' ) break;
		UART_1_SendData( d );
		WAIT_FOR_SEND_1();
	}
	UART_1_PutCRLF();
	UART_1_CmdReset();
}

void command_v(void)
{
	char *recvd;
	char buff[RX_SIZE];
	char send_buff[RX_SIZE];
	char * buff_top = &(buff[0]);
	unsigned char c, i, j;
	char d;

	recvd = UART_1_szGetRestOfParams();
	for ( c = 0, i = 0; c < RX_SIZE; c++ ) {
		d = *(recvd + c);
		if (d == '\r' || d == '\n' || d == '\0') {
			buff[i] = '\r';
			break;
		}
		buff[i] = d;
		i++;
	}
	buff[i] = '\r';
	send_buff[0] = 'p';
	send_buff[1] = ' ';
	for(c = 2, j = 0; c < RX_SIZE; c++, j++){
		send_buff[c] = buff[j];
		if (send_buff[c] == '\r') break;
	}
	send_buff[c] = '\0';
	for(c = 0; c < RX_SIZE; c++){
		d = *(send_buff + c);
		if ( d == '\0' ) break;
		UART_1_SendData( d );
		WAIT_FOR_SEND_1();
	}
	UART_1_PutCRLF();
	UART_1_CmdReset();
	
	for(i=0; i < j; i++){
		UART_2_SendData( *(buff + i) );
		WAIT_FOR_SEND_2();
	}
	
	while (1) {
		UART_2_CPutString("\r");
		WAIT_FOR_SEND_2();
		for(i = 0; i < 200; i++) {
			Delay50uTimes(100); // delay 5ms
			if (UART_2_bCmdCheck()){
				recvd = UART_2_szGetParam();
				for ( c = 0, i = 0; c < RX_SIZE; c++ ) {
					d = *(recvd + c);
					if (d == ' ' || d == '\r' || d == '\n' || d == '\0') {
						send_buff[i] = '\0';
						break;
					}
					send_buff[i] = d;
					i++;
				}
				
				for(c = 0; c < RX_SIZE; c++){
					d = *(send_buff + c);
					if ( d == '\0' ) break;
					UART_1_SendData( d );
					WAIT_FOR_SEND_1();
				}
				UART_1_PutCRLF();
				UART_1_CmdReset();

				UART_2_CmdReset();
				return;
			}
		}
	}
}

void recv(void)
{
	char *recvd;
	if (UART_1_bCmdCheck()){
		recvd = UART_1_szGetParam();
		switch (*(recvd)) {
			case 'w':
				command_w();
				break;
			case 'v':
				command_v();
				break;
		}
		UART_1_CmdReset();
    }
}
int get_servo_val(unsigned char x, unsigned char center_offset, unsigned char width_extender)
{
	unsigned char center_offset_real = 2 * center_offset;
	unsigned char width_extender_real = 2 * width_extender;
	float max = 255.0;
	float ratio = x / max;
	return (2000 - width_extender_real) + (int)((2000 + (width_extender_real * 2)) * ratio) + center_offset_real;
}

void apply_car_accel(unsigned char ac)
{
	PWM16_1_WritePulseWidth(get_servo_val(ac, 20, 50));
}

void apply_car_direction(unsigned char dir)
{
	PWM16_2_WritePulseWidth(get_servo_val(dir, 20 + 86, 50));
}

void apply_cam_pan_h(unsigned char pan)
{
	PWM16_3_WritePulseWidth(get_servo_val(pan, 0, 0));
}

void apply_cam_pan_v(unsigned char pan)
{
	PWM16_4_WritePulseWidth(get_servo_val(pan, 0, 0));
}


void apply_battsw(unsigned char batt0, unsigned char batt1)
{
	if (batt0 == 0) OFF(0, BATTSW0); // off
	else            ON(0, BATTSW0);  // on
	if (batt1 == 0) OFF(0, BATTSW1); // off
	else            ON(0, BATTSW1);  // on
}

void latch(void)
{
	OFF(0, LATCH);
	ON(0, LATCH);
}

void shift1(void)
{
	OFF(0, SCK);
	ON(0, SCK);
}

void read1(void)
{
	if (PRT2DR & DATA) {
		serial_print("1", 1, 0);
	} else {
		serial_print("0", 1, 0);
	}
}

void send_batt_status(void)
{
	unsigned char i = 0;
	serial_print("b ", 2, 0);
	latch();
	for ( i = 0; i < 8; i++) {
		read1();
		shift1();
	}
	serial_print("", 0, 1);
}

void main(void)
{
	unsigned int i = 0;
	init();			//IO Initialize
	send_test();	//send test
	while (1) {
		if (i == 300) { i = 0; send_batt_status(); }
		recv();
		apply_car_accel(data[0]);
		apply_car_direction(data[1]);
		apply_cam_pan_h(data[2]);
		apply_cam_pan_v(data[3]);
		apply_battsw(data[4], data[5]);

		Delay50uTimes(20); // delay 1ms
		i++;
	}
}
