#pragma once

/**
 * @brief Build tool development mode switch (0: disabled, 1: enabled).
 * @details When enabled, the build tool outputs additional runtime debug information to help troubleshoot issues.
 */
#define MAGIC_DEV 0

/**
 * @brief Maximum number of files that a single job (batch) can process.
 * @details Since the number of files to process may be very large, the build tool processes them in batches
 * (100 files per job). This macro defines the batch size limit.
 */
#define MAXIMUM_CAPACITY_PER_JOB 100

/**
 * @brief UTF-8 path to the engine generation information directory.
 */
#define MAGICENGINE_FILE_INFO_PATH u8"engine_generate_info"

/**
 * @brief UTF-8 path to the engine header build metadata file.
 */
#define MAGICENGINE_HEADER_INFO_FILE u8"engine_headers.buildmeta"

/**
 * @brief UTF-8 path to the engine compilation database file.
 */
#define MAGICENGINE_COMPILE_COMMANDS_FILE u8"engine_compile_commands.buildmeta"

/**
 * @brief UTF-8 path to the engine header timestamp check file.
 */
#define MAGICENGINE_HEADER_TIME_CHECK_FILE u8"engine_headers_time_check_file.buildmeta"

/**
 * @brief UTF-8 path to the engine class info file.
 */
#define MAGICENGINE_CLASS_INFO_FILE u8"logs/engine_class_info.log"

/**
 * @brief UTF-8 path to the engine class metadata debug file.
 */
#define MAGICENGINE_CLASS_METADATA_INFO_FILE u8"logs/engine_class_metadata_info.log"
