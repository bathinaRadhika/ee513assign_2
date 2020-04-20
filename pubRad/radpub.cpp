#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include "MQTTClient.h"
#include <MQTTClientPersistence.h>
#include <time.h>
#include "ADXL345.h"
#include <unistd.h>
#include <pthread.h>


using namespace std;
using namespace exploringRPi;


//Please replace the following address with the address of your server
#define ADDRESS    "tcp://192.168.0.193:1883"  // server ip
#define CLIENTID   "rpi1"
#define AUTHMETHOD "radhika"
#define AUTHTOKEN  "radhiga"
#define TOPIC      "ee513/CPUTemp"
#define QOS        0
#define TIMEOUT    10000L


//time_t rawtime;
// struct tm * timeinfo;

float getCPUTemperature() {        // get the CPU temperature
   int cpuTemp;                    // store as an int
   fstream fs;
   fs.open("/sys/class/thermal/thermal_zone0/temp", fstream::in); // read from the file
   fs >> cpuTemp;
   fs.close();
   return (((float)cpuTemp)/1000);
}



int main(int argc, char* argv[]) {
   
   char str_payload1[100];  
   char str_payload2[100];
   char str_payload3[100];
   char str_payload4[100];
   MQTTClient client;
   MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
   MQTTClient_message pubmsg = MQTTClient_message_initializer;
   MQTTClient_deliveryToken token;
   MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);  
   MQTTClient_willOptions willmsg=MQTTClient_willOptions_initializer; 
   opts.cleansession = false;
   opts.keepAliveInterval = 20;
   opts.username = AUTHMETHOD;
   opts.password = AUTHTOKEN;
   opts.will=&willmsg;
   
  
   willmsg.topicName=TOPIC; // selecting for topic
   willmsg.message="sensor Disconnected";
   willmsg.qos=QOS;
   willmsg.retained=0;
   
   int rc;
   if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
      cout << "Failed to connect, return code " << rc << endl;
      return -1;
   }

//publishing temp
   sprintf(str_payload1, "{\"ee513\":{\"CPUTemp\": %f }}", getCPUTemperature());
   pubmsg.payload = str_payload1;
   pubmsg.payloadlen = strlen(str_payload1);
   pubmsg.qos = QOS;
   pubmsg.retained = 0;
   MQTTClient_publishMessage(client,TOPIC, &pubmsg, &token);
   cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
        " seconds for publication of " << str_payload1 <<
        " \non topic " << TOPIC << " for ClientID: " << CLIENTID << endl;

  time_t rawtime;
  struct tm * timeinfo;
  time (&rawtime);
  timeinfo = localtime (&rawtime);


// Time  
   sprintf(str_payload2, "{\"ee513\":{\"CPUTime\": %s }}", asctime(timeinfo));
   pubmsg.payload = str_payload2;
   pubmsg.payloadlen = strlen(str_payload2);
   pubmsg.qos = QOS;
   pubmsg.retained = 0;
   MQTTClient_publishMessage(client, TOPIC , &pubmsg, &token);
   cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
        " seconds for publication of " << str_payload2 <<
        " \non topic " << TOPIC << " for ClientID: " << CLIENTID << endl;

//initalising adxl
   ADXL345 sensor(1,0x53);
   sensor.setResolution(ADXL345::NORMAL);
   sensor.setRange(ADXL345::PLUSMINUS_4_G);
   
        
        

//publishing 
   sprintf(str_payload3, "{\"ee513\":{\"pitch\": %f }}", sensor.getPitchRoll(1)); // 1 for pitch
   pubmsg.payload = str_payload3;
   pubmsg.payloadlen = strlen(str_payload3);
   pubmsg.qos = QOS;
   pubmsg.retained = 0;
   MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
   cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
        " seconds for publication of " << str_payload3 <<
        " \non topic " << TOPIC << " for ClientID: " << CLIENTID << endl; 
        
/*MQTT publisher value for ROLL*/   
   sprintf(str_payload4, "{\"ee513\":{\"roll\": %f }}", sensor.getPitchRoll(2));  // any integer other than 1 is for roll
   pubmsg.payload = str_payload4;
   pubmsg.payloadlen = strlen(str_payload4);
   pubmsg.qos = QOS;
   pubmsg.retained = 0;
   MQTTClient_publishMessage(client,TOPIC , &pubmsg, &token);
   cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
        " seconds for publication of " << str_payload4 <<
        "\non topic " << TOPIC << " for ClientID: " << CLIENTID << endl; 
  
   rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
   cout << "Message with token " << (int)token << " delivered." << endl;
   MQTTClient_disconnect(client, 10000);
   MQTTClient_destroy(&client);
   return rc;
}
