#include "scraw.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Free the reader list allocated when searching.
 * @param ctx
 * @return 0 on success, -1 on failure.
 */
static int32_t scraw_reader_list_free(scraw_st *const ctx)
{
    if (ctx->ctx == 0)
    {
        return 0; /* Nothing to free because context is not present. */
    }
    if (ctx->reader_list.names != NULL)
    {
        if (SCardFreeMemory(ctx->ctx, ctx->reader_list.names) !=
            SCARD_S_SUCCESS)
        {
            return -1;
        }
    }
    /* Setting fields to ensure list is not mistaken for being non-empty. */
    ctx->reader_list.names = NULL;
    ctx->reader_list.list_valid = false;
    ctx->reader_list.names_len = 0U;
    ctx->reader_list.next_offset = 0U;
    return 0;
}

int32_t scraw_init(scraw_st *const ctx)
{
    /**
     * Assume here the ctx struct is uninitialized and that no one normal will
     * init twice.
     */
    memset(ctx, 0U, sizeof(scraw_st));
    SCARDCONTEXT ctx_win;
    /* Create a context. */
    LONG ret = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &ctx_win);
    if (ret != SCARD_S_SUCCESS)
    {
        return -1;
    }
    ctx->ctx = ctx_win;

    /**
     * Just to be sure there won't be any undefined behavior when calling
     * 'select' without searching first. Note that memset already set this to 0
     * so if false==0, this is not necessary.
     */
    ctx->reader_list.list_valid = false;
    return 0;
}

int32_t scraw_fini(scraw_st *const ctx)
{
    if (ctx->ctx == 0)
    {
        return -1;
    }
    LONG ret;
    if (ctx->ctx != 0)
    {
        /* Free memory used when searching for reader names. */
        if (scraw_reader_list_free(ctx) != 0)
        {
            return -1;
        }
        /* Cancel all outstanding actions within the context. */
        ret = SCardCancel(ctx->ctx);
        if (ret != SCARD_S_SUCCESS)
        {
            return -1;
        }
        /* Release the context. */
        ret = SCardReleaseContext(ctx->ctx);
        if (ret != SCARD_S_SUCCESS)
        {
            return -1;
        }
    }
    return 0;
}

int32_t scraw_reader_search(scraw_st *const ctx)
{
    if (ctx->ctx == 0)
    {
        return -1;
    }
    memset(&ctx->reader_list, 0U, sizeof(scraw_reader_list_st));

    /* Avoid leaks by freeing the old names list. */
    if (scraw_reader_list_free(ctx) != 0)
    {
        return -1;
    }

#ifdef PAL_WIN
    char const reader_groups[] = SCARD_ALL_READERS;
#endif
#if defined(PAL_LNX) || defined(PAL_MAC)
    char const *const reader_groups = NULL;
#endif
    LPSTR name_buf = NULL;
    DWORD name_buf_len = SCARD_AUTOALLOCATE;
    /**
     * When auto-allocating, this gets the pointer of the allocated buffer hence
     * the cast for pointer to names.
     */
    LONG ret = SCardListReaders(ctx->ctx, reader_groups, (char *)&name_buf,
                                &name_buf_len);
    switch (ret)
    {
    case SCARD_S_SUCCESS:
        ctx->reader_list.next_offset = 0U;
        ctx->reader_list.names = name_buf;
        ctx->reader_list.names_len =
            (uint32_t)name_buf_len; /* Safe cast due to bound check. */
        ctx->reader_list.list_valid = true;
        break;
    case SCARD_E_NO_READERS_AVAILABLE:
        ctx->reader_list.next_offset = 0U;
        ctx->reader_list.names = NULL;
        ctx->reader_list.names_len = 0;
        ctx->reader_list.list_valid = true;
        break;
    default:
        ctx->reader_list.list_valid = false;
        return -1;
    }
    return 0;
}

