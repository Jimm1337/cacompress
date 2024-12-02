#pragma once

#include <type_traits>
#include <iterator>

namespace soca {

/***
 * @brief 2 iterations forward of second order cellular automaton of given rule
 * @warning UNDEFINED BEHAVIOR FOR RANGES OF SIZE (LESS THAN 4 || ODD)
 * @note INPUT: range [begin, end)
 * @note IN-PLACE
 * Treats the first half of the range as t - 1 and the second half as t.
 * Output is first half as t + 1 and the second half as t + 2.
 ***/
template<uint8_t rule, typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
constexpr void forward(Itr begin, Itr end) {
  const size_t size      = std::distance(begin, end);
  const size_t half_size = std::distance(begin, end) / 2;

  // [0, half_size) -> t - 1
  // [half_size, size) -> t

  // t + 1 leftmost
  *begin ^= (((rule >> ((((*(begin + half_size) & 0b00000011) << 1)) |
                        ((*(begin + half_size + 1) & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((*(begin + half_size) & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((*(begin + half_size) & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((*(begin + half_size) & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((*(begin + half_size) & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((*(begin + half_size) & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((*(begin + half_size) & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((*(begin + half_size) & 0b11000000) >> 6) |
                        ((*(begin + size - 1) & 0b00000001) << 2))) &
              1)
             << 7);

  // t + 1
  for (auto itr = begin + 1; itr != begin + half_size - 1; ++itr) {
    const auto cond_itr = itr + half_size;

    *itr ^= (((rule >> ((((*cond_itr & 0b00000011) << 1)) |
                        ((*(cond_itr + 1) & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((*cond_itr & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((*cond_itr & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((*cond_itr & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((*cond_itr & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((*cond_itr & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((*cond_itr & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((*cond_itr & 0b11000000) >> 6) |
                        ((*(cond_itr - 1) & 0b00000001) << 2))) &
              1)
             << 7);
  }

  // t + 1 rightmost
  *(begin + half_size - 1) ^=
  (((rule >> (((*(begin + size - 1) & 0b00000011) << 1) |
              ((*(begin + half_size) & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((*(begin + size - 1) & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((*(begin + size - 1) & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((*(begin + size - 1) & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((*(begin + size - 1) & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((*(begin + size - 1) & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((*(begin + size - 1) & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((*(begin + size - 1) & 0b11000000) >> 6) |
              ((*(begin + size - 2) & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t + 1
  // [half_size, size) -> t

  // t + 2 leftmost
  *(begin + half_size) ^=
  (((rule >>
     ((((*begin & 0b00000011) << 1)) | ((*(begin + 1) & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((*begin & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((*begin & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((*begin & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((*begin & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((*begin & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((*begin & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((*begin & 0b11000000) >> 6) |
              ((*(begin + half_size - 1) & 0b00000001) << 2))) &
    1)
   << 7);

  // t + 2
  for (auto itr = begin + half_size + 1; itr != begin + size - 1; ++itr) {
    const auto cond_itr = itr - half_size;

    *itr ^= (((rule >> ((((*cond_itr & 0b00000011) << 1)) |
                        ((*(cond_itr + 1) & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((*cond_itr & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((*cond_itr & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((*cond_itr & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((*cond_itr & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((*cond_itr & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((*cond_itr & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((*cond_itr & 0b11000000) >> 6) |
                        ((*(cond_itr - 1) & 0b00000001) << 2))) &
              1)
             << 7);
  }

  // t + 2 rightmost
  *(begin + size - 1) ^=
  (((rule >> (((*(begin + half_size - 1) & 0b00000011) << 1) |
              ((*begin & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((*(begin + half_size - 1) & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((*(begin + half_size - 1) & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((*(begin + half_size - 1) & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((*(begin + half_size - 1) & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((*(begin + half_size - 1) & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((*(begin + half_size - 1) & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((*(begin + half_size - 1) & 0b11000000) >> 6) |
              ((*(begin + half_size - 2) & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t + 1
  // [half_size, size) -> t + 2
}

/***
 * @brief 2 iterations backwards of second order cellular automaton of given rule
 * @warning UNDEFINED BEHAVIOR FOR RANGES OF SIZE (LESS THAN 4 || ODD)
 * @note INPUT: range [begin, end)
 * @note IN-PLACE
 * Treats the first half of the range as t + 1 and the second half as t + 2.
 * Output is first half as t - 1 and the second half as t.
 ***/
template<uint8_t rule, typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
constexpr void reverse(Itr begin, Itr end) {
  const size_t size      = std::distance(begin, end);
  const size_t half_size = std::distance(begin, end) / 2;

  // [0, half_size) -> t + 1
  // [half_size, size) -> t + 2

  // t leftmost
  *(begin + half_size) ^=
  (((rule >>
     ((((*begin & 0b00000011) << 1)) | ((*(begin + 1) & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((*begin & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((*begin & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((*begin & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((*begin & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((*begin & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((*begin & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((*begin & 0b11000000) >> 6) |
              ((*(begin + half_size - 1) & 0b00000001) << 2))) &
    1)
   << 7);

  // t
  for (auto itr = begin + half_size + 1; itr != begin + size - 1; ++itr) {
    const auto cond_itr = itr - half_size;

    *itr ^= (((rule >> ((((*cond_itr & 0b00000011) << 1)) |
                        ((*(cond_itr + 1) & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((*cond_itr & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((*cond_itr & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((*cond_itr & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((*cond_itr & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((*cond_itr & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((*cond_itr & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((*cond_itr & 0b11000000) >> 6) |
                        ((*(cond_itr - 1) & 0b00000001) << 2))) &
              1)
             << 7);
  }

  // t rightmost
  *(begin + size - 1) ^=
  (((rule >> (((*(begin + half_size - 1) & 0b00000011) << 1) |
              ((*begin & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((*(begin + half_size - 1) & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((*(begin + half_size - 1) & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((*(begin + half_size - 1) & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((*(begin + half_size - 1) & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((*(begin + half_size - 1) & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((*(begin + half_size - 1) & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((*(begin + half_size - 1) & 0b11000000) >> 6) |
              ((*(begin + half_size - 2) & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t + 1
  // [half_size, size) -> t

  // t - 1 leftmost
  *begin ^= (((rule >> ((((*(begin + half_size) & 0b00000011) << 1)) |
                        ((*(begin + half_size + 1) & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((*(begin + half_size) & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((*(begin + half_size) & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((*(begin + half_size) & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((*(begin + half_size) & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((*(begin + half_size) & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((*(begin + half_size) & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((*(begin + half_size) & 0b11000000) >> 6) |
                        ((*(begin + size - 1) & 0b00000001) << 2))) &
              1)
             << 7);

  // t - 1
  for (auto itr = begin + 1; itr != begin + half_size - 1; ++itr) {
    const auto cond_itr = itr + half_size;

    *itr ^= (((rule >> ((((*cond_itr & 0b00000011) << 1)) |
                        ((*(cond_itr + 1) & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((*cond_itr & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((*cond_itr & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((*cond_itr & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((*cond_itr & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((*cond_itr & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((*cond_itr & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((*cond_itr & 0b11000000) >> 6) |
                        ((*(cond_itr - 1) & 0b00000001) << 2))) &
              1)
             << 7);
  }

  // t - 1 rightmost
  *(begin + half_size - 1) ^=
  (((rule >> (((*(begin + size - 1) & 0b00000011) << 1) |
              ((*(begin + half_size) & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((*(begin + size - 1) & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((*(begin + size - 1) & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((*(begin + size - 1) & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((*(begin + size - 1) & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((*(begin + size - 1) & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((*(begin + size - 1) & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((*(begin + size - 1) & 0b11000000) >> 6) |
              ((*(begin + size - 2) & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t - 1
  // [half_size, size) -> t
}

/***
 * @brief 2 iterations forward of second order cellular automaton of given rule
 * @warning UNDEFINED BEHAVIOR FOR RANGES OF SIZE (LESS THAN 4 || ODD)
 * @note INPUT: range [begin, end)
 * @note IN-PLACE
 * Treats the first half of the range as t - 1 and the second half as t.
 * Output is first half as t + 1 and the second half as t + 2.
 ***/
template<typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
constexpr void forward(Itr begin, Itr end, uint8_t rule) {
  const size_t size      = std::distance(begin, end);
  const size_t half_size = std::distance(begin, end) / 2;

  // [0, half_size) -> t - 1
  // [half_size, size) -> t

  // t + 1 leftmost
  *begin ^= (((rule >> ((((*(begin + half_size) & 0b00000011) << 1)) |
                        ((*(begin + half_size + 1) & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((*(begin + half_size) & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((*(begin + half_size) & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((*(begin + half_size) & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((*(begin + half_size) & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((*(begin + half_size) & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((*(begin + half_size) & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((*(begin + half_size) & 0b11000000) >> 6) |
                        ((*(begin + size - 1) & 0b00000001) << 2))) &
              1)
             << 7);

  // t + 1
  for (auto itr = begin + 1; itr != begin + half_size - 1; ++itr) {
    const auto cond_itr = itr + half_size;

    *itr ^= (((rule >> ((((*cond_itr & 0b00000011) << 1)) |
                        ((*(cond_itr + 1) & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((*cond_itr & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((*cond_itr & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((*cond_itr & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((*cond_itr & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((*cond_itr & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((*cond_itr & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((*cond_itr & 0b11000000) >> 6) |
                        ((*(cond_itr - 1) & 0b00000001) << 2))) &
              1)
             << 7);
  }

  // t + 1 rightmost
  *(begin + half_size - 1) ^=
  (((rule >> (((*(begin + size - 1) & 0b00000011) << 1) |
              ((*(begin + half_size) & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((*(begin + size - 1) & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((*(begin + size - 1) & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((*(begin + size - 1) & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((*(begin + size - 1) & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((*(begin + size - 1) & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((*(begin + size - 1) & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((*(begin + size - 1) & 0b11000000) >> 6) |
              ((*(begin + size - 2) & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t + 1
  // [half_size, size) -> t

  // t + 2 leftmost
  *(begin + half_size) ^=
  (((rule >>
     ((((*begin & 0b00000011) << 1)) | ((*(begin + 1) & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((*begin & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((*begin & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((*begin & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((*begin & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((*begin & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((*begin & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((*begin & 0b11000000) >> 6) |
              ((*(begin + half_size - 1) & 0b00000001) << 2))) &
    1)
   << 7);

  // t + 2
  for (auto itr = begin + half_size + 1; itr != begin + size - 1; ++itr) {
    const auto cond_itr = itr - half_size;

    *itr ^= (((rule >> ((((*cond_itr & 0b00000011) << 1)) |
                        ((*(cond_itr + 1) & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((*cond_itr & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((*cond_itr & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((*cond_itr & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((*cond_itr & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((*cond_itr & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((*cond_itr & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((*cond_itr & 0b11000000) >> 6) |
                        ((*(cond_itr - 1) & 0b00000001) << 2))) &
              1)
             << 7);
  }

  // t + 2 rightmost
  *(begin + size - 1) ^=
  (((rule >> (((*(begin + half_size - 1) & 0b00000011) << 1) |
              ((*begin & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((*(begin + half_size - 1) & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((*(begin + half_size - 1) & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((*(begin + half_size - 1) & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((*(begin + half_size - 1) & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((*(begin + half_size - 1) & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((*(begin + half_size - 1) & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((*(begin + half_size - 1) & 0b11000000) >> 6) |
              ((*(begin + half_size - 2) & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t + 1
  // [half_size, size) -> t + 2
}

/***
 * @brief 2 iterations backwards of second order cellular automaton of given rule
 * @warning UNDEFINED BEHAVIOR FOR RANGES OF SIZE (LESS THAN 4 || ODD)
 * @note INPUT: range [begin, end)
 * @note IN-PLACE
 * Treats the first half of the range as t + 1 and the second half as t + 2.
 * Output is first half as t - 1 and the second half as t.
 ***/
template<typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
constexpr void reverse(Itr begin, Itr end, uint8_t rule) {
  const size_t size      = std::distance(begin, end);
  const size_t half_size = std::distance(begin, end) / 2;

  // [0, half_size) -> t + 1
  // [half_size, size) -> t + 2

  // t leftmost
  *(begin + half_size) ^=
  (((rule >>
     ((((*begin & 0b00000011) << 1)) | ((*(begin + 1) & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((*begin & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((*begin & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((*begin & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((*begin & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((*begin & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((*begin & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((*begin & 0b11000000) >> 6) |
              ((*(begin + half_size - 1) & 0b00000001) << 2))) &
    1)
   << 7);

  // t
  for (auto itr = begin + half_size + 1; itr != begin + size - 1; ++itr) {
    const auto cond_itr = itr - half_size;

    *itr ^= (((rule >> ((((*cond_itr & 0b00000011) << 1)) |
                        ((*(cond_itr + 1) & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((*cond_itr & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((*cond_itr & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((*cond_itr & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((*cond_itr & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((*cond_itr & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((*cond_itr & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((*cond_itr & 0b11000000) >> 6) |
                        ((*(cond_itr - 1) & 0b00000001) << 2))) &
              1)
             << 7);
  }

  // t rightmost
  *(begin + size - 1) ^=
  (((rule >> (((*(begin + half_size - 1) & 0b00000011) << 1) |
              ((*begin & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((*(begin + half_size - 1) & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((*(begin + half_size - 1) & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((*(begin + half_size - 1) & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((*(begin + half_size - 1) & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((*(begin + half_size - 1) & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((*(begin + half_size - 1) & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((*(begin + half_size - 1) & 0b11000000) >> 6) |
              ((*(begin + half_size - 2) & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t + 1
  // [half_size, size) -> t

  // t - 1 leftmost
  *begin ^= (((rule >> ((((*(begin + half_size) & 0b00000011) << 1)) |
                        ((*(begin + half_size + 1) & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((*(begin + half_size) & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((*(begin + half_size) & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((*(begin + half_size) & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((*(begin + half_size) & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((*(begin + half_size) & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((*(begin + half_size) & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((*(begin + half_size) & 0b11000000) >> 6) |
                        ((*(begin + size - 1) & 0b00000001) << 2))) &
              1)
             << 7);

  // t - 1
  for (auto itr = begin + 1; itr != begin + half_size - 1; ++itr) {
    const auto cond_itr = itr + half_size;

    *itr ^= (((rule >> ((((*cond_itr & 0b00000011) << 1)) |
                        ((*(cond_itr + 1) & 0b10000000) >> 7))) &
              1)
             << 0) |
            (((rule >> ((*cond_itr & 0b00000111) >> 0)) & 1) << 1) |
            (((rule >> ((*cond_itr & 0b00001110) >> 1)) & 1) << 2) |
            (((rule >> ((*cond_itr & 0b00011100) >> 2)) & 1) << 3) |
            (((rule >> ((*cond_itr & 0b00111000) >> 3)) & 1) << 4) |
            (((rule >> ((*cond_itr & 0b01110000) >> 4)) & 1) << 5) |
            (((rule >> ((*cond_itr & 0b11100000) >> 5)) & 1) << 6) |
            (((rule >> (((*cond_itr & 0b11000000) >> 6) |
                        ((*(cond_itr - 1) & 0b00000001) << 2))) &
              1)
             << 7);
  }

  // t - 1 rightmost
  *(begin + half_size - 1) ^=
  (((rule >> (((*(begin + size - 1) & 0b00000011) << 1) |
              ((*(begin + half_size) & 0b10000000) >> 7))) &
    1)
   << 0) |
  (((rule >> ((*(begin + size - 1) & 0b00000111) >> 0)) & 1) << 1) |
  (((rule >> ((*(begin + size - 1) & 0b00001110) >> 1)) & 1) << 2) |
  (((rule >> ((*(begin + size - 1) & 0b00011100) >> 2)) & 1) << 3) |
  (((rule >> ((*(begin + size - 1) & 0b00111000) >> 3)) & 1) << 4) |
  (((rule >> ((*(begin + size - 1) & 0b01110000) >> 4)) & 1) << 5) |
  (((rule >> ((*(begin + size - 1) & 0b11100000) >> 5)) & 1) << 6) |
  (((rule >> (((*(begin + size - 1) & 0b11000000) >> 6) |
              ((*(begin + size - 2) & 0b00000001) << 2))) &
    1)
   << 7);

  // [0, half_size) -> t - 1
  // [half_size, size) -> t
}

}    // namespace soca
