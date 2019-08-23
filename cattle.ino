 #include <SoftwareSerial.h>

SoftwareSerial mySerial(9, 10);

float temp;
int tempPin = 1;
int sensor_pin = 0;                

int led_pin = 13;                  

volatile int heart_rate;          

volatile int analog_data;              

volatile int time_between_beats = 600;            

volatile boolean pulse_signal = false;    

volatile int beat[10];         //heartbeat values will be sotred in this array    

volatile int peak_value = 512;          

volatile int trough_value = 512;        

volatile int thresh = 525;              

volatile int amplitude = 100;                 

volatile boolean first_heartpulse = true;      

volatile boolean second_heartpulse = false;    

volatile unsigned long samplecounter = 0;   

volatile unsigned long lastBeatTime = 0;

void interruptSetup();

void SendMessage()
{
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\"+918428566628\"\r"); // Replace x with mobile number
  delay(1000);
  mySerial.println("MASTITIS DETECTED");// The SMS text you want to send
  delay(100);
   mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}


 void RecieveMessage()
{
  mySerial.println("AT+CNMI=2,2,0,0,0"); // AT Command to receive a live SMS
  delay(1000);
 }
void setup()
{
Serial.begin(9600);
interruptSetup();                  
}
float ec[]={4.1,4.3,4.2,5,5.7,5.9,6,4.1,4.5,4.9,3.9,3.6,3.5,5.1,4.3,4.6,4.1,4,4,4,4.8,5};
int i;
float c;
void loop()
{
for(i=0;i<23;i++)
{
temp = analogRead(tempPin);
temp = temp * 0.48828125;
Serial.print("TEMPRATURE = ");
Serial.print(temp);
Serial.print("*C");
Serial.println();
delay(1000);
Serial.print("BPM: ");
Serial.println(heart_rate);
delay(200); 
c=ec[i];
Serial.print("EC:");
Serial.println(c);
if((temp>38 && ec[i]>5.5)|| c>5.5)
{
Serial.println();
Serial.println("CHECK MASTITIS");
}
if((temp>38 && c<4) || c<4)
{
Serial.println();
Serial.println("READY TO INSEMINATE");
}
if((heart_rate>230 && temp>38)|| c==4)
{
Serial.println();
Serial.println("Calving in 6 Hours");
}
if(temp<38 && heart_rate<230 && c<5.5 && c>4) 
{
Serial.println();
Serial.println("NORMAL"); 
}
else
Serial.println();
}
if (Serial.available()>0)
switch(Serial.read())
{
    case 's':
      SendMessage();
      break;
    case 'r':
      RecieveMessage();
      break;
  }
}
void interruptSetup()
{    

  TCCR2A = 0x02;  // This will disable the PWM on pin 3 and 11

  OCR2A = 0X7C;   // This will set the top of count to 124 for the 500Hz sample rate

  TCCR2B = 0x06;  // DON'T FORCE COMPARE, 256 PRESCALER

  TIMSK2 = 0x02;  // This will enable interrupt on match between OCR2A and Timer

  sei();          // This will make sure that the global interrupts are enable

}
ISR(TIMER2_COMPA_vect)
{ 
  cli();                                     
  analog_data = analogRead(sensor_pin);            
  samplecounter += 2;                        
  int N = samplecounter - lastBeatTime;      
  if(analog_data < thresh && N > (time_between_beats/5)*3)

    {     

      if (analog_data < trough_value)

      {                       

        trough_value = analog_data;

      }

    }
if(analog_data > thresh && analog_data > peak_value)

    {        

      peak_value = analog_data;

    }                          
if (N > 250)

  {                            

    if ( (analog_data > thresh) && (pulse_signal == false) && (N > (time_between_beats/5)*3) )

      {       

        pulse_signal = true;          

        digitalWrite(led_pin,HIGH);

        time_between_beats = samplecounter - lastBeatTime;

        lastBeatTime = samplecounter;     



       if(second_heartpulse)

        {                        

          second_heartpulse = false;   

          for(int i=0; i<=9; i++)    

          {            

            beat[i] = time_between_beats; //Filling the array with the heart beat values                    

          }

        }


        if(first_heartpulse)

        {                        

          first_heartpulse = false;

          second_heartpulse = true;

          sei();            

          return;           

        }  


      word runningTotal = 0;  


      for(int i=0; i<=8; i++)

        {               

          beat[i] = beat[i+1];

          runningTotal += beat[i];

        }


      beat[9] = time_between_beats;             

      runningTotal += beat[9];   

      runningTotal /= 10;        

      heart_rate = 60000/runningTotal;

    }                      

  }
if (analog_data < thresh && pulse_signal == true)

    {  

      digitalWrite(led_pin,LOW); 

      pulse_signal = false;             

      amplitude = peak_value - trough_value;

      thresh = amplitude/2 + trough_value; 

      peak_value = thresh;           

      trough_value = thresh;

    }

if (N > 2500)

    {                          

      thresh = 512;                     

      peak_value = 512;                 

      trough_value = 512;               

      lastBeatTime = samplecounter;     

      first_heartpulse = true;                 

      second_heartpulse = false;               

    }
sei();                                
}















 
