#include "board/Board.h"
#include "board/Internal.h"

#define TOTAL_FLASH_PAGES 12

namespace
{
    Board::detail::map::flashPage_t pageDescriptor[TOTAL_FLASH_PAGES] = {
        {
            .address = 0x8000000,
            .size    = 16384,
        },

        {
            .address = 0x8004000,
            .size    = 16384,
        },

        {
            .address = 0x8008000,
            .size    = 16384,
        },

        {
            .address = 0x800C000,
            .size    = 16384,
        },

        {
            .address = 0x8010000,
            .size    = 65536,
        },

        {
            .address = 0x8020000,
            .size    = 131072,
        },

        {
            .address = 0x8040000,
            .size    = 131072,
        },

        {
            .address = 0x8060000,
            .size    = 131072,
        },

        {
            .address = 0x8080000,
            .size    = 131072,
        },

        {
            .address = 0x80A0000,
            .size    = 131072,
        },

        {
            .address = 0x80C0000,
            .size    = 131072,
        },

        {
            .address = 0x80E0000,
            .size    = 131072,
        },
    };
}

namespace Board
{
    namespace detail
    {
        namespace map
        {
            Board::detail::map::flashPage_t& flashPageDescriptor(size_t pageIndex)
            {
                if (pageIndex >= TOTAL_FLASH_PAGES)
                    return pageDescriptor[TOTAL_FLASH_PAGES - 1];

                return pageDescriptor[pageIndex];
            }
        }    // namespace map
    }        // namespace detail
}    // namespace Board