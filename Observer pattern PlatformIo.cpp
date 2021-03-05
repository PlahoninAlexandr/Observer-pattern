// Plakhonin Alexandr, Lab3.2, use Observer pattern
#include <vector>
#include <Arduino.h>
#include "DHT.h"
#include <WiFiManager.h>
#include <BlynkSimpleEsp8266.h>
#define BLYNK_PRINT Serial

using namespace std;

char auth[] = ****************************";
char ssid[] = "*****";
char pass[] = "**********";

const int diod = 3;
const int timer = 5000;
const int rattling = 50;
unsigned long last_press, curr_time = millis();
boolean led_state = 0, butt_flag = 0, butt;
float h, t, dh, bt;
int led[diod] = { D1, D2, D3 };
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

class Subject {
    vector < class Observer* > observers;
    bool scored;
public:
    void attach(Observer* obs) {
        observers.push_back(obs);
    }

    void setScored(bool Score) {
        scored = Score;
        notify();
    }

    bool getScored() {
        return scored;
    }

    void notify();
};

class Observer {
    Subject* subj;
public:
    Observer(Subject* mod) {
        subj = mod;
        subj->attach(this);
    }

    virtual void update() = 0;

protected:
    Subject* getSubject() {
        return subj;
    }
};

void Subject::notify() {
    for (unsigned int i = 0; i < observers.size(); i++) {
        observers[i]->update();
    }
}

class Young_ConcreteObserver : public Observer {
public:
    Young_ConcreteObserver(Subject* mod) : Observer(mod) {};

    void update() {
        bool scored = getSubject()->getScored();
        if (scored) {
            Serial.println ("mode will be inverted on the next loop iteration");
        }
    }

    void update(float new_h, float new_t, float old_h, float old_t){
        for(auto i : led) digitalWrite(i, led_state);

        if(abs(new_h - old_h) >= 1.0 && abs(new_t - old_t) >= 1.0){
            for(auto i : led) digitalWrite(i, !led_state);
        }
        else if(abs(new_h - old_h) >= 1.0){
            digitalWrite(D1, !led_state);
        }
        else if(abs(new_t - old_t) >= 1.0){
            digitalWrite(D2, !led_state);
        }
    }
};

Subject subj;
Young_ConcreteObserver youngObs1(&subj);

void setup() {
    Serial.begin(9600);
    for (auto i : led) pinMode(i, OUTPUT);
    for (auto i : led) digitalWrite(i, LOW);
    pinMode(D7, INPUT_PULLUP);
    dht.begin();
    h = t = dh = bt = 0;
    Blynk.begin(auth, ssid, pass);
    
}

void loop() {
    Blynk.run();
    butt = !digitalRead(D7);

    if (butt == 1 && butt_flag == 0 && millis() - last_press > rattling) {
        butt_flag = 1;
        last_press = millis();
    }

    if (butt == 0 && butt_flag == 1 && millis() - last_press > rattling) {
        butt_flag = 0;
        led_state = !led_state;
        subj.setScored(true);
        last_press = millis();
    }

    if (millis() - curr_time > timer) {
        dh = h;
        bt = t;

        h = dht.readHumidity();
        t = dht.readTemperature();

        if (isnan(h) || isnan(t)) {
            Serial.println("Read error");
            return;
        }
        Serial.print("humidity: ");
        Serial.print(h);
        Serial.print(" %\t");
        Serial.print("temperature: ");
        Serial.print(t);
        Serial.println(" *C ");
        Blynk.virtualWrite(V5, h);
        Blynk.virtualWrite(V6, t);

        youngObs1.update(h, t, dh, bt);
        curr_time = millis();
    }
}