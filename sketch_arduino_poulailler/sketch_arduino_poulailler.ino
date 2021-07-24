// Porte poulailler automatique
// Baptiste DELPHIN 2021

// Motor A&B : 10 & 11
const int motor_pin_A = 10;
const int motor_pin_B = 11;

int motor_speed_up = 255;
int motor_speed_down = 100;

// Endstop 1 : 8
// Endstop 2 : 7
const int endstop_top_pin = 8;
const int endstop_bottom_pin = 7;

// LED : 6
int diag_led_pin = 6;
unsigned long led_millis = 0;
int led_interval = 500;
bool led_blinking = false;
int led_status = LOW;

// DIP1 : 5
// DIP2 : 4
int dip1_pin = 5;
int dip2_pin = 4;

// Photores : A0

// Door Button (in_pullup) : 3
int door_button_pin = 3;
int door_button_status;
int door_button_last_status = HIGH;

bool door_opened = false;
bool lux_override = false;
bool door_timer_launched = false;

bool bottom_endstop = true;
bool top_endstop = false;

int photores_value = 0;

int seuil_photores = 200;

// duree minimum du jour en hiver : 8h = 3600*8*1000 = 28 800 000 ms
// 1h = 3600*1000 = 3 600 000 ms
unsigned long door_millis = 0;
unsigned long door_interval = 3600000;

// combinaisons DIP switchs :
// DIP1 - DIP2 : action
//  0   -  0   : normal
//  1   -  0   : +1h matin
//  0   -  1   : +1h soir
//  1   -  1   : +1h matin et +1h soir

void setup() 
{
    Serial.begin(9600);
    Serial.println("Poulailler Automatique Arduino");

    pinMode(motor_pin_A, OUTPUT);
    pinMode(motor_pin_B, OUTPUT);

    pinMode(door_button_pin, INPUT_PULLUP);

    pinMode(dip1_pin, INPUT_PULLUP);
    pinMode(dip2_pin, INPUT_PULLUP);
    
}

void loop() 
{
    // photores value
    photores_value = analogRead(A0);
    //Serial.println("Photores Value :");
    //Serial.println(photores_value);  

    // si lux_override est à false (on a pas ouvert nanuelement)
    if(!lux_override and !door_timer_launched)
    {
        if(photores_value < seuil_photores and door_opened)
        {
            // il fait noir et la porte est ouverte, on ferme la porte

            if(digitalRead(dip2_pin) == LOW)
            {
                // le DIP2 est à ON, on décalle la fermeture d'une heure
                door_millis = millis();
                door_timer_launched = true;
            }
            else
            {
                close_door();
            }   
        }
        else if(photores_value >= seuil_photores and !door_opened)
        {
            // il fait jour et la porte est fermee, on ouvre la porte
            
            if(digitalRead(dip1_pin) == LOW)
            {
                // le DIP1 est à ON, on décalle l'ouverture d'une heure
                door_millis = millis();
                door_timer_launched = true;
            }
            else
            {
                open_door();
            }
        }
    }

    if(door_timer_launched)
    {
        unsigned long currentMillis = millis();
        if((unsigned long)(currentMillis - door_millis) >= door_interval)
        {
            if(door_opened)
                close_door();
            else
                open_door();
            door_timer_launched = false;
        }
    }

    // lecture bouton ouverture manuelle
    door_button_status = digitalRead(door_button_pin);
    
    if(door_button_status != door_button_last_status)
    {
        door_button_last_status = door_button_status;

        if(door_button_status == LOW)
        {
            // ouvrir ou fermer porte manuellement
            if(door_opened)
            {
                // close door          
                close_door();    
            }
            else
            {
                // open door
                open_door();
            }
            
            // on passe lux_override à true
            lux_override = true;
        }  
    }

    if(door_opened and digitalRead(endstop_top_pin) != LOW)
    {
        open_door();
    }

    // lecture endstop
    //Serial.println(digitalRead(endstop_top_pin));


    delay(250);
}

void open_door()
{
    Serial.println("On ouvre la porte ...");
    blink_led(true);
    while(digitalRead(endstop_top_pin) != LOW)
    {
        analogWrite(motor_pin_A, motor_speed_up);
        analogWrite(motor_pin_B, 0);
        blink_led(true);
    }
    analogWrite(motor_pin_A, 0);
    analogWrite(motor_pin_B, 0);
    Serial.println("Porte ouverte.");
    blink_led(false);
    door_opened = true;
}

void close_door()
{
    Serial.println("On ferme la porte ...");
    blink_led(true);
    while(digitalRead(endstop_bottom_pin) != LOW)
    {
        analogWrite(motor_pin_A, 0);
        analogWrite(motor_pin_B, motor_speed_down);
        blink_led(true);
    }
    analogWrite(motor_pin_A, 0);
    analogWrite(motor_pin_B, 0);
    Serial.println("Porte fermee.");
    blink_led(false);
    door_opened = false;
}

void blink_led(bool status)
{    
    if (status and !led_blinking)
    {
        // start millis
        led_millis = millis();
        led_blinking = true;
        led_status = HIGH;   
    }
    else if(status and led_blinking)
    {
        // blink led
        unsigned long currentMillis = millis();
        if((unsigned long)(currentMillis - led_millis) >= led_interval)
        {
            if(led_status == HIGH)
                led_status = LOW;
            else
                led_status = HIGH;
            led_millis = currentMillis;
        }
    }
    else if (!status and led_blinking)
    {
        // stop millis & stop led blinking
        led_blinking = false;
        led_status = LOW;      
    }
    digitalWrite(diag_led_pin, led_status);
}
