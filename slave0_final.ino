
// SLAVE 0
#include <string.h>
#define relay1 5
#define COIL_COUNT 25
#define RXD2 16
#define TXD2 17

char* char_arr;

int address;
int slaveid = 0;
String function;
String mem_address;
String my_data;


const int N = 49;
char mystring[N];

//int coil = 3;//0b11
bool coil_reg[COIL_COUNT] = {0};
int register1[16];

void read_package();
void run_read_coil();
void run_write_coil();
void run_write_register();
void run_excp(int excp);
String int2BinStr(int num,int len);
int BinStr2Dec(char* b,int len);

void setup() {
 Serial.begin(115200); // For Monitor
 Serial2.begin(115200,SERIAL_8N1,RXD2,TXD2); // For transmission between boards
 //pinMode(33, OUTPUT);
 pinMode(relay1,OUTPUT);
}
void loop() {
if(Serial2.available())
   {
      delay(100);
      function = "";
      mem_address = "";
      my_data = "";
      Serial2.readBytes(mystring, N);
      Serial.println(" NEW MESSAGE------------------");
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
        if (function == "00000001") {
          run_read_coil();
        } else if (function == "00000101") {
          run_write_coil();
        } else if (function == "00000110") {
          run_write_register();
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

/*void run_read_coil() {
  if (mem_address == "0000000000000000") {
    if (digitalRead(33) == HIGH) {
      String mystring2 = "00000000100000000000000001111111111111111";
      char_arr = &mystring2[0];
      Serial2.write(char_arr, 42);
    } else {
      String mystring2 = "00000000100000000000000000000000000000000";
      char_arr = &mystring2[0];
      Serial2.write(char_arr, 42);
    }
  } else {
    run_excp(1);
  }
}*/

void run_read_coil() {
  int starting_addr = BinStr2Dec(&mem_address[0],16);
  int quantity = BinStr2Dec(&my_data[0],16);

  coil_reg[0] = digitalRead(relay1);  // Assume relay1 is defined and set up

  //coil_reg = "0000000";  // Initialize with a base string      bool coil_reg[COIL_COUNT] = {0};
  //coil_reg += String(temp, BIN); 

  //Serial.println(coil_reg);

  //char *buf;
  if ( (starting_addr + quantity) <= COIL_COUNT) { /// if( starting adress + quantity  < 16)
    String frame="";
    frame.concat(int2BinStr(slaveid , 8)); //add slave address
    frame.concat("00000001"); // add function code
    frame.concat(int2BinStr(quantity/8 , 8)); //adding byte count

    //String val = int2BinStr(coil,COIL_COUNT);  // turning coil int to an string
    //val.toCharArray(buf, COIL_COUNT); // turning the string to an array
    for(int i =0 ; i< quantity/8 ; i++){ // adding the valuese to frame
      for(int j =0 ; j<8;j++){
        frame.concat(String(coil_reg[starting_addr + (i+1)*8-j-1]));
      }
    }

    char_arr = &frame[0];
    Serial2.write(char_arr, N-1);
  } 
  else {
    run_excp(1);
  }
}

void run_write_coil() {
  int starting_addr = BinStr2Dec(&mem_address[0],16);
  int value = BinStr2Dec(&my_data[0],16);
  if (starting_addr <= 16 && 0<=value<=65535) {
    String frame="";
    frame.concat(int2BinStr(slaveid , 8)); //add slave address
    frame.concat(function);
    frame.concat(mem_address);
    frame.concat(my_data);

    if(value == 0 ){
      coil_reg[starting_addr] = 0;
      digitalWrite(relay1, LOW);
    }
    else{
      coil_reg[starting_addr] = 1;
      digitalWrite(relay1, HIGH);
    }
    char_arr = &frame[0];
    Serial2.write(char_arr, N-1);
  } else {
    run_excp(1);
  }
}

void run_write_register() {
  int starting_addr = BinStr2Dec(&mem_address[0],16);
  int value = BinStr2Dec(&my_data[0],16);
  if (starting_addr <= 16 && 0<=value<=65535) {
    String frame="";
    frame.concat(int2BinStr(slaveid , 8)); //add slave address
    frame.concat(function);
    frame.concat(mem_address);
    frame.concat(my_data);
    /*for (int i = 0; i < 16; i++) { /////////////check
      register1[i] = my_data[i] - '0';
    }*/
    register1[starting_addr] = value;
    char_arr = &frame[0];
    Serial2.write(char_arr, N-1);
  } else {
    run_excp(1);
  }
}

void run_excp(int excp) {
  String mystring2 = "00000000";
  mystring2.concat(function);
  mystring2.concat(mem_address);
  mystring2[1] = '1'; // Change function to exception code
  if (excp == 0) {
    my_data = "0000000000000000";
  } else if (excp == 1) {
    my_data = "0000000000000001";
  }
  mystring2.concat(my_data);
  char_arr = &mystring2[0];
  Serial2.write(char_arr, N-1);
}
