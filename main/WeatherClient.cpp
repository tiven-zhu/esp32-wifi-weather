/**The MIT License (MIT)

Copyright (c) 2015 by Daniel Eichhorn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

See more at http://blog.squix.ch
*/

#include "WeatherClient.h"
//#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

cJSON *WeatherClient::parse_file(const char* content){

    cJSON *parsed = NULL;
//    char *content = read_file(filename);

    parsed = cJSON_Parse(content);


    if (content != NULL)
    {
//        free(content);
    }
    return parsed;
}
// extract int from char value.
int WeatherClient::ctoi(const char* value){
	int i,j;
	char tmp[5];
	for(i = 0, j = 0; i < strlen(value); i++){
		if('0' <= value[i] && value[i] <= '9'){
			tmp[j] = value[i];
			j++;
		}else{
			continue;
		}
	}
//	Serial.printf("tiven--------- tmpvalue = %s\n", tmp);

	return atoi(tmp);
}
void WeatherClient::basic_cjson_sys(cJSON *root)
{

	int action_value;
	char *user_value, *pwd_value;
	cJSON *pwd, *user, *data, *action;
	cJSON *tomorrowNode, *tomorrowTemp, *tomorrowLowTemp, *tomorrowHighTemp;
	cJSON *forecastNode, *todayNode, *todayItem, *todayMinTemp, *todayMaxTemp;
    if (NULL == root) {
        return ;
    }
    //将json包转为字符串显示，一般我们使用时选择str1，str2这种方式是更加容易我们观看的方式
    char *str1 = cJSON_PrintUnformatted(root);
    char *str2 = cJSON_Print(root);
//    Serial.printf("str1: \r\n%s\r\n", str1);
//    Serial.printf("str2: \r\n%s\r\n", str2);
    cJSON_Delete(root);
    free(str2);
    //一般我们接收到的也是cjson类型的字符串str1，下面解析
    //将字符串格式cjson数据转为cJSON类型数据
    cJSON *sys_root = cJSON_Parse(str1);
//    cJSON* sys_root = root;
    if (!sys_root) {
        goto err1;
    }
    action = cJSON_GetObjectItem(sys_root, "time");
    if (!action) {
        goto err2;
    }
    action_value = action->valueint;

    data = cJSON_GetObjectItem(sys_root, "data");
    if (!data) {
        goto err2;
    }

    user = cJSON_GetObjectItem(data, "shidu");
    if (!user) {
        goto err2;
    }
    currentHumidity = String(user->valuestring).toInt();

    pwd = cJSON_GetObjectItem(data, "wendu");
    if (!pwd) {
        goto err2;
    }
    currentTemp = String(pwd->valuestring).toInt();

    forecastNode = cJSON_GetObjectItem(data, "forecast");

    todayNode = cJSON_GetArrayItem(forecastNode, 0);
    todayMinTemp = cJSON_GetObjectItem(todayNode, "low");
    todayMaxTemp = cJSON_GetObjectItem(todayNode, "high");

//    minTempToday = String(todayMinTemp->valuestring).toInt();
    minTempToday = ctoi(todayMinTemp->valuestring);
    maxTempToday = ctoi(todayMaxTemp->valuestring);
//    maxTempToday = String(todayMaxTemp->valuestring).toInt();

    tomorrowTemp = cJSON_GetArrayItem(forecastNode, 1);
    tomorrowLowTemp = cJSON_GetObjectItem(tomorrowTemp, "low");
    tomorrowHighTemp = cJSON_GetObjectItem(tomorrowTemp, "high");
//    minTempTomorrow = String(cJSON_GetStringValue(tomorrowValue)).toInt();
//    maxTempTomorrow = String(cJSON_GetStringValue(tomorrowTempHigh)).toInt();
    minTempTomorrow = ctoi(tomorrowLowTemp->valuestring);
    maxTempTomorrow = ctoi(tomorrowHighTemp->valuestring);
//    currentIcon = "snow";

    Serial.printf("tomorrowTemp =%s,  tomorowTemp = %s\n", todayMinTemp->valuestring,
    		todayMaxTemp->valuestring);
    Serial.printf("minTempToday=%d, maxTempToday=%d,\t, ", minTempToday, maxTempToday);
//    Serial.printf("action = %d, user = %s, pwd = %s\r\n", action_value, user_value, pwd_value);
err2:
    cJSON_Delete(sys_root);
err1:
    free(str1);
    return ;
}


