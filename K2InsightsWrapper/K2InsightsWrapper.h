#pragma once

#include <Windows.h>

using namespace System;

namespace K2InsightsWrapper
{
	public ref class InsightsWrapper
	{
	public:
		void Initialize()
		{
			handler.Initialize();
		}

		void LogEvent(const char* name)
		{
			handler.LogEvent(gcnew String(name));
		}

		void LogTrace(const char* message)
		{
			handler.LogTrace(gcnew String(message));
		}

		void LogMetric(const char* name, DOUBLE value)
		{
			handler.LogMetric(gcnew String(name), value);
		}

		void LogPageView(const char* name)
		{
			handler.LogPageView(gcnew String(name));
		}

	private:
		K2InsightsHandler::InsightsHandler handler;
	};
}

ref struct IK2Insights
{
	static K2InsightsWrapper::InsightsWrapper^ wrapper = gcnew K2InsightsWrapper::InsightsWrapper;
};

namespace K2InsightsCLR
{
	__declspec(dllexport) void Initialize()
	{
		IK2Insights::wrapper->Initialize();
	}

	__declspec(dllexport) void LogEvent(const char* name)
	{
		IK2Insights::wrapper->LogEvent(name);
	}

	__declspec(dllexport) void LogTrace(const char* message)
	{
		IK2Insights::wrapper->LogTrace(message);
	}

	__declspec(dllexport) void LogMetric(const char* name, DOUBLE value)
	{
		IK2Insights::wrapper->LogMetric(name, value);
	}

	__declspec(dllexport) void LogPageView(const char* name)
	{
		IK2Insights::wrapper->LogPageView(name);
	}
}
