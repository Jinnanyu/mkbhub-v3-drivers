#include"wdt_driver.h"
#include"atcwdt200.h"
#include"platform.h"

err_t WDT_EN(const wdt_config_t* config)
{
	if (config->RstTime >= 8 || config->IntTime >= 8 || config->RstEn >= 2 || config->IntEn >= 2 || config->ClkSel >= 2 || config->En >= 2)
	{
		return err_invalid_param;
	}
	DEV_WDT->WEN = ATCWDT200_WP_NUM;

	uint32_t CTRL_VALUE = 0;
	CTRL_VALUE |= (config->RstTime << ATCWDT200_CTRL_RSTTIME_OFFSET) & ATCWDT200_CTRL_RSTTIME_MASK;
	CTRL_VALUE |= (config->IntTime << ATCWDT200_CTRL_INTTIME_OFFSET) & ATCWDT200_CTRL_INTTIME_MASK;
	CTRL_VALUE |= (config->RstEn << ATCWDT200_CTRL_RSTEN_OFFSET) & ATCWDT200_CTRL_RSTEN_MASK;
	CTRL_VALUE |= (config->IntEn << ATCWDT200_CTRL_INTEN_OFFSET) & ATCWDT200_CTRL_INTEN_MASK;
	CTRL_VALUE |= (config->ClkSel << ATCWDT200_CTRL_CLK_OFFSET) & ATCWDT200_CTRL_CLK_MASK;
	CTRL_VALUE |= (config->En << ATCWDT200_CTRL_EN_OFFSET) & ATCWDT200_CTRL_EN_MASK;

	DEV_WDT->CTRL = CTRL_VALUE;
	return err_success;
}

err_t WDT_Restart()
{
	DEV_WDT->WEN = ATCWDT200_WP_NUM;
	DEV_WDT->RESTART = ATCWDT200_RESTART_NUM;
	return err_success;
}

err_t WDT_Disable()
{
	DEV_WDT->WEN = ATCWDT200_WP_NUM;
	DEV_WDT->CTRL &= (~ATCWDT200_CTRL_EN_MASK);// only change En into 0
	return err_success;
}