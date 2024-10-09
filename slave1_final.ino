// SLAVE 1
#include <Temperature_LM75_Derived.h>
Generic_LM75 LM75;
#include <Wire.h>

#define LM75_ADDRESS 0x48
#include <string.h>

#define REG_COUNT 16

#define RXD2 16
#define TXD2 17

char* char_arr;
int address;
String function;
String mem_address;
String my_data;
const int N = 49;
int slaveid = 1 ;
char mystring[N];
int input_reg[REG_COUNT]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void read_package();
void run_read_register();
void run_excp(int excp);


void setup() {
 Serial.begin(115200); // For Monitor
 Serial2.begin(115200,SERIAL_8N1,RXD2,TXD2); // For Board communications
 Wire.begin();
 Serial.println("Actuator ESP32 Initialized");
 delay(100);
}
void loop() {  
if(Serial2.available())
   {
      delay(100);
      function = "";
      mem_address = "";
      my_data = "";
      Serial2.readBytes(mystring, N);
      Serial.println(mystring);
      Serial.println(" RECEIVER");
      read_package();
      Serial.print("Address: ");
      Serial.println(address);
      Serial.print("Function: ");
      Serial.println(function);
      Serial.print("Memory Address: ");
      Serial.println(mem_address);
      Serial.print("Data: ");
      Serial.println(my_data);
      if (address == slaveid) {
        if (function == "00000100") {
          run_read_register();
        } else {
          run_excp(0);
        }
      }
   }
}

String int2BinStr(int num,int len){
  String z="";
  String b=String(num, BIN);
  for(int i=0; i<len-b.length();i++){
    z.concat('0');
  }
  z.concat(b);
  return z;

}

int BinStr2Dec(char* b,int len){
  int decimal = 0 ;
  if (b[0] == '1'){
    decimal = 1;
  }
  for(int i=1;i<=len-1;i++){
    decimal = decimal << 1;
    if(b[i]!='0'){
      decimal = decimal + 1;
    }
  }
  return decimal;
}


void read_package() {
  address = BinStr2Dec(mystring,8); 

  // Read Function
  for(int i = 8; i <= 15; i++){
    function.concat(mystring[i]);
  }

  // Read Memory Address
  for(int i = 16; i <= 31; i++){
    mem_address.concat(mystring[i]);
  }

  // Read Data
  for(int i = 32; i <= 47; i++){
    my_data.concat(mystring[i]);
  }
}



void run_read_register() {
  int starting_addr = BinStr2Dec(&mem_address[0],16);
  int quantity = BinStr2Dec(&my_data[0],16);

  float temp=LM75.readTemperatureC();
  Serial.println(temp);
  

  Serial.println((int)(round(temp)));
  input_reg[0] = (int)(round(temp));
  Serial.println(input_reg[0]);
  //input_reg =(uint16_t)(round(temp));
 // for(int i = 0; i <= 15; i++){
   // input_reg[i]=(uint16_t)((round(temp))%10);
  //}
  //char *buf;
  if ( (starting_addr + quantity) <= REG_COUNT){
    String frame = "";
    frame.concat(int2BinStr(slaveid, 8)); //add slave address
    frame.concat(function);
    frame.concat(int2BinStr(quantity*2 , 8));
    for(int i =0 ; i< quantity ; i++){ // adding the valuese to frame
      frame.concat(int2BinStr(input_reg[starting_addr + i] , 16));
    }
    char_arr = &frame[0];
    Serial2.write(char_arr, quantity*2*8+8+8+8);
  Serial.println(char_arr);
  } else {
    run_excp(1);
  }
}


void run_excp(int excp) {
  String mystring2 = "00000001"; // Slave address (1)
  mystring2.concat(function);
  mystring2.concat(mem_address);
  mystring2[8] = '1'; // Change function to exception code
  if (excp == 0) {
    my_data = "0000000000000000";
  } else if (excp == 1) {
    my_data = "0000000000000001";
  }
  mystring2.concat(my_data);
  char_arr = &mystring2[0];
  Serial2.write(char_arr, N);
}