#include "server.h"

int main() {
    // data = fopen("jontan.data", "w+");
    // uint32_t buf = 0;
    // fwrite(&buf, (sizeof(buf)), 1, data);

    // data = fopen("shop.data", "w+");
    // uint32_t buf = 0;
    // fwrite(&buf, (sizeof(buf)), 1, data);

    orders = fopen("orders.data", "w+");
    uint32_t buf = 0;
    fwrite(&buf, (sizeof(buf)), 1, orders);
    return 0;
}