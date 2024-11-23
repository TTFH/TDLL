#include <synchapi.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"

#include "recorder.h"

void FlipImageVertically(int width, int height, uint8_t* data) {
	uint8_t rgb[3];
	for (int y = 0; y < height / 2; y++) {
		for (int x = 0; x < width; x++) {
			int top = 3 * (x + y * width);
			int bottom = 3 * (x + (height - y - 1) * width);
			memcpy(rgb, data + top, sizeof(rgb));
			memcpy(data + top, data + bottom, sizeof(rgb));
			memcpy(data + bottom, rgb, sizeof(rgb));
		}
	}
}

void SaveImageJPG(const char* path, int width, int height, uint8_t* pixels) {
	FlipImageVertically(width, height, pixels);
	stbi_write_jpg(path, width, height, 3, pixels, 80);
}

void SaveImagePNG(const char* path, int width, int height, uint8_t* pixels) {
	stbi_write_png(path, width, height, 3, pixels, 3 * width);
}

void FastRecorder::SaveThread(int index) {
	unsigned int start = starting_frame[index];
	unsigned int count = video_frames[index].size();
	//printf("Thread %d (%2d frames) [%4d -> %4d]\n", index, count, start, start + count - 1);
	for (unsigned int i = 0; i < count; i++) {
		char image_path[256];
		snprintf(image_path, sizeof(image_path), "%s/%04d.jpg", capture_dir.c_str(), start + i);
		if (video_frames[index][i] != NULL) {
			SaveImageJPG(image_path, width, height, video_frames[index][i]);
			delete[] video_frames[index][i];
			video_frames[index][i] = NULL;
		}
		saved_frames++;
	}
	video_frames[index].clear(); // Clear vector for next batch
	//printf("Thread %d finished!\n", index);
	Sleep(1);
	running[index] = false; // Signal that the thread has finished
}

void FastRecorder::DumpRawData(int index) {
	char file_path[256];
	snprintf(file_path, sizeof(file_path), "%s/%04d.tdc", capture_dir.c_str(), sequence_number++);
	FILE* file = fopen(file_path, "wb");
	if (file != NULL) {
		int frame_count = video_frames[index].size();
		fwrite(&starting_frame[index], sizeof(int), 1, file);
		fwrite(&frame_count, sizeof(int), 1, file);
		fwrite(&width, sizeof(int), 1, file);
		fwrite(&height, sizeof(int), 1, file);
		for (unsigned int i = 0; i < video_frames[index].size(); i++) {
			if (video_frames[index][i] != NULL) {
				fwrite(video_frames[index][i], 3 * width * height, 1, file);
				delete[] video_frames[index][i];
				video_frames[index][i] = NULL;
			}
			saved_frames++;
		}
		video_frames[index].clear();
		fclose(file);
	}
	running[index] = false;
}

FastRecorder::FastRecorder() {
	width = 0;
	height = 0;
	saved_frames = 0;
	current_buffer = 0;
	sequence_number = 0;
	capture_dir = "mods/screenrecorder";
	for (int i = 0; i < THREAD_COUNT; i++) {
		starting_frame[i] = 0;
		video_frames[i].reserve(SUGGESTED_BUFFER_SIZE);
		running[i] = false;
	}
}

void FastRecorder::SetResolution(int width, int height) {
	this->width = width;
	this->height = height;
}

void FastRecorder::SetFolder(const char* dir) {
	capture_dir = dir;
}

void FastRecorder::AddFrame(uint8_t* pixels) {
	int size = 3 * width * height;
	uint8_t* frame = new uint8_t[size];
	memcpy(frame, pixels, size);
	video_frames[current_buffer].push_back(frame);
	total_frames++;
	if (total_frames % SUGGESTED_BUFFER_SIZE == 0)
		SaveFrames();
}

int FastRecorder::GetSavedFrames() {
	return saved_frames.load();
}

int FastRecorder::GetTotalFrames() {
	return total_frames;
}

void FastRecorder::SaveFrames() {
	bool found = false;
	int next_buffer = current_buffer;
	do {
		next_buffer = (next_buffer + 1) % THREAD_COUNT;
		if (next_buffer != current_buffer && !running[next_buffer]) {
			if (save_thread[next_buffer].joinable())
				save_thread[next_buffer].join();

			// Start current thread if the next buffer is available for use
			running[current_buffer] = true;
			save_thread[current_buffer] = std::thread(&FastRecorder::SaveThread, this, current_buffer);

			found = true;
			current_buffer = next_buffer;
			starting_frame[current_buffer] = total_frames - 1; // Set starting frame for next buffer
		}
	} while (!found && next_buffer != current_buffer);
}

void FastRecorder::ClearFrames() {
	total_frames = 0;
	saved_frames = 0;
	current_buffer = 0;
	sequence_number = 0;
	for (int i = 0; i < THREAD_COUNT; i++) {
		for (unsigned int j = 0; j < video_frames[i].size(); j++) {
			if (video_frames[i][j] != NULL) {
				delete[] video_frames[i][j];
				video_frames[i][j] = NULL;
			}
		}
		video_frames[i].clear();
		starting_frame[i] = 0;
		running[i] = false;
		if (save_thread[i].joinable())
			save_thread[i].join();
	}
}

FastRecorder::~FastRecorder() {
	ClearFrames();
}
