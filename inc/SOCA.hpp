#pragma once

#include <array>

template<uint8_t rule, size_t size>
constexpr std::array<uint8_t, size> soca_2itr_forward(std::array<uint8_t, size> arr) {
  constexpr static size_t half_size = size / 2;

  // [0, half_size) -> t - 1
  // [half_size, size) -> t

  // t + 1 leftmost
  arr[0] ^= (((rule >> ((((arr[half_size] & 0b00000011) << 1)) |
                        ((arr[half_size + 1] & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((arr[half_size] & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((arr[half_size] & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((arr[half_size] & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((arr[half_size] & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((arr[half_size] & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((arr[half_size] & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((arr[half_size] & 0b11000000) >> 6) |
                        ((arr[size - 1] & 0b00000001) << 2))) &
              1)
             << 7);

  // t + 1
  for (int i = 1; i < half_size - 2; ++i) {
    arr[i] ^= (((rule >> ((((arr[half_size + i] & 0b00000011) << 1)) |
                          ((arr[half_size + i + 1] & 0b10000000) >> 7))) &
                1)
               << 0) |
              (((rule >> ((arr[half_size + i] & 0b00000111) >> 0)) & 1) << 1) |
              (((rule >> ((arr[half_size + i] & 0b00001110) >> 1)) & 1) << 2) |
              (((rule >> ((arr[half_size + i] & 0b00011100) >> 2)) & 1) << 3) |
              (((rule >> ((arr[half_size + i] & 0b00111000) >> 3)) & 1) << 4) |
              (((rule >> ((arr[half_size + i] & 0b01110000) >> 4)) & 1) << 5) |
              (((rule >> ((arr[half_size + i] & 0b11100000) >> 5)) & 1) << 6) |
              (((rule >> (((arr[half_size + i] & 0b11000000) >> 6) |
                          ((arr[half_size + i - 1] & 0b00000001) << 2))) &
                1)
               << 7);
  }

  // t + 1 rightmost
  arr[half_size - 1] ^=
  (((rule >> (((arr[size - 1] & 0b00000011) << 1) |
              ((arr[half_size] & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((arr[size - 1] & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((arr[size - 1] & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((arr[size - 1] & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((arr[size - 1] & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((arr[size - 1] & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((arr[size - 1] & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((arr[size - 1] & 0b11000000) >> 6) |
              ((arr[size - 2] & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t + 1
  // [half_size, size) -> t

  // t + 2 leftmost
  arr[half_size] ^=
  (((rule >> ((((arr[0] & 0b00000011) << 1)) | ((arr[1] & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((arr[0] & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((arr[0] & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((arr[0] & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((arr[0] & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((arr[0] & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((arr[0] & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((arr[0] & 0b11000000) >> 6) |
              ((arr[half_size - 1] & 0b00000001) << 2))) &
    1)
   << 7);

  // t + 2
  for (int i = 1; i < half_size - 1; ++i) {
    arr[half_size + i] ^= (((rule >> ((((arr[i] & 0b00000011) << 1)) |
                                      ((arr[i + 1] & 0b10000000) >> 7))) &
                            1)
                           << 0) |
                          (((rule >> ((arr[i] & 0b00000111) >> 0)) & 1) << 1) |
                          (((rule >> ((arr[i] & 0b00001110) >> 1)) & 1) << 2) |
                          (((rule >> ((arr[i] & 0b00011100) >> 2)) & 1) << 3) |
                          (((rule >> ((arr[i] & 0b00111000) >> 3)) & 1) << 4) |
                          (((rule >> ((arr[i] & 0b01110000) >> 4)) & 1) << 5) |
                          (((rule >> ((arr[i] & 0b11100000) >> 5)) & 1) << 6) |
                          (((rule >> (((arr[i] & 0b11000000) >> 6) |
                                      ((arr[i - 1] & 0b00000001) << 2))) &
                            1)
                           << 7);
  }

  // t + 2 rightmost
  arr[size - 1] ^=
  (((rule >> (((arr[half_size - 1] & 0b00000011) << 1) |
              ((arr[0] & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((arr[half_size - 1] & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((arr[half_size - 1] & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((arr[half_size - 1] & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((arr[half_size - 1] & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((arr[half_size - 1] & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((arr[half_size - 1] & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((arr[half_size - 1] & 0b11000000) >> 6) |
              ((arr[half_size - 2] & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t + 1
  // [half_size, size) -> t + 2

  return arr;
}

template<uint8_t rule, size_t size>
constexpr std::array<uint8_t, size> soca_2itr_reverse(std::array<uint8_t, size> arr) {
  constexpr static size_t half_size = size / 2;

  // [0, half_size) -> t + 1
  // [half_size, size) -> t + 2

 // t leftmost
  arr[half_size] ^=
  (((rule >> ((((arr[0] & 0b00000011) << 1)) | ((arr[1] & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((arr[0] & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((arr[0] & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((arr[0] & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((arr[0] & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((arr[0] & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((arr[0] & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((arr[0] & 0b11000000) >> 6) |
              ((arr[half_size - 1] & 0b00000001) << 2))) &
    1)
   << 7);

  // t
  for (int i = 1; i < half_size - 1; ++i) {
    arr[half_size + i] ^= (((rule >> ((((arr[i] & 0b00000011) << 1)) |
                                      ((arr[i + 1] & 0b10000000) >> 7))) &
                            1)
                           << 0) |
                          (((rule >> ((arr[i] & 0b00000111) >> 0)) & 1) << 1) |
                          (((rule >> ((arr[i] & 0b00001110) >> 1)) & 1) << 2) |
                          (((rule >> ((arr[i] & 0b00011100) >> 2)) & 1) << 3) |
                          (((rule >> ((arr[i] & 0b00111000) >> 3)) & 1) << 4) |
                          (((rule >> ((arr[i] & 0b01110000) >> 4)) & 1) << 5) |
                          (((rule >> ((arr[i] & 0b11100000) >> 5)) & 1) << 6) |
                          (((rule >> (((arr[i] & 0b11000000) >> 6) |
                                      ((arr[i - 1] & 0b00000001) << 2))) &
                            1)
                           << 7);
  }

  // t rightmost
  arr[size - 1] ^=
  (((rule >> (((arr[half_size - 1] & 0b00000011) << 1) |
              ((arr[0] & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((arr[half_size - 1] & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((arr[half_size - 1] & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((arr[half_size - 1] & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((arr[half_size - 1] & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((arr[half_size - 1] & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((arr[half_size - 1] & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((arr[half_size - 1] & 0b11000000) >> 6) |
              ((arr[half_size - 2] & 0b00000001) << 2))) &
    1)
   << 7);

 // [0, half_size) -> t + 1
 // [half_size, size) -> t

  // t - 1 leftmost
  arr[0] ^= (((rule >> ((((arr[half_size] & 0b00000011) << 1)) |
                        ((arr[half_size + 1] & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((arr[half_size] & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((arr[half_size] & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((arr[half_size] & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((arr[half_size] & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((arr[half_size] & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((arr[half_size] & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((arr[half_size] & 0b11000000) >> 6) |
                        ((arr[size - 1] & 0b00000001) << 2))) &
              1)
             << 7);

  // t - 1
  for (int i = 1; i < half_size - 2; ++i) {
    arr[i] ^= (((rule >> ((((arr[half_size + i] & 0b00000011) << 1)) |
                          ((arr[half_size + i + 1] & 0b10000000) >> 7))) &
                1)
               << 0) |
              (((rule >> ((arr[half_size + i] & 0b00000111) >> 0)) & 1) << 1) |
              (((rule >> ((arr[half_size + i] & 0b00001110) >> 1)) & 1) << 2) |
              (((rule >> ((arr[half_size + i] & 0b00011100) >> 2)) & 1) << 3) |
              (((rule >> ((arr[half_size + i] & 0b00111000) >> 3)) & 1) << 4) |
              (((rule >> ((arr[half_size + i] & 0b01110000) >> 4)) & 1) << 5) |
              (((rule >> ((arr[half_size + i] & 0b11100000) >> 5)) & 1) << 6) |
              (((rule >> (((arr[half_size + i] & 0b11000000) >> 6) |
                          ((arr[half_size + i - 1] & 0b00000001) << 2))) &
                1)
               << 7);
  }

  // t - 1 rightmost
  arr[half_size - 1] ^=
  (((rule >> (((arr[size - 1] & 0b00000011) << 1) |
              ((arr[half_size] & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((arr[size - 1] & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((arr[size - 1] & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((arr[size - 1] & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((arr[size - 1] & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((arr[size - 1] & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((arr[size - 1] & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((arr[size - 1] & 0b11000000) >> 6) |
              ((arr[size - 2] & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t - 1
  // [half_size, size) -> t

  return arr;
}

template<size_t size>
constexpr std::array<uint8_t, size> soca_2itr_forward_for_testing(std::array<uint8_t, size> arr, uint8_t rule) {
  constexpr static size_t half_size = size / 2;

  // [0, half_size) -> t - 1
  // [half_size, size) -> t

  // t + 1 leftmost
  arr[0] ^= (((rule >> ((((arr[half_size] & 0b00000011) << 1)) |
                        ((arr[half_size + 1] & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((arr[half_size] & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((arr[half_size] & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((arr[half_size] & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((arr[half_size] & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((arr[half_size] & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((arr[half_size] & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((arr[half_size] & 0b11000000) >> 6) |
                        ((arr[size - 1] & 0b00000001) << 2))) &
              1)
             << 7);

  // t + 1
  for (int i = 1; i < half_size - 2; ++i) {
    arr[i] ^= (((rule >> ((((arr[half_size + i] & 0b00000011) << 1)) |
                          ((arr[half_size + i + 1] & 0b10000000) >> 7))) &
                1)
               << 0) |
              (((rule >> ((arr[half_size + i] & 0b00000111) >> 0)) & 1) << 1) |
              (((rule >> ((arr[half_size + i] & 0b00001110) >> 1)) & 1) << 2) |
              (((rule >> ((arr[half_size + i] & 0b00011100) >> 2)) & 1) << 3) |
              (((rule >> ((arr[half_size + i] & 0b00111000) >> 3)) & 1) << 4) |
              (((rule >> ((arr[half_size + i] & 0b01110000) >> 4)) & 1) << 5) |
              (((rule >> ((arr[half_size + i] & 0b11100000) >> 5)) & 1) << 6) |
              (((rule >> (((arr[half_size + i] & 0b11000000) >> 6) |
                          ((arr[half_size + i - 1] & 0b00000001) << 2))) &
                1)
               << 7);
  }

  // t + 1 rightmost
  arr[half_size - 1] ^=
  (((rule >> (((arr[size - 1] & 0b00000011) << 1) |
              ((arr[half_size] & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((arr[size - 1] & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((arr[size - 1] & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((arr[size - 1] & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((arr[size - 1] & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((arr[size - 1] & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((arr[size - 1] & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((arr[size - 1] & 0b11000000) >> 6) |
              ((arr[size - 2] & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t + 1
  // [half_size, size) -> t

  // t + 2 leftmost
  arr[half_size] ^=
  (((rule >> ((((arr[0] & 0b00000011) << 1)) | ((arr[1] & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((arr[0] & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((arr[0] & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((arr[0] & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((arr[0] & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((arr[0] & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((arr[0] & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((arr[0] & 0b11000000) >> 6) |
              ((arr[half_size - 1] & 0b00000001) << 2))) &
    1)
   << 7);

  // t + 2
  for (int i = 1; i < half_size - 1; ++i) {
    arr[half_size + i] ^= (((rule >> ((((arr[i] & 0b00000011) << 1)) |
                                      ((arr[i + 1] & 0b10000000) >> 7))) &
                            1)
                           << 0) |
                          (((rule >> ((arr[i] & 0b00000111) >> 0)) & 1) << 1) |
                          (((rule >> ((arr[i] & 0b00001110) >> 1)) & 1) << 2) |
                          (((rule >> ((arr[i] & 0b00011100) >> 2)) & 1) << 3) |
                          (((rule >> ((arr[i] & 0b00111000) >> 3)) & 1) << 4) |
                          (((rule >> ((arr[i] & 0b01110000) >> 4)) & 1) << 5) |
                          (((rule >> ((arr[i] & 0b11100000) >> 5)) & 1) << 6) |
                          (((rule >> (((arr[i] & 0b11000000) >> 6) |
                                      ((arr[i - 1] & 0b00000001) << 2))) &
                            1)
                           << 7);
  }

  // t + 2 rightmost
  arr[size - 1] ^=
  (((rule >> (((arr[half_size - 1] & 0b00000011) << 1) |
              ((arr[0] & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((arr[half_size - 1] & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((arr[half_size - 1] & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((arr[half_size - 1] & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((arr[half_size - 1] & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((arr[half_size - 1] & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((arr[half_size - 1] & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((arr[half_size - 1] & 0b11000000) >> 6) |
              ((arr[half_size - 2] & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t + 1
  // [half_size, size) -> t + 2

  return arr;
}

template<size_t size>
constexpr std::array<uint8_t, size> soca_2itr_reverse_for_testing(std::array<uint8_t, size> arr, uint8_t rule) {
  constexpr static size_t half_size = size / 2;

  // [0, half_size) -> t + 1
  // [half_size, size) -> t + 2

 // t leftmost
  arr[half_size] ^=
  (((rule >> ((((arr[0] & 0b00000011) << 1)) | ((arr[1] & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((arr[0] & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((arr[0] & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((arr[0] & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((arr[0] & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((arr[0] & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((arr[0] & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((arr[0] & 0b11000000) >> 6) |
              ((arr[half_size - 1] & 0b00000001) << 2))) &
    1)
   << 7);

  // t
  for (int i = 1; i < half_size - 1; ++i) {
    arr[half_size + i] ^= (((rule >> ((((arr[i] & 0b00000011) << 1)) |
                                      ((arr[i + 1] & 0b10000000) >> 7))) &
                            1)
                           << 0) |
                          (((rule >> ((arr[i] & 0b00000111) >> 0)) & 1) << 1) |
                          (((rule >> ((arr[i] & 0b00001110) >> 1)) & 1) << 2) |
                          (((rule >> ((arr[i] & 0b00011100) >> 2)) & 1) << 3) |
                          (((rule >> ((arr[i] & 0b00111000) >> 3)) & 1) << 4) |
                          (((rule >> ((arr[i] & 0b01110000) >> 4)) & 1) << 5) |
                          (((rule >> ((arr[i] & 0b11100000) >> 5)) & 1) << 6) |
                          (((rule >> (((arr[i] & 0b11000000) >> 6) |
                                      ((arr[i - 1] & 0b00000001) << 2))) &
                            1)
                           << 7);
  }

  // t rightmost
  arr[size - 1] ^=
  (((rule >> (((arr[half_size - 1] & 0b00000011) << 1) |
              ((arr[0] & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((arr[half_size - 1] & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((arr[half_size - 1] & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((arr[half_size - 1] & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((arr[half_size - 1] & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((arr[half_size - 1] & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((arr[half_size - 1] & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((arr[half_size - 1] & 0b11000000) >> 6) |
              ((arr[half_size - 2] & 0b00000001) << 2))) &
    1)
   << 7);

 // [0, half_size) -> t + 1
 // [half_size, size) -> t

  // t - 1 leftmost
  arr[0] ^= (((rule >> ((((arr[half_size] & 0b00000011) << 1)) |
                        ((arr[half_size + 1] & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((arr[half_size] & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((arr[half_size] & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((arr[half_size] & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((arr[half_size] & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((arr[half_size] & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((arr[half_size] & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((arr[half_size] & 0b11000000) >> 6) |
                        ((arr[size - 1] & 0b00000001) << 2))) &
              1)
             << 7);

  // t - 1
  for (int i = 1; i < half_size - 2; ++i) {
    arr[i] ^= (((rule >> ((((arr[half_size + i] & 0b00000011) << 1)) |
                          ((arr[half_size + i + 1] & 0b10000000) >> 7))) &
                1)
               << 0) |
              (((rule >> ((arr[half_size + i] & 0b00000111) >> 0)) & 1) << 1) |
              (((rule >> ((arr[half_size + i] & 0b00001110) >> 1)) & 1) << 2) |
              (((rule >> ((arr[half_size + i] & 0b00011100) >> 2)) & 1) << 3) |
              (((rule >> ((arr[half_size + i] & 0b00111000) >> 3)) & 1) << 4) |
              (((rule >> ((arr[half_size + i] & 0b01110000) >> 4)) & 1) << 5) |
              (((rule >> ((arr[half_size + i] & 0b11100000) >> 5)) & 1) << 6) |
              (((rule >> (((arr[half_size + i] & 0b11000000) >> 6) |
                          ((arr[half_size + i - 1] & 0b00000001) << 2))) &
                1)
               << 7);
  }

  // t - 1 rightmost
  arr[half_size - 1] ^=
  (((rule >> (((arr[size - 1] & 0b00000011) << 1) |
              ((arr[half_size] & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((arr[size - 1] & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((arr[size - 1] & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((arr[size - 1] & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((arr[size - 1] & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((arr[size - 1] & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((arr[size - 1] & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((arr[size - 1] & 0b11000000) >> 6) |
              ((arr[size - 2] & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t - 1
  // [half_size, size) -> t

  return arr;
}
