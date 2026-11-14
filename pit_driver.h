#ifndef __PIT_DRIVER_H
#define __PIT_DRIVER_H
#include <inttypes.h>

//=======================================
// Macros to Judge if your input is valid
//=======================================
#define ATCPIT100_CHANNEL_NUM 4 // ATCPIT100 has 4 channels
#define ATCPIT100_CHMODE_NUM 8 
/**
* There are 6 different Channel Modes, this code focuses on the first 3 modes.
* ChMode = 1    32_bit_timer 
* ChMode = 2    16_bit_timer *2
* ChMode = 3     8_bit_timer *4
* ChMode = 4    PWM
* ChMode = 6    Mixed PWM/16_bit_timer
* ChMode = 7    Mixed PWM/8_bit_timer *2
*/
#define ATCPIT100_CHCLK_NUM 2  // 2 channel clock sources(0:EXTCLK  1:APBCLK) 
#define ATCPIT100_TIMER_NUM 4  // There are up to 4 Timers in a channel mode.

//========================================
// Macros used to Reload value for Timer
//========================================

//Reload Register for 16 - bit Timers mode.
#define ATCPIT100_TMR16_1_MASK (BIT_MASK(31, 16)) 
#define ATCPIT100_TMR16_1_OFFSET (16) 
#define ATCPIT100_TMR16_0_MASK (BIT_MASK(15, 0))
#define ATCPIT100_TMR16_0_OFFSET (0)

//Reload Register for 8-bit Timers mode.
#define ATCPIT100_TMR8_3_MASK (BIT_MASK(31, 24))
#define ATCPIT100_TMR8_3_OFFSET (24)
#define ATCPIT100_TMR8_2_MASK (BIT_MASK(23, 16))
#define ATCPIT100_TMR8_2_OFFSET (16)
#define ATCPIT100_TMR8_1_MASK (BIT_MASK(15, 8))
#define ATCPIT100_TMR8_1_OFFSET (8)
#define ATCPIT100_TMR8_0_MASK (BIT_MASK(7, 0))
#define ATCPIT100_TMR8_0_OFFSET (0)

//====================================
// Function return value for checking
//====================================
typedef enum {
	err_success = 0x00,
	err_invalid_channel_select = 0x01,
	err_invalid_channel_mode = 0x02,
	err_invalid_channel_clock = 0x03,
	err_invalid_timer_select = 0x04,
	err_invalid_reload_value = 0x05
}err_t;

//================================================
// Configuration Parameter of PIT for each Channel
//================================================

typedef struct {
	uint32_t ChMode;         // Channel Mode (1 2 3 4 6 7)
	uint32_t ChClk;	         // Channel Clock Sources(0:EXTCLK  1:APBCLK)
	uint32_t Timer_select;   // Some Channel Modes have multiple timers, in which case you should select Timer to use.
	uint32_t Reload_value;   // Period of Timer = Reload_value + 1
}pit_channel_config_t;

//====================
// Main Functions
//====================

/**
 * @brief Set up Channel Modes and Channel Clock Sources
 *
 * @note  Configure DEV_PIT->CTRL
 *
 * @param Channel_Select   Select your working channel
 * @param ChMode           Channel Mode (1 2 3 4 6 7)
 * @param ChClk            Channel Clock Sources(0:EXTCLK  1:APBCLK)
 *
 * @return 0x00: successfully Channel Modes and Channel Clock Sources
 *		   0x01: invalid channel
 *		   0x02: invalid channel mode
 *         0x03: invalid channel clock source
 */
err_t PIT_Setup(uint32_t Channel_Select, const pit_channel_config_t* channel_config);


/**
 * @brief Set up your Channel Enable Registers
 *
 * @note  Configure DEV_PIT->CHEN
 *
 * @param Channel_Select   Select your working channel
 * @param Timer_select     Each Channel index and Timer index jointly define 1 bit of Channel Enable Register.
 *						   For example: Ch3TMR2En -> ChEn[14]  ;  Ch2TMR0En -> ChEn[8]
 *
 * @return 0x00: successfully set up the Channel Enable Registers
 *		   0x01: invalid channel
 *		   0x04: invalid timer
 */
