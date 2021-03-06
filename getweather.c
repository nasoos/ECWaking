#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

bool isNum (char in){
	if (in >= '0' && in <= '9')
		return true;
	return false;
}

bool checkChar(char in){
	if (in >= 48 && in <= 57)
		return true;
	else if(in == 'e' || in == 'E' || in == '.' || in =='+' || in == '-')
		return true;
	else
		return false;
}

float FahtoCel (float value){
	value -= 32;
	value /= 1.8;
	
	return value;
}

bool stringToFloat(const char input[], float *value) {
	enum State { Start, Normal, Terminal };
	enum InputState { Whole, Decimal, ExponentSign, Exponent };
	
	enum State state = Start;
	enum InputState inState;
	char in = input[0];
	int inIndex = 0;
	bool Done = false;
	
	int sign = 1;
	int exponent = 0;
	int decimal = 0;
	int whole = 0;
	int exponentSign = 1;
	
	int decDigits = 0;
	
	while (!Done){
		in = input[inIndex];
		inIndex++;
		switch(state){
		case Start:
			if (in == '+' || in == '-'){
				if (!isNum(input[inIndex]))
					return false;
				if (in == '-')
					sign = -1;
				inState = Whole;
				state = Normal;
			}
			else if(in == '.'){
				if (!isNum(input[inIndex]))
					return false;
				inState = Decimal;
				state = Normal;
			}
			else if (isNum(in)){
				state = Normal;
				inState = Whole;
				whole *= 10;
				whole += in - 48;				
			}
			else
				return false;
			break;
		case Normal:
			switch(inState){
			case Whole:
				if (isNum(in))
				{
					whole *= 10;
					whole += in - 48;
				}
				else if (in == '.'){
					if (!isNum(input[inIndex]))
						return false;
					inState = Decimal;
				}
				else if (in == 'e' || in == 'E'){
					if (!isNum(input[inIndex]) && !(input[inIndex] == '+' || input[inIndex] == '-'))
						return false;
					inState = ExponentSign;
				}
				else if(in == 0)
					state = Terminal;
				else if (!checkChar(in) || in == '+' || in == '-')
					return false;
				break;
			case Decimal:
				if (isNum(in))
				{
					decimal *= 10;
					decimal += in - 48;
					decDigits++;
				}
				else if (in == 'e' || in == 'E'){
					if (!isNum(input[inIndex]) && !(input[inIndex] == '+' || input[inIndex] == '-'))
						return false;
					inState = ExponentSign;
				}
				else if(in == 0)
					state = Terminal;
				else if (!checkChar(in) || in == '.' || in == '+' || in == '-')
					return false;
				break;
			case ExponentSign:
				if (isNum(in)){
					inState = Exponent;
					exponent *= 10;
					exponent += in - 48;					
				}
				else if (in == '+' || in == '-'){
					if (!isNum(input[inIndex]))
						return false;
					if (in == '-')
						exponentSign = -1;
					inState = Exponent;
				}
				else if (!checkChar(in) || in == '.')
					return false;
				break;
			case Exponent:
				if(isNum(in))
				{
					exponent *= 10;
					exponent += in - 48;
				}
				else if(in == 0)
					state = Terminal;
				else if (!checkChar(in) || in == 'e' || in == 'E' || in == '+' || in == '-')
					return false;
				break;
			}
			break;
		case Terminal:
			Done = true;
			exponent *= exponentSign;
			*value = (sign * (whole + decimal* pow(10, -decDigits)) * pow(10, exponent));
			return true;
			break;
		}		
	}
}

char * substring(char * string, int position, int length){
    char *pointer;
    int c;
    
    pointer = malloc(length+1);
    
    if (pointer == NULL)
    {
        printf("Unable to allocate memory.\n");
        exit(1);
    }
    
    for (c = 0 ; c < length ; c++)
    {
        *(pointer+c) = *(string+position-1);
        string++;
    }
    
    *(pointer+c) = '\0';
    
    return pointer;
}

bool getIP(char ** ipaddress){
	FILE *fp = popen("curl ipinfo.io/ip", "r");

	fscanf(fp, "%s", *ipaddress);
	pclose(fp);
	
	return true;
}

bool getLongandLat(char * buff, char ** latitude, char ** longitude){
    char * result= strstr(buff, "latitude\":");
    int LatitudeIndex = result - buff + 11;
    result= strstr(buff, "longitude\":");
    int LongitudeIndex = result - buff + 12;
    int length = LongitudeIndex - LatitudeIndex - 13;
    
    *longitude = substring(buff, LongitudeIndex, length);    
    *latitude = substring(buff, LatitudeIndex, length);
    
    return true;
}

bool getWoeid(char * buff, char ** woeid){
    int woeidIndex = strstr(buff, "woeid\":") - buff + 9;
    int woeidEndIndex = strstr(buff, "\"}}") - buff + 1;
    *woeid = substring(buff, woeidIndex, woeidEndIndex - woeidIndex);
    
    return true;
}

