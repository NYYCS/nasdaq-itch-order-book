#pragma once

namespace itch
{
    enum class MessageType
    {
        SYSTEM_EVENT = 'S',
        STOCK_DIRECTORY = 'R',
        TRADING_ACTION = 'H',
        REG_SHO_RESTRICT = 'Y',
        MPID_POSITION = 'L',
        MWCB_DECLINE = 'V',
        MWCB_STATUS = 'W',
        IPO_QUOTE_UPDATE = 'K',
        ADD_ORDER = 'A',
        ADD_ORDER_MPID = 'F',
        EXECUTE_ORDER = 'E',
        EXECUTE_ORDER_WITH_PRICE = 'C',
        CANCEL_ORDER = 'X',
        DELETE_ORDER = 'D',
        REPLACE_ORDER = 'U',
        TRADE = 'P',
        CROSS_TRADE = 'Q',
        BROKEN_TRADE = 'B',
        NET_ORDER_IMBALANCE = 'I',
        RETAIL_PRICE_IMPROVEMENT = 'N',
        PROCESS_LULD_AUCTION_COLLAR_MESSAGE = 'J'
    };

}