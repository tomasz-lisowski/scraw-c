#include "scraw.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int32_t main(void)
{
    int32_t main_ret = EXIT_FAILURE;
    int32_t ret;
    scraw_st ctx;
    char *reader_name_selected = NULL;
    int const err_init = scraw_init(&ctx);
    if (err_init == 0)
    {
        int const err_reader_search_begin = scraw_reader_search_begin(&ctx);
        if (err_reader_search_begin == 0)
        {
            bool reader_found = false;
            char const *reader_name_tmp = NULL;
            for (uint32_t reader_idx = 0U;; ++reader_idx)
            {
                ret = scraw_reader_search_next(&ctx, &reader_name_tmp);
                if (ret == 0)
                {
                    printf("Reader[%u]=\"%s\".\n", reader_idx, reader_name_tmp);
                    if (reader_found == false)
                    {
                        printf("Selecting reader %u.\n", reader_idx);
                        uint64_t const reader_name_len =
                            strlen(reader_name_tmp) + 1 /* null-terminator */;
                        reader_name_selected = malloc(reader_name_len);
                        if (reader_name_selected == NULL)
                        {
                            printf(
                                "Failed to allocate memory for reader name.\n");
                        }
                        else
                        {
                            memcpy(reader_name_selected, reader_name_tmp,
                                   reader_name_len);
                            if (scraw_reader_select(&ctx,
                                                    reader_name_selected) == 0)
                            {
                                printf("Selected reader %u \"%s\".\n",
                                       reader_idx, ctx.reader_selected.name);
                                reader_found = true;
                                break; /* Selected a reader so can carry on. */
                            }
                        }
                    }
                }
                else if (ret == 1)
                {
                    printf("Reached end of reader list "
                           "scraw_reader_search_next()=%i reason=0x%llx.\n",
                           ret, ctx.err_reason);
                    break;
                }
                else
                {
                    printf("Error while searching reader list "
                           "scraw_reader_search_next()=%i reason=0x%llx.\n",
                           ret, ctx.err_reason);
                    break;
                }
            }
            int const err_reader_search_end = scraw_reader_search_end(&ctx);
            if (err_reader_search_end != 0)
            {
                printf("Failed to end search scraw_reader_search_end()=%i "
                       "reason=0x%llx.\n",
                       err_reader_search_end, ctx.err_reason);
                /**
                 * This failure leads to a memory leak but is not critical so we
                 * carry on.
                 */
            }
            if (reader_found)
            {
                int const err_connect =
                    scraw_card_connect(&ctx, SCRAW_PROTO_T0);
                if (err_connect == 0)
                {
                    printf("Connected to card scraw_card_connect()=%i.\n",
                           err_connect);

                    /* This is a GET CHALLENGE APDU command. */
                    uint8_t raw_data[] = {0x00, 0x84, 0x00, 0x00, 0xFF};
                    uint8_t res_data[1024U] = {0};
                    scraw_raw_st raw = {
                        .buf = raw_data,
                        .len = sizeof(raw_data) / sizeof(raw_data[0]),
                    };
                    scraw_res_st res = {
                        .buf = res_data,
                        .buf_len = sizeof(res_data) / sizeof(res_data[0]),
                        .res_len = 0,
                    };

                    int const err_send = scraw_send(&ctx, &raw, &res);
                    if (err_send == 0)
                    {
                        printf("Sent data to card scraw_send()=%i "
                               "apdu=%02X%02X%02X%02X%02X.\n",
                               err_send, raw_data[0U], raw_data[1U],
                               raw_data[2U], raw_data[3U], raw_data[4U]);
                        printf("Response raw=");
                        for (uint32_t i = 0; i < res.res_len; ++i)
                        {
                            printf("%02X ", res.buf[i]);
                        }
                        printf("\n");
                        main_ret = EXIT_SUCCESS;
                    }
                    else
                    {
                        printf("Failed to send data to card scraw_send()=%i "
                               "reason=0x%llx.\n",
                               err_send, ctx.err_reason);
                    }
                }
                else
                {
                    printf("Failed to connect to card scraw_card_connect()=%i "
                           "reason=0x%llx.\n",
                           err_connect, ctx.err_reason);
                }

                int const err_disconnect = scraw_card_disconnect(&ctx);
                if (err_disconnect == 0)
                {
                    printf(
                        "Disconnected from card scraw_card_disconnect()=%i.\n",
                        err_disconnect);
                }
                else
                {
                    printf("Failed to disconnect from card "
                           "scraw_card_disconnect()=%i reason=0x%llx.\n",
                           err_disconnect, ctx.err_reason);
                    /**
                     * The response might have been received, but this still
                     * causes the program itself to fail.
                     */
                    main_ret = EXIT_FAILURE;
                }
            }
        }
        else
        {
            printf("Failed to search for readers "
                   "scraw_reader_search_begin()=%i reason=0x%llx.\n",
                   err_reader_search_begin, ctx.err_reason);
        }

        int const err_fini = scraw_fini(&ctx);
        if (err_fini != 0)
        {
            printf("Failed to deinitialize the library scraw_fini()=%i "
                   "reason=0x%llx.\n",
                   err_fini, ctx.err_reason);
            /**
             * The response might have been received, but this still causes the
             * program itself to fail.
             */
            main_ret = EXIT_FAILURE;
        }
    }
    else
    {
        printf(
            "Failed to initialize the library scraw_init()=%i reason=0x%llx.\n",
            err_init, ctx.err_reason);
    }

    if (reader_name_selected != NULL)
    {
        free(reader_name_selected);
    }
    return main_ret;
}
