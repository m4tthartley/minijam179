//
//  Created by Matt Hartley on 03/03/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <Carbon/Carbon.h>
#include <core/core.h>

char* Sys_GetResourcePath(allocator_t* allocator, char* filename) {
	CFBundleRef bundle = CFBundleGetMainBundle();
	CFURLRef bundleUrl = CFBundleCopyBundleURL(bundle);
	char bundlePath[MAX_PATH_LENGTH];
	CFURLGetFileSystemRepresentation(bundleUrl, true, (UInt8*)bundlePath, MAX_PATH_LENGTH);

	file_t file;
	char testPath[MAX_PATH_LENGTH];

	// Check working directory
	if ((file = file_open(filename)) != -1) {
		file_close(file);
		return filename;
	}

	// Check bundle
	copy_memory(testPath, bundlePath, MAX_PATH_LENGTH);
	char_append(testPath, "/", MAX_PATH_LENGTH);
	char_append(testPath, filename, MAX_PATH_LENGTH);
	if ((file = file_open(testPath)) != -1) {
		file_close(file);
		char* result = malloc(str_len(testPath)+1);
		copy_memory(result, testPath, str_len(testPath)+1);
		return result;
	}

	// Check parent directory
	copy_memory(testPath, "../", 4);
	char_append(testPath, filename, MAX_PATH_LENGTH);
	if ((file = file_open(testPath)) != -1) {
		file_close(file);
		char* result = malloc(str_len(testPath)+1);
		copy_memory(result, testPath, str_len(testPath)+1);
		return result;
	}

	return filename;
}