void WeatherClient::updateWeatherData(String apiKey, double lat, double lon) {
  WiFiClient client;
  HTTPClient http;

//  const int httpPort = 80;
/*  if (!client.connect("http://t.weather.itboy.net", httpPort)) {
    Serial.println("connection failed");
    return;
  }*/
  Serial.println("[HTTP] begin...\n");
  cityId = "101190101";//nanjing
  
  // We now create a URI for the request
//  String url = "/rest/weather?apiKey=" + apiKey + "&lat=" + String(lat) + "&lon=" + String(lon) + "&units=" + myUnits;
  String url = "http://t.weather.itboy.net/api/weather/city/"+String(cityId);

  http.begin(url);
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  Serial.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: www.squix.org\r\n" + 
               "Connection: close\r\n\r\n");
  int httpCode = http.GET();
  while(httpCode  < 0) {
    delay(200); 
  }
  
  // Read all the lines of the reply from server and print them to Serial
  if (httpCode > 0) {
		Serial.printf("[HTTP] GET... code: %d\n", httpCode);
		if (httpCode != HTTP_CODE_OK)
			return;
//		String line = http.readStringUntil('\n');
		String line = http.getString();
		const char* payload = line.c_str();
		cJSON *json = cJSON_Parse(payload);

//		Serial.println(cJSON_Print(json));
		basic_cjson_sys(json);


//		Serial.println(line);
/*
		String key = getKey(line);
		if (key.length() > 0) {
			String value = getValue(line);
			if (key == "CURRENT_TEMP") {
				currentTemp = value.toInt();
			} else if (key == "CURRENT_HUMIDITY") {
				currentHumidity = value.toInt();
			} else if (key == "CURRENT_ICON") {
				currentIcon = value;
			} else if (key == "CURRENT_SUMMARY") {
				currentSummary = value;
			} else if (key == "MAX_TEMP_TODAY") {
				maxTempToday = value.toInt();
			} else if (key == "MIN_TEMP_TODAY") {
				minTempToday = value.toInt();
			} else if (key == "ICON_TODAY") {
				iconToday = value;
			} else if (key == "SUMMARY_TODAY") {
				summaryToday = value;
			} else if (key == "MAX_TEMP_TOMORROW") {
				maxTempTomorrow = value.toInt();
			} else if (key == "ICON_TOMORROW") {
				iconTomorrow = value;
			} else if (key == "MIN_TEMP_TOMORROW") {
				minTempTomorrow = value.toInt();
			} else if (key == "SUMMARY_TODAY") {
				summaryTomorrow = value;
			}

		}
*/

	}
  
  Serial.println();
  Serial.println("closing connection");    
}

void WeatherClient::setUnits(String units) {
   myUnits = units; 
}

String WeatherClient::getKey(String line) {
  int separatorPosition = line.indexOf("=");
  if (separatorPosition == -1) {
    return "";
  }  
  return line.substring(0, separatorPosition);
}

String WeatherClient::getValue(String line) {
  int separatorPosition = line.indexOf("=");
  if (separatorPosition == -1) {
    return "";
  }  
  return line.substring(separatorPosition + 1);
}


int WeatherClient::getCurrentTemp(void) {
  return currentTemp;
}
int WeatherClient::getCurrentHumidity(void) {
  return currentHumidity;
}
String WeatherClient::getCurrentIcon(void) {
  return currentIcon;
}
String WeatherClient::getCurrentSummary(void) {
  return currentSummary;
}
String WeatherClient::getIconToday(void) {
  return iconToday;
}
int WeatherClient::getMaxTempToday(void) {
  return maxTempToday;
}
int WeatherClient::getMinTempToday(void) {
  return minTempToday;
}
String WeatherClient::getSummaryToday(void) {
  return summaryToday;
}
int WeatherClient::getMaxTempTomorrow(void) {
  return maxTempTomorrow;
}
int WeatherClient::getMinTempTomorrow(void) {
  return minTempTomorrow;
}
String WeatherClient::getIconTomorrow(void) {
  return iconTomorrow;
}
String WeatherClient::getSummaryTomorrow(void) {
  return summaryTomorrow;
}
