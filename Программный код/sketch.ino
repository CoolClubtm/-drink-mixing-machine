#define SMOOTH_ALGORITHM

#include <GyverStepper.h>
#include <NewPing.h>

#define TRIG 20
#define ECHO 21
NewPing sonar(TRIG, ECHO, 400);
GStepper<STEPPER2WIRE> stepper(2048, 51, 53, 49);

// газированная вода - 1; мятный сироп - 2; апельсиновый сок - 3; лимонад “Мятный” - 4;
// Лимонад “Заводной апельсин” - 5; Лимонад ‘Тройной” - 6; работает только диспенсер - 8;
// работает только доставка - *; разрешить работу диспенсера - 9(режим 'только диспенсер');
// разрешить работу доставки - 7(режим 'только доставка')

#include <SimpleKeypad.h>




uint32_t tmr1;
uint32_t tmr2;
uint32_t wait_time;
uint32_t micro_tmr1;
uint32_t micro_tmr2;
uint32_t timer_stepper;

#define time_for_up 29
#define time_for_other 45
#define time_for_juice 36

float const_need_deg = 22.5;
int intkey = 0;
float need_deg = 0;
bool need_for_motion = false;
bool ready = false;
uint32_t ready_time;
int count_of_orders = 0;
bool start = true;
bool wait = false;

int val = 0;
int current_order = 0;
int const_need_up = 0;
int const_need_orange_juice = 0;
int const_need_mint_syrup = 0;
int need_up = 0;
int need_orange_juice = 0;
int need_mint_syrup = 0;
const int min_distance = 2;
const int max_distance = 8;
int order = 0;
int position;
int pos = 0;
int time_for_motion;
int count_orders = 0;

#define KP_ROWS 4
#define KP_COLS 3
#define relay_sparkling_water 50
#define relay_orange_juice 46
#define relay_mint_syrup 27



