
// -------------------------------USV TRAJECTORY TRACKING CONTROL-------------------------------

#include <Servo.h>
#include <TinyGPSPlus.h>
#include <QMC5883LCompass.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <MPU6050_tockn.h>
#include <math.h>

// Hằng số 
#ifndef pi
#define pi 3.14159265358979323846
#endif

// Hằng số
#define r_long 6378137.0 // Bán trục dài của ellipsoid (WGS84)
#define f 1.0 / 298.257223563 // Dẹt của ellipsoid (WGS84)
#define k0 0.9996 // Hệ số tỉ lệ UTM
#define E0 500000.0 // Kinh độ gốc UTM
#define N0 0.0 // Vĩ độ gốc UTM (0.0 ở bán cầu Bắc)

  bool auto_mode = false;

// MOTOR VARIABLES
  const int pin_throttle = 12;  // D6
  const int pin_turn = 15;      // D8
  const int pin_motor_2 = 13;   // D7
  const int pin_motor_1 = 14;   // D5

   
  Servo f_1;
  Servo f_2;   
  int throttle;
  int turn;
  int thrust = 0;
  int moment = 0;
  int speed_1 = 0;
  int speed_2 = 0;

// MPU6050
  // MPU6050 mpu6050(Wire);

//Compass
  QMC5883LCompass compass;
  float compass_reading;  
  

// GPS
  const int RXPin = 0;          // D3
  const int TXPin = 2;          // D4
  const uint32_t GPSBaud = 9600;
  TinyGPSPlus gps;
  SoftwareSerial ss(RXPin, TXPin);

// UDP WIFI
  const char* ssid = "TP-LINK_23";
  const char* password = "12341234";
  IPAddress qtIp(192, 168, 41, 101);  // Địa chỉ IP của máy chủ  192.168.41.101
  const char* end_char = "; ";
  const char* terminate_char = "~";
  
  WiFiUDP Udp;
  unsigned int localUdpPort = 4210;  // local port to listen on
  char  replyPacket[] = "USV Packet: ";  // a reply string to send back
  int packetSize;

// TRACKING & CONTROL PARAMETERS
  double arrival_start_time;
  double target_lat, target_lng;
  double temp;
  double usv_lat, usv_lng;
  float path_type, target_lat_1, target_lng_1, lat_center, lng_center, r_x, r_y, omega_x, omega_y;
  bool valid = false;
  float initial_x, initial_y;
  int track_time, track_duration;
  float x_path, y_path;
  float v_x_desire, v_y_desire;
  float along_e, cross_e;
  float previous_yaw_error; 

  float usv_yaw, path_yaw, desire_yaw, yaw_error;
  float usv_velocity, desire_u, u_error, u_high, u_low, u_control, U_desire, U_pseudo;
  float x_error, y_error;
  float distance ;
  int high_speed, maneuver, approach;
  const int look_ahead = 100;
  char gpsData[50];
  // PID paramater
  const float Kp_u = 10, Ki_u = 1.5, Kd_u = 0;
  const float Kp_yaw = 2, Ki_yaw = 0.005, Kd_yaw = 0;
  float integral_u = 0, derivative_u = 0, integral_yaw = 0, derivative_yaw = 0;
  unsigned long previousMillis = 0;
  unsigned long previousMillis2 = 0; // Biến để lưu thời gian trước đó
  const long interval = 1000;       // Khoảng thời gian 1 giây (1000ms)


//----------------------------------------------------------------------------- FUNCTIONS ----------------------------------------------------------------------------- 
//-----------------------------------------------------------------------------***-----***----------------------------------------------------------------------------- 



void motor_control(float _thrust, float _moment)
{
  speed_1 = _thrust + _moment; // PWM output, range 0-255
  speed_2 = _thrust - _moment;
  if (speed_1 > 80) speed_1 = 80;
  if (speed_2 > 80) speed_2 = 80;
  if (speed_1 < 0) speed_1 = 0;
  if (speed_2 < 0) speed_2 = 0;
  Serial.print("Motor: "); 
  Serial.print(speed_1);
  Serial.print(" ");
  Serial.println(speed_2);
  f_1.write(speed_1);
  if (speed_2 > 0)
  f_2.write(speed_2 + 10);
  else
  f_2.write(speed_2);
}

