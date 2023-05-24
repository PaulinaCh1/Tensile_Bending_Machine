void joystick()
{
   Serial.println("joystick");
   int yAxis = analogRead(A8);

  // Y-axis used to move platform up/down
  if (yAxis < 470) {

    // Set Motor A clockwise
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);

    // Set Motor B clockwise
    digitalWrite(IN3_PIN, HIGH);
    digitalWrite(IN4_PIN, LOW);

    // Convert the declining Y-axis readings for going backward from 470 to 0 into 0 to 255 value for the PWM signal for increasing the motor speed
    motorSpeedA = map(yAxis, 470, 0, 0, 255);
    motorSpeedB = map(yAxis, 470, 0, 0, 255);
  }
  else if (yAxis > 550) {

    // Set Motor A anticlockwise
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, HIGH);

    // Set Motor B anticlockwise
    digitalWrite(IN3_PIN, LOW);
    digitalWrite(IN4_PIN, HIGH);

    // Convert the increasing Y-axis readings for going clockwise from 550 to 1023 into 0 to 255 value for the PWM signal for increasing the motor speed
    motorSpeedA = map(yAxis, 550, 1023, 0, 255);
    motorSpeedB = map(yAxis, 550, 1023, 0, 255);
  }

  // If joystick stays in middle the motors are not moving
  else {
    motorSpeedA = 0;
    motorSpeedB = 0;
  }

  analogWrite(ENA_PIN, motorSpeedA); // Send PWM signal to motor A
  analogWrite(ENB_PIN, motorSpeedB); // Send PWM signal to motor B
  }
