/*
****************************************************************************
* Copyright (c) 2015 Arduino srl. All right reserved.
*
* File : UnowifiCiao.cpp
* Date : 2016/02/16
* Revision : 0.0.1 $
* Author: adriano[at]arduino[dot]org
*
****************************************************************************
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "Ciao.h"
#include "Wifi.h"

#if defined(__AVR_ATmega328P__)

#include <stdarg.h>
#include <stdio.h>

WifiData espSerial = WifiData(SC16IS750_PROTOCOL_I2C,SC16IS750_ADDRESS_AA);
ESP esp(&espSerial, 4);
REST rest(&esp);

CiaoClass Ciao;
WifiClass Wifi;

boolean wifiConnected = false;

void wifiCb(void* response)
{
	uint32_t status;
	RESPONSE res(response);

	if(res.getArgc() == 1) {
		res.popArgs((uint8_t*)&status, 4);
		if(status == STATION_GOT_IP){
			espSerial.println("DBG: Internet Connected");
			wifiConnected = true;
		}
		else {
			wifiConnected = false;
		}
	}
}

void WifiClass::powerON(){ 	
	
}
void WifiClass::powerOFF(){ 	
	
}

WifiData WifiClass::stream(){
	return espSerial;
}
int WifiClass::read(){
	return espSerial.read();
}
void WifiClass::print(String str){
	espSerial.print(str);
}
void WifiClass::println(String str){
	espSerial.println(str);
}
void CiaoClass::print(String str){
	espSerial.print(str);
}
void CiaoClass::println(String str){
	espSerial.println(str);
}
boolean WifiClass::available(){
	return espSerial.available();
}
void WifiClass::connect(char* ssid,char* pwd){
	esp.wifiConnect(ssid, pwd);
}
boolean WifiClass::connected(){
	return wifiConnected;
}

void WifiBegin() {
	Serial.begin(9600);
	espSerial.begin(9600);
	if(espSerial.ping()!=1) {
		espSerial.println("DBG: esp not found");
		while(1);
	}
	else {
		//espSerial.println("device found");
	}
	//metti GPIO control here !!!
	esp.enable();
	delay(1000);
	esp.reset();
	delay(1000);
	while(!esp.ready());
	esp.wifiCb.attach(&wifiCb);
	espSerial.println("\nDBG: UnoWiFi Start");
}
void WifiClass::begin() {
	WifiBegin();
}
void CiaoClass::begin() {

	WifiBegin();
	espSerial.println("DBG: Ciao start");
	rest.begin("google.com");
	rest.get("/");
	delay(3000);
}

CiaoData responseREAD(){
	CiaoData data;
	delay(400);
	char response[64] = "";
	char cstr[8] = "";

	int ret = rest.getResponse(response, 64);
	//espSerial.println(ret);

	snprintf(cstr,8,"%d",ret);

		data.msg_split[0]="id";
		data.msg_split[1]=cstr;
		data.msg_split[2]=response;
				
	return data;
}
CiaoData requestPOST(char* hostname, String stringone){
	rest.begin(hostname);
	//delay(1000);	fix
	//esp.process(); 
	rest.post((const char*) stringone.c_str(),0);
	return responseREAD(); 
}

CiaoData requestGET(char* hostname, String stringone){

	rest.begin(hostname);
	//delay(1000);   fix
	//esp.process(); 
	rest.get((const char*) stringone.c_str());
	return responseREAD(); 
}

CiaoData PassThroughM(char* connector,char* hostname, String stringone, char* method){
  
	short mode = 0;
	if (!strcmp(connector, "rest")){
		mode = 0;
	}
	else if (!strcmp(connector, "mqtt")){
		mode = 1;
	}
	else {
		CiaoData data;				
		data.msg_split[0]="";
		data.msg_split[1]="";
		data.msg_split[2]="Protocol Error";
		return data;
	}
	
	if (mode == 0){ 
		if (!strcmp(method, "GET")){
			return requestGET(hostname, stringone); 
		}
		else if (!strcmp(method, "POST")){
			return requestPOST(hostname, stringone); 
		}
		else{
			CiaoData data;				
			data.msg_split[0]="";
			data.msg_split[1]="";
			data.msg_split[2]="Method Error";
			return data;
		}   
	}
}

CiaoData CiaoClass::write(char* connector,char* hostname, String stringone, char* method) {
	return PassThroughM(connector, hostname, stringone, method);
}
CiaoData CiaoClass::read(char* connector,char* hostname, String stringone, char* method) {
	return PassThroughM(connector, hostname, stringone, method);
}
CiaoData CiaoClass::write(char* connector,char* hostname, String stringone) {
	return PassThroughM(connector, hostname, stringone, "GET");
}
CiaoData CiaoClass::read(char* connector,char* hostname, String stringone) {
	return PassThroughM(connector, hostname, stringone, "GET");
}

#endif