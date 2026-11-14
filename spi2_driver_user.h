#include "spi2_driver.h"
#include "platform.h"

#define TIME_OUT_VALUE 60000000

static inline ATCSPI200_RegDef* get_spi_device(spi_device dev)
{
	switch (dev) {
	case SPIIN1: return MKBHUB_SPIIN1;
	case SPIIN2: return MKBHUB_SPIIN2;
	case SPIIN3: return MKBHUB_SPIIN3;
	case SPIIN4: return MKBHUB_SPIIN4;
	case SPIIN5: return MKBHUB_SPIIN5;
	case SPIIN6: return MKBHUB_SPIIN6;
	case SPIIN7: return MKBHUB_SPIIN7;
	case SPIIN8: return MKBHUB_SPIIN8;
	case SPIOUT: return MKBHUB_SPIOUT;
	case SPI2:
	default:     return DEV_SPI2; 
	}
}


spi_error_t spi2_device_reset(spi_device dev)
{
	ATCSPI200_RegDef* spi_dev = get_spi_device(dev);
	spi_dev->CTRL = ATCSPI200_CTRL_SPIRST_MASK; // reset SPI
	uint32_t timeout = TIME_OUT_VALUE;
	while ((spi_dev->CTRL & ATCSPI200_CTRL_SPIRST_MASK) && (timeout--))
	{
		if (timeout == 0)
		{
			return SPI_ERROR_TIMEOUT;
		}
		__asm__ __volatile__("nop");
	}
	spi_dev->CTRL = ATCSPI200_CTRL_TXFIFORST_MASK | ATCSPI200_CTRL_RXFIFORST_MASK; // reset FIFO 
	timeout = TIME_OUT_VALUE;
	while ((spi_dev->CTRL & (ATCSPI200_CTRL_TXFIFORST_MASK | ATCSPI200_CTRL_RXFIFORST_MASK)) && (timeout--))
	{
		if (timeout == 0)
		{
			return SPI_ERROR_TIMEOUT;
		}
		__asm__ __volatile__("nop");
	}
	return SPI_SUCCESS;
}

spi_error_t spi2_transfer_control_set(spi_device dev, spi_transfer_format_config_t* spi_transfer_format, spi_control_config_t* spi_control, spi_frequency_enum_t spi_frequency)
{
	ATCSPI200_RegDef* spi_dev = get_spi_device(dev);
	if (spi_transfer_format->AddrLen > 3 ||
		spi_transfer_format->DataLen > 31 ||
		spi_transfer_format->DataMerge > 1 ||
		spi_transfer_format->MOSIBiDir > 1 ||
		spi_transfer_format->LSB > 1 ||
		spi_transfer_format->SlvMode > 1 ||
		spi_transfer_format->CPOL > 1 ||
		spi_transfer_format->CPHA > 1)
	{
		return SPI_ERROR_INVALID_PARAM;
	}
	spi_dev->TRANSFMT = (
		(spi_transfer_format->AddrLen << ATCSPI200_TRANSFMT_ADDRLEN_OFFSET) |
		(spi_transfer_format->DataLen << ATCSPI200_TRANSFMT_DATALEN_OFFSET) |
		(spi_transfer_format->DataMerge << ATCSPI200_TRANSFMT_DATAMERGE_OFFSET) |
		(spi_transfer_format->MOSIBiDir << ATCSPI200_TRANSFMT_MOSIBIDIR_OFFSET) |
		(spi_transfer_format->LSB << ATCSPI200_TRANSFMT_LSB_OFFSET) |
		(spi_transfer_format->SlvMode << ATCSPI200_TRANSFMT_SLVMODE_OFFSET) |
		(spi_transfer_format->CPOL << ATCSPI200_TRANSFMT_CPOL_OFFSET) |
		(spi_transfer_format->CPHA << ATCSPI200_TRANSFMT_CPHA_OFFSET)
		);
	spi_dev->CTRL = (
		(spi_control->TX_threshold << ATCSPI200_CTRL_TXTHRES_OFFSET) |
		(spi_control->RX_threshold << ATCSPI200_CTRL_RXTHRES_OFFSET) |
		(spi_control->TX_DMA_En << ATCSPI200_CTRL_TXDMAEN_OFFSET) |
		(spi_control->RX_DMA_En << ATCSPI200_CTRL_RXDMAEN_OFFSET) |
		(spi_control->TX_FIFO_Reset << ATCSPI200_CTRL_TXFIFORST_OFFSET) |
		(spi_control->RX_FIFO_Reset << ATCSPI200_CTRL_RXFIFORST_OFFSET) |
		(spi_control->SPI_Reset << ATCSPI200_CTRL_SPIRST_OFFSET)
		);
	spi_dev->TIMING = (
		(1 << ATCSPI200_TIMING_CS2SCLK_OFFSET) |
		(0 << ATCSPI200_TIMING_CSHT_OFFSET) |
		(spi_frequency & ATCSPI200_TIMING_SCLK_DIV_MASK)
		);
	return SPI_SUCCESS;
}

