#include "scraw.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int32_t main()
{
    int32_t main_ret = EXIT_FAILURE;
    int32_t ret;
    scraw_st ctx;
    char *reader_name_selected = NULL;
    if (scraw_init(&ctx) == 0)
    {
        if (scraw_reader_search_begin(&ctx) == 0)
        {
            bool reader_found = false;
            char const *reader_name_tmp = NULL;
            for (uint32_t reader_idx = 0U;; ++reader_idx)
            {
                ret = scraw_reader_search_next(&ctx, &reader_name_tmp);
                if (ret == 0)
                {
                    printf("Reader %u: %s\n", reader_idx, reader_name_tmp);
                    if (reader_found == false)
                    {
                        printf("Selecting reader %u\n", reader_idx);
                        uint64_t const reader_name_len =
                            strlen(reader_name_tmp);
                        reader_name_selected = malloc(reader_name_len);
                        if (reader_name_selected == NULL)
                        {
                            printf(
                                "Failed to allocate memory for reader name\n");
                        }
                        else
                        {
                            memcpy(reader_name_selected, reader_name_tmp,
                                   reader_name_len);
                            if (scraw_reader_select(&ctx,
                                                    reader_name_selected) == 0)
                            {
                                printf("Selected reader %u\n", reader_idx);
                                reader_found = true;
                                break; /* Selected a reader so can carry on. */
                            }
                        }
                    }
                }
                else if (ret == 1)
                {
                    printf("Reached end of reader list\n");
                    break;
                }
                else
                {
                    printf("Error while searching reader list\n");
                    break;
                }
            }
            if (scraw_reader_search_end(&ctx) != 0)
            {
                printf("Failed to end search\n");
                /**
                 * This failure leads to a memory leak but is not critical so we
                 * carry on.
                 */
            }
            if (reader_found)
            {
                if (scraw_card_connect(&ctx, SCRAW_PROTO_T0) == 0)
                {
                    printf("Connected to card\n");
                    uint8_t const raw_data[] = {0xA0, 0xA4, 0x00, 0x00,
                                                0x02, 0x3F, 0x00};
                    scraw_raw_st raw = {
                        .buf = raw_data,
                        .len = sizeof(raw_data) / sizeof(raw_data[0U]),
                    };
                    uint8_t res_data[1024U] = {0};
                    scraw_res_st res = {
                        .buf = res_data,
                        .buf_len = sizeof(res_data) / sizeof(res_data[0U]),
                        .res_len = 0,
                    };
                    if (scraw_send(&ctx, &raw, &res) == 0)
                    {
                        printf("Sent data to card\n");
                        printf("Response: ");
                        for (uint32_t res_idx = 0U; res_idx < res.res_len;
                             ++res_idx)
                        {
                            printf("%02X ", res.buf[res_idx]);
                        }
                        printf("\n");
                        main_ret = EXIT_SUCCESS;
                    }
                    else
                    {
                        printf("Failed to send data to card\n");
                    }

                    if (scraw_card_disconnect(&ctx) == 0)
                    {
                        printf("Disconnected from card\n");
                    }
                    else
                    {
                        printf("Failed to disconnect from card\n");
                    }
                }
                else
                {
                    printf("Failed to connect to card\n");
                }
            }
        }
        else
        {
            printf("Failed to search for readers\n");
        }
        if (scraw_fini(&ctx) != 0)
        {
            printf("Failed to deinitialize the library\n");
        }
    }
    else
    {
        printf("Failed to initialize the library\n");
    }
    if (reader_name_selected != NULL)
    {
        free(reader_name_selected);
    }
    return main_ret;
}