float sqr(float base) {
  return base * base;
}


// ------------------------------------* TRAJECTORY TRACKING *------------------------------------
// -----------------------------------------------------------------------------------------------
//
float get_path_x (int _track_time, int _path_type, float _initial_x, float _r_x, float _omega_x)
{
  float _x;

  if (_path_type == 1)
  // ------------Straight path------------
    _x =  _initial_x + _omega_x*_track_time*0.001;
  // ------------Sinusoidal------------
  if (_path_type == 2)
    _x =  _initial_x + _omega_x*_track_time*0.001;
  // ------------Circular--------------
  if (_path_type == 3)
    _x =  _initial_x + _r_x*sin(_omega_x*_track_time*0.001);
  if (_path_type == 4)
    _x = _initial_x + _r_x*_track_time*0.001*sin(_omega_x*_track_time*0.001);
 return _x;
}

float get_path_y (int _track_time, int _path_type, float _initial_y, float _r_y, float _omega_y)
{
  float _y;
  if (_path_type == 1)
  // ------------Straight path------------
    _y =  _initial_y + _omega_y*_track_time*0.001;
  // ------------Sinusoidal------------
  if (_path_type == 2)
    _y =  _initial_y + _r_y*sin(_omega_y*_track_time*0.001);
  // ------------Circular------------ 
  if (_path_type == 3)
    _y =  _initial_y + _r_y*cos(_omega_y*_track_time*0.001) - _r_y;
  if (_path_type == 4)
    _y = _initial_y + _r_y*_track_time*0.001*cos(_omega_y*_track_time*0.001);
  return _y;
}
float delta_2(float _a, float _b)
{
  float _delta_2;
  _delta_2 = pow((pow(_a,2)+pow(_b,2)),0.5);
  return _delta_2;
}

float v_x_path (int _track_time, int _path_type, float _initial_x, float _r_x, float _omega_x)
{
  float _v_x;
  // ------------Straight------------
  if (_path_type == 1) 
    _v_x =  _omega_x;
  // ------------Sinusoidal------------ 
  if (_path_type == 2)
    _v_x =  _omega_x;
  // ------------Circular------------
  if (_path_type == 3)
    _v_x =  _omega_x*r_x*cos(_omega_x*_track_time*0.001);
  if (_path_type == 4)
    _v_x = r_x * (sin(_omega_x*_track_time*0.001) + _omega_x * track_time*0.001 * cos(_omega_x*_track_time*0.001));
 return _v_x;
}

float v_y_path (int _track_time, int _path_type, float _initial_y, float _r_y, float _omega_y)
{
  float _v_y;
  // ------------Straight------------
  if (_path_type == 1) 
    //omega_y = (initial_y - target_lng_4)*10;
    _v_y =  omega_y;
  // ------------Sinusoidal------------ 
  if (_path_type == 2)
    _v_y =  _omega_y*r_y*cos(_omega_y*_track_time*0.001);
  // ------------Circular------------
  if (_path_type == 3)
    _v_y =  -_omega_y*r_y*sin(_omega_y*_track_time*0.001);
  if (_path_type == 4)
    _v_y = r_y * (cos(_omega_y*_track_time*0.001) - _omega_y * track_time*0.001 * cos(_omega_y*_track_time*0.001));
  return _v_y;
}

float wrap_2_pi (float _angle)
{
  float _wrap_angle = _angle;
  
  if (_angle > pi)
    _wrap_angle = _angle - 2*pi;  
  return _wrap_angle;
}



// Hàm tính toán tọa độ UTM x từ kinh độ và vĩ độ
float get_utm_x(float lat, float lon) {
    float e = sqrt(1 - pow(1 - f, 2)); // Độ lệch tâm
    float lat_rad = lat * pi / 180.0;
    float lon_rad = lon * pi / 180.0;

    // Xác định kinh tuyến trung tâm của múi UTM
    int zone = (int)((lon + 180) / 6) + 1;
    float lon0 = ((zone - 1) * 6 - 180 + 3) * pi / 180.0;

    float N = r_long / sqrt(1 - e*e*sin(lat_rad)*sin(lat_rad));
    float T = tan(lat_rad)*tan(lat_rad);
    float C = e*e/(1 - e*e)*cos(lat_rad)*cos(lat_rad);
    float A = cos(lat_rad)*(lon_rad - lon0);

    float x = k0 * N * (A + (1 - T + C) * pow(A, 3) / 6 + (5 - 18 * T + pow(T, 2) + 72 * C - 58 * e*e / (1 - e*e)) * pow(A, 5) / 120) + E0;
    return x;
}

