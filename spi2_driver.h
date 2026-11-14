#ifndef __SPI_DRIVER_H
#define __SPI_DRIVER_H
#include <inttypes.h>
#include <interrupt.h>

#define spi_isr_cb_t nds_interrupt_handler_p 
static spi_isr_cb_t spi_tx_end_cb = 0;
static spi_isr_cb_t spi_tx_fifo_full_cb = 0;

typedef enum {
    SPIIN1,
    SPIIN2,
    SPIIN3,
    SPIIN4,
    SPIIN5,
    SPIIN6,
    SPIIN7,
    SPIIN8,
    SPIOUT,
    SPI2
}spi_device;

/**spi transfer format registers , refer to corresponding part in ATCSPI200 */
typedef struct {
    /**Address length in bytes
       0x0: 1 byte
       0x1: 2 bytes
       0x2: 3 bytes
       0x3: 4 bytes
       */
    uint32_t AddrLen;
    uint32_t DataLen; // data length in bits , actual length is data_len +1 bits
    uint32_t DataMerge;
    uint32_t MOSIBiDir;
    uint32_t LSB;
    /** Spi master/slave mode selection
        0x0: master
        0x1: slave
        */
    uint32_t SlvMode;
    uint32_t CPOL;
    uint32_t CPHA;

}spi_transfer_format_config_t;

#define spi_transmit_format_config_default {\
	.addr_len = 0 ,\
	.data_len = 7 ,\
	.data_merge = 0 ,\
	.mosi_bi_dir =0 ,\
	.lsb =0 ,\
	.slave_mode=0,\
	.cpol =0,\
	.cpha =0 }

/**spi transfer control register , refer to corresponding part in ATCSPI200 */
typedef struct {
    uint32_t SlvDataOnly; // only in slave mode , data only mode
    uint32_t Cmd_En;
    uint32_t Addr_En;
    /**
     *  0x0: single mode address
     *  0x1: dual/quad mode address
     */
    uint32_t Addr_Fmt;
    /**
     * spi transmit mode
     * 0x0: Write and read at the same time
     * 0x1: Write only
     * 0x2: Read only
     * 0x3: Write, Read
     * 0x4: Read, Write
     * 0x5: Write, Dummy, Read
     *  0x6: Read, Dummy, Write
     * 0x7: None Data (must enable CmdEn or AddrEn inmaster mode)
     * 0x8: Dummy, Write
     * 0x9: Dummy, Read
     * 0xa~0xf: Reserved
     */
    uint32_t TransMode;
    /**
     *  0x0: single(regular) mode
     *  0x1: dual I/O mode
     *  0x2: quad I/O mode
     */
    uint32_t Dual_Quad;
    uint32_t Token_En;// unused
    uint32_t WrTranCnt;// write counts , unit is defined in spi_transfer_format_config_t->DataLen in bits
    uint32_t TokenValue;// unused
    uint32_t DummyCnt;//dummy counts between address bits and data bits
    uint32_t RdTranCnt;// read counts , unit is defined in spi_transfer_format_config_t->DataLen in bits
}spi_transfer_control_config_t;

typedef struct {
    uint32_t TX_threshold;
    uint32_t RX_threshold;
    uint32_t TX_DMA_En;
    uint32_t RX_DMA_En;
    uint32_t TX_FIFO_Reset;
    uint32_t RX_FIFO_Reset;
    uint32_t SPI_Reset;

}spi_control_config_t;//spi transfer control register , refer to corresponding part in ATCSPI200


typedef struct {
    uint32_t Slv_Cmd_IntEn;
    uint32_t End_IntEn;
    uint32_t TX_FIFO_IntEn;
    uint32_t RX_FIFO_IntEn;
    uint32_t TX_FIFO_Underrun_IntEn;
    uint32_t RX_FIFO_Overrun_IntEn;

}spi_interrupt_config_t; // spi interrupt enable register , refer to corresponding part in ATCSPI200

typedef enum {
    SPI_FREQUENCY_24M = 0xFF,
    SPI_FREQUENCY_12M = 0,
    SPI_FREQUENCY_6M = 1,
    SPI_FREQUENCY_4M = 2,
    SPI_FREQUENCY_3M = 3,
    SPI_FREQUENCY_2M = 5,
    SPI_FREQUENCY_1M = 11,
    SPI_FREQUENCY_500K = 23
}spi_frequency_enum_t; // some pre-set clk frequencies , refer to spi interface timing register

typedef struct
{
    uint32_t addr;
    uint32_t cmd;
    uint32_t tx_len;
    uint32_t* tx_buffer;
    uint32_t rx_len;
    uint32_t* rx_buffer;
}spi_transaction_t; // Some parameters required during data transmission process

typedef enum {
    SPI_SUCCESS = 0x00,
    SPI_ERROR_TIMEOUT = 0x01,
    SPI_ERROR_BUSY = 0x02,
    SPI_ERROR_INVALID_PARAM = 0x03,
    SPI_ERROR_NOT_INITIALIZED = 0x04
} spi_error_t;  // possible return results