spi_error_t spi2_transfer(spi_device dev, spi_transfer_control_config_t* spi_transfer_config, spi_transaction_t* spi_transfer_desc, uint32_t timeout_ms)
{
	ATCSPI200_RegDef* spi_dev = get_spi_device(dev);
	spi_error_t ret = spi2_transfer_trigger(dev, spi_transfer_config, spi_transfer_desc);
	if (ret != SPI_SUCCESS)
	{
		return ret;
	}
	ret = spi2_transfer_wait(dev, timeout_ms);
	if (ret != SPI_SUCCESS)
	{
		return ret;
	}
	if (spi_transfer_config->TransMode == TRANSMODE_WRnRD ||
		spi_transfer_config->TransMode == TRANSMODE_RDonly)
	{
		for (uint32_t i = 0; i <= spi_transfer_config->RdTranCnt; i++)
		{
			uint32_t timeout = TIME_OUT_VALUE;
			while ((spi_dev->STATUS & ATCSPI200_STATUS_RXEMPTY_MASK) && timeout--)
			{
				if (!timeout)
				{
					return SPI_ERROR_TIMEOUT;
				}
				__asm__ __volatile__("nop"); // wait till RXFIFO is not empty
			}
			spi_transfer_desc->rx_buffer[i] = spi_dev->DATA;
		}
	}
	return SPI_SUCCESS;
}

spi_error_t spi2_transfer_trigger(spi_device dev, spi_transfer_control_config_t* spi_transfer_config, spi_transaction_t* spi_transfer_desc)
{
	ATCSPI200_RegDef* spi_dev = get_spi_device(dev);
	if (spi2_busy(dev))
	{
		return SPI_ERROR_BUSY;
	}
	if ((spi_transfer_config->TransMode == TRANSMODE_WRnRD ||
		spi_transfer_config->TransMode == TRANSMODE_WR_RD ||
		spi_transfer_config->TransMode == TRANSMODE_RD_WR) &&
		spi_transfer_config->WrTranCnt != spi_transfer_config->RdTranCnt) // Ensure WrTranCnt=RdTranCnt when used.
	{
		return SPI_ERROR_INVALID_PARAM;
	}
	spi_dev->TRANSCTRL = (
		(spi_transfer_config->SlvDataOnly << 31) |
		(spi_transfer_config->Cmd_En << ATCSPI200_TRANSCTRL_CMDEN_OFFSET) |
		(spi_transfer_config->Addr_En << ATCSPI200_TRANSCTRL_ADDREN_OFFSET) |
		(spi_transfer_config->Addr_Fmt << ATCSPI200_TRANSCTRL_ADDRFMT_OFFSET) |
		(spi_transfer_config->TransMode << ATCSPI200_TRANSCTRL_TRANSMODE_OFFSET) |
		(spi_transfer_config->Dual_Quad << ATCSPI200_TRANSCTRL_DUALQUAD_OFFSET) |
		(spi_transfer_config->Token_En << ATCSPI200_TRANSCTRL_TOKENEN_OFFSET) |
		(spi_transfer_config->WrTranCnt << ATCSPI200_TRANSCTRL_WRTRANCNT_OFFSET) |
		(spi_transfer_config->TokenValue << ATCSPI200_TRANSCTRL_TOKENVALUE_OFFSET) |
		(spi_transfer_config->DummyCnt << ATCSPI200_TRANSCTRL_DUMMYCNT_OFFSET) |
		(spi_transfer_config->RdTranCnt << ATCSPI200_TRANSCTRL_RDTRANCNT_OFFSET)
		);
	if (spi_transfer_config->Addr_En)
	{
		spi_dev->ADDR = spi_transfer_desc->addr;
	}

	if (spi_transfer_config->TransMode == TRANSMODE_WRonly ||
		spi_transfer_config->TransMode == TRANSMODE_WRnRD)
	{
		for (uint32_t i = 0; i <= spi_transfer_config->WrTranCnt; i++)
		{
			uint32_t timeout = TIME_OUT_VALUE;
			while ((spi_dev->STATUS & ATCSPI200_STATUS_TXFULL_MASK) && timeout--)
			{
				if (!timeout) {
					return SPI_ERROR_TIMEOUT;
				}
				__asm__ __volatile__("nop"); // wait till TXFIFO is not full
			}
			spi_dev->DATA = spi_transfer_desc->tx_buffer[i];
		}
	}
	if (spi_transfer_config->Cmd_En)
	{
		spi_dev->CMD = spi_transfer_desc->cmd;
	}
	return SPI_SUCCESS;
}

spi_error_t spi2_transfer_wait(spi_device dev, uint32_t timeout_ms)
{
	ATCSPI200_RegDef* spi_dev = get_spi_device(dev);
	uint32_t timeout = timeout_ms * 6000;
	while ((spi_dev->STATUS & ATCSPI200_STATUS_SPIACTIVE_MASK) && timeout--)
	{
		if (!timeout) {
			return SPI_ERROR_TIMEOUT;
		}
		__asm__ __volatile__("nop");
	}
	return SPI_SUCCESS;
}