err_t PIT_ChEn(uint32_t Channel_Select, const pit_channel_config_t* channel_config);


/**
 * @brief Set up your Interrupt Enable Registers
 *
 * @note  Configure DEV_PIT->INTEN
 *
 * @param Channel_Select   Select your working channel
 * @param Timer_select     Each Channel index and Timer index jointly define 1 bit of Interrupt Enable Register.
 *						   For example: Ch3Int2En -> IntEn[14]  ;  Ch2Int0En -> IntEn[8]
 *
 * @return 0x00: successfully set up the Interrupt Enable Registers
 *		   0x01: invalid channel
 *		   0x04: invalid timer
 */
err_t PIT_IntEn(uint32_t Channel_Select, const pit_channel_config_t* channel_config);


/**
 * @brief Clear the interruption of PIT
 *
 * @note  Change the specific bit of the IntSt registers from 1 to 0. 
 *
 * @param Channel_Select   Select your working channel
 * @param Timer_select     Each Channel index and Timer index jointly define 1 bit of Interrupt Status Register.
 *						   For example: Ch3Int2 -> IntSt[14]  ;  Ch2Int0 -> IntSt[8]
 *
 * @return 0x00: successfully clear the interruption of PIT
 *		   0x01: invalid channel
 *		   0x04: invalid timer
 */
err_t PIT_IntSt_Clear(uint32_t Channel_Select, const pit_channel_config_t* channel_config);


/**
 * @brief Set up the Reload Registers
 *
 * @note  Configure DEV_PIT->CHANNEL[x].RELOAD
 *	      Use it after PIT_Setup().
 *
 * @param Channel_Select   Select your working channel
 * @param Timer_select     Different Chmode correspond to different timer selecting and different reload mode.
 *						   Check the Chmode and the Timer first, then write Reload_value by rule.
 * @param Reload_value     Be Cautious about the maximum value.
 * 
 * @return 0x00: successfully set up the Reload Registers
 *		   0x01: invalid channel
 *         0x02: invalid channel mode
 *		   0x04: invalid timer
 *         0x05: invalid Reload_value
 */
err_t PIT_Reload(uint32_t Channel_Select, const pit_channel_config_t* channel_config);


/**
 * @brief Restart the Reload Registers
 *
 * @note  If you want to reload different value in the same reload register of same channel,you should restart it first.
 *
 * @param Channel_Select   Select your working channel
 *
 * @return 0x00: successfully Restart the Reload Registers
 *		   0x01: invalid channel
 */
err_t PIT_Reload_Restart(uint32_t Channel_Select);


/**
 * @brief Read the Interrupt Status Registers
 *
 * @note  Usually use it before PIT_IntSt_Clear()
 *
 * @param Channel_Select   Select your working channel
 * @param Timer_select     Choose which timer in which channel to read
 *
 * @return status (1:The certain timer in this channel is interrupted   
				   0:The certain timer in this channel is not interrupted)
 */
uint32_t PIT_IntSt_Read(uint32_t Channel_Select, const pit_channel_config_t* channel_config);


/**
 * @brief Interrupt Service Routine (ISR) Entry
 *
 * @note  When an interruption is requested, the system transmits interrupt events to user through Callback Functions
 *
 * @param  
 * 
 * @return 
 */
static void (*Callback_func)(void);
void PIT_ISR_Entry();


/**
 * @brief Configure Callback Function and bind PIT hardware interrupt
 *
 * @note  Set the callback function as a user designed processing function.
 *
 * @param user_config_func()  // user designed processing function
 *
 * @return
 */
void PIT_ISR_Callback_Func_Config(void* user_config_func);


/**
 * @brief Enable PIT hardware interrupt
 *
 * @note  Make sure that Function PIT_IntEn() has properly processed.
 *
 * @param
 *
 * @return
 */
void PIT_ISR_Enable();

#endif