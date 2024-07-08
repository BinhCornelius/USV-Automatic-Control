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

# ![image](https://github.com/BinhCornelius/USV-Automatic-Control/assets/170936970/e1191e0f-05bf-402e-94b0-09b4f7355351)

Do vấn đề kinh phí nên không thể có mô hình tàu với kích thước vài tải trọng như tiêu đề của đồ án này, vì thế mô hình tàu thực thế đã thu nhỏ lại đã được hoàn thiện nhằm mục đích chạy thử thuật toán điều khiển bám quỹ đạo đã được trình bày ở chương II, hình ảnh của mô hình tàu thu nhỏ được thể hiện ở hình dưới.

# ![image](https://github.com/BinhCornelius/USV-Automatic-Control/assets/170936970/eab4599e-2bb7-4ee1-b514-23c45b8b88ea)

Để chuẩn bị chạy tàu, trạm điều khiển mặt đất được thiết lâp như hình 5.29. Trong đó, Laptop cá nhân đóng vai trò là trạm điều khiển mặt đất và ESP8266 sẽ giao tiếp với nhau bằng giao tiếp UDP thông qua wifi phát bởi cục router.

# ![image](https://github.com/BinhCornelius/USV-Automatic-Control/assets/170936970/2b23b75c-0202-4ca5-8603-d3a8e5330034)

# Chạy tàu

A. Chế độ thủ công  

Tạo một chương trình trên Qt để tạo các nút di chuyển cho tàu USV. Khi nhấn các nút “W”, “A”, “S”, “D” và “Q” trên bàn phím laptop hay là trạm điều khiển thì tàu USV sẽ di chuyển, các chức năng gán cho các phím này lần lượt là: Tăng ga, rẽ trái, giảm ga, rẽ phải và dừng hẳn. 
Chương trình Qt này có cơ chế hoạt động như sau: Khi nhấn các nút điều khiển, chương trình sẽ bắt đầu gửi tín hiệu PWM cho ESP8266 bằng giao thức UDP, giá trị PWM sẽ thay đổi trong khoảng từ 0 đến 199 tuỳ vào nút điều khiển:
+ Khi nhấn giữ hoặc nháy nhiều lần nút “W”, chương trình sẽ gửi 2 tín hiệu PWM1 và PWM2 tăng dần từ 0 đến 199 với bước là 1 cho ESP8266. 2 tính hiệu này có giá trị tăng đều với nhau từ đó 2 động cơ tăng tốc cùng nhau  và quay cùng tốc độ.
+ Khi nhấn giữ “A” tín hiệu PWM2 tăng và PWM1 giảm giá trị với bước là 1. Khi đó động cơ 2 sẽ quay nhanh và tạo lực đẩy lớn hơn động cơ 1, điều nãy sẽ tạu cho USV 1 moment quay tạo ra chuyển động rẽ trái.
+ Ngược lại, Khi nhấn giữ “D” tín hiệu PWM1 tăng và PWM2 giảm giá trị với bước là 1. Khi đó động cơ 1 sẽ quay nhanh và tạo lực đẩy lớn hơn động cơ 2, điều nãy sẽ tạu cho USV 1 moment quay tạo ra chuyển động rẽ phải.
+ Khi nhấn giữ “S”, chương trình sẽ gửi 2 tín hiệu PWM1 và PWM2 giảm dần dần từ giá trị ban đầu về 0 với bước là 1. 2 tính hiệu này có giá trị giảm đều với nhau từ đó 2 động cơ giảm tốc cùng nhau và quay cùng tốc độ cho đến lúc dừng hẳn.
+ Nút “Q” là nút dừng khẩn cấp. Khi nhấn “Q” thì các giá trị PWM1 = 0 và PWM2 = 0 sẽ được gửi đi và khiến tàu dừng hẳn.
Chế độ chạy thủ công được thử nghiệm bằng cách lái tàu chạy trên đài hồ Tiền như hình dưới.

# ![image](https://github.com/BinhCornelius/USV-Automatic-Control/assets/170936970/586ada2c-1343-48c0-9418-2e9e4ac23ce2)

B. Chế độ tự động 

Chạy chế độ tự động với quỹ đạo đường thẳng va đường tròn trên hồ Tiền. Từ chương trình Qt thiết lập trước đó, công cụ Maps and Navigation của Qt được thêm vào và thiết lập để tạo bản đồ, sau đó thiết kế các ô nhập các thông số: loại đường (1 là đường thẳng, 2 là hình sin, 3 là đường tròn, 4 là hình xoắn ốc – phần này được lập trình trên Arduino IDE), bán kính theo x và y, và omega theo x và y là tốc độ góc của chuyển dộng tròn. 
Đầu tiên, quỹ đạo đường thẳng sẽ được thử nghiệm, chọn 2 điểm là điểm đầu và cuối của quỹ đạo đường thẳng. Các điểm chọn trên bản đồ bằng cách click chuột sẽ để lại chấm tròn đen như hình dưới. Nút “Send Coodinates” được thiết kế để gửi toạ độ và các thông số khác qua cho ESP8266 và Nút Enable Key Control để chuyển sang chế độ thủ công. 

# ![image](https://github.com/BinhCornelius/USV-Automatic-Control/assets/170936970/34bc8b35-7fe0-49ed-932f-82fa4a91d6ae)

Sau khi nhấn “Send Coordinates” tài sẽ bắt đầu chạy theo quỹ đạo đường thẳng đã được lập trình sẵn. Đặt thời gian chạy của tàu là 40s, trong quá trình tàu chạy theo quỹ đạo, vị trí của tàu sẽ được tổng hợp và gửi qua cho MATLAB sau mỗi 1s, tổng hợp lại các quỹ đạo đó, ta có được quỹ đạo di chuyển thực tế của USV như hình dưới. Trong hình, đường nét đứt thể hiện quỹ đạo mong muốn và đường nét liều thể hiện quỹ đạo tàu chạy thực tế.

# ![image](https://github.com/BinhCornelius/USV-Automatic-Control/assets/170936970/24faf5b2-f440-44a3-ab84-6093f59d2013)

Dựa và đồ thị quỹ đạo có thể thấy rằng ban đầu tàu USV bám khá sát quỹ đạo đường thẳng, nhưng khi đến gần cuối quỹ đạo thì tàu bị lệch hướng và phải vòng lại để đến điểm cuối của quỹ đạo. Nhìn chung, quỹ đạo di chuyển thực tế bám khá sát so với quỹ đạo đầu vào.
Sau đó, tàu USV sẽ được lập trình để chạy tác vụ bám theo quỹ đạo tròn, trong giao diện bản đồ Qt nhập loại đường là 3 – đường tròn, bán kính quỹ đạo là 2.5m và omega là 1, cuối cùng sẽ chọn tâm quỹ đạo trên bản đồ. Có thể biếu diễn phương trình quỹ đạo đầu vào như phương trình (5.1), đặt thời gian chạy (track duration) là 100s. Sau khi “Send Coordinates”, tàu sẽ tiến hành chạy bám theo quỹ đạo đường tròn như hình 5.35. Khi tàu hoàn thành quỹ đạo, đồ thị quỹ đạo được thể hiện như hình dưới.
█({█(x_d (t)=x_center+2.5 sin⁡0.25t@y_d (t)=y_center+2.5 cos⁡0.25t )┤#(5.1) )