bool getTemperature(char * buff, char ** high, char ** low){
	int highIndex = strstr(buff, "high\":\"") - buff + 8;
	int lowIndex = strstr(buff, "low\":\"") - buff + 7;
	int textIndex = strstr(buff, "text\":\"") - buff + 8;
		
	*high = substring(buff, highIndex, lowIndex - highIndex - 9);
	*low = substring(buff, lowIndex, textIndex - lowIndex - 10);
		
	return true;	
}

bool getUnits(char * buff, char ** temperatureUnits){
	int unitIndex = strstr(buff, "temperature\":") - buff + 15;
	
	*temperatureUnits = substring(buff, unitIndex, 1);
	if (*temperatureUnits != "C" && *temperatureUnits != "F")
		return false;
	
	return true;
}

bool getLineWithString(char * filename, char * string, char * buff){
	FILE * file;
	file = fopen(filename, "r");
	
	if (file == NULL)
		return false;
	
	while (fscanf(file, "%s", buff) != EOF){
		if (strstr(buff, string)){
			return true;			
		}
	}
	return false;
}

bool getTemperatureInC(float * hightemp, float * lowtemp){
	char * ipaddress = (char *) malloc(20);
	char command[256];
	char buff[256];
	
	char * woeid = (char *) malloc(15);
	char * longitude = (char *) malloc(8);
	char * latitude = (char *) malloc(8);
	char * high = (char *) malloc(5);
	char * low = (char *) malloc(5);
	char * temperatureUnits = (char *) malloc(1);
    
	system("curl ipinfo.io/ip > content.txt");
	
	if (getLineWithString("content.txt", ".", buff)){
		strcpy(ipaddress, buff);
	}
	
	snprintf(command, sizeof command, "%s%s%s", "curl freegeoip.net/json/", ipaddress, " > content.txt");
	system(command);
	
	if (getLineWithString("content.txt", "longitude", buff))
		getLongandLat(buff, &longitude, &latitude);	
	
	snprintf(command, sizeof command, "%s%s%s%s%s", "curl query.yahooapis.com/v1/public/yql -d q=\"select woeid from geo.places where text=\\\"(", longitude, ", " , latitude, ")\\\"\" -d format=json > content.txt");
	system(command);
		
	if (getLineWithString("content.txt", "woeid", buff))
		getWoeid(buff, &woeid);	
	
	
	snprintf(command, sizeof command, "%s%s%s", "curl query.yahooapis.com/v1/public/yql -d q=\"select * from weather.forecast where woeid=\\\"", woeid, "\\\"\" -d format=json > content.txt");
	system(command);
	
	if (getLineWithString("content.txt", "high", buff))
		getTemperature(buff, &high, &low);	
	
	if (getLineWithString("content.txt", "temperature", buff))
		getUnits(buff, &temperatureUnits);
	
	float highfloat = 0;
	float lowfloat = 0;
	
	stringToFloat(high, &highfloat);
	stringToFloat(low, &lowfloat);	
	
	if (temperatureUnits == "F" || temperatureUnits[0] == 'F'){
		highfloat = FahtoCel(highfloat);
		lowfloat = FahtoCel(lowfloat);
	}
	
	float averageTemperature = (highfloat + lowfloat)/2;
	
	/*
	printf("Public IP: %s\n", ipaddress);
	printf("Coordinates: (%s, %s)\n", latitude, longitude);
	printf("woeid: %s\n", woeid);
	printf("Units were given in: %s\n", temperatureUnits);
	printf("(in celsius) High: %f , Low: %f\n", highfloat, lowfloat);
	printf("Average Temperature(C): %f\n", averageTemperature);
	*/
	
	*hightemp = highfloat;
	*lowtemp = lowfloat;
	return true;
}

int getWeather(char **line)
{
	float lowtemp;
	float hightemp;
	if (!getTemperatureInC(&hightemp, &lowtemp))
		return -1;
	
	snprintf(*line, 40, "%s%f%s%f", "High: ", hightemp, ", Low: ", lowtemp);
	
}

int main(){	
    char * line;
	line = (char*)malloc(40 * sizeof(char));
	
	getWeather(&line);
	
	printf("%s\n", line);
}

/*
Woooooo it’s a ghost
Spooky
fuck your ghost

How to curl:

Other things to note:
You can view content of variable content with:
echo $content
You can view content of file content.txt with:
cat /content.txt

From the onion omega, run:

To get longitude and latitude from IP address, run:

curl freegeoip.net/json/[IPADDRESS] > content.txt

Parse through coordinates.txt to get longitude and latitude

Given any longitude and latitude, we can get the woeid(specifies where the place is) with:

curl query.yahooapis.com/v1/public/yql -d q="select woeid from geo.places where text=\"([LONGITUDE],[LATITUDE])\"" -d format=json > content.txt

which stores the woeid in weather.txt

Using the woeid, we can get information on the location’s weather with:

curl query.yahooapis.com/v1/public/yql -d q="select * from weather.forecast where woeid=[WOEID]" -d format=json > weather.txt

This stores the information in weather.txt
*/
