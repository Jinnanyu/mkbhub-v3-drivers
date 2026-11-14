#include <interrupt.h>
#include "platform.h"
#include "ndslib.h"
#include <stdio.h>
#include "uwb_drivers.h"
#include "mkb03_ct_func.h"
#include "spi2_driver_user.h"

// FPGA verification only support 24M sysclk
#define USE_72M     1
#define SYS_CLK     ((USE_72M==1) ? 72 : 24)
// Every loop cost 3 system clock cycles
#define US_CYCLE    (SYS_CLK/3)     // 1us

#define MKBHUB_SPIIN1   ((ATCSPI200_RegDef *)(0x50000000))
#define MKBHUB_SPIOUT   ((ATCSPI200_RegDef *)(0x50800000))
#define dev_spi  SPIIN2


void mkbhub_gpio_test(void) {
    // IOMUX config: gpio
    *((uint32_t*)0xF0100240) = 0x0;
    // GPIO Output
    DEV_GPIO->CHANNELDIR = 0xFFFFFFFF;
}
void spi2_interrupt_handler(uint32_t intr_no) {
    spi2_interrupt_clear(dev_spi);
    DEV_GPIO->DATAOUT ^= (1 << 1);
}

void spiin_wait_tc(ATCSPI200_RegDef* SPI) {
    uint32_t spi_status_reg;
    delay_us(10);
    while(1) {
        spi_status_reg = spi2_status_read(dev_spi);
        if (((spi_status_reg & ATCSPI200_STATUS_TXEMPTY_MASK) >> ATCSPI200_STATUS_TXEMPTY_OFFSET) == 1){
            break;
        }
    }
}

// =======================================================
// AHB Bus Priority Config
// =======================================================
void ahb_priority_config(void) {
    // Set DMA(Master3) and IPU(Master6) priority = 1
    DEV_BMC->PR = 0x48;
}
// =======================================================
// AHB Bus Priority Config End
// =======================================================

void setup()
{
		uint32_t input_datalen = 32;
		uint32_t input_cnt = 4;
		uint32_t output_datalen = 32;
	    spi_interrupt_config_t interrupt_config = {
	        .Slv_Cmd_IntEn = 0,
	        .End_IntEn = 1,
	        .TX_FIFO_IntEn = 0,
	        .RX_FIFO_IntEn = 0,
	        .TX_FIFO_Underrun_IntEn = 0,
	        .RX_FIFO_Overrun_IntEn = 0
	    };
	    spi2_interrupt_enable(dev_spi ,&interrupt_config);

	    spi_transfer_format_config_t transformat = {
	        .AddrLen = 2,
	        .DataLen = output_datalen - 1,
	        .DataMerge = 0,
	        .MOSIBiDir = 0,
	        .LSB = 0,
	        .SlvMode = 0,
	        .CPOL = 0,
	        .CPHA = 0
	    };
	    spi_control_config_t control = {};
	    spi2_transfer_control_set(dev_spi ,&transformat , &control, SPI_FREQUENCY_2M);

	    uint32_t txbuffer[4] = {0};

	    for (int i=0;i<=3;i++)
	    {
	    	txbuffer[i] = i|0x12345678;
	    }

	    uint32_t* txbuffer_convert = (uint32_t*)malloc(input_datalen * input_cnt / output_datalen);
	    uint32_t* rxbuffer_convert = (uint32_t*)malloc(input_datalen * input_cnt / output_datalen);

	    for(int i=0;i<input_datalen * input_cnt / output_datalen;i++)
	    {
	    	uint32_t x = input_datalen / output_datalen;
	    	txbuffer_convert[i] = (txbuffer[i/x]>>(output_datalen*(x-1-(i%x))))& ((1 << output_datalen) - 1);
	    }


	    spi_transfer_control_config_t transcontrol = {
	    	            .Cmd_En = 1,
	    	            .Addr_En = 1,
	    	            .Addr_Fmt = 1,
	    	            .TransMode = 1,
	    	            .Dual_Quad = 0,
	    	            .DummyCnt = 0,
	    	            .WrTranCnt = input_datalen * input_cnt / output_datalen - 1,
	    	            .RdTranCnt = input_datalen * input_cnt / output_datalen - 1
	    	        };
	    spi_transaction_t transaction = {
	    		.addr = 0x2345,
				.cmd = 0x00,
				.tx_buffer = txbuffer_convert,
				.rx_buffer = rxbuffer_convert
	    };
	    for (uint8_t i = 0;i<10; i++) {
	       	spi_error_t ret = spi2_transfer(dev_spi ,&transcontrol, &transaction, 100);
	       	//spi2_interrupt_handler(INT_NO_SPIIN1);
	       	if (ret != SPI_SUCCESS)
	       	{
	       		while(1);
	       	}
	       	delay_ms(10);
	   }
}
// =======================================================
// Main Function
// =======================================================
int main(int argc, char **argv) {
	*((uint32_t*)0xF0100240) = 0x00111110;
	//mkbhub_gpio_test();
	spi2_interrupt_set_handler(dev_spi,spi2_interrupt_handler);
	setup();
    return 0;
}
// =======================================================
// Main Function End
// =======================================================
