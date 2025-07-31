#!/bin/bash

# Test script for RTMP reconnect functionality
# This script demonstrates how to use the new reconnect options

echo "RTMP Reconnect Test Script"
echo "========================="
echo

# Test 1: Basic reconnect
echo "Test 1: Basic reconnect (60s timeout, 10 attempts)"
echo "Command: ffmpeg -rtmp_reconnect 1 -i rtmp://test.stream/live/key -t 30 -f null -"
echo

# Test 2: Custom settings
echo "Test 2: Custom reconnect settings (30s timeout, 5 attempts)"
echo "Command: ffmpeg -rtmp_reconnect 1 -rtmp_reconnect_timeout 30 -rtmp_max_reconnect_attempts 5 -i rtmp://test.stream/live/key -t 30 -f null -"
echo

# Test 3: Aggressive reconnect for unstable connections
echo "Test 3: Aggressive reconnect for unstable connections"
echo "Command: ffmpeg -rtmp_reconnect 1 -rtmp_reconnect_timeout 120 -rtmp_max_reconnect_attempts 20 -i rtmp://unstable.stream/live/key -c copy output.mp4"
echo

echo "Notes:"
echo "- The stream will automatically reconnect when connection is lost"
echo "- Metadata (video/audio format) is preserved across reconnections"
echo "- Use higher max_reconnect_attempts for very unstable connections"
echo "- Monitor logs for reconnection attempts and status"