int32_t scraw_reader_next(scraw_st *const ctx, char const **const reader_name)
{
    if (ctx->ctx == 0)
    {
        return -1;
    }
    if (ctx->reader_list.list_valid == false)
    {
        /**
         * There is no list so one should be completed succesfully before
         * calling 'next'.
         */
        return -1;
    }
    if (ctx->reader_list.names == NULL ||
        ctx->reader_list.next_offset >= ctx->reader_list.names_len)
    {
        /**
         * There are no readers available or reached end of reader names array.
         */
        return 1;
    }
    *reader_name = &ctx->reader_list.names[ctx->reader_list.next_offset];
    uint64_t const len_tmp = strlen(*reader_name);
    if (len_tmp + 2U > UINT32_MAX)
    {
        /* Who in their right mind sets a name longer than 4 billion chars... */
        return -1;
    }
    ctx->reader_list.next_offset +=
        (uint32_t)(len_tmp +
                   2U); /* Move offset to where the next name begins. */
    return 0;
}

int32_t scraw_reader_select(scraw_st *const ctx, char const *const reader_name)
{
    if (ctx->ctx == 0)
    {
        return -1;
    }
    /**
     * Avoid using up memory for the reader name list if it won't be needed
     * any longer.
     */
    if (scraw_reader_list_free(ctx) != 0)
    {
        return -1;
    }
    ctx->reader_selected.name = reader_name;
    return 0;
}

int32_t scraw_card_connect(scraw_st *const ctx, scraw_proto_et const proto)
{
    if (ctx->ctx == 0)
    {
        return -1;
    }
    if (ctx->reader_selected.name == NULL)
    {
        /* Reader was not selected. */
        return -1;
    }
    LONG ret;
    if (ctx->card == 0)
    {
        ret = SCardConnect(
            ctx->ctx, ctx->reader_selected.name, SCARD_SHARE_SHARED,
            (uint32_t)((proto == SCRAW_PROTO_T0) * SCARD_PROTOCOL_T0) +
                (uint32_t)((proto == SCRAW_PROTO_T1) * SCARD_PROTOCOL_T1),
            &ctx->card, NULL);
        if (ret != SCARD_S_SUCCESS)
        {
            return -1;
        }
    }
    else
    {
        /* A card handle is already present so will reconnect. */
        ret = SCardReconnect(
            ctx->card, SCARD_SHARE_SHARED,
            (uint32_t)((proto == SCRAW_PROTO_T0) * SCARD_PROTOCOL_T0) +
                (uint32_t)((proto == SCRAW_PROTO_T1) * SCARD_PROTOCOL_T1),
            SCARD_RESET_CARD, NULL);
        if (ret != SCARD_S_SUCCESS)
        {
            return -1;
        }
    }
    ctx->card_proto = proto;
    return 0;
}

int32_t scraw_card_disconnect(scraw_st *const ctx)
{
    if (ctx->ctx == 0)
    {
        return -1;
    }
    if (ctx->card == 0)
    {
        /* Card was never connected. */
        return -1;
    }
    LONG ret = SCardDisconnect(ctx->card, SCARD_LEAVE_CARD);
    if (ret != SCARD_S_SUCCESS)
    {
        return -1;
    }
    return 0;
}

int32_t scraw_send(scraw_st *const ctx, scraw_raw_st *const data,
                   scraw_res_st *const card_res)
{
    if (ctx->ctx == 0)
    {
        return -1;
    }
    if (ctx->card == 0)
    {
        /* No connected card to send data to. */
        return 1;
    }
    LPCSCARD_IO_REQUEST pci;
    switch (ctx->card_proto)
    {
    case SCRAW_PROTO_T0:
        pci = SCARD_PCI_T0;
        break;
    case SCRAW_PROTO_T1:
        pci = SCARD_PCI_T1;
        break;
    default:
        pci = 0;
        break;
    }
    DWORD res_len = card_res->buf_len;
    LONG ret = SCardTransmit(ctx->card, pci, data->buf, data->len, NULL,
                             card_res->buf, &res_len);
    if (ret != SCARD_S_SUCCESS)
    {
        switch (ret)
        {
        case SCARD_E_NO_SMARTCARD:
            /* No card present in the reader. */
            return 1;
        default:
            return -1;
        }
    }
    if (res_len > UINT32_MAX)
    {
        /**
         * PC/SC doesn't even support extended APDUs so this should never
         * happen.
         */
        return -1;
    }
    card_res->res_len = (uint32_t)res_len; /* Safe cast due to bound check. */
    return 0;
}
