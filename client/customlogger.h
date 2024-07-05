
#include <cstdio>
#include <mutex>
#include <ctime>


class LOGGER
{
	public:
	enum LogPriority
	{
		TRACE_PRIORITY, DEBUG_PRIORITY, INFO_PRIORITY, 
        WARN_PRIORITY, ERROR_PRIORITY, CRITICAL_PRIORITY,
		SUCCESS_PRIORITY
	};

	private:
	LogPriority priority = TRACE_PRIORITY;
	std::mutex log_mutex;
	
	const char* filepath = 0;
	std::FILE* file = 0;
	
	// for timestamp formatting
	char buffer[80];
	const char* timestamp_format = "%T  %d-%m-%Y";

	public:
	// Set desired priority for the logger (messages with lower priority will not be recorded)
	// The default priority is LOGGER::INFO_PRIORITY
	static void SetPriority(LogPriority new_priority)
	{
		get_instance().priority = new_priority;
	}

	// Get the current logger priority (messages with lower priority will not be recorded)
	// The default priority is LOGGER::INFO_PRIORITY
	static LogPriority GetPriority()
	{
		return get_instance().priority;
	}

	// Enable file output
	// Logs will be written to /log.txt
	// If the file doesn't exist, it will create it automatically
	// File will be closed when program stops
	// Returns true if a file was successfully opened, false otherwise
	static bool EnableFileOutput()
	{
		LOGGER& logger_instance = get_instance();
		logger_instance.filepath = "log.txt";
		return logger_instance.enable_file_output();
	}

	// Enable file output
	// Logs will be written to /filepath, provided the filepath is valid
	// If the file doesn't exist, it will create it automatically
	// File will be closed when program stops
	// Returns true if a file was successfully opened, false otherwise
	static bool EnableFileOutput(const char* new_filepath)
	{
		LOGGER& logger_instance = get_instance();
		logger_instance.filepath = new_filepath;
		return logger_instance.enable_file_output();
	}

	// Returns the current filepath for file logging
	// if LOGGER::EnableFileOutput was called without specifying a filepath, the filepath will be "log.txt"
	// if file output was not enabled, the filepath will contain NULL
	static const char* GetFilepath()
	{
		return get_instance().filepath;
	}

	// Returns true is file output was enabled and file was successfully opened, false if it wasn't
	static bool IsFileOutputEnabled()
	{
		return get_instance().file != 0;
	}

	// Set a log timestamp format
	// Format follows <ctime> strftime format specification
	// Default format is "%T  %d-%m-%Y" (e.g. 13:20:25  14-02-2021)
	// 4 spaces are added automatically to the end of timestamp each time the message is logged
	static void SetTimestampFormat(const char* new_timestamp_format)
	{
		get_instance().timestamp_format = new_timestamp_format;
	}

	// Get the current log timestamp format
	// Format follows <ctime> strftime format specification
	// Default format is "%T  %d-%m-%Y" (e.g. 13:20:25  14-02-2021)
	static const char* GetTimestampFormat()
	{
		return get_instance().timestamp_format;
	}

	// Log a message (format + optional args, follow printf specification)
	// with log priority level LOGGER::TRACE_PRIORITY
	template<typename... Args>
	static void TRACE(const char* message, Args... args)
	{
		get_instance().log("[TRACE]    ", TRACE_PRIORITY, message, args...);
	}

	// Log a message (format + optional args, follow printf specification)
	// with log priority level LOGGER::DEBUG_PRIORITY
	template<typename... Args>
	static void DEBUG(const char* message, Args... args)
	{
		get_instance().log("[DEBUG]    ", DEBUG_PRIORITY, message, args...);
	}

	// Log a message (format + optional args, follow printf specification)
	// with log priority level LOGGER::INFO_PRIORITY
	template<typename... Args>
	static void INFO(const char* message, Args... args)
	{
		get_instance().log("[INFO]     ", INFO_PRIORITY, message, args...);
	}

	// Log a message (format + optional args, follow printf specification)
	// with log priority level LOGGER::INFO_PRIORITY
	template<typename... Args>
	static void SUCCESS(const char* message, Args... args)
	{
		get_instance().log("[SUCCESS]     ", SUCCESS_PRIORITY, message, args...);
	}

	// Log a message (format + optional args, follow printf specification)
	// with log priority level LOGGER::WARN_PRIORITY
	template<typename... Args>
	static void WARN(const char* message, Args... args)
	{
		get_instance().log("[WARN]     ", WARN_PRIORITY, message, args...);
	}

	// Log a message (format + optional args, follow printf specification)
	// with log priority level LOGGER::ERROR_PRIORITY
	template<typename... Args>
	static void ERROR(const char* message, Args... args)
	{
		get_instance().log("[ERROR]    ", ERROR_PRIORITY, message, args...);
	}

	// Log a message (format + optional args, follow printf specification)
	// with log priority level LOGGER::CRITICAL_PRIORITY
	template<typename... Args>
	static void CRITICAL(const char* message, Args... args)
	{
		get_instance().log("[CRITICAL]     ", CRITICAL_PRIORITY, message, args...);
	}

private:
	LOGGER() {}

	LOGGER(const LOGGER&) = delete;
	LOGGER& operator= (const LOGGER&) = delete;

	~LOGGER()
	{
		free_file();
	}

	static LOGGER& get_instance()
	{
		static LOGGER instance;
		
		return instance;
	}

	template<typename... Args>
	void log(const char* message_priority_str, LogPriority message_priority, const char* message, Args... args)
	{
		// LOGGER::EnableFileOutput();
		if (priority <= message_priority)
		{
			std::time_t current_time = std::time(0);
			std::tm* timestamp = std::localtime(&current_time);

			std::scoped_lock lock(log_mutex);
			std::strftime(buffer, 80, timestamp_format, timestamp);
			std::printf("%s    ", buffer);
			std::printf("%s",message_priority_str);
			std::printf(message, args...);
			std::printf("\n");

			if (file)	
			{
				std::fprintf(file, "%s    ", buffer);
				std::fprintf(file, message_priority_str);
				std::fprintf(file, message, args...);
				std::fprintf(file, "\n");
			}
			// LOGGER::free_file();
		}
	}

	bool enable_file_output()
	{
		free_file();

		file = std::fopen(filepath, "a");

		if (file == 0)
		{
			return false;
		}

		return true;
	}

	void free_file()
	{
		if (file)
		{
			std::fclose(file);
			file = 0;
		}
	}
};