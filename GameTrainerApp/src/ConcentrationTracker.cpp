#include "ConcentrationTracker.h"
#include "EventLogger.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

ConcentrationTracker::ConcentrationTracker() 
    : isReviewMode(false) {
}

ConcentrationTracker::~ConcentrationTracker() {
    if (isReviewMode) {
        EndReview();
    }
}

void ConcentrationTracker::StartReview(const std::string& clipId) {
    currentClipId = clipId;
    concentrationMarks.clear();
    isReviewMode = true;
    
    std::cout << "[ConcentrationTracker] Started review for clip: " << clipId << std::endl;
    std::cout << "[ConcentrationTracker] Use keys 1=Low Focus, 2=Medium Focus, 3=High Focus during playback" << std::endl;
}

void ConcentrationTracker::EndReview() {
    if (isReviewMode) {
        std::cout << "[ConcentrationTracker] Ended review for clip: " << currentClipId << std::endl;
        std::cout << "[ConcentrationTracker] Total concentration marks: " << concentrationMarks.size() << std::endl;
        
        std::string filename = currentClipId + "_concentration.csv";
        SaveConcentrationData(filename);
        
        isReviewMode = false;
        currentClipId.clear();
    }
}

bool ConcentrationTracker::IsInReviewMode() const {
    return isReviewMode;
}

void ConcentrationTracker::MarkConcentrationAtTime(double timestamp, FocusLevel level, const std::string& reason) {
    if (!isReviewMode) return;
    
    RemoveMarkAtTime(timestamp);
    
    ConcentrationMark mark;
    mark.timestamp = timestamp;
    mark.level = level;
    mark.reason = reason;
    
    concentrationMarks.push_back(mark);
    
    std::sort(concentrationMarks.begin(), concentrationMarks.end(), 
              [](const ConcentrationMark& a, const ConcentrationMark& b) {
                  return a.timestamp < b.timestamp;
              });
    
    std::cout << "[ConcentrationTracker] Marked " << GetFocusLevelString(level) 
              << " at " << timestamp << "s" << std::endl;
}

void ConcentrationTracker::RemoveMarkAtTime(double timestamp) {
    concentrationMarks.erase(
        std::remove_if(concentrationMarks.begin(), concentrationMarks.end(),
                      [timestamp](const ConcentrationMark& mark) {
                          return std::abs(mark.timestamp - timestamp) < 0.1;
                      }),
        concentrationMarks.end()
    );
}

ConcentrationMark* ConcentrationTracker::GetMarkAtTime(double timestamp) {
    for (auto& mark : concentrationMarks) {
        if (std::abs(mark.timestamp - timestamp) < 0.1) {
            return &mark;
        }
    }
    return nullptr;
}

std::vector<ConcentrationMark> ConcentrationTracker::GetConcentrationMarks() const {
    return concentrationMarks;
}

FocusLevel ConcentrationTracker::GetFocusLevelAtTime(double timestamp) const {
    FocusLevel currentLevel = FocusLevel::MEDIUM_FOCUS;
    
    for (const auto& mark : concentrationMarks) {
        if (mark.timestamp <= timestamp) {
            currentLevel = mark.level;
        } else {
            break;
        }
    }
    
    return currentLevel;
}

std::map<FocusLevel, int> ConcentrationTracker::GetConcentrationStats() const {
    std::map<FocusLevel, int> stats;
    stats[FocusLevel::LOW_FOCUS] = 0;
    stats[FocusLevel::MEDIUM_FOCUS] = 0;
    stats[FocusLevel::HIGH_FOCUS] = 0;
    
    for (const auto& mark : concentrationMarks) {
        stats[mark.level]++;
    }
    
    return stats;
}

void ConcentrationTracker::SaveConcentrationData(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[ConcentrationTracker] Failed to save concentration data to " << filename << std::endl;
        return;
    }
    
    file << "timestamp,focus_level,reason\n";
    for (const auto& mark : concentrationMarks) {
        file << mark.timestamp << "," 
             << GetFocusLevelString(mark.level) << ","
             << mark.reason << "\n";
    }
    
    file.close();
    std::cout << "[ConcentrationTracker] Saved concentration data to " << filename << std::endl;
}

void ConcentrationTracker::LoadConcentrationData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "[ConcentrationTracker] No existing concentration data found" << std::endl;
        return;
    }
    
    concentrationMarks.clear();
    std::string line;
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string timestampStr, levelStr, reason;
        
        if (std::getline(iss, timestampStr, ',') &&
            std::getline(iss, levelStr, ',') &&
            std::getline(iss, reason)) {
            
            ConcentrationMark mark;
            mark.timestamp = std::stod(timestampStr);
            mark.reason = reason;
            
            if (levelStr == "LOW_FOCUS") mark.level = FocusLevel::LOW_FOCUS;
            else if (levelStr == "MEDIUM_FOCUS") mark.level = FocusLevel::MEDIUM_FOCUS;
            else if (levelStr == "HIGH_FOCUS") mark.level = FocusLevel::HIGH_FOCUS;
            
            concentrationMarks.push_back(mark);
        }
    }
    
    file.close();
    std::cout << "[ConcentrationTracker] Loaded " << concentrationMarks.size() 
              << " concentration marks from " << filename << std::endl;
}

std::string ConcentrationTracker::GetFocusLevelString(FocusLevel level) const {
    switch (level) {
        case FocusLevel::LOW_FOCUS: return "LOW_FOCUS";
        case FocusLevel::MEDIUM_FOCUS: return "MEDIUM_FOCUS";
        case FocusLevel::HIGH_FOCUS: return "HIGH_FOCUS";
        default: return "UNKNOWN";
    }
}

std::string ConcentrationTracker::GetClipId() const {
    return currentClipId;
}
