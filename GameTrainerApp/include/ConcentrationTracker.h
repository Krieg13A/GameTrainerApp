#pragma once
#include <string>
#include <vector>
#include <map>

enum class FocusLevel {
    LOW_FOCUS = 0,
    MEDIUM_FOCUS = 1,
    HIGH_FOCUS = 2
};

struct ConcentrationMark {
    double timestamp;
    FocusLevel level;
    std::string reason;
};

class ConcentrationTracker {
private:
    std::string currentClipId;
    std::vector<ConcentrationMark> concentrationMarks;
    bool isReviewMode;

public:
    ConcentrationTracker();
    ~ConcentrationTracker();
    
    void StartReview(const std::string& clipId);
    void EndReview();
    bool IsInReviewMode() const;
    
    void MarkConcentrationAtTime(double timestamp, FocusLevel level, const std::string& reason = "");
    void RemoveMarkAtTime(double timestamp);
    ConcentrationMark* GetMarkAtTime(double timestamp);
    
    std::vector<ConcentrationMark> GetConcentrationMarks() const;
    FocusLevel GetFocusLevelAtTime(double timestamp) const;
    std::map<FocusLevel, int> GetConcentrationStats() const;
    
    void SaveConcentrationData(const std::string& filename);
    void LoadConcentrationData(const std::string& filename);
    
    std::string GetFocusLevelString(FocusLevel level) const;
    std::string GetClipId() const;
};
