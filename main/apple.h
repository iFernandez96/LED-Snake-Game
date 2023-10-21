#include <cstdint>
class Apple {
    public:
        int position[2]; // Position of the apple
        uint32_t color = strip.Color(0, 255, 0); // GREEN COLOR

        void spawn_random();
        void spawn_at(const int newPosition[2]);
        void show();
        bool check_collision(Snake);
};