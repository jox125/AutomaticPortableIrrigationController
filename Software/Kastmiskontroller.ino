//*********************************************//
//*            LCD seadistus                  *//
//*********************************************//

//Lisa LCD teek.
#include <LiquidCrystal.h>

//seadista LiquidCrystal teek
const int rs = 53, en = 51, d4 = 49, d5 = 47, d6 = 45, d7 = 43;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Ajaviideteks vajalikud muutujad
// Ekraani taimeri eelmine salvestatud aeg
unsigned long ekraan_eelmineMillis = 0;
// Ekraani värskendamise intervall millisekundites (sek x 1000)
unsigned long ekraan_intervall = 500; //0,5 sek

//*********************************************//
//*            Nuppude seadistus              *//
//*********************************************//

//Lisa Nuppude teek.
#include <Button.h>
//seadista nuppude pin-id.
Button ules_nupp(2); //üles nupp ühendatud D2 ja GND vahele
Button alla_nupp(3); //alla nupp ühendatud D3 ja GND vahele
Button ok_nupp(4);   //ok nupp ühendatud D4 ja GND vahele
//Ajaviideteks vajalikud muutujad
// Nuppude taimeri eelmine salvestatud aeg
unsigned long ules_nupp_eelmineMillis = 0;
unsigned long alla_nupp_eelmineMillis = 0;
// nuppuvajutuste intervall millisekundites (sek x 1000)
unsigned long nupud_intervall = 1000; //1 sek

int arv = 0;
bool alla_nupp_vajutatud = false;
bool ules_nupp_vajutatud = false;

//*********************************************//
//*    Globaalsete muutujate deklareerimine   *//
//*********************************************//

//Niiskuse mõõtmiseks vajalikud muutujad

//Mõõdetud niiskus 
int niiskused[6] = {100,100,100,100,100,100};
int min_niiskus = 910;
int max_niiskus = 510;
//Vajalikud analoog sisendid
int N_andur [6] = {10,11,12,13,14,15};  //Niiskuse andurite pin-id A10-A15 
//Ajaviideteks vajalikud muutujad
// Niiskuse taimeri eelmine salvestatud aeg
unsigned long niiskus_eelmineMillis = 0;
// Niiskuse mõõtmise intervall millisekundites (sek x 1000)
unsigned long niiskus_intervall = 10000; //10 sek

//Kastmise alamprogrammi muutujad

//toimub kastmine bit
bool toimub_kastmine [6] = {false, false, false, false, false, false}; 
//releede pin-ide määramine
int releed [6] = {22,24,26,28,30,32}; //releede pin-id D22 - D32
//mulla niiskuse protsent, millest kuivema mulla korral hakatakse kastma
//ja millest niiskema mulla korral lõpetatakse kastmine
int min_niiskus_s2te [6] = {0,0,0,0,0,0}; 
int max_niiskus_s2te [6] = {90,90,90,90,90,90}; 
//Ajaviideteks vajalikud muutujad
// Kastmise alamprogrammi taimeri eelmine salvestatud aeg
unsigned long kastmine_eelmineMillis = 0;
//kastmise alamprogrammi intervall millisekundites (sek x 1000)
unsigned long kastmine_intervall = 1000; //1 sek

//kontroll alamprogrammi globaalsed muutujad.
int aktiivne_ekraan = 0;
int grupp = 1; //hetkel aktiivne grupp, mille seadeid muudetakse

void setup() {
//setup käsud täidetakse üks kord arduino käivitumisel
//Serial.begin(9600);

//Seadista nupud
ules_nupp.begin(); //käivita üles nupp jälgimine 
alla_nupp.begin(); //käivita alla nupp jälgimine
ok_nupp.begin();   //käivita ok nupp jälgimine

//Seadista LCD sümbolite arv ja read
  lcd.begin(20, 4);
  // Print a message to the LCD.
  lcd.print("Kastmis kontroll");
  delay (2000);
  lcd.clear();

//releede pin-ide konfigureerimine
for (int a=0; a<6; a++){
  pinMode(releed [a], OUTPUT);
  }
}

void loop() {
  // Programmi tsükliline osa
niiskus ();
kastmine ();
kontroll ();
}


