#pragma once

#include <vector>
#include <cstdint>

static inline __attribute__((always_inline)) uint64_t read_uint64(const char *src)
{
    return __builtin_bswap64(*(uint64_t *)(src));
}

static inline __attribute__((always_inline)) uint32_t read_uint32(const char *src)
{
    return __builtin_bswap32(*(uint32_t *)(src));
}

static inline __attribute__((always_inline)) uint16_t read_uint16(const char *src)
{
    return __builtin_bswap16(*(uint16_t *)(src));
}

static inline __attribute__((always_inline)) char read_byte(const char *src)
{
    return *src;
}

namespace itch
{

    enum class OrderSide
    {
        BUY,
        SELL
    };

    struct PriceLevel
    {
        uint32_t price;
        uint32_t quantity;
    };

    template <OrderSide S>
    class BookSide
    {

        using iterator = typename std::vector<PriceLevel>::iterator;
        using compare = std::conditional_t<S == OrderSide::BUY, std::greater<>, std::less<>>;

    public:
        BookSide() : m_levels(64) {}
        void insert(const uint32_t price, const uint32_t quantity)
        {
            compare price_compare;

            std::reverse_iterator<iterator> it;

            for (it = m_levels.rbegin(); it != m_levels.rend(); ++it)
            {
                if (it->price == price)
                {
                    it->quantity += quantity;
                    return;
                }
                if (price_compare(price, it->price))
                    break;
            }

            m_levels.insert(it.base(), PriceLevel{price, quantity});
        }

        void reduce(const uint32_t price, const uint32_t quantity)
        {
            for (auto it = m_levels.rbegin(); it != m_levels.rend(); ++it)
            {
                if (it->price == price)
                {
                    if (it->quantity == quantity)
                        m_levels.erase(--(it.base()));
                    else
                        it->quantity -= quantity;
                    return;
                }
            }
        }

        uint32_t best_price()
        {
            return m_levels.back().price;
        }

    private:
        std::vector<PriceLevel> m_levels;
    };

    class OrderBook
    {
    public:
        void insert_buy(const uint32_t price, const uint32_t quantity)
        {
            m_buys.insert(price, quantity);
        }

        void insert_sell(const uint32_t price, const uint32_t quantity)
        {
            m_sells.insert(price, quantity);
        }

        void reduce_buy(const uint32_t price, const uint32_t quantity)
        {
            m_buys.reduce(price, quantity);
        }

        void reduce_sell(const uint32_t price, const uint32_t quantity)
        {
            m_sells.reduce(price, quantity);
        }

    private:
        BookSide<OrderSide::BUY> m_buys;
        BookSide<OrderSide::SELL> m_sells;
    };

    struct Order
    {
        uint32_t price;
        uint32_t quantity;
        OrderBook *book;
        bool buy;
    };

    class Market
    {
        constexpr static const size_t MAX_ACTIVE_ORDERS = 1 << 30;
        constexpr static const size_t MAX_BOOKS = 1 << 16;

    public:
        Market()
            : m_order_books(MAX_BOOKS), m_orders(MAX_ACTIVE_ORDERS){};

        Order *get_order(const uint64_t oid)
        {
            return &m_orders[oid % MAX_ACTIVE_ORDERS];
        }

        OrderBook *get_book(const uint16_t stock)
        {
            return &m_order_books[stock];
        }

        void on_add_order(const char *msg)
        {
            auto stock = read_uint16(msg + 1);
            auto oid = read_uint64(msg + 11);
            auto side = read_byte(msg + 19);
            auto quantity = read_uint32(msg + 20);
            auto price = read_uint32(msg + 32);

            auto order = get_order(oid);
            auto book = get_book(stock);

            order->quantity = quantity;
            order->price = price;
            order->buy = side == 'B';
            order->book = book;

            if (order->buy)
                book->insert_buy(price, quantity);
            else
                book->insert_sell(price, quantity);
        }

        void on_execute_order(const char *msg)
        {
            auto oid = read_uint64(msg + 11);
            auto quantity = read_uint32(msg + 19);
            auto order = get_order(oid);

            if (order->buy)
                order->book->reduce_buy(order->price, quantity);
            else
                order->book->reduce_sell(order->price, quantity);
        }

        void on_cancel_order(const char *msg)
        {
            auto oid = read_uint64(msg + 11);
            auto quantity = read_uint32(msg + 19);
            auto order = get_order(oid);

            if (order->buy)
                order->book->reduce_buy(order->price, quantity);
            else
                order->book->reduce_sell(order->price, quantity);
        }

        void on_delete_order(const char *msg)
        {
            auto oid = read_uint64(msg + 11);
            auto order = get_order(oid);

            if (order->buy)
                order->book->reduce_buy(order->price, order->quantity);
            else
                order->book->reduce_sell(order->price, order->quantity);
        }

        void on_replace_order(const char *msg)
        {
            auto oid = read_uint64(msg + 11);
            auto new_oid = read_uint64(msg + 19);
            auto quantity = read_uint32(msg + 27);
            auto price = read_uint32(msg + 31);

            auto old_order = get_order(oid);
            auto order = get_order(new_oid);

            order->quantity = quantity;
            order->price = price;
            order->book = old_order->book;
            order->buy = old_order->buy;

            if (old_order->buy)
            {
                order->book->reduce_buy(old_order->price, old_order->quantity);
                order->book->insert_buy(price, quantity);
            }
            else
            {
                order->book->reduce_sell(old_order->price, old_order->quantity);
                order->book->insert_sell(price, quantity);
            }
        }

    private:
        std::vector<OrderBook> m_order_books;
        std::vector<Order> m_orders;
    };

}