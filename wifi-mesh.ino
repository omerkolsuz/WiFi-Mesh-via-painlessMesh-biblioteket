/* 
 * --------------------------------------------------------------------------------------------
 * Example: Two way communication between 3 ESP32 using WiFi Mesh via the painlessMesh library. 
 * --------------------------------------------------------------------------------------------
 * Date: 07-11-2024 (7th of Nov, 2024)
 * --------------------------------------------------------------------------------------------
*/

#include <WiFi.h>
#include <painlessMesh.h>

// Wi-Fi-uppgifter
const char* ssid = "123456";       
const char* password = "Omer1234"; 

#define MESH_PREFIX     "ESP32_Mesh_Network"
#define MESH_PASSWORD   "meshpassword"
#define MESH_PORT       5555

Scheduler userScheduler;
painlessMesh mesh;

const int ledPin = 2;
const int buttonPin = 15;

bool buttonState = false;
bool lastButtonState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// inkommande mesh-meddelanden
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Mottog meddelande från nod %u: %s\n", from, msg.c_str());

  if (msg == "ButtonPressed") {
    digitalWrite(ledPin, HIGH); 
    Serial.println("LED tänds på grund av ButtonPressed");
  } else if (msg == "ButtonReleased") {
    digitalWrite(ledPin, LOW);  
    Serial.println("LED släcks på grund av ButtonReleased");
  }

}

void checkButton() {
  bool reading = digitalRead(buttonPin) == LOW;

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      String msg;
      if (buttonState) {
        msg = "ButtonPressed";
        mesh.sendBroadcast(msg);
        Serial.println("Knapp tryckt, skickar 'ButtonPressed'");
      } else {
        msg = "ButtonReleased";
        mesh.sendBroadcast(msg);
        Serial.println("Knapp släppt, skickar 'ButtonReleased'");
      }

    }
  }

  lastButtonState = reading;
}

// kontrollerar knappens status
Task taskCheckButton(100, TASK_FOREVER, &checkButton);

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi anslutet");

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);

  userScheduler.addTask(taskCheckButton);
  taskCheckButton.enable();

  Serial.println("Självständigt mesh-nätverk är startat");
}

void loop() {
  mesh.update();         
  userScheduler.execute(); 
}
