
#include <TimeLib.h>


char RMC[100];
char GGA[100];

float latitudeInDegs = 45.255456;
float longitudeInDegs = 64.5000;
float speedInKnots = 150.1;
float hdg = 45.1;
float magVar = 3.1;


float altitudeMSLinMetres = 101.3;
float GeoidalSep = 23.1;

int PositionFix = 1;     //std::string PosFix = std::format("{}", p2nmea->PositionFix+1);
int HDOP = 4;//std::string HDOP = std::format("{:0.2f}", p2nmea->HDOP);
int numSats = 5; //std::format("{:d}", p2nmea->n_satellites);


#define NMEA_END_CHAR_1 '\n'
#define NMEA_MAX_LENGTH 70

uint8_t nmea_get_checksum(const char *sentence)
{
    const char *n = sentence + 1; // Plus one, skip '$'
    uint8_t chk = 0;

    /* While current char isn't '*' or sentence ending (newline) */
    while ('*' != *n && NMEA_END_CHAR_1 != *n) {
        if ('\0' == *n || n - sentence > NMEA_MAX_LENGTH) {
            /* Sentence too long or short */
            return 0;
        }
        chk ^= (uint8_t) *n;
        n++;
    }

    return chk;
}

  char latStr[12];
  char lngStr[12];

void makeLatLngStrings()
{
  char str_temp[10]; // Buffer to hold the converted float
  
  int latAsInt = (int)latitudeInDegs;
  float fracLatInDegrees = latitudeInDegs - latAsInt;
  float latMins = fracLatInDegrees * 60.0;
  dtostrf(latMins, 6, 4, str_temp); 
  
  sprintf(latStr, "%02d%s", (int)latAsInt,str_temp);
  
  int lngAsInt = (int)longitudeInDegs;
  float fracLngInDegrees = longitudeInDegs - lngAsInt;
  float lngMins = fracLngInDegrees * 60.0;
  dtostrf(lngMins, 6, 4, str_temp); 
  
  sprintf(lngStr, "%02d%s", (int)lngAsInt,str_temp);

}

void makeRMC()
{
  //makeLatLngStrings();

  
  char speedStr[10]; // Buffer to hold the converted float
  sprintf(speedStr, "%2d.%01d", (int)speedInKnots, (int)(speedInKnots * 10) % 10);

  char hdgStr[10]; // Buffer to hold the converted float
  sprintf(hdgStr, "%2d.%01d", (int)hdg, (int)(hdg * 10) % 10);

  char magVarStr[10]; // Buffer to hold the converted float
  sprintf(magVarStr, "%03d.%01d,W", (int)magVar, (int)(magVar * 10) % 10);

  time_t t = now(); 
  int hh = hour(t)+1;
  int mm = minute(t)+1;
  int ss = second(t);

  int _year = year(t)-2000;
  int _month = month(t);
  int _day = day(t);
  sprintf(RMC, "$GPRMC,%02d%02d%02d,A,%s,N,%s,W,%s,%s,%02d%02d%02d,%s*", hh,mm,ss,latStr,lngStr, speedStr,hdgStr,_year,_month,_day,magVarStr);

  int checksum = nmea_get_checksum(RMC);
  char buf2[10];
  sprintf(buf2,"%2X\r\n",checksum);
  strcat(RMC,buf2);
}


void makeGGA()
{
  //makeLatLngStrings();

  float altMSL = 99.1;
  char altMSLStr[10]; // Buffer to hold the converted float
  sprintf(altMSLStr, "%2d.%01d", (int)altMSL, (int)(altMSL * 10) % 10);

  float undulation = 2.5;
  char undulStr[10]; // Buffer to hold the converted float
  sprintf(undulStr, "%1d.%01d", (int)undulation, (int)(undulation * 10) % 10);

  time_t t = now(); 
  int hh = hour(t)+1;
  int mm = minute(t)+1;
  int ss = second(t);

  int _year = year(t)-2000;
  int _month = month(t);
  int _day = day(t);
  sprintf(GGA, "$GPGGA,%02d%02d%02d,%s,N,%s,W,%d,%d,%d,%s,M,%s,M,,,*", hh,mm,ss,latStr,lngStr, PositionFix, numSats, HDOP, altMSLStr,undulStr);

  int checksum = nmea_get_checksum(GGA);
  char buf2[10];
  sprintf(buf2,"%2X\r\n",checksum);
  strcat(GGA,buf2);
}



