/*
* @author acerNacho
* date: 15/11/2018
* description: A esp32 webserver using sd card or spiffs library
*
* PINS OF SD CARD <-> ESP32:
* CS <-> GPIO5
* MOSI <-> GPIO23
* GND <-> GND
* SCK <-> GPIO18
* MISO <-> GPIO19
* VCC <-> 3V3
*/


#include "WiFi.h"
#include "FS.h"
#include "SPIFFS.h"
#include "SD.h"
#include "SPI.h"
#include "ESPAsyncWebServer.h"

#define FORMAT_SPIFFS_IF_FAILED true

#ifdef __cplusplus
extern "C" {
#endif
	uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();


const char* ssid = "ssid";
const char* password =  "password";

AsyncWebServer server(80);

// Load files of a sd card directory and/or spiffs

void loadDir(fs::FS &fs, const char * dirname, uint8_t levels, boolean sd){
	Serial.printf("Listing directory: %s\r\n", dirname);

	File root = fs.open(dirname);
	if(!root){
		Serial.println("- failed to open directory");
		return;
	}
	if(!root.isDirectory()){
		Serial.println(" - not a directory");
		return;
	}

	File file = root.openNextFile();
	String aux= "";
	while(file){
		if(file.isDirectory()){
			if(levels){
				Serial.println(file.name());
				loadDir(fs, file.name(), levels -1, sd);
			}
		} else {
			aux=file.name();
			Serial.println(aux);
			if(sd) {
				server.on(file.name(), HTTP_GET, [aux](AsyncWebServerRequest *request){
					request->send(SD , aux , getDataType(getFileExtension(aux)));
				});
			} else {
				server.on(file.name(), HTTP_GET, [aux](AsyncWebServerRequest *request){
					request->send(SPIFFS , aux , getDataType(getFileExtension(aux)));
				});
			}
		}
		file = root.openNextFile();
	}
}

// Get files extension

static String getFileExtension(String name) {
	int lastIndexOf = name.lastIndexOf(".");
	if (lastIndexOf == -1) {
		return "";
	}
	return name.substring(lastIndexOf);
}

// Getting mime data type

static String getDataType(String extension) {
	if (extension.equals(".html")) {
		return "text/html";
	} else if (extension.equals(".html")){
		return "text/html";
	} else if (extension.equals(".htm")){
		return "text/html";
	} else if (extension.equals(".css")){
		return "text/css";
	} else if (extension.equals(".js")){
		return "application/javascript";
	} else if (extension.equals(".png")){
		return "image/png";
	} else if (extension.equals(".gif")){
		return "image/gif";
	} else if (extension.equals(".jpg")){
		return "image/jpg";
	} else if (extension.equals(".ico")){
		return "image/x-icon";
	} else if (extension.equals(".xml")){
		return "text/xml";
	} else if (extension.equals(".zip")){
		return "application/zip";
	} else if (extension.equals(".pdf")){
		return "application/pdf";
	} else if (extension.equals(".woff")){
		return "application/x-font-woff";
	} else if (extension.equals(".ttf")){
		return "font/ttf";
	} else if (extension.equals(".svg")){
		return "image/svg+xml";
	} else {
		return "text/html";
	}

}

void setup(){
	Serial.begin(115200);


  // Mount the sd card
	if(!SD.begin()){
		Serial.println("Card Mount Failed");
		return;
	}

	uint8_t cardType = SD.cardType();

	if(cardType == CARD_NONE){
		Serial.println("No SD card attached");
		return;
	}

	Serial.print("SD Card Type: ");
	if(cardType == CARD_MMC){
		Serial.println("MMC");
	} else if(cardType == CARD_SD){
		Serial.println("SDSC");
	} else if(cardType == CARD_SDHC){
		Serial.println("SDHC");
	} else {
		Serial.println("UNKNOWN");
	}

	uint64_t cardSize = SD.cardSize() / (1024 * 1024);
	Serial.printf("SD Card Size: %lluMB\n", cardSize);

  // Mount the SPIFFS

	if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
		Serial.println("An Error has occurred while mounting SPIFFS");
		return;
	}


	delay(1000);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.println("Connecting to WiFi..");
	}

	Serial.println(WiFi.localIP());

	loadDir(SD, "/", 0, true);
	loadDir(SD, "/", 2, true);
	loadDir(SPIFFS, "/", 0, false);

  // Additional server parameters
	
	server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, "text/plain", String((temprature_sens_read()-32)/1.8));
	});

  // Redirection

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		request->redirect("/index.html");
	});

	server.begin();
}

void loop(){}