// пины подключения (по порядку штекера)
byte colPins[KP_COLS] = {4, 3, 2};
byte rowPins[KP_ROWS] = {8, 7, 6, 5};
// массив имён кнопок
char keys[KP_ROWS][KP_COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

// создаём клавиатуру
SimpleKeypad pad((char*)keys, rowPins, colPins, KP_ROWS, KP_COLS);


void setup() {
  Serial.begin(9600);

  pinMode(relay_sparkling_water, OUTPUT); //верхняя бутылка
  pinMode(relay_orange_juice, OUTPUT);
  pinMode(relay_mint_syrup, OUTPUT);
  tmr1 = -10001;

  stepper.setRunMode(FOLLOW_POS);
  stepper.setMaxSpeed(12);
  stepper.setAcceleration(4);
  stepper.autoPower(true);
  stepper.enable();
}

void loop() {

  if (start) {
    order = keypad(); // Записывание заказа в переменную из функции кнопочного ввода
    start = false;
    need_for_motion = false;
    Serial.println(order);

  }

  if (order && need_for_motion == false) {
    if (!current_order ) {
      current_order = order % 10;
    }
    if ((min_distance < sonar.ping_cm() < max_distance) && !wait) {
      if (!const_need_up && !const_need_mint_syrup && !const_need_orange_juice) {
        if (current_order == 1) {
          const_need_up = 50;
          need_up = 50;
        }

        if (current_order == 2) {
          const_need_mint_syrup = 10;
          need_mint_syrup = 10;
        }

        if (current_order == 3) {
          const_need_orange_juice = 40;
          need_orange_juice = 40;
        }

        if (current_order == 4) {
          const_need_up = 80;
          need_up = 80;
          const_need_mint_syrup = 20;
          need_mint_syrup = 20;
        }

        if (current_order == 5) {
          const_need_up = 30;
          need_up = 30;
          const_need_orange_juice = 50;
          need_orange_juice = 50;
        }

        if (current_order == 6) {
          const_need_up = 35;
          need_up = 35;
          const_need_orange_juice = 45;
          need_orange_juice = 45;
          const_need_mint_syrup = 10;
          need_mint_syrup = 10;
        }
        micro_tmr1 = millis();
        tmr2 = millis();
      }

    }
    if (((min_distance >= sonar.ping_cm() || sonar.ping_cm() >= max_distance) && !wait)) {
      digitalWrite(relay_sparkling_water, 0);
      digitalWrite(relay_mint_syrup, 0);
      digitalWrite(relay_orange_juice, 0);
      wait_time = millis();
      wait = true;
      tmr2 = millis();

    }
    if ((min_distance < sonar.ping_cm() && sonar.ping_cm() < max_distance) && wait) {
      wait = false;
      wait_time = millis() - wait_time;
    }
    if (min_distance < sonar.ping_cm() < max_distance && !wait) {
      if (need_orange_juice && !need_mint_syrup && !need_up) {
        if (millis() - micro_tmr1 < time_for_juice) {
          digitalWrite(relay_orange_juice, 1);
        }
        if (millis() - micro_tmr1 >= time_for_juice) {
          need_orange_juice --;
          micro_tmr1 = millis();
          if (!need_orange_juice) digitalWrite(relay_orange_juice, 0);
        }
      }

      if (need_mint_syrup && !need_up) {
        if (millis() - micro_tmr1 < time_for_other) digitalWrite(relay_mint_syrup, 1);
        if (millis() - micro_tmr1 >= time_for_other) {
          need_mint_syrup --;
          micro_tmr1 = millis();
          if (!need_up) digitalWrite(relay_mint_syrup, 0);
        }
      }

      if (need_up) {
        if (millis() - micro_tmr1 < time_for_up) digitalWrite(relay_sparkling_water, 1);
        if (millis() - micro_tmr1 >= time_for_up) {
          need_up --;
          micro_tmr1 = millis();
          if (!need_up) digitalWrite(relay_sparkling_water, 0);
        }
      }
    }


    long int true_time = millis();
    long int true_tmr2 = tmr2;
    long int true_wait_time = wait_time;
    if (!wait && (true_time - true_tmr2 - true_wait_time) >= (const_need_up * time_for_up + const_need_mint_syrup * time_for_other + const_need_orange_juice * time_for_juice + 1000)) {
      order = (order - current_order) / 10;
      need_for_motion = true;
      tmr1 = millis();
      wait_time = 0;
      wait = false;
      current_order = 0;
      const_need_up = 0;
      const_need_mint_syrup = 0;
      const_need_orange_juice = 0;
      val = 0;
      count_orders ++;
    }
  } // Функция наливания

  if (need_for_motion) {
    stepper.tick();
    if (!val || (position >= 4 && millis() - timer_stepper >= time_for_motion)) {
      timer_stepper = millis();
      if (order == 0 && position < 4) {
        val =  100;
        stepper.setTarget(val);
        time_for_motion = 10000 - 2000 * position;
        position = 4;

      }
      if (order) {
        position ++;
        val = position * 25;
        stepper.setTarget(val);
        time_for_motion = 3800;
      }
      if (!order && position >= 4 && sonar.ping_cm() > 20) {
        if (count_orders) {
          position ++;
          val = position * 25;
          stepper.setTarget(val);
          time_for_motion = 3800;
          count_orders --;
        }
        else {
          val = -1;
          stepper.setTarget(val);
          time_for_motion = position * 3000;
          position = 0;

        }
      }
    }
    if (millis() - timer_stepper >= time_for_motion) {
      if (position < 4) need_for_motion = false;
      if (val == -1) start = true;
    }
  }

}


int keypad() {
  int order = 0;
  int count = 1;
  while (intkey != -13 && order / 1000 < 1) { // пока не нажата кнопка "подтвердить заказ" (кнопка 9) И пока не сделано четыре заказа
    char key = pad.getKey();
    int intkey = key - 48;;
    if (key > 0 && -1 < intkey && intkey < 7) {
      order = order + intkey * count;
      count *= 10;
    }
    if (intkey == -13 && order == 0) intkey = -100;
    if (intkey == -13) break;
  }
  return order;
}