// Hàm tính toán tọa độ UTM y từ kinh độ và vĩ độ
float get_utm_y(float lat, float lon) {
    float e = sqrt(1 - pow(1 - f, 2)); // Độ lệch tâm
    float lat_rad = lat * pi / 180.0;
    float lon_rad = lon * pi / 180.0;

    // Xác định kinh tuyến trung tâm của múi UTM
    int zone = (int)((lon + 180) / 6) + 1;
    float lon0 = ((zone - 1) * 6 - 180 + 3) * pi / 180.0;

    float N = r_long / sqrt(1 - e*e*sin(lat_rad)*sin(lat_rad));
    float T = tan(lat_rad)*tan(lat_rad);
    float C = e*e/(1 - e*e)*cos(lat_rad)*cos(lat_rad);
    float A = cos(lat_rad)*(lon_rad - lon0);

    float M = r_long * ((1 - e*e/4 - 3*e*e*e*e/64 - 5*e*e*e*e*e*e/256) * lat_rad 
                  - (3*e*e/8 + 3*e*e*e*e/32 + 45*e*e*e*e*e*e/1024) * sin(2 * lat_rad) 
                  + (15*e*e*e*e/256 + 45*e*e*e*e*e*e/1024) * sin(4 * lat_rad) 
                  - (35*e*e*e*e*e*e/3072) * sin(6 * lat_rad));
    
    float y = k0 * (M + N * tan(lat_rad) * (A*A/2 + (5 - T + 9*C + 4*C*C) * pow(A, 4) / 24 
                  + (61 - 58*T + T*T + 600*C - 330*e*e / (1 - e*e)) * pow(A, 6) / 720)) + N0;

    // Điều chỉnh giá trị y cho các tọa độ ở bán cầu Nam
    if (lat < 0) {
        y += 10000000.0;
    }

    return y;
}

// Hàm tính toán vĩ độ từ tọa độ UTM
float get_latitude(float x, float y, int zone, bool isSouthernHemisphere) {
    if (isSouthernHemisphere) {
        y -= 10000000.0f; // Điều chỉnh cho bán cầu Nam
    }

    float e = sqrtf(1 - powf(1 - f, 2)); // Độ lệch tâm
    float e1sq = e * e / (1 - e * e);
    float M = y / k0;
    float mu = M / (r_long * (1 - e * e / 4 - 3 * e * e * e * e / 64 - 5 * e * e * e * e * e * e / 256));

    float phi1Rad = mu + (3 * e / 2 - 27 * e * e * e / 32) * sinf(2 * mu) 
                      + (21 * e * e / 16 - 55 * e * e * e * e / 32) * sinf(4 * mu)
                      + (151 * e * e * e / 96) * sinf(6 * mu);
    float N1 = r_long / sqrtf(1 - e * e * sinf(phi1Rad) * sinf(phi1Rad));
    float T1 = tanf(phi1Rad) * tanf(phi1Rad);
    float C1 = e1sq * cosf(phi1Rad) * cosf(phi1Rad);
    float R1 = r_long * (1 - e * e) / powf(1 - e * e * sinf(phi1Rad) * sinf(phi1Rad), 1.5f);
    float D = (x - E0) / (N1 * k0);

    float lat = phi1Rad - (N1 * tanf(phi1Rad) / R1) * (D * D / 2 - (5 + 3 * T1 + 10 * C1 - 4 * C1 * C1 - 9 * e1sq) * powf(D, 4) / 24
                   + (61 + 90 * T1 + 298 * C1 + 45 * T1 * T1 - 252 * e1sq - 3 * C1 * C1) * powf(D, 6) / 720);
    lat = lat * 180.0f / pi;

    return lat;
}

