
# ♟️ Sephirah - Chess AI Engine

**Sephirah** là một Chess Engine (động cơ cờ vua) hiệu năng cao được phát triển bằng ngôn ngữ C++. Đây là sản phẩm thuộc đồ án môn học **Nhập môn Trí tuệ Nhân tạo (Introduction to AI)**. Dự án tập trung vào việc cài đặt các thuật toán cốt lõi trong Lý thuyết Trò chơi (Game Theory) để xây dựng một đối thủ máy tính có khả năng thi đấu cạnh tranh.

Sephirah hỗ trợ giao thức **UCI (Universal Chess Interface)**, cho phép kết nối dễ dàng với các phần mềm giao diện cờ vua (GUI) phổ biến hiện nay như Arena, BanksiaGUI, CuteChess,...

---

## 👥 Thành viên thực hiện

| STT | Họ và tên | Mã số sinh viên |
|:---:|:---|:---:|
| 1 | **Nguyễn Đình Đăng Dương** | 20230022 |
| 2 | **Ngô Vũ Minh** | 20230084 |
| 3 | **Dương Thanh Minh** | 20230047 |
| 4 | **Mai Lê Phú Quang** | 20230058 |

---

## 🧠 Các thuật toán & Tính năng nổi bật

Dự án áp dụng các kỹ thuật tối ưu hóa và cấu trúc dữ liệu tiên tiến để tăng tốc độ tính toán và khả năng đánh giá thế cờ:

*   **Biểu diễn bàn cờ (Board Representation)**: Sử dụng **Bitboards** để quản lý trạng thái bàn cờ và sinh nước đi (Move Generation) với tốc độ cực nhanh.
*   **Thuật toán tìm kiếm (Search Algorithm)**:
    *   **Minimax** kết hợp với **Alpha-Beta Pruning** để cắt tỉa các nhánh cây tìm kiếm không cần thiết.
    *   **Iterative Deepening** (Làm sâu dần) để quản lý thời gian suy nghĩ hiệu quả.
    *   **Quiescence Search** (Tìm kiếm tĩnh) để giải quyết hiệu ứng chân trời (horizon effect) trong các thế cờ biến động mạnh.
*   **Sắp xếp nước đi (Move Ordering)**:
    *   Tối ưu hóa thứ tự duyệt bằng kỹ thuật **MVV-LVA** (Most Valuable Victim - Least Valuable Aggressor).
    *   Sử dụng **Killer Heuristic** và **History Heuristic**.
*   **Bảng chuyển vị (Transposition Table)**:
    *   Sử dụng **Zobrist Hashing** để lưu trữ và truy xuất các thế cờ đã được duyệt qua, tránh tính toán lặp lại.

---

## ⚙️ Yêu cầu hệ thống

*   **Trình biên dịch C++**: Hỗ trợ chuẩn C++17 trở lên (GCC, Clang, hoặc MSVC).
*   **CMake**: Phiên bản 3.10 trở lên.
*   **Google Test (GTest)**: (Tùy chọn) Chỉ cần thiết nếu bạn muốn chạy các bài kiểm thử unit test.

---

## 🛠️ Hướng dẫn cài đặt & Biên dịch

### 1. Clone dự án
```bash
git clone https://github.com/tht2005/Sephirah.git
cd Sephirah
```

### 2. Biên dịch (Build)

#### 🪟 Đối với Windows (Sử dụng MinGW)

1.  Tạo thư mục build:
    ```bash
    mkdir build
    cd build
    ```
2.  Tạo Makefile và cấu hình biên dịch (Release mode để tối ưu tốc độ):
    ```bash
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
    ```
    > **Lưu ý:** Nếu máy bạn chưa cài đặt thư viện **GTest** và gặp lỗi cấu hình, hãy mở file CMakeLists.txt ở thư mục gốc, tìm dòng `add_subdirectory(tests)` và thêm dấu `#` vào phía trước để tắt nó đi (`# add_subdirectory(tests)`). Sau đó chạy lại lệnh trên.

3.  Tiến hành build:
    ```bash
    cmake --build .
    ```
    Sau khi hoàn tất, file thực thi `sephirah.exe` sẽ nằm trong thư mục src.

#### 🐧 Đối với Linux / macOS

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
File thực thi `sephirah` sẽ được tạo trong thư mục src.

---

## 🎮 Hướng dẫn sử dụng

Sephirah là một chương trình dạng console (dòng lệnh) giao tiếp qua giao thức UCI. Nó không có giao diện đồ họa riêng mà cần chạy thông qua console hoặc một Chess GUI.

### Cách 1: Chạy trực tiếp trên Console (Dành cho Debug)

Bạn có thể chạy engine và nhập các lệnh UCI thủ công:

```bash
./src/sephirah
```

Một số lệnh cơ bản:
*   `uci`: Khởi động và hiển thị thông tin engine.
*   `isready`: Kiểm tra trạng thái sẵn sàng.
*   `position startpos moves e2e4`: Đặt bàn cờ ở vị trí bắt đầu và đi nước e2-e4.
*   `go depth 6`: Yêu cầu máy tính toán nước đi tốt nhất với độ sâu 6.

### Cách 2: Sử dụng với Chess GUI (Khuyên dùng)

Để xem bot thi đấu trực quan trên bàn cờ:

1.  Tải một phần mềm Chess GUI (ví dụ: [Arena](http://www.playwitharena.de/), [BanksiaGUI](https://banksiagui.com/), hoặc [CuteChess](https://cutechess.com/)).
2.  Vào phần cài đặt Engine (Engine Management -> Install New Engine).
3.  Trỏ đường dẫn đến file thực thi `sephirah.exe` vừa build được.
4.  Bắt đầu ván đấu mới và chọn **Sephirah** làm đối thủ.

---

&copy; 2024 - 2025 Sephirah Chess Engine Project.
