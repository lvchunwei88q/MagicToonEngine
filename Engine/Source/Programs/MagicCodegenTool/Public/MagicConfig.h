#pragma once

/**
 * @brief Build tool development mode switch (0: disabled, 1: enabled).
 * @details When enabled, the build tool outputs additional runtime debug information to help troubleshoot issues.
 */
#define MAGIC_DEV 0

/**
 * @brief Maximum number of input files processed in one code generation batch.
 */
#define MAXIMUM_CAPACITY_PER_JOB 256

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
 * @brief UTF-8 path to the engine header content hash check file.
 * @details Records the content hash of every engine source/header file at the last
 * successful code generation. Regeneration is decided by content, not by timestamp,
 * so touching / checking out files no longer triggers a full regeneration.
 */
#define MAGICENGINE_HEADER_HASH_CHECK_FILE u8"engine_headers_hash_check_file.buildmeta"

/**
 * @brief UTF-8 path to the engine class info file.
 */
#define MAGICENGINE_CLASS_INFO_FILE u8"logs/engine_class_info.log"

/**
 * @brief UTF-8 path to the engine class metadata debug file.
 */
#define MAGICENGINE_CLASS_METADATA_INFO_FILE u8"logs/engine_class_metadata_info.log"
