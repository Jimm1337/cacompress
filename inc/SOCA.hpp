#pragma once

#include <type_traits>
#include <iterator>
#include <cstddef>
#include <cstdint>

namespace soca {

/***
 * @brief iteration forward of second order cellular automaton of given rule
 * @warning UNDEFINED BEHAVIOR FOR RANGES OF SIZE (LESS THAN 4 || ODD)
 * @note INPUT: range [begin, end)
 * @note IN-PLACE
 * Treats the first half of the range as t - 1 and the second half as t.
 * Output is first half as t + 1 and the second half as t.
 ***/
template<uint8_t rule, typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
constexpr void forward_front(Itr begin, Itr end) {
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
}

/***
 * @brief iteration forward of second order cellular automaton of given rule
 * @warning UNDEFINED BEHAVIOR FOR RANGES OF SIZE (LESS THAN 4 || ODD)
 * @note INPUT: range [begin, end)
 * @note IN-PLACE
 * Treats the first half of the range as t and the second half as t - 1.
 * Output is first half as t and the second half as t + 1.
 ***/
template<uint8_t rule, typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
constexpr void forward_back(Itr begin, Itr end) {
  const size_t size      = std::distance(begin, end);
  const size_t half_size = std::distance(begin, end) / 2;

  // [0, half_size) -> t
  // [half_size, size) -> t - 1

  // t + 1 leftmost
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

  // t + 1
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

  // t + 1 rightmost
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
}

/***
 * @brief iteration backwards of second order cellular automaton of given rule
 * @warning UNDEFINED BEHAVIOR FOR RANGES OF SIZE (LESS THAN 4 || ODD)
 * @note INPUT: range [begin, end)
 * @note IN-PLACE
 * Treats the first half of the range as t and the second half as t + 1.
 * Output is first half as t and the second half as t - 1.
 ***/
template<uint8_t rule, typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
constexpr void reverse_back(Itr begin, Itr end) {
  const size_t size      = std::distance(begin, end);
  const size_t half_size = std::distance(begin, end) / 2;

  // [0, half_size) -> t
  // [half_size, size) -> t + 1

  // t - 1 leftmost
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

  // t - 1
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

  // t - 1 rightmost
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
}

/***
 * @brief iteration backwards of second order cellular automaton of given rule
 * @warning UNDEFINED BEHAVIOR FOR RANGES OF SIZE (LESS THAN 4 || ODD)
 * @note INPUT: range [begin, end)
 * @note IN-PLACE
 * Treats the first half of the range as t + 1 and the second half as t.
 * Output is first half as t - 1 and the second half as t.
 ***/
template<uint8_t rule, typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
constexpr void reverse_front(Itr begin, Itr end) {
  const size_t size      = std::distance(begin, end);
  const size_t half_size = std::distance(begin, end) / 2;

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
}

/***
 * @brief iteration forward of second order cellular automaton of given rule
 * @warning UNDEFINED BEHAVIOR FOR RANGES OF SIZE (LESS THAN 4 || ODD)
 * @note INPUT: range [begin, end)
 * @note IN-PLACE
 * Treats the first half of the range as t - 1 and the second half as t.
 * Output is first half as t + 1 and the second half as t.
 ***/
template<typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
constexpr void forward_front(Itr begin, Itr end, uint8_t rule) {
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
}

/***
 * @brief iteration forward of second order cellular automaton of given rule
 * @warning UNDEFINED BEHAVIOR FOR RANGES OF SIZE (LESS THAN 4 || ODD)
 * @note INPUT: range [begin, end)
 * @note IN-PLACE
 * Treats the first half of the range as t and the second half as t - 1.
 * Output is first half as t and the second half as t + 1.
 ***/
template<typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
constexpr void forward_back(Itr begin, Itr end, uint8_t rule) {
  const size_t size      = std::distance(begin, end);
  const size_t half_size = std::distance(begin, end) / 2;

  // [0, half_size) -> t
  // [half_size, size) -> t - 1

  // t + 1 leftmost
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

  // t + 1
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

  // t + 1 rightmost
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
}

/***
 * @brief iteration backwards of second order cellular automaton of given rule
 * @warning UNDEFINED BEHAVIOR FOR RANGES OF SIZE (LESS THAN 4 || ODD)
 * @note INPUT: range [begin, end)
 * @note IN-PLACE
 * Treats the first half of the range as t and the second half as t + 1.
 * Output is first half as t and the second half as t - 1.
 ***/
template<typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
constexpr void reverse_back(Itr begin, Itr end, uint8_t rule) {
  const size_t size      = std::distance(begin, end);
  const size_t half_size = std::distance(begin, end) / 2;

  // [0, half_size) -> t
  // [half_size, size) -> t + 1

  // t - 1 leftmost
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

  // t - 1
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

  // t - 1 rightmost
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
}

/***
 * @brief iteration backwards of second order cellular automaton of given rule
 * @warning UNDEFINED BEHAVIOR FOR RANGES OF SIZE (LESS THAN 4 || ODD)
 * @note INPUT: range [begin, end)
 * @note IN-PLACE
 * Treats the first half of the range as t + 1 and the second half as t.
 * Output is first half as t - 1 and the second half as t.
 ***/
template<typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
constexpr void reverse_front(Itr begin, Itr end, uint8_t rule) {
  const size_t size      = std::distance(begin, end);
  const size_t half_size = std::distance(begin, end) / 2;

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
}

}    // namespace soca
