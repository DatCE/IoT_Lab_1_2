/*
 * cooperative_scheduler_O(1).c
 *
 *  Created on: Nov 24, 2023
 *      Author: datph
 */

 #include "cooperative_scheduler_O(1).h"

 typedef struct
 {
     void ( *pTask)(void);
     uint32_t Delay;
     uint32_t Period;
     uint8_t RunMe;
 } sTask;
 
 sTask SCH_tasks_G[SCH_MAX_TASKS];
 
 void SCH_Init()
 {
     uint8_t i;
     for (i = 0; i < SCH_MAX_TASKS; i++)
     {
         SCH_tasks_G[i].pTask = 0;
         SCH_tasks_G[i].Delay = 0;
         SCH_tasks_G[i].Period = 0;
         SCH_tasks_G[i].RunMe = 0;
     }
 }
 
 void SCH_Update(void)
 {
     if (SCH_tasks_G[0].pTask && SCH_tasks_G[0].RunMe == 0)
     {
         if (SCH_tasks_G[0].Delay == 0)
         {
             SCH_tasks_G[0].RunMe = 1;
         }
         else
         {
             SCH_tasks_G[0].Delay -= 1;
         }
     }
 }
 
 void SCH_Add_Task(void(*pFunction)(), uint32_t DELAY, uint32_t PERIOD)
 {
     uint8_t i = 0;
     uint32_t sumDelay = 0;
     uint32_t newDelay = 0;
     for (i = 0; i < SCH_MAX_TASKS; i++)
     {
         sumDelay += SCH_tasks_G[i].Delay;
         if (sumDelay > DELAY)
         {
             newDelay = DELAY - (sumDelay - SCH_tasks_G[i].Delay);
             SCH_tasks_G[i].Delay = sumDelay - DELAY;
             for (uint8_t j =  SCH_MAX_TASKS - 1; j > i; j--)
             {
                 if (SCH_tasks_G[j - 1].pTask)
                 SCH_tasks_G[j] = SCH_tasks_G[j - 1];
             }
             SCH_tasks_G[i].pTask = pFunction;
             SCH_tasks_G[i].Delay = newDelay;
             SCH_tasks_G[i].Period = PERIOD;
             if (SCH_tasks_G[i].Delay == 0) SCH_tasks_G[i].RunMe = 1;
             else SCH_tasks_G[i].RunMe = 0;
             break;
         }
         else
         {
             if (!SCH_tasks_G[i].pTask)
             {
                 SCH_tasks_G[i].pTask = pFunction;
                 SCH_tasks_G[i].Delay = DELAY - sumDelay;
                 SCH_tasks_G[i].Period = PERIOD;
                 if (SCH_tasks_G[i].Delay == 0) SCH_tasks_G[i].RunMe = 1;
                 else SCH_tasks_G[i].RunMe = 0;
                 break;
             }
         }
     }
 }
 
 
 void SCH_Delete_Task(void) // XÓA TASK Ở VỊ TRÍ ĐẦU, ĐƯỢC GỌI KHI NÓ TASK ĐÃ CHẠY THÀNH CÔNG
 {
     for (uint8_t j = 0; j < SCH_MAX_TASKS - 1; j++)
     {
         SCH_tasks_G[j] = SCH_tasks_G[j + 1];
     }
     SCH_tasks_G[SCH_MAX_TASKS - 1].pTask = 0;
     SCH_tasks_G[SCH_MAX_TASKS - 1].Delay = 0;
     SCH_tasks_G[SCH_MAX_TASKS - 1].Period = 0;
     SCH_tasks_G[SCH_MAX_TASKS - 1].RunMe = 0;
 }
 
 void SCH_Dispatch_Tasks(void)
 {
     if (SCH_tasks_G[0].RunMe > 0)
     {
         (*SCH_tasks_G[0].pTask)();
         SCH_tasks_G[0].RunMe = 0;
         sTask tempsTask = SCH_tasks_G[0];
         SCH_Delete_Task();
         if (tempsTask.Period != 0)
         {
             SCH_Add_Task(tempsTask.pTask, tempsTask.Period, tempsTask.Period);
         }
     }
 }