/**
 * @brief  Device reset , Set all registers to default (default parameters defined in spi datasheet),
 *         abandon all ongoing transactions .
 *
 * @note   Call this function to initalize device if needed
 *
 * @param  None
 *
 * @return 0x00 : SPI_SUCCESS
 *         0x01 : SPI_ERROR_TIMEOUT
 */
spi_error_t spi2_device_reset(spi_device dev);


/**
 * @brief  Spi device control
 *
 * @note
 *
 * @param  spi_transfer_format     Spi transfer format
 * @param  spi_control             Spi control register
 * @param  spi_frequency           Spi clk frequency
 *
 * @return 0x00 : SPI_SUCCESS
 *         0x03 : SPI_ERROR_INVALID_PARAM
 */
spi_error_t spi2_transfer_control_set(spi_device dev,spi_transfer_format_config_t* spi_transfer_format,
    spi_control_config_t* spi_control, spi_frequency_enum_t spi_frequency);


/**
 * @brief  Trigger a transaction , wait for it to complete , then read if needed, return result after transaction ends
 *
 * @note
 *
 * @param  spi_transfer_desc     params required during data transmission process
 * @param  spi_transfer_config   transfer device config
 * @param  timeout_ms 
 *
 * @return 0x00 : SPI_SUCCESS
 *         0x01 : SPI_ERROR_TIMEOUT
 *         0x02 : SPI_ERROR_BUSY
 *         0x03 : SPI_ERROR_INVALID_PARAM
 */
spi_error_t spi2_transfer(spi_device dev,spi_transfer_control_config_t* spi_transfer_config, spi_transaction_t* spi_transfer_desc, uint32_t timeout_ms);


/**
 * @brief  trigger a transaction , return immediately
 *
 * @note   Call spi_transmit_wait to wait for completion of this transaction ,do not trigger another transaction
 *         before checking last one's status
 *
 * @param  spi_transfer_desc     params required during data transmission process
 * @param  spi_transfer_config   transfer device config
 *
 * @return 0x00 : SPI_SUCCESS
 *         0x01 : SPI_ERROR_TIMEOUT
 *         0x02 : SPI_ERROR_BUSY
 *         0x03 : SPI_ERROR_INVALID_PARAM
 */
spi_error_t spi2_transfer_trigger(spi_device dev,spi_transfer_control_config_t* spi_transfer_config, spi_transaction_t* spi_transfer_desc);


/**
 * @brief  Wait till the ongoing transaction completes 
 *
 * @note
 *
 * @param  timeout_ms 
 *
 * @return 0x00 : SPI_SUCCESS
 *         0x01 : SPI_ERROR_TIMEOUT
 */
spi_error_t spi2_transfer_wait(spi_device dev,uint32_t timeout_ms);


/**
 * @brief  Set spi interrupt handler function for all spi device interrupts
 *
 * @note   The interrupt handler it should always call spi_interrupt_clear() defined
 *         below in order to clear the device interrupt status , otherwise
 *         the system would stuck in this handler function .
 *
 * @param  spi_isr   SPI interrupt handler function , with one uint32_t parameter intr_no
 *
 * @return 0x00 : SPI_SUCCESS
 *         0x04 : SPI_ERROR_NOT_INITIALIZED
 */
spi_error_t spi2_interrupt_set_handler(spi_device dev,spi_isr_cb_t spi_isr);


/**
 * @brief  Configure Spi Interrupt Enable Register
 *
 * @note
 *
 * @param  interrupt  params to control interrupt
 *
 * @return 0x00 : SPI_SUCCESS
 *         0x03 : SPI_ERROR_INVALID_PARAM
 *         0x04 : SPI_ERROR_NOT_INITIALIZED
 */
spi_error_t spi2_interrupt_enable(spi_device dev,spi_interrupt_config_t* interrupt);


/**
 * @brief  Disable all spi device interrrupts
 *
 * @note
 *
 * @param
 *
 * @return 0x00 : SPI_SUCCESS
 *         0x04 : SPI_ERROR_NOT_INITIALIZED
 */
spi_error_t spi2_interrupt_disable(spi_device dev);


/**
 * @brief  Clear Spi device interrupt status
 *
 * @note   Call this function when entering Spi interrupt handler
 *
 * @param  None
 *
 * @return 0x00 : SPI_SUCCESS
 */
spi_error_t spi2_interrupt_clear(spi_device dev);


/**
 * @brief  Read Spi Status Register to get transaction imformation
 *
 * @note   Function partically overlaps with spi_busy() 
 *
 * @param
 *
 * @return DEV_SPI2->STATUS 
 */
uint32_t spi2_status_read(spi_device dev);


/**
 * @brief  Check if Spi transfer is finished
 *
 * @note
 *
 * @param
 *
 * @return 0x00 : SPI_SUCCESS
 *         0x02 : SPI_ERROR_BUSY
 */
spi_error_t spi2_busy(spi_device dev);

#endif