void niiskus (){
/*
 * Niiskuse mõõtmine. Mõõdab iga määratud T_niiskus (ms) järel
 * gruppide kaupa mulla niiskuse ja salvestab tulemuse niiskused massiivi. 
 */  
  unsigned long hetkeMillis = millis();
  //aja intervalli möödumise kontroll 
  if(hetkeMillis - niiskus_eelmineMillis > niiskus_intervall) {
    niiskus_eelmineMillis = hetkeMillis;
    //kõik grupid 
    //Serial.print("Mulla niiskused: ");  
    for (int i = 0; i<6; i++){
      //kümne mõõtmise keskmine
      int keskmine = 0;
      for (int e=0; e<10; e++){
        keskmine = keskmine + analogRead(N_andur[i]);
        }
      keskmine = keskmine / 10;
      //mõõdetud niiskuse salvestamine massiivi
      niiskused [i] = map(keskmine, min_niiskus, max_niiskus, 0, 100);
      //Serial.print(niiskused[i]);
      //Serial.print(" ; ");
      }
      //Serial.println();
    }
    
}
void kastmine (){
/*
 * Kui eelmisest programmi käivitamisest on möödunud piisavalt aega, siis 
 * gruppide kaupa kontrollib, kui kastmist ei toimu, siis kui niiskus on 
 * allpool määratud piiri, kastmine sisse, kastmine bit tõeseks.   
 * Kui kastmine toimub, siis kui muld on piisavalt märg
 * kastmine välja, kastmine bit vääraks.
 */
unsigned long hetkeMillis = millis();
  //aja intervalli möödumise kontroll 
  if (hetkeMillis - kastmine_eelmineMillis > kastmine_intervall) {
    kastmine_eelmineMillis = hetkeMillis;
    //kõik grupid 
    //Serial.print("Kastmised: ");  
    for (int i = 0; i<6; i++){
      if (!toimub_kastmine[i]){ //kas ei toimu juba antud grupi kastmine
        if(niiskused[i]<min_niiskus_s2te[i]){ //kas mulla niiskus on väiksem etteantud min. sättest
          digitalWrite (releed[i], HIGH); //antud grupi kastmine sisse (relee blokkil led kustu)
          toimub_kastmine[i] = true; //antud grupi kastmine toimub bit tõeseks
          }        
        }
        else if (niiskused[i]>max_niiskus_s2te[i]){//kui toimub juba antud grupi kastmine ja 
                                                //muld on juba piisavalt märg, siis
          digitalWrite (releed[i], LOW); //antud grupi kastmine välja (relee blokkil led sees)
          toimub_kastmine[i] = false; //antud grupi kastmine toimub bit vääraks
          }
 
      //Serial.print(toimub_kastmine[i]);
      //Serial.print(" ; ");
      }
      //Serial.println();
    }
}

void kontroll (){
/*
 * Käivitab vastavalt selle ekraani alamprogrammi, milline on 
 * aktiivne_ekraan muutujaga aktiivseks määratud
 */
switch (aktiivne_ekraan){
  case 0: //avaekraan
    avaekraan();
    break;
  case 1: //avaekraan 2
    ekraan_2();
    break;     
  case 2: //gruppide seadistuse vaatamine
    Gruppide_seaded();
    break;
  case 3: //min niiskuse seadistus
    Min_seade_muutmine();
    break;
  case 4: //max niiskuse seadistus
    Max_seade_muutmine();
    break;
  default:
    avaekraan();
    break;
  }
}


