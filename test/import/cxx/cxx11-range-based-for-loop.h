// https://en.wikipedia.org/wiki/C%2B%2B11#Range-based_for_loop

// #1
int my_array[5] = {1, 2, 3, 4, 5};
// double the value of each element in my_array:
for (int &x : my_array) {
    x *= 2;
}
// similar but also using type inference for array elements
for (auto &x : my_array) {
    x *= 2;
}
