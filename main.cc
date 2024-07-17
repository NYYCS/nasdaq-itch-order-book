#include <iostream>
#include <chrono>

#include <unistd.h>

#include "reader.hpp"
#include "message_type.hpp"
#include "order_book.hpp"

int main()
{
    int fd = STDIN_FILENO;
    itch::Reader<1024 * 1024> reader(fd);
    itch::Market market;

    uint32_t n_msgs = 0;

    auto t0 = std::chrono::steady_clock::now();

    while (reader.ensure(3))
    {
        n_msgs += 1;
        uint16_t msg_size = read_uint16(reader.get());
        reader.advance(2);
        char msg_type = *reader.get();
        reader.ensure(msg_size);

        switch (itch::MessageType(msg_type))
        {
        case itch::MessageType::ADD_ORDER:
        case itch::MessageType::ADD_ORDER_MPID:
            market.on_add_order(reader.get());
            break;
        case itch::MessageType::EXECUTE_ORDER:
        case itch::MessageType::EXECUTE_ORDER_WITH_PRICE:
            market.on_execute_order(reader.get());
            break;
        case itch::MessageType::CANCEL_ORDER:
            market.on_cancel_order(reader.get());
            break;
        case itch::MessageType::DELETE_ORDER:
            market.on_delete_order(reader.get());
            break;
        case itch::MessageType::REPLACE_ORDER:
            market.on_replace_order(reader.get());
            break;
        default:
            break;
        }
        reader.advance(msg_size);
    }

    auto t1 = std::chrono::steady_clock::now();

    std::cout << "Total time took: " << std::chrono::duration_cast<std::chrono::seconds>(t1 - t0).count() << "s" << std::endl;
    std::cout << "Average message process time: " << double(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()) / double(n_msgs) << "ns" << std::endl;
    
    return 0;
}