void avaekraan (){
/*
 * nupu vajutuste kontroll ja alamprogrammide käivitamine
 * ekraani värskendus ettemääratud intervalliga
 */
  if (ules_nupp.pressed()||alla_nupp.pressed()){
    aktiivne_ekraan = 1; //ekraan_2
    }
  if (ok_nupp.pressed()){
    aktiivne_ekraan = 2; //Gruppide seade ekraan
    }
//aja intervalli möödumise kontroll ja ekraani värskendamine 
  unsigned long hetkeMillis = millis();
  if(hetkeMillis - ekraan_eelmineMillis > ekraan_intervall) {
    ekraan_eelmineMillis = hetkeMillis;
  lcd.clear();
  //LCD rida 1.
  lcd.setCursor(0, 0);
  lcd.print("HETKE NIISKUSED:");
  //LCD rida 2: niiskused gruppides 1 ja 2
  for (int i=0; i<2; i++){
      lcd.setCursor(i*10, 1);
      lcd.print("Gr.");
      lcd.print(i+1);
      lcd.print(":");
      lcd.print(niiskused[i]);
      lcd.print("%");
    }
  //LCD rida 3: niiskused gruppides 3 ja 4
  for (int i=2; i<4; i++){
      lcd.setCursor((i-2)*10, 2);
      lcd.print("Gr.");
      lcd.print(i+1);
      lcd.print(":");
      lcd.print(niiskused[i]);
      lcd.print("%");
    } 
  //LCD rida 4: niiskused gruppides 5 ja 6
  for (int i=4; i<6; i++){
      lcd.setCursor((i-4)*10, 3);
      lcd.print("Gr.");
      lcd.print(i+1);
      lcd.print(":");
      lcd.print(niiskused[i]);
      lcd.print("%");
    }    
  }
}

void ekraan_2 (){
/*
 * nupu vajutuste kontroll ja vastavalt aktiivse ekraani määramine
 * ekraani värskendus ettemääratud intervalliga
 * ekraanil ridade kaupa kuvada gruppide kastmine, mitte kastmine
 */
  if (ules_nupp.pressed()||alla_nupp.pressed()){
    aktiivne_ekraan = 0; //avaekraan
    }
  if (ok_nupp.pressed()){
    aktiivne_ekraan = 2; //Gruppide seadete ekraan
    }
//aja intervalli möödumise kontroll ja ekraani värskendamine 
  unsigned long hetkeMillis = millis();
  if(hetkeMillis - ekraan_eelmineMillis > ekraan_intervall) {
    ekraan_eelmineMillis = hetkeMillis;
  lcd.clear();
  //LCD rida 1.
  lcd.setCursor(0, 0);
  lcd.print("KAS TOIMUB KASTMINE?");
  //LCD rida 2: niiskused gruppides 1 ja 2
  for (int i=0; i<2; i++){
      lcd.setCursor(i*10, 1);
      lcd.print("Gr.");
      lcd.print(i+1);
      lcd.print(":");
      if (toimub_kastmine[i]){
        lcd.print(" JAH");
        }
      else {
        lcd.print("  EI");
        }
    }
  //LCD rida 3: niiskused gruppides 3 ja 4
  for (int i=2; i<4; i++){
      lcd.setCursor((i-2)*10, 2);
      lcd.print("Gr.");
      lcd.print(i+1);
      lcd.print(":");
      if (toimub_kastmine[i]){
        lcd.print(" JAH");
        }
      else {
        lcd.print("  EI");
        }
    } 
  //LCD rida 4: niiskused gruppides 5 ja 6
  for (int i=4; i<6; i++){
      lcd.setCursor((i-4)*10, 3);
      lcd.print("Gr.");
      lcd.print(i+1);
      lcd.print(":");
      if (toimub_kastmine[i]){
        lcd.print(" JAH");
        }
      else {
        lcd.print("  EI");
        }
    }    
  }
}

  
void Gruppide_seaded (){
/*
 * nupu vajutuste kontroll ja aktiivsete ekraanide määramine
 * ekraani värskendus ettemääratud intervalliga
 */
  if (ok_nupp.pressed()){
    aktiivne_ekraan = 3; //min seade muutmis ekraan
    }
  if (ules_nupp.pressed()){//aktiivne Grupp järgmine
    if(grupp < 6){
      grupp++; 
      }
      else {
        grupp = 1; //esimesest grupist üles uuesti
        }  
    }
  if (alla_nupp.pressed()){
   if (grupp > 1){
      grupp--;
      }
      else{
        grupp = 6; //6 grupist alla uuesti
        }  
    }
    
//aja intervalli möödumise kontroll ja ekraani värskendamine 
  unsigned long hetkeMillis = millis();
  if(hetkeMillis - ekraan_eelmineMillis > ekraan_intervall) {
    ekraan_eelmineMillis = hetkeMillis;
  lcd.clear();
  //LCD rida 1.
  lcd.setCursor(0, 0);
  lcd.print("G");
  lcd.print(grupp);
  lcd.print(" KASTMISE SEADED:");
  //LCD rida 2: miinimum niiskuse säte
  lcd.setCursor(0, 1);
  lcd.print(" ALGUS: ");
  lcd.print(min_niiskus_s2te[grupp-1]);
  lcd.print("%");
  //LCD rida 3: maksimum niiskuse säte
  lcd.setCursor(0, 2);
  lcd.print("  LOPP: ");
  lcd.print(max_niiskus_s2te[grupp-1]); 
  lcd.print("%");
  //LCD rida 4: tühi rida
  lcd.setCursor(0, 3);
  } 
}