// Hàm tính toán kinh độ từ tọa độ UTM
float get_longitude(float x, float y, int zone, bool isSouthernHemisphere) {
    if (isSouthernHemisphere) {
        y -= 10000000.0f; // Điều chỉnh cho bán cầu Nam
    }

    float e = sqrtf(1 - powf(1 - f, 2)); // Độ lệch tâm
    float e1sq = e * e / (1 - e * e);
    float M = y / k0;
    float mu = M / (r_long * (1 - e * e / 4 - 3 * e * e * e * e / 64 - 5 * e * e * e * e * e * e / 256));

    float phi1Rad = mu + (3 * e / 2 - 27 * e * e * e / 32) * sinf(2 * mu) 
                      + (21 * e * e / 16 - 55 * e * e * e * e / 32) * sinf(4 * mu)
                      + (151 * e * e * e / 96) * sinf(6 * mu);
    float N1 = r_long / sqrtf(1 - e * e * sinf(phi1Rad) * sinf(phi1Rad));
    float T1 = tanf(phi1Rad) * tanf(phi1Rad);
    float C1 = e1sq * cosf(phi1Rad) * cosf(phi1Rad);
    float R1 = r_long * (1 - e * e) / powf(1 - e * e * sinf(phi1Rad) * sinf(phi1Rad), 1.5f);
    float D = (x - E0) / (N1 * k0);

    float lon = (D - (1 + 2 * T1 + C1) * powf(D, 3) / 6 
                   + (5 - 2 * C1 + 28 * T1 - 3 * C1 * C1 + 8 * e1sq + 24 * T1 * T1) * powf(D, 5) / 120) / cosf(phi1Rad);
    lon = lon * 180.0f / pi + (zone - 1) * 6 - 180 + 3;

    return lon;
}


  
//----------------------------------------------------------------------------    SETUP   -----------------------------------------------------------------------------
//-----------------------------------------------------------------------------***-----***----------------------------------------------------------------------------- 
void setup() {
  Serial.begin(115200);
  
  // MOTOR SETUP
  f_1.attach(pin_motor_1,1000,2000);    
  f_2.attach(pin_motor_2,1000,2000);   
  pinMode(pin_throttle, INPUT);         
  pinMode(pin_turn, INPUT); 

  // MPU6050 SETUP
  // mpu6050.begin();
  // mpu6050.calcGyroOffsets(true);

  // COMPASS SETUP
  compass.init();
  
  // GPS SETUP
  ss.begin(GPSBaud);

  // UDP WIFI
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}

