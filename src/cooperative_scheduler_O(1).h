/*
 * cooperative_scheduler_O(1).h
 *
 *  Created on: Nov 24, 2023
 *      Author: datph
 */
 #include <stdint.h>
 #ifndef INC_COOPERATIVE_SCHEDULER_O_1__H_
 #define INC_COOPERATIVE_SCHEDULER_O_1__H_
 
 
 #define SCH_MAX_TASKS	40
 #define	NO_TASK_ID 		0
 
 void SCH_Init(void);
 void SCH_Update(void);
 void SCH_Dispatch_Tasks(void);
 void SCH_Add_Task(void(*pFunction)(), uint32_t DELAY, uint32_t PERIOD);
 void SCH_Delete_Task(void);
 
 #endif /* INC_COOPERATIVE_SCHEDULER_O_1__H_ */