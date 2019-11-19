const int chaveMag = 4; // porta 4
const float pi = 3.14159; //definindo pi 
const float raio = 1; // raio em metros
float ti =0;
float tf=0;
float deltaT=0;
float periodo=0;
float odom = 0; // marca odometro


void setup() {
  // put your setup code here, to run once:
  pinMode(chaveMag,INPUT);// registra chave magnetica
  Serial.begin(9600); // inicializa porta serial
  //falta programar as saídas
  tf = millis();
}

void loop() {

  //Velocimetro
  if(digitalRead(chaveMag) == HIGH){
    ti = tf;
    tf = millis();//tempo final em milisegundos
    while(digitalRead(chaveMag) == HIGH){
      //ESPERA ENQUANTO HIGH
    }  
  } 

  
  deltaT = (tf-ti)/1000; //período da volta em seg

  //Serial.print("Diferença de Tempo: ");  
  //Serial.println(deltaT,DEC);
  
  Serial.print("Rotacao em Hertz: "); 
  periodo=1/deltaT; 
  Serial.println(periodo,DEC); //printa a rotacao em Hz
  float omega = (2*pi)/deltaT;//velocidade angular em rad
  float vel = omega*raio*3.6; // calcula velocidade em km/h

  //Odometro
  odom = odom +(2*pi*raio)*0.001; //atualiza o odometro a cada loop em km
  Serial.print("Odometro:");
  Serial.println(odom,DEC); //printa odometro 
  
   while(digitalRead(chaveMag) == LOW){
      //ESPERA ENQUANTO LOW
    } 

}

