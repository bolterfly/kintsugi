
// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "nordic_common.h"
#include "nrf_drv_clock.h"
#include "sdk_errors.h"


#include "nrf_log.h" 
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// Example
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "bsp.h"
#include "semphr.h"
#include "message_buffer.h"
#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "nrf_soc.h"

#include "hp_freertos_mpu.h"
#include "hp_manager.h"
#include "hp_config.h"

#define MAX_TEST_DATA_BYTES     (15U)                /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE 2048                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 2048                         /**< UART RX buffer size. */
#define UART_HWFC APP_UART_FLOW_CONTROL_DISABLED

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
    else if (p_event->evt_type == APP_UART_TX_EMPTY) {
        UART_PRINT_FLAG = 0;
        
    }
}

static UBaseType_t ulNextRand;
UBaseType_t uxRand( void )
{
    const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

	/* Utility function to generate a pseudo random number. */

	ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
	return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
}

static void prvSRand( UBaseType_t ulSeed )
{
	/* Utility function to seed the pseudo random number generator. */
    ulNextRand = ulSeed;
}

void
__attribute__((optimize("O0")))
vulnerable_task(void* param) { 
    /*
     * This task tries to modify a hotpatch that has been stored in memory
     */
    volatile uint32_t data;
    while (true) {
        // read from the hotpatch code until it contains some data
        data = *(uint32_t *)((uint32_t)((uint32_t)&__hotpatch_code_start + 8));
        if (data != 0) {
            uint32_t control;
            DEBUG_LOG("Read (0x%08X): %08X\r\n", (uint32_t)((uint32_t)&__hotpatch_code_start + 8), data);
            *(uint32_t *)((uint32_t)((uint32_t)&__hotpatch_code_start + 8)) = 0xAABBCCDD; // write a dummy value to the hotpatch code

            if (*(uint32_t *)((uint32_t)((uint32_t)&__hotpatch_code_start + 8)) == 0xAABBCCDD) {
                DEBUG_LOG("Malicious Attack Success!\r\n");
            } else {
                DEBUG_LOG("Malicious Attack Failed!\r\n");
            }
        }

        vTaskDelay(2000);
    }
}

uint32_t check_result(uint8_t *expected, uint32_t expected_len, uint8_t *digest, uint32_t digest_len) {
    if (expected_len != digest_len) {
        return 0;
    }

    for(uint32_t i = 0; i < expected_len; i++) {
        if (expected[i] != digest[i]) {
            return 0;
        }
    }

    return 1;
}

uint32_t
__ramfunc
__attribute__((noinline, used, optimize("O0"), aligned(8)))
target_ram_function() {
    volatile uint32_t result;

    result = 0;
	// 1
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
	// 2
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
	// 3
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
	// 4
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
	// 5
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
	// 6
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
	// 7
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
	// 8
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
	// 9
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
	// 10
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
                    	// 10
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
                    	// 10
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
                    	// 10
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
                    	// 10
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
                    	// 10
    __asm volatile( "nop\n"
                    "nop\n"
                    "nop\n"
                    "nop\n");
    return result;
}


void workload_task(void *param) {
    while (true) {
        for (uint32_t i = 0; i < 100000; i++) {
            __asm volatile ("nop\n");
        }
        vTaskDelay(1000);
    }
}

#define HP_SIZE (sizeof(struct hp_header) + HP_MAX_CODE_SIZE)
volatile uint8_t hotpatch_code[1 * HP_SIZE] = { 0 };

void
task_hp_manager(void *parameters) {

    uint32_t identifier = 0;
    DEBUG_LOG("Starting Hotpatch Task\r\n");


    hp_manager(hotpatch_code, sizeof(hotpatch_code), &identifier);

    DEBUG_LOG("Done\r\n");
    while (1) {
        vTaskDelay(10);
    }
    vTaskDelete(NULL);
}



#define WORK_TASKS  1
TaskHandle_t malicious_task_handle;
TaskHandle_t workload_task_handle[WORK_TASKS];
TaskHandle_t hp_manager_task_handle;

int main(void)
{
    hp_mpu_init();

    hp_manager_init();

    uint32_t alignment =  ((uint32_t)(&target_ram_function) & 0b10) << 16;
    *(uint32_t *)((uint32_t)&hotpatch_code + sizeof(struct hp_header) + 0) = 0xF000F8DF | alignment;
    *(uint32_t *)((uint32_t)&hotpatch_code + sizeof(struct hp_header) + 4) = (uint32_t)(&target_ram_function + 16) | 1;

    ret_code_t err_code;
    
    /* Initialize clock driver for better time accuracy in FREERTOS */
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
    
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          UART_HWFC,
          false,
#if defined (UART_PRESENT)
          NRF_UART_BAUDRATE_115200
#else
          NRF_UARTE_BAUDRATE_115200
#endif
      };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOWEST,
                         err_code);

    APP_ERROR_CHECK(err_code);

    
    volatile struct hp_header *hotpatch_header;
    hotpatch_header = (struct hp_header*)((uint32_t)(&hotpatch_code));
    hotpatch_header->type = HP_TYPE_REDIRECT;
    hotpatch_header->target_address = (uint32_t)&target_ram_function;
    hotpatch_header->code_size = 8;
    hotpatch_header->code_ptr = 0;//&hotpatch_code_bytes;

    printf("Task Malicious: %d\r\n", xTaskCreate(vulnerable_task, "MALICIOUS", configMINIMAL_STACK_SIZE, NULL, 5, &malicious_task_handle));
    printf("Task Manager: %d\r\n", xTaskCreate(task_hp_manager, configHP_TASK_NAME, configMINIMAL_STACK_SIZE, NULL, 1, &hp_manager_task_handle));

    for (uint32_t i = 0; i < WORK_TASKS; i++) {
        printf("Task Workload %d: %d\r\n", i, xTaskCreate(workload_task, "WORK", configMINIMAL_STACK_SIZE, NULL, 5, &workload_task_handle[i]));
    }


    vTaskStartScheduler();
    
    //prvCheckOptionsWrapper(0, 0);
    target_ram_function();
    while(1) {};
}
