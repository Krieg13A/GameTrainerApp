#include "VideoRecorder.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

VideoRecorder::VideoRecorder() 
    : isRecording(false), isInitialized(false), frameWidth(1280), frameHeight(720),
      fps(30.0), bufferSize(300), codec(cv::VideoWriter::fourcc('M', 'P', '4', 'V')) {
    outputPath = "./recordings/";
}

VideoRecorder::~VideoRecorder() {
    if (isRecording) {
        StopRecording();
    }
    ClearBuffer();
}

bool VideoRecorder::Initialize(int width, int height, double frameRate) {
    frameWidth = width;
    frameHeight = height;
    fps = frameRate;
    
    std::filesystem::create_directories(outputPath);
    
    isInitialized = true;
    std::cout << "[VideoRecorder] Initialized with resolution " << width << "x" << height 
              << " at " << fps << " FPS" << std::endl;
    return true;
}

void VideoRecorder::SetOutputPath(const std::string& path) {
    outputPath = path;
    if (!outputPath.empty() && outputPath.back() != '/') {
        outputPath += "/";
    }
    
    std::filesystem::create_directories(outputPath);
    
    std::cout << "[VideoRecorder] Output path set to " << outputPath << std::endl;
}

void VideoRecorder::SetBufferSize(int size) {
    bufferSize = std::max(1, size);
    std::cout << "[VideoRecorder] Buffer size set to " << bufferSize << " frames" << std::endl;
}

bool VideoRecorder::StartRecording(const std::string& filename) {
    if (!isInitialized) {
        std::cerr << "[VideoRecorder] Not initialized" << std::endl;
        return false;
    }
    
    if (isRecording) {
        std::cerr << "[VideoRecorder] Already recording" << std::endl;
        return false;
    }
    
    currentFilename = outputPath + filename;
    
    videoWriter.open(currentFilename, codec, fps, cv::Size(frameWidth, frameHeight));
    
    if (!videoWriter.isOpened()) {
        std::cerr << "[VideoRecorder] Failed to open video file: " << currentFilename << std::endl;
        return false;
    }
    
    isRecording = true;
    std::cout << "[VideoRecorder] Started recording to " << currentFilename << std::endl;
    
    return true;
}

void VideoRecorder::StopRecording() {
    if (!isRecording) {
        return;
    }
    
    WriteBufferedFrames();
    
    videoWriter.release();
    
    isRecording = false;
    std::cout << "[VideoRecorder] Stopped recording. File saved: " << currentFilename << std::endl;
}

bool VideoRecorder::IsRecording() const {
    return isRecording;
}

void VideoRecorder::AddFrame(const cv::Mat& frame, double timestamp) {
    if (!isRecording) {
        return;
    }
    
    cv::Mat resizedFrame;
    if (frame.cols != frameWidth || frame.rows != frameHeight) {
        cv::resize(frame, resizedFrame, cv::Size(frameWidth, frameHeight));
    } else {
        resizedFrame = frame.clone();
    }
    
    videoWriter.write(resizedFrame);
    
    AddFrameToBuffer(resizedFrame, timestamp);
}

void VideoRecorder::AddFrameWithEnemies(const cv::Mat& frame, double timestamp, const std::vector<cv::Point2f>& enemyPositions) {
    if (!isRecording) {
        return;
    }
    
    cv::Mat annotatedFrame = frame.clone();
    
    for (const auto& pos : enemyPositions) {
        cv::circle(annotatedFrame, pos, 5, cv::Scalar(0, 255, 0), -1);
        cv::circle(annotatedFrame, pos, 15, cv::Scalar(0, 255, 0), 2);
    }
    
    AddFrame(annotatedFrame, timestamp);
}

void VideoRecorder::AddFrameToBuffer(const cv::Mat& frame, double timestamp) {
    FrameBuffer bufferFrame;
    bufferFrame.frame = frame.clone();
    bufferFrame.timestamp = timestamp;
    
    frameBuffer.push(bufferFrame);
    
    while (frameBuffer.size() > bufferSize) {
        frameBuffer.pop();
    }
}

void VideoRecorder::WriteBufferedFrames() {
    std::cout << "[VideoRecorder] Writing " << frameBuffer.size() << " buffered frames" << std::endl;
    
    while (!frameBuffer.empty()) {
        const auto& bufferFrame = frameBuffer.front();
        videoWriter.write(bufferFrame.frame);
        frameBuffer.pop();
    }
}

void VideoRecorder::ClearBuffer() {
    while (!frameBuffer.empty()) {
        frameBuffer.pop();
    }
}

void VideoRecorder::StartBuffering() {
    std::cout << "[VideoRecorder] Started buffering frames" << std::endl;
}

void VideoRecorder::StopBuffering() {
    std::cout << "[VideoRecorder] Stopped buffering frames" << std::endl;
    WriteBufferedFrames();
}

size_t VideoRecorder::GetBufferSize() const {
    return frameBuffer.size();
}

std::string VideoRecorder::GenerateFilename(const std::string& prefix, double timestamp) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time);
    
    std::ostringstream oss;
    oss << prefix << "_"
        << std::put_time(&tm, "%Y%m%d_%H%M%S")
        << "_" << std::fixed << std::setprecision(0) << timestamp
        << ".mp4";
    
    return oss.str();
}

bool VideoRecorder::SaveFrameAsImage(const cv::Mat& frame, const std::string& filename) {
    std::string fullPath = outputPath + filename;
    return cv::imwrite(fullPath, frame);
}

void VideoRecorder::SetCodec(int newCodec) {
    codec = newCodec;
    std::cout << "[VideoRecorder] Codec set to " << codec << std::endl;
}

void VideoRecorder::PrintRecordingInfo() const {
    std::cout << "\n=== VIDEO RECORDER INFO ===" << std::endl;
    std::cout << "Initialized: " << (isInitialized ? "YES" : "NO") << std::endl;
    std::cout << "Recording: " << (isRecording ? "YES" : "NO") << std::endl;
    std::cout << "Resolution: " << frameWidth << "x" << frameHeight << std::endl;
    std::cout << "FPS: " << fps << std::endl;
    std::cout << "Output Path: " << outputPath << std::endl;
    std::cout << "Current File: " << currentFilename << std::endl;
    std::cout << "Buffer Size: " << frameBuffer.size() << "/" << bufferSize << std::endl;
    std::cout << std::endl;
}

void VideoRecorder::PrintBufferInfo() const {
    std::cout << "\n=== FRAME BUFFER INFO ===" << std::endl;
    std::cout << "Buffer Size: " << frameBuffer.size() << " frames" << std::endl;
    std::cout << "Max Buffer Size: " << bufferSize << " frames" << std::endl;
    
    if (!frameBuffer.empty()) {
        std::cout << "Oldest Frame: " << frameBuffer.front().timestamp << "s" << std::endl;
        std::cout << "Newest Frame: " << frameBuffer.back().timestamp << "s" << std::endl;
    }
    std::cout << std::endl;
}
