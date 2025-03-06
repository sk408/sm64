/**
 * @file btstack_config.h
 * @brief Configuration for BTstack used in Pico-ASHA
 */

#ifndef BTSTACK_CONFIG_H
#define BTSTACK_CONFIG_H

// BTstack features that can be enabled
#define ENABLE_LOG_INFO
#define ENABLE_LOG_ERROR
#define ENABLE_LE_PERIPHERAL
#define ENABLE_LE_CENTRAL
#define ENABLE_L2CAP_LE_CREDIT_BASED_FLOW_CONTROL_MODE

// BLE GATT Server callbacks
#define ENABLE_GATT_CLIENT_PAIRING
#define ENABLE_LE_SECURE_CONNECTIONS
#define ENABLE_LE_PRIVACY_ADDRESS_RESOLUTION

// ASHA specific settings
#define MAX_NR_GATT_CLIENTS 4
#define MAX_NR_HCI_CONNECTIONS 2
#define MAX_NR_SM_LOOKUP_ENTRIES 4
#define MAX_NR_WHITELIST_ENTRIES 8
#define MAX_NR_LE_DEVICE_DB_ENTRIES 16
#define NVM_NUM_DEVICE_DB_ENTRIES 16  // Required by le_device_db_tlv.c

// HCI and buffer size configurations
#define HCI_ACL_PAYLOAD_SIZE 1024
#define HCI_ACL_BUFFER_SIZE 1024

// Memory configuration
#define MAX_ATT_DB_SIZE 512
#define MAX_NR_GATT_ATTRIBUTES 50
#define MAX_NR_GATT_SERVICES 10
#define MAX_NR_GATT_CHARACTERISTICS 20

// BTstack configuration
#define HAVE_MALLOC
#define HAVE_ASSERT
#define HAVE_BTSTACK_STDIN
#define HAVE_BTSTACK_AUDIO

// Pico SDK integration
#define HAVE_EMBEDDED_TIME_MS

// CPU dependent configurations
#define ENABLE_SOFTWARE_AES128
#define HAVE_POSIX_FILE_IO
#define HAVE_POSIX_TIME

#endif // BTSTACK_CONFIG_H 