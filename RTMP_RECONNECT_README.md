# RTMP Reconnect Feature Implementation

## Tổng quan

Đã implement tính năng tự động kết nối lại (reconnect) cho RTMP protocol trong FFmpeg khi kết nối bị mất hoặc gặp lỗi. Thay vì báo lỗi ngay lập tức, hệ thống sẽ tự động thử kết nối lại trong vòng 60 giây.

## Quy trình Input và Nhận dữ liệu RTMP của FFmpeg

### 1. Khởi tạo kết nối (`rtmp_open`)
- Phân tích URL và tham số kết nối
- Tạo kết nối TCP/TLS/HTTP tunneling tùy theo protocol
- Thực hiện RTMP handshake với server
- Gửi lệnh connect và thiết lập stream
- Khởi tạo buffer FLV để đọc dữ liệu

### 2. Đọc dữ liệu (`rtmp_read`)
- Kiểm tra dữ liệu trong buffer FLV hiện tại
- Nếu buffer trống, gọi `get_packet()` để lấy packet mới
- Copy dữ liệu từ buffer ra output buffer

### 3. Lấy packet từ server (`get_packet`)
- Gọi `ff_rtmp_packet_read()` để đọc packet RTMP từ network
- Xử lý các loại packet khác nhau:
  - Video packets (RTMP_PT_VIDEO)
  - Audio packets (RTMP_PT_AUDIO)  
  - Metadata packets (RTMP_PT_METADATA)
  - Control packets (RTMP_PT_NOTIFY)
- Chuyển đổi dữ liệu thành format FLV
- **ĐIỂM QUAN TRỌNG**: Đây là nơi phát hiện lỗi kết nối

### 4. Đọc packet từ network (`ff_rtmp_packet_read`)
- Đọc header của packet
- Đọc payload theo chunk size được cấu hình
- **ĐIỂM QUAN TRỌNG**: Đây là nơi xảy ra lỗi I/O khi kết nối bị mất

## Tính năng Reconnect được implement

### 1. Cấu trúc dữ liệu mới trong RTMPContext

```c
int           reconnect_streamed;         ///< auto reconnect if connection is broken
int           reconnect_delay_max;        ///< max reconnect delay in ms (default 60s)
int           reconnect_delay;            ///< current reconnect delay in ms
int64_t       reconnect_at;               ///< timestamp when to reconnect
int           reconnect_count;            ///< number of reconnect attempts
int           reconnect_enabled;          ///< reconnect feature enabled
char          *original_uri;              ///< original URI for reconnect
```

### 2. Tùy chọn mới

- `rtmp_reconnect`: Bật/tắt tính năng reconnect (default: false)
- `rtmp_reconnect_streamed`: Tự động reconnect cho streamed mode (default: false) 
- `rtmp_reconnect_delay_max`: Thời gian tối đa cho phép reconnect (default: 60000ms = 60s)

### 3. Logic Reconnect

#### `reset_connection_state()`
- Reset tất cả trạng thái kết nối về ban đầu
- Xóa packet history và tracked methods
- Reset stream state và metadata flags

#### `attempt_reconnect()`
- Kiểm tra timeout (60 giây)
- Implement exponential backoff: 1s, 2s, 4s, 8s, tối đa 10s
- Đóng kết nối cũ và tạo kết nối mới
- Thực hiện lại handshake và connect sequence
- Log chi tiết quá trình reconnect

#### Xử lý lỗi trong `get_packet()`
```c
if (ret <= 0) {
    if (ret == 0) {
        return AVERROR(EAGAIN);
    } else {
        // Connection error, try to reconnect if enabled
        if ((rt->reconnect_enabled || rt->reconnect_streamed) && rt->is_input) {
            av_log(s, AV_LOG_WARNING, "Connection lost, attempting to reconnect\n");
            if (attempt_reconnect(s) >= 0) {
                continue; // Retry with new connection
            }
        }
        return AVERROR(EIO);
    }
}
```

### 4. Cách sử dụng

```bash
# Bật reconnect cho input stream
ffmpeg -rtmp_reconnect 1 -i rtmp://server/live/stream output.mp4

# Bật reconnect với timeout tùy chỉnh (30 giây)
ffmpeg -rtmp_reconnect 1 -rtmp_reconnect_delay_max 30000 -i rtmp://server/live/stream output.mp4

# Bật reconnect cho streamed mode
ffmpeg -rtmp_reconnect_streamed 1 -i rtmp://server/live/stream output.mp4
```

## Điểm mạnh của implementation

1. **Transparent**: Người dùng không cần thay đổi workflow, chỉ thêm option
2. **Configurable**: Có thể tùy chỉnh timeout và mode reconnect
3. **Robust**: Exponential backoff tránh spam server
4. **Logging**: Chi tiết log để debug
5. **Memory safe**: Proper cleanup của resources khi reconnect

## Limitations

1. Chỉ hoạt động với input streams (`rt->is_input`)
2. Không maintain timestamp continuity (có thể skip một số frame)
3. Không hoạt động với listen mode
4. Phụ thuộc vào server hỗ trợ reconnect từ cùng một client

## Testing

Để test tính năng:
1. Start RTMP stream
2. Ngắt kết nối network tạm thời  
3. Kiểm tra log để thấy reconnect attempts
4. Verify stream tiếp tục sau khi network restore
