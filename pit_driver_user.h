#include "pit_driver.h"
#include "platform.h"
#include "atcpit100.h"

err_t PIT_Setup(uint32_t Channel_Select, const pit_channel_config_t* channel_config)
{
    if (Channel_Select >= ATCPIT100_CHANNEL_NUM)
    {
        return err_invalid_channel_select;
    }
    if (channel_config->ChMode >= ATCPIT100_CHMODE_NUM)
    {
        return err_invalid_channel_mode;
    }
    if (channel_config->ChClk >= ATCPIT100_CHCLK_NUM)
    {
        return err_invalid_channel_clock;
    }
    uint32_t CTRL_VALUE = 0;
    CTRL_VALUE |= (channel_config->ChMode & 0x7);       // ChMode[2:0]
    CTRL_VALUE |= (channel_config->ChClk & 0x1) << 3;   // ChClk
    DEV_PIT->CHANNEL[Channel_Select].CTRL = CTRL_VALUE;
    return err_success;
}

err_t PIT_ChEn(uint32_t Channel_Select, const pit_channel_config_t* channel_config)
{
    if (Channel_Select >= ATCPIT100_CHANNEL_NUM)
    {
        return err_invalid_channel_select;
    }
    if ((channel_config->Timer_select) >= ATCPIT100_TIMER_NUM)
    {
        return err_invalid_timer_select;
    }
    uint32_t enable_bit = (Channel_Select * 4) + channel_config->Timer_select;
    DEV_PIT->CHNEN |= (1 << enable_bit);  // ChEn
    return err_success;
}

err_t PIT_IntEn(uint32_t Channel_Select, const pit_channel_config_t* channel_config)
{
    if (Channel_Select >= ATCPIT100_CHANNEL_NUM)
    {
        return err_invalid_channel_select;
    }
    if ((channel_config->Timer_select) >= ATCPIT100_TIMER_NUM)
    {
        return err_invalid_timer_select;
    }
    uint32_t enable_bit = (Channel_Select * 4) + channel_config->Timer_select;
    DEV_PIT->INTEN |= (1 << enable_bit);  // IntEn
    return err_success;
}

err_t PIT_IntSt_Clear(uint32_t Channel_Select, const pit_channel_config_t* channel_config)
{
    if (Channel_Select >= ATCPIT100_CHANNEL_NUM)
    {
        return err_invalid_channel_select;
    }
    if ((channel_config->Timer_select) >= ATCPIT100_TIMER_NUM)
    {
        return err_invalid_timer_select;
    }
    uint32_t status_bit = (Channel_Select * 4) + channel_config->Timer_select;
    DEV_PIT->INTST = (1 << status_bit);  // write 1 in specified bit and turn it into 0
    return err_success;
}

err_t PIT_Reload(uint32_t Channel_Select, const pit_channel_config_t* channel_config)
{
    if (Channel_Select >= ATCPIT100_CHANNEL_NUM)
    {
        return err_invalid_channel_select;
    }

    uint32_t chMode = DEV_PIT->CHANNEL[Channel_Select].CTRL & 0x7; // read ChMode

    switch (chMode) {
    case ATCPIT100_MODE_TMR32: // 32-bit timer mode
        if ((channel_config->Reload_value) > 0xFFFFFFFF)
        {
            return err_invalid_reload_value;
        }
        if ((channel_config->Timer_select) == 0)
        {
            DEV_PIT->CHANNEL[Channel_Select].RELOAD = channel_config->Reload_value;
        }
        else
        {
            return err_invalid_timer_select;
        }
        break;

    case ATCPIT100_MODE_TMR16: // 16-bit timers mode
        if ((channel_config->Reload_value) > 0xFFFF)
        {
            return err_invalid_reload_value;
        }
        switch (channel_config->Timer_select) {
        case 0: DEV_PIT->CHANNEL[Channel_Select].RELOAD |= ((channel_config->Reload_value) << ATCPIT100_TMR16_0_OFFSET) & ATCPIT100_TMR16_0_MASK;
            break;
        case 1: DEV_PIT->CHANNEL[Channel_Select].RELOAD |= ((channel_config->Reload_value) << ATCPIT100_TMR16_1_OFFSET) & ATCPIT100_TMR16_1_MASK;
            break;
        default: return err_invalid_timer_select;
        }
        break;

    case ATCPIT100_MODE_TMR8: // 8-bit timers mode
        if ((channel_config->Reload_value) > 0xFF)
        {
            return err_invalid_reload_value;
        }
        switch (channel_config->Timer_select) {
        case 0: DEV_PIT->CHANNEL[Channel_Select].RELOAD |= ((channel_config->Reload_value) << ATCPIT100_TMR8_0_OFFSET) & ATCPIT100_TMR8_0_MASK;
            break;
        case 1: DEV_PIT->CHANNEL[Channel_Select].RELOAD |= ((channel_config->Reload_value) << ATCPIT100_TMR8_1_OFFSET) & ATCPIT100_TMR8_1_MASK;
            break;
        case 2: DEV_PIT->CHANNEL[Channel_Select].RELOAD |= ((channel_config->Reload_value) << ATCPIT100_TMR8_2_OFFSET) & ATCPIT100_TMR8_2_MASK;
            break;
        case 3: DEV_PIT->CHANNEL[Channel_Select].RELOAD |= ((channel_config->Reload_value) << ATCPIT100_TMR8_3_OFFSET) & ATCPIT100_TMR8_3_MASK;
            break;
        default: return err_invalid_timer_select;
        }
        break;

    default:
        return err_invalid_channel_mode;
    }
    return err_success;
}
err_t PIT_Reload_Restart(uint32_t Channel_Select)
{
    if (Channel_Select >= ATCPIT100_CHANNEL_NUM)
    {
        return err_invalid_channel_select;
    }
    DEV_PIT->CHANNEL[Channel_Select].RELOAD = 0;
    return err_success;
}
uint32_t PIT_IntSt_Read(uint32_t Channel_Select, const pit_channel_config_t* channel_config)
{
    if (Channel_Select >= ATCPIT100_CHANNEL_NUM)
    {
        return err_invalid_channel_select;
    }
    if ((channel_config->Timer_select) >= ATCPIT100_TIMER_NUM)
    {
        return err_invalid_timer_select;
    }
    uint32_t status_bit = (Channel_Select * 4) + channel_config->Timer_select;
    uint32_t status = (DEV_PIT->INTST) & (1 << status_bit);

    return status; // 0 or 1
}

void PIT_ISR_Entry()
{
    Callback_func();
}

void PIT_ISR_Callback_Func_Config(void* user_config_func) 
{
    (*intr_setup_ptr)(INT_NO_PIT, PIT_ISR_Entry);
    Callback_func = user_config_func;
}

void PIT_ISR_Enable() 
{
    (*intr_enable_ptr)(INT_NO_PIT);
}




