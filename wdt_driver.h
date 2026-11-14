#ifndef __WDT_DRIVER_H
#define __WDT_DRIVER_H
#include <inttypes.h>

//==========================================
// Magic Number Definition of Watchdog Timer
//==========================================

#define ATCWDT200_WP_NUM   0x5aa5  // magic number for write protection
#define ATCWDT200_RESTART_NUM	0xcafe // magic number for Restarting the Watchdog Timer

//==========================================
// Configuration Parameter of Watchdog Timer
//==========================================

typedef struct {
	uint32_t RstTime; // Time interval of the reset stage (from 0 to 7)
	uint32_t IntTime; // Time interval of the interrupt stage (usually from 0 to 7)
	uint32_t RstEn;   // Enable(1) or disable(0) the watchdog reset
	uint32_t IntEn;	  // Enable(1) or disable(0) the watchdog interrupt
	uint32_t ClkSel;  // Clock source of Timer (0:EXTCLK  1:PCLK)
	uint32_t En;	  // Enable(1) or disable(0) the watchdog timer
}wdt_config_t;

//====================================
// Function return value for checking
//====================================

typedef enum {
	err_success = 0x00,
	err_invalid_param = 0x01
}err_t;

//====================
// Main Functions
//====================

/**
 * @brief Setup and Enable your Watchdog Timer
 *
 * @note  If you want to initialize or control the WDT , use this function.
 *
 * @param RstTime  Time interval of the reset stage (from 0 to 7)
 * @param IntTime  Time interval of the interrupt stage (usually from 0 to 7)
 * @param RstEn    Enable(1) or disable(0) the watchdog reset
 * @param IntEn    Enable(1) or disable(0) the watchdog interrupt
 * @param ClkSel   Clock source of Timer (0:EXTCLK  1:PCLK)
 * @param En       Enable(1) or disable(0) the watchdog timer
 * 
 * @return 0x00: successfully configure the watchdog timer
 *		   0x01: you may input invalid param
 */ 
err_t WDT_EN(const wdt_config_t* config); 

/**
 * @brief Restart your Watchdog Timer
 *
 * @note  
 *
 * @param 
 *
 * @return 0x00: Successfully configure the watchdog timer
 *		   0x01: Theoretically you cannot get this result
 */
err_t WDT_Restart(); 

/**
 * @brief Disable your Watchdog Timer
 *
 * @note  You can achieve this function by WDT_EN, but WDT_Disable is easier.
 *
 * @param
 *
 * @return 0x00: Successfully disable the watchdog timer
 *		   0x01: Theoretically you cannot get this result
 */
err_t WDT_Disable();

#endif