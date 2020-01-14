
#include <hidef.h>      /* common defines and macros */
#include <mc9s12dg256.h>     /* derivative information */
#pragma LINK_INFO DERIVATIVE "mc9s12dg256b"
#include "main_asm.h" /* interface to the assembly module */

void cook(void);

#define F 1074 //261.63 Hz

int width;
int timeEntered;
char* welcome;
char* howLong;
char key;
int pitch;
int cookTime;
int wait; 
int cookTimeArray[5];
int cookTimeArray2[5];
int cookTime;
int i, j;
int multiply; 
int count;
int tempInF, tempInC, val;
int abort;
int ticks1, ticks;
int numbers [4] = {3, 6, 12, 9};
int pitchval [16] = { F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F };
char * numdisp[16] = {"3", "", "Cook", "0", "", "9", "8", "", "6", "5", "", "7", "4", "1", "", "2"};
                           //D              //C         //B            //*               //A
void interrupt 13 handler1(){
  tone(pitch);
}

void interrupt 7 handler(){    //RTI used for counter
  ticks++;
  clear_RTI_flag();
}
  
void main(void) {
  
  welcome = "Welcome";
  howLong = "Enter Cook Time";
  timeEntered = 0;
  
  count = 0;
  cookTime = 0;
  wait = 1;
  abort = 0;
  
  PLL_init(); 
  seg7_disable();
  keypad_enable();
  ad0_enable();
  servo76_init();
  lcd_init(); // enable LCD
  set_lcd_addr(0x00);
	type_lcd(welcome); //write Welcome
	set_lcd_addr(0x40);
	type_lcd("               ");
	ms_delay(300);
 	set_lcd_addr(0x40);
	type_lcd(howLong); //write Enter Cook Time 
	DDRH = 0x00;
	
	set_lcd_addr(0x00);
	key = getkey();
  clear_lcd();
  
	//this is the input phase, system waits for time to be entered
	while(wait){ 
    key = getkey();
    type_lcd(numdisp[key]);
    pitch = pitchval[1];
  	sound_init();
    sound_on();
    wait_keyup();
    sound_off();
	  switch(key){
	    case 0x3:
	      cookTimeArray[count] = 0;
	      count++;
	      break;
	    case 0xD: 
	      cookTimeArray[count] = 1;
	      count++;
	      break;
	    case 0xF: 
	      cookTimeArray[count] = 2;
	      count++;
	      break;
	    case 0x0: 
	      cookTimeArray[count] = 3;
	      count++;
	      break;
	    case 0xC: 
	      cookTimeArray[count] = 4;
	      count++;
	      break;
	    case 0x9: 
	      cookTimeArray[count] = 5;
	      count++;
	      break;
	    case 0x8: 
	      cookTimeArray[count] = 6;
	      count++;
	      break;
	    case 0xB: 
	      cookTimeArray[count] = 7;
	      count++;
	      break;
	    case 0x6: 
	      cookTimeArray[count] = 8;
	      count++;
	      break;
	    case 0x5: 
	      cookTimeArray[count] = 9;
	      count++;
	      break;
	    case 0x2: // #
	      clear_lcd(); // cook here
	      wait = 0; //exit loop
	      break;
    }
	}
	
	
	//multiply by base 10 to add to correct number
	for(i = 0; i < count; i++){
	  for(j = 0; j < i; j++){  
	    cookTimeArray[j] *= 10;
	  }
	}
	
	for(i = 0; i < count; i++){  //add mumbers of array into cookTime 
	  cookTime += cookTimeArray[i];
	}
	
	for(width = 3500; width <= 6000; width = width + 10){  //servo latch door
      set_servo76(width);
      ms_delay(5); 
  }
	
	RTI_init(); 
	//cooking phase
	while(cookTime && !abort){
     

	   val = ad0conv(2);   //thermistor
	   val = val >> 3;
	   tempInC = val - 20;
	   tempInF = (tempInC*9/5 + 32);
	   set_lcd_addr(0x40);
	   write_int_lcd(tempInF);
	   
	   cook();  //RTI used as counter and abort  	     
	}
	
	RTI_disable();
	led_enable();
	
	//once cooking is finished
	for(i = 0; i < 3; i++){
  	clear_lcd();
  	PORTB ^= 255;
	  type_lcd("Cooking Done");
	  pitch = pitchval[1];
	  sound_on();
	  ms_delay(400);
	  if(i == 2){
	    ms_delay(600); //make third beep longer
	  }
	  sound_off();
	  PORTB &= 0;
	  ms_delay(200);
	}
	
	for(width = 6000; width >= 3500; width = width - 10){  //servo unlatch door
      set_servo76(width);
      ms_delay(5); 
  }
	
	clear_lcd();
	type_lcd("ENJOY");
	
}

void cook(void){    //subroutine to handle cooking
  ticks1 = ticks;   //RTI increments ticks
  set_lcd_addr(0x00); 
	write_int_lcd(cookTime);
	cookTime--;
  while((ticks - ticks1)<65){      //1000/10.24 = 993.28 (close to 1 second)
  
  for(i = 0; i < 4; i++){
       led_disable();     //for a nice flashing effect on leds when cooking
       PORTB = numbers[i];
       ms_delay(5); 
       led_enable();
  } 
                                  //although 65 is not a true second, it feels better at this speed
  if(!(PTH & 0x01)){               //switch 5 to abort cooking
	    abort = 1; 
	    clear_lcd();                 //abort was placed in this subroutine to ensure user would -
	    type_lcd("Cooking Aborted"); //not need to push button for up to one second to abort
	    ms_delay(1000);
	   }
  }                           
}