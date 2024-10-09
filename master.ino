// MASTER
#include <string.h>
#define RXD2 16
#define TXD2 17

const int N = 49;
int error0 = 0;
int error1 = 0;
bool s0oof = false; // Slave 0 Out of Service
bool s1oof = false; // Slave 0 Out of Service
char mystring2[N];
char in_byte[8];
String mystring;
char* char_arr;

int address;
String function;
String mem_address;
String my_data;
String byte_count;
String s_reg_data;

void read_package();
void read_coil(int slave , int StartAddr ,int quantity );
void write_coil(int slave , int StartAddr,int value );
void read_register(int slave , int StartAddr,int quantity );
void  write_register(int slave  , int StartAddr, int value);
void error_handler();
String int2BinStr(int num,int len);
int BinStr2Dec(char* b,int len);

void setup() {                
 Serial.begin(115200); // PC Display
 Serial2.begin(115200,SERIAL_8N1,RXD2,TXD2); // Transmission
}

void loop() {
int i =0;
intrupt:
 if(Serial.available()) 
   {
      String userinput;
      userinput = Serial.readString();
      Serial.print("You wrote ");
      Serial.print(userinput);
      if (userinput == "writecoil1\n") {
        write_coil(0,0,1);
      }else if (userinput == "writecoil0\n") {
        write_coil(0,0,0);
      }
      else if (userinput == "writeregister\n") {
        write_register(0,0,4);
      }
      else if (userinput == "readregister\n") {
        read_register(1,0,1);
      }
      else if (userinput == "readcoil\n") {
        read_coil(0,0,24);
      }
   }
 if(Serial2.available())
   {
      read_package();
      if (function[0] == '1') {
        error_handler();
      } else {
        Serial.println("Status: Command Performed Successfully");
      }
          
   }
   for ( i=0 ; i < 19500; i++) {
      delay(1);
      if(Serial.available() || Serial2.available()) {
        goto intrupt;
      }
   }
   Serial.println("---20secs passed---");
   //read_coil(0,0,1);
   delay(5000);
   read_register(1,0,1);
   i = 0;
}//end off loop


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

void read_coil(int slave ,int StartAddr, int quantity ) {
  // This function works properly (hard coded)
  if (!s0oof) {
    // Read Coil -> Controller
    // Address: 0 | Function: 0000 0001 (x01) | Starting Address: 000 0000 0000 0000 | Quantity: 0000 0000 0000 0001
    //check quantity limit --------------------------------------------------------------------------------------------
    
    //mystring = "00000000100000000000000000000"; // Read Coil 0 without quantity
    String frame = int2BinStr(slave,8); //adding slave id
    frame.concat("00000001"); //adding function code
    frame.concat(int2BinStr(StartAddr,16)); //adding starting address
    frame.concat(int2BinStr(quantity,16)); // adding coil quantity
    char_arr = &frame[0];
    Serial2.write(char_arr, N);
  } else {
    Serial.println("Status: !!!SLAVE 0 OUT OF SERVICE, RESET!!!");
  }
  
}

void write_coil(int slave , int StartAddr, int value ) {
  if (!s0oof) {
    // Write Single Coil -> Controller
    // Address: 0 | Function: 0000 0101 (05) | Address: 0000 0000 0000 0000 | Value: 1111 1111 0000 0000 (High) or 0000 0000 0000 0000 (Low)
        String frame = int2BinStr(slave,8); //adding slave id
    frame.concat("00000101"); //adding function code
    frame.concat(int2BinStr(StartAddr,16)); //adding starting address
    if(value>0){
      frame.concat("1111111100000000"); // adding coil value if HIGH
    } else{
      frame.concat("0000000000000000"); // adding coil value if LOW
    }
    char_arr = &frame[0];
    Serial2.write(char_arr, N);
  } 
  else {
    Serial.println("Status: !!!SLAVE 0 OUT OF SERVICE, RESET!!!");
  }
}

void read_register(int slave , int StartAddr, int quantity ) {
  if (!s1oof) {
    // Read Input Register -> Temperature
    // Address: 1 | Function: 0000 0100 (04) | Address: 0000 0000 0000 0000 | Quantity: 0000 0000 0000 0001
    //mystring = "10000010000000000000000000000000000000001"; //  Read
    String frame = int2BinStr(slave,8); //adding slave id
    frame.concat("00000100"); //adding function code
    frame.concat(int2BinStr(StartAddr,16)); //adding starting address
    frame.concat(int2BinStr(quantity,16)); // adding coil quantity
    char_arr = &frame[0]; 
    Serial2.write(char_arr, N);
  } else {
    Serial.println("Status: !!!SLAVE 1 OUT OF SERVICE, RESET!!!");
  }
  
}

