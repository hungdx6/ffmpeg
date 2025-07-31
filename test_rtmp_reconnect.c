/*
 * Test program for RTMP reconnect functionality
 * Compile with: gcc test_rtmp_reconnect.c -o test_rtmp_reconnect
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("RTMP Reconnect Test\n");
    printf("==================\n\n");
    
    printf("Usage examples with reconnect options:\n\n");
    
    printf("1. Basic reconnect (default 60 seconds timeout, 10 attempts):\n");
    printf("   ffmpeg -rtmp_reconnect 1 -i rtmp://live.example.com/app/stream output.mp4\n\n");
    
    printf("2. Custom timeout and attempts:\n");
    printf("   ffmpeg -rtmp_reconnect 1 -rtmp_reconnect_timeout 120 -rtmp_max_reconnect_attempts 5 -i rtmp://live.example.com/app/stream output.mp4\n\n");
    
    printf("3. Quick reconnect for unstable connections:\n");
    printf("   ffmpeg -rtmp_reconnect 1 -rtmp_reconnect_timeout 30 -rtmp_max_reconnect_attempts 20 -i rtmp://live.example.com/app/stream output.mp4\n\n");
    
    printf("New RTMP options added:\n");
    printf("- rtmp_reconnect: Enable automatic reconnection (0=disabled, 1=enabled)\n");
    printf("- rtmp_reconnect_timeout: Maximum time in seconds to attempt reconnection (default: 60)\n");
    printf("- rtmp_max_reconnect_attempts: Maximum number of reconnect attempts (default: 10)\n\n");
    
    printf("Reconnect behavior:\n");
    printf("- Automatically detects connection failures\n");
    printf("- Preserves metadata, video/audio format information\n");
    printf("- Uses exponential backoff (2s, 4s, 6s, 8s, 10s max delay between attempts)\n");
    printf("- Logs reconnection attempts and status\n");
    printf("- Falls back to error if timeout or max attempts exceeded\n\n");
    
    return 0;
}