void Min_seade_muutmine(){
/*
 * nupu vajutuste kontroll, sätete muutmine ja aktiivsete ekraani määramine
 * ekraani värskendus ettemääratud intervalliga
 */
 unsigned long hetkeMillis = millis();

  if (ok_nupp.pressed()){
    lcd.noBlink(); //kursori vilkumine OFF
    aktiivne_ekraan = 4; //max seade muutmis ekraan 
    }
//Üles nupu vajutusel lisab aktiivse grupi min sättele 1, nupu hoidmisel lisab järjest rohkem 
  if (!ules_nupp_vajutatud && ules_nupp.pressed()){ //kui üles nupp vajutatud esimest korda
    min_niiskus_s2te[grupp-1] += 1; //lisab Min. niiskuse sättele +1 
    ules_nupp_vajutatud = true;
    
    }
  if (ules_nupp.released()){ //kui üles nupp vabastatud
      ules_nupp_vajutatud = false;
      arv = 0; // arvu väärtus tagasi 0-ks.
    }    
  if(hetkeMillis - ules_nupp_eelmineMillis > nupud_intervall) {
    ules_nupp_eelmineMillis = hetkeMillis;
    if (ules_nupp_vajutatud && ules_nupp.read() == Button :: PRESSED){ //kui nupp aja intervalli möödumisel ikka veel vajutatud
      arv += 1; //suurendab iga kord arvu 1 võrra
      min_niiskus_s2te[grupp-1] += arv; //lisab Min. niiskuse sättele arv väärtuse
     }
  }  
  
//Alla nupu vajutusel lisab min sättele -1, nupu hoidmisel lisab järjest vähem 
  if (!alla_nupp_vajutatud && alla_nupp.pressed()){ //kui alla nupp vajutatud esimest korda
    min_niiskus_s2te[grupp-1] -= 1; //lisab Min. niiskuse sättele -1 
    alla_nupp_vajutatud = true;
    
    }
  if (alla_nupp.released()){ //kui alla nupp vabastatud
      alla_nupp_vajutatud = false;
      arv = 0; // arvu väärtus tagasi 0-ks.
    }    
  if(hetkeMillis - alla_nupp_eelmineMillis > nupud_intervall) {
    alla_nupp_eelmineMillis = hetkeMillis;
    if (alla_nupp_vajutatud && alla_nupp.read() == Button :: PRESSED){ //kui nupp aja intervalli möödumisel ikka veel vajutatud
      arv += 1; //suurendab iga kord arvu 1 võrra
      min_niiskus_s2te[grupp-1] -= arv; //vähendab Min. niiskuse sättet arvu väärtuse võrra
     }
  }
    
//aja intervalli möödumise kontroll ja ekraani värskendamine 
  if(hetkeMillis - ekraan_eelmineMillis > ekraan_intervall) {
    ekraan_eelmineMillis = hetkeMillis;
  lcd.clear();
  //LCD rida 1.
  lcd.setCursor(0, 0);
  lcd.print("G");
  lcd.print(grupp);
  lcd.print(" KASTMISE SEADED:");
  //LCD rida 2: miinimum niiskuse säte
  lcd.setCursor(0, 1);
  lcd.print(" ALGUS: ");
  lcd.print(min_niiskus_s2te[grupp-1]);
  lcd.print("%");
  //LCD rida 3: maksimum niiskuse säte
  lcd.setCursor(0, 2);
  lcd.print("  LOPP: ");
  lcd.print(max_niiskus_s2te[grupp-1]); 
  lcd.print("%");
  //LCD rida 4: tühi rida
  lcd.setCursor(0, 3);

  //kursor teksti ALGUS ette vikuma
  lcd.setCursor(0,1);
  lcd.blink(); 
  } 
}

