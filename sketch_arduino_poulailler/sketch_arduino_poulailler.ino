// Porte poulailler automatique
// Baptiste DELPHIN 2021

// OLD VERSION WITH L9110s
// Motor A&B : 10 & 11
//const int motor_pin_A = 10;
//const int motor_pin_B = 11;

// NEW L298N VERSION
const int motor_enable_pin = 10;
const int motor_pin_1 = 11;
const int motor_pin_2 = 12;

int motor_speed_up = 255;
int motor_speed_down = 100;

// Endstop 1 : 8
// Endstop 2 : 7
const int endstop_top_pin = 8;
const int endstop_bottom_pin = 7;

unsigned long closing_millis = 0;
int closing_interval = 2500;
bool error_closing = false;

bool retry_countdown_started = false;
unsigned long retry_millis = 0;
long retry_interval = 600000;

// LED : 6
int diag_led_pin = 6;
unsigned long led_millis = 0;
int led_interval = 250;
bool led_blinking = false;
int led_status = LOW;

// DIP1 : 5
// DIP2 : 4
int dip1_pin = 5;
int dip2_pin = 4;

bool sun_risen = false;

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

    pinMode(motor_enable_pin, OUTPUT);
    pinMode(motor_pin_1, OUTPUT);
    pinMode(motor_pin_2, OUTPUT);

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
    if(!lux_override and !door_timer_launched and !error_closing)
    {
        if(photores_value < seuil_photores and door_opened)
        {
            // il fait noir et la porte est ouverte, on ferme la porte
            sun_risen = false;

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
            sun_risen = true;
            
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
            lux_override = false;
        }
    }

    if(lux_override and !error_closing)
    {
        if(sun_risen and door_opened)
        {
            // la porte a ete ouverte quand il faisait jour
            if(photores_value < seuil_photores)
            {
                // il fait noir et la porte est ouverte, on ferme la porte
                sun_risen = false;
    
                if(digitalRead(dip2_pin) == LOW)
                {
                    // le DIP2 est à ON, on décalle la fermeture d'une heure
                    door_millis = millis();
                    door_timer_launched = true;
                }
                else
                {
                    close_door();
                    lux_override = false;
                }   
            }
        }
        else if(!sun_risen and door_opened)
        {
            // la porte a ete ouverte quand il faisait noir
            // on ne la referme qu'au PROCHAIN coucher de soleil
            if(photores_value < seuil_photores and sun_risen)
            {
                // il fait noir et la porte est ouverte, on ferme la porte
                sun_risen = false;
    
                if(digitalRead(dip2_pin) == LOW)
                {
                    // le DIP2 est à ON, on décalle la fermeture d'une heure
                    door_millis = millis();
                    door_timer_launched = true;
                }
                else
                {
                    close_door();
                    lux_override = false;
                }   
            }
            else if(photores_value >= seuil_photores)
            {
                sun_risen = true;
            }
        }
        else if(sun_risen and !door_opened)
        {
            // la porte a ete fermee quand il faisait jour
            // on ne l'ouvre qu'au PROCHAIN lever de soleil
            if(photores_value >= seuil_photores and !sun_risen)
            {
                // il fait jour et la porte est fermee, on ouvre la porte
                sun_risen = true;
                
                if(digitalRead(dip1_pin) == LOW)
                {
                    // le DIP1 est à ON, on décalle l'ouverture d'une heure
                    door_millis = millis();
                    door_timer_launched = true;
                }
                else
                {
                    open_door();
                    lux_override = false;
                }
            }
            else if(photores_value < seuil_photores)
            {
                sun_risen = false;
            }
        }
        else if(!sun_risen and !door_opened)
        {
            // la porte a ete fermee quand il faisait nuit
            if(photores_value >= seuil_photores)
            {
                // il fait jour et la porte est fermee, on ouvre la porte
                sun_risen = true;
                
                if(digitalRead(dip1_pin) == LOW)
                {
                    // le DIP1 est à ON, on décalle l'ouverture d'une heure
                    door_millis = millis();
                    door_timer_launched = true;
                }
                else
                {
                    open_door();
                    lux_override = false;
                }
            }
        }
    }

    if(error_closing)
    {
        if(retry_countdown_started)
        {
            unsigned long currentRetryMillis = millis();
            if((unsigned long)(currentRetryMillis - retry_millis) >= retry_interval)
            {
                // retry countdown ended, lets retry closing door
                retry_countdown_started = false;
                error_closing = false;
                close_door();
            }
        }
        else
        {
            retry_millis = millis();
            retry_countdown_started = true;
        }
    }

    // lecture bouton ouverture manuelle
    door_button_status = digitalRead(door_button_pin);
    
    if(door_button_status != door_button_last_status)
    {
        door_button_last_status = door_button_status;

        // si le chrono ouverture/fermeture etait lance, on l'arrete
        door_timer_launched = false;

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
        //analogWrite(motor_pin_A, motor_speed_up);
        //analogWrite(motor_pin_B, 0);
        digitalWrite(motor_pin_1, HIGH); 
        digitalWrite(motor_pin_2, LOW);
        analogWrite(motor_enable_pin, motor_speed_up);
        blink_led(true);
    }
    //analogWrite(motor_pin_A, 0);
    //analogWrite(motor_pin_B, 0);
    digitalWrite(motor_pin_1, HIGH); 
    digitalWrite(motor_pin_2, HIGH);
    Serial.println("Porte ouverte.");
    blink_led(false);
    door_opened = true;
}

void close_door()
{
    closing_millis = millis();
    Serial.println("On ferme la porte ...");
    blink_led(true);
    while(digitalRead(endstop_bottom_pin) != LOW)
    {
        //analogWrite(motor_pin_A, 0);
        //analogWrite(motor_pin_B, motor_speed_down);
        digitalWrite(motor_pin_1, LOW); 
        digitalWrite(motor_pin_2, HIGH);
        analogWrite(motor_enable_pin, motor_speed_down);
        blink_led(true);

        unsigned long currentCloseMillis = millis();
        if((unsigned long)(currentCloseMillis - closing_millis) >= closing_interval)
        {
            // la porte a mis plus de 1000ms a se fermer
            open_door();
            error_closing = true;
            digitalWrite(diag_led_pin, HIGH);
            return;
        }
    }
    //analogWrite(motor_pin_A, 0);
    //analogWrite(motor_pin_B, 0);
    digitalWrite(motor_pin_1, HIGH); 
    digitalWrite(motor_pin_2, HIGH);
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
