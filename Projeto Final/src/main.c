#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "system_TM4C1294.h"    // CMSIS-Core
#include "cmsis_os2.h"          // CMSIS-RTOS
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"

#include "src/Uart/uart.h"
#include "src/SysTick/sysTick.h"

osThreadId_t elevadorE_id, elevadorC_id, elevadorD_id;                                        
osThreadId_t uartRead_id, uartWrite_id;

osTimerId_t timerE_id,timerC_id,timerD_id;

osMessageQueueId_t queueE_id,queueC_id,queueD_id,queueUart_id;

void callbackE(void *arg){
     osThreadFlagsSet(elevadorE_id, 0x0001);
}

void callbackC(void *arg){
     osThreadFlagsSet(elevadorC_id, 0x0002);

}

void callbackD(void *arg){
     osThreadFlagsSet(elevadorD_id, 0x0003);

}

void elevadorE (void *argument) {

  static uint8_t curr_floor=0;
  static uint8_t next_floor=-1;
  static uint8_t dest_vect[16];
  
  static uint8_t i;
  static uint8_t ext_btn_floor = 0;
  static uint8_t signal_feedback=0;  
  static uint8_t vect_zero=0;  
  volatile bool floor_changed=false;  
  
  
  char cmd[2];
  char msg[10];    
  char floor_letters[]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};

  //RESET
  cmd[0]='e';   //Qual elevador
  cmd[1]='r';   //resetar
  osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
  osDelay(1000);

  //FECHAR
  cmd[0]='e';   //Qual elevador
  cmd[1]='f';   //fechar
  osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);

  for (;;) {
    osMessageQueueGet(queueE_id, msg ,NULL, osWaitForever);
    //BOTOES EXTERNOS 
    if(msg[1]=='E'){//botao externo
      sscanf(msg,"%*c%*c%d%*c", &ext_btn_floor);  //Auxiliar que recebe o valor do andar do botão externo pressionado 
      dest_vect[ext_btn_floor]=1;               //Atualiza o vetor de destino do elevador     
      
      //Caso seja a primeira vez
      if((next_floor == -1) ||((vect_zero == 0) && (floor_changed))){
        floor_changed = false;
        next_floor = ext_btn_floor;
      }
      
       if(curr_floor < next_floor){
         cmd[0]='e';    //Qual elevador
         cmd[1]='s';    //subir
         osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }     
      else if(curr_floor > next_floor){
             cmd[0]='e';    //Qual elevador
             cmd[1]='d';    //descer
             osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }

   }//if do botão externo    
   else if(msg[1]=='I'){ //botao interno     
      for(i=0;i<16;i++){
        if(floor_letters[i]==msg[2]){
          dest_vect[i]=1;
        }
      }
      
      for(i=0;i<16;i++){
          if(dest_vect[i]==1){
             next_floor = i;
          }      
      }
      
      if(curr_floor < next_floor){
         cmd[0]='e'; //Qual elevador
         cmd[1]='s';//subir
         osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }     
      else if(curr_floor > next_floor){
             cmd[0]='e'; //Qual elevador
             cmd[1]='d';//descer
             osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }
      
    }//if do botão interno
    
    else if((msg[1]>='0' && msg[0]=='c' && msg[1]<='9') || (msg[2]>='0' && msg[1]<='2'))
    {
      floor_changed = true;
      
         sscanf(msg,"%*c%d", &signal_feedback);  
               curr_floor = signal_feedback;
          if(curr_floor == next_floor){
           
           dest_vect[curr_floor]=0; 
           cmd[0]='e'; //Qual elevador
           cmd[1]='p';//parar
           osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
           
           cmd[0]='e'; //Qual elevador
           cmd[1]='a';//abrir porta
           osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
           osTimerStart(timere_id, 3000);
           osThreadFlagsWait(0x0002, osFlagsWaitAny, osWaitForever);

          for(i=0;i<16;i++){
              if(dest_vect[i]==1){
                 next_floor = i;
                 vect_zero = 1;
                 break;
                }else
                  vect_zero =0;
          }
         }
    }//Fim else do feedback    
    else{
        if(msg[1]=='A'){
           cmd[0]='e'; //Qual elevador
           cmd[1]='f';//fechar porta
           osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
        }        
    }
    
    if(vect_zero == 1){
      
      if(curr_floor < next_floor){
         cmd[0]='e'; //Qual elevador
         cmd[1]='s';//subir
         osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }     
      else if(curr_floor > next_floor){
             cmd[0]='e'; //Qual elevador
             cmd[1]='d';//descer
             osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }      
      
      
    }
  }
}

