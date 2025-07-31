# RTMP Reconnect Implementation Documentation

## Tổng quan

Implementation này thêm tính năng tự động kết nối lại (auto-reconnect) cho RTMP input streams trong FFmpeg. Khi kết nối RTMP bị mất hoặc gặp lỗi, hệ thống sẽ tự động thử kết nối lại thay vì báo lỗi ngay lập tức.

## Quy trình Input và Nhận dữ liệu RTMP

### 1. Quy trình ban đầu:
```
rtmp_open() → rtmp_handshake() → gen_connect() → get_packet() → rtmp_read()
```

### 2. Các hàm chính:
- **`rtmp_open()`**: Thiết lập kết nối RTMP, handshake và khởi tạo
- **`rtmp_read()`**: Đọc dữ liệu từ buffer FLV để cung cấp cho demuxer
- **`get_packet()`**: Đọc packet RTMP từ stream và xử lý
- **`ff_rtmp_packet_read()`**: Đọc packet thực tế từ TCP stream

### 3. Luồng dữ liệu:
```
TCP Stream → RTMP Packets → FLV Buffer → Demuxer → Audio/Video Streams
```

## Tính năng Reconnect

### Các field mới được thêm vào RTMPContext:
```c
int           reconnect_enabled;     // enable auto reconnect on error
int           reconnect_timeout;     // timeout in seconds for reconnect attempts  
int64_t       reconnect_start_time;  // timestamp when reconnect started
int           reconnect_attempts;    // number of reconnect attempts made
int           max_reconnect_attempts; // maximum reconnect attempts before giving up
```

### Các options mới:
- `rtmp_reconnect`: Bật/tắt tính năng reconnect (mặc định: 0 - tắt)
- `rtmp_reconnect_timeout`: Thời gian tối đa để thử reconnect (mặc định: 60 giây)
- `rtmp_max_reconnect_attempts`: Số lần thử tối đa (mặc định: 10 lần)

### Hàm `rtmp_do_reconnect()`:
1. Kiểm tra điều kiện reconnect (enabled, timeout, max attempts)
2. Đóng kết nối cũ
3. Parse lại URL
4. Tạo kết nối TCP mới
5. Thực hiện handshake
6. Kết nối lại với cùng app và playpath
7. Đợi kết nối được thiết lập

### Logic xử lý lỗi:
- **Trong `get_packet()`**: Khi `ff_rtmp_packet_read()` trả về lỗi, gọi `rtmp_do_reconnect()`
- **Trong `rtmp_read()`**: Xử lý lỗi kết nối một cách graceful
- **Exponential backoff**: Delay tăng dần giữa các lần thử (2s, 4s, 6s, 8s, max 10s)

## Ưu điểm của Implementation

### 1. Bảo toàn metadata:
- Video codec, resolution, framerate không thay đổi
- Audio codec, sample rate, channels không thay đổi  
- Chỉ tiếp tục đọc dữ liệu stream, không reset decoder

### 2. Transparent reconnection:
- Application layer không biết về reconnection
- Demuxer tiếp tục nhận dữ liệu như bình thường
- Không có gap hoặc discontinuity trong timestamp

### 3. Robust error handling:
- Phân biệt các loại lỗi (network vs protocol)
- Retry logic với backoff strategy
- Timeout protection để tránh infinite loop

### 4. Logging và monitoring:
- Log chi tiết các lần thử reconnect
- Thông báo khi reconnect thành công/thất bại
- Metrics về số lần retry và thời gian

## Cách sử dụng

### Basic usage:
```bash
ffmpeg -rtmp_reconnect 1 -i rtmp://live.example.com/app/stream output.mp4
```

### Advanced usage:
```bash
ffmpeg -rtmp_reconnect 1 \
       -rtmp_reconnect_timeout 120 \
       -rtmp_max_reconnect_attempts 5 \
       -i rtmp://live.example.com/app/stream \
       output.mp4
```

### Cho live streaming không ổn định:
```bash
ffmpeg -rtmp_reconnect 1 \
       -rtmp_reconnect_timeout 30 \
       -rtmp_max_reconnect_attempts 20 \
       -i rtmp://unstable.stream.com/live/key \
       -c copy output.mp4
```

## Test cases

### 1. Connection drop test:
- Mô phỏng network failure
- Kiểm tra reconnect behavior
- Verify data continuity

### 2. Server restart test:
- RTMP server restart
- Kiểm tra reconnection sau khi server up lại
- Verify metadata preservation

### 3. Timeout test:
- Long network outage
- Kiểm tra timeout logic
- Verify graceful fallback

## Compatibility

- Chỉ hoạt động với RTMP input streams
- Không ảnh hưởng đến RTMP output streams
- Backward compatible - các application cũ vẫn hoạt động bình thường
- Thread-safe cho multi-threading environments

## Performance Impact

- Minimal overhead khi không có lỗi
- Memory usage không tăng đáng kể
- CPU usage chỉ tăng khi reconnecting
- Network bandwidth tương tự như kết nối bình thường