void write_register(int slave  , int StartAddr, int value) {
  if (!s0oof) {
     // Write Single Register -> Controller
     // Address: 0 | Function: 0000 0110 (06) | Address: 0000 0000 0000 0100 | Value: 0000 0000 1111 1111
    String frame = int2BinStr(slave,8); //adding slave id
    frame.concat("00000110"); //adding function code
    frame.concat(int2BinStr(StartAddr,16)); //adding starting address
    frame.concat(int2BinStr(value,16)); //adding register value
    char_arr = &frame[0];
    Serial2.write(char_arr, N);
  } 
  else {
    Serial.println("Status: !!!SLAVE 0 OUT OF SERVICE, RESET!!!");
  }
}

void error_handler() {
  int proper_function = BinStr2Dec(&function[8], 7); // dont want the first bit since it only indicates an error
  int excp = BinStr2Dec(&mystring2[16],8); // mystring2[9] should be right after function code and sice the exception is 01,02,03,04 reading one byte is enough
  int slave = address;
  //slave un available  ?????????????????????????????????????????????????
  if (excp == 1) {
    Serial.println("ModBus excptoin 1: !!!SLAVE "); 
    Serial.print(slave);  
    Serial.println(" NON EXITSTING FUNCTION!");     
  } else if (excp == 2) {
    Serial.println("ModBus excptoin 1: !!!SLAVE "); 
    Serial.print(slave);  
    Serial.println(" NON EXITSTING ADDRESS!");
  } else if (excp == 3) {
    Serial.println("ModBus excptoin 1: !!!SLAVE "); 
    Serial.print(slave);  
    Serial.println(" NON EXITSTING VALUE OR QUANTITY!");
  } else if (excp == 4) {
    Serial.println("ModBus excptoin 1: !!!SLAVE "); 
    Serial.print(slave);  
    Serial.println(" %d CANT DO THE TASK!");
  }
}

void read_package() {

  Serial.println("Received Information:");
  function = "";
  mem_address = "";
  my_data = "";
  byte_count ="";
  s_reg_data="";
  Serial.println("");

  Serial2.readBytes(in_byte,8 );
  Serial.print(in_byte);
  address = BinStr2Dec(in_byte,8); //ASCI value to real value (int) since we have only two slaves


  // Read Function
  Serial2.readBytes(in_byte,8 );
  Serial.print(in_byte);
  for(int i = 0; i <= 7; i++){
    function.concat(in_byte[i]);
  }

  if(function == "00000100"){//readregister

    Serial2.readBytes(in_byte,8 );
    Serial.print(in_byte);
    for(int i = 0; i <= 7; i++){
      byte_count.concat(in_byte[i]); //reading the byte count
    }
    int byte_count_int = BinStr2Dec(&byte_count[0],8);

    for( int j=0; j<byte_count_int;j++){
      Serial2.readBytes(in_byte,8);
      Serial.print(in_byte);
      for(int i = 0; i <= 7; i++){
        s_reg_data.concat(in_byte[i]); //reading register data according to byte count
      }
    }
    Serial.print("\nAddress: ");
    Serial.println(address);
    Serial.print("Function: ");
    Serial.println(function);
    Serial.print("byte count: ");
    Serial.println(byte_count);
    Serial.print("register data: ");
    Serial.println(s_reg_data);
    Serial.println("Flushing Serial Buffers\n");
    Serial.flush();
    Serial2.flush();
  }
  else if(function == "00000001"){//readregister

    Serial2.readBytes(in_byte,8 );
    Serial.print(in_byte);
    for(int i = 0; i <= 7; i++){
      byte_count.concat(in_byte[i]); //reading the byte count
    }
    int byte_count_int = BinStr2Dec(&byte_count[0],8);

    for( int j=0; j<byte_count_int;j++){
      Serial2.readBytes(in_byte,8);
      Serial.print(in_byte);
      for(int i = 0; i <= 7; i++){
        s_reg_data.concat(in_byte[i]); //reading register data according to byte count
      }
    }
    Serial.print("\nAddress: ");
    Serial.println(address);
    Serial.print("Function: ");
    Serial.println(function);
    Serial.print("byte count: ");
    Serial.println(byte_count);
    Serial.print("coil register data: ");
    for(int j=0; j<s_reg_data.length();j++){
      if(j%8 == 0){
        Serial.println();
      }
      Serial.print(s_reg_data[j]);
    }
    Serial.println("\nFlushing Serial Buffers\n");
    Serial.flush();
    Serial2.flush();
  }
  else{

    // Read Memory Address
    for(int j=0;j<2;j++){
      Serial2.readBytes(in_byte,8 );
      Serial.print(in_byte);
      for(int i = 0; i <= 7; i++){
        mem_address.concat(in_byte[i]);
      }
    }

    // Read Data
    for(int j=0;j<2;j++){
      Serial2.readBytes(in_byte,8 );
      Serial.print(in_byte);
      for(int i = 0; i <= 7; i++){
        my_data.concat(in_byte[i]);
      }
    }
    Serial.print("\nAddress: ");
    Serial.println(address);
    Serial.print("Function: ");
    Serial.println(function);
    Serial.print("Memory Address: ");
    Serial.println(mem_address);
    Serial.print("Data: ");
    Serial.println(my_data);
    Serial.println("Flushing Serial Buffers\n");
    Serial.flush();
    Serial2.flush();
  }

}