void elevadorC_id (void *argument) {
  static uint8_t curr_floor=0;
  static uint8_t next_floor=-1;
  static uint8_t dest_vect[16];
  
  static uint8_t i;
  static uint8_t ext_btn_floor = 0;
  static uint8_t signal_feedback=0;  
  static uint8_t vect_zero=0;  
  volatile bool  floor_changed=false;  
  
  
  char cmd[2];
  char msg[10];    
  char floor_letters[]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
 
  //ENVIA RESET
  cmd[0]='c'; //Qual elevador
  cmd[1]='r';//resetar
  osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
  osDelay(1000);

  //FECHAR AS PORTAS
  cmd[0]='c';
  cmd[1]='f';//fechar
  osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
    
  for (;;) {
    osMessageQueueGet(queueC_id, msg ,NULL, osWaitForever);
    //TRATAMENTO DOS BOTÕES EXTERNOS 
    if(msg[1]=='E'){//botao externo
      sscanf(msg,"%*c%*c%d%*c", &ext_btn_floor);  //Auxiliar que recebe o valor do andar do botão externo pressionado 
      dest_vect[ext_btn_floor]=1;                  //Atualiza o vetor de destino do elevador     
      
      //Caso seja a primeira vez
      if((next_floor == -1) ||((vect_zero == 0) && (floor_changed))){  //se for a primeira vez (next_floor = -1), se o vetor de destino tiver somente zeros, ou seja, só foi pedido um andar (vect_zero), se ja mudou de andar (floor_changed)
        floor_changed = false;
        next_floor = ext_btn_floor;
      }
      
      //manda subir caso curr_floor for menor que o próximo andar
       if(curr_floor < next_floor){
         cmd[0]='c'; //Qual elevador
         cmd[1]='s';//subir
         osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }
      //manda descer caso curr_floor for maior que o próximo andar
      else if(curr_floor > next_floor){
             cmd[0]='c'; //Qual elevador
             cmd[1]='d';//descer
             osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }
   }//if do botão externo    
   else if(msg[1]=='I'){ //botao interno     
      //Tem um vetor de letras, pois para os botões internos, ao invés de ter números indicando os andares são letras, então aqui ja converte pra números e ja salva no vetor destino, só existe um vetor de destino tanto para subir quanto para descer
      for(i=0;i<16;i++){
        if(floor_letters[i]==msg[2]){
          dest_vect[i]=1;
        }
      }
      
      //Atualiza o próximo andar
      for(i=0;i<16;i++){
          if(dest_vect[i]==1){
             next_floor = i;
          }      
      }
      
      //manda subir caso curr_floor for menor que o próximo andar
      if(curr_floor < next_floor){
         cmd[0]='c'; //Qual elevador
         cmd[1]='s';//subir
         osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }
      //manda descer caso curr_floor for maior que o próximo andar
      else if(curr_floor > next_floor){
             cmd[0]='c'; //Qual elevador
             cmd[1]='d';//descer
             osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }
    }//if do botão interno
    
    else if((msg[1]>='0' && msg[0]=='c' && msg[1]<='9') || (msg[2]>='0' && msg[1]<='2'))
    {              
      floor_changed = true; //Flag que vai indicar que teve mudança de andar
      
         sscanf(msg,"%*c%d", &signal_feedback);  
               curr_floor = signal_feedback;
          if(curr_floor == next_floor){
           
           dest_vect[curr_floor]=0; //Zera a posição que ja foi atendida no vetor de destino
           
           cmd[0]='c'; //Qual elevador
           cmd[1]='p';//parar
           osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
           
           cmd[0]='c'; //Qual elevador
           cmd[1]='a';//abrir porta
           osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
           osTimerStart(timerc_id, 3000); //Da start no timer do sistema para que a porta fique aberta 3 segundos
           osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever); //Bloqueia a Thread esperando uma flag de sinal, só vai receber a flag quando for chamada a função de callback

          for(i=0;i<16;i++){
              if(dest_vect[i]==1){
                 next_floor = i;
                 vect_zero = 1;
                 break;
                }else
                  vect_zero =0;
          }     
         }  
    }//Fim else do feedback    
    else{// ultimo else são os feeback de porta aberta e porta fechada
        if(msg[1]=='A'){
           cmd[0]='c'; //Qual elevador
           cmd[1]='f';//fechar porta
           osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
        }        
    }
    
    //Aqui lógica para comandar a subida e descida do elevador, mesmo se os botões não forem mais apertados, porém o vetor de destino ainda tem elementos
    if(vect_zero == 1){
      
      if(curr_floor < next_floor){
         cmd[0]='c'; //Qual elevador
         cmd[1]='s';//subir
         osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }     
      else if(curr_floor > next_floor){
             cmd[0]='c'; //Qual elevador
             cmd[1]='d';//descer
             osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }      
      
      
    }
  }
}

