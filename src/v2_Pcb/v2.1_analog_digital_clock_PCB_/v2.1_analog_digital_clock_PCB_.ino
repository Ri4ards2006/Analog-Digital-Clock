#include <RTClib.h>
/*1100 bytes ca! D: //Remember: ATmega-8.x, dies ist lediglich ein 8bit prozessor
(-16PU variante für 40PDIP mit 16bit I/O) //Selbst moderne Arduino Unos haben 16bit!
256-1 Werte pro byte! also Max /64Kb an speicher! Das programm muss möglich klein und
effizient laufen.
*/
//##Pin-Belegungs-Index###############################################//
// >Digital-Pins(1/0)[1->5,16->20,23->24]:
// > 1 : D1
// > 2 : D2
// > 3 : D3
// > 4 : D4
// > 5 : D5
// > 14: RxD !Fehler !Vlt zum programmieren benötigt
// > 15: TxD !Fehler !Vlt zum pryogrammieren benötigt
// > 16: D6
// > 17: D7
// > 18: D8
// > 19: D9
// > 20: D10
// > !21: SCL !Fehler !Bridged to Pin 22
// > !22: SDA !Fehler !Bridged to Pin 23
// > 23: D11 !Bridged to Pin 24
// > 24: D12 !Bridged to Pin 25
// >Analog-Pins(-1/0/+1)[35->40]:
// > !29: H !Fehler// > 35: H !Bridged to Pin 29
// > 36: M5
// > 37: M4
// > 38: M3
// > 39: M2
// > 40: M1
//##//
RTC_DS1307 RTC; //Neues RTC_DS1307 objekt als RTC
class Pinout{
public:
int x[8];
};
/* Inspieriert duch VMs und System Modellierungs Sprachen.Virtuelle Uhr mit Identischer Matrix
Simuliert
(In 444Bytes ;D). LEDs sind durch bits im Buffer repräsentiert. der Pin index wird in PINOUT
definiert. */
loop durch die bits im buffer.. wenn 1 = LEDx HIGH wenn 0 = LEDx LOW
class Zeit{
int zeitMinvTag = 1; // x von 1440 /24 /12 -1/12
class Minute{
Zeit* const owner; //owner is poiter zu Zeit
Minute(Zeit & owner): owner(&owner){};
// Minute braucht Zeit und owner von owner von welchem es Zeit erbt.
bool An = true;
bool buffer[5][12];
void Show(){
int y = int(ceil(((owner -> zeitMinvTag %60) -1) /5)); //Pointer zuweisung
int x = int(ceil((((owner -> zeitMinvTag %60) -1) %5)));//for(int i = 0;i<12;i++){ // int(ceil(((Zeit::zeitMinvTag %60) -1) /12))
// int(ceil((((Zeit::zeitMinvTag %60) -1) %12) -1))
switch(Zeit::Minute::buffer[x][y]){case true:digitalWrite((x+1)*(y+1),HIGH);break; case
false:digitalWrite((x+1)*(y+1),LOW);break;}
};
};
class Hour{
bool An = true;
bool buffer [12];
void Show(){
analogWrite(6,LOW);
for(int i = 0;i<12;i++){
switch(Zeit::Hour::buffer[i]){case true:digitalWrite(i,HIGH)
;break; case false:digitalWrite(i,LOW);break;}
};
};
};
};
void setup() {
int Pos = 13;
for(int x=Pos;x<sizeof(Pinout::x)+Pos+1;x++){
pinMode(x,OUTPUT);
};
// put your setup code here, to run once:
}
void loop() {
// put your main code here, to run repeatedly:
}