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
 * @brief Path to the engine generation information directory (wide-character version).
 * @details A fixed path used to store engine generation information.
 */
#define MAGICENGINE_FILE_INFO_PATH_W L"engine_generate_info"

/**
 * @brief Path to the engine generation information directory (narrow-character version).
 * @details A fixed path corresponding to MAGICENGINE_FILE_INFO_PATH_W.
 */
#define MAGICENGINE_FILE_INFO_PATH "engine_generate_info"

/**
 * @brief Path to the engine header build metadata file (wide-character version).
 * @details A fixed file path.
 */
#define MAGICENGINE_HEADER_INFO_FILE_W L"engine_headers.buildmeta"

/**
 * @brief Path to the engine header build metadata file (narrow-character version).
 * @details A fixed path corresponding to MAGICENGINE_HEADER_INFO_FILE_W.
 */
#define MAGICENGINE_HEADER_INFO_FILE "engine_headers.buildmeta"

/**
 * @brief Path to the engine header timestamp check file (wide-character version).
 * @details A fixed file path used to record timestamps for incremental build checks.
 */
#define MAGICENGINE_HEADER_TIME_CHECK_FILE_W L"engine_headers_time_check_file.buildmeta"

/**
 * @brief Path to the engine header timestamp check file (narrow-character version).
 * @details A fixed path corresponding to MAGICENGINE_HEADER_TIME_CHECK_FILE_W.
 */
#define MAGICENGINE_HEADER_TIME_CHECK_FILE "engine_headers_time_check_file.buildmeta"

/**
 * @brief Path to the engine class info file (wide-character version).
 * @details A fixed file path used to store class information.
 */
#define MAGICENGINE_CLASS_INFO_FILE_W L"engine_class_info.log"

/**
 * @brief Path to the engine class info file (narrow-character version).
 * @details A fixed path corresponding to MAGICENGINE_CLASS_INFO_FILE_W.
 */
#define MAGICENGINE_CLASS_INFO_FILE "engine_class_info.log"


/**
 * @brief Path to the engine class metadata debug file (wide-character version).
 * @details A fixed file path used for debugging the generation of Engine class metadata.
 */
#define MAGICENGINE_CLASS_METADATA_DEBUG_FILE_W L"engine_class_metadata_debug_file.log"

/**
 * @brief Path to the engine class metadata debug file (narrow-character version).
 * @details A fixed path corresponding to MAGICENGINE_CLASS_METADATA_DEBUG_FILE_W.
 */
#define MAGICENGINE_CLASS_METADATA_DEBUG_FILE "engine_class_metadata_debug_file.log"
