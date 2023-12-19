#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define TIME 15000
#define UPDATE_SERVER_TIME 1000
SoftwareSerial s(10, 11); // reference change
LiquidCrystal_I2C lcd(0x27,16,2);// change

int Opne_lane = 0;
unsigned int prev_time = 0;
int prev_server_time = 0;
int counter = 0;
int current_lane = 0;
bool emergency_on = false;
int emergency_lane = 3;

int sensor_pin1 = 2;
int sensor_pin11  = 6;
int sensor_pin2 = 3;
int sensor_pin22  = 7;
int sensor_pin3 = 4;
int sensor_pin33  = 8;
int sensor_pin4 = 5;
int sensor_pin44  = 9;
int sounds = 16;
int led_red[4] = {12, 13, 14, 15};
int lane_status[4] = {0, 0, 0, 0};
int sensor_count[4] = {0, 0, 0, 0};
int previous_state[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int current_state[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void simple_serial() {
  int dataa = 50;
  s.write(dataa);
  if (s.available() > 0) {
    int x = s.read();
    Serial.println(x);
    s.write(dataa);
  }
}

int timer = 0;
void json_serial() {
  StaticJsonDocument<1000> jsonBuffer;
  //JsonObject root = jsonBuffer.createObject();
  jsonBuffer["data1"] = sensor_count[0];
  jsonBuffer["data2"] = sensor_count[1];
  jsonBuffer["data3"] = sensor_count[2];
  jsonBuffer["data4"] = sensor_count[3];
  if (s.available() > 0) {
    serializeJson(jsonBuffer, s); //root.printTo(s);
  } else {
    Serial.println("Waiting for connection...");
  }
}

void read_sensor() {
  current_state[0] = 1 - digitalRead(sensor_pin1);
  current_state[1] = 1 - digitalRead(sensor_pin2);
  current_state[2] = 1 - digitalRead(sensor_pin3);
  current_state[3] = 1 - digitalRead(sensor_pin4);
  for (int i = 0; i < 4; i++) {
    if (current_state[i] != previous_state[i] && current_state[i] == 1) {
      sensor_count[i] += 1;
    }
    previous_state[i] = current_state[i];
  }
  current_state[4] = 1 - digitalRead(sensor_pin11);
  current_state[5] = 1 - digitalRead(sensor_pin22);
  current_state[6] = 1 - digitalRead(sensor_pin33);
  current_state[7] = 1 - digitalRead(sensor_pin44);
  for (int i = 0; i < 4; i++) {
    if (current_state[i + 4] != previous_state[i + 4] && current_state[i + 4] == 1) {
      sensor_count[i] -= 1;
    }
    previous_state[i + 4] = current_state[i + 4];
  }

}

void resetLaneStatus() {
  for (int i = 0; i < 4; i++) {
    lane_status[i] = 0;
  }
}
int selectLane() {
  int maximum = -1;
  for (int i = 0; i < 4; i++) {
    if (maximum == -1 && lane_status[i] == 0) {
      maximum = i;
    }
    else if (sensor_count[maximum] < sensor_count[i] && lane_status[i] == 0) {
      maximum = i;
    }
  }
  lane_status[maximum] = 1;
  return maximum;
}

void turnLightOn(int lane) {
  current_lane = lane;
  for (int i = 0; i < 4; i++) {
    if (i == lane) {
      digitalWrite(led_red[i], LOW);
    } else {
      digitalWrite(led_red[i], HIGH);
    }
  }
}

void next_lane() {
  if (counter == 4) {
    resetLaneStatus();
    counter = 0;
  }
  int current_lane = selectLane();
  turnLightOn(current_lane);
  counter++;
}
void displayArray() {
  for (int i = 0; i < 4; i++) {
    lcd.print("Lane");
    lcd.print(i);
    lcd.print(":");
    lcd.print(sensor_count[i]);
    Serial.println(sensor_count[i]);
    if(i==1)lcd.setCursor(0,1);
  }
  Serial.println("------xxxxx-----");
}
void setup() {
  s.begin(9600);
  Serial.begin(9600);
  pinMode(sensor_pin1, INPUT);
  pinMode(sensor_pin11, INPUT);
  pinMode(sensor_pin2, INPUT);
  pinMode(sensor_pin22, INPUT);
  pinMode(sensor_pin3, INPUT);
  pinMode(sensor_pin33, INPUT);
  pinMode(sensor_pin4, INPUT);
  pinMode(sensor_pin44, INPUT);

  pinMode(sounds, INPUT);

  pinMode(led_red[0], OUTPUT);
  pinMode(led_red[1], OUTPUT);
  pinMode(led_red[2], OUTPUT);
  pinMode(led_red[3], OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
}
void loop() {

  unsigned int cur_time = millis();
  int emergency = 1-digitalRead(sounds);
  Serial.println(emergency);
  if (emergency == 1) {
    emergency_on = true;
  }
  if (emergency_on) {
    Serial.println("xxxxxxxxxxxxxxxxxxx");
    turnLightOn(emergency_lane);
    if ((cur_time - prev_time) > TIME) {
      emergency_on = false;
      prev_time = cur_time;
//      lane_status[1]=1;
    }
  } else {
    if ((cur_time - prev_time) > TIME) {
      next_lane();
      prev_time = cur_time;
    }
  }

  if (cur_time - prev_server_time >= UPDATE_SERVER_TIME) {
    prev_server_time = cur_time;
    json_serial();
  }
  Serial.print("Current Lane ");
  Serial.println(current_lane);
  displayArray();
  read_sensor();

}
