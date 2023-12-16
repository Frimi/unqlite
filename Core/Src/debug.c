/***************************************************************************************************
* @file
* @brief     Debug related functions.
* @details   Tab == 4 spaces (use Tab char instead of spaces).
* @date      09/2023
***************************************************************************************************/

/***************************************************************************************************
* Includes
***************************************************************************************************/
#include "debug.h"
#include "stdarg.h"	// variadic arguments.
#include "stdio.h"	// vfprintf
#include "stm32u5xx_hal.h"
#include "SEGGER_RTT.h"
#include "main.h"
#include <stdbool.h>
/***************************************************************************************************
* Vars
***************************************************************************************************/
bool enabled = true;
static uint32_t oldSysTick;
static char workingBuffer[512];
extern RTC_HandleTypeDef hrtc;

/***************************************************************************************************
* @brief Enable the debug system (RTT output + Shell commands processing).
***************************************************************************************************/
void DEBUG_Enable(void)
{
	SEGGER_RTT_WriteString(0, "DEBUG SYSTEM ENABLED\n");
	enabled = true;
}
/***************************************************************************************************
* @brief Disable the debug system (RTT output + Shell commands processing).
***************************************************************************************************/
void DEBUG_Disable(void)
{
	SEGGER_RTT_WriteString(0, "DEBUG SYSTEM DISABLED\n");
	enabled = false;
}
/***************************************************************************************************
* @brief Returns the time difference in ms between the current and the previous saved Systick.
***************************************************************************************************/
uint32_t GetSysTickDiff(void)
{
	uint32_t tick = HAL_GetTick();
	uint32_t diff = tick - oldSysTick;
	oldSysTick = tick;
	return diff;
}
/***************************************************************************************************
* @brief Outputs the timestamp to the log output.
***************************************************************************************************/
void OutputTimestamp(void)
{
	RTC_DateTypeDef sdatestructureget;
	RTC_TimeTypeDef stimestructureget;
	char printBuffer[50];

	HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);

	sprintf(printBuffer, "%02d/%02d/%02d %02d:%02d:%02d", (2000 + sdatestructureget.Year) % 100, sdatestructureget.Month, sdatestructureget.Date, stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);

	snprintf(workingBuffer, sizeof(workingBuffer), "[%s]", printBuffer);
	SEGGER_RTT_WriteString(0, workingBuffer);
}
/***************************************************************************************************
* @brief Outputs the specified text to the debug output.
* @note  This function also prints a prefix made of the system tick and datetime and add the line ending.
***************************************************************************************************/
void DEBUG_Log(char * text)
{
	if (!enabled)
		return;

	OutputTimestamp();
	snprintf(workingBuffer, sizeof(workingBuffer), "[%06ld] %s\n", GetSysTickDiff(), text);
	SEGGER_RTT_WriteString(0, workingBuffer);
}
/***************************************************************************************************
* @brief Outputs the specified text to the debug output.
* @note  This function also prints a prefix made of the system tick and datetime without the line ending.
***************************************************************************************************/
void DEBUG_LogNoLF(char * text)
{
	if (!enabled)
		return;

	OutputTimestamp();
	snprintf(workingBuffer, sizeof(workingBuffer), "[%06ld] %s", GetSysTickDiff(), text);
	SEGGER_RTT_WriteString(0, workingBuffer);
}
/***************************************************************************************************
* @brief Outputs the specified text to the debug output without printing the prefixes and the line ending.
***************************************************************************************************/
void DEBUG_LogNoPreNoLF(char * text)
{
	if (!enabled)
		return;

	SEGGER_RTT_WriteString(0, text);
}
/***************************************************************************************************
* @brief Outputs the specified formatted text to the debug output.
* @note  This function also prints a prefix made of the system tick and datetime.
***************************************************************************************************/
void DEBUG_printf(const char* fmt, ...)
{
	if (!enabled)
		return;

	va_list args;

	OutputTimestamp();

	snprintf(workingBuffer, sizeof(workingBuffer), "[%06ld] ", GetSysTickDiff());
	SEGGER_RTT_WriteString(0, workingBuffer);

	va_start(args, fmt);
	vsnprintf(workingBuffer, sizeof(workingBuffer), fmt, args);
	va_end(args);
	SEGGER_RTT_WriteString(0, workingBuffer);

	SEGGER_RTT_WriteString(0, "\n");
}
/***************************************************************************************************
* @brief Outputs the specified formatted text to the debug output.
* @note  This function also prints a prefix made of the system tick and datetime.
***************************************************************************************************/
void DEBUG_printfNoLF(const char* fmt, ...)
{
	if (!enabled)
		return;

	va_list args;
	OutputTimestamp();

	snprintf(workingBuffer, sizeof(workingBuffer), "[%06ld] ", GetSysTickDiff());
	SEGGER_RTT_WriteString(0, workingBuffer);

	va_start(args, fmt);
	vsnprintf(workingBuffer, sizeof(workingBuffer), fmt, args);
	va_end(args);
	SEGGER_RTT_WriteString(0, workingBuffer);
}
/***************************************************************************************************
* @brief Outputs the specified formatted text to the debug output without printing the prefixes.
***************************************************************************************************/
void DEBUG_printfNoPreNoLF(const char* fmt, ...)
{
	if (!enabled)
		return;

	va_list args;

	va_start(args, fmt);
	vsnprintf(workingBuffer, sizeof(workingBuffer), fmt, args);
	va_end(args);
	SEGGER_RTT_WriteString(0, workingBuffer);
}
/***************************************************************************************************
* @brief Outputs the specified error text to the debug output.
***************************************************************************************************/
void ERROR_Log(char * text)
{
	if (!enabled)
		return;

	OutputTimestamp();

	snprintf(workingBuffer, sizeof(workingBuffer), "[%06ld] ***ERROR*** %s\n", GetSysTickDiff(), text);
	SEGGER_RTT_WriteString(0, workingBuffer);
}
/***************************************************************************************************
* @brief Outputs the specified formatted error text to the debug output.
***************************************************************************************************/
void ERROR_printf(const char* fmt, ...)
{
	if (!enabled)
		return;

	va_list args;
	OutputTimestamp();

	snprintf(workingBuffer, sizeof(workingBuffer), "[%06ld] ***ERROR*** ", GetSysTickDiff());
	SEGGER_RTT_WriteString(0, workingBuffer);

	va_start(args, fmt);
	vsnprintf(workingBuffer, sizeof(workingBuffer), fmt, args);
	va_end(args);
	SEGGER_RTT_WriteString(0, workingBuffer);

	SEGGER_RTT_WriteString(0, "\n");
}
/***************************************************************************************************
* @brief Outputs the specified formatted text to the debug output and do not call the logger module.
* @note  This function avoids recursive and stack overflows when we need to log FS access and
*        log inside the Logger module.
***************************************************************************************************/
void DEBUG_printfNoLogger(const char* fmt, ...)
{
	if (!enabled)
		return;

	va_list args;
	OutputTimestamp();

	snprintf(workingBuffer, sizeof(workingBuffer), "[%06ld] ", GetSysTickDiff());
	SEGGER_RTT_WriteString(0, workingBuffer);

	va_start(args, fmt);
	vsnprintf(workingBuffer, sizeof(workingBuffer), fmt, args);
	va_end(args);
	SEGGER_RTT_WriteString(0, workingBuffer);
}