void elevadorD_id (void *argument) {
    
  static uint8_t curr_floor=0;
  static uint8_t next_floor=-1;
  static uint8_t dest_vect[16];
  
  static uint8_t i;
  static uint8_t ext_btn_floor = 0;
  static uint8_t signal_feedback=0;  
  static uint8_t vect_zero=0;  
  volatile bool floor_changed=false;  
  
  
  char cmd[2];
  char msg[10];    
  char floor_letters[]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
  
  //ENVIA RESET
  cmd[0]='d'; //Qual elevador
  cmd[1]='r';//resetar
  osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
  osDelay(1000);

  //FECHAR AS PORTAS
  cmd[0]='d';
  cmd[1]='f';//fechar
  osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
 

         
  for (;;) {
    osMessageQueueGet(queueD_id, msg ,NULL, osWaitForever);
    //TRATAMENTO DOS BOTÕES EXTERNOS 
    if(msg[1]=='E'){//botao externo
      sscanf(msg,"%*c%*c%d%*c", &ext_btn_floor);  //Auxiliar que recebe o valor do andar do botão externo pressionado 
      dest_vect[ext_btn_floor]=1;               //Atualiza o vetor de destino do elevador     
      
      //Caso seja a primeira vez
      if((next_floor == -1) ||((vect_zero == 0) && (floor_changed))){
        floor_changed = false;
        next_floor = ext_btn_floor;
      }          
        if(curr_floor < next_floor){
         cmd[0]='d'; //Qual elevador
         cmd[1]='s';//subir
         osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }
      else if(curr_floor > next_floor){
         cmd[0]='d'; //Qual elevador
         cmd[1]='d';//descer
         osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }
   }//if do botão externo    
   else if(msg[1]=='I'){ //botao interno     
      for(i=0;i<16;i++){
        if(floor_letters[i]==msg[2]){
          dest_vect[i]=1;
        }
      }
      
      for(i=0;i<16;i++){
          if(dest_vect[i]==1){
             next_floor = i;
          }      
      }
      
      if(curr_floor < next_floor){
         cmd[0]='d'; //Qual elevador
         cmd[1]='s';//subir
         osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }     
      else if(curr_floor > next_floor){
             cmd[0]='d'; //Qual elevador
             cmd[1]='d';//descer
             osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }
      
    }//if do botão interno
    
    else if((msg[1]>='0' && msg[1]<='9') || (msg[2]>='0' && msg[1]<='2')) {
      floor_changed = true;
      
         sscanf(msg,"%*c%d", &signal_feedback);  
         curr_floor = signal_feedback;

          if(curr_floor == next_floor){
           
           dest_vect[curr_floor]=0; 
           cmd[0]='d'; //Qual elevador
           cmd[1]='p';//parar
           osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
           
           cmd[0]='d'; //Qual elevador
           cmd[1]='a';//abrir porta
           osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
           osTimerStart(timerd_id, 3000);
           osThreadFlagsWait(0x0003, osFlagsWaitAny, osWaitForever);

          for(i=0;i<16;i++){
              if(dest_vect[i]==1){
                 next_floor = i;
                 vect_zero = 1;
                 break;
                }else
                  vect_zero =0;
          }
         }
    }//Fim else do feedback    
    else{
        if(msg[1]=='A'){
           cmd[0]='d'; //Qual elevador
           cmd[1]='f';//fechar porta
           osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
        }        
    }
    
    if(vect_zero == 1){
      if(curr_floor < next_floor){
         cmd[0]='d'; //Qual elevador
         cmd[1]='s';//subir
         osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }     
      else if(curr_floor > next_floor){
             cmd[0]='d'; //Qual elevador
             cmd[1]='d';//descer
             osMessageQueuePut(queueUart_id, cmd, NULL, osWaitForever);
      }      
    }
  }
}

void uartRead(void *arg){
    char msg[6];
    while(1){
        //msg recebe a mensagem da uart
        if(msg[0]=='e'){
        osMessageQueuePut(queueE_id, msg, NULL, 0);
        }
        else if(msg[0]=='c'){
        osMessageQueuePut(queueC_id, msg, NULL, 0);
        }
        else if(msg[0]=='d'){
        osMessageQueuePut(queueD_id, msg, NULL, 0);
        }  
        osThreadYield(); // talvez usar osDelay(1) no lugar
    } 
}

void uartWrite(void *arg){
    char msg[2];
    while(1){
        osMessageQueueGet(queueUart_id, msg ,NULL, osWaitForever);
        //printa msg na uart
    } 
} 

void main(void){
    
    sysTick_setClkFreq();
    sysTick_setupSysTick();
    uart_setupUart();

    osKernelInitialize(); 

    uartRead_id= osThreadNew(uartRead, NULL, NULL);
    uartWrite_id= osThreadNew(uartWrite, NULL, NULL);

    queueE_id =  osMessageQueueNew (15, sizeof(char)*6,NULL);
    queueC_id =  osMessageQueueNew (15, sizeof(char)*6,NULL); 
    queueD_id =  osMessageQueueNew (15, sizeof(char)*6,NULL);

    queueUart_id =  osMessageQueueNew (30, sizeof(char)*3,NULL);

    elevadorE_id = osThreadNew(elevadorE, NULL, NULL);
    elevadorC_id = osThreadNew(elevadorC, NULL, NULL);
    elevadorD_id = osThreadNew(elevadorD, NULL, NULL);

    timerE_id = osTimerNew(callbackE, osTimerOnce, NULL,NULL);
    timerC_id = osTimerNew(callbackC, osTimerOnce, NULL,NULL);
    timerD_id = osTimerNew(callbackD, osTimerOnce, NULL,NULL);

    osKernelStart();    

    while(1);
}
