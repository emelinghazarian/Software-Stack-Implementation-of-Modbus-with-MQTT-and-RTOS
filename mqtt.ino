#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

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
void Read_Modbus_Task( void* pvParameters);

// WiFi
const char *ssid = "Emelin"; // Enter your WiFi name
const char *password = "23232323";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "esp32";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);


const char* ca_cert= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=" \
"-----END CERTIFICATE-----\n";



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // PC Display
  Serial2.begin(115200,SERIAL_8N1,RXD2,TXD2); // Transmission
  
  // Creating RTOS task 
  xTaskCreatePinnedToCore(
    Read_Modbus_Task,         // function name
    "Read_Modbus_Task",       // a name just for humans
    4096,                     // Stack size
    NULL,                     // input parameters
    2,                        // priority (0 to 3) higher has more priority
    NULL,                     // handler
    xPortGetCoreID()          // core name xPortGetCoreID() returns current cpu core id (can be 0 or 1 for ESP32)
  );
  
  // Connecting to a Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);


  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("Public EMQX MQTT broker connected");
    } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
    }
  }

  // init wifi secure client
  //espClient.setCACert(ca_cert);

  // publish and subscribe
  //client.publish(topic, "Hi, I'm ESP32 ^^");
  client.subscribe(topic);
}

void loop() {
  // put your main code here, to run repeatedly:
    // MQTT Connection

  client.loop();

  
  if(Serial.available()) 
  {
    String userinput;
    userinput = Serial.readString();
    Serial.print("You wrote ");
    Serial.print(userinput);
    }
  if(Serial2.available())
  {
    read_package();
    if (function[0] == '1') {
      error_handler();
    }
    else {
      Serial.println("Status: Command Performed Successfully");
    }
          
  }

  

  

}

// for arrived massages used to change the relay condition
void callback(char *topic, byte *payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++)
      message += (char) payload[i];
    Serial.println("Message arrived topic: " + String(topic) + ", message: " + message);

    if(message == "relayon"){
      Serial.printf("relay = %d\n", (message == "relayon"));
      write_coil(0,0,1);
      client.publish("masiha", "coil value: 1");
      client.subscribe(topic);
      Serial.println("reaaaa");
    }
    else if(message == "relayoff" ){
      Serial.printf("relay = %d\n", (message == "relayoff"));
      write_coil(0,0,0);
      client.publish("masiha", "coil value: 0");
      client.subscribe(topic);
      Serial.println("reaaaa");
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

void write_coil(int slave , int StartAddr, int value ) {
  if (!s0oof) {
    // Write Single Coil relay
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

void read_coil(int slave ,int StartAddr, int quantity ) {
  if (!s0oof) {
    // Read Coil -> Controller
    // Address: 0 | Function: 0000 0001 (x01) | Starting Address: 000 0000 0000 0000 | Quantity: 0000 0000 0000 0001 
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
    const char* c_str_reg_data = s_reg_data.c_str(); // Convert String to const char*
    int decimalValue = strtol(c_str_reg_data, NULL, 2);
    char msg[50];
    sprintf(msg, "rom temperature is: %ld", decimalValue);
    client.publish("masiha", msg);
    client.subscribe(topic);
  }
  else if(function == "00000001"){//readcoil

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
  /*

    const char* c_str_reg_data = s_reg_data.c_str(); // Convert String to const char*
    int decimalValue = strtol(c_str_reg_data, NULL, 2);
    char msg[50];
    sprintf(msg, "coil value: %ld", decimalValue);
    client.publish("masiha", msg);
    client.subscribe(topic);
    */
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

void Read_Modbus_Task( void* pvParameters){
  // infinity loop of this RTOS Task
  for(;;){
    // printing recieved modbus messages to serial monitor
    Serial.println(Serial2.read());
    Serial.println("---10secs passed---");
    read_coil(0,0,1);
    delay(5000);
    read_register(1,0,1);
    int i = 0;
    delay(5000); // every 10 second

    // Passing control to other tasks when called. Ideally yield() should be used in functions that will take awhile to complete.
    yield();
  }
}