void Max_seade_muutmine(){
/*
 * nupu vajutuste kontroll, sätete muutmine ja aktiivsete ekraanide määramine
 * ekraani värskendus ettemääratud intervalliga
 */
 unsigned long hetkeMillis = millis();

  if (ok_nupp.pressed()){
    lcd.noBlink();
    aktiivne_ekraan = 0; //tagasi avaekraanile 
    }
//Üles nupu vajutusel lisab aktiivse gruppi max sättele 1, nupu hoidmisel lisab järjest rohkem 
  if (!ules_nupp_vajutatud && ules_nupp.pressed()){ //kui üles nupp vajutatud esimest korda
    max_niiskus_s2te[grupp-1] += 1; //lisab Max. niiskuse sättele +1 
    ules_nupp_vajutatud = true;
    
    }
  if (ules_nupp.released()){ //kui üles nupp vabastatud
      ules_nupp_vajutatud = false;
      arv = 0; // arvu väärtus tagasi 0-ks.
    }    
  if(hetkeMillis - ules_nupp_eelmineMillis > nupud_intervall) {
    ules_nupp_eelmineMillis = hetkeMillis;
    if (ules_nupp_vajutatud && ules_nupp.read() == Button :: PRESSED){ //kui nupp aja intervalli möödumisel ikka veel vajutatud
      arv += 1; //suurendab iga kord arvu 1 võrra
      max_niiskus_s2te[grupp-1] += arv; //lisab Max. niiskuse sättele arv väärtuse
     }
  }  
  
//Alla nupu vajutusel lisab aktiivse gruppi max sättele -1, nupu hoidmisel lisab järjest vähem 
  if (!alla_nupp_vajutatud && alla_nupp.pressed()){ //kui alla nupp vajutatud esimest korda
    max_niiskus_s2te[grupp-1] -= 1; //lisab Max. niiskuse sättele -1 
    alla_nupp_vajutatud = true;
    
    }
  if (alla_nupp.released()){ //kui alla nupp vabastatud
      alla_nupp_vajutatud = false;
      arv = 0; // arvu väärtus tagasi 0-ks.
    }    
  if(hetkeMillis - alla_nupp_eelmineMillis > nupud_intervall) {
    alla_nupp_eelmineMillis = hetkeMillis;
    if (alla_nupp_vajutatud && alla_nupp.read() == Button :: PRESSED){ //kui nupp aja intervalli möödumisel ikka veel vajutatud
      arv += 1; //suurendab iga kord arvu 1 võrra
      max_niiskus_s2te[grupp-1] -= arv; //vähendab Min. niiskuse sättet arvu väärtuse võrra
     }
  }
    
//aja intervalli möödumise kontroll ja ekraani värskendamine 
  if(hetkeMillis - ekraan_eelmineMillis > ekraan_intervall) {
    ekraan_eelmineMillis = hetkeMillis;
  lcd.clear();
  //LCD rida 1.
  lcd.setCursor(0, 0);
  lcd.print("G");
  lcd.print(grupp);
  lcd.print(" KASTMISE SEADED:");
  //LCD rida 2: miinimum niiskuse säte
  lcd.setCursor(0, 1);
  lcd.print(" ALGUS: ");
  lcd.print(min_niiskus_s2te[grupp-1]);
  lcd.print("%");
  //LCD rida 3: maksimum niiskuse säte
  lcd.setCursor(0, 2);
  lcd.print("  LOPP: ");
  lcd.print(max_niiskus_s2te[grupp-1]); 
  lcd.print("%");
  //LCD rida 4: tühi rida
  lcd.setCursor(0, 3);

  //kursor teksti LOPP ette vikuma
  lcd.setCursor(0,2);
  lcd.blink(); 
  } 
}
