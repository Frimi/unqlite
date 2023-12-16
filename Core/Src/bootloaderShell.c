/***************************************************************************************************
 * @file
 * @brief     Shell RTT interface using micrium.
 * @details   Tab == 4 spaces (use Tab char instead of spaces).
 * @author    Vinicius Diori
 * @date      10/2023
 ***************************************************************************************************/

/***************************************************************************************************
 * Includes
 ***************************************************************************************************/
// System
#include "stm32u575xx.h"
#include "main.h"
#include <string.h>

// Software
#include "debug.h"
#include "shell.h"
#include "bootloaderShell.h"

// Library
#include "fs_shell.h"
#include "SEGGER_RTT.h"
#include "app_cfg.h"
/***************************************************************************************************
 * Externs
 ***************************************************************************************************/
extern SHELL_CMD MainShellCmdTbl[];
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
/***************************************************************************************************
 * Vars
 ***************************************************************************************************/
static CPU_CHAR currentCmd[5000] = {0};

/***************************************************************************************************
 * @brief Macro for printing information about initialazed micrium Shell.
***************************************************************************************************/
#define INIT_CMD_TABLE(STRINGID, CMDTABLE)                          \
    {                                                               \
        DEBUG_LogNoLF("Initialize " STRINGID " command table... "); \
        Shell_CmdTblAdd(STRINGID, CMDTABLE, &err);                  \
        if (err == SHELL_ERR_NONE)                                  \
        {                                                           \
            DEBUG_LogNoPreNoLF("done\n");                           \
        }                                                           \
        else                                                        \
        {                                                           \
            DEBUG_LogNoPreNoLF("failed\n");                         \
        }                                                           \
    }

/***************************************************************************************************
 * @brief Initializes micrium shell and the command table.
***************************************************************************************************/
void bootloaderShell_Init(void)
{
    CPU_BOOLEAN success;
    SHELL_ERR err;

    APP_TRACE(("Initialize Shell ... "));

    success = Shell_Init();

    if (success == DEF_OK)
    {
        DEBUG_LogNoPreNoLF("done\n");
    }
    else
    {
        DEBUG_LogNoPreNoLF("failed\n");
        ERROR_Log("Error initializing Micrium Shell. Shell is disabled.");
        return;
    }

    APP_TRACE("Initialize FSShell command table... ");

    success = FSShell_Init();

    if (success == DEF_OK)
    {
        DEBUG_LogNoPreNoLF("done\n");
    }
    else
    {
        DEBUG_LogNoPreNoLF("failed\n");
        return;
    }

    /* The number of command tables cannot exceed SHELL_CFG_CMD_TBL_SIZE.
       Command table name cannot exceed SHELL_CFG_MODULE_CMD_NAME_LEN_MAX. */
    INIT_CMD_TABLE("shell", MainShellCmdTbl);
}

/***************************************************************************************************
 * @brief App shell output function.
***************************************************************************************************/
static CPU_INT16S bootloaderShell_OutputFunc(CPU_CHAR *pbuf, CPU_INT16U buf_len, void *popt)
{
    DEBUG_LogNoPreNoLF(pbuf);
    return (buf_len);
}

/***************************************************************************************************
 * @brief Receives complete messages and execute shell commands.
***************************************************************************************************/
static void bootloaderShell_ReceiveMSG(void)
{
    SHELL_ERR err;

    Shell_Exec(currentCmd, &bootloaderShell_OutputFunc, NULL, &err);
    switch (err)
    {
    case SHELL_ERR_NONE:
        break;

    case SHELL_ERR_NULL_PTR:
        ERROR_Log("Error: NULL pointer");
        break;

    case SHELL_ERR_CMD_NOT_FOUND:
        ERROR_Log("Command NOT found");
        break;

    case SHELL_ERR_CMD_SEARCH:
        ERROR_Log("Error searching command");
        break;

    case SHELL_ERR_ARG_TBL_FULL:
        ERROR_Log("Error: too many arguments");
        break;

    case SHELL_ERR_CMD_EXEC:
        ERROR_Log("SHELL_ERR_CMD_EXEC");
        break;

    default:
        ERROR_Log("Unknown error");
        break;
    }
    memset(currentCmd, 0, sizeof(CPU_CHAR));
}

/***************************************************************************************************
* @brief Process characters that arrived via RTT interface.
***************************************************************************************************/
static void bootloaderShell_ProcessRTT()
{
    uint32_t dataAvailable = SEGGER_RTT_Available(0);
    if (dataAvailable == 0)
        return;

    uint32_t read = SEGGER_RTT_Read(0, currentCmd, dataAvailable);
    if (read != dataAvailable)
    {
        ERROR_Log("Read != dataAvailable");
    }

    CPU_CHAR *pMsg = currentCmd;
    for (uint32_t c = 0; c < read; c++, pMsg++)
    {
        if (*pMsg == '\r' || *pMsg == '\n')
        {
            /* Remove these chars from the string because the Shell processing does not accept them. */
            Str_Char_Replace_N(currentCmd, '\r', '\0', read);
            Str_Char_Replace_N(currentCmd, '\n', '\0', read);
            bootloaderShell_ReceiveMSG();
            return;
        }
    }

    /* If this point was reached, it means that the message was not sent with line endings
       Probably the RTT viewer is set up to send on keypress and not on enter.
       Free the malloc to avoid memory leak since the data will not be processed. */
    ERROR_Log("Did not received a full message. Change RTT to send all on enter!");
    memset(currentCmd, 0, sizeof(CPU_CHAR));
}

/***************************************************************************************************
 * @brief Callback for Hardware Timer interrupt.
 *
 * @param htim - timerHandler.
***************************************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim1)
    {
        bootloaderShell_ProcessRTT();
    }
    else if (htim == &htim2)
    {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
    }
}
