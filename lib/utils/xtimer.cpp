// #include "user_config.h"
// #include "mem.h"
// #include "ets_sys.h"
// #include "osapi.h"
// #include "gpio.h"
// #include "os_type.h"
// #include "ip_addr.h"
// #include "pwm.h"
// #include "espconn.h"
// #include "user_interface.h"


// /* ====================================== */
// /* HARDWARE TIMER                         */
// /* ====================================== */

// #define FRC1_ENABLE_TIMER  BIT7
// #define FRC1_AUTO_LOAD  BIT6

// //TIMER PREDIVED MODE
// typedef enum {
//     DIVDED_BY_1 = 0,	//timer clock
//     DIVDED_BY_16 = 4,	//divided by 16
//     DIVDED_BY_256 = 8,	//divided by 256
// } TIMER_PREDIVED_MODE;

// typedef enum {			//timer interrupt mode
//     TM_LEVEL_INT = 1,	// level interrupt
//     TM_EDGE_INT   = 0,	//edge interrupt
// } TIMER_INT_MODE;

// typedef enum {
//     FRC1_SOURCE = 0,
//     NMI_SOURCE = 1,
// } FRC1_TIMER_SOURCE_TYPE;


// static uint32 	intervalArr[70] = {0};
// static uint32	edgeIndex = 0;

// /* initialize to a very big number */
// static uint32	minInterval = 0xFFFFFFFF;

// /* this array will contain the raw IR message */
// static uint32 	rawIrMsg[100] = {0};
// static uint32 	rawIrMsgLen = 0;

// /* this variable will contain the decoded IR command */
// static uint32 	irCmd = 0;

// /* assumes timer clk of 5MHz */
// static uint32 usToTicks( uint32_t us )
// {
// 	return ( us * 10) >> 1;
// }

// /* assumes timer clk of 5MHz */
// static uint32 ticksToUs( uint32_t ticks )
// {
// 	return ( ticks << 1 ) / 10;
// }




// /* ====================================== */
// /* HARDWARE TIMER                         */
// /* ====================================== */

// void hwTimerCallback( void )
// {
// 	int i, j;
// 	int logicState = 1;
// 	int logicStateLen = 0;
// 	bool repeatCode = false;

// 	/* stop the HW TIMER */
// 	RTC_REG_WRITE(FRC1_CTRL_ADDRESS, DIVDED_BY_16 | TM_EDGE_INT);

// 	//Set GPIO0 to LOW
// 	gpio_output_set(0, BIT0, BIT0, 0);

// 	/* load the HW TIMER for next IR message frame */
// 	uint32 ticks = usToTicks(70000);
// 	RTC_REG_WRITE(FRC1_LOAD_ADDRESS, ticks);




// 	return;
// }

// void startTimer ( void )
// {
//     /* ====================================== */
// 	/* HARDWARE TIMER                         */
// 	/* ====================================== */

// 	/* The hardware timer is used to indicate when a complete IR message frame should have
// 	 * arrived in order to process the received data and calculate the IR command.
// 	 *
// 	 * It is configured in "one-shot" mode. It is started when the beginning of an
// 	 * IR message frame is detected and stopped after the complete message frame has been read.
// 	 * This means that the duration of the HW timer should be longer than the duration of
// 	 * the longest message frame. In the NEC IR tranmission protocol all message frames have
// 	 * a duration of approximately 67.5ms.
// 	 */

// 	/* load the HW TIMER */
// 	uint32 ticks = usToTicks(70000); // 70ms
// 	RTC_REG_WRITE(FRC1_LOAD_ADDRESS, ticks);

// 	/* register callback function */
// 	ETS_FRC_TIMER1_INTR_ATTACH( hwTimerCallback, NULL );

// 	/* enable interrupts */
// 	TM1_EDGE_INT_ENABLE();
// 	ETS_FRC1_INTR_ENABLE();

//     /* start the HW TIMER */
//     RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
//             DIVDED_BY_16 | FRC1_ENABLE_TIMER | TM_EDGE_INT);
// }