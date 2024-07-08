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

Thuật toán đã mô phỏng ở chương 3 được lập trình và gỡ lỗi lên vi điều khiển bằng ngôn ngữ lập trình C++, qua phần mềm Arduino IDE. Cấu hình các chuẩn giao tiếp với ngoại vi và đọc dữ liệu từ thanh ghi trên các cảm biến theo hướng dẫn của datasheet. Phần giao diện điều khiển và bản đồ được thiết lập trong phần mềm QT Creator cũng bằng ngôn ngữ lập trình C++ và qml như hình dưới.

#![image](https://github.com/BinhCornelius/USV-Automatic-Control/assets/170936970/e1191e0f-05bf-402e-94b0-09b4f7355351)

Do vấn đề kinh phí nên không thể có mô hình tàu với kích thước vài tải trọng như tiêu đề của đồ án này, vì thế mô hình tàu thực thế đã thu nhỏ lại đã được hoàn thiện nhằm mục đích chạy thử thuật toán điều khiển bám quỹ đạo đã được trình bày ở chương II, hình ảnh của mô hình tàu thu nhỏ được thể hiện ở hình 5.27.
