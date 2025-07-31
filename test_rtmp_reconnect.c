/**
 * Test file để kiểm tra syntax của RTMP reconnect feature
 * Chỉ để kiểm tra compile, không chạy được
 */

// Minimal includes để test syntax
typedef struct URLContext URLContext;
typedef struct RTMPContext RTMPContext;
typedef struct RTMPPacket RTMPPacket;
typedef struct AVDictionary AVDictionary;

// Test các function signatures đã thêm
static void reset_connection_state(RTMPContext *rt);
static int attempt_reconnect(URLContext *s);
static int rtmp_open(URLContext *s, const char *uri, int flags, AVDictionary **opts);

int main() {
    // Test code syntax
    return 0;
}
