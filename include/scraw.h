#pragma once

#include <stdbool.h>
#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#define PAL_WIN
#elif defined(unix) || defined(__unix) || defined(__unix__) ||                 \
    defined(__linux__)
#define PAL_LNX
#elif defined(__APPLE__) || defined(__MACH__)
#define PAL_MAC
#endif

#ifdef PAL_WIN
#include <winscard.h>
typedef SCARDCONTEXT scraw_ctx_kt;
typedef SCARDHANDLE scraw_card_kt;
#else
#ifdef PAL_MAC
#warn macOS support was not tested but should work
#endif

/* Linux and macOS */
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
typedef SCARDCONTEXT scraw_ctx_kt;
typedef SCARDHANDLE scraw_card_kt;
#endif

typedef struct scraw_raw_s
{
    uint8_t const *const buf;
    uint32_t const len;
} scraw_raw_st;

typedef struct scraw_res_s
{
    uint8_t *const buf;
    uint32_t const buf_len;
    uint32_t res_len;
} scraw_res_st;

typedef struct scraw_reader_list_s
{
    bool list_valid; /* True if the list fields are valid e.g. after completing
                      a succesful search, false otherwise. */
    uint32_t next_offset; /* Keep track of which reader is should be returned
                             call to 'next'. */
    char const *names;
    uint32_t names_len;
} scraw_reader_list_st;

typedef struct scraw_reader_s
{
    char const *name;
} scraw_reader_st;

typedef enum scraw_proto_e
{
    SCRAW_PROTO_T0,
    SCRAW_PROTO_T1,
} scraw_proto_et;

typedef struct scraw_s
{
    scraw_ctx_kt ctx;
    scraw_card_kt card; /* Selected card. */
    scraw_proto_et card_proto;
    scraw_reader_list_st reader_list;
    scraw_reader_st reader_selected;
} scraw_st;

/**
 * @brief Initialize the library context. This allows use of the library
 * functions.
 * @param ctx Pointer to a struct that will hold the context.
 * @return 0 on success, -1 on failure.
 */
int32_t scraw_init(scraw_st *const ctx);

/**
 * @brief Deinitialize the library context. After it's complete, passing the
 * context to library functions may lead to undefined behavior.
 * @param ctx
 * @return 0 on success, -1 on failure.
 */
int32_t scraw_fini(scraw_st *const ctx);

/**
 * @brief Retrieve an updated list of readers from the OS. This list is then
 * used when selecting a reader.
 * @param ctx
 * @return 0 on success, -1 on failure.
 */
int32_t scraw_reader_search(scraw_st *const ctx);

/**
 * @brief Get the next reader in the list of readers. First call after a call to
 * 'search', this shall return the first reader (at index 0).
 * @param ctx
 * @param reader_name Where the pointer to the reader name will be written. It
 * will be null-terminated.
 * @return 0 on success, 1 if there are no more cards in the list, -1 on
 * failure.
 * @note A call to 'search' is needed to be able to traverse the list again.
 */
int32_t scraw_reader_next(scraw_st *const ctx, char const **const reader_name);

/**
 * @brief Select a reader by name. If completes successfully, data can be sent
 * to the card.
 * @param ctx
 * @param reader_name Reader name to select. Must be null-terminated and the
 * buffer must be allocated by the user (i.e. not pointing to a string returned
 * by 'next').
 * @return 0 on success, -1 on failure.
 * @note A 'search' is not necessary before selecting if the name of the reader
 * is known.
 */
int32_t scraw_reader_select(scraw_st *const ctx, char const *const reader_name);

/**
 * @brief Establish a connection with the card in the selected reader. If a
 * connection has already been established, a reconnect is performed.
 * @param ctx
 * @param proto What protocol to use to communicate with the card.
 * @return 0 on success, -1 on failure.
 */
int32_t scraw_card_connect(scraw_st *const ctx, scraw_proto_et const proto);

/**
 * @brief Close connection with the card. If completes successfully, any
 * operation involving the card will be undefined.
 * @param ctx
 * @return 0 on success, -1 on failure.
 */
int32_t scraw_card_disconnect(scraw_st *const ctx);

/**
 * @brief Send raw bytes to a smart card and get the response data and status
 * word back.
 * @param ctx
 * @param data What to send to the card.
 * @param card_res Where the card reponse will be written.
 * @return 0 on success, 1 if card is not present, -1 on failure.
 */
int32_t scraw_send(scraw_st *const ctx, scraw_raw_st *const data,
                   scraw_res_st *const card_res);
