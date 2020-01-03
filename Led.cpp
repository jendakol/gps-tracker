#include <Arduino.h>

enum State {
    On, Off, BlinkingSlow, BlinkingFast
};

class Led {
public:
    explicit Led(String name, int pin);

    void display(unsigned int drawPhase);

    void setState(State state);

private:
    String name_;
    int pin_;
    State state_;

    void setOn() { digitalWrite(pin_, LOW); }

    void setOff() { digitalWrite(pin_, HIGH); }
};

void Led::display(unsigned int drawPhase) {

    switch (state_) {
        case On:
            setOn();
            break;

        case Off:
            setOff();
            break;

        case BlinkingSlow:
            switch (drawPhase) {
                case 0:
//                    Serial.print(name_);Serial.println(" SLOW -> on");
                    setOn();
                    break;
                case 2:
//                    Serial.print(name_);Serial.println(" SLOW -> off");
                    setOff();
                    break;
                default:
                    break;
            }
            break;

        case BlinkingFast:
            switch (drawPhase) {
                case 0:
                case 2:
//                    Serial.print(name_);Serial.println(" FAST -> on");
                    setOn();
                    break;
                case 1:
                case 3:
//                    Serial.print(name_);Serial.println(" FAST -> off");
                    setOff();
                    break;
                default:
                    break;
            }
            break;
    }
    delay(5);
};

void Led::setState(State state) {
    state_ = state;
//    Serial.print("Led ");
//    Serial.print(name_);
//    Serial.print(" to ");
//    Serial.println(state);
}

Led::Led(String name, int pin) {
    name_ = name;
    pin_ = pin;
    state_ = Off;
};
