#ifndef CLOSE_WINDOW_H_
#define CLOSE_WINDOW_H_

#include <string>

#include <Windows.h>

bool closeWindowByClassName(const std::wstring& wsTargetWindowClassName);

#endif // CLOSE_WINDOW_H_
