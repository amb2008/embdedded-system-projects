#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <string>
#include <ESP_Mail_Client.h>

// replace wifi name and password with your WiFi credentials inside the quotation marks.
// Set password to "" for open networks.
char ssid[] = "WiFi name";
char pass[] = "WiFi password";
#define WIFI_SSID "WiFi name"
#define WIFI_PASSWORD "WiFi password"

//replace with your email
#define AUTHOR_EMAIL "your email"
//replace with the 16 character google generated app password from step 1
#define AUTHOR_PASSWORD "16 character google generated app password"

//replace with who you want to recieve this email, for testing this should probably be yourself
#define RECIPIENT_EMAIL "recipient email"

// Nothing past here has to be changed except line 81 and 134
int sensorVal; 
int temperature;
int nTemp;
int asapEmail = 1;
String temp;

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465


SMTPSession smtp;

void smtpCallback(SMTP_Status status);

void setup(){
  Serial.begin(9600);
  Serial.println();
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);

  }
  pinMode(A0, INPUT);

  sensorVal = analogRead(A0); 
  temperature = sensorVal*0.32;
  Serial.println(temperature+5);
  nTemp = temperature+5;
  //converting temperature data from integer to string
  auto temp = std::to_string(nTemp);
  
  Serial.println("");
  Serial.println("WiFi connected.");

  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the session config data */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = "ESP";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Temperature Data";
  //replace with the recipient's name and email, if you want to add more recipient copy and paste the line
  message.addRecipient("recipient's name", "recipient's email");

  //Send raw text message
String textMsg = "The temperature is: ";
message.text.content = textMsg.c_str()+temp;
message.text.charSet = "us-ascii";
message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
}

void loop(){
  // Reading sensor from hardware analog pin A0
  sensorVal = analogRead(A0); 
  temperature = sensorVal*0.32;
  Serial.println(temperature);

  //asapEmail is low so we can test that it works, but you may increase it to whatever temperature you want for an emergency email
  if(temperature > 80 && asapEmail == 1){
  auto temp = std::to_string(temperature);

  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the session config data */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = "ESP";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "EMERGENCY EMAIL";
  //replace with the recipient's name and email, if you want to add more recipient copy and paste the line
  message.addRecipient("recipient's name", "recipient's email");

  //Send raw text message
String textMsg = "The temperature is too high! Evacuate Now! Temperature is: ";
message.text.content = textMsg.c_str()+temp;
message.text.charSet = "us-ascii";
message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Set the custom message header */
  //message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());

  asapEmail = 0;
  
}

if(temperature<79){
  asapEmail = 1;  
}
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");
  }
}