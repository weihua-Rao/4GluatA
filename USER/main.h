
#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef 	Main_def
#define Extern_def  
#else
#define Extern_def extern
#endif

#include "n32g45x.h"
//#include "log.h"
//#include <absacc.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>  //bool布尔类型变量定义
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h" 
#include "event_groups.h"
#include "semphr.h"

#include	"n32g45x_it.h"
#include	"n32g45x_rcc.h"
#include	"n32g45x_gpio.h"
#include	"n32g45x_rtc.h"
//#include	"system_n32g43x.h"
#include	"usart.h"//printf
#include "syslib.h"
#include "sysport.h"

#include	"rs232.h"
#include	"EEPROM_read_wirte.h"
#include	"timer_n32.h"
#include	"CDMA.H"
#include	"MOBILE.H"
#include "PhoneCode.h"
#include "mac.h"
#include "misc.h"
#include "malloc.h"

#include "userapp.h"
#include "user_tcp.h"
//#include "logflash.h"
#include "flash.h"
#include "beep.h"
#include "dma.h"
#include "io.h"
#include "watchdog.h"
#include "W433.h"
#include "user_http.h"
#include "ec20net.h" 
#include "ec20module.h"
//#include "ec20http.h" //RunResult Send_Post( POSTP_s *psHttpP, char* postBody )
//#include "sysport.h"
//#include "cjson.h"
#include "usb.h"
#include "systick.h"	
#include "queue.h"
 #include "logflash.h" 
 #include "oled.h"
 #include "user_oled.h"
  #include "I2C.h"
#include "AHT20.h"
#include "rtc.h"
#include "User_RTC_Config.h"
#include "rs485.h"
#include "wifi.h"
#endif 
