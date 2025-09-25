#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>
#include <queue>

struct FrameBuffer {
    cv::Mat frame;
    double timestamp;
    std::vector<cv::Point2f> enemyPositions;
};

class VideoRecorder {
private:
    cv::VideoWriter videoWriter;
    std::queue<FrameBuffer> frameBuffer;
    std::string outputPath;
    std::string currentFilename;
    bool isRecording;
    bool isInitialized;
    
    int frameWidth;
    int frameHeight;
    double fps;
    int bufferSize;
    int codec;
    
    void AddFrameToBuffer(const cv::Mat& frame, double timestamp);
    void WriteBufferedFrames();
    void ClearBuffer();
    
public:
    VideoRecorder();
    ~VideoRecorder();
    
    bool Initialize(int width = 1280, int height = 720, double fps = 30.0);
    void SetOutputPath(const std::string& path);
    void SetBufferSize(int size);
    
    bool StartRecording(const std::string& filename);
    void StopRecording();
    bool IsRecording() const;
    
    void AddFrame(const cv::Mat& frame, double timestamp);
    void AddFrameWithEnemies(const cv::Mat& frame, double timestamp, const std::vector<cv::Point2f>& enemyPositions);
    
    void StartBuffering();
    void StopBuffering();
    size_t GetBufferSize() const;
    
    std::string GenerateFilename(const std::string& prefix, double timestamp);
    bool SaveFrameAsImage(const cv::Mat& frame, const std::string& filename);
    void SetCodec(int codec);
    
    void PrintRecordingInfo() const;
    void PrintBufferInfo() const;
};
