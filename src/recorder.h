#ifndef _RECORDER_H
#define _RECORDER_H

#include <stdint.h>
#include <atomic>
#include <string>
#include <thread>
#include <vector>

class FastRecorder {
private:
	#define THREAD_COUNT 4					// One thread is always idle
	#define SUGGESTED_BUFFER_SIZE 60		// Number of frames to save per thread, can be bigger
	int width;
	int height;
	int total_frames;						// The total number of frames
	int current_buffer;						// The current buffer to write frames to
	std::string capture_dir;
	std::atomic<int> saved_frames;			// The number of frames saved to disk
	std::atomic<int> sequence_number;		// The current file for raw data
	int starting_frame[THREAD_COUNT];		// The starting frame index for each buffer
	std::thread save_thread[THREAD_COUNT];
	std::atomic<bool> running[THREAD_COUNT];// The state of the save thread
	std::vector<uint8_t*> video_frames[THREAD_COUNT];

	void SaveThread(int index);
	void DumpRawData(int index);
public:
	FastRecorder();
	void SetResolution(int width, int height);
	void SetFolder(const char* dir);
	void AddFrame(uint8_t* pixels);
	int GetSavedFrames();
	int GetTotalFrames();
	void SaveFrames();
	void ClearFrames();
	~FastRecorder();
};

#endif
