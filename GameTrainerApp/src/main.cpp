#include <iostream>
#include "ScreenCapture.h"
#include "InputTracker.h"
#include "ReviewInterface.h"

int main() {
    std::cout << "GameTrainerApp initialized successfully." << std::endl;
    std::cout << "=== GAME TRAINER APP ===" << std::endl;
    std::cout << "1. Background Recording Mode" << std::endl;
    std::cout << "2. Review Mode (Post-Match Analysis)" << std::endl;
    std::cout << "Choose mode (1 or 2): ";
    
    int mode;
    std::cin >> mode;
    
    if (mode == 1) {
        std::cout << "\n=== BACKGROUND RECORDING MODE ===" << std::endl;
        std::cout << "Starting input tracking and screen capture..." << std::endl;
        
        if (CaptureScreenToBMP("screenshot.bmp")) {
            std::cout << "Screenshot captured successfully." << std::endl;
        } else {
            std::cout << "Screenshot Error." << std::endl;
        }
        
        std::cout << "Starting input tracking..." << std::endl;
        TrackInput();
        
    } else if (mode == 2) {
        std::cout << "\n=== REVIEW MODE ===" << std::endl;
        std::cout << "Loading gameplay clips for analysis..." << std::endl;
        
        ReviewInterface reviewInterface;
        reviewInterface.LoadClips("session_2024-01-15_14-30-25");
        
        std::cout << "\nStarting clip review..." << std::endl;
        reviewInterface.PlayClip(0);
        
        char input;
        while (std::cin >> input && input != 'q') {
            reviewInterface.ProcessUserInput(input);
            reviewInterface.ShowCurrentClipInfo();
            reviewInterface.ShowPlaybackControls();
        }
        
        reviewInterface.GenerateConcentrationReport();
        reviewInterface.GenerateTechnicalReport();
        reviewInterface.ShowImprovementPlan();
        
    } else {
        std::cout << "Invalid mode selected." << std::endl;
    }
    
    return 0;
}