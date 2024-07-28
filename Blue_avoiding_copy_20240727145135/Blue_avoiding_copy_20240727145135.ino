#include <Arduino_FreeRTOS.h>
#include <Servo.h>
#include <task.h>
#include <queue.h>
#include <AFMotor.h>

#define ECHO_PIN A0
#define TRIG_PIN A1
AF_DCMotor motor1(3);  // M3 Motor object
AF_DCMotor motor2(4);  // M4 Motor object
Servo myServo;
char command;
TaskHandle_t xTask1;
TaskHandle_t xTask2;
TaskHandle_t xTask3;

QueueHandle_t f;
QueueHandle_t r;
QueueHandle_t l;

void Task1(void *pvP); //nhận dữ liệu từ người dùng
void Task2(void *pvP); // chế độ tự hành
void Task3(void *pvP); // đo khoảng cách từ 0-180 độ

void setup() {
  // put your setup code here, to run once:
  pinMode(TRIG_PIN, OUTPUT);  // Thiết lập chân TRIG_PIN là OUTPUT A1
  pinMode(ECHO_PIN, INPUT);   // Thiết lập chân ECHO_PIN là INPUT A0
  Serial.begin(9600);
  myServo.attach(9);
  char message[4];
  f = xQueueCreate(1, sizeof(message));
  r = xQueueCreate(1, sizeof(message));
  l = xQueueCreate(1, sizeof(message));
  xTaskCreate(Task1, "Task 1", 128, NULL, 1, &xTask1);
  vTaskStartScheduler();
}

void Task1(void *pvP) {
  char num[4];
  int l;
  int r;
  int f;
  xTaskCreate(Task3, "Task 3", 128, NULL, 1, &xTask3);
  for (;;) {
    if (Serial.available() > 0) {
      command = Serial.read();
      Serial.println(command);
      if (command == 'F') {
        xQueuePeek(f, num, (TickType_t)0);
        f = atoi(num);
        Serial.println(f);
        if (f > 25) {
          huongxe(1);
          Serial.println(" xe di thang!");
        }
      }
      if (command == 'B') {
        huongxe(2);
        Serial.println(" xe di lui!");
      }
      if (command == 'L') {
        huongxe(4);
        Serial.println(" xe sang trai!");
      }
      if (command == 'R') {
        huongxe(3);
        Serial.println(" xe sang phai!");
      }
      if (command == 'S') {
        huongxe(0);
        Serial.println(" xe dung!");
      }
      if (command == 'X') {
        xTaskCreate(Task2, "Task 2", 128, NULL, 1, &xTask2);
      }
    }
  }
}

void Task2(void *pvP) {
  Serial.println(" task2!");
  //xe di auto
  for (;;) {
    dichuyen(25, 1000);
  }
}

void Task3(void *pvP) {
  //do khoang cach
  int pos = 0;
  char message[4];
  unsigned long duration1;
  unsigned long duration2;
  unsigned long duration3;
  unsigned long duration;
  int distance;
  int distance1;
  int distance2;
  int distance3;
  for (;;) {
    for (pos = 0; pos <= 180; pos += 1) {
      myServo.write(pos);
      delay(15);
      distance = dokhoangcach(pos);
      if (distance <= 30) {
        Serial.print(distance);
        Serial.println("==========phat hien co vat can phia truoc");
      }
      distance1 = dokhoangcach(180);
      Serial.println(distance1);
      sprintf(message, "%d", distance1);
      if (distance1 > 0) {
        xQueueOverwrite(l, message);
      }

      distance2 = dokhoangcach(90);
      Serial.println(distance2);
      sprintf(message, "%d", distance2);
      if (distance2 > 0) {
        xQueueOverwrite(f, message);
      }

      distance3 = dokhoangcach(0);
      Serial.println(distance3);
      sprintf(message, "%d", distance3);
      if (distance3 > 0) {
        xQueueOverwrite(r, message);
      }
    }
    for (pos = 180; pos >= 0; pos -= 1) {
      myServo.write(pos);
      delay(15);

      distance1 = dokhoangcach(180);
      Serial.println(distance1);
      sprintf(message, "%d", distance1);
      if (distance1 > 0) {
        xQueueOverwrite(l, message);
      }

      distance2 = dokhoangcach(90);
      Serial.println(distance2);
      sprintf(message, "%d", distance2);
      if (distance2 > 0) {
        xQueueOverwrite(f, message);
      }

      distance3 = dokhoangcach(0);
      Serial.println(distance3);
      sprintf(message, "%d", distance3);
      if (distance3 > 0) {
        xQueueOverwrite(r, message);
      }
    }
  }
}

void dichuyen(int gioihan, int thoigiantre) {
  char num[4];
  int l;
  int r;
  int f;

  xQueuePeek(l, num, (TickType_t)0);
  l = atoi(num);
  xQueuePeek(r, num, (TickType_t)0);
  r = atoi(num);
  huongxe(1);

  xQueuePeek(f, num, (TickType_t)0);
  f = atoi(num);
  if (f > gioihan || f == 0) {
    huongxe(1);
    Serial.println("");
    delay(10);
  }
  if (f <= gioihan) {
    huongxe(2);
    Serial.println("xe di lui 300ms");
    delay(300);
    //xe đứng yên
    huongxe(0);
    Serial.println("xe dung yen");
    if (l > r) {
      if (l > gioihan || l == 0) {
        //quay xe sang trái
        huongxe(4);
        Serial.println("quay xe sang trai");
        delay(thoigiantre / 2);
      } else {
        //quay xe về phía sau
        huongxe(3);
        Serial.println("quay ve sau");
        delay(thoigiantre);
      }
    } else {
      if (r > gioihan || r == 0) {
        //quay xe sang phải
        huongxe(3);
        Serial.println("quay xe sang phai");
        delay(thoigiantre / 2);
      } else {
        //quay xe về sau
        huongxe(4);
        Serial.println("quay ve sau");
        delay(thoigiantre);
      }
    }
  }
}

void huongxe(int trangthai) {
  switch (trangthai) {
    case 0:// không di chuyển
      dieukhiendongco(motor1, 0);
      dieukhiendongco(motor2, 0);
      break;
    case 1://đi thẳng
      dieukhiendongco(motor1, 1);
      dieukhiendongco(motor2, 1);
      break;
    case 2:// lùi lại
      dieukhiendongco(motor1, 2);
      dieukhiendongco(motor2, 2);
      break;
    case 3:// quay trái
      dieukhiendongco(motor1, 1);
      dieukhiendongco(motor2, 2);
      break;
    case 4:// quay phải
      dieukhiendongco(motor1, 2);
      dieukhiendongco(motor2, 1);
      break;
  }
}

void dieukhiendongco(AF_DCMotor& motor, int num) {
  switch (num) {
    case 0://2 bánh đứng yên
      motor.setSpeed(0);
      motor.run(RELEASE);
      break;
    case 1:// Quay bánh thứ nhất
      motor.setSpeed(255);
      motor.run(FORWARD);
      break;
    case 2:// Quay bánh thứ 2
      motor.setSpeed(255);
      motor.run(BACKWARD);
      break;
  }
}

int dokhoangcach(int goc) {
  //myServo.write(goc);
  unsigned long thoigian;
  int khoangcach;
  digitalWrite(TRIG_PIN, 0);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, 1);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, 0);
  thoigian = pulseIn(ECHO_PIN, HIGH);
  khoangcach = int(thoigian / 2 / 29.412);
  //Serial.println(khoangcach);
  return khoangcach;
}
void loop() {

}
