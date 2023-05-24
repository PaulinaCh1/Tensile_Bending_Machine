void tensileTest()
{
  Serial.println("Tensile test");

  bool isWorking = true;
  int i = 0;

  while(isWorking) // loop for tensile test
  {

  digitalWrite(IN1_PIN, HIGH); // control motor A spins clockwise
  digitalWrite(IN3_PIN, LOW);  // control motor A spins clockwise
  digitalWrite(IN2_PIN, LOW); // control motor A spins clockwise
  digitalWrite(IN4_PIN, HIGH);  // control motor A spins clockwise

  for (int speed = 0; speed <= 255; speed++) {
    analogWrite(ENA_PIN, speed); // control the speed
    analogWrite(ENB_PIN, speed); // control the speed
    }
    
   // Read the current state of CLK
      currentStateCLK = digitalRead(CLK);


  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      counter --;
      currentDir ="CCW";
     }
    else {
      // Encoder is rotating CW so increment
      counter ++;
      currentDir ="CW";
      }

    static boolean newDataReady = 0;
    const int serialPrintInterval = 0;
   if (LoadCell.update()) newDataReady = true;
   if(newDataReady) {
    if (counter % 10 == 0)
      {
        firstvalue = LoadCell.getData();
        newDataReady = 0;    
      }
       if (counter % 5 == 0){
         Serial.print("Tick Counter: ");
         Serial.println(counter);
          
            secondvalue = LoadCell.getData();
            Serial.print("Load Cell Value:  ");
            Serial.println(secondvalue);
            newDataReady = 0;   
       } 

        if (firsttime == false){

        float difference = abs(firstvalue - secondvalue);

        while (difference > 100){
           Serial.println(difference);
           digitalWrite(IN1_PIN, LOW);
           digitalWrite(IN2_PIN, LOW);

          // Set Motor B anticlockwise
          digitalWrite(IN3_PIN, LOW);
          digitalWrite(IN4_PIN, LOW);
          Serial.println("Test END");
          isWorking = false;  
        }                   
    }

      firsttime = false;
      }}

    // Remember last CLK state
    lastStateCLK = currentStateCLK;
  }

  tensileTestStarted = false;
  tensileTestButtonPressed = false;
}