//-----------------------------------------------------------------------------   LOOP    -----------------------------------------------------------------------------
//-----------------------------------------------------------------------------***-----***-----------------------------------------------------------------------------
void loop() {

  // ----------------------------------------------- READING -----------------------------------------------
    unsigned long currentMillis = millis();
    packetSize = Udp.parsePacket();
    char incomingPacket[512]; 
    
    if (packetSize) {
      int len = Udp.read(incomingPacket, 512);
      if (len > 0) {
        incomingPacket[len] = 0;
      }   
      Serial.printf("UDP packet contents: %s\n", incomingPacket);
      if ((incomingPacket[0] == 33)||(incomingPacket[0] == 35)) {
        sscanf(incomingPacket, "! %f %f %f %f %f %f %f %f %f", &path_type, &target_lat_1, &target_lng_1, &lat_center, &lng_center, &r_x, &r_y, &omega_x, &omega_y);
        Serial.printf("Received Path Type: %0.0f; ", path_type);
        Serial.printf("Received Coordinates: (%0.15f,%0.15f); (%0.15f,%0.15f); ", target_lat_1, target_lng_1, lat_center, lng_center);
        Serial.printf("Received rX: %f; rY: %f; omgX: %f; omgY: %f ", r_x, r_y, omega_x, omega_y);
        auto_mode = true;
        initial_x = get_utm_x(lat_center, lng_center);
        initial_y = get_utm_y(lat_center, lng_center);
        Serial.printf("x_center: %f; y_center; %f; ", initial_x, initial_y);
        arrival_start_time = millis();
        track_duration = 100;
      }
      else
       auto_mode = false;
    }
    Serial.print("Auto = ");
    Serial.print(auto_mode);
    Serial.print("; "); 
  
    // MPU6050 reading
      //mpu6050.update();

    // Compass reading
      compass.read();
      compass_reading = map(compass.getAzimuth(),0,180,180,0);
       
      //compass_reading = map(mpu6050.getAngleZ(),0,180,180,0);
      Serial.printf("Compass: %f; ", compass_reading);
      
    // GPS
      valid = false;
      while (ss.available() > 0)
        if (gps.encode(ss.read())) 
          if (gps.location.isValid()) {
            valid = true;
          usv_lat = gps.location.lat();
          usv_lng = gps.location.lng();
            
            // char gpsData[50];
            // snprintf(gpsData, sizeof(gpsData), "%0.15f,%0.15f", usv_lat, usv_lng);
            // Udp.beginPacket(qtIp, 12345); // IP và port UDP gửi cho của Qt
            // Udp.write(gpsData);
            // Udp.endPacket();
          }
          if (currentMillis - previousMillis >= interval) {
            // Cập nhật thời gian trước đó
            previousMillis = currentMillis;

            // Chuẩn bị dữ liệu GPS
            snprintf(gpsData, sizeof(gpsData), "%0.15f, %0.15f", usv_lat, usv_lng);

            // Gửi dữ liệu qua UDP
            Udp.beginPacket(qtIp, 12345); // IP và port UDP gửi cho của Qt
            Udp.write(gpsData);
            Udp.endPacket();
          }
          
      Serial.print("valid: "); 
      Serial.print(valid);
      Serial.print("; ");
      // Serial.printf("USV Position: (%0.15f, %0.15f); ", usv_lat, usv_lng);
      Serial.printf("USV Position: (");
      Serial.print(usv_lat, 15);
      Serial.print(", ");
      Serial.print(usv_lng, 15);
      Serial.println("); ");
    // ----------------------------------------------- CHOOSE TARGET -----------------------------------------------
      track_time = millis() - arrival_start_time;
      //float dt = track_time/ 1000;
      Serial.println(track_time);
      if (incomingPacket[0] == 35)
      {
        target_lat = target_lat_1;
        target_lng = target_lng_1;
      }

      if (incomingPacket[0] == 33)
      { 
        x_path = get_path_x (track_time, path_type, initial_x, r_x, omega_x);
        y_path = get_path_y (track_time, path_type, initial_y, r_y, omega_y);
        target_lat = target_lat_1 + get_latitude(x_path, y_path, 48, false);
        target_lng = target_lng_1 + get_longitude(x_path, y_path, 48, false);
      }
      // ----------------------------------------------- GET USV YAW -----------------------------------------------
      
      usv_yaw = wrap_2_pi(compass_reading * pi / 180);
      Serial.print("usv_yaw: ");
      Serial.print(usv_yaw);
      Serial.print("; "); 

      usv_velocity = gps.speed.kmph() / 3.6;
      Serial.printf("USV heading speed: %f; ", usv_velocity);
  
 
  // ----------------------------------------------- AUTO MODE -----------------------------------------------
  if ((incomingPacket[0] == 33)||(incomingPacket[0] == 35)) {
    // ---------------------------------- TARGET TRACKING MODE ----------------------------------
    
    if (incomingPacket[0] == 35) {
        x_error = get_utm_x(target_lat - usv_lat, target_lng - usv_lng); 
        y_error = get_utm_x(target_lat - usv_lat, target_lng - usv_lng); 
        desire_yaw = atan2(y_error, x_error);             // range -pi ~ pi    
        Serial.printf("yaw_d: %.0f; ", desire_yaw);
        yaw_error = wrap_2_pi(desire_yaw - usv_yaw);       // range -pi ~ pi                            
        Serial.printf("yaw_e: %.0f; ", yaw_error);
       
      // ------------ GET DISTANCE AND VELOCITY U ------------

        distance = delta_2(x_error,y_error);
        if (distance >= 5)  approach = 1;
        else approach = 0;
        u_high = 0.8; 
        u_low = 0.4;
        if (abs(yaw_error) <= pi/6) {
          high_speed = 1;
          maneuver = 0;
        }
        else {
          high_speed = 0;
          maneuver = 1;
        } 
        u_control = approach * (high_speed * u_high + maneuver * u_low);
    }

    //
    // ----------------------------------* TRAJECTORY TRACKING MODE *---------------------------------- 
    //
    if (incomingPacket[0] == 33) {
        v_x_desire = v_x_path (track_time, path_type, initial_x, r_x, omega_x);
        v_y_desire = v_y_path (track_time, path_type, initial_y, r_y, omega_y);
        path_yaw = atan2(v_x_desire, v_y_desire);   
        Serial.printf("path_yaw: %f; ", path_yaw);
        Serial.printf("x path: %f; ", x_path); 
        Serial.printf("y path: %f; ", y_path);

      // ------------ GET TRACKING ERROR ------------

        x_error = get_utm_x(target_lat - usv_lat, target_lng - usv_lng); 
        y_error = get_utm_x(target_lat - usv_lat, target_lng - usv_lng); 
        along_e = cos(path_yaw)*x_error + sin(path_yaw)*y_error;
        cross_e = -sin(path_yaw)*x_error + cos(path_yaw)*y_error;

      // ------------ GET YAW ERROR ------------
        temp = atan2(cross_e,look_ahead);
        Serial.printf("atan2(cross_e,look_ahead) = %.0f; ", temp);         
        desire_yaw = wrap_2_pi(atan2(y_error, x_error));       
        Serial.print("yaw_d: ; ");
        Serial.print(desire_yaw);
        Serial.println("; ");
        yaw_error = wrap_2_pi(desire_yaw - usv_yaw);  

        float yaw_error_rate = (yaw_error - previous_yaw_error) / 1; 

        Serial.printf("yaw_e: %f; ", yaw_error);
        U_desire = sqrt(sqr(v_x_desire) + sqr(v_y_desire));        
        // U_desire = 0.5;
        U_pseudo = ((U_desire - 0.0000012 * along_e) * sqrt(sqr(cross_e) + sqr(look_ahead))) / look_ahead;
        u_high = U_pseudo;
        u_low = 0.8;
        approach = 1;
        if (abs(yaw_error) <= pi/6) u_control = u_high * approach;
        else u_control = u_low * approach;
        }
        Serial.printf("u_control: %f; ", u_control);

  // ------------ DESIGN AUTO THRUST AND MOMENT ------------

    u_error = u_control - usv_velocity;
    
    if (track_time*0.001 <= track_duration) {
      if (currentMillis - previousMillis2 >= interval) {
            // Cập nhật thời gian trước đó
        previousMillis2= currentMillis;
        //integral_u += u_error * track_time * 0.001;
        integral_u += u_error;
        //integral_yaw += wrap_2_pi(yaw_error * track_time * 0.001);
        integral_yaw += yaw_error;
        thrust = Kp_u * u_error + Ki_u * integral_u + Kd_u * derivative_u;
        moment = Kp_yaw * yaw_error + Ki_yaw * integral_yaw + Kd_yaw * derivative_yaw;
      }
    } else {
      thrust = 0;
      moment = 0;
      integral_u = 0;
      integral_yaw = 0;
    }
    Serial.print("delta_u: "); 
    Serial.print(u_error);
    Serial.println("; ");
    Serial.printf("integral_u: %f; integral_yaw: %f ", integral_u, integral_yaw);
    Serial.print("Thrust: "); 
    Serial.print(thrust);
    Serial.print("; ");
    Serial.print("Moment: "); 
    Serial.print(moment);
    Serial.println("; ");
    //Serial.printf("thrust: %.0f; moment: %.0f; ", thrust, moment);

    // ------------CONTROL MOTOR------------

    motor_control(thrust, moment);
    //delay(1000);
  }  

  // ----------------------------------------------- MANUAL MODE -----------------------------------------------
  else if (strncmp(incomingPacket, "PWM", 3) == 0) {
      sscanf(incomingPacket, "PWM:%d,%d", &speed_1, &speed_2);
      f_1.write(speed_1);
      if (speed_2 > 0) f_2.write(speed_2 + 10);
      else f_2.write(speed_2);
      Serial.printf("PWM Values: pwm1 = %d, pwm2 = %d\n", speed_1, speed_2);
    }
}


