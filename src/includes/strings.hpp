#pragma once
#include <string.h>
#include <iostream>
#include <tuple>
#include <cctype> // Contains the std::isspace method;
#include <algorithm>

// Some extra strings methods that are very useful.
class Strings {
public:
	static std::tuple<int, std::string*> split(std::string str, char delimter = ' ') {
		// Checking for existence of delimiter in the string
		auto pos = str.find(delimter);
		// If the delimiter doesn't exist, return NULL
		if (pos == std::string::npos)
			return { -1, NULL };

		int size = 1;
		int begin = 0;
		int iter = 0;
		int lastAt = 0;
		size_t len_str = str.length();
		for (size_t i = 0; i < len_str; i++) {
			if (str[i] == delimter)
				size++;
		}
		std::string* ret = new std::string[size];

		for (int i = 0; i < size; i++)
			ret[i] = "";

		for (size_t i = 0; i < len_str; i++) {
			if (str[i] == delimter) {
				for (size_t j = begin; j < i; j++)
					ret[iter] += str[j];
				iter++;
				begin = i + 1;
				lastAt = i;
			}
			if (i == len_str - 1 && lastAt != len_str) {
				for (size_t j = lastAt + 1; j < len_str; j++) {
					if (iter < size)
						ret[iter] += str[j];
				}
			}
		}
		return { size, ret };
	}
	static std::string lrstrip(std::string str) {
		auto begin = str.begin();
		auto end = str.rbegin();
		while (std::isspace(*begin))
			begin++;
		while (std::isspace((*end)))
			end++;
		return std::string(begin, end.base());
	}
	static std::string strip(std::string str) {
		std::string output;
		std::unique_copy(str.begin(), str.end(), std::back_insert_iterator<std::string>(output),
			[](char a, char b) {
				return isspace(a) && isspace(b);
			}
		);
		return lrstrip(output);
	}

	static void printStringArray(std::string* arr, int size) {
		std::cout << "====================================\n";
		std::cout << "Index\t:\tValue\n\n";
		for (int i = 0; i < size; i++)
			std::cout << i << "\t:\t" << arr[i] << std::endl;
		std::cout << "====================================\n";
	}
	static void removeNewLine(char* arr) {
		int size = std::remove(arr, arr + strlen(arr), '\n') - arr;
		arr[size] = 0;
	}
};

template <class _T>
void printArray(_T* arr, int size) {
	std::cout << "====================================\n";
	std::cout << "Index\t:\tValue\n\n";
	for (int i = 0; i < size; i++)
		std::cout << i << "\t:\t" << arr[i] << std::endl;
	std::cout << "====================================\n";
}