# USV-Automatic-Control
This's my thesis project that I've done and finished in Aerospace Engineering Dept. group, Department of Vehicle and Energy Conversion Engineering, Hanoi University of Science and Technology.

# Sơ đồ mạch sử dụng trong dự án
Sơ đồ hệ thống điện tàu USV thực tế được thể hiện ở hình 5.25 và bao gồm:
+ Module Wifi – ESP8266 đóng vai trò làm vi điều khiển chính.
+ Module GPS dùng để xác định vị trí của USV.
+ Cảm biến gia tốc và con quay hồi chuyển MPU6050 - xác định góc hướng của USV.
+ 2 bộ điều tốc (ESC) và động cơ.
+ Pin và mạch cấp nguồn dự phòng.
+ Laptop cá nhân đóng vai trò là GCS.

# ![image](https://github.com/BinhCornelius/USV-Automatic-Control/assets/170936970/cbea1257-3ad7-4483-8d55-581ce79b84c4)

Dựa vào sơ đồ trên, hệ thống điện bao gồm các cảm biến và vi điều khiển sẽ được hàn trên phíp lỗ như hình dưới. Trong đó, Pin 3.7V sẽ được sử dụng để cấp nguồn riêng cho vi điều khiển và các cảm biến. Còn 2 động cơ và ESC sẽ được cấp nguồn bởi pin 3S 2200mAh

# ![image](https://github.com/BinhCornelius/USV-Automatic-Control/assets/170936970/87988b2f-7991-43b7-aeb4-97ae2fb2d670)