void setup()    // put your setup code here, to run once:
{
  //time_t t;
  setTime(20,43,0,9,5,26); //hr, min, sec, day, month, year

  pinMode(LED_BUILTIN, OUTPUT); // Initiatimetype:AllTopic:Communicationlize LED pintimetype:AllTopic:Communication
  Serial.begin(9600); // Start serial communication

}


float deg2rad(float degrees) {
    static const float pi_on_180 = 4.0 * atan(1.0) / 180.0;
    return degrees * pi_on_180;
}

float CalcEndPointFromRangeInMetresAndBearing(float lat1, float lon1, float azi1, float s12, float& lat2, float& lon2)  //s12 is in metres
{
  /*
  geod_init(&geod, geod_a, geod_f);
  double azi2;
  geod_direct(&geod, lat1, lon1, azi1, s12, &lat2, &lon2, &azi2);
  return azi2;
  */
  char buf[100];
  
  float lengthLatDegree = 111120; // metres in 60 nm
  float deltaLatInMetres = s12 * cos(deg2rad(azi1));
  float deltaLatinDegrees = deltaLatInMetres/lengthLatDegree;

  float lengthLngDegree = abs(111120* cos(deg2rad(latitudeInDegs)));
  float deltaLngInMetres = s12 * sin(deg2rad(azi1));
  float deltaLnginDegrees = deltaLngInMetres/lengthLngDegree;

if (false)
{

  sprintf(buf, "deltaLatInMetres  %4d.%04d \r\n", (int)deltaLatInMetres, (int)(deltaLatInMetres * 1000) % 1000);
  Serial.print(buf);
  sprintf(buf, "deltaLatIndegrees  %4d.%07d \r\n", (int)deltaLatinDegrees, (int)(deltaLatinDegrees * 1000000) % 1000000);
  Serial.print(buf);

  sprintf(buf, "deltaLngInMetres  %4d.%04d \r\n", (int)deltaLngInMetres, (int)(deltaLngInMetres * 1000) % 1000);
  Serial.print(buf);
  sprintf(buf, "deltaLngIndegrees  %4d.%07d \r\n", (int)deltaLnginDegrees, (int)(deltaLnginDegrees * 1000000) % 1000000);
  Serial.print(buf);
}

  latitudeInDegs += deltaLatinDegrees;
  longitudeInDegs -= deltaLnginDegrees;



}


#define METER_PER_NM 1852
float CalcDistanceInMetresFromSpeedAndTime(float velInKnots, float travelTimeInSeconds) //vel in knots (1 Kt=1852 m/hr) or 30.8666m/minute or .5144444m per second
{
    char buf[40];
    sprintf(buf,"distance [m] %d\r\n" ,(int)(travelTimeInSeconds * velInKnots * METER_PER_NM / 3600.0));
    //Serial.print(buf);
    return travelTimeInSeconds * velInKnots * METER_PER_NM / 3600.0;
}

void UpdatePosition()
{
  float resLat, resLng;
  float deltaDistanceInMetres = CalcDistanceInMetresFromSpeedAndTime(speedInKnots, 1.0); //speed is in knots, time is in seconds
  float GCHdg = CalcEndPointFromRangeInMetresAndBearing(latitudeInDegs, longitudeInDegs, hdg, deltaDistanceInMetres, resLat, resLng);
  //latitudeInDegs = abs(resLat);
  //longitudeInDegs = abs(resLng);
  hdg += 1.0;
  if (hdg>359) hdg=0;

}

void loop() 
{
  digitalWrite(LED_BUILTIN, HIGH); // Turn LED on
  delay(500);                     // Wait
  digitalWrite(LED_BUILTIN, LOW);  // Turn LED off
  delay(500);                     // Wait

  makeLatLngStrings();
  makeRMC();
  Serial.print(RMC);
  makeGGA();
  Serial.print(GGA);

  UpdatePosition();
}