spi_error_t spi2_interrupt_set_handler(spi_device dev, spi_isr_cb_t spi_isr)
{
	if (!spi_isr)
	{
		return SPI_ERROR_INVALID_PARAM;
	}
	switch (dev) {
	case SPI2: (*intr_setup_ptr)(INT_NO_SPI2, spi_isr); break;
	case SPIIN1: (*intr_setup_ptr)(INT_NO_SPIIN1, spi_isr); break;
	case SPIIN2: (*intr_setup_ptr)(INT_NO_SPIIN2, spi_isr); break;
	case SPIIN3: (*intr_setup_ptr)(INT_NO_SPIIN3, spi_isr); break;
	case SPIIN4: (*intr_setup_ptr)(INT_NO_SPIIN4, spi_isr); break;
	case SPIIN5: (*intr_setup_ptr)(INT_NO_SPIIN5, spi_isr); break;
	case SPIIN6: (*intr_setup_ptr)(INT_NO_SPIIN6, spi_isr); break;
	case SPIIN7: (*intr_setup_ptr)(INT_NO_SPIIN7, spi_isr); break;
	case SPIIN8: (*intr_setup_ptr)(INT_NO_SPIIN8, spi_isr); break;
	case SPIOUT: (*intr_setup_ptr)(INT_NO_SPIOUT, spi_isr); break;
	default: return SPI_ERROR_INVALID_PARAM;
	}
	return SPI_SUCCESS;
}

spi_error_t spi2_interrupt_enable(spi_device dev, spi_interrupt_config_t* interrupt)
{
	switch (dev){
	case SPI2: (*intr_enable_ptr)(INT_NO_SPI2); break;
	case SPIIN1: (*intr_enable_ptr)(INT_NO_SPIIN1); break;
	case SPIIN2: (*intr_enable_ptr)(INT_NO_SPIIN2); break;
	case SPIIN3: (*intr_enable_ptr)(INT_NO_SPIIN3); break;
	case SPIIN4: (*intr_enable_ptr)(INT_NO_SPIIN4); break;
	case SPIIN5: (*intr_enable_ptr)(INT_NO_SPIIN5); break;
	case SPIIN6: (*intr_enable_ptr)(INT_NO_SPIIN6); break;
	case SPIIN7: (*intr_enable_ptr)(INT_NO_SPIIN7); break;
	case SPIIN8: (*intr_enable_ptr)(INT_NO_SPIIN8); break;
	case SPIOUT: (*intr_enable_ptr)(INT_NO_SPIOUT); break;
	default: return SPI_ERROR_INVALID_PARAM;
	}
	ATCSPI200_RegDef* spi_dev = get_spi_device(dev);
	spi_dev->INTREN = (
		(interrupt->Slv_Cmd_IntEn << ATCSPI200_INTREN_SLVCMDEN_OFFSET) |
		(interrupt->End_IntEn << ATCSPI200_INTREN_ENDINTEN_OFFSET) |
		(interrupt->TX_FIFO_IntEn << ATCSPI200_INTREN_TXFIFOINTEN_OFFSET) |
		(interrupt->RX_FIFO_IntEn << ATCSPI200_INTREN_RXFIFOINTEN_OFFSET) |
		(interrupt->TX_FIFO_Underrun_IntEn << ATCSPI200_INTREN_TXFIFOURINTEN_OFFSET) |
		(interrupt->RX_FIFO_Overrun_IntEn << ATCSPI200_INTREN_RXFIFOORINTEN_OFFSET)
		);
	return SPI_SUCCESS;
}

spi_error_t spi2_interrupt_disable(spi_device dev)
{
	ATCSPI200_RegDef* spi_dev = get_spi_device(dev);
	(*intr_disable_ptr)(INT_NO_SPI2);
	spi_dev->INTREN = 0x00;
	return SPI_SUCCESS;
}

spi_error_t spi2_interrupt_clear(spi_device dev)
{
	ATCSPI200_RegDef* spi_dev = get_spi_device(dev);
	uint32_t intr_status = 0x00;
	intr_status = spi_dev->INTRST;
	spi_dev->INTRST = intr_status;
	(*intr_clear_ptr)(INT_NO_SPI2);
	return SPI_SUCCESS;
}

uint32_t spi2_status_read(spi_device dev)
{
	ATCSPI200_RegDef* spi_dev = get_spi_device(dev);
	return spi_dev->STATUS;
}

spi_error_t spi2_busy(spi_device dev)
{
	ATCSPI200_RegDef* spi_dev = get_spi_device(dev);
	return (spi_dev->STATUS & ATCSPI200_STATUS_SPIACTIVE_MASK) ? SPI_ERROR_BUSY : SPI_SUCCESS;
}
