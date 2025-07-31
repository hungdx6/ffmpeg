# RTMP Reconnect Feature - Summary

## Mô tả tính năng

Đã thành công implement tính năng **tự động kết nối lại (auto-reconnect)** cho RTMP protocol trong FFmpeg. Khi kết nối RTMP bị ngắt hoặc gặp lỗi, thay vì dừng ngay lập tức và báo lỗi, FFmpeg sẽ:

1. **Tự động thử kết nối lại** trong vòng 60 giây (có thể cấu hình)
2. **Sử dụng exponential backoff** để tránh spam server: 1s → 2s → 4s → 8s → 10s (max)
3. **Log chi tiết** quá trình reconnect để người dùng theo dõi
4. **Chỉ báo lỗi** sau khi timeout (60s) hoặc không thể kết nối lại

## Quy trình xử lý RTMP Input của FFmpeg

### Luồng chính:
```
rtmp_open() → rtmp_read() → get_packet() → ff_rtmp_packet_read()
     ↓            ↓             ↓                    ↓
 Kết nối     Đọc buffer    Lấy packet        Đọc từ network
```

### Chi tiết từng bước:

1. **`rtmp_open()`**: 
   - Parse URL và tham số
   - Tạo TCP/TLS connection  
   - RTMP handshake
   - Connect to app/stream
   - Tạo FLV buffer

2. **`rtmp_read()`**:
   - Copy data từ FLV buffer
   - Gọi `get_packet()` khi buffer trống

3. **`get_packet()`**:
   - Đọc RTMP packet từ network
   - Parse và convert sang FLV format
   - **⚠️ Điểm phát hiện lỗi kết nối**

4. **`ff_rtmp_packet_read()`**:
   - Low-level network I/O
   - **⚠️ Điểm xảy ra lỗi network**

## Cách sử dụng

### Bật tính năng reconnect:
```bash
# Cơ bản
ffmpeg -rtmp_reconnect 1 -i rtmp://server/live/stream output.mp4

# Với timeout tùy chỉnh (30 giây)  
ffmpeg -rtmp_reconnect 1 -rtmp_reconnect_delay_max 30000 -i rtmp://server/live/stream output.mp4

# Cho streamed mode
ffmpeg -rtmp_reconnect_streamed 1 -i rtmp://server/live/stream output.mp4
```

### Tùy chọn mới:
- `rtmp_reconnect`: Bật/tắt reconnect (default: 0)
- `rtmp_reconnect_streamed`: Auto reconnect cho streamed (default: 0)  
- `rtmp_reconnect_delay_max`: Timeout tối đa ms (default: 60000)

## Kết quả

### Trước khi có tính năng:
```
[rtmp] Connection lost
[rtmp] Error reading packet
Input/output error
```

### Sau khi có tính năng:
```
[rtmp] Connection lost, attempting to reconnect
[rtmp] Reconnect attempt 1, waiting 1000 ms
[rtmp] Successfully reconnected to RTMP server
```

## Files đã được modify:

1. **`libavformat/rtmpproto.c`** - Core implementation
   - Thêm struct fields cho reconnect state
   - Implement `attempt_reconnect()` và `reset_connection_state()`
   - Modify `get_packet()` để handle lỗi và retry
   - Thêm options cho user config

## Lợi ích:

✅ **Robust streaming**: Tự động recover từ network issues  
✅ **User-friendly**: Transparent, chỉ cần thêm option  
✅ **Configurable**: Tuỳ chỉnh timeout và behavior  
✅ **Production-ready**: Proper error handling và resource cleanup  
✅ **No breaking changes**: Backward compatible 100%

## Test scenarios:

1. **Network hiccup**: Ngắt WiFi 5-10s → Auto reconnect
2. **Server restart**: RTMP server restart → Auto reconnect  
3. **Long disconnection**: Ngắt network > 60s → Timeout và exit
4. **Invalid server**: Server không tồn tại → Fail ngay lập tức

Implementation này giải quyết được vấn đề streaming bị ngắt do network không ổn định, đặc biệt hữu ích cho live streaming scenarios.
